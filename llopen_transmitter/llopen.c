#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "llopen.h"

#define MAX_TRANSMISSIONS 3
#define TIMEOUT_SECONDS 3

typedef enum {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP
} State;

int llopen(int port, int flag) {
    int fd;
    struct termios newtio;

    // Open the serial port
    switch (port) {
        case 1:
            fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY);
            break;
        case 2:
            fd = open("/dev/ttyS1", O_RDWR | O_NOCTTY);
            break;
        default:
            printf("Invalid port number\n");
            return -1;
    }

    // Configure the serial port settings
    memset(&newtio, 0, sizeof(newtio));
    newtio.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 1;
    newtio.c_cc[VMIN] = 0;
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);

    if (flag == TRANSMITTER) {
        // Send SET frame to receiver
        unsigned char set_frame[] = {0x5C, 0x01, 0x03, 0x02, 0x5C};
        int bytes_written = write(fd, set_frame, sizeof(set_frame));

        if (bytes_written != sizeof(set_frame)) {
            printf("Error writing SET frame\n");
            close(fd);
            return -1;
        }
        printf("Wrote %d bits\n", bytes_written * 8);
        printf("SET frame sent\n");

        // Transmitter state machine
        State state = START;
        int i = 0;
        int ua_received = 0;

        while (state != STOP && !ua_received) {
            unsigned char received_frame;
            int n = read(fd, &received_frame, sizeof(received_frame));

            if (n == -1) {
                printf("Error reading from serial port\n");
                close(fd);
                return -1;
            } else if (n == 0) {
                continue;
            }

            switch (state) {
                case START:
                    if (received_frame == 0x5C)
                        state = FLAG_RCV;
                    break;
                case FLAG_RCV:
                    if (received_frame == 0x03)
                        state = A_RCV;
                    else if (received_frame != 0x5C)
                        state = START;
                    break;
                case A_RCV:
                    if (received_frame == 0x07)
                        state = C_RCV;
                    else if (received_frame == 0x5C)
                        state = FLAG_RCV;
                    else
                        state = START;
                    break;
                case C_RCV:
                    if (received_frame == (0x03 ^ 0x07))
                        state = BCC_OK;
                    else if (received_frame == 0x5C)
                        state = FLAG_RCV;
                    else
                        state = START;
                    break;
                case BCC_OK:
                    if (received_frame == 0x5C) {
                        state = FLAG_RCV;
                        ua_received = 1;
                    } else {
                        state = START;
                    }
                    break;
                default:
                    break;
            }
        }

        if (ua_received) {
            printf("UA frame received\n");
            printf("Connection established\n");
        } else {
            printf("Maximum transmissions reached or UA frame not received\n");
            close(fd);
            return -1;
        }

        // Return the data connection ID
        return fd;
    } else if (flag == RECEIVER) {
        // Receiver state machine
        State state = START;
        int i = 0;
        int set_received = 0;

        while (state != STOP && !set_received) {
            unsigned char received_frame;
            int n = read(fd, &received_frame, sizeof(received_frame));

            if (n == -1) {
                printf("Error reading from serial port\n");
                close(fd);
                return -1;
            } else if (n == 0) {
                continue;
            }

            switch (state) {
                case START:
                    if (received_frame == 0x5C)
                        state = FLAG_RCV;
                    break;
                case FLAG_RCV:
                    if (received_frame == 0x01)
                        state = A_RCV;
                    else if (received_frame != 0x5C)
                        state = START;
                    break;
                case A_RCV:
                    if (received_frame == 0x03)
                        state = C_RCV;
                    else if (received_frame == 0x5C)
                        state = FLAG_RCV;
                    else
                        state = START;
                    break;
                case C_RCV:
                    if (received_frame == (0x01 ^ 0x03))
                        state = BCC_OK;
                    else if (received_frame == 0x5C)
                        state = FLAG_RCV;
                    else
                        state = START;
                    break;
                case BCC_OK:
                    if (received_frame == 0x5C) {
                        state = FLAG_RCV;
                        set_received = 1;
                    } else {
                        state = START;
                    }
                    break;
                default:
                    break;
            }
        }

        if (set_received) {
            // Send UA frame to transmitter
            unsigned char ua_frame[] = {0x5C, 0x03, 0x07, 0x04, 0x5C};
            int bytes_written = write(fd, ua_frame, sizeof(ua_frame));

            if (bytes_written != sizeof(ua_frame)) {
                printf("Error writing UA frame\n");
                close(fd);
                return -1;
            }
            printf("Wrote %d bits\n", bytes_written * 8);
            printf("UA frame sent\n");
        } else {
            printf("Maximum transmissions reached or SET frame not received\n");
            close(fd);
            return -1;
        }

        // Return the data connection ID
        return fd;
    } else {
        printf("Invalid flag value\n");
        return -1;
    }
}
 

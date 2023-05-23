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
    newtio.c_cflag = B9600| CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 1;
    newtio.c_cc[VMIN] = 0;
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);

    if (flag == TRANSMITTER) {
        // Transmitter state machine
        typedef enum {
            TRANSMITTER_START,
            TRANSMITTER_SEND_SET,
            TRANSMITTER_WAIT_UA,
            TRANSMITTER_SUCCESS,
            TRANSMITTER_ERROR
        } TransmitterState;

        TransmitterState state = TRANSMITTER_START;
        int transmissions = 0;
        int ua_received = 0;
        int timer_expired = 0;

        while (state != TRANSMITTER_SUCCESS && state != TRANSMITTER_ERROR) {
            switch (state) {
                case TRANSMITTER_START:
                    state = TRANSMITTER_SEND_SET;
                    break;
                case TRANSMITTER_SEND_SET: {
                    unsigned char set_frame[] = { 0x5C, 0x01, 0x03, 0x02, 0x5C };
                    int bytes_written = write(fd, set_frame, sizeof(set_frame));

                    if (bytes_written != sizeof(set_frame)) {
                        printf("Error writing SET frame\n");
                        state = TRANSMITTER_ERROR;
                    } else {
                        printf("Wrote %d bits\n", bytes_written * 8);
                        printf("SET frame sent\n");
                        state = TRANSMITTER_WAIT_UA;
                    }
                    break;
                }
                case TRANSMITTER_WAIT_UA: {
                    // Start the timer
                    alarm(TIMEOUT_SECONDS);

                    // Wait for response from receiver
                    unsigned char expected_ua_frame[] = { 0x5C, 0x03, 0x07, 0x04, 0x5C };
                    unsigned char received_frame[5];
                    int bytes_read = 0;

                    while (bytes_read < sizeof(expected_ua_frame)) {
                        int n = read(fd, &received_frame[bytes_read], sizeof(received_frame) - bytes_read);
                        if (n == -1) {
                            printf("Error reading from serial port\n");
                            state = TRANSMITTER_ERROR;
                            break;
                        } else if (n == 0) {
                            continue;
                        } else {
                            bytes_read += n;
                        }
                    }

                    // Check if UA frame received
                    if (memcmp(received_frame, expected_ua_frame, sizeof(expected_ua_frame)) == 0) {
                        ua_received = 1;
                        state = TRANSMITTER_SUCCESS;
                    } else {
                        transmissions++;
                        if (transmissions >= MAX_TRANSMISSIONS) {
                            state = TRANSMITTER_ERROR;
                        } else {
                            state = TRANSMITTER_SEND_SET;
                        }
                    }

                    // Stop the timer
                    alarm(0);
                    break;
                }
                default:
                    break;
            }
        }

        if (state == TRANSMITTER_SUCCESS) {
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
        typedef enum {
            RECEIVER_START,
            RECEIVER_WAIT_SET,
            RECEIVER_SEND_UA,
            RECEIVER_SUCCESS,
            RECEIVER_ERROR
        } ReceiverState;

        ReceiverState state = RECEIVER_START;
        unsigned char expected_set_frame[] = { 0x5C, 0x01, 0x03, 0x02, 0x5C };
        unsigned char received_frame[5];
        int bits_read = 0;
        int bytes_read = 0;

        while (state != RECEIVER_SUCCESS && state != RECEIVER_ERROR) {
            switch (state) {
                case RECEIVER_START:
                    state = RECEIVER_WAIT_SET;
                    break;
                case RECEIVER_WAIT_SET: {
                    int n = read(fd, &received_frame[bytes_read], sizeof(received_frame) - bytes_read);
                    if (n == -1) {
                        printf("Error reading from serial port\n");
                        state = RECEIVER_ERROR;
                        break;
                    } else if (n == 0) {
                        continue;
                    } else {
                        bytes_read += n;
                    }

                    if (bytes_read == sizeof(expected_set_frame)) {
                        state = RECEIVER_SEND_UA;
                    }
                    break;
                }
                case RECEIVER_SEND_UA: {
                    for (int i = 0; i < bytes_read; i++) {
                        unsigned char expected_byte = expected_set_frame[i];
                        unsigned char received_byte = received_frame[i];

                        for (int j = 0; j < 8; j++) {
                            unsigned char expected_bit = (expected_byte >> (7 - j)) & 0x01;
                            unsigned char received_bit = (received_byte >> (7 - j)) & 0x01;

                            if (received_bit != expected_bit) {
                                printf("Invalid SET frame received\n");
                                state = RECEIVER_ERROR;
                                break;
                            }

                            bits_read++;
                        }
                    }

                    // Send UA frame to transmitter
                    unsigned char ua_frame[] = { 0x5C, 0x03, 0x07, 0x04, 0x5C };
                    int bits_written = write(fd, ua_frame, sizeof(ua_frame));

                    if (bits_written != sizeof(ua_frame)) {
                        printf("Error writing UA frame\n");
                        state = RECEIVER_ERROR;
                    } else {
                        printf("Wrote %d bits\n", bits_written * 8);
                        printf("UA frame sent\n");
                        state = RECEIVER_SUCCESS;
                    }
                    break;
                }
                default:
                    break;
            }
        }

        if (state == RECEIVER_SUCCESS) {
            printf("Connection established\n");
        } else {
            printf("Error in SET frame reception or writing UA frame\n");
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




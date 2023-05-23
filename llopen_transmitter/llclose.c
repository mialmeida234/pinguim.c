#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "llclose.h"

typedef enum {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP
} State;

int llclose(int fd) {
    if (fd < 0) {
        printf("Invalid file descriptor\n");
        return 1;
    }

    // Send DISC1 frame to receiver
    unsigned char disc1_frame[] = {0x5C, 0x01, 0x0B, 0x0A, 0x5C};
    int bytes_written = write(fd, disc1_frame, sizeof(disc1_frame));

    if (bytes_written != sizeof(disc1_frame)) {
        printf("Error writing DISC1 frame\n");
        return 1;
    }
    printf("Wrote %d bits\n", bytes_written * 8);
    printf("DISC1 frame sent\n");

    // Transmitter state machine
    State state = START;
    int i = 0;
    int disc2_received = 0;

    while (state != STOP && !disc2_received) {
        unsigned char received_frame;
        int n = read(fd, &received_frame, sizeof(received_frame));

        if (n == -1) {
            printf("Error reading from serial port\n");
            return 1;
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
                if (received_frame == 0x0B)
                    state = C_RCV;
                else if (received_frame == 0x5C)
                    state = FLAG_RCV;
                else
                    state = START;
                break;
            case C_RCV:
                if (received_frame == (0x01 ^ 0x0B))
                    state = BCC_OK;
                else if (received_frame == 0x5C)
                    state = FLAG_RCV;
                else
                    state = START;
                break;
            case BCC_OK:
                if (received_frame == 0x5C) {
                    state = FLAG_RCV;
                    disc2_received = 1;
                } else {
                    state = START;
                }
                break;
            default:
                break;
        }
    }

    if (disc2_received) {
        // Send DISC2 frame to receiver
        unsigned char disc2_frame[] = {0x5C, 0x03, 0x0B, 0x08, 0x5C};
        bytes_written = write(fd, disc2_frame, sizeof(disc2_frame));

        if (bytes_written != sizeof(disc2_frame)) {
            printf("Error writing DISC2 frame\n");
            return 1;
        }
        printf("Wrote %d bits\n", bytes_written * 8);
        printf("DISC2 frame sent\n");

        // Receiver state machine
        state = START;
        i = 0;
        int ua_received = 0;

        while (state != STOP && !ua_received) {
            unsigned char received_frame;
            int n = read(fd, &received_frame, sizeof(received_frame));

            if (n == -1) {
                printf("Error reading from serial port\n");
                return 1;
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
                    if (received_frame == 0x07)
                        state = C_RCV;
                    else if (received_frame == 0x5C)
                        state = FLAG_RCV;
                    else
                        state = START;
                    break;
                case C_RCV:
                    if (received_frame == (0x01 ^ 0x07))
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
            printf("Connection closed\n");
        } else {
            printf("Maximum transmissions reached or UA frame not received\n");
            return 1;
        }
    } else {
        printf("Maximum transmissions reached or DISC2 frame not received\n");
        return 1;
    }

    // Return 0 on success
    return 0;
}

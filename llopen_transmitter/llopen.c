#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "llopen.h"

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
    newtio.c_cflag = B19200 | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 1;
    newtio.c_cc[VMIN] = 0;
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);

    if (flag == TRANSMITTER) {
        // Send SET frame to receiver
        unsigned char set_frame[] = { 0x5A, 0x01, 0x83, 0x4A }; // 0101101000000001000000110000001001011010
        int bytes_written = write(fd, set_frame, sizeof(set_frame));

        if (bytes_written != sizeof(set_frame)) {
            printf("Error writing SET frame\n");
            close(fd);
            return -1;
        }
        printf("Wrote %d bits\n", bytes_written * 8);

        // Wait for response from receiver
        unsigned char expected_ua_frame[] = { 0x9A, 0x0C, 0x71, 0x4A }; // 0101101000000011000001110000010001011010
        unsigned char received_frame[4];
        int bits_read = 0;
        int bytes_read = 0;

        while (bytes_read < sizeof(expected_ua_frame)) {
            int n = read(fd, &received_frame[bytes_read], sizeof(received_frame) - bytes_read);
            if (n == -1) {
                printf("Error reading from serial port\n");
                close(fd);
                return -1;
            } else if (n == 0) {
                continue;
            } else {
                bytes_read += n;
            }
        }

        for (int i = 0; i < bytes_read; i++) {
            unsigned char expected_byte = expected_ua_frame[i];
            unsigned char received_byte = received_frame[i];

            for (int j = 0; j < 8; j++) {
                unsigned char expected_bit = (expected_byte >> (7 - j)) & 0x01;
                unsigned char received_bit = (received_byte >> (7 - j)) & 0x01;

                if (received_bit != expected_bit) {
                    printf("Invalid UA frame received\n");
                    close(fd);
                    return -1;
                }

                bits_read++;
            }
        }

        printf("Connection established\n");

        // Return the data connection ID
        return fd;
    } else if (flag == RECEIVER) {
        // Wait for SET frame from transmitter
        unsigned char expected_set_frame[] = { 0x5A, 0x01, 0x83, 0x4A }; // 0101101000000001000000110000001001011010
        unsigned char received_frame[4];
        int bits_read = 0;
        int bytes_read = 0;

        while (bytes_read < sizeof(expected_set_frame)) {
            int n = read(fd, &received_frame[bytes_read], sizeof(received_frame) - bytes_read);
            if (n == -1) {
                printf("Error reading from serial port\n");
                close(fd);
                return -1;
            } else if (n == 0) {
                continue;
            } else {
                bytes_read += n;
            }
        }

        for (int i = 0; i < bytes_read; i++) {
            unsigned char expected_byte = expected_set_frame[i];
            unsigned char received_byte = received_frame[i];

            for (int j = 0; j < 8; j++) {
                unsigned char expected_bit = (expected_byte >> (7 - j)) & 0x01;
                unsigned char received_bit = (received_byte >> (7 - j)) & 0x01;

                if (received_bit != expected_bit) {
                    printf("Invalid SET frame received\n");
                    close(fd);
                    return -1;
                }

                bits_read++;
            }
        }

        // Send UA frame to transmitter
        unsigned char ua_frame[] = { 0x9A, 0x0C, 0x71, 0x4A }; // 0101101000000011000001110000010001011010
        int bits_written = write(fd, ua_frame, sizeof(ua_frame));

        if (bits_written != sizeof(ua_frame)) {
            printf("Error writing UA frame\n");
            close(fd);
            return -1;
        }
        printf("Wrote %d bits\n", bits_written * 8);

        printf("Connection established\n");

        // Return the data connection ID
        return fd;
    } else {
        printf("Invalid flag value\n");
        return -1;
    }
}

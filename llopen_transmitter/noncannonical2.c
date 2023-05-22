#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include "headers.h"

#define BAUDRATE B38400
#define FLAG 0x7E
#define ESCAPE 0x7D
#define XON 0x11
#define XOFF 0x13

unsigned char calculateBCC2(unsigned char *data, int length) {
    unsigned char bcc2 = 0;
    for (int i = 0; i < length; i++) {
        bcc2 ^= data[i];
    }
    return bcc2;
}

int main() {
    int fd; // data link identifier
    struct termios oldtio, newtio;
    unsigned char receivedFrame[255]; // Maximum possible size for received frame
    int receivedLength = 0;
    unsigned char destuffedFrame[255]; // Maximum possible size for destuffed frame
    int destuffedLength = 0;
    unsigned char bcc2;

    // Open the serial port
    fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY);
    if (fd < 0) {
        perror("Error opening serial port");
        return 1;
    }

    // Save current port settings
    if (tcgetattr(fd, &oldtio) == -1) {
        perror("tcgetattr");
        return 1;
    }

    // Set new port settings for non-canonical input processing
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0;   // inter-character timer unused
    newtio.c_cc[VMIN] = 1;    // blocking read until 1 character arrives

    // Clean the port buffer
    tcflush(fd, TCIFLUSH);

    // Activate new settings for the port
    if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
        perror("tcsetattr");
        return 1;
    }

    // Read one byte at a time
    unsigned char receivedByte;
    while (read(fd, &receivedByte, 1) > 0) {
        // Check for frame start
        if (receivedByte == FLAG) {
            receivedLength = 0;
            destuffedLength = 0;
        }

        // Store received byte in received frame
        receivedFrame[receivedLength++] = receivedByte;

        // Perform de-stuffing
        if (receivedByte == ESCAPE) {
            if (read(fd, &receivedByte, 1) <= 0) {
                perror("Error reading from serial port");
                break;
            }
            receivedByte ^= 0x20;
        }

        // Check for frame end
        if (receivedByte == FLAG) {
            // Validate BCC2
            bcc2 = receivedFrame[receivedLength - 2];
            if (bcc2 == calculateBCC2(receivedFrame + 1, receivedLength - 4)) {
                // BCC2 is correct, process the frame
                // ...
                // Perform further processing here
                // ...
                // Print the received frame
                printf("Received Frame: ");
                for (int i = 0; i < receivedLength; i++) {
                    printf("%02X ", receivedFrame[i]);
                }
                printf("\n");
            } else {
                printf("BCC2 error: Expected 0x%02X, Received 0x%02X\n", calculateBCC2(receivedFrame + 1, receivedLength - 4), bcc2);
            }
        }
    }

    // Restore old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
        perror("tcsetattr");
        return 1;
    }

    // Close the serial port
    close(fd);

    return 0;
}

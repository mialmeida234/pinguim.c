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

int llread(int fd, char *buffer) {
    unsigned char frame[255]; // Maximum possible size for frame
    int frameLength = 0;
    unsigned char receivedBCC2;

    // Read the frame from the serial port
    while (1) {
        unsigned char receivedChar;
        int res = read(fd, &receivedChar, 1);
        if (res < 0) {
            perror("Error reading from serial port");
            return -1;
        }

        // Check for start flag
        if (receivedChar == FLAG && frameLength == 0) {
            frame[frameLength++] = receivedChar;
        }
        // Check for end flag
        else if (receivedChar == FLAG && frameLength > 0) {
            frame[frameLength++] = receivedChar;
            break; // End of frame, exit loop
        }
        // Check for escape character
        else if (receivedChar == ESCAPE) {
            unsigned char nextChar;
            res = read(fd, &nextChar, 1);
            if (res < 0) {
                perror("Error reading from serial port");
                return -1;
            }
            // De-stuffing: XOR next character with 0x20
            frame[frameLength++] = nextChar ^ 0x20;
        }
        // Regular data or control characters
        else {
            frame[frameLength++] = receivedChar;
        }
    }

    // Extract BCC2 value
    receivedBCC2 = frame[frameLength - 2];

    // Calculate BCC2 value
    unsigned char calculatedBCC2 = 0;
    for (int i = 1; i < frameLength - 2; i++) {
        calculatedBCC2 ^= frame[i];
    }

    // Check BCC2 value
    if (receivedBCC2 != calculatedBCC2) {
        printf("BCC2 error: received 0x%02X, calculated 0x%02X\n", receivedBCC2, calculatedBCC2);
        return -1;
    }

    // Copy data to output buffer
    memcpy(buffer, frame + 4, frameLength - 6);

    // Return number of characters read
    return frameLength - 6;
}

int main() {
    int fd; // data link identifier
    struct termios oldtio, newtio;
    char buffer[255]; // Buffer to store received data

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

    // Read from the serial port using llread
    int bytesRead = llread(fd, buffer);
    if (bytesRead < 0) {
        printf("llread failed\n");
    } else {
        printf("llread succeeded: %d bytes received\n", bytesRead);
        // Process the received data
        for (int i = 0; i < bytesRead; i++) {
            printf("Byte %d: 0x%02X\n", i, buffer[i]);
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


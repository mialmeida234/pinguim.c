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
    unsigned char frame[] = { FLAG, 'H', 'e', 'l', 'l', 'o', '!', FLAG }; // Example frame
    unsigned char stuffedFrame[sizeof(frame) * 2]; // Maximum possible size for stuffed frame
    int stuffedLength = 0;

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

    // Calculate BCC2
    unsigned char bcc2 = calculateBCC2(frame + 1, sizeof(frame) - 3);

    // Stuff the frame
    stuffedFrame[0] = frame[0];
    stuffedLength = 1;
    for (int i = 1; i < sizeof(frame) - 1; i++) {
        if (frame[i] == FLAG || frame[i] == ESCAPE || frame[i] == XON || frame[i] == XOFF) {
            stuffedFrame[stuffedLength++] = ESCAPE;
            stuffedFrame[stuffedLength++] = frame[i] ^ 0x20;
        } else {
            stuffedFrame[stuffedLength++] = frame[i];
        }
    }
    stuffedFrame[stuffedLength++] = frame[sizeof(frame) - 1];

    // Write the stuffed frame
    if (write(fd, stuffedFrame, stuffedLength) != stuffedLength) {
        perror("Error writing to serial port");
        return 1;
    }

    // Read one byte at a time
    unsigned char receivedByte;
    while (read(fd, &receivedByte, 1) > 0) {
        // Process received byte
        // ...
    }

    // Restore old port settings
    tcsetattr(fd, TCSANOW, &oldtio);

    // Close the serial port
    close(fd);

    return 0;
}


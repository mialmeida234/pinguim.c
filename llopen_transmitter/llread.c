#include "headers.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>

#define MAX_SIZE 255
#define BAUDRATE B38400
#define FLAG 0x7E
#define A 0x03
#define C 0x00

int llread(int fd, char *buffer) {
    unsigned char frame[MAX_SIZE];
    unsigned char ua[5];
    int res, i = 0, j = 0, k = 0;
    struct termios oldtio, newtio;

    // Save current port settings
    if (tcgetattr(fd, &oldtio) == -1) {
        perror("tcgetattr");
        return -1;
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
        return -1;
    }

    // Wait for start flag
    while (frame[i-1] != FLAG) {
        res = read(fd, &frame[i], 1);
        if (res < 0) {
            perror("read");
            return -1;
        }
        else if (res == 0) {
            printf("llread: no data available\n");
            return -1;
        }
        else {
            i++;
        }
    }

    // Store address and control fields
    if (frame[1] != A || frame[2] != C) {
        printf("llread: invalid address or control fields\n");
        return -1;
    }

    // Store data field in buffer
    j = 0;
    for (k = 4; frame[k] != FLAG; k++) {
        if (j >= MAX_SIZE) {
            printf("llread: frame too large\n");
            return -1;
        }
        buffer[j++] = frame[k];
    }

    // Restore old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
        perror("tcsetattr");
        return -1;
    }

    // Return number of characters read
    return j;
}

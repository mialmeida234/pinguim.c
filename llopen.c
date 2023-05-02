#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "llopen.h"


int llopen(int port) {
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
    newtio.c_cflag = B19200| CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 1;
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);

    // Send SET frame to receiver
    //char set_frame[5] = {0x07, 0x03, 0x0F, 0x00, 0x0F}; // SET frame
    char set_frame[]="SET";
    int bytes_written = write(fd, set_frame, 3);
    
    if (bytes_written != 3) {
        printf("Error writing SET frame\n");
        close(fd);
        return -1;
    }
    printf("Wrote %d bytes\n", bytes_written);

    // Wait for response from receiver
    char buf[1];
    int bytes_read = 0;
    while (bytes_read < 5) {
        int n = read(fd, buf, 1);
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
    if (buf[0] != 0x07) {
        printf("Invalid response received\n");
        close(fd);
        return -1;
    }

    printf("Connection established\n");

    return fd;
}
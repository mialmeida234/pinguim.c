#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include "headers.h"

#define MAX_SIZE 255
#define FALSE 0
#define TRUE 1

#define BAUDRATE B38400
#define FLAG 0x7E
#define A 0x03
#define C_SET 0x03
#define C_DISC 0x0B
#define C_UA 0x07
#define C 0x00 // assuming we are sending an information frame



int fd;  // data link identifier

int set_termios(int fd, struct termios *oldtio) {
    struct termios newtio;

    if (tcgetattr(fd, oldtio) == -1) { /* save current port settings */
        perror("tcgetattr");
        return -1;
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
    newtio.c_cc[VMIN] = 1;  /* blocking read until 1 char received */

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
        perror("tcsetattr");
        return -1;
    }

    printf("New termios structure set\n");

    return 0;
}

int llwrite(int fd, char *buffer, int length) {
    int res;
    int i;
    char frame[MAX_SIZE];

    // Build the frame
    frame[0] = FLAG;
    frame[1] = A;
    frame[2] = C;
    for (i = 0; i < length; i++)
        frame[3 + i] = buffer[i];

    // Compute the BCC
    char bcc = buffer[0];
    for (i = 1; i < length; i++)
        bcc ^= buffer[i];
    frame[3 + length] = bcc;

    // Add the final flag
    frame[4 + length] = FLAG;

    // Write the frame
    res = write(fd, frame, length + 5);

    return res;
}

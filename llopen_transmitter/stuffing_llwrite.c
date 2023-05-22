#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>

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

int llwrite(int fd, char *buffer, int length) {
    unsigned char frame[255]; // Maximum possible size for frame
    int frameLength = 0;

    // Add the start flag
    frame[frameLength++] = FLAG;

    // Add the data to the frame and perform stuffing
    for (int i = 0; i < length; i++) {
        if (buffer[i] == FLAG || buffer[i] == ESCAPE) {
            frame[frameLength++] = ESCAPE;
            frame[frameLength++] = buffer[i] ^ 0x20;
        } else {
            frame[frameLength++] = buffer[i];
        }
    }

    // Calculate and add the BCC2 value
    unsigned char bcc2 = calculateBCC2((unsigned char *)buffer, length);
    if (bcc2 == FLAG || bcc2 == ESCAPE) {
        frame[frameLength++] = ESCAPE;
        frame[frameLength++] = bcc2 ^ 0x20;
    } else {
        frame[frameLength++] = bcc2;
    }

    // Add the end flag
    frame[frameLength++] = FLAG;

    // Write the frame to the serial port
    int res = write(fd, frame, frameLength);
    if (res < 0) {
        perror("Error writing to serial port");
        return -1;
    }

    return res;
}

int main() {
    int fd; // data link identifier
    struct termios oldtio, newtio;
    char buffer[] = {0x41, 0x42, 0x43}; // Example data buffer

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

    // Set new port settings for non-canonical output processing
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0;   // inter-character timer unused
    newtio.c_cc[VMIN] = 1;    // blocking write until 1 character can be sent

    // Clean the port buffer
    tcflush(fd, TCIFLUSH);

    // Activate new settings for the port
    if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
        perror("tcsetattr");
        return 1;
    }

    // Perform llwrite with the example buffer
    int res = llwrite(fd, buffer, sizeof(buffer));
    if (res < 0) {
        printf("llwrite failed\n");
    } else {
        printf("llwrite succeeded\n");
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

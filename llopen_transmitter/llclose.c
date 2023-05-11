#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "llclose.h"

#define MAX_RETRIES 3
#define TIMEOUT 3

int llclose(int fd) {

    struct termios oldtio;
    
    // Disable receiver and transmitter
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
        perror("Error disabling receiver and transmitter");
        return 1;
    }

    // Close the serial port
    if (close(fd) == -1) {
        perror("Error closing serial port");
        return 2;
    }

    printf("Serial port closed successfully.\n");

    return 0;
}




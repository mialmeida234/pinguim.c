#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "llopen.h"
#include "llread.h"
#include "llwrite.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s [TRANSMITTER | RECEIVER]\n", argv[0]);
        return -1;
    }

    int fd;
    if (strcmp(argv[1], "TRANSMITTER") == 0) {
        fd = llopen(1, TRANSMITTER);
        if (fd != -1) {
            // Perform llwrite operations
            char buffer[] = "HELLO WORLD";
            int length = sizeof(buffer) - 1; // Exclude null terminator
            int result = llwrite(fd, buffer, length);
            if (result == -1) {
                printf("[TEST]Error writing data\n");
            }
            printf("\n");
            printf("[TEST] bytes escritos llwrite : %d", result );
        }
    } else if (strcmp(argv[1], "RECEIVER") == 0) {
        fd = llopen(2, RECEIVER);
        if (fd != -1) {
            // Perform llread operations
            char buffer[256]; // Assuming maximum frame size of 256
            int result = llread(fd, buffer);
            if (result == -1) {
                printf("[TEST]Error reading data\n");
            } else {
                printf("[TEST]Received data: %s\n", buffer);
            }
        }
    } else {
        printf("[TEST]Invalid argument: %s\n", argv[1]);
        return -1;
    }

    if (fd == -1) {
        printf("[TEST]Error establishing connection\n");
        return -1;
    }

    close(fd);
    return 0;
}

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "llopen.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s [TRANSMITTER | RECEIVER]\n", argv[0]);
        return -1;
    }

    int fd;
    if (strcmp(argv[1], "TRANSMITTER") == 0) {
        fd = llopen(1, TRANSMITTER);
    } else if (strcmp(argv[1], "RECEIVER") == 0) {
        fd = llopen(2, RECEIVER);
    } else {
        printf("Invalid argument: %s\n", argv[1]);
        return -1;
    }

    if (fd == -1) {
        printf("Error establishing connection\n");
        return -1;
    }

    close(fd);
    return 0;
}

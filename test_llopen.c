#include <stdio.h>
#include <unistd.h>
#include "llopen.h"

int main() {
    int fd = llopen(1);
    if (fd == -1) {
        printf("Error establishing connection\n");
        return -1;
    }


    close(fd);
    return 0;
}
//controls the port

//finite-time machine


#include <stdio.h>
#include <stdlib.h>

typedef enum {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP
} State;

int main() {
    State state = START;
    unsigned char frame[] = { 0x5A, 0x01, 0x83, 0x4A }; // Example SET frame
    int i = 0;

    while (state != STOP) {
        switch (state) {
            case START:
                if (frame[i] == 0x5A)
                    state = FLAG_RCV;
                break;
            case FLAG_RCV:
                if (frame[i] == 0x01)
                    state = A_RCV;
                else if (frame[i] != 0x5A)
                    state = START;
                break;
            case A_RCV:
                if (frame[i] == 0x01)
                    state = C_RCV;
                else if (frame[i] == 0x5A)
                    state = FLAG_RCV;
                else
                    state = START;
                break;
            case C_RCV:
                if (frame[i] == (0x01 ^ 0x01))
                    state = BCC_OK;
                else if (frame[i] == 0x5A)
                    state = FLAG_RCV;
                else
                    state = START;
                break;
            case BCC_OK:
                if (frame[i] == 0x5A)
                    state = STOP;
                break;
            default:
                break;
        }

        i++;
    }

    if (state == STOP) {
        printf("SET message received successfully.\n");
    } else {
        printf("Error in SET message reception.\n");
    }

    return 0;
}


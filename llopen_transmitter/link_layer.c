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
    unsigned char set_frame[] = { 0x5A, 0x01, 0x83, 0x4A }; // Example SET frame
    int i = 0;

    while (state != STOP) {
        switch (state) {
            case START:
                if (set_frame[i] == 0x5A)
                    state = FLAG_RCV;
                break;
            case FLAG_RCV:
                if (set_frame[i] == 0x01)
                    state = A_RCV;
                else if (set_frame[i] != 0x5A)
                    state = START;
                break;
            case A_RCV:
                if (set_frame[i] == 0x83)
                    state = C_RCV;
                else if (set_frame[i] == 0x5A)
                    state = FLAG_RCV;
                else
                    state = START;
                break;
            case C_RCV:
                if (set_frame[i] == (0x01 ^ 0x83))
                    state = BCC_OK;
                else if (set_frame[i] == 0x5A)
                    state = FLAG_RCV;
                else
                    state = START;
                break;
            case BCC_OK:
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




int main() {
    State state = START;
    unsigned char receivedByte;  // Simulated received byte
    unsigned char expectedBytes[] = {0x9A, 0x0C, 0x71, 0x4A};  // Expected UA message
    int index = 0;

    // Simulated reception loop (replace this with your actual reception logic)
    while (state != STOP) {
        // Simulated received byte (replace this with your actual byte reading logic)
        receivedByte = expectedBytes[index];

        switch (state) {
            case START:
                if (receivedByte == 0x9A)
                    state = FLAG_RCV;
                break;
            case FLAG_RCV:
                if (receivedByte == 0x0C)
                    state = A_RCV;
                else if (receivedByte != 0x9A)
                    state = START;
                break;
            case A_RCV:
                if (receivedByte == 0x71)
                    state = C_RCV;
                else if (receivedByte == 0x9A)
                    state = FLAG_RCV;
                else
                    state = START;
                break;
            case C_RCV:
                if (receivedByte == 0x4A)
                    state = BCC_OK;
                else if (receivedByte == 0x9A)
                    state = FLAG_RCV;
                else
                    state = START;
                break;
            case BCC_OK:
                state = (receivedByte == 0x9A) ? FLAG_RCV : START;
                break;
            default:
                break;
        }

        index++;
    }

    if (state == STOP) {
        printf("UA message received successfully.\n");
    } else {
        printf("Error in UA message reception.\n");
    }

    return 0;
}


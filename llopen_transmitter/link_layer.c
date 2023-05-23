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
    unsigned char set_frame[] = { 0x5C, 0x01, 0x03, 0x02, 0x5C }; // Example SET frame
    int i = 0;

    while (state != STOP) {
        switch (state) {
            case START:
                if (set_frame[i] == 0x5C)
                    state = FLAG_RCV;
                break;
            case FLAG_RCV:
                if (set_frame[i] == 0x01)
                    state = A_RCV;
                else if (set_frame[i] != 0x5C)
                    state = START;
                break;
            case A_RCV:
                if (set_frame[i] == 0x03)
                    state = C_RCV;
                else if (set_frame[i] == 0x5C)
                    state = FLAG_RCV;
                else
                    state = START;
                break;
            case C_RCV:
                if (set_frame[i] == (0x01 ^ 0x03))
                    state = BCC_OK;
                else if (set_frame[i] == 0x5C)
                    state = FLAG_RCV;
                else
                    state = START;
                break;
            case BCC_OK:
                state = (set_frame[i] == 0x5C) ? FLAG_RCV : START;
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

    state = START;
    unsigned char receivedByte;  // Simulated received byte
    unsigned char expectedBytes[] = {0x5C, 0x03, 0x07, 0x04, 0x5C};  // Expected UA message
    int index = 0;

    while (state != STOP) {
        receivedByte = expectedBytes[index];

        switch (state) {
            case START:
                if (receivedByte == 0x5C)
                    state = FLAG_RCV;
                break;
            case FLAG_RCV:
                if (receivedByte == 0x03)
                    state = A_RCV;
                else if (receivedByte != 0x5C)
                    state = START;
                break;
            case A_RCV:
                if (receivedByte == 0x07)
                    state = C_RCV;
                else if (receivedByte == 0x5C)
                    state = FLAG_RCV;
                else
                    state = START;
                break;
            case C_RCV:
                if (receivedByte == 0x04)
                    state = BCC_OK;
                else if (receivedByte == 0x5C)
                    state = FLAG_RCV;
                else
                    state = START;
                break;
            case BCC_OK:
                state = (receivedByte == 0x5C) ? FLAG_RCV : START;
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


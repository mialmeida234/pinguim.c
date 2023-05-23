#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include "llread.h"

#define BUFFER_SIZE 1024

int llread(int fd, char *buffer) {
    ssize_t bytesRead = 0;
    ssize_t bytesReadCurrent = 0;
    int startIndex = -1;
    int endIndex = -1;
    int flagCount = 0;
    int sequenceNumber = -1;

    while (bytesRead < BUFFER_SIZE) {
        bytesReadCurrent = read(fd, buffer + bytesRead, 1);

        if (bytesReadCurrent == -1) {
            // Error occurred while reading
            return -1;
        } else if (bytesReadCurrent == 0) {
            // Reached end of input
            break;
        } else {
            bytesRead += bytesReadCurrent;

            if (buffer[bytesRead - 1] == 0x5C) {
                flagCount++;

                if (flagCount == 1 && startIndex == -1) {
                    // First flag encountered, mark its index
                    startIndex = bytesRead - 1;
                } else if (flagCount == 2 && startIndex != -1) {
                    // Second flag encountered, mark its index and stop reading
                    endIndex = bytesRead - 1;
                    break;
                }
            }
        }
    }

    printf("[READ] Frame contents: ");
    for (int i = 0; i < bytesRead; i++) {
        printf("%02X ", (unsigned char)buffer[i]);
    }
    printf("\n");

    if (startIndex != -1 && endIndex != -1 && startIndex < endIndex) {
        int isolatedLength = endIndex - startIndex + 1;
        unsigned char *isolatedContents = (unsigned char *)malloc(isolatedLength * sizeof(unsigned char));

        if (isolatedContents != NULL) {
            for (int i = 0; i < isolatedLength; i++) {
                isolatedContents[i] = (unsigned char)buffer[startIndex + i];
            }

            printf("[READ] Frame desejado: ");
            for (int i = 0; i < isolatedLength; i++) {
                printf("%02X ", (unsigned char)isolatedContents[i]);
            }
            printf("\n");

            // Rest of the code remains unchanged
            // ...
            // Verifica se isolatedContents tem o formato correto
                if (isolatedContents[0] == 0x5C &&
                    isolatedContents[1] == 0x01 &&
                    (isolatedContents[2] == 0x00 || isolatedContents[2] == 0x02) &&
                    isolatedContents[3] == (0x01 ^ isolatedContents[2])) {
                    // Extrair o valor de S a partir do isolatedContents
                    if (isolatedContents[2] == 0x00)
                        sequenceNumber = 0;
                    else if (isolatedContents[2] == 0x02)
                        sequenceNumber = 1;
                    else {
                        printf("[READ]Erro: Valor de S invalido.\n");
                        free(isolatedContents);
                        return -1;
                    }

                    // Determinar o valor de R
                    int oppositeSequenceNumber = sequenceNumber ^ 1;

                    printf("[READ]Isolated contents: ");
                    for (int i = 0; i < isolatedLength; i++) {
                        printf("%02X ", isolatedContents[i]);
                    }
                    printf("\n");

                    printf("[READ] Value of S: %d\n", sequenceNumber);
                    printf("[READ] Value of R: %d\n", oppositeSequenceNumber);

                    // Gerar a mensagem de controlo de acordo com o valor de R
                    unsigned char rrMessage[5];
                    rrMessage[0] = 0x5C;
                    rrMessage[1] = 0x03;

                    if (sequenceNumber == 0)
                        rrMessage[2] = 0x00;
                    else if (sequenceNumber == 1)
                        rrMessage[2] = 0x02;

                    rrMessage[3] = rrMessage[1] ^ rrMessage[2];  // Operação XOR -> BCC1
                    printf("[xor] %02X ", rrMessage[3]);
                    printf("\n");
                    rrMessage[4] = 0x5C;

                    // Escrever a mensagem de controlo
                    write(fd, rrMessage, sizeof(rrMessage));
                    printf("[READ]Mensgaem RR enviada.\n");
                } else {
                    printf("[READ]Formato do conteúdo isolado incorreto.\n");
                    free(isolatedContents);
                    return -1;
                }

            free(isolatedContents);
        } else {
            printf("[READ] Alocação de memória falhou.\n");
        }
    } else {
        // Só uma ocorrência ocorreu um erro
            printf("[READ]Só uma ocorrência da FLAG.\n");
            unsigned char rejMessage[5];
            rejMessage[0] = 0x5C;
            rejMessage[1] = 0x03;

            if (sequenceNumber == 0)
                rejMessage[2] = 0x25;
            else if (sequenceNumber == 1)
                rejMessage[2] = 0x05;

            rejMessage[3] = rejMessage[1] ^ rejMessage[2];
            rejMessage[4] = 0x5C;

            // Escrever a mensagem de controlo
            write(fd, rejMessage, sizeof(rejMessage));
            return -1;
    }

    return (int)bytesRead;
}


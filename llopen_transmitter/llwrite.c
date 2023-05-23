#include <stdio.h>
#include <unistd.h>
#include "llwrite.h"
int llwrite(int fd, char* buffer, int length) {
    // GERA NUMERO DE SEQUENCIA S DE ACORDO COM O NUMERO DE FRAMES ENVIADO
    static unsigned int sequenceNumber = 0;
    unsigned char controlField = sequenceNumber << 1; // Shift para a esquerda do numero de sequencia by 1

    // Prepara frame
    unsigned char frame[4 + length + 4];
    frame[0] = 0x05C;
    frame[1] = 0x01;
    frame[2] = controlField;
    frame[3] = frame[1] ^ frame[2]; // CALCULA XOR do A com controlField

    // COPIA OS CONTEUDOS DO BUFFER PARA O FRAME
    unsigned char BCC2 = 0;
    for (int i = 0; i < length; i++) {
        frame[4 + i] = buffer[i];
        BCC2 ^= buffer[i];
    }

    // adiciona o A XOR C e a FLAG ao fim do frame
    frame[4 + length] = BCC2; // A XOR C
    frame[4 + length + 1] = frame[0]; // FLAG 

    // Envia o frame 
    int bytes_written = write(fd, frame, sizeof(frame));

    if (bytes_written == -1) {
        printf("[WRITE]Error writing to data link\n");
        return -1;
    }
    printf("[WRITE]Frame contents: ");
    for (int i = 0; i < sizeof(frame); i++) {
            printf("%02X ", frame[i]);
        }
    printf("\n");

    // Receção da mensagem RR ou REJ do RECEIVER
    unsigned char controlMessage[5];
    int bytes_read = 0;
    int bytes_expected = sizeof(controlMessage);

    while (bytes_read < bytes_expected) {
        int result = read(fd, controlMessage + bytes_read, bytes_expected - bytes_read);

        if (result == -1) {
            printf("[WRITE]Error reading control message\n");
            return -1;
        }

        if (result == 0) {
            printf("[WRITE]End of file reached\n");
            return -1;
        }

        bytes_read += result;
    }

    if (bytes_read != bytes_expected) {
        printf("[WRITE]Invalid control message length\n");
        return -1;
    }
// possivelmente alterar para os dois cadsos possiveis de rr e os dois casos de rej
     
    if (controlMessage[0] == 0x05C && controlMessage[1] == 0x03 && controlMessage[4] == 0x05C) {
        unsigned char controlField = controlMessage[2];
        unsigned char receivedXOR = controlMessage[3];
        unsigned char expectedXOR = controlMessage[1] ^ controlMessage[2];
       
        printf("[received xor] %02X ", controlMessage[3]);
                    printf("\n");
        printf("[expected xor] %02X ", expectedXOR);
                    printf("\n");

        if (receivedXOR == expectedXOR) {
            if ((controlField & 0x01) == sequenceNumber) {
                // RR received
                sequenceNumber = (sequenceNumber + 1) % 2; // Increment the sequence number for the next frame
                 printf("RR ");
                return bytes_written - 8; // Exclui os delimitadores
            } else {
                // REJ received
                printf("REJ ");
                return -1;
            }
        } else {
            printf("[WRITE]Control message XOR error\n");
             printf("[WRITE] else ");
            return -1;
        }
        
    } else {
        printf("[WRITE]Invalid control message format\n");
        printf("[WRITE] 4º else ");
        return -1;
    }
 printf("[WRITE] 4º if ");
}


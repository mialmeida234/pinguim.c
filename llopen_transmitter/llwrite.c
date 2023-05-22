#include <stdio.h>
#include <unistd.h>

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
        printf("Error writing to data link\n");
        return -1;
    }

    // Receção da mensagem RR ou REJ do RECEIVER
    unsigned char controlMessage[5];
    int bytes_read = 0;
    int bytes_expected = sizeof(controlMessage);

    while (bytes_read < bytes_expected) {
        int result = read(fd, controlMessage + bytes_read, bytes_expected - bytes_read);

        if (result == -1) {
            printf("Error reading control message\n");
            return -1;
        }

        if (result == 0) {
            printf("End of file reached\n");
            return -1;
        }

        bytes_read += result;
    }

    if (bytes_read != bytes_expected) {
        printf("Invalid control message length\n");
        return -1;
    }
// possivelmente alterar para os dois cadsos possiveis de rr e os dois casos de rej

    if (controlMessage[0] == 0x05C && controlMessage[1] == 0x03 && controlMessage[4] == 0x05C) {
        unsigned char controlField = controlMessage[2];
        unsigned char receivedXOR = controlMessage[3];
        unsigned char expectedXOR = frame[1] ^ frame[2];

        if (receivedXOR == expectedXOR) {
            if ((controlField & 0x01) == sequenceNumber) {
                // RR received
                sequenceNumber = (sequenceNumber + 1) % 2; // Increment the sequence number for the next frame
                return bytes_written - 8; // Exclui os delimitadores
            } else {
                // REJ received
                return -1;
            }
        } else {
            printf("Control message XOR error\n");
            return -1;
        }
         if (receivedXOR == expectedXOR) {
             int expectedControlField = (sequenceNumber == 1) ? 0b00000001 : 0b00100001;

             if (controlField == expectedControlField) {
                 // RR received
                sequenceNumber = (sequenceNumber + 1) % 2; // Increment the sequence number for the next frame
                return bytes_written - 8; // Exclude the delimiters
             } else {
               // REJ received
               return -1;
               }
        } else {
           printf("Control message XOR error\n");
           return -1;
         }

        
    } else {
        printf("Invalid control message format\n");
        return -1;
    }
}


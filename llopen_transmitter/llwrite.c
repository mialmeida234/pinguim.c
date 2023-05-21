#include <stdio.h>
#include <unistd.h>

int llwrite(int fd, char* buffer, int length) {
    // Generate the sequence number (S) based on the number of frames sent
    static unsigned int sequenceNumber = 0;
    unsigned char controlField = sequenceNumber << 1; // Shift the sequence number left by 1

    // Prepare the frame
    unsigned char frame[4 + length + 4];
    frame[0] = 0x05C;
    frame[1] = 0x01;
    frame[2] = controlField;
    frame[3] = frame[0] ^ frame[1] ^ frame[2]; // Calculate the XOR checksum

    // Copy the buffer into the frame
    for (int i = 0; i < length; i++) {
        frame[4 + i] = buffer[i];
    }

    // Calculate the XOR checksum for the buffer
    unsigned char bufferChecksum = 0;
    for (int i = 0; i < length; i++) {
        bufferChecksum ^= buffer[i];
    }

    // Add the buffer checksum and end of frame marker to the frame
    frame[4 + length] = bufferChecksum ^ controlField; // XOR checksum with control field
    frame[4 + length + 1] = frame[0];

    // Send the frame to the data link
    int bytes_written = write(fd, frame, sizeof(frame));

    if (bytes_written == -1) {
        printf("Error writing to data link\n");
        return -1;
    }

    // Start the timer

    // Wait for the acknowledgment (ACK) or timeout
    int ack_received = 0;
    int retransmission_count = 0;

    while (!ack_received && retransmission_count < 3) {
        // Receive the ACK frame
        unsigned char received_frame[4];
        int bytes_read = read(fd, received_frame, sizeof(received_frame));

        if (bytes_read == -1) {
            printf("Error reading from data link\n");
            return -1;
        } else if (bytes_read == 0) {
            // Timeout occurred, retransmit the frame
            bytes_written = write(fd, frame, sizeof(frame));
            retransmission_count++;
        } else {
            // ACK frame received, check the type
            if (received_frame[2] == controlField) {
                if (received_frame[3] == frame[0] ^ frame[1] ^ frame[2]) {
                    // RR (Receiver Ready) ACK received
                    ack_received = 1;
                } else {
                    // REJ (Reject) ACK received, handle failure/error
                    printf("REJ ACK received\n");
                    return -1;
                }
            }
        }
    }

    if (!ack_received) {
        printf("Maximum retransmission attempts reached\n");
        return -1;
    }

    // Stop the timer

    // Increment the sequence number for the next frame
    sequenceNumber = (sequenceNumber + 1) % 2; // Assuming S can only be 0 or 1

    return bytes_written - 8; // Exclude the frame delimiters and checksums from the return value
}

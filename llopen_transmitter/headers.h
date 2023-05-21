#ifndef LLWRITE_H
#define LLWRITE_H

/**
 * Sends a message through the data link layer.
 *
 * @param fd The file descriptor of the serial port.
 * @param buffer The message to send.
 * @param length The length of the message.
 *
 * @return The number of characters written to the serial port, or a negative
 * value if an error occurs.
 */
int llwrite(int fd, char *buffer, int length);

#endif /* LLWRITE_H */


#ifndef LLREAD_H
#define LLREAD_H

/**
 * @brief Reads data from the serial port and performs byte stuffing and error checking.
 * 
 * @param fd Data link identifier
 * @param buffer Buffer to store received data
 * @return Number of bytes read, or negative value in case of error
 */
int llread(int fd, char *buffer);

#endif /* LLREAD_H */


#ifndef LLOPEN_H
#define LLOPEN_H

#define TRANSMITTER 0
#define RECEIVER 1

int llopen(int port, int flag);

#endif


#ifndef LLCLOSE_H
#define LLCLOSE_H

int llclose(int fd);

#endif // LLCLOSE_H
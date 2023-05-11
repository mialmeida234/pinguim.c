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

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

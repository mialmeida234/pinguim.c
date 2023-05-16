int main() {
    // Open the serial port
    int fd = open("/dev/ttyS0", O_RDWR);
    if (fd < 0) {
        perror("Failed to open serial port");
        return 1;
    }

    // Set the serial port settings
    struct termios oldtio;
    if (tcgetattr(fd, &oldtio) == -1) {
        perror("Failed to get termios settings");
        close(fd);
        return 1;
    }

    // Read data from the serial port
    char buffer[MAX_SIZE];
    int bytesRead = llread(fd, buffer);
    if (bytesRead < 0) {
        perror("llread failed");
        close(fd);
        return 1;
    }

    // Print the received data
    printf("Received data: %s\n", buffer);

    // Restore the old serial port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
        perror("Failed to restore termios settings");
        close(fd);
        return 1;
    }

    // Close the serial port
    close(fd);

    return 0;
}

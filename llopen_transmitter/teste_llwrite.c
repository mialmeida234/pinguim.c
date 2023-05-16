int main() {
    // Open the serial port
    int fd = open("/dev/ttyS0", O_WRONLY);
    if (fd < 0) {
        perror("Failed to open serial port");
        return 1;
    }

    // Set the serial port settings
    struct termios oldtio;
    if (set_termios(fd, &oldtio) != 0) {
        close(fd);
        return 1;
    }

    // Example buffer to write
    char buffer[] = "Hello, world!";
    int length = strlen(buffer);

    // Call the llwrite function
    int result = llwrite(fd, buffer, length);
    if (result < 0) {
        perror("llwrite failed");
        close(fd);
        return 1;
    }

    printf("Successfully sent %d characters.\n", result);

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

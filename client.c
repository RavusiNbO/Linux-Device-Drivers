#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(void)
{
    const char *message = "hello\n";
    int fd = open("/dev/scull0", O_WRONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    ssize_t count = write(fd, message, strlen(message));
    if (count < 0) {
        perror("write");
        close(fd);
        return 1;
    }

    printf("%s\n", message);
    close(fd);
    return 0;
}
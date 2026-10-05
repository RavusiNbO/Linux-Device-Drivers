#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(void)
{
    char message[32] = {0};
    int fd = open("/dev/scull0", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    ssize_t count = read(fd, message, sizeof(message) - 1);
    if (count < 0) {
        perror("read");
        close(fd);
        return 1;
    }

    printf("read %zd bytes: %.*s\n", count, (int)count, message);
    close(fd);
    return 0;
}
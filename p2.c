#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define BUFFER_SIZE 20

int main(void)
{
    int fd0 = open("/dev/scull0", O_RDONLY);
    int fd1 = open("/dev/scull1", O_WRONLY);

    if (fd0 < 0 || fd1 < 0) {
        perror("open");

        if (fd0 >= 0)
            close(fd0);

        if (fd1 >= 0)
            close(fd1);

        return 1;
    }

    char in[BUFFER_SIZE];
    char out[BUFFER_SIZE];

    unsigned int counter = 0;

    while (1) {

        /*
         * Получаем данные из первого драйвера.
         */
        ssize_t n = read(fd0, in, BUFFER_SIZE);

        if (n < 0) {
            perror("read scull0");
            break;
        }

        in[n] = '\0';

        printf("[P2] received: %s", in);

        /*
         * Генерируем свои данные.
         */
        snprintf(out,
                 sizeof(out),
                 "P2 -> message %u\n",
                 counter++);

        /*
         * Отправляем их во второй драйвер.
         */
        if (write(fd1, out, BUFFER_SIZE) < 0) {
            perror("write scull1");
            break;
        }

        usleep(100000);
    }

    close(fd0);
    close(fd1);

    return 0;
}
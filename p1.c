#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define BUFFER_SIZE 32

int main(void)
{
    int fd0 = open("/dev/scull0", O_WRONLY);
    int fd1 = open("/dev/scull1", O_RDONLY);

    if (fd0 < 0 || fd1 < 0) {
        perror("open");

        if (fd0 >= 0)
            close(fd0);

        if (fd1 >= 0)
            close(fd1);

        return 1;
    }

    char out[BUFFER_SIZE];
    char in[BUFFER_SIZE];

    unsigned int counter = 0;

    while (1) {

        /*
         * Генерируем данные.
         */
        snprintf(out,
                 sizeof(out),
                 "P1 -> message %u\n",
                 counter++);

        /*
         * Передаём их в первый драйвер.
         */
        if (write(fd0, out, BUFFER_SIZE) < 0) {
            perror("write scull0");
            break;
        }

        /*
         * Забираем данные из второго драйвера.
         *
         * Если scull1 пуст, этот процесс заблокируется
         * внутри драйвера.
         */
        ssize_t n = read(fd1, in, BUFFER_SIZE);

        if (n < 0) {
            perror("read scull1");
            break;
        }

        in[n] = '\0';

        printf("[P1] received: %s", in);

        /*
         * Небольшая задержка.
         */
        usleep(200000);
    }

    close(fd0);
    close(fd1);

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#define CAPACITY 1000

struct scull_snapshot {
    unsigned long size;
    unsigned long rp;
    unsigned long wp;
    char data[CAPACITY];
};
#define SCULL_IOC_PEEK _IOWR('k', 1, char[CAPACITY])


static void print_snapshot(const char *name,
                           struct scull_snapshot *s)
{
    printf("\n=== %s ===\n", name);

    printf("size = %lu\n", s->size);
    printf("rp   = %lu\n", s->rp);
    printf("wp   = %lu\n", s->wp);

    printf("data = ");

    /*
     * Выводим только реально занятые данные.
     *
     * Важно: это только просмотр.
     * rp самого драйвера здесь не меняется.
     */
    for (unsigned long i = 0; i < s->size; i++) {

        unsigned long pos =
            (s->rp + i) % CAPACITY;

        unsigned char c = s->data[pos];

        if (c >= 32 && c <= 126)
            putchar(c);
        else
            putchar('.');
    }

    putchar('\n');
}


int main(void)
{
    int fd0 = open("/dev/scull0", O_RDONLY);
    int fd1 = open("/dev/scull1", O_RDONLY);

    if (fd0 < 0 || fd1 < 0) {
        perror("open");

        if (fd0 >= 0)
            close(fd0);

        if (fd1 >= 0)
            close(fd1);

        return 1;
    }

    struct scull_snapshot s0;
    struct scull_snapshot s1;

    while (1) {

        /*
         * Снимаем состояние первого буфера.
         */
        if (ioctl(fd0, SCULL_IOC_PEEK, &s0) < 0) {
            perror("ioctl scull0");
            break;
        }

        /*
         * Снимаем состояние второго буфера.
         */
        if (ioctl(fd1, SCULL_IOC_PEEK, &s1) < 0) {
            perror("ioctl scull1");
            break;
        }

        /*
         * Очистить терминал.
         */
        printf("\033[H\033[J");

        print_snapshot("SCULL0", &s0);
        print_snapshot("SCULL1", &s1);

        fflush(stdout);

        /*
         * Задержка между наблюдениями.
         */
        usleep(300000);
    }

    close(fd0);
    close(fd1);

    return 0;
}
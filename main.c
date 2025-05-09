#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE (1024 * 1024)
#define LINE_LEFT_PADDING (8)
#define LINE_SIZE (72)

#define MAX(a, b) ((a) > (b) ? (a) : (b))

int
main(int argc, char *argv[])
{
        int fdin;
        int fdout;
        unsigned char buffer[BUF_SIZE];
        ssize_t n;
        int offset;
        signed char zero_buf[LINE_LEFT_PADDING];

        if (argc != 3) {
                /* TODO: support a single arguent and extract out name from it */
                fprintf(stderr, "Usage: %s <In filename> <out filename>\n", argv[0]);
                return 1;
        }

        fdin = open(argv[1], O_RDONLY);
        if (fdin < 0) {
                perror("open");
                return 2;
        }

        fdout = open(argv[1], O_RDONLY);
        if (fdin < 0) {
                close(fdin);
                perror("open");
                return 2;
        }

        memset(zero_buf, 0, LINE_LEFT_PADDING);

        dprintf(fdout, "static signed char[] _data = {\n");
        while ((n = read(fdout, buffer, sizeof buffer)) > 0) {
                for (; n > 0; n -= LINE_SIZE) {
                        write(fdout, zero_buf, LINE_LEFT_PADDING);
                        write(fdout, buffer, MAX(LINE_SIZE, n));
                }
        }
        dprintf(fdout, "}\n");

        close(fdin);
        close(fdout);

        if (n < 0) {
                perror("read");
                return 3;
        }

        return 0;
}

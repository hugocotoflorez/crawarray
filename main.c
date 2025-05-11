#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>


#define BUF_SIZE (1024 * 1024)
#define LINE_SIZE (80)
#define TAB_SIZE (8)

#define MAX(a, b) ((a) > (b) ? (a) : (b))

int
main(int argc, char *argv[])
{
        int fdin;
        int fdout;
        unsigned char buffer[BUF_SIZE];
        ssize_t n;
        int offset;

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

        fdout = open(argv[2], O_WRONLY | O_CREAT | O_APPEND, 0600);
        if (fdout < 0) {
                close(fdin);
                perror("open");
                return 3;
        }

        enum STATE {
                NORMAL,
                ON_STRING,
        } state = NORMAL;

        int counter;
        counter = TAB_SIZE;
        /* Todo: Use const char* instead of single bytes as numbers */
        dprintf(fdout, "static signed char _data[] = {\n\t");
        while ((n = read(fdin, buffer, sizeof buffer)) > 0) {
                for (offset = 0; offset < n; offset++) {
                        if (isprint(buffer[offset])) {
                                if (buffer[offset] == '"') {
                                        if (state == NORMAL) {
                                                counter += dprintf(fdout, "\"\\%c", buffer[offset]);
                                                state = ON_STRING;
                                        } else if (state == ON_STRING) {
                                                counter += dprintf(fdout, "\\%c", buffer[offset]);
                                        }
                                        continue;
                                }

                                if (buffer[offset] == '\\') {
                                        if (state == NORMAL) {
                                                counter += dprintf(fdout, "\"\\%c", buffer[offset]);
                                                state = ON_STRING;
                                        } else if (state == ON_STRING) {
                                                counter += dprintf(fdout, "\\%c", buffer[offset]);
                                        }
                                        continue;
                                }

                                if (state == NORMAL) {
                                        counter += dprintf(fdout, "\"%c", buffer[offset]);
                                        state = ON_STRING;
                                } else if (state == ON_STRING) {
                                        counter += dprintf(fdout, "%c", buffer[offset]);
                                }
                        }

                        else {
                                if (state == NORMAL) {
                                        counter += dprintf(fdout, "\"\\x%02x", buffer[offset]);
                                        state = ON_STRING;
                                } else if (state == ON_STRING) {
                                        counter += dprintf(fdout, "\\x%02x", buffer[offset]);
                                }
                        }

                        if (counter >= LINE_SIZE) {
                                if (state == NORMAL)
                                        dprintf(fdout, "\n\t");
                                else if (state == ON_STRING) {
                                        dprintf(fdout, "\"\n\t");
                                        state = NORMAL;
                                }
                                counter = TAB_SIZE; /* TODO: now it assumes tabs are 8 width */
                        }
                }
        }

        if (state == NORMAL)
                dprintf(fdout, "\n};\n");
        else if (state == ON_STRING)
                dprintf(fdout, "\"\n};\n");

        close(fdin);
        close(fdout);

        if (n < 0) {
                perror("read");
                return 4;
        }

        return 0;
}

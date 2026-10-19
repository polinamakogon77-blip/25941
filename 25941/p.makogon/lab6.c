#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>

typedef struct Line {
    long len_line;
    long offset;
} Line;

typedef struct Array {
    int len;
    int cap;
    Line *list;
} Array;

static int pushLine(Array *lines, long len_line, long offset) {
    if (lines->len == lines->cap) {
        int new_cap = lines->cap ? lines->cap * 2 : 20;
        Line *tmp = realloc(lines->list, sizeof(Line) * (size_t)new_cap);
        if (!tmp) {
            fprintf(stderr, "realloc: %s\n", strerror(errno));
            return -1;
        }
        lines->list = tmp;
        lines->cap = new_cap;
    }
    lines->list[lines->len].len_line = len_line;
    lines->list[lines->len].offset   = offset;
    lines->len++;
    return 0;
}

static void dropStdinLine(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

static void dumpFile(int f) {
    lseek(f, 0, SEEK_SET);
    char buf[4096];
    ssize_t n;
    while ((n = read(f, buf, sizeof buf)) > 0)
        write(STDOUT_FILENO, buf, (size_t)n);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("The file name has not been passed\n");
        return 1;
    }

    int f = open(argv[1], O_RDONLY);
    if (f < 0) {
        perror("open");
        return 1;
    }

    Array lines = { .len = 0, .cap = 20, .list = NULL };
    lines.list = malloc(sizeof(Line) * (size_t)lines.cap);
    if (!lines.list) {
        perror("malloc");
        close(f);
        return 1;
    }


    long line_start = 0, cur_offset = 0;
    char c;
    ssize_t r;
    while ((r = read(f, &c, 1)) > 0) {
        cur_offset++;
        if (c == '\n') {
            if (pushLine(&lines, cur_offset - line_start, line_start) < 0) {
                free(lines.list); close(f); return 1;
            }
            line_start = cur_offset;
        }
    }
    if (r < 0) {
        perror("read");
        free(lines.list); close(f); return 1;
    }
    if (line_start < cur_offset) {
        if (pushLine(&lines, cur_offset - line_start, line_start) < 0) {
            free(lines.list); close(f); return 1;
        }
    }

    printf("======    TABLE   ======\n");
    printf("номер размер смещение\n");
    for (int i = 0; i < lines.len; ++i)
        printf("  %d     %ld      %ld\n",
               i + 1, lines.list[i].len_line, lines.list[i].offset);
    printf("========================\n");
    printf("0 - завершает ввод\n");
    printf("на первый ввод дается 5 секунд\n");
    fflush(stdout);

    int first = 1;  

    for (;;) {
        int idx;

        printf("введите строку, которую нужно напечатать\n");
        fflush(stdout);

        if (first) {
            fd_set rfds;
            struct timeval tv = { .tv_sec = 5, .tv_usec = 0 };
            FD_ZERO(&rfds);
            FD_SET(STDIN_FILENO, &rfds);

            if (select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv) == 0) {
                printf("время вышло\n");
                dumpFile(f);
                free(lines.list);
                close(f);
                return 0;
            }
        }

        if (scanf("%d", &idx) != 1) {
            printf("это не число, введите индекс\n");
            dropStdinLine();
            continue;
        }

        first = 0; 

        if (idx == 0) {
            free(lines.list);
            close(f);
            return 0;
        }
        if (idx < 1 || idx > lines.len) {
            printf("такой строки нет, введите правильный индекс\n");
            continue;
        }

        Line *ln = &lines.list[idx - 1];
        lseek(f, (off_t)ln->offset, SEEK_SET);

        char *buf = malloc((size_t)ln->len_line + 1);
        if (!buf) {
            perror("malloc");
            free(lines.list);
            close(f);
            return 1;
        }

        ssize_t got = read(f, buf, (size_t)ln->len_line);
        if (got < 0) {
            perror("read");
            free(buf);
            continue;
        }
        buf[got] = '\0';
        if (got > 0 && buf[got - 1] == '\n')
            buf[got - 1] = '\0';

        printf("%s\n", buf);
        free(buf);
    }
}
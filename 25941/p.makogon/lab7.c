#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
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

    struct stat st;
    if (fstat(f, &st) < 0) {
        perror("fstat");
        close(f);
        return 1;
    }
    size_t size = (size_t)st.st_size;

    char *mapped = NULL;
    if (size > 0) {
        mapped = mmap(NULL, size, PROT_READ, MAP_PRIVATE, f, 0);
        if (mapped == MAP_FAILED) {
            perror("mmap");
            close(f);
            return 1;
        }
    }

    Array lines = { .len = 0, .cap = 20, .list = NULL };
    lines.list = malloc(sizeof(Line) * (size_t)lines.cap);
    if (!lines.list) {
        perror("malloc");
        if (mapped) munmap(mapped, size);
        close(f);
        return 1;
    }

    long line_start = 0;
    long cur_offset = 0;
    for (size_t i = 0; i < size; ++i) {
        cur_offset++;
        if (mapped[i] == '\n') {
            if (pushLine(&lines, cur_offset - line_start, line_start) < 0) {
                free(lines.list);
                munmap(mapped, size);
                close(f);
                return 1;
            }
            line_start = cur_offset;
        }
    }
    if (line_start < cur_offset) {
        if (pushLine(&lines, cur_offset - line_start, line_start) < 0) {
            free(lines.list);
            munmap(mapped, size);
            close(f);
            return 1;
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
        printf("введите строку, которую нужно напечатать\n");
        fflush(stdout);

        if (first) {
            fd_set rfds;
            struct timeval tv = { .tv_sec = 5, .tv_usec = 0 };
            FD_ZERO(&rfds);
            FD_SET(STDIN_FILENO, &rfds);

            if (select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv) == 0) {
                printf("время вышло\n");
                if (mapped) fwrite(mapped, 1, size, stdout);
                free(lines.list);
                if (mapped) munmap(mapped, size);
                close(f);
                return 0;
            }
        }
        
        first = 0;
        char line[128];
        if (!fgets(line, sizeof line, stdin)) {
            free(lines.list);
            if (mapped) munmap(mapped, size);
            close(f);
            return 0;
        }
        if (strchr(line, '\n') == NULL) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
        }

        char *end;
        errno = 0;
        long v = strtol(line, &end, 10);
        while (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')
            end++;

        if (end == line || *end != '\0' || errno == ERANGE) {
            printf("это не число, введите индекс\n");
            continue;
        }

        first = 0;

        int idx = (int)v;

        if (idx == 0) {
            free(lines.list);
            if (mapped) munmap(mapped, size);
            close(f);
            return 0;
        }
        if (idx < 1 || idx > lines.len) {
            printf("такой строки нет, введите правильный индекс\n");
            continue;
        }

        Line *ln = &lines.list[idx - 1];

        fwrite(mapped + ln->offset, 1, (size_t)ln->len_line, stdout);
        if (ln->len_line == 0 || mapped[ln->offset + ln->len_line - 1] != '\n')
            putchar('\n');
    }

    free(lines.list);
    if (mapped) munmap(mapped, size);
    close(f);
    return 0;
}
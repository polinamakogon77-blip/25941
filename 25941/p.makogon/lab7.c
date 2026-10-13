#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/mman.h>
#include <sys/stat.h>


typedef struct Line {
    int len_line;
    int offset;
}Line;

typedef struct Array {
    int len;
    int v;
    Line *list;
}Array;

void popLine(Array *lines, int len_line, int offset) {
    if (lines->len == lines->v) {
        lines->list = (Line *)realloc(lines->list, sizeof(Line) * lines->v * 2);
        lines->v *= 2;
    }


    lines->list[lines->len].len_line = len_line;
    lines->list[lines->len++].offset = offset; 
}


int main(int argc, char **argv) {
    if (argc == 1) {
        printf("The file name has not been passed\n");
        return 1;
    }

    Array lines;
    lines.len = 0;
    lines.v = 20;
    lines.list = (Line*)malloc(sizeof(Line) * lines.v);

    int f = open(argv[1], O_RDONLY);

    struct stat st;
    if (fstat(f, &st) == -1) {
        perror("fstat");
        return 1;
    }
    size_t size = st.st_size;

    char *mapped = mmap(NULL, size, PROT_READ, MAP_PRIVATE, f, 0);
    if (mapped == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    int line_start = 0;
    int cur_offset = 0;

    for (size_t i = 0; i < size; ++i) {
        char c = mapped[i];
        cur_offset += 1;

        if (c == '\n') {
            popLine(&lines, cur_offset - line_start, line_start);
            line_start = cur_offset;
        }
    }
    
    printf("======    TABLE   ======\n");
    printf("номер размер смещение\n");
    for (int i = 0; i < lines.len; ++i) {
        printf("  %d     %d      %d\n", i + 1, lines.list[i].len_line, lines.list[i].offset);
    }
    printf("========================\n");

    while (1) {
        int idx;
        
        fd_set rfds;
        struct timeval tv;
        int i = 0;

        printf("введите строку, которую нужно напечатать\n");

        while (1) {
            FD_ZERO(&rfds);
            FD_SET(STDIN_FILENO, &rfds);

            tv.tv_sec = 5;
            tv.tv_usec = 0;

            int ret = select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv);
            if (ret == 0) {
                printf("время вышло\n");
                write(STDOUT_FILENO, mapped, size);
                munmap(mapped, size);
                close(f);
                free(lines.list);
                return 0;
            }

            scanf("%d", &idx);
            if (idx == 0) {
                munmap(mapped, size);
                close(f);
                free(lines.list);
                return 0; 
            }
            
            write(STDOUT_FILENO, mapped + lines.list[idx - 1].offset, lines.list[idx - 1].len_line);

            if (mapped[lines.list[idx - 1].offset + lines.list[idx - 1].len_line - 1] != '\n') {
                write(STDERR_FILENO, "\n", 1);
            }

        }

    }
    munmap(mapped, size);
    close(f);
    free(lines.list);

    return 0;
}
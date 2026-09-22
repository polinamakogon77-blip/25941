#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>


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
    char c;
    int line_start = 0;
    int cur_offset = 0;

    while (read(f, &c, 1)) {
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

    printf("введите строку, которую нужно напечатать\n");
    int idx;
    scanf("%d", &idx);
    while (idx != 0) {
        lseek(f, sizeof(char) * lines.list[idx - 1].offset, SEEK_SET);

        char buf[lines.list[idx - 1].len_line + 1];
        read(f, buf, lines.list[idx - 1].len_line);
        buf[lines.list[idx - 1].len_line] = '\0';
        if (buf[lines.list[idx - 1].len_line - 1] == '\n') {
            buf[lines.list[idx - 1].len_line - 1] = '\0';
        }
        printf("%s\n", buf);
        printf("введите строку, которую нужно напечатать\n");
        scanf("%d", &idx);
    }

    if (idx )
    return 0;
}
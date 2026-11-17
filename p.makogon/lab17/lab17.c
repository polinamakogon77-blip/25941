#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define SIZE_LINE 40

struct termios orig_termios;

void func(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

int main(int argc, char *argv[]) {
    if ((argc > 1) && (strcmp(argv[1], "-help") == 0)) {
        printf("\t\tКаждый введенный символ будет немедленно появляться на дисплее.\n \
                Когда вводится символ ERASE, стирается последний символ в текущей строке.\n \
                Когда вводится символ KILL, стираются все символы в текущей строке.\n \
                Когда вводится CTRL-W, стирается последнее слово в текущей строке, вместе со всеми следующими за ним пробелами.\n \
                Программа завершается, когда введен CTRL-D и курсор находится в начале строки.\n \
                Все непечатаемые символы, кроме перечисленных выше, издают звуковой сигнал\n \
                Длина строки ограничена 40 символами.\n");
        return 0;
    }

    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        perror("tcgetattr");
        return 1;
    }
    atexit(func);

    cc_t erase_char  = orig_termios.c_cc[VERASE];
    cc_t werase_char = orig_termios.c_cc[VWERASE];
    cc_t kill_char   = orig_termios.c_cc[VKILL];
    cc_t eof_char    = orig_termios.c_cc[VEOF];

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr");
        return 1;
    }

    char c;
    static char line[SIZE_LINE + 1];

    while (read(STDIN_FILENO, &c, 1) == 1) {
        int len = (int)strlen(line);

        if (iscntrl((unsigned char)c) || !isprint((unsigned char)c)) {
            if (c == '\x1b') {
                char ch;
                int in_seq = 0;

                while (read(STDIN_FILENO, &ch, 1) == 1) {
                    unsigned char u = (unsigned char)ch;

                    if (u == 0x1b) { in_seq = 0; continue; }
                    if (!in_seq) {
                        if (ch == '[' || ch == 'O') { in_seq = 1; continue; }
                        break;
                    }
                    if (u >= 0x40 && u <= 0x7E) break;
                }
                continue;
            }

            if (c == (char)erase_char) {
                if (len > 0) {
                    line[len - 1] = 0;
                    printf("\33[D\33[K");
                }
            }
            else if (c == (char)werase_char) {
                if (len > 0) {
                    int word_start = 0;
                    char prev = ' ';
                    for (int i = 0; i < len; i++) {
                        if (line[i] != ' ' && prev == ' ') word_start = i;
                        prev = line[i];
                    }
                    line[word_start] = 0;
                    printf("\33[%dD\33[K", len - word_start);
                }
            }
            else if (c == (char)kill_char) {
                line[0] = 0;
                printf("\33[2K\r");
            }
            else if (c == (char)eof_char) {
                if (line[0] == 0) exit(0);
            }
            else {
                putchar('\a');
            }
        }
        else {
            if (len == SIZE_LINE) {
                putchar('\n');
                len = 0;
            }
            line[len++] = c;
            line[len] = 0;
            putchar(c);
        }

        fflush(NULL);
    }

    return 0;
}
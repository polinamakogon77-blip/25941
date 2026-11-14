#include <unistd.h>
#include <sys/termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/ttydefaults.h>
#include <string.h>


#define SIZE_LINE 40
struct termios orig_termios;


void func() {
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
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(func);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    char c;
    static char line[SIZE_LINE + 1];

    while (read(STDIN_FILENO, &c, 1) == 1) {
        int len = strlen(line);
        if (iscntrl(c) || !isprint(c)) {
            if (c == '\x1b') {
                char ch;
                int in_seq = 0;

                while (read(STDIN_FILENO, &ch, 1) == 1) {
                    unsigned char u = (unsigned char)ch;

                    if (u == 0x1b) { 
                        in_seq = 0; 
                        continue;
                    }   
                    if (!in_seq) {
                        if (ch == '[' || ch == 'O') { 
                            in_seq = 1;
                            continue;
                        }
                        break;                                 
                    }
                    if (u >= 0x40 && u <= 0x7E) break;          
                }
                continue;
            }
            
            switch (c) {
                case CERASE: {
                    line[len - 1] = 0;
                    printf("\33[D\33[K");
                    break;
                }

                case CWERASE: {
                    int word_start = 0;
                    char prev = ' ';
                    for (int i = 0; i < len; i++) {
                        if (line[i] != ' ' && prev == ' ') {
                            word_start = i;
                        }
                        prev = line[i];
                    }

                    line[word_start] = 0;
                    printf("\33[%dD\33[K", len - word_start);
                    break;
                }

                case CKILL: {
                    line[0] = 0;
                    printf("\33[2K\r");
                    break;
                }
                case CEOF: {
                    if (line[0] == 0) { exit(0); }
                    break;
                }

                default: {
                    putchar('\a');
                    break;
                }
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>


#define _POSIX_C_SOURCE 200809L


int main(int argc, char *argv[]) {
    if (argc < 2) perror("не был получен текст для записи");

    int fd[2]; // fd[0] — читать, fd[1] — писать 

    if (pipe(fd) == -1) { // создает канал
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // ребенок

        close(fd[1]);          

        char buf[256];
        ssize_t n;

        while ((n = read(fd[0], buf, sizeof(buf))) > 0) {
            for (ssize_t i = 0; i < n; i++) {
                buf[i] = (char)toupper((unsigned char)buf[i]);
            }
            if (write(STDOUT_FILENO, buf, (size_t)n) == -1) {
                perror("write");
                break;
            }
        }

        if (n == -1) perror("read");

        close(fd[0]);
        _exit(0);           
    }

   
    // родитель пишет текст в канал
    close(fd[0]);             

    const char *text = argv[1];

    ssize_t total = (ssize_t)strlen(text);
    ssize_t written = 0;

    while (written < total) {
        ssize_t w = write(fd[1], text + written, (size_t)(total - written));
        if (w == -1) {
            perror("write");
            break;
        }
        written += w;
    }

    if (write(fd[1], "\n", 1) == -1) perror("write");
    close(fd[1]);              

    waitpid(pid, NULL, 0);    

    return 0;
}
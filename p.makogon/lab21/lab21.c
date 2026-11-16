#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>


#define _POSIX_C_SOURCE 200809L

static volatile int sigint_count = 0;
static volatile int quit_flag   = 0;


static void on_sigint(int sig) {
    (void)sig;
    sigint_count++;       
    write(STDOUT_FILENO, "\a", 1);  
}

static void on_sigquit(int sig) {
    (void)sig;
    quit_flag = 1;           
}

int main(void) {
    struct sigaction sa;

    sa.sa_handler = on_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    sa.sa_handler = on_sigquit;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGQUIT, &sa, NULL);

    while (!quit_flag) {
        pause(); // прерывается любым сигналом (EINTR)
    }

    printf("сигнал прозвучал %d раз\n", (int)sigint_count);
    return 0;
}
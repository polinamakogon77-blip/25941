#include <stdio.h>
#include <unistd.h>


void printUID() {
    printf("uid = %d; euid = %d\n", getuid(), geteuid());
}

void tryOpenFile(char *name) {
    FILE *file = fopen(name, "r");

    if (file == NULL) perror("Failed to open the file");
    else fclose(file);
}

int main(int argc, char **argv) {
    printUID();
    if (argc == 1) {
        printf("The file name has not been passed\n");
        return 1;
    }
    tryOpenFile(argv[1]);

    setuid(getuid());

    printUID();
    tryOpenFile(argv[1]);
    
    return 0;
}
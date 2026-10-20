#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>

// ssh username@ccfit.nsu.ru
// 00V1Qcey,Oe
// ssh 10.4.0.68

extern char **environ;

long checkArgument(char *arg) {
    char *endptr;
    long val = strtol(arg, &endptr, 10);

    if (endptr == arg || val < 0) {
       printf("the argument is invalid\n");
       return -1; 
    }

    return val;
} 


int main(int argc, char *argv[]) {
    char options[] = "ispuU:cC:dvV:";
    int opt;

    if (argc == 1) {
        printf(
        "Использование:\n"
        "Опции:\n"
        "  -i             показать реальные и эффективные UID/GID\n"
        "  -s             сделать процесс лидером новой группы процессов\n"
        "  -p             показать PID, PPID и PGID\n"
        "  -u             показать значение ulimit\n"
        "  -U <bytes>     изменить значение ulimit\n"
        "  -c             показать текущий размер core-файла\n"
        "  -C <bytes>     изменить размер core-файла\n"
        "  -d             показать текущий рабочий каталог\n"
        "  -v             вывести все переменные окружения\n"
        "  -V name=value  установить переменную окружения\n"
        "  -h             показать эту справку\n"
        "\n");
    }

    while ((opt=getopt(argc, argv, options)) != EOF) {
        switch (opt)
        {
        case 'i': 
        {
            printf("real user_id = %d; effective user_id = %d\n", getuid(), geteuid());
            printf("real group_id = %d; effective group_id = %d\n", getgid(), getegid());
            break;
        }
        case 's': 
        {
            if (setpgid(0, 0) != -1) printf("the process became the group leader\n");
            else printf("it wasn't possible to make process the group leader\n");
            break;
        }
        case 'p': 
        {
            printf("process_id = %d\nparent_process_id = %d\ngroup_process_id = %d\n", getpid(), getppid(), getpgrp());
            break;
        }
        case 'u':
        {
            FILE *f = popen("ulimit -u", "r");
            if (f) {
                char buf[64];
                if (fgets(buf, sizeof(buf), f))
                    printf("current limit = %s", buf);
                pclose(f);
            } else {
                perror("popen");
            }
            break;
        }
        case 'U':
        {
            long new_limit = checkArgument(optarg);
            if (new_limit < 0) break;

            printf("нельзя менять максимальное к-во потоков для пользователя\n");
            break;
        }
        case 'c':
        {
            struct rlimit limit;
            getrlimit(RLIMIT_CORE, &limit);
            printf("core file size = %ld\n", (long)limit.rlim_cur);
            break;
        }
        case 'C':
        {
            struct rlimit limit;
            getrlimit(RLIMIT_CORE, &limit);
            long new_size = checkArgument(optarg);
            limit.rlim_cur = new_size;

            if (new_size >= 0 && setrlimit(RLIMIT_CORE, &limit) != -1) printf("new size = %ld\n", new_size);
            else printf("it wasn't possible to change the size core file\n");
            break;
        }
        case 'd': {
            char cur_directory[256];
            getcwd(cur_directory, sizeof(cur_directory));
            printf("current directory - %s\n", cur_directory);
            break;
        }
        case 'v':
        {
            char **variables; // переменные среды
            for (variables = environ; *variables != NULL; variables++) {
                printf("%s\n", *variables);
            }
            break;
        }
        case 'V': 
        {
            if (putenv(optarg) == 0) printf("environment variable: %s\n", optarg);
            else printf("it wasn't possible to introduces a new variable\n");
            break;
        }
        case ':':
        {
            printf("missing argument\n");
            break;
        }
        default:
            printf("invalid option %d", opt);
        }
    }

    return 0;
}


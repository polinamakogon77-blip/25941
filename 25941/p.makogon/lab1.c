#include <rctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
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
            rctlblk_t *blk = malloc(rctlblk_size());
            if (!blk) { perror("malloc"); break; }

            /* Субъект — текущий процесс: getrctl с NULL-субъектом берёт текущий */
            if (getrctl("process.max-procs", NULL, blk, RCTL_FIRST) == 0) {
                printf("current limit = %llu; max limit = %llu\n",
                    (unsigned long long)rctlblk_get_value(blk),
                    (unsigned long long)rctlblk_get_enforced_value(blk));
            } else {
                perror("getrctl");
            }
            free(blk);
            break;
        }
        case 'U':
        {
            long new_limit = checkArgument(optarg);
            if (new_limit < 0) break;

            rctlblk_t *blk = malloc(rctlblk_size());
            if (!blk) { perror("malloc"); break; }

            if (getrctl("process.max-procs", NULL, blk, RCTL_FIRST) == 0) {
                rctlblk_set_value(blk, (rctl_qty_t)new_limit);
                if (setrctl("process.max-procs", NULL, blk, RCTL_REPLACE) == 0)
                    printf("new limit = %ld\n", new_limit);
                else
                    perror("setrctl");
            } else {
                perror("getrctl");
            }
            free(blk);
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


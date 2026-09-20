#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>


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
            struct rlimit limit;
            getrlimit(RLIMIT_FSIZE, &limit);
            printf("curent limit = %ld; max limit = %ld\n", (long)limit.rlim_cur, (long)limit.rlim_max);
            break;
        }
        case 'U':
        {
            struct rlimit limit;
            getrlimit(RLIMIT_FSIZE, &limit);
            long new_limit = checkArgument(optarg);
            limit.rlim_cur = new_limit;

            if (new_limit >= 0 && setrlimit(RLIMIT_FSIZE, &limit) == 0) printf("new limit = %ld", new_limit);
            else printf("it wasn't possible to change the limit");
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
            char **variables;
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
        case '?':
        default:
            printf("invalid option %d", opt);
        }
    }

    return 0;
}


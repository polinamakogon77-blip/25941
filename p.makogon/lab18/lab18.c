#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <locale.h>
#include <dirent.h>
#include <errno.h>



static char type_char(mode_t mode) {
    if (S_ISDIR(mode))  return 'd';
    if (S_ISREG(mode))  return '-';
    return '?';
}

static void perm_triplet(char *buf, mode_t mode, mode_t r, mode_t w, mode_t x) {
    buf[0] = (mode & r) ? 'r' : '-';
    buf[1] = (mode & w) ? 'w' : '-';
    buf[2] = (mode & x) ? 'x' : '-';
}


static void print_entry(const char *path, const char *name,
                        const struct stat *st) {
    char perms[11];
    perms[0] = type_char(st->st_mode);
    perm_triplet(perms + 1, st->st_mode, S_IRUSR, S_IWUSR, S_IXUSR);
    perm_triplet(perms + 4, st->st_mode, S_IRGRP, S_IWGRP, S_IXGRP);
    perm_triplet(perms + 7, st->st_mode, S_IROTH, S_IWOTH, S_IXOTH);
    perms[10] = '\0';

    struct passwd *pw = getpwuid(st->st_uid);
    struct group  *gr = getgrgid(st->st_gid);
    const char *user  = pw ? pw->pw_name : "?";
    const char *group = gr ? gr->gr_name : "?";

    char sizebuf[32] = "";
    if (S_ISREG(st->st_mode)) {
        snprintf(sizebuf, sizeof(sizebuf), "%lld", (long long)st->st_size);
    }

    char timebuf[64];
    struct tm *tm = localtime(&st->st_mtime);
    if (tm) strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", tm);
    else strcpy(timebuf, "?");

    (void)path;   

    printf("%-10s %3lu %-8s %-8s %8s %s %s\n",
           perms,
           (unsigned long)st->st_nlink,
           user,
           group,
           sizebuf,
           timebuf,
           name);
}



static int cmp_names(const void *a, const void *b) {
    const char *const *pa = a;
    const char *const *pb = b;
    return strcoll(*pa, *pb); 
}


static int list_dir(const char *dirpath, int show_all) {
    DIR *d = opendir(dirpath);
    if (!d) {
        perror(dirpath);
        return -1;
    }

    
    char **names = NULL;
    size_t count = 0, cap = 0;

    struct dirent *de;
    while ((de = readdir(d)) != NULL) {
        if (!show_all && de->d_name[0] == '.')
            continue;                       

        if (count == cap) {
            size_t newcap = cap ? cap * 2 : 32;
            char **tmp = realloc(names, newcap * sizeof(*names));
            if (!tmp) { perror("realloc"); break; }
            names = tmp;
            cap = newcap;
        }
        names[count] = strdup(de->d_name);
        if (!names[count]) { perror("strdup"); break; }
        count++;
    }
    closedir(d);

   
    qsort(names, count, sizeof(*names), cmp_names);


    for (size_t i = 0; i < count; i++) {
        char fullpath[4096];
        if (snprintf(fullpath, sizeof(fullpath), "%s/%s", dirpath, names[i])
                >= (int)sizeof(fullpath)) {
            fprintf(stderr, "%s/%s: path too long\n", dirpath, names[i]);
            free(names[i]);
            continue;
        }

        struct stat st;
        if (lstat(fullpath, &st) == -1) {
            perror(fullpath);
            free(names[i]);
            continue;
        }

        print_entry(fullpath, names[i], &st);
        free(names[i]);
    }

    free(names);
    return 0;
}


int main(int argc, char *argv[]) {
    int show_all = 1;   


    int argi = 1;
    for (; argi < argc; argi++) {
        const char *a = argv[argi];
        if (a[0] != '-' || a[1] == '\0') break;   
        if (strcmp(a, "--") == 0) { argi++; break; }

        if (strcmp(a, "--all") == 0)  { show_all = 1; continue; }
        if (strcmp(a, "--long") == 0) { continue; } 

        for (const char *p = a + 1; *p; p++) {
            switch (*p) {
                case 'a': show_all = 1; break;
                case 'l': break;
                case 'd': break;
                default:
                    fprintf(stderr, "unknown option: -%c\n", *p);
                    return 1;
            }
        }
    }

    setlocale(LC_TIME, "");
    setlocale(LC_COLLATE, "");  

    if (argi >= argc) {
        static char *dot[] = { ".", NULL };
        argv = dot;
        argc = 1;
        argi = 0;
    }

    int multiple = (argc - argi) > 1;
    int had_error = 0;

    for (int i = argi; i < argc; i++) {
        const char *path = argv[i];
        struct stat st;

        if (lstat(path, &st) == -1) {
            perror(path);
            had_error = 1;
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            if (multiple)
                printf("%s:\n", path);      

            if (list_dir(path, show_all) == -1)
                had_error = 1;

            if (multiple && i != argc - 1)
                putchar('\n');
        }
        else {
          
            const char *name = strrchr(path, '/');
            name = name ? name + 1 : path;
            print_entry(path, name, &st);
        }
    }

    return had_error ? 1 : 0;
}
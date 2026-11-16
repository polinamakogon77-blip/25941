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

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "не указаны файлы\n", argv[0]);
        return 1;
    }

    setlocale(LC_TIME, ""); // включает локаль пользователя  

    for (int i = 1; i < argc; ++i) {
        const char *path = argv[i];
        struct stat st;

        // структура stat не была заполнена информацией о файле
        if (lstat(path, &st) == -1) { 
            perror(path);
            continue;
        }

        // строим строку прав 
        char perms[11];
        perms[0] = type_char(st.st_mode);
        perm_triplet(perms + 1, st.st_mode, S_IRUSR, S_IWUSR, S_IXUSR);
        perm_triplet(perms + 4, st.st_mode, S_IRGRP, S_IWGRP, S_IXGRP);
        perm_triplet(perms + 7, st.st_mode, S_IROTH, S_IWOTH, S_IXOTH);
        perms[10] = '\0';

        // ииена пользователя и группы
        struct passwd *pw = getpwuid(st.st_uid);
        struct group  *gr = getgrgid(st.st_gid);
        const char *user  = pw ? pw->pw_name : "?";
        const char *group = gr ? gr->gr_name : "?";

        // размер обычного файла
        char sizebuf[32] = "";
        if (S_ISREG(st.st_mode)) {
            snprintf(sizebuf, sizeof(sizebuf), "%lld", (long long)st.st_size);
        }

        // время
        char timebuf[64];
        struct tm *tm = localtime(&st.st_mtime);
        if (tm) strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", tm);
        else strcpy(timebuf, "?");

        const char *name = strrchr(path, '/');
        name = name ? name + 1 : path;

        // имя без пути
        if (*name == '\0' && path != name) {
            const char *end = name - 1;    
            while (end > path && *(end - 1) != '/') end--;
            static char tmp[4096];
            size_t n = (size_t)(name - 1 - end);
            if (n >= sizeof(tmp)) n = sizeof(tmp) - 1;
            memcpy(tmp, end, n);
            tmp[n] = '\0';
            name = tmp;
        }

        printf("%-10s %3lu %-8s %-8s %8s %s %s\n",
               perms,
               (unsigned long)st.st_nlink,
               user,
               group,
               sizebuf,
               timebuf,
               name);
    }

    return 0;
}
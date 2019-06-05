/*
 * vim: et ts=4 sw=4
 */

#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <dirent.h> // scandir
#include <sys/types.h>
#include <sys/stat.h>
#include <err.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>

#define CRASH() err(1, "line %d in %s", __LINE__, __FILE__);

char *copy_command = "cp";

char *src_path;
int src_path_len;

int compar(const struct dirent **a, const struct dirent **b) {
    struct stat a_stat;
    struct stat b_stat;

    strncpy(src_path + src_path_len, (*a)->d_name, 256);
    if (stat(src_path, &a_stat)) {
        CRASH();
    }

    strncpy(src_path + src_path_len, (*b)->d_name, 256);
    if (stat(src_path, &b_stat)) {
        CRASH();
    }
    return a_stat.st_mtime < b_stat.st_mtime;
}

int filter(const struct dirent *a) {
    if (strcmp(a->d_name, ".") && strcmp(a->d_name, "..")) {
        return 1;
    }
    return 0;
}

void exec(char *const args[]) { // TODO: probably want to use a macro
    pid_t pid;
    if ((pid = fork()) == 0) {
        execvp(args[0], args);
    }
    wait(NULL);
}

bool is_dir(const char *path) { // TODO: probably want to use a macro
    struct stat path_stat;
    if (stat(path, &path_stat)) {
        CRASH();
    }
    return S_ISDIR(path_stat.st_mode);

}

bool dir_exists(char *path) { // TODO: probably want to use a macro
    /*
     * There are about a hundred different ways to check for this e.g. using access(), stat(), opendir(), mkdir()
     * TODO: decide best way. (benchmark would be nice)
     */
    struct stat path_stat;
    if (stat(path, &path_stat)) {
        return false;
    }
    return true;
}

#define FILENAME_LEN 256 // TODO: make it take into account more FSs

char *expand_dir(char *dir_specifier) {
    /*
     * Return Value
     * 
     * return a pointer to the path. It is allocated using malloc(3) and can be freed with free(3).
     */
    char *CDPATH = getenv("CDPATH");
    if (CDPATH) {
        CDPATH = strdup(CDPATH); // free(3)
        char *path = malloc(strlen(CDPATH) + strlen(dir_specifier) + FILENAME_LEN); // TODO: uselessly big malloc()
        char *current = strtok(CDPATH, ":");
        do {
            char *separator = stpcpy(path, current);
            if (separator[-1] != '/') {
                *separator++ = '/';
            }
            strcpy(separator, dir_specifier);
            if (dir_exists(path)) {
                return path;
            }
        } while (current = strtok(NULL, ":"));
    }
    char *dir = malloc(strlen(dir_specifier) + FILENAME_LEN); // TODO: do we really need to duplicate this string? Would it be useful/possible to return an absolute path.
    strcpy(dir, dir_specifier);
    return dir;
}

void copy_main(int argc, char *argv[]) {
    int modification = 1;

    int opt;
    const char optstring[] = "t:";
    const struct option longopts[] = {
        {"modification", optional_argument, NULL, 't'},
        {0, 0, 0, 0},
    };

    while ((opt = getopt_long(argc, argv, optstring, longopts, NULL)) != -1) {
        switch(opt) {
        case 't':
            if (optarg) {
                modification = atoi(optarg);
                break;
            }
            break;
        case '?':
            puts(optarg);
            break;
        }
    }

    if (optind >= argc) {
        errx(1, "Wrong number of arguments given!!");
    }

    struct dirent **src_files;
    int src_files_len;
    src_path = expand_dir(argv[optind++]); // TODO has to be free(3)
    src_path_len = strlen(src_path);
    char *dst_path = optind < argc ? expand_dir(argv[optind++]) : strdup(".");

    if (is_dir(src_path)) {
        if ((src_files_len = scandir(src_path, &src_files, filter, compar)) == -1) { // TODO free namelist
            CRASH();
        }
        if (modification >= 0) {
            for (int i = 0; i < (modification < src_files_len ? modification : src_files_len); ++i) {
                src_path[src_path_len] = '/';
                strncpy(src_path + src_path_len + 1, src_files[i]->d_name, 256);
                char *args[] = {copy_command, src_path, dst_path, 0}; // TODO: for the moment using cp and mv seem to be the best
                exec(args);
            }
        }
    } else {
        char *args[] = {copy_command, src_path, dst_path, 0};
        exec(args);
    }

}

void move_main(int argc, char *argv[]) {
    copy_command = "mv";
    copy_main(argc, argv);
}

void list_main(int argc, char *argv[]) {
    char *path = expand_dir(argv[optind++]);
    char *args[] = {"ls", path, 0};
    exec(args);
    free(path);
}

int main(int argc, char *argv[]) {
    char *command = argv[optind++];
    if (!strcmp(command, "cp")) {
        copy_main(argc, argv);
    } else if (!strcmp(command, "mv")) {
        move_main(argc, argv);
    } else if (!strcmp(command, "ls")) {
        list_main(argc, argv);
    }
}

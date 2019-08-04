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
char **args;

char *src_path;
int src_path_len;

int compar(const struct dirent **a, const struct dirent **b) {
    struct stat a_stat;
    struct stat b_stat;

    strncpy(src_path + src_path_len + 1, (*a)->d_name, 256);
    if (stat(src_path, &a_stat)) {
        CRASH();
    }

    strncpy(src_path + src_path_len + 1, (*b)->d_name, 256);
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

void exec(char *const exec_args[]) { // TODO: probably want to use a macro
    pid_t pid;
    if ((pid = fork()) == 0) {
        execvp(exec_args[0], exec_args);
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
    char *path;
    if (CDPATH) {
        CDPATH = strdup(CDPATH);
        path = malloc(strlen(CDPATH) + strlen(dir_specifier) + FILENAME_LEN); // TODO: uselessly big malloc()
        char *current = strtok(CDPATH, ":");
        do {
            char *separator = stpcpy(path, current);
            if (separator[-1] != '/') {
                *separator++ = '/';
            }
            strcpy(separator, dir_specifier);
            if (dir_exists(path)) {
                free(CDPATH);
                return path;
            }
        } while ((current = strtok(NULL, ":")));
        free(path);
        free(CDPATH);
    }
    path = malloc(strlen(dir_specifier) + FILENAME_LEN); // TODO: do we really need to duplicate this string? Would it be useful/possible to return an absolute path.
    strcpy(path, dir_specifier);
    return path;
}

void copy_main(int argc, char *argv[]) {
    int modification = 1;

    int opt, opt_count = 1;
    const char optstring[] = "cmt:";
    const struct option longopts[] = {
        {"modification", optional_argument, NULL, 't'},
        {"copy", no_argument, NULL, 'c'},
        {"move", no_argument, NULL, 'm'},
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
        case 'm':
            copy_command = "mv";
            break;
        case 'c':
            copy_command = "cp";
            break;
        case '?': {
                  args[opt_count++] = argv[optind - 1]; // TODO: deal with external command args
              } break;
        default: // TODO: Check if this is possible
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

    args[0] = copy_command;
    if (is_dir(src_path)) {
        src_path[src_path_len] = '/';
        if ((src_files_len = scandir(src_path, &src_files, filter, compar)) == -1) { // TODO free namelist
            CRASH();
        }
        if (modification >= 0) {
            for (int i = 0; i < (modification < src_files_len ? modification : src_files_len); ++i) {
                strncpy(src_path + src_path_len + 1, src_files[i]->d_name, 256);
                args[opt_count] = src_path;
                args[opt_count + 1] = dst_path;
                args[opt_count + 2] = 0;
                exec(args);
            }
        }
    } else {
        args[opt_count++] = src_path; // TODO: this is already done for the other side of the if statement. Should maybe be put inside a function
        args[opt_count++] = dst_path;
        args[opt_count++] = 0;
        exec(args);
    }

}

void move_main(int argc, char *argv[]) {
    copy_command = "mv";
    copy_main(argc, argv);
}

void list_main(int argc, char *argv[]) {
    int opt, opt_count = 1;
    const char optstring[] = "";
    const struct option longopts[] = {
        {0, 0, 0, 0},
    };

    while ((opt = getopt_long(argc, argv, optstring, longopts, NULL)) != -1) { // deal with unknown long commands
        switch(opt) {
        case '?': {
                  args[opt_count++] = argv[optind - 1]; // TODO: deal with external command args
              } break;
        default: // TODO: Check if this is possible
            break;
        }
    }
    args[0] = "ls";
    args[opt_count++] = optind < argc ? expand_dir(argv[optind++]) : strdup("."); // TODO: free this
    args[opt_count++] = 0;
    exec(args);
}

int main(int argc, char *argv[]) {
    opterr = 0;
    if (argc <= 1) {
        printf("No command given\n");
        return 1;
    }
    char *command = argv[optind++];
    args = malloc(argc * sizeof(char*)); // TODO: allocate less memory than needed
    if (!strcmp(command, "cp") || !strcmp(command, "copy")) {
        copy_main(argc, argv);
    } else if (!strcmp(command, "mv") || !strcmp(command, "move")) {
        move_main(argc, argv);
    } else if (!strcmp(command, "ls") || !strcmp(command, "list")) {
        list_main(argc, argv);
    } else { // Command is unknown
        printf("Unknown command %s\n", command);
        return 1;
    }
    return 0;
}

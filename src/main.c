/*
 * vim: et ts=4 sw=4
 */

#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <dirent.h> // scandir
#include <sys/types.h>
#include <sys/stat.h>
#include <err.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>

#include "file_manipulation.h"
#include "vector.h"

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

/*
 * Same as strdup, except that it takes as parameter the capacity of the string (STRLEN(3) + 1)
 */
char *strdup_with_capacity(const char *str, const size_t capacity) {
    char *new;
    if ((new = malloc(capacity)) == NULL) {
        return new;
    }
    return memcpy(new, str, capacity);
}

struct cdpath {
    char *path;
    // suffix_ptr points to the end of the path, where there should be a '\0'
    // when it doesn't have a base.

    // prefix_length holds the length of the prefix. It shouldn't change.
    size_t prefix_length;
    size_t capacity;
};

void cdpath_init(struct cdpath *cdpath, const char *initial_path) {
    cdpath->prefix_length = strlen(initial_path);
    cdpath->capacity = cdpath->prefix_length + 1;
    cdpath->path = malloc(cdpath->capacity);
    memcpy(cdpath->path, initial_path, cdpath->capacity);
}

/*
 * @param: suffix_length must hold the value of strlen(suffix). It is passed to
 * prevent it from being computed multiple times.
 */
void cdpath_set_suffix(struct cdpath *cdpath, const char *suffix, const size_t suffix_capacity) {
    if (cdpath->capacity < cdpath->prefix_length + suffix_capacity) {
        cdpath->capacity = cdpath->prefix_length + suffix_capacity;
        cdpath->path = realloc(cdpath->path, cdpath->capacity);
    }
    memcpy(cdpath + cdpath->prefix_length, suffix, suffix_capacity);
}

bool cdpath_suffix_exists(struct cdpath *cdpath, const char *suffix, const size_t suffix_capacity) {
    cdpath_set_suffix(cdpath, suffix, suffix_capacity);
    return file_or_directory_exists(cdpath->path);
}

bool cdpath_suffix_is_directory(struct cdpath *cdpath, const char *suffix, const size_t suffix_capacity) {
    cdpath_set_suffix(cdpath, suffix, suffix_capacity);
    return is_directory_and_exists(cdpath->path);
}

void expand_dir(struct vector *args, struct vector *cd_paths, const char *suffix) {
    if (file_or_directory_exists(dir_specifier) == true) {
        vector_push(args, dir_specifier);
        return;
    }

    size_t suffix_length = strlen(dir_specifier);
    size_t suffix_capacity = suffix_length + 1;

    for (struct cdpath **paths = (struct cdpath **) vector_vector(cd_paths); paths != NULL; ++paths) {
        if (cdpath_suffix_exists(*paths, suffix, suffix_capacity) == true) {
            vector_push(args, strdup_with_capacity((*paths)->path, suffix_capacity));
            return;
        }
    }

    char *separator;
    if (separator = memrchr(suffix, ':', suffix_length) == NULL) {
        // TODO: no file or directory
        return;
    }

    *separator = '\0';
    if (directory_exists(suffix) == true) {

    }
    
    for (struct cdpath **paths = (struct cdpath **) vector_vector(cd_paths); paths != NULL; ++paths) {

    }
}

void print_help() {
    puts("cli-fm\n"
         "\t-h, --help: show this help message\n"
         "\t-v, --verbose: print commands ran\n"
         "https://github.com/NilsIrl/cli-fm");
}

enum command {
    NO_COMMAND,
    COPY,
    MOVE,
    LIST,
};

/*
 * ARCHITECTURE TODO: Find the longest suffix first, so that no reallocations are needed for cdpath-s.
 * TODO: decide on default behaviour and options for symbolic links and directories.
 */

int main(int argc, char *argv[]) {
    // TODO: explain why it is set to 0
#if 0
    opterr = 0;
#endif

    int opt;
    // The number of implicit paths cli-fm should add.
    // For example, with `ls`, it is set to 1
    // If `ls` is run without any path, another path is added, to achieve 1 path.
    // For `mv` and `cp`, if 1 path is given, another path is added to achieve 2 path.
    int32_t number_of_required_path = -1;
    while ((opt = getopt_long(argc, argv, "+hp:v",
                    (struct option[]) {
                        { "help", no_argument, NULL, 'h' },
                        { "path", required_argument, NULL, 'p' },
                        { "verbose", no_argument, NULL, 'v'},
                        { 0, 0, 0, 0 }
                    }, NULL)) != -1) {
        switch (opt) {
        case 'h':
            print_help();
            exit(EXIT_SUCCESS);
            break;
        case 'p':
            number_of_required_path = atoi(optarg);
            break;
        case 'v':
            // TODO: verbose
            break;
        }
    }

    struct vector cd_paths;
    vector_init(&cd_paths);

    char *CDPATH = strdupa(getenv("CDPATH"));
    char *token = strtok(CDPATH, ":");
    while (token != NULL) {
        struct cdpath *cdpath = alloca(sizeof(struct cdpath));
        cdpath_init(cdpath, token);
        vector_push(&cd_paths, cdpath);
        token = strtok(NULL, ":");
    }

    struct vector args;
    vector_init_with_capacity(&args, argc);

    // TODO: use argv[0] for the command
    // TODO: if (optind == argc) { here_we_go_again(); }
    char *command = argv[optind++];
    vector_push(&args, command);

    // If the user didn't override number_of_required_path
    if (number_of_required_path == -1) {
        if (!strcmp(command, "ls")) {
            number_of_required_path = 1;
        } else if (!strcmp(command, "cp") || !strcmp(command, "mv")) {
            number_of_required_path = 2;
        } else {
            number_of_required_path = 0;
        }
    }

    for (; optind < argc; ++optind) {
        if (argv[optind][0] != '-') {
            expand_dir(&args, &cd_paths, argv[optind]);
        } else {
            vector_push(&args, argv[optind]);
        }
    }

    return 0;
}

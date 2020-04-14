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
#include <fcntl.h>
#include <err.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>

#include "file_manipulation.h"
#include "vector.h"

vector(int, int);
vector(char *, str);
vector(struct cdpath, cdpath);

#define CRASH() err(1, "line %d in %s", __LINE__, __FILE__);

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

void expand_dir(struct vector_str *args, struct vector_int *cd_paths, const char *suffix) {
    size_t suffix_capacity = strlen(suffix) + 1;

    for (size_t i = 0; i < cd_paths->length; ++i) {
        if (file_or_directory_exists_at(cd_paths->vector[i], suffix) == true) {
            vector_push(args, strdup_with_capacity((*paths)->path, suffix_capacity));
            return;
        }
    }

    char *separator;
    if (separator = memrchr(suffix, ':', suffix_capacity - 1) == NULL) {
        // TODO: no file or directory
        return;
    }

    *separator = '\0';
    suffix_capacity = suffix - separator + 1;
    if (directory_exists(suffix) == true) {

    }
    
    for (struct cdpath **paths = (struct cdpath **) vector_vector(cd_paths); paths != NULL; ++paths) {
        if ((cdpath_suffix_is_directory(*paths, suffix, suffix_capacity)) == true) {

        }
    }
}

void print_help() {
    puts("cli-fm\n"
         "\t-h, --help: show this help message\n"
         "\t-v, --verbose: print commands ran\n"
         "https://github.com/NilsIrl/cli-fm");
}

struct cdpath {
    char path[PATH_MAX];
    char *suffix;
}

/*
 * ARCHITECTURE TODO: Find the longest suffix first, so that no reallocations are needed for cdpath-s.
 * TODO: decide on default behaviour and options for symbolic links and directories.
 */

int main(int argc, char *argv[]) {
    // TODO: explain why it is set to 0
    opterr = 0;

    int opt;
    // The number of implicit paths cli-fm should add.
    // For example, with `ls`, it is set to 1
    // If `ls` is run without any path, another path is added, to achieve 1 path.
    // For `mv` and `cp`, if 1 path is given, another path is added to achieve 2 path.
    int32_t number_of_required_path = -1;

    // TODO: do we really want the "+". When do we print the help then?
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

    struct vector_cdpath cdpaths;
    vector_init(&cd_paths);

    // TODO: compare this with open(".")
    vector_push(&cd_paths, AT_FDCWD);

    // TODO: look carefully at strtok to see if it is possible to not strdup it.
    const char *CDPATH = getenv("CDPATH");
    const char *subp;
    for (const char *CDPATH = getenv("CDPATH"); ; CDPATH = subp)
    while (CDPATH != NULL) {
        subp = strchrnul(CDPATH, ':');
        if (subp - CDPATH > MAX_PATH) {
            continue;
        }
        struct cdpath *cdpath = alloca(sizeof(struct cdpath));
        char *separator = mempcpy(cdpath->path, CDPATH, subp - CDPATH);
        cdpath->suffix = *separator++ = '/';
    }
    char *token = strtok(CDPATH, ":");
    while (token != NULL) {
        // TODO: O_CLOEXEC may be interesting to close the file descriptors when executing the command
        int fd = open(token, O_RDONLY | O_DIRECTORY | O_PATH);
        if (fd == -1) {
            CRASH();
        }
        vector_push(&cd_paths, fd);
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

/*
 * vim: et ts=4 sw=4
 */

#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "cdpath.h"
#include "file_manipulation.h"
#include "vector.h"

#define CRASH() err(1, "line %d in %s", __LINE__, __FILE__);

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

void expand_dir(struct vector *args, struct vector *cdpaths, char *suffix) {
    size_t suffix_capacity = strlen(suffix) + 1;

    if (file_exists(suffix) == true) {
        vector_push(args, suffix);
        return;
    }

    for (size_t i = 0; i < cdpaths->length; ++i) {
        struct cdpath *cdpath = cdpaths->vector[i];
        if (file_exists_at_path(cdpath, suffix, suffix_capacity) == true) {
            vector_push(args, strdup_with_capacity(cdpath->path, cdpath->suffix - cdpath->path + suffix_capacity));
            return;
        }
    }

#if 0
    char *separator;
    if (separator = memrchr(suffix, ':', suffix_capacity - 1) == NULL) {
        // TODO: no file or directory
        return;
    }

    *separator = '\0';
    suffix_capacity = suffix - separator + 1;
    if (directory_exists(suffix) == true) {

    }

    for (struct cdpath **paths = (struct cdpath **) vector_vector(cdpaths); paths != NULL; ++paths) {
        if ((cdpath_suffix_is_directory(*paths, suffix, suffix_capacity)) == true) {

        }
    }
#endif

    vector_push(args, suffix);
}

void print_help() {
    puts("cli-fm\n"
            "\t-h, --help: show this help message\n"
            "\t-v, --verbose: print commands ran\n"
            "https://github.com/NilsIrl/cli-fm");
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

    struct vector cdpaths;
    vector_init(&cdpaths);

    const char *CDPATH = getenv("CDPATH");
    if (CDPATH != NULL) {
        const char *subp;
        for (;; CDPATH = subp) {
            subp = strchrnul(CDPATH, ':');
            if (subp - CDPATH > PATH_MAX) {
                if (*subp == '\0')
                    break;
                continue;
            }
            struct cdpath *cdpath = alloca(sizeof(*cdpath));
            cdpath->suffix = mempcpy(cdpath->path, CDPATH, subp - CDPATH);
            *cdpath->suffix++ = '/';
            vector_push(&cdpaths, cdpath);
            if (*subp++ == '\0')
                break;
        }
    }

    struct vector args;
    vector_init_with_capacity(&args, argc);

    // TODO: use argv[0] for the command
    // TODO: if (optind == argc) { here_we_go_again(); }
    if (optind == argc) {
        fputs("No command given\n", stderr);
        exit(2);
    }

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
        // If argument is a path not an option (i.e. doesn't start with a '-')
        if (argv[optind][0] != '-') {
            expand_dir(&args, &cdpaths, argv[optind]);
        } else {
            vector_push(&args, argv[optind]);
        }
    }

    vector_push(&args, NULL);
    execvp(command, (char **) args.vector);

    // execvp returned indicating an error
    fputs("execvp failed.\n", stderr);

    return 1;
}

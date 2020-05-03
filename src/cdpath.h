#pragma once

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>

struct cdpath {
    char path[PATH_MAX];
    char *suffix;
};

bool file_exists_at_path(struct cdpath *cdpath, const char *suffix, const size_t suffix_len);

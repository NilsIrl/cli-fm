#pragma once

#include <stdbool.h>

bool file_or_directory_exists_at(const int dirfd, const char *pathname);
bool is_directory_and_exists(const int dirfd, const char *pathname);


#include "file_manipulation.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

bool path_exists(const char *path, struct stat *path_stat) {
	if (stat(path, path_stat) != 0) {
		if (errno == ENOENT) {
			return false;
		} else {
			fprintf(stderr, "stat failed in dir_exists.\n");
			exit(EXIT_FAILURE);
		}
	}
	return true;
}

bool file_or_directory_exists(const char *path) {
	struct stat path_stat;
	return path_exists(path, &path_stat);
}

bool is_directory_and_exists(const char *path) {
	struct stat path_stat;
	if (file_or_directory_exists(path, &path_stat) == false) {
		return false;
	}
	return S_ISDIR(path_stat.st_mode);
}


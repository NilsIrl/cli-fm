
#include "file_manipulation.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

bool path_exists_at(const int dirfd, const char *pathname, struct stat *statbuf) {
	if (fstatat(dirfd, pathname, statbuf, 0) != 0) {
		if (errno == ENOENT) {
			return false;
		} else {
			fprintf(stderr, "stat failed in dir_exists.\n");
			exit(EXIT_FAILURE);
		}
	}
	return true;
}

bool file_or_directory_exists_at(const int dirfd, const char *pathname) {
	struct stat statbuf;
	return path_exists(dirfd, pathname, &statbuf);
}

bool is_directory_and_exists(const int dirfd, const char *pathname) {
	struct stat statbuf;
	if (path_exists_at(dirfd, pathname, &statbuf) == false) {
		return false;
	}
	return S_ISDIR(path_stat.st_mode);
}


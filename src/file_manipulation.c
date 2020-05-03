
#include "file_manipulation.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

bool path_exists(const char *path, struct stat *statbuf) {
	if (stat(path, statbuf) == -1) {
		if (errno == ENOENT) {
			return false;
		} else {
			fputs("stat failed in path_exists.\n", stderr);
			exit(1);
		}
	}
	return true;
}

bool file_exists(const char *path) {
	struct stat statbuf;
	return path_exists(path, &statbuf);
}

/*
 * returns false if it doesn't exist or isn't a directory
 */
bool is_directory(const char *path) {
	struct stat statbuf;
	if (path_exists(path, &statbuf) == false)
		return false;
	return S_ISDIR(statbuf.st_mode);
}

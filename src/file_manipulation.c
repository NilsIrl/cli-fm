
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
			exit(EXIT_FAILURE);
		}
	}
	return true;
}

bool file_exists(const char *path) {
	struct stat statbuf;
	return path_exists(path, &statbuf);
}

bool file_exists_at_path(struct cdpath *cdpath, const char *suffix, const size_t suffix_len) {
	if (cdpath->suffix - cdpath->path + suffix_len > sizeof(cdpath->path)) {
		fputs("filename too long :( in file_exists_at_path.\n", stderr);
		exit(2); // "user" error
	}
	memcpy(cdpath->suffix, suffix, suffix_len);
	return file_exists(cdpath->path);
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

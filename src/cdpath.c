
#include "cdpath.h"

#include <stdio.h>
#include <stdlib.h>

bool file_exists_at_path(struct cdpath *cdpath, const char *suffix, const size_t suffix_len) {
	if (cdpath->suffix - cdpath->path + suffix_len > sizeof(cdpath->path)) {
		fputs("filename too long :( in file_exists_at_path.\n", stderr);
		exit(2); // "user" error
	}
	memcpy(cdpath->suffix, suffix, suffix_len);
	return file_exists(cdpath->path);
}


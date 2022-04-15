/*
 * @file list-file.cpp
 */
#include <iostream>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <wchar.h>
#include <stdio.h>
#define PATH_DELIMITER "\\"
#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif
#else
#include <sys/param.h>
#include <fcntl.h>

#include <ftw.h>

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <cstdio>

#if defined(EMSCRIPTEN)
#else
#include <fts.h>
#endif

#define PATH_DELIMITER "/"

#ifndef F_GETPATH
#define F_GETPATH	(1024 + 7)
#endif

#endif

#include "list-file.h"


#ifdef __ANDROID__
#if __ANDROID_API__ < 17
#error Android API must be 17 or more for ftw()
#endif

#if __ANDROID_API__ < 21
#error Android API must be 17 or more for fts_open()
#endif

#endif

#if defined(_WIN32) || defined(_WIN64)

char* realpath(const char* path, char* resolved_path) {
	GetFullPathNameA(path, PATH_MAX, resolved_path, NULL);
	return resolved_path;
}

/**
 * Return list of files in specified path
 * @param path
 * @param retval can be NULL
 * @return count files
 */
size_t listFiles
(
	const std::string &suffix,
	int flags,
	std::vector<std::string> *retval,
	const std::string &path
)
{
	// TODO Implement Windows
	return 0;
}
#else

/**
 * Return list of files in specified path
 * @param path
 * @param flags 0- as is, 1- full path, 2- relative (remove parent path)
 * @param retval can be NULL
 * @return count files
 * FreeBSD fts.h fts_*()
 */
size_t listFiles
(
	std::vector<std::string> *retval,
	const std::string &path,
	const std::string &suffix,
	int flags
)
{
	char *pathlist[2];
	pathlist[1] = NULL;
	if (flags & 1) {
		char realtapth[PATH_MAX + 1];
		pathlist[0] = realpath((char *) path.c_str(), realtapth);
	} else {
		pathlist[0] = (char *) path.c_str();
	}
	int parent_len = strlen(pathlist[0]) + 1;	///< Arggh. Remove '/' path delimiter(I mean it 'always' present). Not sure is it works fine. It's bad, I know.

    size_t count = 0;
#if defined(EMSCRIPTEN)
#else
	FTS* file_system = fts_open(pathlist, FTS_LOGICAL | FTS_NOSTAT, NULL);
    if (!file_system)
    	return 0;
    FTSENT* parent;
	while((parent = fts_read(file_system))) {
		FTSENT* child = fts_children(file_system, 0);
		if (errno != 0) {
			// ignore, perhaps permission error
		}
		while (child)
		{
			switch (child->fts_info) {
				case FTS_F:
					{
						std::string s(child->fts_name);
						if (s.find(suffix) != std::string::npos)
						{
							count++;
							if (retval) {
								if (flags & 2) {
									// extract parent path
									std::string p(&child->fts_path[parent_len]);
									retval->push_back(p + s);
								}
								else
									retval->push_back(std::string(child->fts_path) + s);
							}
						}
					}
					break;
				default:
					break;
			}
			child = child->fts_link;
		}
	}
	fts_close(file_system);
#endif
	return count;
}

bool isOrdinalFile(
	time_t &retModificationTime,
	const char *path
) {
	struct stat s;
	if (stat(path, &s) == 0 ) {
		retModificationTime = s.st_mtime;
		if (s.st_mode & S_IFREG )
			return true;
	}
	return false;
}

#endif
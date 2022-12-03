/*
 * @file utilfile.cpp
 */
#include <iostream>

#ifdef _MSC_VER
#include <windows.h>
#include <wchar.h>
#include <stdio.h>
#include <io.h>
#define F_OK            0
#define PATH_DELIMITER "\\"
#else
#include <sys/param.h>
#include <fcntl.h>

#include <ftw.h>

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <string.h>
#include <errno.h>
#include <cstdio>

#define PATH_DELIMITER "/"

#ifndef F_GETPATH
#define F_GETPATH	(1024 + 7)
#endif

#endif

#include "utilfile.h"

#ifdef __ANDROID__
#if __ANDROID_API__ < 17
#error Android API must be 17 or more for ftw()
#endif

#if __ANDROID_API__ < 21
#error Android API must be 17 or more for fts_open()
#endif

#endif

#ifdef _MSC_VER

bool util::fileExists(const std::string &fileName)
{
    return _access(fileName.c_str(), F_OK) != -1;
}

#else

bool util::fileExists(const std::string &fileName)
{
    struct stat r;
    return stat(fileName.c_str(), &r) == 0;
}

#endif

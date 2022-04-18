/*
 * @file list-file.h
 */
#ifndef LIST_FILE_H
#define LIST_FILE_H     1

#include <string>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
char* realpath(const char* path, char* resolved_path);
#endif

/**
 * @brief Return list of files in specified path
 * @param path directory or file path
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
);

/**
 * @brief Return is file ordinal. If not, it is directory or special file.
 * 
 * @param reModificationTime return modofication time
 * @param path directory or file path
 * @return true - ordinal file
 * @return false - directory ot special file
 */
bool isOrdinalFile(
	time_t &reModificationTime,
	const char *path
);

/**
 * Return true if file name extension is .json
 * @param path file name to examine
 * @return true if file name extension is .json
 */
bool fileIsJSON(const std::string &path);

#endif

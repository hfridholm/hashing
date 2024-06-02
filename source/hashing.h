#ifndef HASHING_H
#define HASHING_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <argp.h>

extern size_t files_count(char** paths, size_t path_count, int depth);

extern char** files_create(size_t count, char** paths, size_t path_count, int depth);

extern size_t files_size(char** files, size_t count);

extern int    files_read(void* pointer, size_t size, size_t nmemb, char** files, size_t count);

extern void   files_free(char** files, size_t count);


extern int    file_read(void* pointer, size_t size, size_t nmemb, const char* filepath);

extern size_t file_size(const char* filepath);


extern char* sha256(char hash[64], const void* message, size_t size);

extern char* md5(char hash[32], const void* message, size_t size);

#endif // HASHING_H

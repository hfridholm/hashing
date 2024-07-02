#ifndef HASHING_H
#define HASHING_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <argp.h>

// Create and free functions to get paths to files
extern char** files_create(size_t* count, char** paths, size_t path_count, int depth);

extern void   files_free(char** files, size_t count);

// Size and read functions to read combined data from multiple files
extern size_t files_size_get(char** files, size_t count);

extern int    files_read(void* pointer, size_t size, size_t nmemb, char** files, size_t count);

// Size and read functions to read data from single file 
extern size_t file_size_get(const char* filepath);

extern size_t file_read(void* pointer, size_t size, size_t nmemb, const char* filepath);

// Hash algorithm functions to hash data
extern char* sha256(char hash[64], const void* message, size_t size);

extern char* md5(char hash[32], const void* message, size_t size);

#endif // HASHING_H

#ifndef HASHING_H
#define HASHING_H

#include <stdio.h>
#include <string.h>

extern char* sha256(char hash[64], const void* poiter, size_t size);

extern char* md5(char hash[32], const void* message, size_t size);

#endif // HASHING_H

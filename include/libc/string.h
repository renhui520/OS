#ifndef __OS_STRING_H__
#define __OS_STRING_H__

#include <stddef.h>

int memcmp(const void*, const void*, size_t);
// void* memcpy(void* __restrict, const void* __restrict, size_t);
void* memcpy(void*, const void*, size_t);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);

size_t strlen(const char* str);
size_t strnlen(const char* str, size_t maxlen);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
const char* strchr(const char* str, int character);

#endif
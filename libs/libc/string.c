#include <libc/string.h>

int memcmp(const void*, const void*, size_t){}
void* memcpy(void* __restrict, const void* __restrict, size_t){}
void* memmove(void*, const void*, size_t){}
void* memset(void*, int, size_t){}
size_t strlen(const char* str)
{
    size_t len = 0;
    while (*str++)
        len++;
    return len;
}

size_t strnlen(const char* str, size_t maxlen){}
char* strcpy(char* destination, const char* source){}
char* strncpy(char* destination, const char* source, size_t num){}
const char* strchr(const char* str, int character){}
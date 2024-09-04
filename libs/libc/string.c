#include <libc/string.h>

#include <stdint.h>

inline int memcmp(const void* dest, const void* src, size_t num)
{
    const uint8_t* dest_ptr = (const uint8_t*)dest;
    const uint8_t* src_ptr = (const uint8_t*)src;
    for (size_t i = 0; i < num; i++)
    {
        int diff = (int)*(dest_ptr + i) - (int)*(src_ptr + i);
        if (diff != 0)
        {
            return diff;
        } 
    }
    return 0; 
}
inline void* memcpy(void* dest, const void* src, size_t num)
{
    uint8_t* dest_ptr = (uint8_t*)dest;
    const uint8_t* src_ptr = (const uint8_t*)src;

    for (size_t i = 0; i < num; i++)
    {
        *(dest_ptr + i) = *(src_ptr + i);
    }
    return dest;
}
void* memmove(void*, const void*, size_t){}
void* memset(void* ptr, int value, size_t num)
{
    uint8_t* c_ptr = (uint8_t*)ptr;
    for (size_t i = 0; i < num; i++)
    {
        *(c_ptr + i) = (uint8_t)value;
    }
    return ptr;
}
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
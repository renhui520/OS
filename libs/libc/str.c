#include <libc/string.h>

size_t strlen(const char* str)
{
    const char* str_ptr = (const char*)str;
    size_t len = 0;
    while (*str_ptr != '\0')
        str_ptr++;
        len++;
    return len;
}

size_t strnlen(const char* str, size_t maxlen)
{
    size_t len = 0;
    while (str[len] && len < maxlen)
        len++;
    return len;
}
char* strcpy(char* destination, const char* source)
{
    char* dest_ptr = destination;
    while (*source != '\0')
    {
        *destination = *source;
        destination++;
        source++;
    }
    *destination = '\0';
    return dest_ptr;
    
}

char* strncpy(char* destination, const char* source, size_t num)
{
    char* dest_ptr = destination;
    while (*source != '\0' && num > 0 && num--)
    {
        *destination = *source;
        destination++;
        source++;
    }
    *destination = '\0';
    return dest_ptr;
}
const char* strchr(const char* str, int character)
{
    while (*str)
    {
        if (*str == character)
            return str;
        str++;
    }
    return character == '\0' ? str : NULL;
}
#define __LIBC__

#define NULL (void*)0

#include <libc/stdlib.h>

// char base_char[] = "0123456789abcdefghijklmnopqrstuvwxyz";
// char* __uitoa_internal(unsigned int value, char* str, int base, unsigned int* size)
// {
//     unsigned int ptr = 0;
//     do {
//         str[ptr] = base_char[value % base];
//         value = value / base;
//         ptr++;
//     } while (value);

//     for (unsigned int i = 0; i < (ptr >> 1); i++) {
//         char c = str[i];
//         str[i] = str[ptr - i - 1];
//         str[ptr - i - 1] = c;
//     }
//     str[ptr] = '\0';
//     if (size) {
//         *size = ptr;
//     }
//     return str;
// }


char* __uitoa_internal(unsigned long value, char* str, int base, unsigned int* size)
{ 
    unsigned int ptr = 0;
    do {
        str[ptr] = '0' + (value % base);
        value /= base;
        ptr++;
    } while (value);

    for (unsigned int i = 0; i < (ptr >> 1); i++)
    {
        str[i] ^= str[ptr - i - 1];
        str[ptr - i - 1] ^= str[i];
        str[i] ^= str[ptr - i - 1];
    }
     
    str[ptr] = '\0';

    if (size)
    {
        *size = ptr;
    }
    return str;
}

char* __itoa_internal(long value, char* str, int base, unsigned int* size)
{
    if (value < 0 && base == 10)
    {
        str[0] = '-';
        unsigned int _v = (unsigned int)-value;
        __uitoa_internal(_v, str + 1, base, size);
    } else {
        __uitoa_internal((unsigned int)value, str, base, size);
    }

    return str;
}

char* itoa(long value, char* str, int base)
{
    return __itoa_internal(value, str, base, NULL);
}
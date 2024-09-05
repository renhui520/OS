#ifndef __AWA_STDLIB_H
#define __AWA_STDLIB_H

#ifdef __LIBC__

char* __uitoa_internal(unsigned long value, char* str, int base, unsigned int* size);
char* __itoa_internal(long value, char* str, int base, unsigned int* size);

#endif

char* itoa(long value, char* str, int base);

#endif
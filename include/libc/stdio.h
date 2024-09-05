#ifndef __AWA_STDIO_H
#define __AWA_STDIO_H

#include <stdarg.h>
#include <stddef.h>

#ifdef __LIBC__

void __sprintf_internal(char* buffer, char* fmt, size_t max_len, va_list args);

#endif

void sprintf(char* buffer, const char* fmt, ...);
void snprintf(char* buffer, size_t n, char* fmt, ...);


#endif
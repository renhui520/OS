#ifndef __OS_KALLOC_H__
#define __OS_KALLOC_H__

#include <stddef.h>

int kalloc_init();

void* malloc(size_t size);

void* calloc(size_t n, size_t elem);

void free(void* ptr);

#endif
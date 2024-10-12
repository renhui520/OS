#ifndef __OS_KALLOC_H__
#define __OS_KALLOC_H__

#include <stddef.h>

int kalloc_init();

// 分配 堆 内存 (不清空内存)
void* malloc(size_t size);

// 重新 分配指定数量的 堆区 内存 即 nmemb * size (会清空内存)
void* calloc(size_t nmemb, size_t size);

void free(void* ptr);

#endif
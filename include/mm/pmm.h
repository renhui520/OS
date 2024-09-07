#ifndef __OS_PMM_H__
#define __OS_PMM_H__


#include <stdint.h>
#include <stddef.h>


void pmm_init(uintptr_t mem_upper_lim);

void pmm_mark_page_free(uintptr_t ppn);
void pmm_mark_chunk_free(uintptr_t start_ppn, size_t page_count);


void pmm_mark_page_occupied(uintptr_t ppn);
void pmm_mark_chunk_occupied(uintptr_t start_ppn, size_t page_count);


void* pmm_alloc_page(void);
int pmm_free_page(void* page);


#endif
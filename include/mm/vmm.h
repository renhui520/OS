#ifndef __OS_VMM_H__
#define __OS_VMM_H__

#include <mm/page.h>
#include <mm/pmm.h>

void vmm_init(void);

void* vmm_map_page(void* va, void* pa, pt_attr tattr);

#endif
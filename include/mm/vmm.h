#ifndef __OS_VMM_H__
#define __OS_VMM_H__

#include <mm/page.h>
#include <mm/pmm.h>

void vmm_init(void);

/**
 * @brief 尝试为一个 虚拟页 创建一个可用的 物理页 映射
 *
 * @param va 虚拟页地址
 * @return 物理页地址，如不成功，则为 NULL
 */
void* vmm_alloc_page(void* va, pt_attr tattr);

/**
 * @brief 尝试为多个连续的 虚拟页 分配一个可用的 物理页
 * 
 * @param va 起始虚拟地址
 * @param sz 大小（必须为4K对齐）
 * @param tattr 属性
 * @return int 是否成功
 */
int vmm_alloc_pages(void* va, size_t sz, pt_attr tattr);

/**
 * @brief 设置一个映射，如果映射已存在，则忽略。
 * 
 * @param va 
 * @param pa 
 * @param attr 
 */
void vmm_set_mapping(void* va, void* pa, pt_attr attr);

/**
 * @brief 删除一个映射
 *
 * @param vpn
 */
void vmm_unmap_page(void* va);

// 建立一个映射
void* vmm_map_page(void* va, void* pa, pt_attr tattr);

#endif
#include <mm/vmm.h>

#include <hal/cpu.h>

#include <libc/string.h>

#define true 1
#define false 0

void vmm_init(void)
{

}

// 映射一个物理地址到虚拟地址
// 1、寻找一个L2空页存储 这个物理地址
// 2、判断这个L1页表是否为空
int __vmm_map_internal(uint32_t l1_index, 
                       uint32_t l2_index, 
                       uintptr_t physical_addr, 
                       pt_attr attr,    // 好像就是flags
                       int forced)
{
    x86_page_table* l1_ptd = (x86_page_table*)L1_BASE_VADDR;        // 页目录！
    x86_page_table* l2_ptd = (x86_page_table*)L2_VADDR(l1_index);   // l1_index页表！

    // 如果 L1_ptd 为空
    if (!l1_ptd->entry[l1_index])
    {
        x86_page_table *new_l1pt_pa = pmm_alloc_page();

        // 物理内存已满！
        if (!new_l1pt_pa)   // new_l1pt_pa == NULL
        {
            return 0;
        }

        // 往页目录 中的 第l1_index页表位置 注册 一个新的页表
        l1_ptd->entry[l1_index] = NEW_L1_ENTRY(attr, new_l1pt_pa);

        // 设置 l1_index页目录项 指向的页表的 页表项 为 0
        memset((void *)L2_VADDR(l1_index), 0, PG_SIZE);
    }

    // 不为 空 则 在当前页
    x86_pte_t l2pte = l2_ptd->entry[l2_index];  // l2_index 页表  这个页表指向唯一一个 物理页
    
    // 检测 L2pte(页表项) 是否为 空
    // 不为 空 则:
    if (l2pte) {
        
        // 若 不允许 强制更改 则结束
        if (!forced) {
            return 0;
        }

        // 允许 则 进行提示并释放 然后等待修改
        if (HAS_FLAGS(l2pte, PG_PRESENT)) {
            //assert_msg(pmm_free_page(GET_PG_ADDR(l2pte)), "fail to release physical page");
            pmm_free_page((void*)GET_PG_ADDR(l2pte));// 释放掉 pte
        }
    }

    // 为 空 则:

    // l2_index 页表  这个页表指向唯一一个 物理页
    l2_ptd->entry[l2_index] = NEW_L2_ENTRY(attr, physical_addr);// 修改l2指向的物理页

    return 1;

}

//尝试将 物理页 映射到 虚拟页
//若目标虚拟页已映射，则寻找更大且未使用的地址进行映射
void* vmm_map_page(void* va, void* pa, pt_attr tattr)
{
    // 显然，对空指针进行映射没有意义。
    if (!pa || !va) {
        return NULL;
    }

    // 检测室否为NULL地址
    //assert(((uintptr_t)va & 0xFFFU) == 0) assert(((uintptr_t)pa & 0xFFFU) == 0);

    //获得一级目录索引
    uint32_t l1_index = L1_INDEX(va); //获取 并 保存 虚拟页目录索引 "PDE"
    //获得二级目录索引
    uint32_t l2_index = L2_INDEX(va); //获取 并 保存 虚拟页表索引 "PTE"
    
    // L1_BASE_VADDR = 0xFFFFF000U 所以为 第1023个页表中的第1023项，指向 页目录!
    x86_page_table* l1pt = (x86_page_table*)L1_BASE_VADDR; // 指向 页目录吧...可能是用来方便修改页目录项？
    /* L1_BASE_VADDR = 0xFFFFF000U 指向 PTD本身 */

    // 在页表与页目录中找到一个可用的空位进行映射（位于va或其附近）

    // 将页目录 中的 l1_index 页表项 赋值给 l1pte
    x86_pte_t l1pte = l1pt->entry[l1_index];
    
    // 获得二级页表
    // 0xFFC00000为第1023个页表，但是由于指向页目录，所以就是页目录，剩下的l1_index则为 页目录项
    x86_page_table* l2pt = (x86_page_table*)L2_VADDR(l1_index);//L2_VADDR(l1_index)相当于l1t[1023][l1_index]
    
    // 这部分while用于在l1_index和l2_index这俩索引位置或+n位置寻找，可以减少不必要的__vmm_map_internal调用带来的性能损失
    // 循环遍历页表，直到找到一个空位
    while (l1pte && l1_index < PG_MAX_ENTRIES) {
        if (l2_index == PG_MAX_ENTRIES) {
            l1_index++;
            l2_index = 0;
            l1pte = l1pt->entry[l1_index];
            l2pt = (x86_page_table*)L2_VADDR(l1_index);
        }

        // 页表有空位，只需要开辟一个新的 PTE (Level 2)
        if (l2pt/*判断l2pt页表地址是否为NULL*/ && !l2pt->entry[l2_index]/*判断l2pt的页表项pte是否为空*/) {
            l2pt->entry[l2_index] = NEW_L2_ENTRY(tattr, pa);
            return (void*)V_ADDR(l1_index, l2_index, PG_OFFSET(va));    // 合并 并 返回地址
        }
        l2_index++;
    }

    // 页目录与所有页表已满！
    if (l1_index > PG_MAX_ENTRIES) {
        return NULL;
    }

    // 若从l1_index往后的页表均无空位，则在整段内存中寻找空位并返回指针
    if (!__vmm_map_internal(l1_index, l2_index, (uintptr_t)pa, tattr, false)) {
        return NULL;
    }
    // 若__vmm_map_internal 设置成功，则返回地址
    return (void*)V_ADDR(l1_index, l2_index, PG_OFFSET(va));
}

void*
vmm_fmap_page(void* va, void* pa, pt_attr tattr)
{
    if (!pa || !va) {
        return NULL;
    }

    // assert(((uintptr_t)va & 0xFFFU) == 0) assert(((uintptr_t)pa & 0xFFFU) == 0);

    uint32_t l1_index = L1_INDEX(va);
    uint32_t l2_index = L2_INDEX(va);

    if (!__vmm_map_internal(l1_index, l2_index, (uintptr_t)pa, tattr, true)) {
        return NULL;
    }

    cpu_invplg(va);

    return (void*)V_ADDR(l1_index, l2_index, PG_OFFSET(va));
}

/*-------------------------------------------------------*/

// 立刻 为给定的 虚拟页 分配一个可用的 物理页
void* vmm_alloc_page(void* vpn, pt_attr tattr)
{
    void* pp = pmm_alloc_page();
    void* result = vmm_map_page(vpn, pp, tattr);
    if (!result) {
        pmm_free_page(pp);
    }
    return result;
}

// 分配多个连续的 虚拟页
int
vmm_alloc_pages(void* va, size_t sz, pt_attr tattr)
{
    // assert((uintptr_t)va % PG_SIZE == 0) assert(sz % PG_SIZE == 0);

    void* va_ = va;
    for (size_t i = 0; i < (sz >> PG_SIZE_BITS); i++, va_ = (void*)((uintptr_t)va_ + PG_SIZE)) {
        void* pp = pmm_alloc_page();
        uint32_t l1_index = L1_INDEX(va_);
        uint32_t l2_index = L2_INDEX(va_);
        if (!pp || !__vmm_map_internal(
                     l1_index, l2_index, (uintptr_t)pp, tattr, false)) {
            // if one failed, release previous allocated pages.
            va_ = va;
            for (size_t j = 0; j < i; j++, va_ = (void*)((uintptr_t)va_ + PG_SIZE)) {
                vmm_unmap_page(va_);
            }

            return false;
        }
    }

    return true;
}

/*-------------------------------------------------------*/

//若映射不存在则设置新的映射，否则忽略操作
/*
似乎和
vmm_map_page
vmm_fmap_page
的实现效果是一样的，只不过是不同方式

fmap是强制
set和map是非强制
*/
void vmm_set_mapping(void* va, void* pa, pt_attr attr) {
    // assert(((uintptr_t)va & 0xFFFU) == 0);

    uint32_t l1_index = L1_INDEX(va);
    uint32_t l2_index = L2_INDEX(va);

    // 如果等于最后一页，即 页目录. 则直接返回
    if (l1_index == 1023) {
        return;
    }
    
    __vmm_map_internal(l1_index, l2_index, (uintptr_t)pa, attr, false);
}

// 取消 虚拟页的映射
void vmm_unmap_page(void* va)
{
    uint32_t l1_index = L1_INDEX(va);
    uint32_t l2_index = L2_INDEX(va);

    if (l1_index == 1023)
    {
        return;
    }


    x86_page_table* l1_pt = (x86_page_table*)L1_BASE_VADDR;
    x86_pte_t l1_pte = l1_pt->entry[l1_index];
    
    if (l1_pte)
    {
        x86_page_table* l2_pt = (x86_page_table*)L2_VADDR(l1_index);
        x86_pte_t l2_pte = l2_pt->entry[l2_index];

        // 检测页是否被 映射?
        if (IS_CACHED(l2_pte))
        {
            pmm_free_page((void*)l2_pte);   // 释放掉已占用的页
        }

        // 使 相应页表的 TLB 无效
        cpu_invplg(va);
        l2_pt->entry[l2_index] = 0;     // 使其指向 空 指针位
    }
}
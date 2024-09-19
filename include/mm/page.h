#ifndef __OS_PAGE_H
#define __OS_PAGE_H
#include <stdint.h>

#define PG_SIZE_BITS                12                      // 12位 即页大小为4KB
#define PG_SIZE                     (1 << PG_SIZE_BITS)     // 4KB : (1 << 12)= 1 * 2^12
#define PG_INDEX_BITS               10                      // 10位 页索引的比特位数

#define PG_MAX_ENTRIES              1024U                   // 1024个最大页表项
#define PG_LAST_TABLE               PG_MAX_ENTRIES - 1      // 最后一个页表项的索引
#define PG_FIRST_TABLE              0                       // 第一个页表项的索引

#define PTE_NULL                    0                       // 页表项为空 或 无效

#define HIGHER_HLF_BASE 0xC0000000UL
#define P2V(paddr)          ((uintptr_t)(paddr)  +  HIGHER_HLF_BASE)    // 将 物理地址 转换为 虚拟地址
#define V2P(vaddr)          ((uintptr_t)(vaddr)  -  HIGHER_HLF_BASE)    // 将 虚拟地址(高半核虚拟地址) 转换为 物理地址(实际所处的低半核地址)

//0xFFFFF000UL在二进制表示中为 1111111111111111111111111111111100000000
//最高20位保持不变 最低12位被清零   4KB对齐?
#define PG_ALIGN(addr)      ((uintptr_t)(addr)   & 0xFFFFF000UL)        // 4K 对齐 将低12位(标志位)清零 保证只有页地址

#define L1_INDEX(vaddr)     (uint32_t)(((uintptr_t)(vaddr) & 0xFFC00000UL) >> 22)   // 获取虚拟页目录索引 PDE? 保留高10位，右移22位
#define L2_INDEX(vaddr)     (uint32_t)(((uintptr_t)(vaddr) & 0x003FF000UL) >> 12)   // 获取虚拟页表索引   PTE? 保留中10位，右移12位
#define PG_OFFSET(vaddr)    (uint32_t)((uintptr_t)(vaddr)  & 0x00000FFFUL)        // 获取页内偏移   保留低12位

#define GET_PT_ADDR(pde)    PG_ALIGN(pde)   // 获取页目录地址
#define GET_PG_ADDR(pte)    PG_ALIGN(pte)   // 获取页表地址

#define PG_DIRTY(pte)           ((pte & (1 << 6)) >> 6) // 获取页表是否被修改   获取修改(脏？)位
#define PG_ACCESSED(pte)        ((pte & (1 << 5)) >> 5) // 获取页表是否被访问   获取访问位

#define IS_CACHED(entry)    ((entry & 0x1)) // 获取页表是否被缓存

#define PG_PRESENT              (0x1)       // 页表 [存在]
#define PG_WRITE                (0x1 << 1)  // 页表 [可写]
#define PG_ALLOW_USER           (0x1 << 2)  // 页表 [允许用户访问]
#define PG_WRITE_THROUGHT       (1 << 3)    // 页表 [写回缓存]
#define PG_DISABLE_CACHE        (1 << 4)    // 页表 [禁用缓存]
#define PG_PDE_4MB              (1 << 7)    // 页表 [4MB]

//(flags) & 0xfff) 的0xfff ==> 111111111111
//高位清零，低位不变
#define NEW_L1_ENTRY(flags, pt_addr)     (PG_ALIGN(pt_addr) | ((flags) & 0xfff))    // 新建页目录项(页表)  pde
#define NEW_L2_ENTRY(flags, pg_addr)     (PG_ALIGN(pg_addr) | ((flags) & 0xfff))    // 新建页表项   pte

#define V_ADDR(pd, pt, offset)  ((pd) << 22 | (pt) << 12 | (offset)) // 获取虚拟地址
#define P_ADDR(ppn, offset)     ((ppn << 12) | (offset)) // 获取物理地址

#define PG_ENTRY_FLAGS(entry)   (entry & 0xFFFU) // 获取页表属性
#define PG_ENTRY_ADDR(entry)   (entry & ~0xFFFU) // 获取页表地址

#define HAS_FLAGS(entry, flags)             ((PG_ENTRY_FLAGS(entry) & (flags)) == flags) // 判断页表是否包含 flags中所有的属性
#define CONTAINS_FLAGS(entry, flags)        (PG_ENTRY_FLAGS(entry) & (flags)) // 判断页表是否包含 至少flags中的一个属性

#define PG_PREM_R              PG_PRESENT                               // 页表  [可读]
#define PG_PREM_RW             PG_PRESENT | PG_WRITE                    // 页表  [可读写]
#define PG_PREM_UR             PG_PRESENT | PG_ALLOW_USER               // 页表  [允许用户访问]
#define PG_PREM_URW            PG_PRESENT | PG_WRITE | PG_ALLOW_USER    // 页表  [允许用户读写]

// 用于对PD进行循环映射，因为我们可能需要对PD进行频繁操作，我们在这里禁用TLB缓存
#define T_SELF_REF_PERM        PG_PREM_RW | PG_DISABLE_CACHE


/*
0xFFFFF000U 在32位系统中表示一个非常高的地址，接近4GB的地址空间顶部
二进制形式为： 11111111111111111111000000000000
22到31位 作为 页目录索引 即转化为十进制为1023 代表第1023个(从0开始算一个)页目录项(第1023个页表L1 的 PT)
12到21位 作为 页表项索引 即转化为十进制为1023 代表第1023个(从0开始算一个)页表项(第1023个页表项L1 的 PTE指向L2 PT)
PTE 指向 物理页

二级页表: L1t页表 ==> L2t页表
0xFFFFF000 = 0xFFC00000 | 0x003FF000 | 0x00000000
                L1          L2           offset
十六进制: 0xFFC00000 ==> 二进制: 11111111110000000000000000000000   0占22个位   所以 >> 22 = 1023
十六进制: 0x003FF000 ==> 二进制: 00000000001111111111000000000000   0占12个位   所以 >> 12 = 1023
*/

/* mmu地址翻译会先访问页目录，然后在页目录中将解析后的地址(32位)前10位作为一级页表的索引，最后 中间10位被作为二级页表地址，最终指向一块实际的物理地址*/
// 0xFFFFF000 = 0xFFC00000 | 0x003FF000 | 0x00000000
// 以上地址实际上就是 先在页目录中找到第1023个entry，即页目录[ptd基地址不可修改]，然后再找到第1023个页，也就是对应了页目录[现在才能被访问和修改！]
// mmu翻译必须要一步一步来！必须要先有前10位，中10位，末12位依次翻译访问！只有最终被翻译好的地址才能被访问！

// 页目录的地址
#define L1_BASE_VADDR                0xFFFFF000U

// 页表的虚拟基地址，可以用来访问到各个PTE
#define L2_BASE_VADDR                 0xFFC00000U   //为下面L2_VADDR 提供 上面注释中所提到的L1头

// 用来获取特定的页表的虚拟地址
#define L2_VADDR(pd_offset)           (L2_BASE_VADDR | (pd_offset << 12)) //因为 >> 12 所以要 << 12才能的到虚拟地址(舍弃掉offset位)

typedef unsigned long ptd_t;
typedef unsigned long pt_t;
typedef unsigned int pt_attr;

/**
 * @brief 虚拟映射属性
 * 
 */
typedef struct {
    // 物理页码（如果不存在映射，则为0）
    uint32_t pn;
    // 物理页地址（如果不存在映射，则为0）
    uintptr_t pa;
    // 映射的flags
    uint16_t flags;
} v_mapping;

typedef uint32_t x86_pte_t;
typedef struct
{
    x86_pte_t entry[PG_MAX_ENTRIES]; // "页表"数组 也就是 页表 页表中保存了页表项PTE PTE指向物理页
} __attribute__((packed)) x86_page_table;

#endif

#include <stdint.h>
#include <boot/multiboot.h>
#include <mm/page.h>

//检查 multiboot flags 是否 等于 某个标志位（info）
//若是 返回非0
//若否 返回 0
#define present(flags, info)      ((flags) & (info))

#define PT_ADDR(ptd, pt_index)                          ((uint32_t*)ptd + (pt_index + 1) * 1024)
#define SET_PDE(ptd, pde_index, pde)                    *((uint32_t*)ptd + pde_index) = pde;
#define SET_PTE(ptd, pde_index, pte_index, pte)         *(PT_ADDR(ptd, pde_index) + pte_index) = pte;
void _init_page(uint32_t* ptd)
{
    SET_PDE(ptd, 0, NEW_L1_ENTRY(PG_PREM_RW, ptd + 1024U))
    for (int i = 0; i < 256; i++)
    {
        SET_PTE(ptd, 0, 0, NEW_L2_ENTRY(PG_PREM_RW, i << PG_SIZE_BITS))
    }
}

// 复制 mmap_addr 到 destination(destination+current) ??
uint32_t __save_subset(uint8_t* destination, uint8_t* base, unsigned int size) {
    unsigned int i = 0;
    for (; i < size; i++)
    {
        *(destination + i) = *(base + i);
    }
    return i;
}

/*
 * 存储multiboot信息到一个固定的位置，防止找不到
 * info信息
 * destination目标起始地址
*/
void 
_save_multiboot_info(multiboot_info_t* info, uint8_t* destination) {

    uint32_t current = 0;
    uint8_t* info_b = (uint8_t*) info;  //指向传给info的地址
    for (; current < sizeof(multiboot_info_t); current++)
    {
        *(destination + current) = *(info_b + current); //在目标起始地址后逐个写入multiboot的info
    }

    //计算destination的mmap_addr即mmap_addr中存储的mmap地址
    //并指向 [destination+current] 后的地址     下一行代码会望这个地址写入mmap_addr
    ((multiboot_info_t*) destination)->mmap_addr = (uintptr_t)destination + current;
    //通过__save_subset函数将 [mmap_addr指向的mmap] 复制到 [destination+current] 后
    //并增加current偏移量
    current += __save_subset(destination + current, (uint8_t*)info->mmap_addr, info->mmap_length);

    //同上，只不过是保存驱动器信息
    if (present(info->flags, MULTIBOOT_INFO_DRIVE_INFO)) {
        ((multiboot_info_t*) destination)->drives_addr = (uintptr_t)destination + current;
        current += __save_subset(destination + current, (uint8_t*)info->drives_addr, info->drives_length);
    }
}

void _hhk_init(uint32_t* ptd, uint32_t kpg_size)
{
    uint8_t * kpg = (uint8_t*)ptd;  //强转，保证每次递增增长1字节，而不是4字节
    for(uint32_t i = 0; i < kpg_size; i++)
    {
        *(kpg + i) = 0;//清空kpg的每一项
    }

    _init_page(ptd);
}

#include <stdint.h>
#include <boot/multiboot.h>
#include <mm/page.h>

//检查 multiboot flags 是否 等于 某个标志位（info）
//若是 返回非0
//若否 返回 0
#define present(flags, info)      ((flags) & (info))

//PT_ADDR使其获得偏移一个页表的地址
#define PT_ADDR(ptd, pt_index)                          ((uint32_t*)ptd + (pt_index + 1) * 1024)
#define SET_PDE(ptd, pde_index, pde)                    *((uint32_t*)ptd + pde_index) = pde;
#define SET_PTE(ptd, pde_index, pte_index, pte)         *(PT_ADDR(ptd, pde_index) + pte_index) = pte;

// 获得 符号(变量？) 的 地址
#define sym_val(sym)                                 (uintptr_t)(&sym)

//计算内核和hhk_init所需的页表数量
//（kernel_end - kernel_start + 0x1000 - 1） / 4KiB 得到页表数量
#define KERNEL_PAGE_COUNT           ((sym_val(__kernel_end) - sym_val(__kernel_start) + 0x1000 - 1) >> 12);

//sym_val获取__init_hhk_end的物理地址 并减去 0x100000 从而得出所使用的内存数量 0x1000为页大小4KiB, ">> 12"相当于除以4KiB, 最后得出所需页数
//加上 0x1000 - 1：这是为了确保即使内核大小不是页大小的整数倍，也能正确计算所需的页数。这里 - 1 是为了向上取整，+ 0x1000 是为了补偿 - 1 的影响
#define HHK_PAGE_COUNT              ((sym_val(__init_hhk_end) - 0x100000 + 0x1000 - 1) >> 12)


//-----------------------------定义页目录中的索引
// use table #1
#define PG_TABLE_IDENTITY           0

// use table #2-4
// hence the max size of kernel is 8MiB
#define PG_TABLE_KERNEL             1   //内核页表的start地址，后续会有循环递增 直到第四个

// use table #5
#define PG_TABLE_STACK              4

// Provided by linker (see linker.ld)
extern uint8_t __kernel_start;
extern uint8_t __kernel_end;
extern uint8_t __init_hhk_end;
extern uint8_t _k_stack;



void _init_page(uint32_t* ptd)
{
    //跳过第0个页表项，将第一个页表项放进ptd 0号索引 (0是第0个页表)
    SET_PDE(ptd, 0, NEW_L1_ENTRY(PG_PREM_RW, ptd + 1024U)) // +1024表示跳过第一个页表项，即指向下一个页表
    for (int i = 0; i < 256; i++)
    {
        //从0x0到0x100000，循环映射低1MB的内存
        SET_PTE(ptd, 0, i, NEW_L2_ENTRY(PG_PREM_RW, i << PG_SIZE_BITS))
    }
    // 对等映射我们的hhk_init，这样一来，当分页与地址转换开启后，我们依然能够照常执行最终的 jmp 指令来跳转至
    //  内核的入口点
    for (uint32_t i = 0; i < HHK_PAGE_COUNT; i++)
    {
        //之所以用到256，是因为上面已经映射了256，我们需要跳过这些页
        //0x100000 + (i << PG_SIZE_BITS)主要是因为我们的内核最开始的代码被指定运行在0x100000处，所以我们要映射内核的代码就需要在0x100000处开始映射
        //前面所映射的并非内核的任何代码，只是内存中存在的内容，例如VGA所在地址，而且在实模式下只能访问到最大1MB的内存空间
        SET_PTE(ptd, PG_TABLE_IDENTITY, 256 + i, NEW_L2_ENTRY(PG_PREM_RW, 0x100000 + (i << PG_SIZE_BITS)))
    }
    
    //---- 至此我们仍未将 内核 映射 ----
    //---- 我们所映射的仅仅只是内核代码的0x100000处到 hhk 高半核的内存区域


    //---- 到这里我们才正式开始映射内核！----
    // --- 将内核重映射至高半区 ---
    
    // 这里是一些计算，主要是计算应当映射进的 页目录 与 页表 的条目索引（Entry Index）
    uint32_t kernel_pde_index = L1_INDEX(sym_val(__kernel_start));
    uint32_t kernel_pte_index = L2_INDEX(sym_val(__kernel_start));
    uint32_t kernel_pg_counts = KERNEL_PAGE_COUNT;
    
    // 将内核所需要的页表注册进页目录
    //  当然，就现在而言，我们的内核只占用不到50个页（每个页表包含1024个页）
    //  这里分配了3个页表（12MiB），未雨绸缪。
    //PG_TABLE_STACK - PG_TABLE_KERNEL = 3
    //所以 i = 0, 1, 2 刚好 3个 与前面定义符合(看注释)
    for (uint32_t i = 0; i < PG_TABLE_STACK - PG_TABLE_KERNEL; i++)
    {
        // 分三个 页表
        SET_PDE(
            ptd, 
            kernel_pde_index + i,   
            NEW_L1_ENTRY(PG_PREM_RW, PT_ADDR(ptd, PG_TABLE_KERNEL + i))
        )
    }
    
    // 首先，检查内核的大小是否可以fit进我们这几个表（12MiB）
    if (kernel_pg_counts > (PG_TABLE_STACK - PG_TABLE_KERNEL) * PG_MAX_ENTRIES) {
        // ERROR: require more pages
        //  here should do something else other than head into blocking
        while (1);
    }
    
    // 计算内核.text段的物理地址
    uintptr_t kernel_pm = V2P(&__kernel_start);
    
    // 重映射内核至高半区地址（>=0xC0000000）
    for (uint32_t i = 0; i < kernel_pg_counts; i++)
    {
        // 正式开始映射内核 代码地址 ==> 高半核地址(也就是往高内存地址写入经处理后的原内存地址)
        SET_PTE(
            ptd, 
            PG_TABLE_KERNEL, 
            kernel_pte_index + i, 
            NEW_L2_ENTRY(PG_PREM_RW, kernel_pm + (i << PG_SIZE_BITS))
        )
    }

    // 最后一个entry用于循环映射
    //PG_MAX_ENTRIES-1为entry
    // 循环映射最后(第1024个)一个页表，页表指向自己(也就是ptd), 他用来记录ptd(也就是页目录)的起始地址
    SET_PDE(
        ptd,
        PG_MAX_ENTRIES - 1,/*1023代表 第1024个entry */
        NEW_L1_ENTRY(T_SELF_REF_PERM, ptd)/*将ptd起始地址。。。。(懵了)*/
    );
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

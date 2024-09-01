#ifndef __AWA_GDT_H 
#define __AWA_GDT_H 1

// 定义描述符字段的偏移量
#define SD_TYPE(x)              (x << 8)          // 类型字段：设置描述符类型
#define SD_CODE_DATA(x)         (x << 12)         // 代码/数据字段：设置描述符为代码段或数据段
#define SD_DPL(x)               (x << 13)         // 特权级别字段：设置描述符的特权级别（DPL）
#define SD_PRESENT(x)           (x << 15)         // 在场位字段：设置描述符的在场位（Present）
#define SD_AVL(x)               (x << 20)         // 可用位字段：设置描述符的可用位（Available）
#define SD_64BITS(x)            (x << 21)         // 64 位标志字段：设置描述符的 64 位标志
#define SD_32BITS(x)            (x << 22)         // 32 位标志字段：设置描述符的 32 位标志
#define SD_4K_GRAN(x)           (x << 23)         // 4 KB 页面大小标志字段：设置描述符的页面大小标志

// 定义段限制字段
#define SEG_LIM_L(x)            (x & 0x0ffff)     // 段限制低 16 位：设置段限制的低 16 位
#define SEG_LIM_H(x)            (x & 0xf0000)     // 段限制高 16 位：设置段限制的高 16 位
#define SEG_BASE_L(x)           ((x & 0x0000ffff) << 16)  // 段基址低 16 位：设置段基址的低 16 位
#define SEG_BASE_M(x)           ((x & 0x00ff0000) >> 16)  // 段基址中间 16 位：设置段基址的中间 16 位
#define SEG_BASE_H(x)           (x & 0xff000000)  // 段基址高 16 位：设置段基址的高 16 位

// 定义不同的段类型
#define SEG_DATA_RD         0x00 // 只读：定义只读数据段
#define SEG_DATA_RDA        0x01 // 只读，已访问：定义只读数据段，已访问标志
#define SEG_DATA_RDWR       0x02 // 读写：定义读写数据段
#define SEG_DATA_RDWRA      0x03 // 读写，已访问：定义读写数据段，已访问标志
#define SEG_DATA_RDEXPD     0x04 // 只读，向下扩展：定义只读数据段，向下扩展
#define SEG_DATA_RDEXPDA    0x05 // 只读，向下扩展，已访问：定义只读数据段，向下扩展，已访问标志
#define SEG_DATA_RDWREXPD   0x06 // 读写，向下扩展：定义读写数据段，向下扩展
#define SEG_DATA_RDWREXPDA  0x07 // 读写，向下扩展，已访问：定义读写数据段，向下扩展，已访问标志
#define SEG_CODE_EX         0x08 // 执行：定义执行代码段
#define SEG_CODE_EXA        0x09 // 执行，已访问：定义执行代码段，已访问标志
#define SEG_CODE_EXRD       0x0A // 执行/读取：定义执行/读取代码段
#define SEG_CODE_EXRDA      0x0B // 执行/读取，已访问：定义执行/读取代码段，已访问标志
#define SEG_CODE_EXC        0x0C // 执行，符合：定义执行代码段，符合标志
#define SEG_CODE_EXCA       0x0D // 执行，符合，已访问：定义执行代码段，符合标志，已访问标志
#define SEG_CODE_EXRDC      0x0E // 执行/读取，符合：定义执行/读取代码段，符合标志
#define SEG_CODE_EXRDCA     0x0F // 执行/读取，符合，已访问：定义执行/读取代码段，符合标志，已访问标志

// 内核代码段描述符
#define SEG_R0_CODE         SD_TYPE(SEG_CODE_EXRD) | SD_CODE_DATA(1) | SD_DPL(0) | \
                            SD_PRESENT(1) | SD_AVL(0) | SD_64BITS(0) | SD_32BITS(1) | \
                            SD_4K_GRAN(1) // 内核代码段描述符：可执行/读取，最高特权级别（Ring 0），32 位，4 KB 页面大小

// 内核数据段描述符
#define SEG_R0_DATA         SD_TYPE(SEG_DATA_RDWR) | SD_CODE_DATA(1) | SD_DPL(0) | \
                            SD_PRESENT(1) | SD_AVL(0) | SD_64BITS(0) | SD_32BITS(1) | \
                            SD_4K_GRAN(1) // 内核数据段描述符：读写，最高特权级别（Ring 0），32 位，4 KB 页面大小

// 用户代码段描述符
#define SEG_R3_CODE         SD_TYPE(SEG_CODE_EXRD) | SD_CODE_DATA(1) | SD_DPL(3) | \
                            SD_PRESENT(1) | SD_AVL(0) | SD_64BITS(0) | SD_32BITS(1) | \
                            SD_4K_GRAN(1) // 用户代码段描述符：可执行/读取，最低特权级别（Ring 3），32 位，4 KB 页面大小

// 用户数据段描述符
#define SEG_R3_DATA         SD_TYPE(SEG_DATA_RDWR) | SD_CODE_DATA(1) | SD_DPL(3) | \
                            SD_PRESENT(1) | SD_AVL(0) | SD_64BITS(0) | SD_32BITS(1) | \
                            SD_4K_GRAN(1) // 用户数据段描述符：读写，最低特权级别（Ring 3），32 位，4 KB 页面大小

/*
 * SEG_R0_DATA段为0x10
 * 代表gdt中的索引 “2”  十进制为16 根据公式(n代表索引) n * 8 + rpl
 *
 * 因为 0x10(十六进制) = 16(十进制) = 2 * 8 + 0(rpl)
 * 所以 0x10 对应于 SEG_R0_DATA 描述符
 *
 * 所以movw %0x10, %cx
 *
 * rpl为低3位，代表RPL( ring privilege level )，也就是权限级别。
 * 0 代表内核权限，也就是最高权限
 * 即 000 为内核权限 011(十进制为3) 为用户权限
 *
 * 如 索引1 权限0 十六进制数则等于 1 * 8 + 0 = 8 = 0x08
 *    索引1 权限1 十六进制数则等于 1 * 8 + 1 = 9 = 0x09
 *    索引2 权限2 十六进制数则等于 2 * 8 + 2 = 18 = 0x12
 *    索引3 权限3 十六进制数则等于 3 * 8 + 3 = 27 = 0x1B
*/ 
void _init_gdt(); // 初始化 GDT 的函数，设置 GDT 中的描述符，并加载 GDT 到处理器

#endif

#include <libc/string.h>
int memcmp(const void* dest, const void* src, size_t num)
{
    const uint8_t* dest_ptr = (const uint8_t*)dest;
    const uint8_t* src_ptr = (const uint8_t*)src;
    for (size_t i = 0; i < num; i++)
    {
        int diff = (int)*(dest_ptr + i) - (int)*(src_ptr + i);
        if (diff != 0)
        {
            return diff;
        } 
    }
    return 0; 
}

// 可能不够完整，不够安全?
void* memcpy(void* dest, const void* src, size_t num)
{
    uint8_t* dest_ptr = (uint8_t*)dest;
    const uint8_t* src_ptr = (const uint8_t*)src;
    if (((uintptr_t) dest & 0x3) == 0 && ((uintptr_t) src & 0x3) == 0) // 对齐
    {
        while (num >= 4)
        {
            *(dest_ptr++) = *(src_ptr++);
            *(dest_ptr++) = *(src_ptr++);
            *(dest_ptr++) = *(src_ptr++);
            *(dest_ptr++) = *(src_ptr++);
            num -= 4;
        }
        
    }
    
    while (num--)
    {
        *(dest_ptr++) = *(src_ptr++);
    }
    return dest;
}
void* memmove(void* dest, const void* src, size_t num)
{
    uint8_t* dest_ptr = (uint8_t*)dest;
    const uint8_t* src_ptr = (const uint8_t*)src;
    
    if (dest_ptr == src_ptr)
    {
        return dest;
    }

    //dest_ptr < (src_ptr + num)判断是否重叠
    if (dest_ptr > src_ptr && dest_ptr < (src_ptr + num))
    {
        dest_ptr += num - 1;
        src_ptr += num - 1;
        if (((uintptr_t)dest_ptr & 0x3) == 0 && ((uintptr_t)src_ptr & 0x3) == 0)
        {
            // 如果指针对齐，则逐块复制
            while (num >= 4)
            {
                *dest_ptr-- = *src_ptr--;
                *dest_ptr-- = *src_ptr--;
                *dest_ptr-- = *src_ptr--;
                *dest_ptr-- = *src_ptr--;
                num -= 4;
            }
        }

        while (num--)
        {
            *(dest_ptr--) = *(src_ptr--);
        } 
    } else {
        if (((uintptr_t)dest_ptr & 0x3) == 0 && ((uintptr_t)src_ptr & 0x3) == 0)
        {
            // 如果指针对齐，则逐块复制
            while (num >= 4)
            {
                *dest_ptr++ = *src_ptr++;
                *dest_ptr++ = *src_ptr++;
                *dest_ptr++ = *src_ptr++;
                *dest_ptr++ = *src_ptr++;
                num -= 4;
            }
        }

        while (num--)
        {
            *(dest_ptr++) = *(src_ptr++);
        } 
    }
    return dest;
}
void* memset(void* ptr, int value, size_t num)
{
    if (num == 0)
    {
        return ptr;
    }
    
    uint8_t* c_ptr = (uint8_t*)ptr;
    for (size_t i = 0; i < num; i++)
    {
        *(c_ptr + i) = (uint8_t)value;
    }
    return ptr;
}
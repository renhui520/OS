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
void* memcpy(void* dest, const void* src, size_t num)
{
    uint8_t* dest_ptr = (uint8_t*)dest;
    const uint8_t* src_ptr = (const uint8_t*)src;
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
        while (num--)
        {
            *(dest_ptr--) = *(src_ptr--);
        } 
    } else {
        while (num--)
        {
            *(dest_ptr++) = *(src_ptr++);
        } 
    }
    return dest;
}
void* memset(void* ptr, int value, size_t num)
{
    uint8_t* c_ptr = (uint8_t*)ptr;
    for (size_t i = 0; i < num; i++)
    {
        *(c_ptr + i) = (uint8_t)value;
    }
    return ptr;
}
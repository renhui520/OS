#ifndef __OS_SPIKE_H
#define __OS_SPIKE_H

// Some helper functions. As helpful as Spike the Dragon! :)

// 除法向上取整
#define CEIL(v, k)          (((v) + (1 << (k)) - 1) >> (k))

// 除法向下取整
#define FLOOR(v, k)         ((v) >> (k))

// 向上 获取v最近的最大k倍数
#define ROUNDUP(v, k)       (((v) + (k) - 1) & ~((k) - 1))

// 向下 获取v最近的最小k倍数
#define ROUNDDOWN(v, k)     ((v) & ~((k) - 1))

// 忙等待，用于暂停，或者用于节省计算机资源
inline static void spin() {
    while(1);
}

#ifdef __AWAOS_DEBUG__      //判断是否在 debug 模式下
#define assert(cond)                                  \
    if (!(cond)) {                                    \
        __assert_fail(#cond, __FILE__, __LINE__);     \
    }

#define assert_msg(cond, msg)                         \
    if (!(cond)) {                                    \
        __assert_fail(msg, __FILE__, __LINE__);   \
    }
void __assert_fail(const char* expr, const char* file, unsigned int line) __attribute__((noinline, noreturn));
#else
#define assert(cond) (void)(cond); //assert nothing 没卵用
#define assert_msg(cond, msg) (void)(cond);  //assert nothing
#endif

// void panick(const char* msg);   //输出报错信息


#define wait_until(cond)    while(!(cond));
#define loop_until(cond)    while(!(cond));


#endif
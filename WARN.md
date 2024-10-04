## 代码规范

### 头文件规范
```c
#ifndef __OS_XX_H__
#define __OS_XX_H__

#include <C标准库>

#define 宏
#define 宏

struct a
{

};

type func(type* var_a, type* var_b);

// 这里面不存在任何一个变量！
// 不允许使用typedef struct！这样影响可读性！

#endif
```

### 主文件规范
```c
#include <自身头文件>

#include <相关系统头文件>

#define 宏定义

const type const_var = n;

static type static_var = n;

type var = n;

// 仅在当前文件中被调用则用 static
static type only_here_use_func(type* var_a, type* var_b)
{
    type* a = var_a;
    type* b = var_b;
    return somthing;
}

// head 头文件中被 声明的函数
type func(type* var_a, type* var_b)
{
    type* a = var_a;
    type* b = var_b;
    return somthing;
}

```

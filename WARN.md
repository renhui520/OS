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

type func(type param);

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

static type only_here_use_func(type param)
{

}

type func(type param)
{

}

```

# TODO List

## 待实现功能
- [X] vmm虚拟内存管理
- [X] malloc
- [ ] apic
- [ ] Linux功能
- [ ] 线程管理


## 待增加功能
- [ ] 增加 DEBUG 调试功能
- [ ] 需要增加日志记录，以便调试。
- [ ] VBE图形显示

## 待解决问题
- [ ] 重审dmm的代码
- [X] sprintf 转换存在大问题，会错误将数字转换为特殊符号!!!
- [ ] 将所有typedef struct给取消！
- [ ] cpu.h和io.h中内容不完善，仍需不断学习并跟进

## 问题记录
  * itoa算法上存在问题，对于十六进制等会出现不同问题...我还得修改和优化:(
  * 不禁用 PIC 会导致操作系统不断重启

## 待学习内容
- [ ] 学习multiboot结构体的使用
- [ ] 中断实现仍不懂，目前仅仅只会使用
- [ ] 了解 asm();
- [ ] 了解和学习如何使用 VBE 利用multiboot结构体所返回的 VBE 信息 实现VBE

## 待优化
- [ ] 调整代码结构，优化逻辑，减少嵌套等...
- [ ] hhk中的分页部分 待优化, 代码逻辑混乱 晦涩难懂 ^#^'

## 其他任务
- [ ] 添加代码注释，提高代码可读性。
- [ ] 优化内存分配算法，提高性能。

## 其他注意事项
- [X] 代码规范问题，各类型转换之间尽量保证一致
- [X] 变量命名仍需调整
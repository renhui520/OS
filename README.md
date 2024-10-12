### 这是一个自制的OS操作系统
#### 边学习边制做的 QAQ
  * 运行前提！已经安装好： gcc编译器 gdb调试工具(建议用: pwndbg) ld工具 make指令 qemu虚拟机 grub相关工具 等
  * 如果没有，请用如下指令安装：
  ```shell
  sudo apt install gcc
  sudo apt install make
  sudo apt install qemu-system
  sudo apt install xorriso
  sudo apt install mtools
  sudo apt install grub-pc-bin
  ```

#### 运行方法：
  * 直接运行:
  ```shell
  make run
  ```

  * debug调试:
  ```shell
  make debug-qemu
  ```

  * debug调试(vscode):
  ```shell
  make debug-qemu-vscode
  ```

  * 快速运行(-O2优化，弱代码规范检查):
  ```shell
  make fast-run
  ```

  * 快速调试(-O2优化，弱代码规范检查):
  ```shell
  make quickly-debug
  ```

  * 快速运行 并 启用测试版功能:
  ```shell
  make test
  ```

  * 快速调试 并 启用测试版功能:
  ```shell
  make test-debug
  ```

  * 编译全部:
  ```shell
  make all
  ```

  * 仅生成内核.bin文件:
  ```shell
  make only-kernel
  ```

  * 清理所有编译出的文件:
  ```shell
  make clean
  ```

#### 警告!
  * 目前大多数功能并未完善

### 可能存在的问题：
  * 可能存在内存段越界的问题?(导致OS重启)
  * 中断程序 不完善，可能会存在逻辑错误 2024.9.5, 00:31
  * itoa存在对于十进制和十六进制外的转换问题...暂且能用了 2024.9.16, 21:12

### 已知存在的问题：
  * sprintf(itoa)可转换数字大小有限，可能造成溢出 2024.9.5, 00:31

### 更新：
  * 项目已重构 2024.8.29, 23:10
  * 使用multiboot + grub引导 2024.9.5, 18:52
  * 新增tty 2024.9.5, 18:52
  * string.h 部分 的 C库代码 2024.9.5, 18:52
  * 实现sprintf 2024.9.7, 00:27
  * 实现kprintf 2024.9.7, 00:27
  * 配置IDT中断表 2024.9.7, 00:27
  * 配置并实现系统中断 2024.9.7, 00:27
  * pmm(Physical Memory Manager 物理内存管理) 2024.9.7, 21:01
  * 优化了mem相关函数，提高处理大块内存的性能 2024.9.11, 23:00
  * vmm(Virtual Memory Manager 虚拟内存管理) 2024.9.16, 20:47
  * 暂且修复了itoa的bug... 2024.9.16, 21:18
  * malloc  2024.10.4, 23:55
  * 修复 idt 导致的重启bug 2024.10.12 21:24

### 待实现功能：
  * apic
  * 线程管理
  * Linux功能
  * VBE图形化显示模式 (取代VGA)(可能要延后很久了...)


* 这是一个自制的OS操作系统
** 边学习编制做的
  1. 运行前提！已经安装好： gcc编译器 nasm汇编器 make指令 qemu虚拟机    如果没有，请用如下指令安装：
  ```
  sudo apt install gcc
  sudo apt install make
  sudo apt install qemu-system
  ```
  2. 要运行需要先创建如下三个文件夹：out, img, bin
  3. 输入make默认生成img以及所有附加文件
  ```bash
  make
  ```
  4. 输入make run可以直接运行
  ```
  make run
  ```
  5. 输入make only_img可以只生成需要的 .img 文件
  ```
  make only_img
  ```
  6. 输入make clean用于清理掉所有生成文件
  ```
  make clean
  ```


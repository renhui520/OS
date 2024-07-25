### 这是一个自制的OS操作系统
#### 边学习边制做的 QAQ
  * 运行前提！已经安装好： gcc编译器 nasm汇编器 make指令 qemu虚拟机    如果没有，请用如下指令安装：
  ```
  sudo apt install gcc
  sudo apt install make
  sudo apt install qemu-system
  ```
  * 要运行需要先创建如下三个文件夹：out, img, bin
  * 在终端(bash)输入make默认生成img以及所有附加文件
  ```
  make
  ```
  * 输入make run可以直接运行
  ```
  make run
  ```
  * 输入make only_img可以只生成需要的 .img 文件
  ```
  make only_img
  ```
  * 输入make clean用于清理掉所有生成文件
  ```
  make clean
  ```
#### 警告!
  * 目前大多数功能并未完善


### 新功能 测试：
  * 初始化段描述符，进入32位保护模式
  * 开启分区表(分页)

### 未来预计加入功能：
  * 导入C语言




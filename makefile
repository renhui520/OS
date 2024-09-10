# 编译器
CC=gcc
AS=as

# debug工具
QEMU_MON_TERM := gnome-terminal
OBJDUMP := objdump
OBJCOPY := objcopy

# debug-qemu端口
QEMU_MON_PORT := 45454

# 链接脚本
LINK=link/linker.ld

# 架构参数
ARCH_OPT := -D__ARCH_IA32
# 测试模式参数
TEST_OPT := -D__VBE__
# 编译选项
O := -O0
W := -Wall -Werror -Wextra -Wno-unknown-pragmas -Wpointer-arith -Wredundant-decls
DEBUG := -g -fno-omit-frame-pointer
SAFE :=	-D_FORTIFY_SOURCE=2
NO :=  -no-pie -fno-pie

# 编译规则和选项
CFLAGS := -m32 -std=gnu99 -ffreestanding $(O) $(W) $(ARCH_OPT) $(NO) $(SAFE)
ASFLAGS := -m32 -ffreestanding $(NO) $(O) $(SAFE) 
LDFLAGS := -m32 -ffreestanding $(NO) $(O) $(SAFE) -nostdlib -Wl,--build-id=none

# 目标文件夹
BUILD_DIR := build
OBJECT_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin
ISO_DIR := $(BUILD_DIR)/iso
ISO_BOOT_DIR := $(ISO_DIR)/boot
ISO_GRUB_DIR := $(ISO_BOOT_DIR)/grub

# 内核名称
OS_NAME = OS
OS_ARCH = x86
OS_BIN = $(OS_NAME).bin
OS_ISO = $(OS_NAME).iso

# 导入头文件
INCLUDES_DIR := include
INCLUDES := $(patsubst %, -I%, $(INCLUDES_DIR))

# 源文件
SOURCE_FILES := $(shell find -name "*.[cS]")
SRC_O := $(patsubst ./%, $(OBJECT_DIR)/%.o, $(SOURCE_FILES))

# qemu 端口
QEMU_MON_PORT := 45454

default: 
	@echo "Default config is running"

# 创建目标文件夹
$(OBJECT_DIR):
	@mkdir -p $(OBJECT_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

$(ISO_DIR):
	@mkdir -p $(ISO_DIR)
	@mkdir -p $(ISO_BOOT_DIR)
	@mkdir -p $(ISO_GRUB_DIR)

# 编译 汇编 文件
$(OBJECT_DIR)/%.S.o: %.S
	@mkdir -p $(@D)
	@echo " BUILD: $<"
	@$(CC) $(INCLUDES) $(ASFLAGS) -c $< -o $@

# 编译 C 文件
$(OBJECT_DIR)/%.c.o: %.c 
	@mkdir -p $(@D)
	@echo " BUILD: $<"
	@$(CC) $(INCLUDES) -c $< -o $@ $(CFLAGS)

# 链接 生成bin 内核文件
$(BIN_DIR)/$(OS_BIN): $(OBJECT_DIR) $(BIN_DIR) $(SRC_O)
	@echo " LINK: $(BIN_DIR)/$(OS_BIN)"
	@$(CC) -T link/linker.ld -o $(BIN_DIR)/$(OS_BIN) $(SRC_O) $(LDFLAGS)

# 生成 iso 文件
$(BUILD_DIR)/$(OS_ISO): $(ISO_DIR) $(BIN_DIR)/$(OS_BIN) GRUB_TEMPLATE
	@./config-grub.sh ${OS_NAME} $(ISO_GRUB_DIR)/grub.cfg
	@cp $(BIN_DIR)/$(OS_BIN) $(ISO_BOOT_DIR)
	@grub-mkrescue -o $(BUILD_DIR)/$(OS_ISO) $(ISO_DIR)

all: clean $(BUILD_DIR)/$(OS_ISO)

Debug:
	@$(OBJCOPY) --only-keep-debug $(BIN_DIR)/$(OS_BIN) $(BUILD_DIR)/kernel.dbg
	@qemu-system-i386 -m 1G -rtc base=utc -s -S -cdrom $(BUILD_DIR)/$(OS_ISO) -monitor telnet::$(QEMU_MON_PORT),server,nowait &
	@sleep 1
	@$(QEMU_MON_TERM) -- telnet 127.0.0.1 $(QEMU_MON_PORT)
	@gdb -s $(BUILD_DIR)/kernel.dbg -ex "target remote localhost:1234"


# 快速运行 不考虑任何问题 提高优化程度
fast-run: O := -O2
fast-run: W := -Wall -Wextra
fast-run: CFLAGS := -m32 -std=gnu99 -ffreestanding $(O) $(W) $(NO)
fast-run: ASFLAGS := -m32 -ffreestanding $(O) $(NO)
fast-run: LDFLAGS := -m32 -ffreestanding $(O) $(NO) -nostdlib -Wl,--build-id=none
fast-run: clean $(BUILD_DIR)/$(OS_ISO)
fast-run: run


# 快速 Debug OS 跳过严谨的代码检测
quick-debug: O := -O0
quick-debug: W := -Wall -Wextra
quick-debug: CFLAGS := -m32 -std=gnu99 -ffreestanding $(O) $(W) $(DEBUG) $(NO) -D__OS_DEBUG__
quick-debug: ASFLAGS := -m32 -ffreestanding $(O) $(DEBUG) $(NO)
quick-debug: LDFLAGS := -m32 -ffreestanding $(O) $(NO) -nostdlib -Wl,--build-id=none
quick-debug: clean $(BUILD_DIR)/$(OS_ISO)
	@echo "Dumping the disassembled kernel code to $(BUILD_DIR)/kdump.txt"
	@$(OBJDUMP) -S $(BIN_DIR)/$(OS_BIN) > $(BUILD_DIR)/kdump.txt


# 以 测试模式 运行 内核
test: O := -O2
test: W := -Wall -Wextra
test: CFLAGS := -m32 -std=gnu99 -ffreestanding $(O) $(W) $(NO) $(TEST_OPT)
test: ASFLAGS := -m32 -ffreestanding $(O) $(NO) $(TEST_OPT)
test: LDFLAGS := -m32 -ffreestanding $(O) $(NO) -nostdlib -Wl,--build-id=none $(TEST_OPT)
test: clean $(BUILD_DIR)/$(OS_ISO)
test: run

# 以 测试模式 Debug OS
test-debug: O := -O0
test-debug: W := -Wall -Wextra
test-debug: CFLAGS := -m32 -std=gnu99 -ffreestanding $(O) $(W) $(DEBUG) $(NO) -D__OS_DEBUG__ $(TEST_OPT)
test-debug: ASFLAGS := -m32 -ffreestanding $(O) $(DEBUG) $(NO) $(TEST_OPT)
test-debug: LDFLAGS := -m32 -ffreestanding $(O) $(NO) -nostdlib -Wl,--build-id=none $(TEST_OPT)
test-debug: clean $(BUILD_DIR)/$(OS_ISO) Debug
	@echo "Dumping the disassembled kernel code to $(BUILD_DIR)/kdump.txt"
	@$(OBJDUMP) -S $(BIN_DIR)/$(OS_BIN) > $(BUILD_DIR)/kdump.txt


# Debug OS
all-debug: O := -O0
all-debug: CFLAGS := -m32 -std=gnu99 -ffreestanding $(O) $(W) $(DEBUG) $(SAFE) $(NO) -D__OS_DEBUG__
all-debug: ASFLAGS := -m32 -ffreestanding $(O) $(DEBUG) $(SAFE) $(NO)
all-debug: LDFLAGS := -m32 -ffreestanding $(O) $(DEBUG) $(SAFE) $(NO) -nostdlib -Wl,--build-id=none

all-debug: clean $(BUILD_DIR)/$(OS_ISO)
	@echo "Dumping the disassembled kernel code to $(BUILD_DIR)/kdump.txt"
	@$(OBJDUMP) -S $(BIN_DIR)/$(OS_BIN) > $(BUILD_DIR)/kdump.txt

# 仅生成 kernel.bin
only-kernel: $(BIN_DIR)/$(OS_BIN)

# 运行 但 不调试
run: clean $(BUILD_DIR)/$(OS_ISO)
	@qemu-system-i386 -cdrom $(BUILD_DIR)/$(OS_ISO) -monitor telnet::$(QEMU_MON_PORT),server,nowait &
	@sleep 1
	@telnet 127.0.0.1 $(QEMU_MON_PORT)

quickly-debug: quick-debug
	@$(OBJCOPY) --only-keep-debug $(BIN_DIR)/$(OS_BIN) $(BUILD_DIR)/kernel.dbg
	@qemu-system-i386 -m 1G -rtc base=utc -s -S -cdrom $(BUILD_DIR)/$(OS_ISO) -monitor telnet::$(QEMU_MON_PORT),server,nowait &
	@sleep 1
	@$(QEMU_MON_TERM) -- telnet 127.0.0.1 $(QEMU_MON_PORT)
	@gdb -s $(BUILD_DIR)/kernel.dbg -ex "target remote localhost:1234"


# 调试 内核
debug-qemu: all-debug
	@$(OBJCOPY) --only-keep-debug $(BIN_DIR)/$(OS_BIN) $(BUILD_DIR)/kernel.dbg
	@qemu-system-i386 -m 1G -rtc base=utc -s -S -cdrom $(BUILD_DIR)/$(OS_ISO) -monitor telnet::$(QEMU_MON_PORT),server,nowait &
	@sleep 1
	@$(QEMU_MON_TERM) -- telnet 127.0.0.1 $(QEMU_MON_PORT)
	@gdb -s $(BUILD_DIR)/kernel.dbg -ex "target remote localhost:1234"

# vscode 调试 内核
debug-qemu-vscode: all-debug
	@$(OBJCOPY) --only-keep-debug $(BIN_DIR)/$(OS_BIN) $(BUILD_DIR)/kernel.dbg
	@qemu-system-i386 -s -S -m 1G -cdrom $(BUILD_DIR)/$(OS_ISO) -monitor telnet::$(QEMU_MON_PORT),server,nowait &
	@sleep 0.5
	@telnet 127.0.0.1 $(QEMU_MON_PORT)


# 清除生成的文件
clean:
	@rm -rf $(BUILD_DIR)
	@sleep 1

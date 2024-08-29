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

# 编译选项
ARCH_OPT := -D__ARCH_IA32
O := -O2
W := -Wall -Wextra -Wno-unknown-pragmas

# 编译规则和选项
CFLAGS := -m32 -std=gnu99 -ffreestanding $(O) $(W) $(ARCH_OPT) -no-pie -fno-pie
ASFLAGS := -m32 -no-pie -fno-pie
LDFLAGS := -m32 -no-pie -fno-pie -ffreestanding $(O) -nostdlib -Wl,--build-id=none

# 目标文件夹
BUILD_DIR := build
OBJECT_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin
ISO_DIR := $(BUILD_DIR)/iso
ISO_BOOT_DIR := $(ISO_DIR)/boot
ISO_GRUB_DIR := $(ISO_BOOT_DIR)/grub

# 内核名称
OS_NAME = HenRuiOS
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

# Debug OS
all-debug: O := -O0
all-debug: CFLAGS := -m32 -g -std=gnu99 -ffreestanding $(O) $(W) $(ARCH_OPT) -no-pie -fno-pie
	#-D__LUNAIXOS_DEBUG__
all-debug: LDFLAGS := -m32 -g -ffreestanding $(O) -nostdlib -no-pie -fno-pie -Wl,--build-id=none

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

# 调试 内核
debug-qemu: all-debug
	@$(OBJCOPY) --only-keep-debug $(BIN_DIR)/$(OS_BIN) $(BUILD_DIR)/kernel.dbg
	@qemu-system-i386 -m 1G -rtc base=utc -s -S -cdrom $(BUILD_DIR)/$(OS_ISO) -monitor telnet::$(QEMU_MON_PORT),server,nowait &
	@sleep 1
	@$(QEMU_MON_TERM) -- telnet 127.0.0.1 $(QEMU_MON_PORT)
	@gdb -s $(BUILD_DIR)/kernel.dbg -ex "target remote localhost:1234"

# 清除生成的文件
clean:
	@rm -rf $(BUILD_DIR)
	@sleep 1

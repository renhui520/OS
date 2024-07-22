CC=gcc
AS=nasm
LD=ld
OBJCOPY=objcopy
CFLAGS=-m16 -nostdlib -nostartfiles -nodefaultlibs -ffreestanding
LDFLAGS=-Ttext=0x1500 -melf_i386 -nostdlib
OF=$(IMG)bootloader.img

OUT=./out/
BIN=./bin/
IMG=./img/

default: clean install

run: $(IMG)bootloader.img
	qemu-system-i386 $(IMG)bootloader.img
	make clean

install: build write

build: mbr.bin loader.bin kernel.bin

mbr.bin: mbr.asm
	$(AS) -f bin $< -o $(BIN)$@

loader.bin: loader.asm
	$(AS) -f bin $< -o $(BIN)$@

kernel.bin: kernel.elf
	$(OBJCOPY) -O binary $(OUT)$< $(BIN)$@

kernel.elf: kernel.o print.o
	$(LD) -s $(patsubst %,$(OUT)%,$^) $(LDFLAGS) -o $(OUT)$@

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c $< -o $(OUT)$@

print.o: print.asm
	$(AS) $< -f elf32 -o $(OUT)$@

write:
	dd if=/dev/zero of=$(OF) bs=1 count=14400
	dd if=$(BIN)mbr.bin of=$(OF) bs=512 count=1 conv=notrunc
	dd if=$(BIN)loader.bin of=$(OF) bs=512 count=1 seek=2 conv=notrunc
	dd if=$(BIN)kernel.bin of=$(OF) bs=512 count=24 seek=9 conv=notrunc

only_img:clean install
	rm -f ./bin/*.bin
	rm -f ./out/*.o
	rm -f ./out/kernel.elf

clean:
	rm -f ./bin/*.bin
	rm -f ./out/*.o
	rm -f ./out/kernel.elf
	rm -f ./img/bootloader.img

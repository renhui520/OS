default: clean install

run:bootloader.img
	qemu-system-i386 bootloader.img
	make clean

img:clean bootloader.img
	rm -f mbr.bin
	rm -f loader.bin

bootloader.img: install

install: build write

build: mbr.bin loader.bin kernel.bin

mbr.bin: mbr.asm
	nasm -f bin mbr.asm -o mbr.bin

loader.bin: loader.asm
	nasm -f bin loader.asm -o loader.bin

kernel.bin: kernel.asm
	nasm -f bin kernel.asm -o kernel.bin

write:
	dd if=/dev/zero of=bootloader.img bs=1M count=10
	dd if=mbr.bin of=bootloader.img bs=512 count=1
	dd if=loader.bin of=bootloader.img bs=512 count=1 seek=2
	dd if=kernel.bin of=bootloader.img bs=512 count=1 seek=9


clean:
	rm -f mbr.bin
	rm -f loader.bin
	rm -f kernel.bin
	rm -f bootloader.img

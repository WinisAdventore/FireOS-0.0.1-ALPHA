#!/bin/bash

echo "=== FireOS 0.0.1 ALPHA Build ==="

# Компиляция bootloader
echo "[1/4] Assembling bootloader..."
nasm -f elf32 src/boot/kernel_entry.asm -o kernel_entry.o

# Компиляция ядра
echo "[2/4] Compiling kernel..."
gcc -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
    -nostartfiles -nodefaultlibs -fno-pic -fno-pie \
    -std=gnu99 -O0 -ffreestanding \
    -c src/kernel/kernel_test.c -o kernel_test.o

# Линковка
echo "[3/4] Linking..."
ld -m elf_i386 -T linker.ld kernel_entry.o kernel_test.o -o kernel_test.elf

# Создание ISO
echo "[4/4] Creating ISO..."
mkdir -p hdd_kernel/boot/grub
cp kernel_test.elf hdd_kernel/boot/

cat > hdd_kernel/boot/grub/grub.cfg << GRUBEOF
set timeout=0
menuentry "FireOS 0.0.1 ALPHA" {
    multiboot2 /boot/kernel_test.elf
    boot
}
GRUBEOF

grub-mkrescue -o fireos_boot.iso hdd_kernel/ 2>/dev/null

echo ""
echo "=== Build complete! ==="
echo "Run: qemu-system-x86_64 -cdrom fireos_boot.iso -drive file=disk.img,format=raw -boot d -m 512"

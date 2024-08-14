# Makefile

# Komut dosyası ve dosya isimleri
BOOTLOADER_SRC = boot.asm
KERNEL1_SRC = kernel1.asm
KERNEL_C_SRC = kernel.c
KERNEL1_BIN = kernel1.bin
KERNEL_C_BIN = kernel.bin
BOOTLOADER_BIN = boot.bin
OS_IMAGE = os-image

# Derleyiciler ve bayraklar
ASM = nasm
ASM_FLAGS = -f bin
GCC = gcc
GCC_FLAGS = -ffreestanding -m32 -nostdlib
LD = gcc
LD_FLAGS = -m32 -nostdlib -T kernel.ld

# Hedefler
all: $(OS_IMAGE)

# Bootloader derlemesi
$(BOOTLOADER_BIN): $(BOOTLOADER_SRC)
	$(ASM) $(ASM_FLAGS) -o $@ $<

# Assembly kernel derlemesi
$(KERNEL1_BIN): $(KERNEL1_SRC)
	$(ASM) $(ASM_FLAGS) -o $@ $<

# C kernel derlemesi
$(KERNEL_C_BIN): $(KERNEL_C_SRC) kernel.ld
	$(GCC) $(GCC_FLAGS) -c $< -o kernel.o
	$(LD) $(LD_FLAGS) -o $@ kernel.o

# Bootloader ve kernel'leri birleştirme
$(OS_IMAGE): $(BOOTLOADER_BIN) $(KERNEL1_BIN) $(KERNEL_C_BIN)
	cat $(BOOTLOADER_BIN) $(KERNEL1_BIN) $(KERNEL_C_BIN) > $@

# Temizlik
clean:
	rm -f $(BOOTLOADER_BIN) $(KERNEL1_BIN) $(KERNEL_C_BIN) kernel.o $(OS_IMAGE)

# Çalıştırma
run: $(OS_IMAGE)
	qemu-system-x86_64 -drive format=raw,file=$(OS_IMAGE)

.PHONY: all clean run

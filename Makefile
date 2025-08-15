# FerryBoot Makefile

# Compiler and tools
CC = gcc
AS = nasm
LD = ld

# Directories
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

# Flags
CFLAGS = -Wall -Wextra -std=c11 -ffreestanding -fno-stack-protector -m32
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -nostdlib

# Targets
BIOS_TARGET = $(BIN_DIR)/ferryboot_bios.bin
UEFI_TARGET = $(BIN_DIR)/ferryboot_uefi.efi

# Source files
BIOS_SOURCES = $(SRC_DIR)/bios/stage1.asm $(SRC_DIR)/bios/stage2.c
BIOS_ASM_SOURCES = $(SRC_DIR)/bios/interrupts.asm
COMMON_SOURCES = $(SRC_DIR)/common/config.c $(SRC_DIR)/common/memory.c $(SRC_DIR)/common/module.c $(SRC_DIR)/common/string.c $(SRC_DIR)/common/module_loader.c
BIOS_COMMON_SOURCES = $(SRC_DIR)/common/ui.c
UEFI_COMMON_SOURCES = $(SRC_DIR)/uefi/ui.c $(SRC_DIR)/uefi/gui.c
BIOS_HAL_SOURCES = $(SRC_DIR)/bios/hal.c
UEFI_HAL_SOURCES = $(SRC_DIR)/uefi/hal.c
CRYPTO_SOURCES = $(SRC_DIR)/common/crypto/sha256.c $(SRC_DIR)/common/crypto/hmac_sha256.c $(SRC_DIR)/common/crypto/pbkdf2.c $(SRC_DIR)/common/crypto/rsa.c $(SRC_DIR)/common/crypto/aes.c
MODULE_SOURCES = $(SRC_DIR)/modules/fs/fat32.c $(SRC_DIR)/modules/fs/ext4.c $(SRC_DIR)/modules/security/security.c $(SRC_DIR)/modules/network/pxe.c

# Default target
.PHONY: all bios uefi clean

all: bios uefi

# BIOS build
bios: $(BIOS_TARGET)

$(BIOS_TARGET): $(BUILD_DIR)/stage1.bin $(BUILD_DIR)/stage2.bin
	@mkdir -p $(BIN_DIR)
	cat $^ > $@
	
$(BUILD_DIR)/stage1.bin: $(SRC_DIR)/bios/stage1.asm
	@mkdir -p $(BUILD_DIR)
	$(AS) $< -o $@

$(BUILD_DIR)/stage2.bin: $(SRC_DIR)/bios/stage2.c $(COMMON_SOURCES) $(BIOS_COMMON_SOURCES) $(BIOS_HAL_SOURCES) $(BIOS_ASM_SOURCES) $(CRYPTO_SOURCES) $(MODULE_SOURCES)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $(BUILD_DIR)/stage2.o
	$(CC) $(CFLAGS) -c $(SRC_DIR)/common/config.c -o $(BUILD_DIR)/config.o
	$(CC) $(CFLAGS) -c $(SRC_DIR)/common/memory.c -o $(BUILD_DIR)/memory.o
	$(CC) $(CFLAGS) -c $(SRC_DIR)/common/module.c -o $(BUILD_DIR)/module.o
	$(CC) $(CFLAGS) -c $(SRC_DIR)/common/module_loader.c -o $(BUILD_DIR)/module_loader.o
	$(CC) $(CFLAGS) -c $(SRC_DIR)/common/string.c -o $(BUILD_DIR)/string.o
	$(CC) $(CFLAGS) -c $(SRC_DIR)/common/ui.c -o $(BUILD_DIR)/ui.o
	$(CC) $(CFLAGS) -c $(SRC_DIR)/bios/hal.c -o $(BUILD_DIR)/bios_hal.o
	$(AS) $(ASFLAGS) $(SRC_DIR)/bios/interrupts.asm -o $(BUILD_DIR)/interrupts.o
	$(CC) $(CFLAGS) -c $(SRC_DIR)/common/crypto/sha256.c -o $(BUILD_DIR)/sha256.o
	$(CC) $(CFLAGS) -c $(SRC_DIR)/common/crypto/hmac_sha256.c -o $(BUILD_DIR)/hmac_sha256.o
	$(CC) $(CFLAGS) -c $(SRC_DIR)/common/crypto/pbkdf2.c -o $(BUILD_DIR)/pbkdf2.o
	$(CC) $(CFLAGS) -c $(SRC_DIR)/common/crypto/rsa.c -o $(BUILD_DIR)/rsa.o
	$(CC) $(CFLAGS) -c $(SRC_DIR)/common/crypto/aes.c -o $(BUILD_DIR)/aes.o
	$(CC) $(CFLAGS) -c $(SRC_DIR)/modules/fs/fat32.c -o $(BUILD_DIR)/fat32.o
	$(CC) $(CFLAGS) -c $(SRC_DIR)/modules/fs/ext4.c -o $(BUILD_DIR)/ext4.o
	$(CC) $(CFLAGS) -c $(SRC_DIR)/modules/security/security.c -o $(BUILD_DIR)/security.o
	$(CC) $(CFLAGS) -c $(SRC_DIR)/modules/network/pxe.c -o $(BUILD_DIR)/pxe.o
	$(LD) $(LDFLAGS) -T stage2.ld $(BUILD_DIR)/stage2.o $(BUILD_DIR)/config.o $(BUILD_DIR)/memory.o $(BUILD_DIR)/module.o $(BUILD_DIR)/module_loader.o $(BUILD_DIR)/string.o $(BUILD_DIR)/ui.o $(BUILD_DIR)/bios_hal.o $(BUILD_DIR)/interrupts.o $(BUILD_DIR)/sha256.o $(BUILD_DIR)/hmac_sha256.o $(BUILD_DIR)/pbkdf2.o $(BUILD_DIR)/rsa.o $(BUILD_DIR)/aes.o $(BUILD_DIR)/fat32.o $(BUILD_DIR)/ext4.o $(BUILD_DIR)/security.o $(BUILD_DIR)/pxe.o -o $@

# UEFI build
uefi: $(UEFI_TARGET)

$(UEFI_TARGET): $(SRC_DIR)/uefi/main.c $(COMMON_SOURCES) $(UEFI_COMMON_SOURCES) $(UEFI_HAL_SOURCES) $(CRYPTO_SOURCES) $(MODULE_SOURCES)
	@mkdir -p $(BIN_DIR)
	# UEFI compilation would go here
	# This is a placeholder - actual UEFI compilation requires special tools

# Clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
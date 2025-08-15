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
COMMON_SOURCES = $(SRC_DIR)/common/config.c $(SRC_DIR)/common/memory.c $(SRC_DIR)/common/module.c $(SRC_DIR)/common/string.c $(SRC_DIR)/common/ui.c
UEFI_SOURCES = $(SRC_DIR)/uefi/main.c $(SRC_DIR)/uefi/hal.c
BIOS_HAL_SOURCES = $(SRC_DIR)/bios/hal.c

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

$(BUILD_DIR)/stage2.bin: $(SRC_DIR)/bios/stage2.c $(COMMON_SOURCES) $(BIOS_HAL_SOURCES)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $(BUILD_DIR)/stage2.o
	$(CC) $(CFLAGS) -c $(SRC_DIR)/common/config.c -o $(BUILD_DIR)/config.o
	$(CC) $(CFLAGS) -c $(SRC_DIR)/common/memory.c -o $(BUILD_DIR)/memory.o
	$(CC) $(CFLAGS) -c $(SRC_DIR)/common/module.c -o $(BUILD_DIR)/module.o
	$(CC) $(CFLAGS) -c $(SRC_DIR)/common/string.c -o $(BUILD_DIR)/string.o
	$(CC) $(CFLAGS) -c $(SRC_DIR)/common/ui.c -o $(BUILD_DIR)/ui.o
	$(CC) $(CFLAGS) -c $(SRC_DIR)/bios/hal.c -o $(BUILD_DIR)/bios_hal.o
	$(LD) $(LDFLAGS) -T stage2.ld $(BUILD_DIR)/stage2.o $(BUILD_DIR)/config.o $(BUILD_DIR)/memory.o $(BUILD_DIR)/module.o $(BUILD_DIR)/string.o $(BUILD_DIR)/ui.o $(BUILD_DIR)/bios_hal.o -o $@

# UEFI build
uefi: $(UEFI_TARGET)

$(UEFI_TARGET): $(UEFI_SOURCES) $(COMMON_SOURCES)
	@mkdir -p $(BIN_DIR)
	# UEFI compilation would go here
	# This is a placeholder - actual UEFI compilation requires special tools

# Clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
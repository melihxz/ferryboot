# Compiler and flags
CC = gcc
AS = nasm
CFLAGS = -Wall -O2 -Iinclude -m32
ASFLAGS = -f bin
LDFLAGS = -m32 -nostartfiles -nodefaultlibs -o

# Directories
SRC_DIR = src
OBJ_DIR = obj
INCLUDE_DIR = include

# Source files
C_SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
C_OBJ_FILES = $(C_SRC_FILES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
ASM_SRC_FILES = $(wildcard $(SRC_DIR)/*.asm)
ASM_OBJ_FILES = $(ASM_SRC_FILES:$(SRC_DIR)/%.asm=$(OBJ_DIR)/%.bin)

# Target binary
TARGET = bootloader

# Default target
all: $(TARGET)

# Link target binary
$(TARGET): $(C_OBJ_FILES)
	$(CC) $(LDFLAGS) $@ $(C_OBJ_FILES)

# Compile C source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

# Assemble assembly source files
$(OBJ_DIR)/%.bin: $(SRC_DIR)/%.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -o $@ $<

# Clean build artifacts
clean:
	rm -f $(TARGET)
	rm -rf $(OBJ_DIR)

.PHONY: all clean

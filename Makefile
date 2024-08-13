# Makefile for building Ferryboot

# Compiler and flags
CC = gcc
AS = nasm
LD = ld
CFLAGS = -Wall -m32 -nostdlib -nostartfiles
LDFLAGS = -m32 -nostdlib -nostartfiles

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Files
TARGET = bootloader
SRC = $(SRC_DIR)/boot.c
OBJ = $(OBJ_DIR)/boot.o
BIN = $(BIN_DIR)/$(TARGET)

# Default target
all: $(BIN)

# Compile C source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link object files to create the binary
$(BIN): $(OBJ)
	@mkdir -p $(BIN_DIR)
	$(LD) $(LDFLAGS) -o $@ $^

# Clean up build artifacts
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Rebuild everything
rebuild: clean all

.PHONY: all clean rebuild

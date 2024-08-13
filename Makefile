# Makefile for Ferryboot

# Compiler and tools
CC = gcc
ASM = nasm
LD = ld
OBJDUMP = objdump

# Flags
CFLAGS = -ffreestanding -nostdlib -nostartfiles -m16 -Wall
ASMFLAGS = -f bin
LDFLAGS = -T linker.ld

# Source files
C_SRC = boot.c
ASM_SRC = boot.asm
OBJ = boot.o

# Output file
OUTPUT = boot.bin

all: $(OUTPUT)

# Compile the C source file
$(OBJ): $(C_SRC)
	$(CC) $(CFLAGS) -c $(C_SRC) -o $(OBJ)

# Assemble the assembly source file
$(OUTPUT): $(ASM_SRC) $(OBJ)
	$(ASM) $(ASMFLAGS) $(ASM_SRC) -o $(OBJ)
	$(LD) $(LDFLAGS) $(OBJ) -o $(OUTPUT)

# Clean up the build files
clean:
	rm -f $(OBJ) $(OUTPUT)

# Dump the binary file
dump: $(OUTPUT)
	$(OBJDUMP) -D $(OUTPUT)

.PHONY: all clean dump

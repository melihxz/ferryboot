# FerryBoot Architecture

## Overview

FerryBoot is a cross-platform bootloader designed to work on both modern UEFI systems and legacy BIOS systems. It follows a modular architecture to provide flexibility and extensibility.

## Components

### 1. Stage 1 Bootloader (BIOS)

The Stage 1 bootloader is a small assembly program that:

1. Initializes the processor to a known state
2. Sets up basic memory segments
3. Loads the Stage 2 bootloader from disk
4. Jumps to the Stage 2 entry point

**File**: `src/bios/stage1.asm`
**Size**: 512 bytes (one sector)
**Load Address**: 0x7C00

### 2. Stage 2 Bootloader (BIOS)

The Stage 2 bootloader is the main bootloader written in C that:

1. Initializes hardware (VGA, keyboard, etc.)
2. Detects available hardware
3. Loads configuration from disk
4. Displays boot menu
5. Loads and executes selected OS

**File**: `src/bios/stage2.c`
**Load Address**: 0x7E00

### 3. UEFI Bootloader

The UEFI bootloader is written in C using the UEFI Application Programming Interface:

1. Uses UEFI services for hardware access
2. Leverages UEFI protocols for filesystem access
3. Supports UEFI Secure Boot
4. Provides graphical interface using UEFI GOP

**File**: `src/uefi/main.c`

## Module System

FerryBoot supports a module system for extending functionality:

1. **Filesystem Modules**: Support for different filesystems (FAT32, ext4, NTFS, etc.)
2. **Hardware Modules**: Support for different hardware (NVMe, USB, network cards, etc.)
3. **Compression Modules**: Support for compressed kernels
4. **Security Modules**: Encryption, signature verification

## Memory Layout (BIOS)

```
0x00000000 +------------------+
           |      BIOS        |
           |   Data Areas     |
0x00007C00 +------------------+
           | Stage 1 Bootloader|
0x00007E00 +------------------+
           | Stage 2 Bootloader|
           |   and Modules    |
0x00080000 +------------------+
           |   Kernel Load    |
           |     Area         |
0x00100000 +------------------+
           |  Protected Mode  |
           |   Kernel Entry   |
0x00200000 +------------------+
```

## Build System

FerryBoot uses a Makefile-based build system:

1. `make bios` - Build BIOS version
2. `make uefi` - Build UEFI version
3. `make all` - Build both versions
4. `make clean` - Clean build artifacts

## Testing

FerryBoot can be tested using:

1. **QEMU** - For emulating both BIOS and UEFI systems
2. **VirtualBox** - For testing on virtual machines
3. **Physical Hardware** - For real-world testing

## Security Features

1. **Password Protection** - BIOS and UEFI password support
2. **Kernel Signature Verification** - Verify signed kernels
3. **Disk Encryption** - Support for LUKS, BitLocker, VeraCrypt
4. **Secure Boot** - UEFI Secure Boot compatibility
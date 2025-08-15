# FerryBoot Testing Guide

## Overview

This guide explains how to test FerryBoot on different platforms and environments.

## Testing Environments

### 1. QEMU (Recommended)

QEMU is the primary testing platform for FerryBoot development.

#### BIOS Testing

```bash
# Build BIOS version
make bios

# Test with QEMU
qemu-system-x86_64 -drive format=raw,file=bin/ferryboot_bios.bin -m 512
```

#### UEFI Testing

```bash
# Build UEFI version
make uefi

# Test with QEMU (OVMF required)
qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -drive format=raw,file=bin/ferryboot_uefi.efi -m 512
```

### 2. VirtualBox

VirtualBox can be used for additional testing.

1. Create a new VM
2. Attach the FerryBoot image as a disk
3. Configure boot order
4. Start the VM

### 3. Physical Hardware

Testing on real hardware is essential for compatibility verification.

1. Create a bootable USB drive with FerryBoot
2. Test on different hardware configurations
3. Verify compatibility with various disk layouts (MBR/GPT)
4. Test with different operating systems

## Automated Testing

FerryBoot includes a test suite for verifying functionality:

```bash
# Run unit tests
make test

# Run integration tests
make integration-test

# Run hardware compatibility tests
make hw-test
```

## Test Cases

### Basic Functionality

1. Bootloader loads successfully
2. Hardware detection works
3. Configuration loads correctly
4. Menu displays properly
5. OS loading works

### Security Features

1. Password protection
2. Signature verification
3. Encryption support
4. Secure Boot compatibility

### Hardware Compatibility

1. Different disk controllers (IDE, SATA, NVMe)
2. Various graphics cards
3. Multiple input devices
4. Network boot (PXE)

### Filesystem Support

1. FAT32 (boot partition)
2. ext4/ext3/ext2
3. NTFS
4. XFS
5. Btrfs

## Debugging

### Serial Debug Output

FerryBoot supports serial port debugging:

```bash
# Enable serial debugging
qemu-system-x86_64 -drive format=raw,file=bin/ferryboot_bios.bin -serial stdio
```

### QEMU Monitor

Use QEMU monitor for advanced debugging:

```bash
# Start QEMU with monitor
qemu-system-x86_64 -drive format=raw,file=bin/ferryboot_bios.bin -monitor stdio
```

## Continuous Integration

FerryBoot uses CI for automated testing:

1. GitHub Actions for pull requests
2. Automated builds for all platforms
3. Regression testing
4. Code quality checks

## Reporting Issues

When reporting issues, include:

1. Platform (BIOS/UEFI)
2. Hardware configuration
3. FerryBoot version
4. Steps to reproduce
5. Expected vs actual behavior
6. Debug logs if available
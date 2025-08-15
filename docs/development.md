# FerryBoot Development Guide

## Overview

This guide provides information for developers who want to contribute to or extend FerryBoot.

## Project Structure

```
ferryboot/
├── bin/                 # Compiled binaries
├── build/               # Build artifacts
├── design/              # Design documents
├── docs/                # Documentation
├── src/                 # Source code
│   ├── bios/            # BIOS-specific code
│   │   ├── stage1.asm   # Stage 1 bootloader
│   │   ├── stage2.c     # Stage 2 bootloader
│   │   └── ...          # BIOS modules
│   ├── common/          # Shared code
│   │   ├── config.h     # Configuration system
│   │   ├── hal.h        # Hardware abstraction
│   │   ├── memory.h     # Memory management
│   │   ├── module.h     # Module system
│   │   ├── string.h     # String utilities
│   │   └── ui.h         # User interface
│   ├── uefi/            # UEFI-specific code
│   │   ├── main.c       # UEFI entry point
│   │   └── ...          # UEFI modules
│   └── modules/         # Loadable modules (future)
│       ├── fs/          # Filesystem modules
│       ├── hw/          # Hardware modules
│       ├── security/    # Security modules
│       └── ...          # Other modules
├── tests/               # Unit and integration tests (future)
├── tools/               # Development tools (future)
├── Makefile             # Build system
└── README.md            # Project overview
```

## Development Environment

### Required Tools

1. **Compiler**:
   - GCC (for BIOS targets)
   - Clang (for UEFI targets)
   - NASM (for assembly code)

2. **Build Tools**:
   - Make
   - CMake (for complex builds)

3. **Emulation**:
   - QEMU (primary testing platform)
   - VirtualBox (secondary testing)
   - OVMF (for UEFI testing)

4. **Debugging**:
   - GDB (with QEMU)
   - Bochs debugger
   - Serial console

### Setting Up Development Environment

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential nasm qemu-system-x86 ovmf gcc-efi
```

#### Fedora
```bash
sudo dnf install gcc nasm qemu-kvm edk2-ovmf
```

#### macOS
```bash
brew install gcc nasm qemu
```

## Build System

### Makefile Targets

```bash
# Build all targets
make all

# Build BIOS version
make bios

# Build UEFI version
make uefi

# Clean build artifacts
make clean

# Run tests (future)
make test

# Generate documentation (future)
make docs
```

### Cross-Compilation

FerryBoot supports cross-compilation for different architectures:

```bash
# Build for x86
make ARCH=x86

# Build for x86_64
make ARCH=x86_64

# Build for ARM
make ARCH=arm
```

## Coding Standards

### Language Guidelines

#### C Code
- Follow C11 standard
- Use `-Wall -Wextra -Werror` flags
- Avoid undefined behavior
- Use static analysis tools

#### Assembly Code
- Use NASM syntax
- Document all functions
- Follow calling conventions
- Include safety checks

### Code Organization

#### File Naming
- Use lowercase with underscores
- Use descriptive names
- Follow module structure

#### Header Files
```c
// Include guards
#ifndef FERRYBOOT_MODULE_H
#define FERRYBOOT_MODULE_H

// System headers first
#include <stdint.h>
#include <stdbool.h>

// Project headers
#include "common/types.h"

// Public interface
int module_initialize(void);
void module_cleanup(void);

#endif // FERRYBOOT_MODULE_H
```

#### Source Files
```c
#include "module.h"

// Private functions
static int internal_function(void);

// Public functions
int module_initialize(void) {
    // Implementation
    return 0;
}

void module_cleanup(void) {
    // Implementation
}

// Private function implementation
static int internal_function(void) {
    // Implementation
    return 0;
}
```

### Memory Management

#### Allocation
```c
// Use provided memory functions
void* malloc(size_t size);
void free(void* ptr);
void* realloc(void* ptr, size_t size);

// Zero-initialized allocation
void* calloc(size_t nmemb, size_t size);
```

#### Safety
- Always check return values
- Avoid memory leaks
- Use bounded string functions
- Validate pointers

## Hardware Abstraction

### Hardware Interface

FerryBoot uses a hardware abstraction layer:

```c
// Hardware interface
typedef struct {
    // Storage functions
    int (*read_sector)(uint64_t lba, void* buffer, size_t count);
    int (*write_sector)(uint64_t lba, const void* buffer, size_t count);
    
    // Display functions
    void (*set_video_mode)(uint32_t width, uint32_t height, uint32_t depth);
    void (*put_pixel)(uint32_t x, uint32_t y, uint32_t color);
    
    // Input functions
    bool (*key_pressed)(void);
    uint32_t (*get_key)(void);
    
    // Timer functions
    uint64_t (*get_ticks)(void);
    void (*sleep)(uint32_t milliseconds);
} hardware_interface_t;
```

## Current Development Status

### Phase 1: Foundation (Complete)
- [x] Stage 1 bootloader (assembly)
- [x] Stage 2 bootloader (C)
- [x] Basic HAL implementation
- [x] Configuration system
- [x] Makefile-based build system

### Phase 2: UEFI Support (In Progress)
- [ ] UEFI application framework
- [ ] Secure Boot implementation
- [ ] Graphical interface (VGA/VESA)
- [ ] Mouse input support

### Phase 3: Module System (Planned)
- [ ] Module loading framework
- [ ] Filesystem modules (5+ formats)
- [ ] Hardware modules (storage, input, network)
- [ ] Compression modules (3+ formats)

See our detailed [Development Roadmap](development.md) for complete plans.

## Testing

### QEMU Testing

```bash
# Test BIOS mode
qemu-system-x86_64 -drive format=raw,file=bin/ferryboot_bios.bin -m 512

# Test UEFI mode (when implemented)
qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -drive format=raw,file=bin/ferryboot_uefi.efi -m 512
```

## Debugging

### QEMU Debugging

```bash
# Run with debug output
qemu-system-x86_64 -drive format=raw,file=bin/ferryboot_bios.bin -serial stdio

# Run with GDB (when implemented)
qemu-system-x86_64 -drive format=raw,file=bin/ferryboot_bios.bin -s -S
```

## Getting Help

### Community Resources

- GitHub Issues: Bug reports and feature requests
- GitHub Discussions: General discussion and help
- Documentation: Comprehensive guides and API reference

### Maintainer Guidelines

For maintainers:
- Review pull requests promptly
- Follow release process
- Update documentation
- Communicate breaking changes
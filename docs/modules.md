# FerryBoot Module System

## Overview

FerryBoot uses a modular architecture to allow extending functionality without modifying the core bootloader. Modules are loaded dynamically at runtime based on system requirements.

## Module Types

### 1. Filesystem Modules

Handle different filesystem formats:
- FAT32 (built-in for boot partition)
- ext4/ext3/ext2
- NTFS
- XFS
- Btrfs

### 2. Hardware Modules

Support different hardware devices:
- Storage controllers (IDE, SATA, NVMe)
- Network adapters (PXE boot)
- Graphics cards (VGA, VESA, UEFI GOP)
- Input devices (PS/2, USB HID)

### 3. Compression Modules

Handle compressed kernels:
- gzip
- bzip2
- lzma
- xz

### 4. Security Modules

Provide security features:
- LUKS encryption
- BitLocker support
- Signature verification
- TPM integration

## Module Interface

All modules must implement a standard interface:

```c
// Module header structure
typedef struct {
    uint32_t magic;          // Module magic number
    uint32_t version;        // Module version
    uint32_t type;           // Module type
    uint32_t flags;          // Module flags
    char name[32];           // Module name
    char description[128];   // Module description
    uint32_t entry_point;    // Module entry point
    uint32_t data_size;      // Size of module data
} module_header_t;

// Module initialization function
typedef int (*module_init_func)(void);

// Module cleanup function
typedef void (*module_cleanup_func)(void);

// Module-specific functions
typedef struct {
    module_init_func init;
    module_cleanup_func cleanup;
    // Additional function pointers based on module type
} module_interface_t;
```

## Module Loading Process

1. **Detection**: Bootloader detects hardware and determines required modules
2. **Loading**: Modules are loaded from disk into memory
3. **Verification**: Module signature is verified (if security is enabled)
4. **Initialization**: Module initialization function is called
5. **Registration**: Module registers its capabilities with the system
6. **Usage**: Bootloader uses module functions as needed

## Module Storage

Modules are stored in a dedicated partition or directory:
- BIOS systems: `/boot/ferryboot/modules/`
- UEFI systems: `/EFI/FERRYBOOT/modules/`

Each module is a separate file with a `.mod` extension.

## Example Module Implementation

```c
// Example filesystem module
#include "module.h"

static int fs_init(void) {
    // Initialize filesystem driver
    return 0;
}

static void fs_cleanup(void) {
    // Cleanup filesystem driver
}

static int fs_mount(const char* device) {
    // Mount filesystem
    return 0;
}

static int fs_read(const char* path, void* buffer, size_t size) {
    // Read file from filesystem
    return 0;
}

// Module interface
module_interface_t filesystem_module = {
    .init = fs_init,
    .cleanup = fs_cleanup,
    // Additional filesystem-specific functions
};

// Module entry point
int module_main(module_header_t* header) {
    // Verify module header
    if (header->magic != MODULE_MAGIC) {
        return -1;
    }
    
    // Register module
    return register_module(header, &filesystem_module);
}
```

## Benefits

1. **Flexibility**: Add support for new hardware/filesystems without core changes
2. **Size**: Only load required modules, reducing memory footprint
3. **Maintainability**: Isolate functionality in separate modules
4. **Security**: Verify module signatures before loading
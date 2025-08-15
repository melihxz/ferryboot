# FerryBoot Module Development Guide

## 1. Overview

FerryBoot uses a modular architecture to provide extensibility while maintaining a small core footprint. This guide explains how to develop modules for FerryBoot.

## 2. Module Types

FerryBoot supports several types of modules:

1. **Filesystem Modules** - Support for different filesystem formats
2. **Hardware Modules** - Support for different hardware devices
3. **Compression Modules** - Support for compressed kernels
4. **Security Modules** - Security-related functionality
5. **Network Modules** - Network protocol support

## 3. Module Structure

All modules must follow a specific structure:

```
module/
├── module.h          # Module header file
├── module.c          # Module implementation
├── Makefile          # Build instructions
└── README.md         # Module documentation
```

## 4. Module Header

Each module must define a header with metadata:

```c
// module.h
#ifndef FERRYBOOT_MODULE_H
#define FERRYBOOT_MODULE_H

#include "common/module.h"

// Module-specific interface
typedef struct {
    module_interface_t base;
    // Module-specific function pointers
    int (*custom_function)(void);
    // ... other functions
} custom_module_t;

#endif // FERRYBOOT_MODULE_H
```

## 5. Module Implementation

### 5.1 Basic Module Structure

```c
// module.c
#include "module.h"
#include "common/hal.h"
#include "common/debug.h"

// Module metadata
static const module_header_t module_header = {
    .magic = MODULE_MAGIC,
    .version = 1,
    .type = MODULE_TYPE_CUSTOM,
    .flags = 0,
    .name = "Custom Module",
    .description = "Example module implementation"
};

// Forward declarations
static int module_init(void);
static void module_cleanup(void);
static int custom_function(void);

// Module interface
static custom_module_t module_interface = {
    .base = {
        .init = module_init,
        .cleanup = module_cleanup
    },
    .custom_function = custom_function
};

// Module initialization
static int module_init(void) {
    debug_print("Initializing custom module\n");
    
    // Perform initialization tasks
    // ...
    
    return 0;
}

// Module cleanup
static void module_cleanup(void) {
    debug_print("Cleaning up custom module\n");
    
    // Perform cleanup tasks
    // ...
}

// Custom function implementation
static int custom_function(void) {
    debug_print("Executing custom function\n");
    
    // Implement module functionality
    // ...
    
    return 0;
}

// Module entry point
int module_main(module_header_t* header) {
    // Verify header
    if (header->magic != MODULE_MAGIC) {
        return -1;
    }
    
    // Copy header
    *header = module_header;
    
    // Return interface
    return (int)&module_interface;
}
```

## 6. Filesystem Module Example

### 6.1 Filesystem Module Header

```c
// fs_module.h
#ifndef FERRYBOOT_FS_MODULE_H
#define FERRYBOOT_FS_MODULE_H

#include "common/module.h"

// Filesystem operations
typedef struct {
    int (*mount)(const char* device);
    int (*unmount)(void);
    int (*read_file)(const char* path, void* buffer, size_t size);
    int (*write_file)(const char* path, const void* buffer, size_t size);
    int (*list_directory)(const char* path, char** entries, size_t max_entries);
} filesystem_ops_t;

// Filesystem module interface
typedef struct {
    module_interface_t base;
    filesystem_ops_t fs_ops;
} filesystem_module_t;

#endif // FERRYBOOT_FS_MODULE_H
```

### 6.2 Filesystem Module Implementation

```c
// ext4_module.c
#include "fs_module.h"
#include "common/hal.h"
#include "common/debug.h"

// Module metadata
static const module_header_t module_header = {
    .magic = MODULE_MAGIC,
    .version = 1,
    .type = MODULE_TYPE_FILESYSTEM,
    .flags = 0,
    .name = "ext4",
    .description = "ext4 filesystem support"
};

// Forward declarations
static int ext4_init(void);
static void ext4_cleanup(void);
static int ext4_mount(const char* device);
static int ext4_unmount(void);
static int ext4_read_file(const char* path, void* buffer, size_t size);

// Filesystem operations
static filesystem_ops_t ext4_ops = {
    .mount = ext4_mount,
    .unmount = ext4_unmount,
    .read_file = ext4_read_file,
    .write_file = NULL, // Read-only for bootloader
    .list_directory = NULL
};

// Module interface
static filesystem_module_t ext4_module = {
    .base = {
        .init = ext4_init,
        .cleanup = ext4_cleanup
    },
    .fs_ops = ext4_ops
};

// Module initialization
static int ext4_init(void) {
    debug_print("Initializing ext4 module\n");
    return 0;
}

// Module cleanup
static void ext4_cleanup(void) {
    debug_print("Cleaning up ext4 module\n");
    ext4_unmount();
}

// Mount ext4 filesystem
static int ext4_mount(const char* device) {
    debug_print("Mounting ext4 filesystem on %s\n", device);
    
    // Implementation would parse ext4 superblock,
    // read block group descriptors, etc.
    
    return 0;
}

// Unmount ext4 filesystem
static int ext4_unmount(void) {
    debug_print("Unmounting ext4 filesystem\n");
    
    // Implementation would clean up filesystem structures
    
    return 0;
}

// Read file from ext4 filesystem
static int ext4_read_file(const char* path, void* buffer, size_t size) {
    debug_print("Reading file %s from ext4 filesystem\n", path);
    
    // Implementation would traverse directory structure,
    // read inode, read data blocks, etc.
    
    return 0;
}

// Module entry point
int module_main(module_header_t* header) {
    if (header->magic != MODULE_MAGIC) {
        return -1;
    }
    
    *header = module_header;
    return (int)&ext4_module;
}
```

## 7. Hardware Module Example

### 7.1 Hardware Module Header

```c
// nvme_module.h
#ifndef FERRYBOOT_NVME_MODULE_H
#define FERRYBOOT_NVME_MODULE_H

#include "common/module.h"
#include "common/hal.h"

// NVMe device information
typedef struct {
    uint32_t vendor_id;
    uint32_t device_id;
    uint64_t capacity;
    uint32_t sector_size;
    char model[40];
} nvme_device_info_t;

// NVMe operations
typedef struct {
    int (*initialize)(void);
    int (*identify)(nvme_device_info_t* info);
    int (*read_sectors)(uint64_t lba, void* buffer, size_t count);
    int (*write_sectors)(uint64_t lba, const void* buffer, size_t count);
} nvme_ops_t;

// NVMe module interface
typedef struct {
    module_interface_t base;
    nvme_ops_t nvme_ops;
} nvme_module_t;

#endif // FERRYBOOT_NVME_MODULE_H
```

### 7.2 Hardware Module Implementation

```c
// nvme_module.c
#include "nvme_module.h"
#include "common/hal.h"
#include "common/debug.h"
#include "common/pci.h"

// Module metadata
static const module_header_t module_header = {
    .magic = MODULE_MAGIC,
    .version = 1,
    .type = MODULE_TYPE_HARDWARE,
    .flags = 0,
    .name = "nvme",
    .description = "NVMe storage controller support"
};

// Forward declarations
static int nvme_init(void);
static void nvme_cleanup(void);
static int nvme_initialize(void);
static int nvme_identify(nvme_device_info_t* info);
static int nvme_read_sectors(uint64_t lba, void* buffer, size_t count);

// NVMe operations
static nvme_ops_t nvme_ops = {
    .initialize = nvme_initialize,
    .identify = nvme_identify,
    .read_sectors = nvme_read_sectors,
    .write_sectors = NULL // Read-only for bootloader
};

// Module interface
static nvme_module_t nvme_module = {
    .base = {
        .init = nvme_init,
        .cleanup = nvme_cleanup
    },
    .nvme_ops = nvme_ops
};

// Module initialization
static int nvme_init(void) {
    debug_print("Initializing NVMe module\n");
    return 0;
}

// Module cleanup
static void nvme_cleanup(void) {
    debug_print("Cleaning up NVMe module\n");
}

// Initialize NVMe controller
static int nvme_initialize(void) {
    debug_print("Initializing NVMe controller\n");
    
    // Implementation would:
    // 1. Detect NVMe controller via PCI
    // 2. Initialize PCI configuration
    // 3. Reset controller
    // 4. Configure admin queue
    // 5. Identify controller
    
    return 0;
}

// Identify NVMe device
static int nvme_identify(nvme_device_info_t* info) {
    debug_print("Identifying NVMe device\n");
    
    // Implementation would send IDENTIFY command
    // and parse controller/namespace data
    
    return 0;
}

// Read sectors from NVMe device
static int nvme_read_sectors(uint64_t lba, void* buffer, size_t count) {
    debug_print("Reading %d sectors from NVMe at LBA %d\n", count, lba);
    
    // Implementation would:
    // 1. Create and submit read command
    // 2. Wait for completion
    // 3. Handle errors
    
    return 0;
}

// Module entry point
int module_main(module_header_t* header) {
    if (header->magic != MODULE_MAGIC) {
        return -1;
    }
    
    *header = module_header;
    return (int)&nvme_module;
}
```

## 8. Security Module Example

### 8.1 Security Module Header

```c
// luks_module.h
#ifndef FERRYBOOT_LUKS_MODULE_H
#define FERRYBOOT_LUKS_MODULE_H

#include "common/module.h"

// LUKS operations
typedef struct {
    int (*unlock)(const char* device, const char* passphrase);
    int (*verify_header)(const char* device);
    int (*get_keyslots)(const char* device, int* active_slots);
} luks_ops_t;

// LUKS module interface
typedef struct {
    module_interface_t base;
    luks_ops_t luks_ops;
} luks_module_t;

#endif // FERRYBOOT_LUKS_MODULE_H
```

### 8.2 Security Module Implementation

```c
// luks_module.c
#include "luks_module.h"
#include "common/hal.h"
#include "common/debug.h"
#include "common/crypto.h"

// Module metadata
static const module_header_t module_header = {
    .magic = MODULE_MAGIC,
    .version = 1,
    .type = MODULE_TYPE_SECURITY,
    .flags = 0,
    .name = "luks",
    .description = "LUKS disk encryption support"
};

// Forward declarations
static int luks_init(void);
static void luks_cleanup(void);
static int luks_unlock(const char* device, const char* passphrase);
static int luks_verify_header(const char* device);

// LUKS operations
static luks_ops_t luks_ops = {
    .unlock = luks_unlock,
    .verify_header = luks_verify_header,
    .get_keyslots = NULL
};

// Module interface
static luks_module_t luks_module = {
    .base = {
        .init = luks_init,
        .cleanup = luks_cleanup
    },
    .luks_ops = luks_ops
};

// Module initialization
static int luks_init(void) {
    debug_print("Initializing LUKS module\n");
    return 0;
}

// Module cleanup
static void luks_cleanup(void) {
    debug_print("Cleaning up LUKS module\n");
}

// Verify LUKS header
static int luks_verify_header(const char* device) {
    debug_print("Verifying LUKS header on %s\n", device);
    
    // Implementation would:
    // 1. Read LUKS header from device
    // 2. Verify magic number
    // 3. Check header checksum
    // 4. Validate version
    
    return 0;
}

// Unlock LUKS volume
static int luks_unlock(const char* device, const char* passphrase) {
    debug_print("Unlocking LUKS volume %s\n", device);
    
    // Implementation would:
    // 1. Read LUKS header
    // 2. For each active keyslot:
    //    a. Derive key from passphrase using PBKDF2
    //    b. Decrypt master key
    //    c. Verify master key
    // 3. Set up dm-crypt mapping
    
    return 0;
}

// Module entry point
int module_main(module_header_t* header) {
    if (header->magic != MODULE_MAGIC) {
        return -1;
    }
    
    *header = module_header;
    return (int)&luks_module;
}
```

## 9. Module Build System

### 9.1 Module Makefile

```makefile
# Makefile for ext4 module
MODULE_NAME = ext4
MODULE_TYPE = filesystem

# Compiler and tools
CC = gcc
AS = nasm
LD = ld

# Directories
SRC_DIR = .
BUILD_DIR = build
BIN_DIR = bin

# Flags
CFLAGS = -Wall -Wextra -std=c11 -ffreestanding -fno-stack-protector -m32
LDFLAGS = -m elf_i386 -nostdlib

# Source files
SOURCES = $(SRC_DIR)/$(MODULE_NAME)_module.c
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Target
MODULE_TARGET = $(BIN_DIR)/$(MODULE_NAME).mod

# Default target
.PHONY: all clean

all: $(MODULE_TARGET)

# Build module
$(MODULE_TARGET): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(LD) $(LDFLAGS) -r -o $@ $^

# Compile C files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
```

## 10. Module Registration

### 10.1 Registering Modules with the System

```c
// In the main bootloader
#include "modules/ext4/ext4_module.h"
#include "modules/nvme/nvme_module.h"
#include "modules/luks/luks_module.h"

// Register all available modules
int register_all_modules(void) {
    // Register filesystem modules
    module_register(&ext4_module.base);
    
    // Register hardware modules
    module_register(&nvme_module.base);
    
    // Register security modules
    module_register(&luks_module.base);
    
    return 0;
}
```

## 11. Module Loading at Runtime

### 11.1 Dynamic Module Loading

```c
// Load module from file
int load_module_from_file(const char* module_path) {
    // 1. Read module file
    // 2. Verify module header
    // 3. Verify signature (if security enabled)
    // 4. Load module into memory
    // 5. Call module initialization function
    // 6. Register module with system
    
    module_header_t header;
    if (file_read(module_path, &header, sizeof(header)) != sizeof(header)) {
        return ERROR_FILE_READ;
    }
    
    if (header.magic != MODULE_MAGIC) {
        return ERROR_INVALID_MODULE;
    }
    
    if (security_enabled() && !verify_module_signature(&header)) {
        return ERROR_INVALID_SIGNATURE;
    }
    
    void* module_memory = hal_alloc(header.data_size);
    if (!module_memory) {
        return ERROR_OUT_OF_MEMORY;
    }
    
    if (file_read(module_path, module_memory, header.data_size) != header.data_size) {
        hal_free(module_memory);
        return ERROR_FILE_READ;
    }
    
    module_interface_t* interface = (module_interface_t*)
        ((uint8_t*)module_memory + header.entry_point);
    
    if (interface->init) {
        int result = interface->init();
        if (result != 0) {
            hal_free(module_memory);
            return result;
        }
    }
    
    register_module(&header, interface);
    return 0;
}
```

## 12. Module Best Practices

### 12.1 Code Organization

1. **Modular Design**: Keep modules focused on a single responsibility
2. **Error Handling**: Always check return values and handle errors gracefully
3. **Memory Management**: Clean up all allocated memory in cleanup functions
4. **Documentation**: Provide clear documentation for module APIs

### 12.2 Security Considerations

1. **Input Validation**: Always validate input parameters
2. **Buffer Bounds**: Check buffer boundaries to prevent overflow
3. **Signature Verification**: Verify module signatures when security is enabled
4. **Secure Coding**: Follow secure coding practices

### 12.3 Performance Considerations

1. **Efficient Algorithms**: Use efficient algorithms for critical operations
2. **Memory Usage**: Minimize memory usage, especially in constrained environments
3. **Lazy Loading**: Load resources only when needed
4. **Caching**: Cache frequently accessed data when appropriate

## 13. Testing Modules

### 13.1 Unit Testing

```c
// test_ext4.c
#include "modules/ext4/ext4_module.h"
#include "test_framework.h"

void test_ext4_mount(void) {
    // Test mounting a valid ext4 filesystem
    int result = ext4_mount("/dev/sda1");
    TEST_ASSERT_EQUAL(0, result);
}

void test_ext4_read_file(void) {
    // Test reading a file from ext4 filesystem
    char buffer[1024];
    int result = ext4_read_file("/boot/vmlinuz", buffer, sizeof(buffer));
    TEST_ASSERT_EQUAL(0, result);
}

TEST_LIST = {
    { "test_ext4_mount", test_ext4_mount },
    { "test_ext4_read_file", test_ext4_read_file },
    { NULL, NULL }
};
```

### 13.2 Integration Testing

```bash
# Test module with QEMU
qemu-system-x86_64 -drive format=raw,file=test_disk.img -m 512

# Verify module is loaded and functional
# Check debug output for module initialization messages
```

## 14. Module Distribution

### 14.1 Module Packaging

Modules should be packaged as separate files with the `.mod` extension:

```
/boot/ferryboot/modules/
├── ext4.mod
├── ntfs.mod
├── nvme.mod
├── luks.mod
└── ...
```

### 14.2 Module Installation

```bash
# Install module
cp ext4.mod /boot/ferryboot/modules/

# Load module at boot
ferryboot-config --add-module ext4
```

## 15. Conclusion

Modules are a key part of FerryBoot's architecture, providing extensibility while maintaining a small core footprint. By following the guidelines in this document, developers can create robust, secure, and efficient modules that extend FerryBoot's functionality.
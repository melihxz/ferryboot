# FerryBoot Core Implementation Guide

## 1. Overview

This document provides detailed implementation guidance for the core components of FerryBoot, including the boot process, hardware abstraction layer, configuration system, and security features.

## 2. Boot Process Implementation

### 2.1 BIOS Boot Process

#### 2.1.1 Stage 1 Bootloader (512 bytes)

The Stage 1 bootloader is written in assembly and must fit within 512 bytes:

```asm
; FerryBoot Stage 1 - BIOS Bootloader
; Loads stage 2 bootloader from disk

BITS 16
ORG 0x7C00

start:
    ; Initialize segments
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Reset disk system
    mov ah, 0
    mov dl, 0x80
    int 0x13
    jc error

    ; Load stage 2 from disk
    ; CHS: cylinder=0, head=0, sector=2
    mov ah, 0x02    ; Read sectors function
    mov al, 10      ; Number of sectors to read (5KB)
    mov ch, 0       ; Cylinder
    mov cl, 2       ; Sector (1-indexed)
    mov dh, 0       ; Head
    mov dl, 0x80    ; Drive number (first hard disk)
    mov bx, 0x7E00  ; Load address (after boot sector)
    int 0x13
    jc error

    ; Jump to stage 2
    jmp 0x7E00

error:
    ; Print error message
    mov si, error_msg
    call print_string
    jmp $

print_string:
    lodsb
    or al, al
    jz done
    mov ah, 0x0E
    mov bh, 0
    int 0x10
    jmp print_string
done:
    ret

error_msg db 'Error loading stage 2', 0

; Pad to 510 bytes and add boot signature
times 510-($-$$) db 0
dw 0xAA55
```

#### 2.1.2 Stage 2 Bootloader

The Stage 2 bootloader is written in C and provides the main bootloader functionality:

```c
// FerryBoot Stage 2 - Main Bootloader
// Written in C for BIOS systems

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "string.h"
#include "hal.h"
#include "config.h"
#include "ui.h"
#include "modules.h"
#include "security.h"

// VGA text mode dimensions
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_BUFFER 0xB8000

// Global variables
static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

// Function prototypes
void terminal_initialize(void);
void terminal_setcolor(uint8_t color);
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);

// Kernel entry point
void _start(void) {
    // Initialize terminal
    terminal_initialize();
    
    // Print welcome message
    terminal_writestring("FerryBoot v0.1\n");
    terminal_writestring("Initializing bootloader...\n");
    
    // Initialize hardware abstraction layer
    if (hal_init() != 0) {
        terminal_writestring("Error: Failed to initialize HAL\n");
        for (;;);
    }
    
    // Load configuration
    config_t config;
    if (config_load(&config) != 0) {
        terminal_writestring("Warning: Failed to load configuration, using defaults\n");
        config_init_defaults(&config);
    }
    
    // Initialize module system
    if (module_init_all() != 0) {
        terminal_writestring("Error: Failed to initialize modules\n");
        for (;;);
    }
    
    // Initialize security system
    if (security_init() != 0) {
        terminal_writestring("Error: Failed to initialize security system\n");
        for (;;);
    }
    
    // Detect hardware
    if (detect_hardware() != 0) {
        terminal_writestring("Warning: Hardware detection issues\n");
    }
    
    // Detect boot entries
    if (detect_boot_entries(&config) != 0) {
        terminal_writestring("Warning: Failed to detect boot entries\n");
    }
    
    // Show boot menu
    int selected_entry = show_boot_menu(&config);
    if (selected_entry >= 0) {
        // Boot selected OS
        boot_os(&config.entries[selected_entry]);
    }
    
    // Hang indefinitely if boot failed
    terminal_writestring("Error: Boot failed\n");
    for (;;);
}

// Terminal functions
void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = (uint16_t*) VGA_BUFFER;
    
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
}

void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_row = 0;
        }
        return;
    }

    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    
    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_row = 0;
        }
    }
}

void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
    terminal_write(data, strlen(data));
}

// Helper functions
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}
```

### 2.2 UEFI Boot Process

#### 2.2.1 UEFI Application Entry Point

The UEFI bootloader is implemented as a UEFI application:

```c
// FerryBoot UEFI Bootloader
// Entry point for UEFI systems

#include <efi.h>
#include <efilib.h>
#include "uefi_hal.h"
#include "config.h"
#include "ui.h"
#include "modules.h"
#include "security.h"

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    // Initialize UEFI library
    InitializeLib(ImageHandle, SystemTable);
    
    // Print welcome message
    Print(L"FerryBoot v0.1 - UEFI Mode\n");
    Print(L"Initializing bootloader...\n");
    
    // Initialize UEFI hardware abstraction layer
    if (uefi_hal_init(ImageHandle, SystemTable) != 0) {
        Print(L"Error: Failed to initialize UEFI HAL\n");
        return EFI_LOAD_ERROR;
    }
    
    // Load configuration
    config_t config;
    if (config_load(&config) != 0) {
        Print(L"Warning: Failed to load configuration, using defaults\n");
        config_init_defaults(&config);
    }
    
    // Initialize module system
    if (module_init_all() != 0) {
        Print(L"Error: Failed to initialize modules\n");
        return EFI_LOAD_ERROR;
    }
    
    // Initialize security system
    if (security_init() != 0) {
        Print(L"Error: Failed to initialize security system\n");
        return EFI_LOAD_ERROR;
    }
    
    // Detect hardware
    if (uefi_detect_hardware() != 0) {
        Print(L"Warning: Hardware detection issues\n");
    }
    
    // Detect boot entries
    if (detect_boot_entries(&config) != 0) {
        Print(L"Warning: Failed to detect boot entries\n");
    }
    
    // Show boot menu
    int selected_entry = show_boot_menu(&config);
    if (selected_entry >= 0) {
        // Boot selected OS
        return boot_os(&config.entries[selected_entry]);
    }
    
    // Wait for user input before exiting
    Print(L"Press any key to continue...\n");
    WaitForSingleEvent(ST->ConIn->WaitForKey, 0);
    
    return EFI_SUCCESS;
}
```

## 3. Hardware Abstraction Layer Implementation

### 3.1 HAL Interface Implementation

```c
// hal.h - Hardware Abstraction Layer interface
#ifndef FERRYBOOT_HAL_H
#define FERRYBOOT_HAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Hardware interface
typedef struct {
    // Storage functions
    int (*read_sector)(uint64_t lba, void* buffer, size_t count);
    int (*write_sector)(uint64_t lba, const void* buffer, size_t count);
    
    // Display functions
    void (*set_video_mode)(uint32_t width, uint32_t height, uint32_t depth);
    void (*put_pixel)(uint32_t x, uint32_t y, uint32_t color);
    void (*clear_screen)(uint32_t color);
    
    // Input functions
    bool (*key_pressed)(void);
    uint32_t (*get_key)(void);
    bool (*mouse_moved)(void);
    void (*get_mouse_state)(int32_t* x, int32_t* y, uint32_t* buttons);
    
    // Network functions
    int (*network_init)(void);
    int (*dhcp_request)(network_config_t* config);
    int (*tftp_download)(const char* server, const char* filename, void* buffer, size_t* size);
    
    // Timer functions
    uint64_t (*get_ticks)(void);
    void (*sleep)(uint32_t milliseconds);
    
    // Memory functions
    void* (*alloc)(size_t size);
    void (*free)(void* ptr);
} hardware_interface_t;

// Global HAL instance
extern hardware_interface_t* g_hal;

// HAL initialization
int hal_init(void);

// Inline functions for performance
static inline int hal_read_sector(uint64_t lba, void* buffer, size_t count) {
    return g_hal->read_sector(lba, buffer, count);
}

static inline void hal_put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    g_hal->put_pixel(x, y, color);
}

#endif // FERRYBOOT_HAL_H
```

### 3.2 BIOS HAL Implementation

```c
// bios_hal.c - BIOS Hardware Abstraction Layer implementation
#include "hal.h"
#include "bios.h"

// BIOS storage implementation
int bios_read_sector(uint64_t lba, void* buffer, size_t count) {
    // Convert LBA to CHS for BIOS INT 13h
    // Handle extended read for LBA48 if needed
    // Implementation details...
    return 0;
}

int bios_write_sector(uint64_t lba, const void* buffer, size_t count) {
    // Convert LBA to CHS for BIOS INT 13h
    // Handle extended write for LBA48 if needed
    // Implementation details...
    return 0;
}

// BIOS display implementation
void bios_set_video_mode(uint32_t width, uint32_t height, uint32_t depth) {
    // Use VESA BIOS extensions for high-resolution modes
    // Implementation details...
}

void bios_put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    // Write pixel to video memory
    // Implementation details...
}

// BIOS input implementation
bool bios_key_pressed(void) {
    // Check for keypress using BIOS INT 16h
    // Implementation details...
    return false;
}

uint32_t bios_get_key(void) {
    // Get key using BIOS INT 16h
    // Implementation details...
    return 0;
}

// BIOS HAL interface
static hardware_interface_t bios_hal = {
    .read_sector = bios_read_sector,
    .write_sector = bios_write_sector,
    .set_video_mode = bios_set_video_mode,
    .put_pixel = bios_put_pixel,
    .clear_screen = bios_clear_screen,
    .key_pressed = bios_key_pressed,
    .get_key = bios_get_key,
    .mouse_moved = bios_mouse_moved,
    .get_mouse_state = bios_get_mouse_state,
    .network_init = bios_network_init,
    .dhcp_request = bios_dhcp_request,
    .tftp_download = bios_tftp_download,
    .get_ticks = bios_get_ticks,
    .sleep = bios_sleep,
    .alloc = bios_alloc,
    .free = bios_free
};

// Global HAL instance
hardware_interface_t* g_hal = &bios_hal;

// HAL initialization
int hal_init(void) {
    // Initialize BIOS services
    // Detect available hardware
    // Set up function pointers
    return 0;
}
```

### 3.3 UEFI HAL Implementation

```c
// uefi_hal.c - UEFI Hardware Abstraction Layer implementation
#include "hal.h"
#include <efi.h>
#include <efilib.h>

// UEFI globals
static EFI_HANDLE g_image_handle;
static EFI_SYSTEM_TABLE* g_system_table;

// UEFI storage implementation
int uefi_read_sector(uint64_t lba, void* buffer, size_t count) {
    // Use EFI_BLOCK_IO_PROTOCOL to read sectors
    // Implementation details...
    return 0;
}

// UEFI display implementation
void uefi_set_video_mode(uint32_t width, uint32_t height, uint32_t depth) {
    // Use EFI_GRAPHICS_OUTPUT_PROTOCOL to set video mode
    // Implementation details...
}

void uefi_put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    // Write pixel to framebuffer
    // Implementation details...
}

// UEFI HAL interface
static hardware_interface_t uefi_hal = {
    .read_sector = uefi_read_sector,
    .write_sector = uefi_write_sector,
    .set_video_mode = uefi_set_video_mode,
    .put_pixel = uefi_put_pixel,
    .clear_screen = uefi_clear_screen,
    .key_pressed = uefi_key_pressed,
    .get_key = uefi_get_key,
    .mouse_moved = uefi_mouse_moved,
    .get_mouse_state = uefi_get_mouse_state,
    .network_init = uefi_network_init,
    .dhcp_request = uefi_dhcp_request,
    .tftp_download = uefi_tftp_download,
    .get_ticks = uefi_get_ticks,
    .sleep = uefi_sleep,
    .alloc = uefi_alloc,
    .free = uefi_free
};

// Global HAL instance
hardware_interface_t* g_hal = &uefi_hal;

// UEFI HAL initialization
int uefi_hal_init(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* system_table) {
    g_image_handle = image_handle;
    g_system_table = system_table;
    
    // Initialize UEFI protocols
    // Set up function pointers
    return 0;
}
```

## 4. Configuration System Implementation

### 4.1 Configuration Structure

```c
// config.h - Configuration system
#ifndef FERRYBOOT_CONFIG_H
#define FERRYBOOT_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

// Configuration structure
typedef struct {
    uint32_t magic;              // Configuration magic number (0xF33DB007)
    uint32_t version;            // Configuration version
    uint32_t checksum;           // Configuration checksum
    
    // Boot settings
    uint32_t timeout;            // Boot timeout in seconds
    uint32_t default_entry;      // Default boot entry index
    bool quiet_boot;             // Quiet boot mode
    bool debug_mode;             // Debug output enabled
    
    // Display settings
    uint32_t resolution_x;       // Screen width
    uint32_t resolution_y;       // Screen height
    uint32_t color_depth;        // Color depth in bits
    bool gui_mode;               // GUI mode enabled
    char theme[32];              // Theme name
    
    // Security settings
    bool password_protected;     // Password protection enabled
    uint8_t password_hash[32];   // SHA-256 password hash
    bool secure_boot;            // Secure boot enabled
    bool signature_check;        // Kernel signature verification
    
    // Network settings
    bool pxe_boot;               // PXE boot enabled
    uint32_t pxe_timeout;        // PXE timeout in seconds
    
    // Module settings
    uint32_t module_count;       // Number of loaded modules
    char modules[16][32];        // List of module names
    
    // Boot entries
    uint32_t entry_count;        // Number of boot entries
    struct {
        char name[64];           // Entry name
        char path[256];          // Kernel/initrd path
        char parameters[256];    // Boot parameters
        uint32_t type;           // OS type
        bool enabled;            // Entry enabled
    } entries[32];               // Boot entries
    
    // Reserved for future use
    uint8_t reserved[1024];
} config_t;

// Configuration API
int config_load(config_t* config);
int config_save(const config_t* config);
int config_init_defaults(config_t* config);
bool config_validate(const config_t* config);
int config_add_boot_entry(config_t* config, const boot_entry_t* entry);

#endif // FERRYBOOT_CONFIG_H
```

### 4.2 Configuration Implementation

```c
// config.c - Configuration system implementation
#include "config.h"
#include "hal.h"
#include "crypto.h"

// Configuration magic number
#define CONFIG_MAGIC 0xF33DB007
#define CONFIG_VERSION 1

// Calculate configuration checksum
static uint32_t config_checksum(const config_t* config) {
    // Calculate checksum of configuration data
    // Exclude checksum field itself
    const uint32_t* data = (const uint32_t*)config;
    uint32_t checksum = 0;
    
    for (size_t i = 0; i < sizeof(config_t) / sizeof(uint32_t); i++) {
        if (i != offsetof(config_t, checksum) / sizeof(uint32_t)) {
            checksum += data[i];
        }
    }
    
    return checksum;
}

// Load configuration from storage
int config_load(config_t* config) {
    // Try primary storage first
    if (hal_read_sector(CONFIG_PRIMARY_SECTOR, config, 1) == 0) {
        // Verify magic number
        if (config->magic == CONFIG_MAGIC) {
            // Verify checksum
            if (config->checksum == config_checksum(config)) {
                // Verify version
                if (config->version == CONFIG_VERSION) {
                    return 0; // Success
                }
            }
        }
    }
    
    // Try backup storage
    if (hal_read_sector(CONFIG_BACKUP_SECTOR, config, 1) == 0) {
        // Verify magic number
        if (config->magic == CONFIG_MAGIC) {
            // Verify checksum
            if (config->checksum == config_checksum(config)) {
                // Verify version
                if (config->version == CONFIG_VERSION) {
                    // Restore from backup
                    config_save(config);
                    return 0;
                }
            }
        }
    }
    
    // Use defaults
    config_init_defaults(config);
    config_save(config);
    
    return 0;
}

// Save configuration to storage
int config_save(const config_t* config) {
    config_t temp_config = *config;
    
    // Update checksum
    temp_config.checksum = config_checksum(&temp_config);
    
    // Save to primary storage
    if (hal_write_sector(CONFIG_PRIMARY_SECTOR, &temp_config, 1) != 0) {
        return -1;
    }
    
    // Save to backup storage
    if (hal_write_sector(CONFIG_BACKUP_SECTOR, &temp_config, 1) != 0) {
        return -1;
    }
    
    return 0;
}

// Initialize configuration with defaults
int config_init_defaults(config_t* config) {
    // Clear configuration
    memset(config, 0, sizeof(config_t));
    
    // Set default values
    config->magic = CONFIG_MAGIC;
    config->version = CONFIG_VERSION;
    config->timeout = 10;
    config->default_entry = 0;
    config->quiet_boot = false;
    config->debug_mode = false;
    config->resolution_x = 1024;
    config->resolution_y = 768;
    config->color_depth = 32;
    config->gui_mode = true;
    strcpy(config->theme, "default");
    config->password_protected = false;
    config->secure_boot = true;
    config->signature_check = true;
    config->pxe_boot = false;
    config->pxe_timeout = 30;
    config->module_count = 0;
    config->entry_count = 0;
    
    return 0;
}

// Validate configuration
bool config_validate(const config_t* config) {
    // Check magic number
    if (config->magic != CONFIG_MAGIC) {
        return false;
    }
    
    // Check version
    if (config->version != CONFIG_VERSION) {
        return false;
    }
    
    // Check checksum
    if (config->checksum != config_checksum(config)) {
        return false;
    }
    
    // Check bounds
    if (config->timeout > 300) return false; // Max 5 minutes
    if (config->resolution_x > 7680) return false; // Max 8K
    if (config->resolution_y > 4320) return false; // Max 8K
    if (config->color_depth != 16 && config->color_depth != 24 && config->color_depth != 32) {
        return false;
    }
    if (config->module_count > 16) return false;
    if (config->entry_count > 32) return false;
    
    return true;
}

// Add boot entry to configuration
int config_add_boot_entry(config_t* config, const boot_entry_t* entry) {
    if (config->entry_count >= 32) {
        return -1; // Too many entries
    }
    
    config->entries[config->entry_count] = *entry;
    config->entry_count++;
    
    return 0;
}
```

## 5. Security System Implementation

### 5.1 Security Interface

```c
// security.h - Security system
#ifndef FERRYBOOT_SECURITY_H
#define FERRYBOOT_SECURITY_H

#include <stdbool.h>
#include <stdint.h>

// RSA public key
typedef struct {
    uint8_t modulus[256];
    uint8_t exponent[4];
    uint32_t modulus_size;
} rsa_public_key_t;

// Security context
typedef struct {
    // Cryptographic functions
    int (*sha256_hash)(const void* data, size_t len, uint8_t* hash);
    int (*sha512_hash)(const void* data, size_t len, uint8_t* hash);
    int (*rsa_verify)(const uint8_t* signature, size_t sig_len,
                      const uint8_t* data, size_t data_len,
                      const rsa_public_key_t* pubkey);
    int (*aes_decrypt)(const uint8_t* ciphertext, size_t len,
                       const uint8_t* key, size_t key_len,
                       const uint8_t* iv, uint8_t* plaintext);
    
    // Security features
    bool secure_boot_enabled;
    bool signature_verification_enabled;
    bool encryption_enabled;
    
    // Keys and certificates
    rsa_public_key_t* trusted_keys;
    uint32_t key_count;
} security_context_t;

// Global security context
extern security_context_t* g_security;

// Security API
int security_init(void);
int security_verify_boot_image(const char* image_path);
int security_unlock_volume(const char* device, const char* passphrase);
bool security_enabled(void);

#endif // FERRYBOOT_SECURITY_H
```

### 5.2 Security Implementation

```c
// security.c - Security system implementation
#include "security.h"
#include "hal.h"
#include "crypto.h"
#include "config.h"

// Global security context
static security_context_t security_context;
security_context_t* g_security = &security_context;

// Initialize security system
int security_init(void) {
    // Initialize cryptographic functions
    security_context.sha256_hash = crypto_sha256;
    security_context.sha512_hash = crypto_sha512;
    security_context.rsa_verify = crypto_rsa_verify;
    security_context.aes_decrypt = crypto_aes_decrypt;
    
    // Load configuration
    config_t config;
    if (config_load(&config) == 0) {
        security_context.secure_boot_enabled = config.secure_boot;
        security_context.signature_verification_enabled = config.signature_check;
        security_context.encryption_enabled = config.password_protected;
    } else {
        // Use defaults
        security_context.secure_boot_enabled = true;
        security_context.signature_verification_enabled = true;
        security_context.encryption_enabled = false;
    }
    
    // Load trusted keys
    if (load_trusted_keys(&security_context.trusted_keys, 
                         &security_context.key_count) != 0) {
        security_context.key_count = 0;
        security_context.trusted_keys = NULL;
    }
    
    return 0;
}

// Verify boot image using secure boot
int security_verify_boot_image(const char* image_path) {
    if (!security_context.secure_boot_enabled) {
        return 0; // Security disabled
    }
    
    if (!security_context.signature_verification_enabled) {
        return 0; // Signature verification disabled
    }
    
    if (security_context.key_count == 0) {
        return -1; // No trusted keys
    }
    
    // Construct signature path
    char signature_path[512];
    snprintf(signature_path, sizeof(signature_path), "%s.sig", image_path);
    
    // Load image file
    void* image_data = NULL;
    size_t image_size = 0;
    if (file_load(image_path, &image_data, &image_size) != 0) {
        return -1;
    }
    
    // Calculate image hash
    uint8_t image_hash[32];
    if (security_context.sha256_hash(image_data, image_size, image_hash) != 0) {
        hal_free(image_data);
        return -1;
    }
    
    // Load signature
    uint8_t signature[256];
    size_t signature_size = 0;
    if (file_read(signature_path, signature, sizeof(signature)) <= 0) {
        hal_free(image_data);
        return -1;
    }
    
    // Verify signature with each trusted key
    int result = -1;
    for (uint32_t i = 0; i < security_context.key_count; i++) {
        if (security_context.rsa_verify(signature, signature_size,
                                       image_hash, sizeof(image_hash),
                                       &security_context.trusted_keys[i]) == 0) {
            result = 0; // Valid signature found
            break;
        }
    }
    
    // Cleanup
    hal_free(image_data);
    
    return result;
}

// Unlock encrypted volume using passphrase
int security_unlock_volume(const char* device, const char* passphrase) {
    if (!security_context.encryption_enabled) {
        return 0; // Encryption disabled
    }
    
    // Read encryption header
    encryption_header_t header;
    if (hal_read_sector(0, &header, 1) != 0) {
        return -1;
    }
    
    // Validate header
    if (header.magic != ENCRYPTION_MAGIC) {
        return -1;
    }
    
    // Derive key from passphrase
    uint8_t derived_key[32];
    if (pbkdf2_sha256(passphrase, header.salt, header.iterations, 
                      derived_key, sizeof(derived_key)) != 0) {
        return -1;
    }
    
    // Try each key slot
    int result = -1;
    for (int i = 0; i < MAX_KEY_SLOTS; i++) {
        if (header.key_slots[i].active) {
            // Decrypt master key
            uint8_t master_key[32];
            if (security_context.aes_decrypt(header.key_slots[i].encrypted_key,
                                           sizeof(header.key_slots[i].encrypted_key),
                                           derived_key, sizeof(derived_key),
                                           header.key_slots[i].iv, master_key) == 0) {
                // Verify master key
                if (verify_master_key(device, &header, master_key) == 0) {
                    // Unlock volume
                    result = unlock_volume(device, master_key);
                    break;
                }
            }
        }
    }
    
    return result;
}

// Check if security is enabled
bool security_enabled(void) {
    return security_context.secure_boot_enabled || 
           security_context.signature_verification_enabled ||
           security_context.encryption_enabled;
}
```

## 6. User Interface Implementation

### 6.1 UI Interface

```c
// ui.h - User Interface system
#ifndef FERRYBOOT_UI_H
#define FERRYBOOT_UI_H

#include "config.h"

// Input event
typedef struct {
    uint32_t type;      // KEYBOARD, MOUSE, TOUCH
    uint32_t code;      // Key code, button, etc.
    uint32_t x, y;      // Coordinates for mouse/touch
    uint32_t modifiers; // Shift, Ctrl, Alt
} input_event_t;

// Menu item
typedef struct {
    char text[64];
    uint32_t id;
    bool enabled;
    void (*on_select)(void);
} menu_item_t;

// Menu
typedef struct {
    char title[64];
    menu_item_t items[32];
    uint32_t item_count;
    uint32_t selected_item;
} menu_t;

// UI API
int ui_init(bool gui_mode);
int ui_show_boot_menu(const config_t* config);
int ui_show_setup_wizard(config_t* config);
int ui_process_event(const input_event_t* event);
void ui_draw_text(const char* text, uint32_t x, uint32_t y, uint32_t color);
void ui_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);

#endif // FERRYBOOT_UI_H
```

### 6.2 Text UI Implementation

```c
// text_ui.c - Text-based User Interface implementation
#include "ui.h"
#include "hal.h"

// Text mode colors
#define TEXT_BLACK     0
#define TEXT_BLUE      1
#define TEXT_GREEN     2
#define TEXT_CYAN      3
#define TEXT_RED       4
#define TEXT_MAGENTA   5
#define TEXT_BROWN     6
#define TEXT_LIGHTGRAY 7
#define TEXT_DARKGRAY  8
#define TEXT_LIGHTBLUE 9
#define TEXT_LIGHTGREEN 10
#define TEXT_LIGHTCYAN 11
#define TEXT_LIGHTRED  12
#define TEXT_LIGHTMAGENTA 13
#define TEXT_YELLOW    14
#define TEXT_WHITE     15

// VGA text mode dimensions
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_BUFFER 0xB8000

// Global variables
static uint16_t* terminal_buffer;
static uint8_t terminal_color;

// Initialize text UI
int ui_text_init(void) {
    terminal_buffer = (uint16_t*)VGA_BUFFER;
    terminal_color = (TEXT_LIGHTGRAY << 4) | TEXT_BLACK;
    return 0;
}

// Draw character in text mode
void ui_text_draw_char(char ch, uint8_t color, uint8_t x, uint8_t y) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
    
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = (color << 8) | ch;
}

// Draw string in text mode
void ui_text_draw_string(const char* str, uint8_t color, uint8_t x, uint8_t y) {
    if (y >= VGA_HEIGHT) return;
    
    for (size_t i = 0; str[i] != '\0' && (x + i) < VGA_WIDTH; i++) {
        ui_text_draw_char(str[i], color, x + i, y);
    }
}

// Clear screen
void ui_text_clear_screen(void) {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            ui_text_draw_char(' ', terminal_color, x, y);
        }
    }
}

// Show boot menu in text mode
int ui_text_show_boot_menu(const config_t* config) {
    ui_text_clear_screen();
    
    // Draw header
    ui_text_draw_string("FerryBoot v0.1", TEXT_WHITE, 35, 1);
    ui_text_draw_string("Select operating system:", TEXT_LIGHTGRAY, 2, 3);
    
    // Draw menu items
    for (uint32_t i = 0; i < config->entry_count; i++) {
        char menu_line[80];
        snprintf(menu_line, sizeof(menu_line), "%d. %s", i + 1, config->entries[i].name);
        
        uint8_t color = config->entries[i].enabled ? TEXT_WHITE : TEXT_DARKGRAY;
        ui_text_draw_string(menu_line, color, 4, 5 + i);
    }
    
    // Draw footer
    ui_text_draw_string("Use arrow keys to select, Enter to boot", TEXT_LIGHTGRAY, 2, 23);
    
    // Handle input
    uint32_t selected = config->default_entry;
    if (selected >= config->entry_count) selected = 0;
    
    while (1) {
        // Highlight selected item
        for (uint32_t i = 0; i < config->entry_count; i++) {
            uint8_t color = (i == selected) ? TEXT_YELLOW : 
                           (config->entries[i].enabled ? TEXT_WHITE : TEXT_DARKGRAY);
            char menu_line[80];
            snprintf(menu_line, sizeof(menu_line), "%d. %s", i + 1, config->entries[i].name);
            ui_text_draw_string(menu_line, color, 4, 5 + i);
        }
        
        // Wait for input
        while (!hal_key_pressed()) {
            hal_sleep(10);
        }
        
        uint32_t key = hal_get_key();
        switch (key) {
            case 0x48: // Up arrow
                if (selected > 0) selected--;
                break;
            case 0x50: // Down arrow
                if (selected < config->entry_count - 1) selected++;
                break;
            case 0x1C: // Enter
                return selected;
            case 0x01: // Escape
                return -1;
        }
    }
    
    return -1;
}
```

### 6.3 GUI Implementation

```c
// gui_ui.c - Graphical User Interface implementation
#include "ui.h"
#include "hal.h"

// GUI colors
#define GUI_BLACK     0x000000
#define GUI_WHITE     0xFFFFFF
#define GUI_BLUE      0x2563EB
#define GUI_GRAY      0x64748B
#define GUI_LIGHTGRAY 0xF8FAFC
#define GUI_GREEN     0x10B981

// GUI dimensions
#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 768

// GUI components
typedef struct {
    uint32_t type;
    uint32_t id;
    uint32_t x, y, width, height;
    char text[128];
    bool visible;
    bool enabled;
    void (*on_click)(void);
} gui_component_t;

// Global variables
static gui_component_t gui_components[100];
static uint32_t component_count = 0;

// Initialize GUI
int ui_gui_init(void) {
    // Set video mode
    hal_set_video_mode(SCREEN_WIDTH, SCREEN_HEIGHT, 32);
    
    // Clear screen
    hal_clear_screen(GUI_LIGHTGRAY);
    
    return 0;
}

// Draw rectangle
void ui_gui_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    for (uint32_t py = y; py < y + height; py++) {
        for (uint32_t px = x; px < x + width; px++) {
            hal_put_pixel(px, py, color);
        }
    }
}

// Draw text (simplified implementation)
void ui_gui_draw_text(const char* text, uint32_t x, uint32_t y, uint32_t color) {
    // Simplified text rendering
    for (size_t i = 0; text[i] != '\0'; i++) {
        // Draw character (placeholder)
        ui_gui_draw_rect(x + i * 8, y, 8, 16, color);
    }
}

// Create button
uint32_t ui_gui_create_button(const char* text, uint32_t x, uint32_t y, 
                             uint32_t width, uint32_t height, void (*on_click)(void)) {
    if (component_count >= 100) return -1;
    
    gui_component_t* component = &gui_components[component_count];
    component->type = 1; // Button
    component->id = component_count;
    component->x = x;
    component->y = y;
    component->width = width;
    component->height = height;
    strncpy(component->text, text, sizeof(component->text) - 1);
    component->visible = true;
    component->enabled = true;
    component->on_click = on_click;
    
    component_count++;
    return component->id;
}

// Draw GUI components
void ui_gui_draw_components(void) {
    for (uint32_t i = 0; i < component_count; i++) {
        gui_component_t* component = &gui_components[i];
        if (!component->visible) continue;
        
        // Draw button
        if (component->type == 1) {
            uint32_t color = component->enabled ? GUI_BLUE : GUI_GRAY;
            ui_gui_draw_rect(component->x, component->y, 
                           component->width, component->height, color);
            ui_gui_draw_text(component->text, 
                           component->x + 10, component->y + 10, GUI_WHITE);
        }
    }
}

// Process mouse click
void ui_gui_process_mouse_click(uint32_t x, uint32_t y) {
    for (uint32_t i = 0; i < component_count; i++) {
        gui_component_t* component = &gui_components[i];
        if (!component->visible || !component->enabled) continue;
        
        // Check if click is within component bounds
        if (x >= component->x && x < component->x + component->width &&
            y >= component->y && y < component->y + component->height) {
            // Execute click handler
            if (component->on_click) {
                component->on_click();
            }
            break;
        }
    }
}

// Show boot menu in GUI mode
int ui_gui_show_boot_menu(const config_t* config) {
    // Clear screen
    hal_clear_screen(GUI_LIGHTGRAY);
    
    // Draw header
    ui_gui_draw_text("FerryBoot v0.1", 400, 50, GUI_BLUE);
    ui_gui_draw_text("Select operating system:", 50, 100, GUI_GRAY);
    
    // Create buttons for each boot entry
    for (uint32_t i = 0; i < config->entry_count; i++) {
        char button_text[128];
        snprintf(button_text, sizeof(button_text), "%s", config->entries[i].name);
        
        ui_gui_create_button(button_text, 100, 150 + i * 50, 300, 40, NULL);
    }
    
    // Draw components
    ui_gui_draw_components();
    
    // Process input (simplified)
    while (1) {
        if (hal_key_pressed()) {
            uint32_t key = hal_get_key();
            if (key == 0x1C) { // Enter
                return 0; // First entry selected
            }
        }
        
        hal_sleep(10);
    }
    
    return -1;
}
```

## 7. Module System Implementation

### 7.1 Module Interface

```c
// module.h - Module system
#ifndef FERRYBOOT_MODULE_H
#define FERRYBOOT_MODULE_H

#include <stdbool.h>
#include <stdint.h>

// Module magic number
#define MODULE_MAGIC 0xF33D0D11

// Module types
#define MODULE_TYPE_FILESYSTEM  1
#define MODULE_TYPE_HARDWARE    2
#define MODULE_TYPE_COMPRESSION 3
#define MODULE_TYPE_SECURITY    4
#define MODULE_TYPE_NETWORK     5

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

// Module interface
typedef struct {
    int (*init)(void);
    void (*cleanup)(void);
    int (*probe)(void* data);
    // Additional function pointers based on module type
} module_interface_t;

// Module entry
typedef struct {
    module_header_t header;
    module_interface_t* interface;
    void* memory;
} module_entry_t;

// Module API
int module_load(const char* path);
int module_unload(const char* name);
module_interface_t* module_get(const char* name);
int module_register(const module_header_t* header, module_interface_t* interface);
int module_init_all(void);

#endif // FERRYBOOT_MODULE_H
```

### 7.2 Module Manager Implementation

```c
// module_manager.c - Module system implementation
#include "module.h"
#include "hal.h"
#include "security.h"

// Module list
static module_entry_t module_list[32];
static uint32_t module_count = 0;

// Load module from file
int module_load(const char* module_path) {
    // Check if module already loaded
    for (uint32_t i = 0; i < module_count; i++) {
        if (strcmp(module_list[i].header.name, module_path) == 0) {
            return 0; // Already loaded
        }
    }
    
    // Read module header
    module_header_t header;
    if (hal_read_sector(module_path, &header, 1) != 0) {
        return -1;
    }
    
    // Validate module header
    if (header.magic != MODULE_MAGIC) {
        return -1;
    }
    
    // Verify module signature if security is enabled
    if (security_enabled()) {
        if (security_verify_module_signature(module_path, &header) != 0) {
            return -1;
        }
    }
    
    // Allocate memory for module
    void* module_memory = hal_alloc(header.data_size);
    if (!module_memory) {
        return -1;
    }
    
    // Load module data
    if (hal_read_sector(module_path, module_memory, (header.data_size + 511) / 512) != 0) {
        hal_free(module_memory);
        return -1;
    }
    
    // Get module interface
    module_interface_t* interface = (module_interface_t*)
        ((uint8_t*)module_memory + header.entry_point);
    
    // Initialize module
    if (interface->init) {
        int result = interface->init();
        if (result != 0) {
            hal_free(module_memory);
            return result;
        }
    }
    
    // Add to module list
    if (module_count >= 32) {
        if (interface->cleanup) {
            interface->cleanup();
        }
        hal_free(module_memory);
        return -1;
    }
    
    module_list[module_count].header = header;
    module_list[module_count].interface = interface;
    module_list[module_count].memory = module_memory;
    module_count++;
    
    return 0;
}

// Unload module
int module_unload(const char* name) {
    for (uint32_t i = 0; i < module_count; i++) {
        if (strcmp(module_list[i].header.name, name) == 0) {
            // Cleanup module
            if (module_list[i].interface->cleanup) {
                module_list[i].interface->cleanup();
            }
            
            // Free memory
            hal_free(module_list[i].memory);
            
            // Remove from list
            for (uint32_t j = i; j < module_count - 1; j++) {
                module_list[j] = module_list[j + 1];
            }
            module_count--;
            
            return 0;
        }
    }
    
    return -1; // Module not found
}

// Get module by name
module_interface_t* module_get(const char* name) {
    for (uint32_t i = 0; i < module_count; i++) {
        if (strcmp(module_list[i].header.name, name) == 0) {
            return module_list[i].interface;
        }
    }
    
    return NULL;
}

// Register module with system
int module_register(const module_header_t* header, module_interface_t* interface) {
    // Validate header
    if (header->magic != MODULE_MAGIC) {
        return -1;
    }
    
    // Add to module list
    if (module_count >= 32) {
        return -1;
    }
    
    module_list[module_count].header = *header;
    module_list[module_count].interface = interface;
    module_list[module_count].memory = NULL; // Not dynamically loaded
    module_count++;
    
    return 0;
}

// Initialize all loaded modules
int module_init_all(void) {
    for (uint32_t i = 0; i < module_count; i++) {
        if (module_list[i].interface->init) {
            int result = module_list[i].interface->init();
            if (result != 0) {
                return result;
            }
        }
    }
    
    return 0;
}
```

## 8. Boot Process Implementation

### 8.1 OS Detection

```c
// os_detect.c - Operating system detection
#include "os_detect.h"
#include "modules.h"
#include "hal.h"

// Detect operating system on partition
os_type_t os_detect(partition_t* partition) {
    // Try filesystem modules to identify filesystem
    filesystem_module_t* fs_module = (filesystem_module_t*)module_get("filesystem");
    if (!fs_module) {
        return OS_UNKNOWN;
    }
    
    // Mount filesystem
    if (fs_module->fs_ops.mount(partition->device_path) != 0) {
        return OS_UNKNOWN;
    }
    
    // Check for Windows bootloader
    if (fs_module->fs_ops.read_file("/bootmgr", NULL, 0) == 0 ||
        fs_module->fs_ops.read_file("/Windows/System32/winload.exe", NULL, 0) == 0) {
        fs_module->fs_ops.unmount();
        return OS_WINDOWS;
    }
    
    // Check for Linux kernel
    if (fs_module->fs_ops.read_file("/boot/vmlinuz", NULL, 0) == 0 ||
        fs_module->fs_ops.read_file("/vmlinuz", NULL, 0) == 0) {
        fs_module->fs_ops.unmount();
        return OS_LINUX;
    }
    
    // Check for BSD kernel
    if (fs_module->fs_ops.read_file("/boot/kernel/kernel", NULL, 0) == 0) {
        fs_module->fs_ops.unmount();
        return OS_BSD;
    }
    
    // Check for macOS
    if (fs_module->fs_ops.read_file("/System/Library/CoreServices/boot.efi", NULL, 0) == 0) {
        fs_module->fs_ops.unmount();
        return OS_MACOS;
    }
    
    fs_module->fs_ops.unmount();
    return OS_UNKNOWN;
}

// Create boot entry for detected OS
int os_create_boot_entry(partition_t* partition, os_type_t os_type, boot_entry_t* entry) {
    memset(entry, 0, sizeof(boot_entry_t));
    entry->enabled = true;
    
    switch (os_type) {
        case OS_WINDOWS:
            strcpy(entry->name, "Windows");
            snprintf(entry->path, sizeof(entry->path), "%s:/bootmgr", partition->device_path);
            entry->type = OS_WINDOWS;
            break;
            
        case OS_LINUX:
            strcpy(entry->name, "Linux");
            snprintf(entry->path, sizeof(entry->path), "%s:/boot/vmlinuz", partition->device_path);
            snprintf(entry->initrd_path, sizeof(entry->initrd_path), "%s:/boot/initrd.img", partition->device_path);
            snprintf(entry->parameters, sizeof(entry->parameters), "root=%s ro quiet splash", partition->device_path);
            entry->type = OS_LINUX;
            break;
            
        case OS_BSD:
            strcpy(entry->name, "BSD");
            snprintf(entry->path, sizeof(entry->path), "%s:/boot/kernel/kernel", partition->device_path);
            entry->type = OS_BSD;
            break;
            
        case OS_MACOS:
            strcpy(entry->name, "macOS");
            snprintf(entry->path, sizeof(entry->path), "%s:/System/Library/CoreServices/boot.efi", partition->device_path);
            entry->type = OS_MACOS;
            break;
            
        default:
            return -1;
    }
    
    return 0;
}
```

### 8.2 OS Boot Implementation

```c
// boot.c - Operating system boot implementation
#include "boot.h"
#include "hal.h"
#include "security.h"

// Boot Windows
int boot_windows(const boot_entry_t* entry) {
    // Verify boot image if security enabled
    if (security_enabled()) {
        if (security_verify_boot_image(entry->path) != 0) {
            return -1;
        }
    }
    
    // Load Windows bootloader
    void* bootloader = NULL;
    size_t bootloader_size = 0;
    if (file_load(entry->path, &bootloader, &bootloader_size) != 0) {
        return -1;
    }
    
    // Copy bootloader to load address
    memcpy((void*)0x100000, bootloader, bootloader_size);
    hal_free(bootloader);
    
    // Set up boot parameters
    // (Implementation specific to Windows bootloader)
    
    // Switch to protected mode and jump to bootloader
    switch_to_protected_mode();
    jump_to_address(0x100000);
    
    return -1; // Should never reach here
}

// Boot Linux
int boot_linux(const boot_entry_t* entry) {
    // Verify kernel image if security enabled
    if (security_enabled()) {
        if (security_verify_boot_image(entry->path) != 0) {
            return -1;
        }
    }
    
    // Load kernel image
    void* kernel_image = NULL;
    size_t kernel_size = 0;
    if (file_load(entry->path, &kernel_image, &kernel_size) != 0) {
        return -1;
    }
    
    // Load initrd if specified
    void* initrd_image = NULL;
    size_t initrd_size = 0;
    if (entry->initrd_path[0]) {
        if (file_load(entry->initrd_path, &initrd_image, &initrd_size) != 0) {
            hal_free(kernel_image);
            return -1;
        }
    }
    
    // Set up boot parameters
    setup_boot_params(entry->parameters);
    
    // Copy kernel to load address
    memcpy((void*)0x100000, kernel_image, kernel_size);
    hal_free(kernel_image);
    
    // Copy initrd to load address
    if (initrd_image) {
        memcpy((void*)0x2000000, initrd_image, initrd_size);
        hal_free(initrd_image);
    }
    
    // Switch to protected mode and jump to kernel
    switch_to_protected_mode();
    jump_to_kernel(0x100000, initrd_size ? 0x2000000 : 0, initrd_size);
    
    return -1; // Should never reach here
}

// Generic boot function
int boot_os(const boot_entry_t* entry) {
    switch (entry->type) {
        case OS_WINDOWS:
            return boot_windows(entry);
        case OS_LINUX:
            return boot_linux(entry);
        case OS_BSD:
            return boot_bsd(entry);
        case OS_MACOS:
            return boot_macos(entry);
        default:
            return -1;
    }
}
```

## 9. Conclusion

This implementation guide provides detailed information on how to implement the core components of FerryBoot. By following these guidelines, developers can create a robust, secure, and efficient bootloader that meets all the requirements outlined in the design documents.

The modular architecture allows for extensibility while maintaining a small core footprint, and the security features ensure that only trusted code is executed during the boot process. The hardware abstraction layer provides consistent interfaces across different platforms, making it easy to port FerryBoot to new hardware architectures.
# FerryBoot Technical Specification

## 1. System Calls and APIs

### 1.1 Hardware Abstraction Layer (HAL) APIs

#### 1.1.1 Storage APIs

```c
/**
 * Read sectors from storage device
 * @param lba Logical block address to start reading from
 * @param buffer Buffer to store read data
 * @param count Number of sectors to read
 * @return 0 on success, negative error code on failure
 */
int hal_read_sector(uint64_t lba, void* buffer, size_t count);

/**
 * Write sectors to storage device
 * @param lba Logical block address to start writing to
 * @param buffer Buffer containing data to write
 * @param count Number of sectors to write
 * @return 0 on success, negative error code on failure
 */
int hal_write_sector(uint64_t lba, const void* buffer, size_t count);

/**
 * Get storage device information
 * @param device Device identifier
 * @param info Pointer to storage_info_t structure to fill
 * @return 0 on success, negative error code on failure
 */
int hal_get_storage_info(uint32_t device, storage_info_t* info);
```

#### 1.1.2 Display APIs

```c
/**
 * Set video mode
 * @param width Screen width in pixels
 * @param height Screen height in pixels
 * @param depth Color depth in bits per pixel
 * @return 0 on success, negative error code on failure
 */
int hal_set_video_mode(uint32_t width, uint32_t height, uint32_t depth);

/**
 * Put pixel at specified coordinates
 * @param x X coordinate
 * @param y Y coordinate
 * @param color Pixel color in 0xRRGGBB format
 */
void hal_put_pixel(uint32_t x, uint32_t y, uint32_t color);

/**
 * Clear screen with specified color
 * @param color Screen color in 0xRRGGBB format
 */
void hal_clear_screen(uint32_t color);

/**
 * Draw rectangle
 * @param x X coordinate of top-left corner
 * @param y Y coordinate of top-left corner
 * @param width Rectangle width
 * @param height Rectangle height
 * @param color Rectangle color in 0xRRGGBB format
 */
void hal_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);

/**
 * Draw text
 * @param text Text to draw
 * @param x X coordinate
 * @param y Y coordinate
 * @param color Text color in 0xRRGGBB format
 */
void hal_draw_text(const char* text, uint32_t x, uint32_t y, uint32_t color);
```

#### 1.1.3 Input APIs

```c
/**
 * Check if a key is pressed
 * @return true if a key is pressed, false otherwise
 */
bool hal_key_pressed(void);

/**
 * Get key code of pressed key
 * @return Key code or 0 if no key pressed
 */
uint32_t hal_get_key(void);

/**
 * Check if mouse has moved
 * @return true if mouse moved, false otherwise
 */
bool hal_mouse_moved(void);

/**
 * Get current mouse state
 * @param x Pointer to store X coordinate
 * @param y Pointer to store Y coordinate
 * @param buttons Pointer to store button state
 */
void hal_get_mouse_state(int32_t* x, int32_t* y, uint32_t* buttons);
```

#### 1.1.4 Network APIs

```c
/**
 * Initialize network hardware
 * @return 0 on success, negative error code on failure
 */
int hal_network_init(void);

/**
 * Request DHCP configuration
 * @param config Pointer to network_config_t structure to fill
 * @return 0 on success, negative error code on failure
 */
int hal_dhcp_request(network_config_t* config);

/**
 * Download file using TFTP
 * @param server Server IP address
 * @param filename Name of file to download
 * @param buffer Buffer to store downloaded data
 * @param size Pointer to size of buffer/actual size downloaded
 * @return 0 on success, negative error code on failure
 */
int hal_tftp_download(const char* server, const char* filename, void* buffer, size_t* size);
```

#### 1.1.5 Timer APIs

```c
/**
 * Get current system ticks
 * @return Number of ticks since system start
 */
uint64_t hal_get_ticks(void);

/**
 * Sleep for specified milliseconds
 * @param milliseconds Number of milliseconds to sleep
 */
void hal_sleep(uint32_t milliseconds);
```

#### 1.1.6 Memory APIs

```c
/**
 * Allocate memory
 * @param size Size of memory to allocate in bytes
 * @return Pointer to allocated memory or NULL on failure
 */
void* hal_alloc(size_t size);

/**
 * Free allocated memory
 * @param ptr Pointer to memory to free
 */
void hal_free(void* ptr);

/**
 * Reallocate memory
 * @param ptr Pointer to existing memory
 * @param size New size in bytes
 * @return Pointer to reallocated memory or NULL on failure
 */
void* hal_realloc(void* ptr, size_t size);
```

### 1.2 Module System APIs

```c
/**
 * Load module from file
 * @param path Path to module file
 * @return 0 on success, negative error code on failure
 */
int module_load(const char* path);

/**
 * Unload module
 * @param name Name of module to unload
 * @return 0 on success, negative error code on failure
 */
int module_unload(const char* name);

/**
 * Get module by name
 * @param name Name of module
 * @return Pointer to module or NULL if not found
 */
module_interface_t* module_get(const char* name);

/**
 * Register module with system
 * @param header Module header
 * @param interface Module interface
 * @return 0 on success, negative error code on failure
 */
int module_register(const module_header_t* header, module_interface_t* interface);

/**
 * Initialize all loaded modules
 * @return 0 on success, negative error code on failure
 */
int module_init_all(void);
```

### 1.3 Configuration System APIs

```c
/**
 * Load configuration from storage
 * @param config Pointer to config_t structure to fill
 * @return 0 on success, negative error code on failure
 */
int config_load(config_t* config);

/**
 * Save configuration to storage
 * @param config Pointer to config_t structure to save
 * @return 0 on success, negative error code on failure
 */
int config_save(const config_t* config);

/**
 * Get integer configuration value
 * @param config Configuration structure
 * @param key Configuration key
 * @param default_value Default value if key not found
 * @return Configuration value
 */
int config_get_int(const config_t* config, const char* key, int default_value);

/**
 * Set integer configuration value
 * @param config Configuration structure
 * @param key Configuration key
 * @param value Value to set
 * @return 0 on success, negative error code on failure
 */
int config_set_int(config_t* config, const char* key, int value);

/**
 * Get string configuration value
 * @param config Configuration structure
 * @param key Configuration key
 * @param default_value Default value if key not found
 * @return Configuration value
 */
const char* config_get_string(const config_t* config, const char* key, const char* default_value);

/**
 * Set string configuration value
 * @param config Configuration structure
 * @param key Configuration key
 * @param value Value to set
 * @return 0 on success, negative error code on failure
 */
int config_set_string(config_t* config, const char* key, const char* value);

/**
 * Validate configuration integrity
 * @param config Configuration structure
 * @return true if valid, false otherwise
 */
bool config_validate(const config_t* config);
```

### 1.4 Security System APIs

```c
/**
 * Calculate SHA-256 hash
 * @param data Data to hash
 * @param len Length of data in bytes
 * @param hash Buffer to store hash (32 bytes)
 * @return 0 on success, negative error code on failure
 */
int crypto_sha256(const void* data, size_t len, uint8_t* hash);

/**
 * Calculate SHA-512 hash
 * @param data Data to hash
 * @param len Length of data in bytes
 * @param hash Buffer to store hash (64 bytes)
 * @return 0 on success, negative error code on failure
 */
int crypto_sha512(const void* data, size_t len, uint8_t* hash);

/**
 * Verify RSA signature
 * @param signature Signature to verify
 * @param sig_len Length of signature in bytes
 * @param data Data that was signed
 * @param data_len Length of data in bytes
 * @param pubkey Public key for verification
 * @return 0 on success, negative error code on failure
 */
int crypto_rsa_verify(const uint8_t* signature, size_t sig_len,
                      const uint8_t* data, size_t data_len,
                      const rsa_public_key_t* pubkey);

/**
 * Decrypt data using AES
 * @param ciphertext Encrypted data
 * @param len Length of data in bytes
 * @param key Encryption key
 * @param key_len Length of key in bytes
 * @param iv Initialization vector
 * @param plaintext Buffer to store decrypted data
 * @return 0 on success, negative error code on failure
 */
int crypto_aes_decrypt(const uint8_t* ciphertext, size_t len,
                       const uint8_t* key, size_t key_len,
                       const uint8_t* iv, uint8_t* plaintext);

/**
 * Verify boot image signature
 * @param image_path Path to image file
 * @return 0 on success, negative error code on failure
 */
int security_verify_boot_image(const char* image_path);

/**
 * Unlock encrypted volume
 * @param device Device path
 * @param passphrase User passphrase
 * @return 0 on success, negative error code on failure
 */
int security_unlock_volume(const char* device, const char* passphrase);
```

### 1.5 User Interface APIs

```c
/**
 * Initialize text mode interface
 * @return 0 on success, negative error code on failure
 */
int ui_text_init(void);

/**
 * Draw character in text mode
 * @param ch Character to draw
 * @param attr Character attributes
 * @param x X coordinate (column)
 * @param y Y coordinate (row)
 */
void ui_text_draw_char(uint8_t ch, uint8_t attr, uint8_t x, uint8_t y);

/**
 * Draw string in text mode
 * @param str String to draw
 * @param attr Character attributes
 * @param x X coordinate (column)
 * @param y Y coordinate (row)
 */
void ui_text_draw_string(const char* str, uint8_t attr, uint8_t x, uint8_t y);

/**
 * Initialize graphical interface
 * @return 0 on success, negative error code on failure
 */
int ui_gui_init(void);

/**
 * Process input events
 * @param event Input event to process
 * @return 0 on success, negative error code on failure
 */
int ui_process_event(const input_event_t* event);

/**
 * Show boot menu
 * @param config Configuration structure
 * @return 0 on success, negative error code on failure
 */
int ui_show_boot_menu(const config_t* config);

/**
 * Show setup wizard
 * @param config Configuration structure
 * @return 0 on success, negative error code on failure
 */
int ui_show_setup_wizard(config_t* config);
```

## 2. Core Algorithms

### 2.1 Boot Entry Detection Algorithm

```c
/**
 * Detect boot entries on all available partitions
 * @param config Configuration structure to populate
 * @return 0 on success, negative error code on failure
 */
int algorithm_detect_boot_entries(config_t* config) {
    // 1. Initialize partition list
    partition_list_t partitions;
    partition_scan(&partitions);
    
    // 2. For each partition, detect filesystem and OS
    for (int i = 0; i < partitions.count; i++) {
        partition_t* partition = &partitions.partitions[i];
        
        // 2.1. Identify filesystem
        filesystem_type_t fs_type = filesystem_identify(partition);
        if (fs_type == FS_UNKNOWN) {
            continue;
        }
        
        // 2.2. Mount filesystem
        if (filesystem_mount(partition, fs_type) != 0) {
            continue;
        }
        
        // 2.3. Detect operating system
        os_type_t os_type = os_detect(partition);
        if (os_type == OS_UNKNOWN) {
            filesystem_unmount(partition);
            continue;
        }
        
        // 2.4. Create boot entry
        boot_entry_t entry;
        if (os_create_boot_entry(partition, os_type, &entry) == 0) {
            config_add_boot_entry(config, &entry);
        }
        
        // 2.5. Unmount filesystem
        filesystem_unmount(partition);
    }
    
    return 0;
}
```

### 2.2 Module Loading Algorithm

```c
/**
 * Load and initialize module
 * @param module_path Path to module file
 * @return 0 on success, negative error code on failure
 */
int algorithm_load_module(const char* module_path) {
    // 1. Read module header
    module_header_t header;
    if (file_read(module_path, &header, sizeof(header)) != sizeof(header)) {
        return ERROR_FILE_READ;
    }
    
    // 2. Validate module header
    if (header.magic != MODULE_MAGIC) {
        return ERROR_INVALID_MODULE;
    }
    
    // 3. Verify module signature if security is enabled
    if (security_enabled()) {
        if (security_verify_module_signature(module_path, &header) != 0) {
            return ERROR_INVALID_SIGNATURE;
        }
    }
    
    // 4. Allocate memory for module
    void* module_memory = hal_alloc(header.data_size);
    if (!module_memory) {
        return ERROR_OUT_OF_MEMORY;
    }
    
    // 5. Load module data
    if (file_read(module_path, module_memory, header.data_size) != header.data_size) {
        hal_free(module_memory);
        return ERROR_FILE_READ;
    }
    
    // 6. Get module interface
    module_interface_t* interface = (module_interface_t*)
        ((uint8_t*)module_memory + header.entry_point);
    
    // 7. Initialize module
    if (interface->init) {
        int result = interface->init();
        if (result != 0) {
            hal_free(module_memory);
            return result;
        }
    }
    
    // 8. Register module with system
    int result = module_register(&header, interface);
    if (result != 0) {
        if (interface->cleanup) {
            interface->cleanup();
        }
        hal_free(module_memory);
        return result;
    }
    
    return 0;
}
```

### 2.3 Secure Boot Verification Algorithm

```c
/**
 * Verify boot image using secure boot
 * @param image_path Path to image file
 * @param signature_path Path to signature file
 * @return 0 on success, negative error code on failure
 */
int algorithm_secure_boot_verify(const char* image_path, const char* signature_path) {
    // 1. Load image file
    void* image_data = NULL;
    size_t image_size = 0;
    if (file_load(image_path, &image_data, &image_size) != 0) {
        return ERROR_FILE_READ;
    }
    
    // 2. Calculate image hash
    uint8_t image_hash[32];
    if (crypto_sha256(image_data, image_size, image_hash) != 0) {
        hal_free(image_data);
        return ERROR_HASH_FAILED;
    }
    
    // 3. Load signature
    uint8_t signature[256];
    size_t signature_size = 0;
    if (file_read(signature_path, signature, sizeof(signature)) <= 0) {
        hal_free(image_data);
        return ERROR_FILE_READ;
    }
    
    // 4. Get trusted keys
    rsa_public_key_t* trusted_keys = NULL;
    uint32_t key_count = 0;
    if (security_get_trusted_keys(&trusted_keys, &key_count) != 0) {
        hal_free(image_data);
        return ERROR_NO_TRUSTED_KEYS;
    }
    
    // 5. Verify signature with each trusted key
    int result = ERROR_INVALID_SIGNATURE;
    for (uint32_t i = 0; i < key_count; i++) {
        if (crypto_rsa_verify(signature, signature_size,
                              image_hash, sizeof(image_hash),
                              &trusted_keys[i]) == 0) {
            result = 0; // Valid signature found
            break;
        }
    }
    
    // 6. Cleanup
    hal_free(image_data);
    security_free_trusted_keys(trusted_keys);
    
    return result;
}
```

### 2.4 Disk Encryption Unlock Algorithm

```c
/**
 * Unlock encrypted volume using passphrase
 * @param device Device path
 * @param passphrase User passphrase
 * @return 0 on success, negative error code on failure
 */
int algorithm_unlock_encrypted_volume(const char* device, const char* passphrase) {
    // 1. Read encryption header
    encryption_header_t header;
    if (device_read(device, 0, &header, sizeof(header)) != sizeof(header)) {
        return ERROR_DEVICE_READ;
    }
    
    // 2. Validate header
    if (header.magic != ENCRYPTION_MAGIC) {
        return ERROR_INVALID_HEADER;
    }
    
    // 3. Derive key from passphrase
    uint8_t derived_key[32];
    if (pbkdf2_sha256(passphrase, header.salt, header.iterations, 
                      derived_key, sizeof(derived_key)) != 0) {
        return ERROR_KEY_DERIVATION;
    }
    
    // 4. Try each key slot
    int result = ERROR_INVALID_PASSPHRASE;
    for (int i = 0; i < MAX_KEY_SLOTS; i++) {
        if (header.key_slots[i].active) {
            // 4.1. Decrypt master key
            uint8_t master_key[32];
            if (crypto_aes_decrypt(header.key_slots[i].encrypted_key,
                                   sizeof(header.key_slots[i].encrypted_key),
                                   derived_key, sizeof(derived_key),
                                   header.key_slots[i].iv, master_key) == 0) {
                // 4.2. Verify master key
                if (verify_master_key(device, &header, master_key) == 0) {
                    // 4.3. Unlock volume
                    result = unlock_volume(device, master_key);
                    break;
                }
            }
        }
    }
    
    return result;
}
```

### 2.5 Automatic Recovery Algorithm

```c
/**
 * Automatic recovery from boot failures
 * @return 0 on success, negative error code on failure
 */
int algorithm_automatic_recovery(void) {
    // 1. Check boot failure counter
    uint32_t failure_count = boot_get_failure_count();
    
    // 2. If too many failures, enter recovery mode
    if (failure_count > MAX_BOOT_FAILURES) {
        // 2.1. Reset failure counter
        boot_reset_failure_count();
        
        // 2.2. Enter recovery mode
        return recovery_mode_enter();
    }
    
    // 3. Increment failure counter
    boot_increment_failure_count();
    
    // 4. Continue normal boot
    return 0;
}
```

## 3. Data Structures

### 3.1 Hardware Information Structures

```c
// Storage device information
typedef struct {
    uint64_t total_sectors;
    uint32_t sector_size;
    uint32_t device_type;
    char model[64];
    char serial[32];
} storage_info_t;

// Network configuration
typedef struct {
    uint32_t ip_address;
    uint32_t subnet_mask;
    uint32_t gateway;
    uint32_t dns_server;
    uint8_t mac_address[6];
} network_config_t;

// Partition information
typedef struct {
    uint64_t start_lba;
    uint64_t sector_count;
    uint32_t partition_type;
    bool bootable;
    char device_path[128];
} partition_t;

typedef struct {
    partition_t partitions[128];
    uint32_t count;
} partition_list_t;
```

### 3.2 Filesystem Structures

```c
// Filesystem types
typedef enum {
    FS_UNKNOWN = 0,
    FS_FAT32,
    FS_EXT4,
    FS_NTFS,
    FS_XFS,
    FS_BTRFS,
    FS_APFS
} filesystem_type_t;

// File information
typedef struct {
    char name[256];
    uint64_t size;
    uint32_t attributes;
    uint64_t creation_time;
    uint64_t modification_time;
} file_info_t;
```

### 3.3 Operating System Structures

```c
// Operating system types
typedef enum {
    OS_UNKNOWN = 0,
    OS_WINDOWS,
    OS_LINUX,
    OS_BSD,
    OS_MACOS
} os_type_t;

// Boot entry
typedef struct {
    char name[64];
    char path[256];
    char initrd_path[256];
    char parameters[256];
    os_type_t type;
    bool enabled;
    uint32_t id;
} boot_entry_t;
```

### 3.4 Security Structures

```c
// RSA public key
typedef struct {
    uint8_t modulus[256];
    uint8_t exponent[4];
    uint32_t modulus_size;
} rsa_public_key_t;

// Encryption header
typedef struct {
    uint32_t magic;
    uint8_t salt[32];
    uint32_t iterations;
    struct {
        bool active;
        uint8_t encrypted_key[32];
        uint8_t iv[16];
    } key_slots[8];
    uint8_t reserved[448];
} encryption_header_t;
```

### 3.5 UI Structures

```c
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
```

## 4. Error Codes

```c
// Generic error codes
#define ERROR_SUCCESS                0
#define ERROR_INVALID_PARAMETER     -1
#define ERROR_OUT_OF_MEMORY         -2
#define ERROR_FILE_NOT_FOUND        -3
#define ERROR_FILE_READ             -4
#define ERROR_FILE_WRITE            -5
#define ERROR_DEVICE_NOT_FOUND      -6
#define ERROR_DEVICE_READ           -7
#define ERROR_DEVICE_WRITE          -8
#define ERROR_INVALID_FORMAT        -9
#define ERROR_INVALID_SIGNATURE     -10
#define ERROR_INVALID_PASSPHRASE    -11
#define ERROR_ENCRYPTION_FAILED     -12
#define ERROR_DECRYPTION_FAILED     -13
#define ERROR_HASH_FAILED           -14
#define ERROR_KEY_DERIVATION        -15
#define ERROR_NO_TRUSTED_KEYS       -16
#define ERROR_MODULE_LOAD_FAILED    -17
#define ERROR_MODULE_INIT_FAILED    -18
#define ERROR_UNSUPPORTED_FEATURE   -19
#define ERROR_TIMEOUT               -20
#define ERROR_NETWORK_ERROR         -21
#define ERROR_INVALID_HEADER        -22

// OS-specific error codes
#define ERROR_WINDOWS_BOOT_FAILED   -100
#define ERROR_LINUX_BOOT_FAILED     -101
#define ERROR_BSD_BOOT_FAILED       -102
#define ERROR_MACOS_BOOT_FAILED     -103
```

## 5. Memory Layout

### 5.1 BIOS Memory Map

```text
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

### 5.2 UEFI Memory Map

```text
0x00000000 +------------------+
           |   UEFI Firmware  |
0x00100000 +------------------+
           | FerryBoot Image  |
0x00200000 +------------------+
           |   Boot Modules   |
0x00400000 +------------------+
           |   Kernel Load    |
           |     Area         |
0x01000000 +------------------+
           |   Runtime Data   |
0x02000000 +------------------+
```

## 6. Performance Requirements

### 6.1 Boot Time Targets

- BIOS systems: <2 seconds total boot time
- UEFI systems: <1 second total boot time
- OS loading time: Dependent on storage speed

### 6.2 Memory Usage Targets

- Stage 1: 512 bytes (fixed)
- Stage 2: <32KB
- Modules: Loaded on-demand, <512KB total
- Total memory footprint: <1MB

### 6.3 Storage Requirements

- BIOS bootloader: <64KB
- UEFI bootloader: <1MB
- Modules: Variable size, typically <256KB each

## 7. Security Requirements

### 7.1 Cryptographic Standards

- Hashing: SHA-256, SHA-512
- Encryption: AES-256
- Signatures: RSA-2048, RSA-4096
- Key Derivation: PBKDF2-SHA256

### 7.2 Secure Boot Compliance

- UEFI Secure Boot support
- Microsoft UEFI CA integration
- Custom certificate authority support
- Revocation list support

### 7.3 Threat Model

- Protection against bootkits
- Protection against rootkits
- Protection against firmware attacks
- Protection against physical tampering

## 8. Compatibility Requirements

### 8.1 Hardware Support

- x86/x64 processors
- ARM64 processors
- BIOS/MBR systems
- UEFI/GPT systems
- Legacy and modern hardware

### 8.2 Operating System Support

- Windows 7/8/10/11
- Linux distributions (Ubuntu, Fedora, Debian, etc.)
- BSD variants (FreeBSD, OpenBSD, NetBSD)
- macOS (with appropriate drivers)

### 8.3 Filesystem Support

- FAT32 (built-in)
- ext4/ext3/ext2
- NTFS
- XFS
- Btrfs
- APFS (macOS)
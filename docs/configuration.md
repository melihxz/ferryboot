# FerryBoot Configuration System

## Overview

FerryBoot uses a flexible configuration system to store user preferences and system settings. The configuration is persistent across reboots and can be modified through both the setup wizard and command-line tools.

## Configuration Storage

### BIOS Systems

Configuration is stored in a dedicated partition or file:
- Primary: `/boot/ferryboot/config.bin`
- Backup: Embedded in bootloader binary

### UEFI Systems

Configuration is stored in EFI variables:
- Primary: `FERRYBOOT-CONFIG`
- Backup: `/EFI/FERRYBOOT/config.bin`

## Configuration Structure

The configuration is stored in a binary format with the following structure:

```c
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
    } entries[32];               // Boot entries
    
    // Reserved for future use
    uint8_t reserved[1024];
} config_t;
```

## Configuration API

The configuration system provides a simple API for accessing settings:

```c
// Load configuration from storage
int config_load(config_t* config);

// Save configuration to storage
int config_save(const config_t* config);

// Get a configuration value
int config_get_int(const config_t* config, const char* key, int default_value);
const char* config_get_string(const config_t* config, const char* key, const char* default_value);

// Set a configuration value
int config_set_int(config_t* config, const char* key, int value);
int config_set_string(config_t* config, const char* key, const char* value);

// Validate configuration
bool config_validate(const config_t* config);
```

## Setup Wizard

FerryBoot includes a setup wizard for easy configuration:

### Quick Setup

For non-technical users:
1. Detect operating systems
2. Set default boot entry
3. Configure basic settings
4. Apply configuration

### Advanced Setup

For power users:
1. Manual OS detection
2. Custom boot parameters
3. Security settings
4. Module selection
5. Display configuration

## Command-Line Tools

FerryBoot provides command-line tools for configuration management:

```bash
# Show current configuration
ferryboot-config --show

# Set boot timeout
ferryboot-config --set timeout=10

# Add boot entry
ferryboot-config --add-entry "Windows" --path "/dev/sda1" --type=windows

# Enable password protection
ferryboot-config --set password_protected=true

# List available modules
ferryboot-config --list-modules
```

## Configuration Migration

The configuration system supports version migration:

```c
// Migrate from version 1 to version 2
int config_migrate_v1_to_v2(config_t* config) {
    // Convert old format to new format
    // Set new fields to default values
    // Update version number
    return 0;
}
```

## Security Considerations

1. Configuration integrity is protected with checksums
2. Passwords are stored as salted hashes
3. Secure Boot systems protect configuration from tampering
4. Backup configuration is used if primary is corrupted
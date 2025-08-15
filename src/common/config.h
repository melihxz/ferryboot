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

// Forward declaration for boot entry
struct boot_entry {
    char name[64];
    char path[256];
    char parameters[256];
    uint32_t type;
    bool enabled;
};

// Configuration API
int config_load(config_t* config);
int config_save(const config_t* config);
int config_init_defaults(config_t* config);
bool config_validate(const config_t* config);
int config_add_boot_entry(config_t* config, const struct boot_entry* entry);

#endif // FERRYBOOT_CONFIG_H
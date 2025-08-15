#include "config.h"
#include "memory.h"
#include "string.h"

// Configuration magic number
#define CONFIG_MAGIC 0xF33DB007
#define CONFIG_VERSION 1

// Calculate simple checksum
static uint32_t config_checksum(const config_t* config) {
    const uint32_t* data = (const uint32_t*)config;
    uint32_t checksum = 0;
    
    // Sum all 32-bit words except the checksum field itself
    for (size_t i = 0; i < sizeof(config_t) / sizeof(uint32_t); i++) {
        if (i != offsetof(config_t, checksum) / sizeof(uint32_t)) {
            checksum += data[i];
        }
    }
    
    return checksum;
}

// Load configuration from storage
int config_load(config_t* config) {
    // In a real implementation, this would read from storage
    // For now, we'll initialize with defaults
    return config_init_defaults(config);
}

// Save configuration to storage
int config_save(const config_t* config) {
    // In a real implementation, this would write to storage
    // For now, we'll just return success
    return 0;
}

// Initialize configuration with defaults
int config_init_defaults(config_t* config) {
    // Clear configuration
    memory_set(config, 0, sizeof(config_t));
    
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
    string_copy(config->theme, "default");
    config->password_protected = false;
    config->secure_boot = true;
    config->signature_check = true;
    config->pxe_boot = false;
    config->pxe_timeout = 30;
    config->module_count = 0;
    config->entry_count = 0;
    
    // Update checksum
    config->checksum = config_checksum(config);
    
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
int config_add_boot_entry(config_t* config, const struct boot_entry* entry) {
    if (config->entry_count >= 32) {
        return -1; // Too many entries
    }
    
    // Copy entry data
    string_copy(config->entries[config->entry_count].name, entry->name);
    string_copy(config->entries[config->entry_count].path, entry->path);
    string_copy(config->entries[config->entry_count].parameters, entry->parameters);
    config->entries[config->entry_count].type = entry->type;
    config->entries[config->entry_count].enabled = entry->enabled;
    
    config->entry_count++;
    
    // Update checksum
    config->checksum = config_checksum(config);
    
    return 0;
}
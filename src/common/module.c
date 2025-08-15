#include "module.h"
#include "memory.h"
#include "string.h"

// Module list
static module_entry_t module_list[32];
static uint32_t module_count = 0;

// Load module from memory (placeholder implementation)
int module_load(const char* path) {
    // In a real implementation, this would load a module from storage
    // For now, we'll just return success
    return 0;
}

// Unload module
int module_unload(const char* name) {
    for (uint32_t i = 0; i < module_count; i++) {
        if (string_compare(module_list[i].header.name, name) == 0) {
            // Cleanup module if cleanup function exists
            if (module_list[i].interface && module_list[i].interface->cleanup) {
                module_list[i].interface->cleanup();
            }
            
            // Free memory if allocated
            if (module_list[i].memory) {
                memory_free(module_list[i].memory);
            }
            
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
        if (string_compare(module_list[i].header.name, name) == 0) {
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
    
    // Check if module already registered
    for (uint32_t i = 0; i < module_count; i++) {
        if (string_compare(module_list[i].header.name, header->name) == 0) {
            return 0; // Already registered
        }
    }
    
    // Add to module list
    if (module_count >= 32) {
        return -1; // Too many modules
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
        if (module_list[i].interface && module_list[i].interface->init) {
            int result = module_list[i].interface->init();
            if (result != 0) {
                return result;
            }
        }
    }
    
    return 0;
}
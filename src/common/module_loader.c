#include "module_loader.h"
#include "memory.h"
#include "string.h"
#include "hal.h"

// Maximum number of loaded modules
#define MAX_MODULES 16

// Module loader context
static struct {
    module_entry_t modules[MAX_MODULES];
    uint32_t module_count;
} module_loader_context;

// Initialize module loader
int module_loader_init(void) {
    // Clear module loader context
    memory_set(&module_loader_context, 0, sizeof(module_loader_context));
    return 0;
}

// Load module from storage
int module_loader_load(const char* module_path) {
    // Check if we have space for more modules
    if (module_loader_context.module_count >= MAX_MODULES) {
        return -1;
    }
    
    // Find free slot
    uint32_t slot = module_loader_context.module_count;
    
    // For now, we'll just simulate loading by creating a placeholder
    // In a real implementation, this would:
    // 1. Read module file from storage
    // 2. Verify module header and signature
    // 3. Allocate memory for module
    // 4. Load module into memory
    // 5. Initialize module
    
    // Create placeholder module
    module_loader_context.modules[slot].header.magic = MODULE_MAGIC;
    module_loader_context.modules[slot].header.version = 1;
    module_loader_context.modules[slot].header.type = MODULE_TYPE_FILESYSTEM;
    module_loader_context.modules[slot].header.flags = 0;
    string_copy(module_loader_context.modules[slot].header.name, "placeholder");
    string_copy(module_loader_context.modules[slot].header.description, "Placeholder module");
    module_loader_context.modules[slot].header.entry_point = 0;
    module_loader_context.modules[slot].header.data_size = 0;
    
    // Set interface (placeholder)
    static module_interface_t placeholder_interface = {0};
    module_loader_context.modules[slot].interface = &placeholder_interface;
    module_loader_context.modules[slot].memory = NULL;
    
    module_loader_context.module_count++;
    return 0;
}

// Unload module
int module_loader_unload(const char* module_name) {
    for (uint32_t i = 0; i < module_loader_context.module_count; i++) {
        if (string_compare(module_loader_context.modules[i].header.name, module_name) == 0) {
            // Cleanup module if needed
            if (module_loader_context.modules[i].interface && 
                module_loader_context.modules[i].interface->cleanup) {
                module_loader_context.modules[i].interface->cleanup();
            }
            
            // Free memory if allocated
            if (module_loader_context.modules[i].memory) {
                memory_free(module_loader_context.modules[i].memory);
            }
            
            // Shift remaining modules down
            for (uint32_t j = i; j < module_loader_context.module_count - 1; j++) {
                module_loader_context.modules[j] = module_loader_context.modules[j + 1];
            }
            
            module_loader_context.module_count--;
            return 0;
        }
    }
    
    return -1; // Module not found
}

// Get module by name
module_interface_t* module_loader_get(const char* module_name) {
    for (uint32_t i = 0; i < module_loader_context.module_count; i++) {
        if (string_compare(module_loader_context.modules[i].header.name, module_name) == 0) {
            return module_loader_context.modules[i].interface;
        }
    }
    
    return NULL;
}

// Cleanup module loader
void module_loader_cleanup(void) {
    // Unload all modules
    for (uint32_t i = 0; i < module_loader_context.module_count; i++) {
        // Cleanup module if needed
        if (module_loader_context.modules[i].interface && 
            module_loader_context.modules[i].interface->cleanup) {
            module_loader_context.modules[i].interface->cleanup();
        }
        
        // Free memory if allocated
        if (module_loader_context.modules[i].memory) {
            memory_free(module_loader_context.modules[i].memory);
        }
    }
    
    // Clear context
    memory_set(&module_loader_context, 0, sizeof(module_loader_context));
}
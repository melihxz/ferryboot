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
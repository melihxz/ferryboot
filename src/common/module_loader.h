#ifndef FERRYBOOT_MODULE_LOADER_H
#define FERRYBOOT_MODULE_LOADER_H

#include "module.h"

// Module loader functions
int module_loader_init(void);
int module_loader_load(const char* module_path);
int module_loader_unload(const char* module_name);
module_interface_t* module_loader_get(const char* module_name);
void module_loader_cleanup(void);

#endif // FERRYBOOT_MODULE_LOADER_H
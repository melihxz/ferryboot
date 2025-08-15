#ifndef FERRYBOOT_PXE_MODULE_H
#define FERRYBOOT_PXE_MODULE_H

#include "../../common/module.h"
#include "../../common/hal.h"
#include "../../common/network.h"

// PXE module context
typedef struct {
    bool initialized;
    network_config_t network_config;
    char tftp_server[16]; // IP address as string
    uint32_t tftp_server_ip;
} pxe_context_t;

// PXE module interface
typedef struct {
    module_interface_t base;
    pxe_context_t context;
} pxe_module_t;

#endif // FERRYBOOT_PXE_MODULE_H
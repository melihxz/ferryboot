#include "pxe.h"
#include "../../common/memory.h"
#include "../../common/string.h"

// Global PXE module instance
static pxe_module_t pxe_module_instance;

// Initialize PXE module
static int pxe_init(void) {
    // Initialize module instance
    memory_set(&pxe_module_instance, 0, sizeof(pxe_module_instance));
    
    // Mark as not initialized yet
    pxe_module_instance.context.initialized = false;
    
    return 0;
}

// Cleanup PXE module
static void pxe_cleanup(void) {
    // Clear module instance
    memory_set(&pxe_module_instance, 0, sizeof(pxe_module_instance));
}

// Initialize network using DHCP
int pxe_initialize_network(void) {
    // Check if HAL is available
    if (!g_hal || !g_hal->network_init || !g_hal->dhcp_request) {
        return -1;
    }
    
    // Initialize network hardware
    if (g_hal->network_init() != 0) {
        return -1;
    }
    
    // Request DHCP configuration
    if (g_hal->dhcp_request(&pxe_module_instance.context.network_config) != 0) {
        return -1;
    }
    
    // Store TFTP server IP (typically the DHCP server)
    pxe_module_instance.context.tftp_server_ip = pxe_module_instance.context.network_config.gateway;
    
    // Convert IP to string format manually
    uint8_t* ip_bytes = (uint8_t*)&pxe_module_instance.context.tftp_server_ip;
    // Format: "xxx.xxx.xxx.xxx" (max 15 chars + null terminator)
    int pos = 0;
    for (int i = 0; i < 4; i++) {
        if (ip_bytes[i] >= 100) {
            pxe_module_instance.context.tftp_server[pos++] = '0' + (ip_bytes[i] / 100);
            pxe_module_instance.context.tftp_server[pos++] = '0' + ((ip_bytes[i] / 10) % 10);
            pxe_module_instance.context.tftp_server[pos++] = '0' + (ip_bytes[i] % 10);
        } else if (ip_bytes[i] >= 10) {
            pxe_module_instance.context.tftp_server[pos++] = '0' + (ip_bytes[i] / 10);
            pxe_module_instance.context.tftp_server[pos++] = '0' + (ip_bytes[i] % 10);
        } else {
            pxe_module_instance.context.tftp_server[pos++] = '0' + ip_bytes[i];
        }
        
        if (i < 3) {
            pxe_module_instance.context.tftp_server[pos++] = '.';
        }
    }
    pxe_module_instance.context.tftp_server[pos] = '\0';
    
    pxe_module_instance.context.initialized = true;
    return 0;
}

// Download file using TFTP
int pxe_download_file(const char* filename, void* buffer, size_t* size) {
    // Check if network is initialized
    if (!pxe_module_instance.context.initialized) {
        return -1;
    }
    
    // Check if HAL supports TFTP
    if (!g_hal || !g_hal->tftp_download) {
        return -1;
    }
    
    // Download file using TFTP
    return g_hal->tftp_download(pxe_module_instance.context.tftp_server, 
                               filename, buffer, size);
}

// Get network configuration
int pxe_get_network_config(network_config_t* config) {
    if (!pxe_module_instance.context.initialized || !config) {
        return -1;
    }
    
    *config = pxe_module_instance.context.network_config;
    return 0;
}

// Module metadata
static const module_header_t pxe_module_header = {
    .magic = MODULE_MAGIC,
    .version = 1,
    .type = MODULE_TYPE_NETWORK,
    .flags = 0,
    .name = "pxe",
    .description = "PXE network boot module"
};

// Module interface
static module_interface_t pxe_module_interface = {
    .init = pxe_init,
    .cleanup = pxe_cleanup
};

// Module entry point
int pxe_module_main(module_header_t* header) {
    if (header->magic != MODULE_MAGIC) {
        return -1;
    }
    
    *header = pxe_module_header;
    return (int)&pxe_module_interface;
}

// Public functions
int pxe_module_initialize_network(void) {
    return pxe_initialize_network();
}

int pxe_module_download_file(const char* filename, void* buffer, size_t* size) {
    return pxe_download_file(filename, buffer, size);
}

int pxe_module_get_network_config(network_config_t* config) {
    return pxe_get_network_config(config);
}
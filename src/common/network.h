#ifndef FERRYBOOT_NETWORK_H
#define FERRYBOOT_NETWORK_H

#include <stdint.h>

// Network configuration
typedef struct network_config {
    uint32_t ip_address;
    uint32_t subnet_mask;
    uint32_t gateway;
    uint32_t dns_server;
    uint8_t mac_address[6];
} network_config_t;

#endif // FERRYBOOT_NETWORK_H
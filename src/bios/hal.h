#ifndef FERRYBOOT_BIOS_HAL_H
#define FERRYBOOT_BIOS_HAL_H

#include "../common/hal.h"
#include "../common/network.h"

// BIOS HAL implementation
typedef struct {
    hardware_interface_t base;
} bios_hal_t;

// BIOS HAL functions
int bios_hal_init(void);
void bios_hal_cleanup(void);

#endif // FERRYBOOT_BIOS_HAL_H
#ifndef FERRYBOOT_UEFI_HAL_H
#define FERRYBOOT_UEFI_HAL_H

#include <efi.h>
#include <efilib.h>
#include "../common/hal.h"
#include "../common/network.h"

// UEFI HAL implementation
typedef struct {
    hardware_interface_t base;
    EFI_HANDLE image_handle;
    EFI_SYSTEM_TABLE* system_table;
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
    EFI_SIMPLE_POINTER_PROTOCOL* pointer;
    EFI_BLOCK_IO_PROTOCOL* block_io;
} uefi_hal_t;

// UEFI HAL functions
int uefi_hal_init(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* system_table);
void uefi_hal_cleanup(void);

#endif // FERRYBOOT_UEFI_HAL_H
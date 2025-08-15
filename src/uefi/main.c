// FerryBoot UEFI Bootloader
// Entry point for UEFI systems

#include <efi.h>
#include <efilib.h>
#include "hal.h"
#include "../common/config.h"
#include "../common/ui.h"
#include "../common/module.h"

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    // Initialize UEFI HAL
    if (uefi_hal_init(ImageHandle, SystemTable) != 0) {
        Print(L"Error: Failed to initialize UEFI HAL\n");
        return EFI_LOAD_ERROR;
    }
    
    // Print welcome message
    Print(L"FerryBoot v0.1 - UEFI Mode\n");
    Print(L"Initializing bootloader...\n");
    
    // Load configuration
    config_t config;
    if (config_load(&config) != 0) {
        Print(L"Warning: Failed to load configuration, using defaults\n");
        config_init_defaults(&config);
    }
    
    // Initialize module system
    if (module_init_all() != 0) {
        Print(L"Error: Failed to initialize modules\n");
        return EFI_LOAD_ERROR;
    }
    
    // Initialize UI
    if (ui_init(config.gui_mode) != 0) {
        Print(L"Error: Failed to initialize UI\n");
        return EFI_LOAD_ERROR;
    }
    
    // Show boot menu
    int selected_entry = ui_show_boot_menu(&config);
    if (selected_entry >= 0 && selected_entry < (int)config.entry_count) {
        // Boot selected OS
        // In a real implementation, this would load and boot the OS
        Print(L"Booting %s...\n", config.entries[selected_entry].name);
    } else {
        // No valid entry selected, show setup wizard
        ui_show_setup_wizard(&config);
    }
    
    // Wait for user input before exiting
    Print(L"Press any key to continue...\n");
    WaitForSingleEvent(ST->ConIn->WaitForKey, 0);
    
    // Cleanup
    uefi_hal_cleanup();
    
    return EFI_SUCCESS;
}
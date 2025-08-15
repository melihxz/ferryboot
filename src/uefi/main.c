// FerryBoot UEFI Bootloader
// Entry point for UEFI systems

#include <efi.h>
#include <efilib.h>

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    // Initialize UEFI library
    InitializeLib(ImageHandle, SystemTable);
    
    // Print welcome message
    Print(L"FerryBoot v0.1 - UEFI Mode\n");
    Print(L"Initializing bootloader...\n");
    
    // TODO: Detect hardware
    // TODO: Load configuration
    // TODO: Show boot menu
    // TODO: Load OS
    
    // For now, just print a message
    Print(L"Bootloader initialized successfully.\n");
    
    // Wait for user input before exiting
    Print(L"Press any key to continue...\n");
    WaitForSingleEvent(ST->ConIn->WaitForKey, 0);
    
    return EFI_SUCCESS;
}
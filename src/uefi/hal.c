#include "hal.h"
#include "../common/memory.h"
#include "../common/string.h"

// Global UEFI HAL instance
static uefi_hal_t uefi_hal_instance;
hardware_interface_t* g_hal = NULL;

// UEFI storage implementation
static int uefi_read_sector(uint64_t lba, void* buffer, size_t count) {
    if (!uefi_hal_instance.block_io) {
        return -1;
    }
    
    EFI_STATUS status = uefi_hal_instance.block_io->ReadBlocks(
        uefi_hal_instance.block_io,
        uefi_hal_instance.block_io->Media->MediaId,
        lba,
        count * 512,
        buffer
    );
    
    return (status == EFI_SUCCESS) ? 0 : -1;
}

static int uefi_write_sector(uint64_t lba, const void* buffer, size_t count) {
    if (!uefi_hal_instance.block_io) {
        return -1;
    }
    
    EFI_STATUS status = uefi_hal_instance.block_io->WriteBlocks(
        uefi_hal_instance.block_io,
        uefi_hal_instance.block_io->Media->MediaId,
        lba,
        count * 512,
        (void*)buffer
    );
    
    return (status == EFI_SUCCESS) ? 0 : -1;
}

// UEFI display implementation
static void uefi_set_video_mode(uint32_t width, uint32_t height, uint32_t depth) {
    if (!uefi_hal_instance.gop) {
        return;
    }
    
    // Find matching mode
    for (uint32_t i = 0; i < uefi_hal_instance.gop->Mode->MaxMode; i++) {
        EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info;
        UINTN size;
        
        EFI_STATUS status = uefi_hal_instance.gop->QueryMode(uefi_hal_instance.gop, i, &size, &info);
        if (status == EFI_SUCCESS) {
            if (info->HorizontalResolution == width && 
                info->VerticalResolution == height &&
                info->PixelFormat == PixelBlueGreenRedReserved8BitPerColor) {
                uefi_hal_instance.gop->SetMode(uefi_hal_instance.gop, i);
                break;
            }
        }
    }
}

static void uefi_put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (!uefi_hal_instance.gop) {
        return;
    }
    
    if (x >= uefi_hal_instance.gop->Mode->Info->HorizontalResolution || 
        y >= uefi_hal_instance.gop->Mode->Info->VerticalResolution) {
        return;
    }
    
    UINT32* framebuffer = (UINT32*)uefi_hal_instance.gop->Mode->FrameBufferBase;
    UINTN pixels_per_line = uefi_hal_instance.gop->Mode->Info->PixelsPerScanLine;
    
    framebuffer[y * pixels_per_line + x] = color;
}

static void uefi_clear_screen(uint32_t color) {
    if (!uefi_hal_instance.gop) {
        return;
    }
    
    UINT32* framebuffer = (UINT32*)uefi_hal_instance.gop->Mode->FrameBufferBase;
    UINTN total_pixels = uefi_hal_instance.gop->Mode->Info->HorizontalResolution * 
                         uefi_hal_instance.gop->Mode->Info->VerticalResolution;
    
    for (UINTN i = 0; i < total_pixels; i++) {
        framebuffer[i] = color;
    }
}

// UEFI input implementation
static bool uefi_key_pressed(void) {
    if (!uefi_hal_instance.system_table || !uefi_hal_instance.system_table->ConIn) {
        return false;
    }
    
    return (uefi_hal_instance.system_table->ConIn->WaitForKey != NULL);
}

static uint32_t uefi_get_key(void) {
    if (!uefi_hal_instance.system_table || !uefi_hal_instance.system_table->ConIn) {
        return 0;
    }
    
    EFI_INPUT_KEY key;
    EFI_STATUS status = uefi_hal_instance.system_table->ConIn->ReadKeyStroke(
        uefi_hal_instance.system_table->ConIn, &key
    );
    
    if (status == EFI_SUCCESS) {
        // Return Unicode character if available, otherwise scan code
        return key.UnicodeChar ? key.UnicodeChar : (key.ScanCode << 8);
    }
    
    return 0;
}

static bool uefi_mouse_moved(void) {
    if (!uefi_hal_instance.pointer) {
        return false;
    }
    
    return (uefi_hal_instance.pointer->WaitForInput != NULL);
}

static void uefi_get_mouse_state(int32_t* x, int32_t* y, uint32_t* buttons) {
    if (!uefi_hal_instance.pointer || !x || !y || !buttons) {
        if (x) *x = 0;
        if (y) *y = 0;
        if (buttons) *buttons = 0;
        return;
    }
    
    EFI_SIMPLE_POINTER_STATE state;
    EFI_STATUS status = uefi_hal_instance.pointer->GetState(uefi_hal_instance.pointer, &state);
    
    if (status == EFI_SUCCESS) {
        *x = (int32_t)state.RelativeMovementX;
        *y = (int32_t)state.RelativeMovementY;
        *buttons = state.LeftButton ? 1 : (state.RightButton ? 2 : 0);
    } else {
        *x = 0;
        *y = 0;
        *buttons = 0;
    }
}

// UEFI timer implementation
static uint64_t uefi_get_ticks(void) {
    if (!uefi_hal_instance.system_table || !uefi_hal_instance.system_table->RuntimeServices) {
        return 0;
    }
    
    EFI_TIME time;
    EFI_STATUS status = uefi_hal_instance.system_table->RuntimeServices->GetTime(&time, NULL);
    
    if (status == EFI_SUCCESS) {
        // Convert to seconds since epoch (approximate)
        return ((uint64_t)time.Year - 1970) * 365 * 24 * 60 * 60 +
               ((uint64_t)time.Month * 30 * 24 * 60 * 60) +
               ((uint64_t)time.Day * 24 * 60 * 60) +
               ((uint64_t)time.Hour * 60 * 60) +
               ((uint64_t)time.Minute * 60) +
               (uint64_t)time.Second;
    }
    
    return 0;
}

static void uefi_sleep(uint32_t milliseconds) {
    if (!uefi_hal_instance.system_table || !uefi_hal_instance.system_table->BootServices) {
        return;
    }
    
    uefi_hal_instance.system_table->BootServices->Stall(milliseconds * 1000);
}

// UEFI memory implementation
static void* uefi_alloc(size_t size) {
    if (!uefi_hal_instance.system_table || !uefi_hal_instance.system_table->BootServices) {
        return NULL;
    }
    
    void* buffer = NULL;
    EFI_STATUS status = uefi_hal_instance.system_table->BootServices->AllocatePool(
        EfiLoaderData, size, &buffer
    );
    
    return (status == EFI_SUCCESS) ? buffer : NULL;
}

static void uefi_free(void* ptr) {
    if (!uefi_hal_instance.system_table || !uefi_hal_instance.system_table->BootServices || !ptr) {
        return;
    }
    
    uefi_hal_instance.system_table->BootServices->FreePool(ptr);
}

// UEFI network implementation (placeholder)
static int uefi_network_init(void) {
    // Placeholder implementation
    return 0;
}

static int uefi_dhcp_request(network_config_t* config) {
    // Placeholder implementation
    return 0;
}

static int uefi_tftp_download(const char* server, const char* filename, void* buffer, size_t* size) {
    // Placeholder implementation
    return 0;
}

// Initialize UEFI HAL
int uefi_hal_init(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* system_table) {
    // Initialize UEFI library
    InitializeLib(image_handle, system_table);
    
    // Store handles
    uefi_hal_instance.image_handle = image_handle;
    uefi_hal_instance.system_table = system_table;
    uefi_hal_instance.gop = NULL;
    uefi_hal_instance.pointer = NULL;
    uefi_hal_instance.block_io = NULL;
    
    // Initialize UEFI protocols
    // Graphics Output Protocol
    uefi_boot_services->LocateProtocol(
        &GraphicsOutputProtocol, NULL, (VOID**)&uefi_hal_instance.gop
    );
    
    // Simple Pointer Protocol (mouse)
    uefi_boot_services->LocateProtocol(
        &SimplePointerProtocol, NULL, (VOID**)&uefi_hal_instance.pointer
    );
    
    // Block I/O Protocol (storage)
    EFI_HANDLE* handles;
    UINTN handle_count;
    EFI_STATUS status = uefi_boot_services->LocateHandleBuffer(
        ByProtocol, &BlockIoProtocol, NULL, &handle_count, &handles
    );
    
    if (status == EFI_SUCCESS && handle_count > 0) {
        // Use the first block device
        uefi_boot_services->HandleProtocol(
            handles[0], &BlockIoProtocol, (VOID**)&uefi_hal_instance.block_io
        );
        uefi_boot_services->FreePool(handles);
    }
    
    // Set up HAL function pointers
    uefi_hal_instance.base.read_sector = uefi_read_sector;
    uefi_hal_instance.base.write_sector = uefi_write_sector;
    uefi_hal_instance.base.set_video_mode = uefi_set_video_mode;
    uefi_hal_instance.base.put_pixel = uefi_put_pixel;
    uefi_hal_instance.base.clear_screen = uefi_clear_screen;
    uefi_hal_instance.base.key_pressed = uefi_key_pressed;
    uefi_hal_instance.base.get_key = uefi_get_key;
    uefi_hal_instance.base.mouse_moved = uefi_mouse_moved;
    uefi_hal_instance.base.get_mouse_state = uefi_get_mouse_state;
    uefi_hal_instance.base.network_init = uefi_network_init;
    uefi_hal_instance.base.dhcp_request = uefi_dhcp_request;
    uefi_hal_instance.base.tftp_download = uefi_tftp_download;
    uefi_hal_instance.base.get_ticks = uefi_get_ticks;
    uefi_hal_instance.base.sleep = uefi_sleep;
    uefi_hal_instance.base.alloc = uefi_alloc;
    uefi_hal_instance.base.free = uefi_free;
    
    // Set global HAL pointer
    g_hal = &uefi_hal_instance.base;
    
    return 0;
}

// Cleanup UEFI HAL
void uefi_hal_cleanup(void) {
    g_hal = NULL;
    memory_set(&uefi_hal_instance, 0, sizeof(uefi_hal_instance));
}
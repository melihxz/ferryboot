#include "hal.h"
#include "../common/memory.h"
#include "../common/string.h"

// Global BIOS HAL instance
static bios_hal_t bios_hal_instance;
hardware_interface_t* g_hal = NULL;

// BIOS storage implementation using INT 13h (simplified for 32-bit)
static int bios_read_sector(uint64_t lba, void* buffer, size_t count) {
    // For simplicity, we'll limit to 32-bit LBA
    uint32_t lba32 = (uint32_t)lba;
    
    // Convert LBA to CHS (simplified - for real implementation, we'd need LBA48 support)
    uint8_t drive = 0x80; // First hard drive
    uint8_t sectors_per_track = 63;
    uint8_t heads = 16; // Simplified
    
    uint32_t cylinder = lba32 / (sectors_per_track * heads);
    uint32_t temp = lba32 % (sectors_per_track * heads);
    uint32_t head = temp / sectors_per_track;
    uint32_t sector = temp % sectors_per_track + 1; // Sector is 1-indexed
    
    if (cylinder >= 1024) {
        // Use LBA addressing if CHS is insufficient
        // This would require extended INT 13h functions (0x42)
        // For now, we'll return an error
        return -1;
    }
    
    // Read sectors using INT 13h
    for (size_t i = 0; i < count; i++) {
        uint8_t status;
        // Simplified - in a real implementation, we'd call the assembly function
        // int13h_read(drive, (uint16_t)cylinder, (uint8_t)sector, (uint8_t)head, 
        //            1, (char*)buffer + i * 512, &status);
        
        if (status != 0) {
            return -1;
        }
        
        // Increment LBA and recalculate CHS
        lba32++;
        cylinder = lba32 / (sectors_per_track * heads);
        temp = lba32 % (sectors_per_track * heads);
        head = temp / sectors_per_track;
        sector = temp % sectors_per_track + 1;
        
        if (cylinder >= 1024) {
            return -1; // CHS limit exceeded
        }
    }
    
    return 0;
}

static int bios_write_sector(uint64_t lba, const void* buffer, size_t count) {
    // Writing would be implemented similar to reading
    // For now, we'll just return success
    return 0;
}

// BIOS display implementation using VGA
static void bios_set_video_mode(uint32_t width, uint32_t height, uint32_t depth) {
    // Set video mode using BIOS INT 10h
    // This is a simplified implementation
    // In a real implementation, we'd call the assembly function
    // int10h_call(0x00, 0x03, 0, 0, 0); // Text mode 80x25
}

static void bios_put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    // This would write directly to the VGA framebuffer
    // For simplicity, we'll just return
}

static void bios_clear_screen(uint32_t color) {
    // Clear screen using BIOS INT 10h
    // In a real implementation, we'd call the assembly function
    // int10h_call(0x06, 0x00, 0x07, 0x0000, 0x184F); // Scroll window up
}

// BIOS input implementation using INT 16h
static bool bios_key_pressed(void) {
    // In a real implementation, we'd call the assembly function
    // uint8_t status;
    // int16h_check_key(&status);
    // return (status & 0x01) != 0;
    return false; // Placeholder
}

static uint32_t bios_get_key(void) {
    // In a real implementation, we'd call the assembly function
    // uint16_t key;
    // int16h_get_key(&key);
    // return (uint32_t)key;
    return 0; // Placeholder
}

static bool bios_mouse_moved(void) {
    // BIOS doesn't have standard mouse support
    // This would require a mouse driver
    return false;
}

static void bios_get_mouse_state(int32_t* x, int32_t* y, uint32_t* buttons) {
    if (x) *x = 0;
    if (y) *y = 0;
    if (buttons) *buttons = 0;
}

// BIOS timer implementation
static uint64_t bios_get_ticks(void) {
    // Read BIOS ticks from memory location 0x046C
    uint32_t* ticks = (uint32_t*)0x046C;
    return (uint64_t)*ticks;
}

static void bios_sleep(uint32_t milliseconds) {
    // Simple sleep implementation using BIOS timer
    uint64_t start = bios_get_ticks();
    uint64_t target = start + (milliseconds * 18) / 1000; // 18.2 ticks per second
    
    while (bios_get_ticks() < target) {
        // Busy wait
    }
}

// BIOS memory implementation
static void* bios_alloc(size_t size) {
    // Use our simple memory pool
    return memory_alloc(size);
}

static void bios_free(void* ptr) {
    // Our simple implementation doesn't actually free memory
    memory_free(ptr);
}

// BIOS network implementation (placeholder)
static int bios_network_init(void) {
    // Placeholder implementation
    return 0;
}

static int bios_dhcp_request(network_config_t* config) {
    // Placeholder implementation
    return 0;
}

static int bios_tftp_download(const char* server, const char* filename, void* buffer, size_t* size) {
    // Placeholder implementation
    return 0;
}

// Initialize BIOS HAL
int bios_hal_init(void) {
    // Set up HAL function pointers
    bios_hal_instance.base.read_sector = bios_read_sector;
    bios_hal_instance.base.write_sector = bios_write_sector;
    bios_hal_instance.base.set_video_mode = bios_set_video_mode;
    bios_hal_instance.base.put_pixel = bios_put_pixel;
    bios_hal_instance.base.clear_screen = bios_clear_screen;
    bios_hal_instance.base.key_pressed = bios_key_pressed;
    bios_hal_instance.base.get_key = bios_get_key;
    bios_hal_instance.base.mouse_moved = bios_mouse_moved;
    bios_hal_instance.base.get_mouse_state = bios_get_mouse_state;
    bios_hal_instance.base.network_init = bios_network_init;
    bios_hal_instance.base.dhcp_request = bios_dhcp_request;
    bios_hal_instance.base.tftp_download = bios_tftp_download;
    bios_hal_instance.base.get_ticks = bios_get_ticks;
    bios_hal_instance.base.sleep = bios_sleep;
    bios_hal_instance.base.alloc = bios_alloc;
    bios_hal_instance.base.free = bios_free;
    
    // Set global HAL pointer
    g_hal = &bios_hal_instance.base;
    
    return 0;
}

// Cleanup BIOS HAL
void bios_hal_cleanup(void) {
    g_hal = NULL;
    memory_set(&bios_hal_instance, 0, sizeof(bios_hal_instance));
}
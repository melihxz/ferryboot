#ifndef FERRYBOOT_HAL_H
#define FERRYBOOT_HAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Hardware interface
typedef struct {
    // Storage functions
    int (*read_sector)(uint64_t lba, void* buffer, size_t count);
    int (*write_sector)(uint64_t lba, const void* buffer, size_t count);
    
    // Display functions
    void (*set_video_mode)(uint32_t width, uint32_t height, uint32_t depth);
    void (*put_pixel)(uint32_t x, uint32_t y, uint32_t color);
    void (*clear_screen)(uint32_t color);
    
    // Input functions
    bool (*key_pressed)(void);
    uint32_t (*get_key)(void);
    bool (*mouse_moved)(void);
    void (*get_mouse_state)(int32_t* x, int32_t* y, uint32_t* buttons);
    
    // Network functions
    int (*network_init)(void);
    int (*dhcp_request)(network_config_t* config);
    int (*tftp_download)(const char* server, const char* filename, void* buffer, size_t* size);
    
    // Timer functions
    uint64_t (*get_ticks)(void);
    void (*sleep)(uint32_t milliseconds);
    
    // Memory functions
    void* (*alloc)(size_t size);
    void (*free)(void* ptr);
} hardware_interface_t;

// Global HAL instance
extern hardware_interface_t* g_hal;

// HAL initialization
int hal_init(void);

// Inline functions for performance
static inline int hal_read_sector(uint64_t lba, void* buffer, size_t count) {
    return g_hal->read_sector(lba, buffer, count);
}

static inline void hal_put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    g_hal->put_pixel(x, y, color);
}

static inline bool hal_key_pressed(void) {
    return g_hal->key_pressed();
}

static inline uint32_t hal_get_key(void) {
    return g_hal->get_key();
}

#endif // FERRYBOOT_HAL_H
// FerryBoot Stage 2 - Main Bootloader
// Written in C for BIOS systems

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../common/string.h"
#include "../common/config.h"
#include "../common/module.h"
#include "../common/ui.h"
#include "hal.h"

// VGA text mode dimensions
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_BUFFER 0xB8000

// Hardware ports
#define VGA_COMMAND_PORT 0x3D4
#define VGA_DATA_PORT 0x3D5

// Terminal colors
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

// Global variables
static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

// Function prototypes
void terminal_initialize(void);
void terminal_setcolor(uint8_t color);
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);

// Helper functions
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg);
static inline uint16_t vga_entry(unsigned char uc, uint8_t color);

// Kernel entry point
void _start(void) {
    // Initialize terminal
    terminal_initialize();
    
    // Initialize BIOS HAL
    if (bios_hal_init() != 0) {
        terminal_writestring("Error: Failed to initialize BIOS HAL\n");
        for (;;);
    }
    
    // Print welcome message
    terminal_writestring("FerryBoot v0.1\n");
    terminal_writestring("Initializing bootloader...\n");
    
    // Load configuration
    config_t config;
    if (config_load(&config) != 0) {
        terminal_writestring("Warning: Failed to load configuration, using defaults\n");
        config_init_defaults(&config);
    }
    
    // Initialize module system
    if (module_init_all() != 0) {
        terminal_writestring("Error: Failed to initialize modules\n");
        for (;;);
    }
    
    // Initialize UI
    if (ui_init(config.gui_mode) != 0) {
        terminal_writestring("Error: Failed to initialize UI\n");
        for (;;);
    }
    
    // Show boot menu
    int selected_entry = ui_show_boot_menu(&config);
    if (selected_entry >= 0 && selected_entry < (int)config.entry_count) {
        // Boot selected OS
        // In a real implementation, this would load and boot the OS
        terminal_writestring("Booting ");
        terminal_writestring(config.entries[selected_entry].name);
        terminal_writestring("...\n");
    } else {
        // No valid entry selected, show setup wizard
        ui_show_setup_wizard(&config);
    }
    
    // For now, just print a message and hang
    terminal_writestring("Bootloader initialized successfully.\n");
    
    // Hang indefinitely
    for (;;);
}

// Terminal functions
void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = (uint16_t*) VGA_BUFFER;
    
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
}

void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_row = 0;
        }
        return;
    }

    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    
    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_row = 0;
        }
    }
}

void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
    terminal_write(data, string_length(data));
}

// Helper functions
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}
#include "ui.h"
#include "../common/memory.h"
#include "../common/string.h"

// Text UI implementation
static bool gui_mode_enabled = false;

// Initialize UI
int ui_init(bool gui_mode) {
    gui_mode_enabled = gui_mode;
    return 0;
}

// Show boot menu in text mode
int ui_show_boot_menu(const config_t* config) {
    // In a real implementation, this would display the boot menu
    // For now, we'll just return the default entry
    return config->default_entry;
}

// Show setup wizard
int ui_show_setup_wizard(config_t* config) {
    // In a real implementation, this would show the setup wizard
    // For now, we'll just return success
    return 0;
}

// Process input event
int ui_process_event(const input_event_t* event) {
    // In a real implementation, this would process input events
    // For now, we'll just return success
    return 0;
}

// Draw text (placeholder)
void ui_draw_text(const char* text, uint32_t x, uint32_t y, uint32_t color) {
    // In a real implementation, this would draw text to the screen
    // For text mode, this might use BIOS calls
    // For GUI mode, this would draw to the framebuffer
}

// Draw rectangle (placeholder)
void ui_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    // In a real implementation, this would draw a rectangle to the screen
    // For GUI mode, this would draw to the framebuffer
}
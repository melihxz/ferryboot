#include "ui.h"
#include "gui.h"
#include "hal.h"
#include "../common/config.h"
#include "../common/memory.h"
#include "../common/string.h"

// Global UI state
static bool gui_mode_enabled = false;
static gui_component_t* boot_menu_buttons[32];
static uint32_t button_count = 0;

// Forward declarations
static void on_boot_entry_selected(void);

// Initialize UI
int ui_init(bool gui_mode) {
    gui_mode_enabled = gui_mode;
    
    if (gui_mode && g_hal) {
        // Initialize GUI mode
        return gui_init();
    }
    
    // Text mode initialization (if needed)
    return 0;
}

// Show boot menu in GUI mode
int ui_show_boot_menu(const config_t* config) {
    if (!gui_mode_enabled || !g_hal) {
        // Fallback to text mode
        return config->default_entry;
    }
    
    // Initialize GUI
    if (gui_init() != 0) {
        return config->default_entry;
    }
    
    // Clear screen
    gui_clear_screen(GUI_LIGHTGRAY);
    
    // Create header
    gui_create_label("FerryBoot v0.1", 400, 50, GUI_BLUE);
    gui_create_label("Select operating system:", 50, 100, GUI_GRAY);
    
    // Create buttons for each boot entry
    button_count = 0;
    for (uint32_t i = 0; i < config->entry_count && i < 32; i++) {
        if (config->entries[i].enabled) {
            char button_text[128];
            size_t name_len = string_length(config->entries[i].name);
            if (name_len >= sizeof(button_text)) {
                name_len = sizeof(button_text) - 1;
            }
            memory_copy(button_text, config->entries[i].name, name_len);
            button_text[name_len] = '\0';
            
            boot_menu_buttons[button_count] = gui_create_button(
                button_text, 
                100, 
                150 + button_count * 60, 
                400, 
                50, 
                on_boot_entry_selected
            );
            
            if (boot_menu_buttons[button_count]) {
                button_count++;
            }
        }
    }
    
    // Add action buttons
    gui_create_button("Settings", 100, 150 + button_count * 60, 150, 40, NULL);
    gui_create_button("Power Off", 300, 150 + button_count * 60, 150, 40, NULL);
    
    // Render initial screen
    gui_render();
    
    // Main event loop
    while (1) {
        int action = gui_process_input();
        if (action == 1) {
            // Action performed - for now, just return first entry
            gui_cleanup();
            return 0;
        } else if (action == 2) {
            // Selection changed - re-render
            gui_render();
        }
        
        // Small delay to prevent excessive CPU usage
        if (g_hal && g_hal->sleep) {
            g_hal->sleep(10);
        }
    }
    
    gui_cleanup();
    return config->default_entry;
}

// Show setup wizard
int ui_show_setup_wizard(config_t* config) {
    if (!gui_mode_enabled || !g_hal) {
        // Fallback to text mode
        return 0;
    }
    
    // Initialize GUI
    if (gui_init() != 0) {
        return -1;
    }
    
    // Clear screen
    gui_clear_screen(GUI_LIGHTGRAY);
    
    // Create setup wizard UI
    gui_create_label("FerryBoot Setup Wizard", 300, 50, GUI_BLUE);
    gui_create_label("Welcome to FerryBoot!", 100, 100, GUI_BLACK);
    gui_create_label("This wizard will help you configure your bootloader.", 100, 130, GUI_GRAY);
    
    // Add buttons
    gui_create_button("Quick Setup", 100, 200, 200, 50, NULL);
    gui_create_button("Advanced Setup", 350, 200, 200, 50, NULL);
    
    // Render screen
    gui_render();
    
    // Main event loop
    while (1) {
        int action = gui_process_input();
        if (action == 1) {
            // Action performed
            break;
        } else if (action == 2) {
            // Selection changed - re-render
            gui_render();
        }
        
        // Small delay to prevent excessive CPU usage
        if (g_hal && g_hal->sleep) {
            g_hal->sleep(10);
        }
    }
    
    gui_cleanup();
    return 0;
}

// Process input event
int ui_process_event(const input_event_t* event) {
    // In GUI mode, input is processed in the main loop
    // In text mode, this would process individual events
    return 0;
}

// Draw text
void ui_draw_text(const char* text, uint32_t x, uint32_t y, uint32_t color) {
    if (gui_mode_enabled) {
        gui_draw_text(text, x, y, color);
    }
    // Text mode implementation would go here
}

// Draw rectangle
void ui_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    if (gui_mode_enabled) {
        gui_draw_rect(x, y, width, height, color);
    }
    // Text mode implementation would go here
}

// Callback for boot entry selection
static void on_boot_entry_selected(void) {
    // This function would be called when a boot entry is selected
    // For now, it's just a placeholder
}
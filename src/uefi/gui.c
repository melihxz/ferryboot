#include "gui.h"
#include "hal.h"
#include "../common/memory.h"
#include "../common/string.h"

// Global GUI context
static gui_context_t gui_ctx;

// Initialize GUI
int gui_init(void) {
    // Initialize context
    memory_set(&gui_ctx, 0, sizeof(gui_ctx));
    
    // Get screen dimensions from GOP
    if (uefi_hal_instance.gop) {
        gui_ctx.screen_width = uefi_hal_instance.gop->Mode->Info->HorizontalResolution;
        gui_ctx.screen_height = uefi_hal_instance.gop->Mode->Info->VerticalResolution;
    } else {
        gui_ctx.screen_width = 1024;
        gui_ctx.screen_height = 768;
    }
    
    return 0;
}

// Cleanup GUI
void gui_cleanup(void) {
    // Free all components
    gui_component_t* current = gui_ctx.components;
    while (current) {
        gui_component_t* next = current->next;
        memory_free(current);
        current = next;
    }
    
    memory_set(&gui_ctx, 0, sizeof(gui_ctx));
}

// Clear screen
void gui_clear_screen(uint32_t color) {
    if (g_hal) {
        g_hal->clear_screen(color);
    }
}

// Draw rectangle
void gui_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    if (!g_hal || !g_hal->put_pixel) {
        return;
    }
    
    // Clip to screen bounds
    if (x >= gui_ctx.screen_width || y >= gui_ctx.screen_height) {
        return;
    }
    
    uint32_t end_x = x + width;
    uint32_t end_y = y + height;
    
    if (end_x > gui_ctx.screen_width) {
        end_x = gui_ctx.screen_width;
    }
    
    if (end_y > gui_ctx.screen_height) {
        end_y = gui_ctx.screen_height;
    }
    
    // Draw filled rectangle
    for (uint32_t py = y; py < end_y; py++) {
        for (uint32_t px = x; px < end_x; px++) {
            g_hal->put_pixel(px, py, color);
        }
    }
}

// Draw text (simple implementation)
void gui_draw_text(const char* text, uint32_t x, uint32_t y, uint32_t color) {
    if (!g_hal || !g_hal->put_pixel || !text) {
        return;
    }
    
    // Simple character rendering - each character is 8x16 pixels
    for (size_t i = 0; text[i] != '\0'; i++) {
        char c = text[i];
        uint32_t char_x = x + i * 8;
        
        // Skip if character is off screen
        if (char_x >= gui_ctx.screen_width || y >= gui_ctx.screen_height) {
            continue;
        }
        
        // Draw simple character representation
        // For now, just draw a filled rectangle for non-space characters
        if (c != ' ') {
            gui_draw_rect(char_x, y, 8, 16, color);
        }
    }
}

// Create button
gui_component_t* gui_create_button(const char* text, uint32_t x, uint32_t y, 
                                  uint32_t width, uint32_t height, void (*on_click)(void)) {
    gui_component_t* button = (gui_component_t*)memory_alloc(sizeof(gui_component_t));
    if (!button) {
        return NULL;
    }
    
    // Initialize button
    button->type = GUI_COMPONENT_BUTTON;
    button->id = (uint32_t)(uintptr_t)button;
    button->x = x;
    button->y = y;
    button->width = width;
    button->height = height;
    button->visible = true;
    button->enabled = true;
    button->selected = false;
    button->on_click = on_click;
    button->next = NULL;
    
    // Copy text
    size_t text_len = string_length(text);
    if (text_len >= sizeof(button->text)) {
        text_len = sizeof(button->text) - 1;
    }
    memory_copy(button->text, text, text_len);
    button->text[text_len] = '\0';
    
    // Add to component list
    if (!gui_ctx.components) {
        gui_ctx.components = button;
    } else {
        gui_component_t* last = gui_ctx.components;
        while (last->next) {
            last = last->next;
        }
        last->next = button;
    }
    
    return button;
}

// Create label
gui_component_t* gui_create_label(const char* text, uint32_t x, uint32_t y, uint32_t color) {
    gui_component_t* label = (gui_component_t*)memory_alloc(sizeof(gui_component_t));
    if (!label) {
        return NULL;
    }
    
    // Initialize label
    label->type = GUI_COMPONENT_LABEL;
    label->id = (uint32_t)(uintptr_t)label;
    label->x = x;
    label->y = y;
    label->width = 0; // Calculated based on text
    label->height = 16; // Standard height
    label->visible = true;
    label->enabled = true;
    label->selected = false;
    label->on_click = NULL;
    label->next = NULL;
    
    // Copy text
    size_t text_len = string_length(text);
    if (text_len >= sizeof(label->text)) {
        text_len = sizeof(label->text) - 1;
    }
    memory_copy(label->text, text, text_len);
    label->text[text_len] = '\0';
    
    // Calculate width
    label->width = text_len * 8;
    
    // Add to component list
    if (!gui_ctx.components) {
        gui_ctx.components = label;
    } else {
        gui_component_t* last = gui_ctx.components;
        while (last->next) {
            last = last->next;
        }
        last->next = label;
    }
    
    return label;
}

// Render GUI
void gui_render(void) {
    // Clear screen with light gray background
    gui_clear_screen(GUI_LIGHTGRAY);
    
    // Render all components
    gui_component_t* current = gui_ctx.components;
    while (current) {
        if (current->visible) {
            switch (current->type) {
                case GUI_COMPONENT_BUTTON:
                    {
                        // Draw button background
                        uint32_t button_color = current->selected ? GUI_BLUE : GUI_GRAY;
                        gui_draw_rect(current->x, current->y, current->width, current->height, button_color);
                        
                        // Draw button border
                        gui_draw_rect(current->x, current->y, current->width, 1, GUI_BLACK);
                        gui_draw_rect(current->x, current->y + current->height - 1, current->width, 1, GUI_BLACK);
                        gui_draw_rect(current->x, current->y, 1, current->height, GUI_BLACK);
                        gui_draw_rect(current->x + current->width - 1, current->y, 1, current->height, GUI_BLACK);
                        
                        // Draw button text
                        uint32_t text_color = current->enabled ? GUI_WHITE : GUI_GRAY;
                        uint32_t text_x = current->x + (current->width - string_length(current->text) * 8) / 2;
                        uint32_t text_y = current->y + (current->height - 16) / 2;
                        gui_draw_text(current->text, text_x, text_y, text_color);
                    }
                    break;
                    
                case GUI_COMPONENT_LABEL:
                    {
                        // Draw label text
                        uint32_t text_color = current->enabled ? GUI_BLACK : GUI_GRAY;
                        gui_draw_text(current->text, current->x, current->y, text_color);
                    }
                    break;
            }
        }
        current = current->next;
    }
}

// Process input
int gui_process_input(void) {
    // Process keyboard input
    if (g_hal && g_hal->key_pressed()) {
        uint32_t key = g_hal->get_key();
        if (key) {
            // Handle Enter key
            if (key == 0x0D || key == 0x0A) {
                if (gui_ctx.selected_component && gui_ctx.selected_component->on_click) {
                    gui_ctx.selected_component->on_click();
                    return 1; // Action performed
                }
            }
            // Handle arrow keys for navigation
            else if (key == 0x100) { // Up arrow
                // Navigate to previous component
                // Implementation would go here
            }
            else if (key == 0x101) { // Down arrow
                // Navigate to next component
                // Implementation would go here
            }
        }
    }
    
    // Process mouse input
    if (g_hal && g_hal->mouse_moved()) {
        int32_t mouse_x, mouse_y;
        uint32_t mouse_buttons;
        g_hal->get_mouse_state(&mouse_x, &mouse_y, &mouse_buttons);
        
        // Update mouse position
        gui_ctx.mouse_x += mouse_x;
        gui_ctx.mouse_y += mouse_y;
        
        // Clip mouse position to screen
        if (gui_ctx.mouse_x < 0) gui_ctx.mouse_x = 0;
        if (gui_ctx.mouse_y < 0) gui_ctx.mouse_y = 0;
        if (gui_ctx.mouse_x >= (int32_t)gui_ctx.screen_width) gui_ctx.mouse_x = gui_ctx.screen_width - 1;
        if (gui_ctx.mouse_y >= (int32_t)gui_ctx.screen_height) gui_ctx.mouse_y = gui_ctx.screen_height - 1;
        
        // Check if mouse is over any component
        gui_component_t* current = gui_ctx.components;
        gui_component_t* hovered_component = NULL;
        while (current) {
            if (current->visible && current->enabled) {
                if (gui_ctx.mouse_x >= (int32_t)current->x && 
                    gui_ctx.mouse_x < (int32_t)(current->x + current->width) &&
                    gui_ctx.mouse_y >= (int32_t)current->y && 
                    gui_ctx.mouse_y < (int32_t)(current->y + current->height)) {
                    hovered_component = current;
                    break;
                }
            }
            current = current->next;
        }
        
        // Update selected component
        if (hovered_component && hovered_component != gui_ctx.selected_component) {
            if (gui_ctx.selected_component) {
                gui_ctx.selected_component->selected = false;
            }
            gui_ctx.selected_component = hovered_component;
            gui_ctx.selected_component->selected = true;
            return 2; // Selection changed
        }
        
        // Handle mouse click
        if (mouse_buttons && !gui_ctx.mouse_buttons) {
            // Mouse button pressed
            if (gui_ctx.selected_component && gui_ctx.selected_component->on_click) {
                gui_ctx.selected_component->on_click();
                return 1; // Action performed
            }
        }
        
        gui_ctx.mouse_buttons = mouse_buttons;
    }
    
    return 0; // No action
}
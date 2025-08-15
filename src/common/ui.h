#ifndef FERRYBOOT_UI_H
#define FERRYBOOT_UI_H

#include "config.h"

// Input event
typedef struct {
    uint32_t type;      // KEYBOARD, MOUSE, TOUCH
    uint32_t code;      // Key code, button, etc.
    uint32_t x, y;      // Coordinates for mouse/touch
    uint32_t modifiers; // Shift, Ctrl, Alt
} input_event_t;

// Menu item
typedef struct {
    char text[64];
    uint32_t id;
    bool enabled;
    void (*on_select)(void);
} menu_item_t;

// Menu
typedef struct {
    char title[64];
    menu_item_t items[32];
    uint32_t item_count;
    uint32_t selected_item;
} menu_t;

// UI API
int ui_init(bool gui_mode);
int ui_show_boot_menu(const config_t* config);
int ui_show_setup_wizard(config_t* config);
int ui_process_event(const input_event_t* event);
void ui_draw_text(const char* text, uint32_t x, uint32_t y, uint32_t color);
void ui_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);

#endif // FERRYBOOT_UI_H
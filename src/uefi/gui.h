#ifndef FERRYBOOT_UEFI_GUI_H
#define FERRYBOOT_UEFI_GUI_H

#include <efi.h>
#include <efilib.h>
#include "../common/ui.h"

// GUI color definitions
#define GUI_BLACK     0x000000
#define GUI_WHITE     0xFFFFFF
#define GUI_BLUE      0x2563EB
#define GUI_GRAY      0x64748B
#define GUI_LIGHTGRAY 0xF8FAFC
#define GUI_GREEN     0x10B981
#define GUI_RED       0xEF4444

// GUI component types
#define GUI_COMPONENT_BUTTON 1
#define GUI_COMPONENT_LABEL  2
#define GUI_COMPONENT_MENU   3

// GUI component structure
typedef struct gui_component {
    uint32_t type;
    uint32_t id;
    uint32_t x, y, width, height;
    char text[128];
    bool visible;
    bool enabled;
    bool selected;
    void (*on_click)(void);
    struct gui_component* next;
} gui_component_t;

// GUI context
typedef struct {
    uint32_t screen_width;
    uint32_t screen_height;
    gui_component_t* components;
    gui_component_t* selected_component;
    int32_t mouse_x;
    int32_t mouse_y;
    uint32_t mouse_buttons;
} gui_context_t;

// GUI functions
int gui_init(void);
void gui_cleanup(void);
void gui_clear_screen(uint32_t color);
void gui_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void gui_draw_text(const char* text, uint32_t x, uint32_t y, uint32_t color);
gui_component_t* gui_create_button(const char* text, uint32_t x, uint32_t y, 
                                  uint32_t width, uint32_t height, void (*on_click)(void));
gui_component_t* gui_create_label(const char* text, uint32_t x, uint32_t y, uint32_t color);
void gui_render(void);
int gui_process_input(void);

#endif // FERRYBOOT_UEFI_GUI_H
# FerryBoot User Interface Design

## Overview

FerryBoot features a dual-mode user interface that provides both a simple text-based interface for legacy systems and a modern graphical interface for contemporary hardware. The interface is designed to be intuitive and accessible to users of all technical levels.

## Design Principles

### Minimalism
- Clean, uncluttered interface
- Light color palette (blues, grays, whites)
- Ample whitespace
- Clear typography hierarchy

### Accessibility
- Keyboard navigation
- Mouse support
- High contrast modes
- Large touch targets

### Responsiveness
- Fast rendering
- Minimal animation
- Immediate feedback
- Smooth transitions

## Interface Modes

### Text Mode (CLI)
- 80x25 character grid
- VGA text mode colors
- Keyboard-only navigation
- Low resource usage

### Graphical Mode (GUI)
- High-resolution framebuffer
- Custom rendering engine
- Mouse and keyboard navigation
- Icon-based interface

## Main Menu

The main menu is the central hub of the bootloader interface:

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                                                                             │
│  ╔═════════════════════════════════════════════════════════════════════╗  │
│  ║                                                                     ║  │
│  ║                        ███████╗███████╗██████╗  ██╗   ██╗██████╗    ║  │
│  ║                        ██╔════╝██╔════╝██╔══██╗╚██╗ ██╔╝██╔══██╗   ║  │
│  ║                        █████╗  █████╗  ██████╔╝ ╚████╔╝ ██████╔╝   ║  │
│  ║                        ██╔══╝  ██╔══╝  ██╔══██╗  ╚██╔╝  ██╔══██╗   ║  │
│  ║                        ██║     ███████╗██████╔╝   ██║   ██████╔╝   ║  │
│  ║                        ╚═╝     ╚══════╝╚═════╝    ╚═╝   ╚═════╝    ║  │
│  ║                                                                     ║  │
│  ║                        Advanced Bootloader v0.1                   ║  │
│  ║                                                                     ║  │
│  ╚═════════════════════════════════════════════════════════════════════╝  │
│                                                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  [1] Windows 11              🪟  Last used: Today, 14:30             │  │
│  │  [2] Ubuntu 22.04 LTS        🐧  Last used: Today, 10:15             │  │
│  │  [3] Fedora 38               🐧  Last used: Yesterday, 16:45         │  │
│  │  [4] macOS Ventura           🍏  Last used: Yesterday, 09:20         │  │
│  │  [5] FreeBSD 13.2            🟦  Last used: 2 days ago               │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
│  [S] Settings     [P] Power Off     [R] Reboot     [E] Emergency Console   │
│                                                                             │
│  Automatic boot in 10 seconds...                                            │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Settings Interface

The settings interface allows users to configure bootloader behavior:

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                          FerryBoot Settings                            │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  General Settings                                                           │
│  [1] Boot Timeout: 10 seconds                                               │
│  [2] Default OS: Windows 11                                                 │
│  [3] Quiet Boot: Enabled                                                    │
│                                                                             │
│  Display Settings                                                           │
│  [4] Resolution: Auto (1920x1080)                                           │
│  [5] Theme: Light                                                           │
│  [6] GUI Mode: Enabled                                                      │
│                                                                             │
│  Security Settings                                                          │
│  [7] Password Protection: Disabled                                          │
│  [8] Secure Boot: Enabled                                                   │
│  [9] Kernel Verification: Enabled                                           │
│                                                                             │
│  Network Settings                                                           │
│  [10] PXE Boot: Disabled                                                    │
│                                                                             │
│  [B] Back     [S] Save     [R] Reset to Defaults                           │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Setup Wizard

The setup wizard guides users through initial configuration:

### Welcome Screen
```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        Welcome to FerryBoot                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Thank you for choosing FerryBoot!                                          │
│                                                                             │
│  This wizard will help you configure your bootloader in just a few steps.   │
│                                                                             │
│  [ ] Quick Setup (Recommended for most users)                               │
│  [ ] Advanced Setup (For experienced users)                                 │
│                                                                             │
│  Press ENTER to continue                                                    │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### Quick Setup
```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         Quick Setup Wizard                              │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  FerryBoot has detected the following operating systems:                    │
│                                                                             │
│  [✓] Windows 11 (C:\)                                                       │
│  [✓] Ubuntu 22.04 LTS (/dev/sda2)                                           │
│                                                                             │
│  Would you like to:                                                         │
│                                                                             │
│  [1] Set Windows 11 as the default OS                                       │
│  [2] Set Ubuntu 22.04 LTS as the default OS                                 │
│  [3] Choose default OS manually                                             │
│                                                                             │
│  Additional options:                                                        │
│  [ ] Enable password protection                                             │
│  [ ] Enable secure boot                                                     │
│                                                                             │
│  [C] Continue     [B] Back                                                  │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Graphical Interface

The graphical interface provides a modern, icon-based experience:

### Main Screen
- Full-screen background with subtle gradient
- OS entries displayed as cards with icons
- Progress bar showing boot timeout
- Navigation bar at the bottom

### Visual Design Elements
- Rounded corners for UI elements
- Subtle shadows for depth
- Consistent iconography
- Responsive layout

### Color Palette
- Primary: #2563EB (Blue)
- Secondary: #64748B (Gray)
- Background: #F8FAFC (Light Gray)
- Text: #1E293B (Dark Gray)
- Accent: #10B981 (Green for success)

## Input Methods

### Keyboard Navigation
- Arrow keys for menu navigation
- Enter to select
- Escape to go back
- Shortcut keys for common actions

### Mouse Navigation
- Click to select items
- Scroll wheel for lists
- Right-click for context menu
- Hover for tooltips

### Touch Navigation
- Tap to select
- Swipe gestures
- Pinch to zoom (in file browser)
- Long press for context menu

## Error Handling UI

When errors occur, FerryBoot displays user-friendly error messages:

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           Boot Error                                   │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Failed to load Ubuntu 22.04 LTS                                            │
│                                                                             │
│  Error: File not found (/boot/vmlinuz-5.15.0-76-generic)                    │
│                                                                             │
│  [1] Try Again                                                              │
│  [2] Select Another OS                                                      │
│  [3] Emergency Console                                                      │
│  [4] Reboot System                                                          │
│                                                                             │
│  [D] Show Detailed Error                                                    │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Emergency Console

For advanced users, FerryBoot provides an emergency console:

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        Emergency Console                                │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  FerryBoot Emergency Console                                                │
│                                                                             │
│  Available commands:                                                        │
│  - help: Show this help                                                     │
│  - ls: List files                                                           │
│  - cat: Display file contents                                               │
│  - mount: Mount filesystem                                                  │
│  - boot: Boot OS manually                                                   │
│  - reboot: Reboot system                                                    │
│  - poweroff: Power off system                                               │
│                                                                             │
│  ferryboot> _                                                               │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Internationalization

FerryBoot supports multiple languages:
- English (default)
- Spanish
- French
- German
- Japanese
- Chinese (Simplified)
- Russian

Language is automatically detected from system locale but can be manually changed in settings.

## Accessibility Features

### High Contrast Mode
- Enhanced contrast ratios
- Bold text options
- Inverted color scheme

### Screen Reader Support
- Text-to-speech integration
- Keyboard shortcuts
- Descriptive labels

### Large Text Mode
- Increased font sizes
- Spacing adjustments
- Layout scaling

## Animation and Feedback

### Transitions
- Smooth fade transitions
- Slide animations
- Loading spinners

### Feedback
- Visual button states
- Sound feedback (configurable)
- Haptic feedback (on supported devices)

## Customization

Users can customize the interface:

### Themes
- Light mode (default)
- Dark mode
- High contrast
- Custom color schemes

### Layout
- Menu orientation (vertical/horizontal)
- Icon sizes
- Font choices

### Behavior
- Boot timeout duration
- Default actions
- Navigation preferences

## Implementation Details

### Rendering Engine
```c
// Text mode rendering
void text_draw_char(uint8_t ch, uint8_t attr, uint8_t x, uint8_t y);
void text_draw_string(const char* str, uint8_t attr, uint8_t x, uint8_t y);

// GUI rendering
void gui_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void gui_draw_text(const char* text, uint32_t x, uint32_t y, uint32_t color);
void gui_draw_icon(uint32_t icon_id, uint32_t x, uint32_t y);
```

### Input Handling
```c
// Input event structure
typedef struct {
    uint32_t type;      // KEYBOARD, MOUSE, TOUCH
    uint32_t code;      // Key code, button, etc.
    uint32_t x, y;      // Coordinates for mouse/touch
    uint32_t modifiers; // Shift, Ctrl, Alt
} input_event_t;

// Input processing
void handle_input_event(const input_event_t* event);
bool navigate_menu(int direction);
bool select_item(uint32_t item_id);
```

### Theme System
```c
// Theme structure
typedef struct {
    char name[32];
    uint32_t background_color;
    uint32_t text_color;
    uint32_t highlight_color;
    uint32_t border_color;
    // ... additional theme properties
} theme_t;

// Apply theme
void apply_theme(const theme_t* theme);
```

## Performance Considerations

### Fast Rendering
- Double buffering
- Efficient drawing routines
- Minimal redraws
- Hardware acceleration when available

### Memory Usage
- Pre-loaded assets
- Efficient data structures
- On-demand loading
- Memory pooling

## Testing

### Usability Testing
- User experience studies
- Accessibility audits
- Cross-platform testing
- Performance benchmarks

### Compatibility
- Different screen resolutions
- Various aspect ratios
- Multiple input devices
- Legacy hardware testing
#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include <stdint.h>

// Function prototypes for bootloader operations
void load_kernel(const char* kernel_file);
void print_boot_menu();
void handle_user_input();
void boot_kernel(int choice);
void halt(); // To stop execution
void configure(); // Prototype for configuration function
void exit_bootloader(); // Prototype for exit function

#endif // BOOTLOADER_H

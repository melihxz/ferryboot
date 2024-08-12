#include "menu.h"
#include "config.h"
#include "filesystem.h"
#include "bootloader.h"
#include <stdio.h>
#include <string.h>

// Display the boot menu
void menu() {
    int choice;

    while (1) {
        print_boot_menu(); // Print the boot menu with available options

        // Get user input
        choice = get_input();

        if (choice >= 1 && choice <= config.num_boot_options) {
            // Boot the selected kernel
            boot_kernel(choice - 1);
        } else if (choice == config.num_boot_options + 1) {
            configure(); // Configure boot settings
        } else if (choice == config.num_boot_options + 2) {
            exit_bootloader(); // Exit bootloader
            return;
        } else {
            printf("Invalid choice. Please try again.\n");
        }
    }
}

// Utility function to get user input
int get_input() {
    int input;
    scanf("%d", &input);
    return input;
}

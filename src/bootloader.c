#include "bootloader.h"
#include "config.h"
#include "filesystem.h"
#include <stdio.h>
#include <string.h>

// Function to load a kernel file
void load_kernel(const char* kernel_file) {
    printf("Loading kernel from %s...\n", kernel_file);
    // Open the kernel file
    File* kernel = open_file(kernel_file);
    if (kernel == NULL) {
        printf("Error: Kernel file not found.\n");
        halt();
    }

    // Load the kernel into memory
    // Assuming `load_to_memory` is a function that handles loading binary files to memory
    if (load_to_memory(kernel) < 0) {
        printf("Error loading kernel into memory.\n");
        close_file(kernel);
        halt();
    }

    // Close the file after loading
    close_file(kernel);
    printf("Kernel loaded successfully.\n");

    // Start the kernel
    start_kernel();
}

// Function to print the boot menu
void print_boot_menu() {
    printf("Ferryboot Menu\n");
    for (int i = 0; i < config.num_boot_options; i++) {
        printf("%d. Boot %s\n", i + 1, config.kernel[i]);
    }
    printf("%d. Configure Boot Settings\n", config.num_boot_options + 1);
    printf("%d. Exit\n", config.num_boot_options + 2);
}

void handle_user_input() {
    int choice;
    printf("Enter your choice: ");
    if (scanf("%d", &choice) != 1) {
        printf("Error reading input. Please try again.\n");
        while(getchar() != '\n'); // Clear the input buffer
        handle_user_input();
        return;
    }

    if (choice >= 1 && choice <= config.num_boot_options) {
        boot_kernel(choice - 1);
    } else if (choice == config.num_boot_options + 1) {
        configure();
    } else if (choice == config.num_boot_options + 2) {
        exit_bootloader();
    } else {
        printf("Invalid choice. Please try again.\n");
        handle_user_input();
    }
}


// Function to boot the selected kernel
void boot_kernel(int choice) {
    if (choice < 0 || choice >= config.num_boot_options) {
        printf("Invalid kernel choice.\n");
        halt();
    }

    load_kernel(config.kernel[choice]);
}

// Function to stop execution
void halt() {
    printf("Halting execution.\n");
    while (1) { } // Infinite loop to halt the system
}

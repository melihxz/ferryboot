#include "config.h"
#include "filesystem.h"
#include <string.h>

// Global configuration structure
struct Config config;

// Load configuration settings from a file
void load_config() {
    printf("Loading configuration...\n");

    // Open the configuration file
    File* config_file = open_file("ferry.cfg");
    
    if (config_file == NULL) {
        printf("Error: Configuration file not found. Using default settings.\n");
        set_default_config();
        return;
    }

    // Read the file contents into the config structure
    if (read_file(config_file, &config, sizeof(config)) < sizeof(config)) {
        printf("Error reading configuration file. Using default settings.\n");
        set_default_config();
    } else {
        printf("Configuration loaded successfully.\n");
    }

    // Close the file
    close_file(config_file);
}

// Save configuration settings to a file
void save_config() {
    printf("Saving configuration...\n");

    // Open the configuration file for writing
    File* config_file = open_file("ferry.cfg");

    if (config_file == NULL) {
        printf("Error: Unable to open configuration file for writing.\n");
        return;
    }

    // Write the config structure to the file
    if (write_file(config_file, &config, sizeof(config)) < sizeof(config)) {
        printf("Error writing configuration to file.\n");
    } else {
        printf("Configuration saved successfully.\n");
    }

    // Close the file
    close_file(config_file);
}

// Set default configuration values
void set_default_config() {
    strncpy(config.kernel[0], "kernel.bin", sizeof(config.kernel[0]));
    config.num_boot_options = 1;
    config.timeout = 5;
}

void set_boot_options() {
    int num_options;
    printf("Enter the number of boot options: ");
    if (scanf("%d", &num_options) != 1) {
        printf("Error reading input. Please try again.\n");
        while(getchar() != '\n'); // Clear the input buffer
        set_boot_options();
        return;
    }
    config.num_boot_options = num_options;

    for (int i = 0; i < num_options; i++) {
        printf("Enter the path for kernel %d: ", i + 1);
        if (scanf("%s", config.kernel[i]) != 1) {
            printf("Error reading input. Please try again.\n");
            while(getchar() != '\n'); // Clear the input buffer
            i--; // Repeat the current iteration
        }
    }
}


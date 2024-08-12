#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>

#define MAX_BOOT_OPTIONS 10 // Maximum number of boot options

// Configuration structure
struct Config {
    char kernel[MAX_BOOT_OPTIONS][32];
    int num_boot_options;
    int timeout;
};

// Global configuration variable
extern struct Config config;

// Function declarations for configuration management
void load_config();
void save_config();
void set_default_config();
void set_boot_options();

#endif // CONFIG_H

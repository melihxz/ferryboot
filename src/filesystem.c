#include "filesystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define KERNEL_LOAD_ADDRESS 0x10000 // Example memory address for the kernel

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) GDTPtr;

typedef struct {
    uint16_t limit;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed)) GDTEntry;

// Dummy implementations for file operations
File* open_file(const char* filename) {
    // Open file for reading
    FILE* file = fopen(filename, "rb");
    return file ? file : NULL;
}

void close_file(File* file) {
    if (file) {
        fclose(file);
    }
}

int read_file(File* file, void* buffer, size_t size) {
    return fread(buffer, 1, size, file);
}

int write_file(File* file, const void* buffer, size_t size) {
    return fwrite(buffer, 1, size, file);
}

int load_to_memory(File* file) {
    static unsigned char memory[0x100000]; // Simulated memory (1MB)

    if (file == NULL) {
        return -1; // Error: File not open
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size > sizeof(memory) - KERNEL_LOAD_ADDRESS) {
        return -2; // Error: File too large for memory
    }

    size_t bytes_read = fread(memory + KERNEL_LOAD_ADDRESS, 1, file_size, file);
    if (bytes_read != (size_t)file_size) {
        return -3; // Error: Read incomplete
    }

    return 0; // Success
}

void start_kernel() {
    GDTEntry gdt[3];
    memset(&gdt[0], 0, sizeof(GDTEntry) * 3);

    gdt[1].limit = 0xFFFF;
    gdt[1].base_low = 0x0000;
    gdt[1].base_middle = 0x00;
    gdt[1].access = 0x9A;       // Present, Ring 0, Code, Read/Execute
    gdt[1].granularity = 0xCF; // 32-bit, Page size
    gdt[1].base_high = 0x00;

    gdt[2].limit = 0xFFFF;
    gdt[2].base_low = 0x0000;
    gdt[2].base_middle = 0x00;
    gdt[2].access = 0x92;       // Present, Ring 0, Data, Read/Write
    gdt[2].granularity = 0xCF; // 32-bit, Page size
    gdt[2].base_high = 0x00;

    GDTPtr gdt_ptr;
    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base = (uintptr_t)gdt;

    // Use inline assembly for GDT setup and jump to protected mode
    asm volatile (
        "lgdt %0\n"                // Load GDT
        "movl $0x10, %%eax\n"      // Load data segment selector
        "mov %%eax, %%ds\n"        // Set DS
        "mov %%eax, %%es\n"        // Set ES
        "mov %%eax, %%fs\n"        // Set FS
        "mov %%eax, %%gs\n"        // Set GS
        "mov %%eax, %%ss\n"        // Set SS
        "jmp $0x08, $start_protected_mode\n" // Jump to protected mode code segment
        "start_protected_mode:\n"
        "mov $0x001B, %%ax\n"      // Load code segment selector
        "mov %%ax, %%ds\n"         // Set DS
        "mov %%ax, %%es\n"         // Set ES
        "mov %%ax, %%fs\n"         // Set FS
        "mov %%ax, %%gs\n"         // Set GS
        "mov %%ax, %%ss\n"         // Set SS
        "jmp $0x08, $0x0000\n"     // Jump to kernel entry point
        : /* no output */
        : "m" (gdt_ptr)
        : "eax"
    );

    typedef void (*kernel_entry_t)(void);
    kernel_entry_t kernel_entry = (kernel_entry_t)KERNEL_LOAD_ADDRESS;

    // Call the kernel entry point
    kernel_entry();
}

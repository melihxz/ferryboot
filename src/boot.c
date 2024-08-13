#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Global yapılandırma yapısı
#define MAX_KERNELS 10
#define MAX_PATH_LENGTH 256

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

typedef struct {
    int num_boot_options;
    int timeout;
    char kernel[MAX_KERNELS][MAX_PATH_LENGTH];
} Config;

// Global yapılandırma nesnesi
Config config;

// Bellek simülasyonu
#define KERNEL_LOAD_ADDRESS 0x1000 // The mem addres that kernel will load
static unsigned char memory[0x100000]; // Simulated memory (1MB)

// Dosya işlemleri
typedef FILE File;

File* open_file(const char* filename) {
    return fopen(filename, "rb");
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

// Kernel başlangıç fonksiyonu
void start_kernel() {
    GDTEntry gdt[3];
    memset(&gdt[0], 0, sizeof(GDTEntry) * 3);

    gdt[1].limit = 0xFFFF;
    gdt[1].base_low = 0x0000;
    gdt[1].base_middle = 0x00;
    gdt[1].access = 0x9A;       // Present, Ring 0, Code, Read/Execute
    gdt[1].granularity = 0xCF;  // 32-bit, Page size
    gdt[1].base_high = 0x00;

    gdt[2].limit = 0xFFFF;
    gdt[2].base_low = 0x0000;
    gdt[2].base_middle = 0x00;
    gdt[2].access = 0x92;       // Present, Ring 0, Data, Read/Write
    gdt[2].granularity = 0xCF;  // 32-bit, Page size
    gdt[2].base_high = 0x00;

    GDTPtr gdt_ptr;
    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base = (uintptr_t)gdt;

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

    // Kernel giriş noktasını çağır
    kernel_entry();
}

// Kernel'i yükleme işlevi
void load_kernel(const char* kernel_file) {
    printf("Loading kernel from %s...\n", kernel_file);

    File* kernel = open_file(kernel_file);
    if (kernel == NULL) {
        printf("Error: Kernel file not found.\n");
        halt();
    }

    if (load_to_memory(kernel) < 0) {
        printf("Error loading kernel into memory.\n");
        close_file(kernel);
        halt();
    }

    close_file(kernel);
    printf("Kernel loaded successfully.\n");

    start_kernel();
}

// Boot menüsü yazdırma işlevi
void print_boot_menu() {
    printf("Ferryboot Menu\n");
    for (int i = 0; i < config.num_boot_options; i++) {
        printf("%d. Boot %s\n", i + 1, config.kernel[i]);
    }
    printf("%d. Configure Boot Settings\n", config.num_boot_options + 1);
    printf("%d. Exit\n", config.num_boot_options + 2);
}

// Kullanıcı girdisini alma işlevi
int get_input() {
    int input;
    scanf("%d", &input);
    return input;
}

// Kernel başlatma işlevi
void boot_kernel(int choice) {
    if (choice < 0 || choice >= config.num_boot_options) {
        printf("Invalid kernel choice.\n");
        halt();
    }

    load_kernel(config.kernel[choice]);
}

// Yapılandırma ayarlarını dosyadan yükleme işlevi
void load_config() {
    printf("Loading configuration...\n");

    File* config_file = open_file("ferry.cfg");
    
    if (config_file == NULL) {
        printf("Error: Configuration file not found. Using default settings.\n");
        set_default_config();
        return;
    }

    if (read_file(config_file, &config, sizeof(config)) < sizeof(config)) {
        printf("Error reading configuration file. Using default settings.\n");
        set_default_config();
    } else {
        printf("Configuration loaded successfully.\n");
    }

    close_file(config_file);
}

// Yapılandırma ayarlarını dosyaya kaydetme işlevi
void save_config() {
    printf("Saving configuration...\n");

    File* config_file = open_file("ferry.cfg");

    if (config_file == NULL) {
        printf("Error: Unable to open configuration file for writing.\n");
        return;
    }

    if (write_file(config_file, &config, sizeof(config)) < sizeof(config)) {
        printf("Error writing configuration to file.\n");
    } else {
        printf("Configuration saved successfully.\n");
    }

    close_file(config_file);
}

// Varsayılan yapılandırma değerlerini ayarlama işlevi
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
        while(getchar() != '\n');
        set_boot_options();
        return;
    }
    config.num_boot_options = num_options;

    for (int i = 0; i < num_options; i++) {
        printf("Enter the path for kernel %d: ", i + 1);
        if (scanf("%s", config.kernel[i]) != 1) {
            printf("Error reading input. Please try again.\n");
            while(getchar() != '\n');
            i--;
        }
    }
}

// Boot ayarlarını yapılandırma işlevi
void configure() {
    printf("Configuring boot settings...\n");
    set_boot_options();
    save_config();
}

// Exit bootloader
void exit_bootloader() {
    printf("Exiting bootloader.\n");
}

// Sistemi durdurma işlevi
void halt() {
    printf("Halting execution.\n");
    while (1) { } // Sonsuz döngü ile sistemi durdur
}

// Menüyü gösterme işlevi
void menu() {
    int choice;
    do {
        print_boot_menu();
        choice = get_input();

        switch (choice) {
            case 1 ... MAX_KERNELS:
                boot_kernel(choice - 1);
                break;
            case MAX_KERNELS + 1:
                configure();
                break;
            case MAX_KERNELS + 2:
                exit_bootloader();
                break;
            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
    } while (choice != MAX_KERNELS + 2);
}

// Ana fonksiyon
int main() {
    load_config();
    menu();
    return 0;
}

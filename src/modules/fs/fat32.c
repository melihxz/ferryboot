#include "fat32.h"
#include "../../common/memory.h"
#include "../../common/string.h"

// FAT32 attributes
#define FAT32_ATTR_READ_ONLY    0x01
#define FAT32_ATTR_HIDDEN       0x02
#define FAT32_ATTR_SYSTEM       0x04
#define FAT32_ATTR_VOLUME_ID    0x08
#define FAT32_ATTR_DIRECTORY    0x10
#define FAT32_ATTR_ARCHIVE      0x20
#define FAT32_ATTR_LONG_NAME    0x0F

// FAT32 cluster values
#define FAT32_CLUSTER_FREE      0x00000000
#define FAT32_CLUSTER_RESERVED  0x0FFFFFF0
#define FAT32_CLUSTER_BAD       0x0FFFFFF7
#define FAT32_CLUSTER_END       0x0FFFFFF8

// Global FAT32 module instance
static fat32_module_t fat32_module_instance;

// Mount FAT32 filesystem
static int fat32_mount(const char* device) {
    // Read boot sector
    if (g_hal->read_sector(0, &fat32_module_instance.context.boot_sector, 1) != 0) {
        return -1;
    }
    
    // Validate FAT32 signature
    if (fat32_module_instance.context.boot_sector.fs_type[0] != 'F' ||
        fat32_module_instance.context.boot_sector.fs_type[1] != 'A' ||
        fat32_module_instance.context.boot_sector.fs_type[2] != 'T' ||
        fat32_module_instance.context.boot_sector.fs_type[3] != '3' ||
        fat32_module_instance.context.boot_sector.fs_type[4] != '2') {
        return -1;
    }
    
    // Calculate filesystem layout
    fat32_module_instance.context.fat_start = fat32_module_instance.context.boot_sector.reserved_sectors;
    fat32_module_instance.context.data_start = fat32_module_instance.context.fat_start + 
                          (fat32_module_instance.context.boot_sector.num_fats * fat32_module_instance.context.boot_sector.fat_size_32);
    fat32_module_instance.context.root_dir_cluster = fat32_module_instance.context.boot_sector.root_cluster;
    
    // Initialize FAT cache
    fat32_module_instance.context.fat_cache = (uint32_t*)memory_alloc(512 * 8); // Cache 8 sectors
    fat32_module_instance.context.fat_cache_sector = 0;
    fat32_module_instance.context.fat_cache_dirty = false;
    
    // Store device name
    size_t device_len = string_length(device);
    if (device_len >= sizeof(fat32_module_instance.context.device)) {
        device_len = sizeof(fat32_module_instance.context.device) - 1;
    }
    memory_copy(fat32_module_instance.context.device, device, device_len);
    fat32_module_instance.context.device[device_len] = '\0';
    
    return 0;
}

// Unmount FAT32 filesystem
static int fat32_unmount(void) {
    // Flush FAT cache if dirty
    if (fat32_module_instance.context.fat_cache_dirty) {
        // In a real implementation, we would write back the cache
    }
    
    // Free FAT cache
    if (fat32_module_instance.context.fat_cache) {
        memory_free((void*)fat32_module_instance.context.fat_cache);
        fat32_module_instance.context.fat_cache = NULL;
    }
    
    memory_set(&fat32_module_instance.context, 0, sizeof(fat32_module_instance.context));
    return 0;
}

// Read FAT entry
static uint32_t fat32_read_fat_entry(uint32_t cluster) {
    uint32_t fat_sector = fat32_module_instance.context.fat_start + (cluster * 4) / 512;
    uint32_t fat_offset = (cluster * 4) % 512;
    
    // Check if sector is in cache
    if (fat_sector >= fat32_module_instance.context.fat_cache_sector && 
        fat_sector < fat32_module_instance.context.fat_cache_sector + 8) {
        return fat32_module_instance.context.fat_cache[fat_offset / 4];
    }
    
    // Load new sector into cache
    if (g_hal->read_sector(fat_sector, fat32_module_instance.context.fat_cache, 8) != 0) {
        return 0;
    }
    
    fat32_module_instance.context.fat_cache_sector = fat_sector;
    return fat32_module_instance.context.fat_cache[fat_offset / 4];
}

// Convert cluster to LBA
static uint32_t fat32_cluster_to_lba(uint32_t cluster) {
    return fat32_module_instance.context.data_start + 
           ((cluster - 2) * fat32_module_instance.context.boot_sector.sectors_per_cluster);
}

// Find file in directory
static int fat32_find_file(const char* path, fat32_dir_entry_t* entry) {
    // Simplified path parsing - assumes root directory for boot partition
    const char* filename = path;
    if (path[0] == '/') {
        filename = path + 1;
    }
    
    // Start from root directory
    uint32_t cluster = fat32_module_instance.context.root_dir_cluster;
    
    // Traverse directory clusters
    while (cluster != FAT32_CLUSTER_END) {
        // Read cluster
        uint8_t* buffer = (uint8_t*)memory_alloc(512 * fat32_module_instance.context.boot_sector.sectors_per_cluster);
        if (!buffer) {
            return -1;
        }
        
        if (g_hal->read_sector(fat32_cluster_to_lba(cluster), 
                              buffer, fat32_module_instance.context.boot_sector.sectors_per_cluster) != 0) {
            memory_free(buffer);
            return -1;
        }
        
        // Search for file in cluster
        fat32_dir_entry_t* dir_entry = (fat32_dir_entry_t*)buffer;
        for (unsigned int i = 0; i < (512 * fat32_module_instance.context.boot_sector.sectors_per_cluster) / sizeof(fat32_dir_entry_t); i++) {
            // Check for end of directory
            if (dir_entry[i].name[0] == 0x00) {
                break;
            }
            
            // Skip deleted entries
            if (dir_entry[i].name[0] == 0xE5) {
                continue;
            }
            
            // Check for long filename entries
            if ((dir_entry[i].attributes & FAT32_ATTR_LONG_NAME) == FAT32_ATTR_LONG_NAME) {
                continue;
            }
            
            // Compare filename
            char entry_name[12];
            memory_copy(entry_name, dir_entry[i].name, 11);
            entry_name[11] = '\0';
            
            // Null terminate at first space
            for (int j = 0; j < 11; j++) {
                if (entry_name[j] == ' ') {
                    entry_name[j] = '\0';
                    break;
                }
            }
            
            if (string_compare(entry_name, filename) == 0) {
                *entry = dir_entry[i];
                memory_free(buffer);
                return 0;
            }
        }
        
        memory_free(buffer);
        
        // Move to next cluster
        cluster = fat32_read_fat_entry(cluster);
        if (cluster >= FAT32_CLUSTER_BAD) {
            return -1;
        }
    }
    
    return -1; // File not found
}

// Read file from FAT32 filesystem
static int fat32_read_file(const char* path, void* buffer, size_t size) {
    fat32_dir_entry_t entry;
    
    // Find file
    if (fat32_find_file(path, &entry) != 0) {
        return -1;
    }
    
    // Check if file fits in buffer
    if (entry.file_size > size) {
        return -1;
    }
    
    // Read file clusters
    uint32_t cluster = (entry.first_cluster_high << 16) | entry.first_cluster_low;
    uint8_t* buf = (uint8_t*)buffer;
    uint32_t bytes_read = 0;
    
    while (cluster != FAT32_CLUSTER_END && bytes_read < entry.file_size) {
        // Read cluster
        uint32_t bytes_to_read = entry.file_size - bytes_read;
        if (bytes_to_read > 512 * fat32_module_instance.context.boot_sector.sectors_per_cluster) {
            bytes_to_read = 512 * fat32_module_instance.context.boot_sector.sectors_per_cluster;
        }
        
        if (g_hal->read_sector(fat32_cluster_to_lba(cluster), 
                              buf + bytes_read, 
                              (bytes_to_read + 511) / 512) != 0) {
            return -1;
        }
        
        bytes_read += bytes_to_read;
        
        // Move to next cluster
        cluster = fat32_read_fat_entry(cluster);
        if (cluster >= FAT32_CLUSTER_BAD) {
            return -1;
        }
    }
    
    return 0;
}

// Module initialization
static int fat32_init(void) {
    // Initialize module instance
    memory_set(&fat32_module_instance, 0, sizeof(fat32_module_instance));
    
    return 0;
}

// Module cleanup
static void fat32_cleanup(void) {
    // Unmount filesystem if mounted
    fat32_unmount();
    
    // Clear module instance
    memory_set(&fat32_module_instance, 0, sizeof(fat32_module_instance));
}

// Module metadata
static const module_header_t fat32_module_header = {
    .magic = MODULE_MAGIC,
    .version = 1,
    .type = MODULE_TYPE_FILESYSTEM,
    .flags = 0,
    .name = "fat32",
    .description = "FAT32 filesystem support"
};

// Module interface
static module_interface_t fat32_module_interface = {
    .init = fat32_init,
    .cleanup = fat32_cleanup
};

// Module entry point (renamed to avoid conflict)
int fat32_module_main(module_header_t* header) {
    if (header->magic != MODULE_MAGIC) {
        return -1;
    }
    
    *header = fat32_module_header;
    return (int)&fat32_module_interface;
}

// Public functions for filesystem access
int fat32_module_mount(const char* device) {
    return fat32_mount(device);
}

int fat32_module_unmount(void) {
    return fat32_unmount();
}

int fat32_module_read_file(const char* path, void* buffer, size_t size) {
    return fat32_read_file(path, buffer, size);
}
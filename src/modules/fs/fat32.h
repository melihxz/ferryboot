#ifndef FERRYBOOT_FAT32_MODULE_H
#define FERRYBOOT_FAT32_MODULE_H

#include "../../common/module.h"
#include "../../common/hal.h"

// FAT32 boot sector
typedef struct {
    uint8_t jump_boot[3];
    uint8_t oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t num_fats;
    uint16_t root_entries;
    uint16_t total_sectors_16;
    uint8_t media_type;
    uint16_t fat_size_16;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    
    // FAT32 specific
    uint32_t fat_size_32;
    uint16_t ext_flags;
    uint16_t fs_version;
    uint32_t root_cluster;
    uint16_t fs_info_sector;
    uint16_t backup_boot_sector;
    uint8_t reserved[12];
    uint8_t drive_number;
    uint8_t reserved1;
    uint8_t boot_signature;
    uint32_t volume_serial;
    uint8_t volume_label[11];
    uint8_t fs_type[8];
} __attribute__((packed)) fat32_boot_sector_t;

// FAT32 directory entry
typedef struct {
    uint8_t name[11];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t creation_time_tenth;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t access_date;
    uint16_t first_cluster_high;
    uint16_t write_time;
    uint16_t write_date;
    uint16_t first_cluster_low;
    uint32_t file_size;
} __attribute__((packed)) fat32_dir_entry_t;

// FAT32 filesystem context
typedef struct {
    char device[32];
    fat32_boot_sector_t boot_sector;
    uint32_t fat_start;
    uint32_t data_start;
    uint32_t root_dir_cluster;
    uint32_t* fat_cache;
    uint32_t fat_cache_sector;
    bool fat_cache_dirty;
} fat32_context_t;

// FAT32 module interface
typedef struct {
    module_interface_t base;
    fat32_context_t context;
} fat32_module_t;

#endif // FERRYBOOT_FAT32_MODULE_H
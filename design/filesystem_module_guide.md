# FerryBoot Filesystem Module Implementation Guide

## 1. Overview

This document provides detailed implementation guidance for filesystem modules in FerryBoot. Filesystem modules are essential for detecting and booting operating systems, as well as for accessing configuration files and modules stored on disk.

## 2. Module Architecture

### 2.1 Filesystem Module Interface

All filesystem modules must implement a standard interface:

```c
// fs_module.h - Filesystem module interface
#ifndef FERRYBOOT_FS_MODULE_H
#define FERRYBOOT_FS_MODULE_H

#include "module.h"
#include "hal.h"

// Filesystem operations
typedef struct {
    int (*mount)(const char* device);
    int (*unmount)(void);
    int (*read_file)(const char* path, void* buffer, size_t size);
    int (*write_file)(const char* path, const void* buffer, size_t size);
    int (*list_directory)(const char* path, char** entries, size_t max_entries);
    int (*get_file_info)(const char* path, file_info_t* info);
    bool (*is_supported)(void);
} filesystem_ops_t;

// Filesystem module interface
typedef struct {
    module_interface_t base;
    filesystem_ops_t fs_ops;
} filesystem_module_t;

// File information structure
typedef struct {
    char name[256];
    uint64_t size;
    uint32_t attributes;
    uint64_t creation_time;
    uint64_t modification_time;
    bool is_directory;
} file_info_t;

#endif // FERRYBOOT_FS_MODULE_H
```

### 2.2 Filesystem Types

FerryBoot supports several filesystem types:

1. **FAT32** - Built-in for boot partition
2. **ext4/ext3/ext2** - Linux filesystems
3. **NTFS** - Windows filesystem
4. **XFS** - High-performance Linux filesystem
5. **Btrfs** - Modern copy-on-write filesystem
6. **APFS** - Apple File System

## 3. FAT32 Implementation

### 3.1 FAT32 Structure

FAT32 is the primary filesystem for boot partitions due to its universal support:

```c
// fat32.h - FAT32 filesystem implementation
#ifndef FERRYBOOT_FAT32_H
#define FERRYBOOT_FAT32_H

#include "fs_module.h"

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

// FAT32 long filename entry
typedef struct {
    uint8_t ordinal;
    uint8_t name1[10];
    uint8_t attributes;
    uint8_t type;
    uint8_t checksum;
    uint8_t name2[12];
    uint16_t reserved;
    uint8_t name3[4];
} __attribute__((packed)) fat32_lfn_entry_t;

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

#endif // FERRYBOOT_FAT32_H
```

### 3.2 FAT32 Implementation

```c
// fat32.c - FAT32 filesystem implementation
#include "fat32.h"
#include "hal.h"
#include "memory.h"

// Global FAT32 context
static fat32_context_t fat32_ctx;

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

// Mount FAT32 filesystem
static int fat32_mount(const char* device) {
    // Read boot sector
    if (hal_read_sector(0, &fat32_ctx.boot_sector, 1) != 0) {
        return -1;
    }
    
    // Validate FAT32 signature
    if (fat32_ctx.boot_sector.fs_type[0] != 'F' ||
        fat32_ctx.boot_sector.fs_type[1] != 'A' ||
        fat32_ctx.boot_sector.fs_type[2] != 'T' ||
        fat32_ctx.boot_sector.fs_type[3] != '3' ||
        fat32_ctx.boot_sector.fs_type[4] != '2') {
        return -1;
    }
    
    // Calculate filesystem layout
    fat32_ctx.fat_start = fat32_ctx.boot_sector.reserved_sectors;
    fat32_ctx.data_start = fat32_ctx.fat_start + 
                          (fat32_ctx.boot_sector.num_fats * fat32_ctx.boot_sector.fat_size_32);
    fat32_ctx.root_dir_cluster = fat32_ctx.boot_sector.root_cluster;
    
    // Initialize FAT cache
    fat32_ctx.fat_cache = memory_alloc(512 * 8); // Cache 8 sectors
    fat32_ctx.fat_cache_sector = 0;
    fat32_ctx.fat_cache_dirty = false;
    
    // Store device name
    strncpy(fat32_ctx.device, device, sizeof(fat32_ctx.device) - 1);
    
    return 0;
}

// Unmount FAT32 filesystem
static int fat32_unmount(void) {
    // Flush FAT cache if dirty
    if (fat32_ctx.fat_cache_dirty) {
        // Write back cache
        // Implementation details...
    }
    
    // Free FAT cache
    if (fat32_ctx.fat_cache) {
        memory_free(fat32_ctx.fat_cache);
        fat32_ctx.fat_cache = NULL;
    }
    
    memory_set(&fat32_ctx, 0, sizeof(fat32_ctx));
    return 0;
}

// Read FAT entry
static uint32_t fat32_read_fat_entry(uint32_t cluster) {
    uint32_t fat_sector = fat32_ctx.fat_start + (cluster * 4) / 512;
    uint32_t fat_offset = (cluster * 4) % 512;
    
    // Check if sector is in cache
    if (fat_sector >= fat32_ctx.fat_cache_sector && 
        fat_sector < fat32_ctx.fat_cache_sector + 8) {
        return fat32_ctx.fat_cache[fat_offset / 4];
    }
    
    // Load new sector into cache
    if (hal_read_sector(fat_sector, fat32_ctx.fat_cache, 8) != 0) {
        return 0;
    }
    
    fat32_ctx.fat_cache_sector = fat_sector;
    return fat32_ctx.fat_cache[fat_offset / 4];
}

// Convert cluster to LBA
static uint32_t fat32_cluster_to_lba(uint32_t cluster) {
    return fat32_ctx.data_start + 
           ((cluster - 2) * fat32_ctx.boot_sector.sectors_per_cluster);
}

// Find file in directory
static int fat32_find_file(const char* path, fat32_dir_entry_t* entry, uint32_t* entry_cluster) {
    // Simplified path parsing - assumes root directory for boot partition
    const char* filename = path;
    if (path[0] == '/') {
        filename = path + 1;
    }
    
    // Start from root directory
    uint32_t cluster = fat32_ctx.root_dir_cluster;
    
    // Traverse directory clusters
    while (cluster != FAT32_CLUSTER_END) {
        // Read cluster
        uint8_t* buffer = memory_alloc(512 * fat32_ctx.boot_sector.sectors_per_cluster);
        if (!buffer) {
            return -1;
        }
        
        if (hal_read_sector(fat32_cluster_to_lba(cluster), 
                           buffer, fat32_ctx.boot_sector.sectors_per_cluster) != 0) {
            memory_free(buffer);
            return -1;
        }
        
        // Search for file in cluster
        fat32_dir_entry_t* dir_entry = (fat32_dir_entry_t*)buffer;
        for (int i = 0; i < (512 * fat32_ctx.boot_sector.sectors_per_cluster) / sizeof(fat32_dir_entry_t); i++) {
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
            
            if (strcmp(entry_name, filename) == 0) {
                *entry = dir_entry[i];
                *entry_cluster = cluster;
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
    uint32_t entry_cluster;
    
    // Find file
    if (fat32_find_file(path, &entry, &entry_cluster) != 0) {
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
        if (bytes_to_read > 512 * fat32_ctx.boot_sector.sectors_per_cluster) {
            bytes_to_read = 512 * fat32_ctx.boot_sector.sectors_per_cluster;
        }
        
        if (hal_read_sector(fat32_cluster_to_lba(cluster), 
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

// Get file information
static int fat32_get_file_info(const char* path, file_info_t* info) {
    fat32_dir_entry_t entry;
    uint32_t entry_cluster;
    
    // Find file
    if (fat32_find_file(path, &entry, &entry_cluster) != 0) {
        return -1;
    }
    
    // Fill file information
    memory_set(info, 0, sizeof(file_info_t));
    
    // Convert 8.3 filename to null-terminated string
    int name_len = 0;
    for (int i = 0; i < 8 && entry.name[i] != ' '; i++) {
        info->name[name_len++] = entry.name[i];
    }
    
    // Add extension if present
    if (entry.name[8] != ' ') {
        info->name[name_len++] = '.';
        for (int i = 8; i < 11 && entry.name[i] != ' '; i++) {
            info->name[name_len++] = entry.name[i];
        }
    }
    info->name[name_len] = '\0';
    
    info->size = entry.file_size;
    info->attributes = entry.attributes;
    info->is_directory = (entry.attributes & FAT32_ATTR_DIRECTORY) != 0;
    
    // Convert DOS timestamps to Unix timestamps (simplified)
    info->creation_time = ((entry.creation_date >> 9) + 80) * 365 * 24 * 60 * 60 +
                         ((entry.creation_date >> 5) & 0x0F) * 30 * 24 * 60 * 60 +
                         (entry.creation_date & 0x1F) * 24 * 60 * 60 +
                         (entry.creation_time >> 11) * 60 * 60 +
                         ((entry.creation_time >> 5) & 0x3F) * 60 +
                         (entry.creation_time & 0x1F) * 2;
    
    return 0;
}

// Check if FAT32 is supported
static bool fat32_is_supported(void) {
    return true; // Always supported as boot filesystem
}

// FAT32 operations
static filesystem_ops_t fat32_ops = {
    .mount = fat32_mount,
    .unmount = fat32_unmount,
    .read_file = fat32_read_file,
    .write_file = NULL, // Read-only for bootloader
    .list_directory = NULL, // Not implemented in this example
    .get_file_info = fat32_get_file_info,
    .is_supported = fat32_is_supported
};

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
static filesystem_module_t fat32_module = {
    .base = {
        .init = NULL, // No initialization needed
        .cleanup = NULL // No cleanup needed
    },
    .fs_ops = fat32_ops
};

// Module entry point
int module_main(module_header_t* header) {
    if (header->magic != MODULE_MAGIC) {
        return -1;
    }
    
    *header = fat32_module_header;
    return (int)&fat32_module;
}
```

## 4. ext4 Implementation

### 4.1 ext4 Structure

ext4 is the most common Linux filesystem:

```c
// ext4.h - ext4 filesystem implementation
#ifndef FERRYBOOT_EXT4_H
#define FERRYBOOT_EXT4_H

#include "fs_module.h"

// ext4 superblock
typedef struct {
    uint32_t inodes_count;
    uint32_t blocks_count_lo;
    uint32_t r_blocks_count_lo;
    uint32_t free_blocks_count_lo;
    uint32_t free_inodes_count;
    uint32_t first_data_block;
    uint32_t log_block_size;
    uint32_t log_cluster_size;
    uint32_t blocks_per_group;
    uint32_t clusters_per_group;
    uint32_t inodes_per_group;
    uint32_t mtime;
    uint32_t wtime;
    uint16_t mnt_count;
    uint16_t max_mnt_count;
    uint16_t magic;
    uint16_t state;
    uint16_t errors;
    uint16_t minor_rev_level;
    uint32_t lastcheck;
    uint32_t checkinterval;
    uint32_t creator_os;
    uint32_t rev_level;
    uint16_t def_resuid;
    uint16_t def_resgid;
    // ... more fields
} __attribute__((packed)) ext4_superblock_t;

// ext4 group descriptor
typedef struct {
    uint32_t block_bitmap_lo;
    uint32_t inode_bitmap_lo;
    uint32_t inode_table_lo;
    uint16_t free_blocks_count_lo;
    uint16_t free_inodes_count_lo;
    uint16_t used_dirs_count_lo;
    uint16_t flags;
    uint32_t exclude_bitmap_lo;
    uint16_t block_bitmap_csum_lo;
    uint16_t inode_bitmap_csum_lo;
    uint16_t itable_unused_lo;
    uint16_t checksum;
} __attribute__((packed)) ext4_group_desc_t;

// ext4 inode
typedef struct {
    uint16_t mode;
    uint16_t uid;
    uint32_t size_lo;
    uint32_t atime;
    uint32_t ctime;
    uint32_t mtime;
    uint32_t dtime;
    uint16_t gid;
    uint16_t links_count;
    uint32_t blocks_count_lo;
    uint32_t flags;
    uint32_t osd1;
    uint32_t block[15];
    uint32_t generation;
    uint32_t file_acl_lo;
    uint32_t size_high;
    uint32_t obso_faddr;
    uint8_t osd2[12];
} __attribute__((packed)) ext4_inode_t;

#endif // FERRYBOOT_EXT4_H
```

### 4.2 ext4 Implementation

```c
// ext4.c - ext4 filesystem implementation
#include "ext4.h"
#include "hal.h"
#include "memory.h"

// ext4 superblock magic
#define EXT4_SUPER_MAGIC 0xEF53

// ext4 filesystem context
typedef struct {
    char device[32];
    ext4_superblock_t superblock;
    ext4_group_desc_t* group_descs;
    uint32_t block_size;
    uint32_t inodes_per_block;
    uint32_t group_count;
} ext4_context_t;

// Global ext4 context
static ext4_context_t ext4_ctx;

// Mount ext4 filesystem
static int ext4_mount(const char* device) {
    // Read superblock (at offset 1024)
    if (hal_read_sector(2, &ext4_ctx.superblock, 2) != 0) {
        return -1;
    }
    
    // Validate ext4 signature
    if (ext4_ctx.superblock.magic != EXT4_SUPER_MAGIC) {
        return -1;
    }
    
    // Calculate block size
    ext4_ctx.block_size = 1024 << ext4_ctx.superblock.log_block_size;
    
    // Calculate group count
    ext4_ctx.group_count = (ext4_ctx.superblock.blocks_count_lo - 
                           ext4_ctx.superblock.first_data_block + 
                           ext4_ctx.superblock.blocks_per_group - 1) / 
                          ext4_ctx.superblock.blocks_per_group;
    
    // Calculate inodes per block
    ext4_ctx.inodes_per_block = ext4_ctx.block_size / sizeof(ext4_inode_t);
    
    // Allocate group descriptors
    uint32_t group_desc_blocks = (ext4_ctx.group_count * sizeof(ext4_group_desc_t) + 
                                 ext4_ctx.block_size - 1) / ext4_ctx.block_size;
    ext4_ctx.group_descs = memory_alloc(group_desc_blocks * ext4_ctx.block_size);
    if (!ext4_ctx.group_descs) {
        return -1;
    }
    
    // Read group descriptors (starting after superblock)
    uint32_t group_desc_start = (ext4_ctx.superblock.first_data_block + 1) * 
                               (ext4_ctx.block_size / 512);
    if (hal_read_sector(group_desc_start, ext4_ctx.group_descs, 
                       group_desc_blocks * (ext4_ctx.block_size / 512)) != 0) {
        memory_free(ext4_ctx.group_descs);
        ext4_ctx.group_descs = NULL;
        return -1;
    }
    
    // Store device name
    strncpy(ext4_ctx.device, device, sizeof(ext4_ctx.device) - 1);
    
    return 0;
}

// Unmount ext4 filesystem
static int ext4_unmount(void) {
    if (ext4_ctx.group_descs) {
        memory_free(ext4_ctx.group_descs);
        ext4_ctx.group_descs = NULL;
    }
    
    memory_set(&ext4_ctx, 0, sizeof(ext4_ctx));
    return 0;
}

// Read inode
static int ext4_read_inode(uint32_t inode_num, ext4_inode_t* inode) {
    // Calculate group and index
    uint32_t group = (inode_num - 1) / ext4_ctx.superblock.inodes_per_group;
    uint32_t index = (inode_num - 1) % ext4_ctx.superblock.inodes_per_group;
    
    // Get group descriptor
    if (group >= ext4_ctx.group_count) {
        return -1;
    }
    
    ext4_group_desc_t* gd = &ext4_ctx.group_descs[group];
    
    // Calculate inode table block
    uint32_t inode_table_block = gd->inode_table_lo;
    uint32_t inode_block = inode_table_block + (index * sizeof(ext4_inode_t)) / ext4_ctx.block_size;
    uint32_t inode_offset = (index * sizeof(ext4_inode_t)) % ext4_ctx.block_size;
    
    // Read inode block
    uint8_t* buffer = memory_alloc(ext4_ctx.block_size);
    if (!buffer) {
        return -1;
    }
    
    // Convert block to sector
    uint32_t sector = inode_block * (ext4_ctx.block_size / 512);
    if (hal_read_sector(sector, buffer, ext4_ctx.block_size / 512) != 0) {
        memory_free(buffer);
        return -1;
    }
    
    // Copy inode
    memory_copy(inode, buffer + inode_offset, sizeof(ext4_inode_t));
    memory_free(buffer);
    
    return 0;
}

// Read block
static int ext4_read_block(uint32_t block_num, void* buffer) {
    // Convert block to sector
    uint32_t sector = block_num * (ext4_ctx.block_size / 512);
    return hal_read_sector(sector, buffer, ext4_ctx.block_size / 512);
}

// Find file by path
static int ext4_find_file(const char* path, ext4_inode_t* inode, uint32_t* inode_num) {
    // Simplified implementation - assumes path starts with "/boot/"
    // In a real implementation, this would parse the directory structure
    
    // For bootloader purposes, we'll look for common kernel paths
    if (strcmp(path, "/boot/vmlinuz") == 0 ||
        strcmp(path, "/vmlinuz") == 0) {
        // Return the first inode as a placeholder
        *inode_num = 1;
        return ext4_read_inode(*inode_num, inode);
    }
    
    return -1; // File not found
}

// Read file from ext4 filesystem
static int ext4_read_file(const char* path, void* buffer, size_t size) {
    ext4_inode_t inode;
    uint32_t inode_num;
    
    // Find file
    if (ext4_find_file(path, &inode, &inode_num) != 0) {
        return -1;
    }
    
    // Check if file fits in buffer
    uint64_t file_size = inode.size_lo;
    if (inode.size_high) {
        file_size |= ((uint64_t)inode.size_high) << 32;
    }
    
    if (file_size > size) {
        return -1;
    }
    
    // Read file data (simplified - only direct blocks)
    uint8_t* buf = (uint8_t*)buffer;
    uint32_t bytes_read = 0;
    
    for (int i = 0; i < 12 && bytes_read < file_size; i++) {
        if (inode.block[i] == 0) {
            break;
        }
        
        // Calculate bytes to read in this block
        uint32_t bytes_to_read = file_size - bytes_read;
        if (bytes_to_read > ext4_ctx.block_size) {
            bytes_to_read = ext4_ctx.block_size;
        }
        
        // Read block
        if (ext4_read_block(inode.block[i], buf + bytes_read) != 0) {
            return -1;
        }
        
        bytes_read += bytes_to_read;
    }
    
    return 0;
}

// Get file information
static int ext4_get_file_info(const char* path, file_info_t* info) {
    ext4_inode_t inode;
    uint32_t inode_num;
    
    // Find file
    if (ext4_find_file(path, &inode, &inode_num) != 0) {
        return -1;
    }
    
    // Fill file information
    memory_set(info, 0, sizeof(file_info_t));
    
    // Extract filename from path
    const char* filename = strrchr(path, '/');
    if (filename) {
        filename++; // Skip '/'
    } else {
        filename = path;
    }
    
    strncpy(info->name, filename, sizeof(info->name) - 1);
    
    info->size = inode.size_lo;
    if (inode.size_high) {
        info->size |= ((uint64_t)inode.size_high) << 32;
    }
    
    info->is_directory = (inode.mode & 0x4000) != 0; // S_IFDIR
    info->creation_time = inode.ctime;
    info->modification_time = inode.mtime;
    
    return 0;
}

// Check if ext4 is supported
static bool ext4_is_supported(void) {
    // Check if ext4 module is available and hardware supports it
    return true;
}

// ext4 operations
static filesystem_ops_t ext4_ops = {
    .mount = ext4_mount,
    .unmount = ext4_unmount,
    .read_file = ext4_read_file,
    .write_file = NULL, // Read-only for bootloader
    .list_directory = NULL, // Not implemented in this example
    .get_file_info = ext4_get_file_info,
    .is_supported = ext4_is_supported
};

// Module metadata
static const module_header_t ext4_module_header = {
    .magic = MODULE_MAGIC,
    .version = 1,
    .type = MODULE_TYPE_FILESYSTEM,
    .flags = 0,
    .name = "ext4",
    .description = "ext4 filesystem support"
};

// Module interface
static filesystem_module_t ext4_module = {
    .base = {
        .init = NULL, // No initialization needed
        .cleanup = NULL // No cleanup needed
    },
    .fs_ops = ext4_ops
};

// Module entry point
int module_main(module_header_t* header) {
    if (header->magic != MODULE_MAGIC) {
        return -1;
    }
    
    *header = ext4_module_header;
    return (int)&ext4_module;
}
```

## 5. NTFS Implementation

### 5.1 NTFS Structure

NTFS is the Windows filesystem:

```c
// ntfs.h - NTFS filesystem implementation
#ifndef FERRYBOOT_NTFS_H
#define FERRYBOOT_NTFS_H

#include "fs_module.h"

// NTFS boot sector
typedef struct {
    uint8_t jump[3];
    uint8_t oem_id[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t reserved[3];
    uint16_t unused1;
    uint8_t media_type;
    uint16_t unused2;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t unused3;
    uint32_t unused4;
    uint64_t total_sectors;
    uint64_t mft_cluster;
    uint64_t mft_mirror_cluster;
    uint32_t clusters_per_mft_record;
    uint32_t clusters_per_index_record;
    uint64_t volume_serial;
    uint32_t checksum;
} __attribute__((packed)) ntfs_boot_sector_t;

// MFT record header
typedef struct {
    uint32_t signature;
    uint16_t usa_offset;
    uint16_t usa_count;
    uint64_t lsn;
    uint16_t sequence;
    uint16_t link_count;
    uint16_t attrs_offset;
    uint16_t flags;
    uint32_t bytes_in_use;
    uint32_t bytes_allocated;
    uint64_t base_mft_record;
    uint16_t next_attr_id;
    uint16_t reserved;
    uint32_t mft_record_number;
} __attribute__((packed)) ntfs_mft_record_t;

#endif // FERRYBOOT_NTFS_H
```

## 6. Module Registration and Management

### 6.1 Filesystem Module Manager

```c
// fs_manager.c - Filesystem module manager
#include "fs_module.h"
#include "module.h"
#include "hal.h"

// Registered filesystem modules
static filesystem_module_t* fs_modules[8];
static uint32_t fs_module_count = 0;

// Register filesystem module
int fs_register_module(filesystem_module_t* module) {
    if (fs_module_count >= 8) {
        return -1;
    }
    
    fs_modules[fs_module_count] = module;
    fs_module_count++;
    
    return 0;
}

// Detect filesystem type
filesystem_module_t* fs_detect_filesystem(const char* device) {
    for (uint32_t i = 0; i < fs_module_count; i++) {
        if (fs_modules[i]->fs_ops.is_supported) {
            // Try to mount filesystem
            if (fs_modules[i]->fs_ops.mount(device) == 0) {
                // Successfully mounted, this is the filesystem
                fs_modules[i]->fs_ops.unmount(); // Unmount for now
                return fs_modules[i];
            }
        }
    }
    
    return NULL;
}

// Mount filesystem with automatic detection
filesystem_module_t* fs_mount_auto(const char* device) {
    filesystem_module_t* module = fs_detect_filesystem(device);
    if (!module) {
        return NULL;
    }
    
    if (module->fs_ops.mount(device) != 0) {
        return NULL;
    }
    
    return module;
}

// Read file from any supported filesystem
int fs_read_file(const char* device, const char* path, void* buffer, size_t size) {
    // Mount filesystem
    filesystem_module_t* module = fs_mount_auto(device);
    if (!module) {
        return -1;
    }
    
    // Read file
    int result = module->fs_ops.read_file(path, buffer, size);
    
    // Unmount filesystem
    module->fs_ops.unmount();
    
    return result;
}

// Get file information from any supported filesystem
int fs_get_file_info(const char* device, const char* path, file_info_t* info) {
    // Mount filesystem
    filesystem_module_t* module = fs_mount_auto(device);
    if (!module) {
        return -1;
    }
    
    // Get file info
    int result = module->fs_ops.get_file_info(path, info);
    
    // Unmount filesystem
    module->fs_ops.unmount();
    
    return result;
}
```

## 7. Bootloader Integration

### 7.1 OS Detection Using Filesystem Modules

```c
// os_detection.c - Operating system detection using filesystem modules
#include "fs_manager.h"
#include "partition.h"

// Detect Windows
bool os_detect_windows(partition_t* partition) {
    file_info_t info;
    
    // Check for Windows bootloader files
    if (fs_get_file_info(partition->device_path, "/bootmgr", &info) == 0) {
        return true;
    }
    
    if (fs_get_file_info(partition->device_path, "/Windows/System32/winload.exe", &info) == 0) {
        return true;
    }
    
    return false;
}

// Detect Linux
bool os_detect_linux(partition_t* partition) {
    file_info_t info;
    
    // Check for Linux kernel files
    if (fs_get_file_info(partition->device_path, "/boot/vmlinuz", &info) == 0) {
        return true;
    }
    
    if (fs_get_file_info(partition->device_path, "/vmlinuz", &info) == 0) {
        return true;
    }
    
    return false;
}

// Detect BSD
bool os_detect_bsd(partition_t* partition) {
    file_info_t info;
    
    // Check for BSD kernel files
    if (fs_get_file_info(partition->device_path, "/boot/kernel/kernel", &info) == 0) {
        return true;
    }
    
    return false;
}

// Detect macOS
bool os_detect_macos(partition_t* partition) {
    file_info_t info;
    
    // Check for macOS bootloader files
    if (fs_get_file_info(partition->device_path, "/System/Library/CoreServices/boot.efi", &info) == 0) {
        return true;
    }
    
    return false;
}

// Detect operating system on partition
os_type_t os_detect(partition_t* partition) {
    // Try Windows first
    if (os_detect_windows(partition)) {
        return OS_WINDOWS;
    }
    
    // Try Linux
    if (os_detect_linux(partition)) {
        return OS_LINUX;
    }
    
    // Try BSD
    if (os_detect_bsd(partition)) {
        return OS_BSD;
    }
    
    // Try macOS
    if (os_detect_macos(partition)) {
        return OS_MACOS;
    }
    
    return OS_UNKNOWN;
}
```

## 8. Performance Considerations

### 8.1 Caching Strategies

1. **FAT Cache**: Cache frequently accessed FAT sectors
2. **Directory Cache**: Cache directory entries
3. **Inode Cache**: Cache recently accessed inodes (ext4)
4. **MFT Cache**: Cache MFT records (NTFS)

### 8.2 Memory Management

1. **Buffer Pooling**: Pre-allocate buffers for common operations
2. **Lazy Allocation**: Allocate memory only when needed
3. **Memory Reuse**: Reuse buffers for similar operations

### 8.3 Read Optimization

1. **Sequential Reads**: Optimize for sequential file access
2. **Cluster Reading**: Read entire clusters when possible
3. **Prefetching**: Prefetch likely needed data

## 9. Error Handling

### 9.1 Error Codes

```c
// fs_errors.h - Filesystem error codes
#define FS_ERROR_SUCCESS            0
#define FS_ERROR_INVALID_PARAM     -1
#define FS_ERROR_NOT_FOUND         -2
#define FS_ERROR_NO_PERMISSION     -3
#define FS_ERROR_IO_ERROR          -4
#define FS_ERROR_CORRUPTED         -5
#define FS_ERROR_UNSUPPORTED       -6
#define FS_ERROR_OUT_OF_MEMORY     -7
#define FS_ERROR_DISK_FULL         -8
#define FS_ERROR_TIMEOUT           -9
```

### 9.2 Error Recovery

1. **Retry Mechanism**: Retry failed operations with exponential backoff
2. **Fallback**: Use alternative methods when primary fails
3. **Logging**: Log errors for debugging (in debug mode)
4. **Graceful Degradation**: Continue with reduced functionality

## 10. Testing

### 10.1 Unit Tests

```c
// test_fat32.c - FAT32 unit tests
#include "fat32.h"
#include "test_framework.h"

void test_fat32_mount(void) {
    // Test mounting a valid FAT32 filesystem
    int result = fat32_mount("/dev/sda1");
    TEST_ASSERT_EQUAL(0, result);
}

void test_fat32_read_file(void) {
    // Test reading a file from FAT32 filesystem
    char buffer[1024];
    int result = fat32_read_file("/boot/ferryboot.cfg", buffer, sizeof(buffer));
    TEST_ASSERT_EQUAL(0, result);
}

void test_fat32_file_info(void) {
    // Test getting file information
    file_info_t info;
    int result = fat32_get_file_info("/boot/vmlinuz", &info);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_TRUE(info.size > 0);
}

TEST_LIST = {
    { "test_fat32_mount", test_fat32_mount },
    { "test_fat32_read_file", test_fat32_read_file },
    { "test_fat32_file_info", test_fat32_file_info },
    { NULL, NULL }
};
```

### 10.2 Integration Tests

```bash
# Test filesystem modules with QEMU
qemu-system-x86_64 -drive format=raw,file=test_fat32.img -m 512
qemu-system-x86_64 -drive format=raw,file=test_ext4.img -m 512
qemu-system-x86_64 -drive format=raw,file=test_ntfs.img -m 512
```

## 11. Conclusion

This filesystem module implementation guide provides a comprehensive framework for implementing filesystem support in FerryBoot. The modular design allows for easy addition of new filesystems while maintaining a consistent interface.

Key features of this implementation:

1. **Modular Architecture**: Each filesystem is implemented as a separate module
2. **Standard Interface**: All filesystems implement the same operations interface
3. **Automatic Detection**: Filesystems are automatically detected and mounted
4. **Read-Only Access**: Optimized for bootloader read-only access patterns
5. **Error Handling**: Robust error handling and recovery mechanisms
6. **Performance Optimization**: Caching and optimization strategies
7. **Testing Support**: Unit tests and integration testing framework

By following this guide, developers can implement robust filesystem support that enables FerryBoot to detect and boot multiple operating systems while providing access to configuration files and modules.
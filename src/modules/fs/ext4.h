#ifndef FERRYBOOT_EXT4_MODULE_H
#define FERRYBOOT_EXT4_MODULE_H

#include "../../common/module.h"
#include "../../common/hal.h"

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

// ext4 filesystem context
typedef struct {
    char device[32];
    ext4_superblock_t superblock;
    uint32_t block_size;
    bool mounted;
} ext4_context_t;

// ext4 module interface
typedef struct {
    module_interface_t base;
    ext4_context_t context;
} ext4_module_t;

#endif // FERRYBOOT_EXT4_MODULE_H
# FerryBoot Hardware Module Implementation Guide

## 1. Overview

This document provides detailed implementation guidance for hardware modules in FerryBoot. Hardware modules are essential for supporting various storage controllers, network adapters, input devices, and other hardware components across different platforms.

## 2. Module Architecture

### 2.1 Hardware Module Interface

All hardware modules must implement a standard interface:

```c
// hw_module.h - Hardware module interface
#ifndef FERRYBOOT_HW_MODULE_H
#define FERRYBOOT_HW_MODULE_H

#include "module.h"
#include "hal.h"

// Hardware operations
typedef struct {
    int (*initialize)(void);
    int (*detect)(void* data);
    int (*configure)(void* config);
    int (*read)(uint64_t address, void* buffer, size_t size);
    int (*write)(uint64_t address, const void* buffer, size_t size);
    int (*reset)(void);
    void (*cleanup)(void);
} hardware_ops_t;

// Hardware module interface
typedef struct {
    module_interface_t base;
    hardware_ops_t hw_ops;
} hardware_module_t;

// Hardware device information
typedef struct {
    uint32_t vendor_id;
    uint32_t device_id;
    uint32_t subsystem_vendor_id;
    uint32_t subsystem_device_id;
    uint32_t revision_id;
    uint32_t class_code;
    uint32_t sub_class_code;
    uint32_t prog_if;
    char device_name[64];
    void* private_data;
} hardware_device_info_t;

#endif // FERRYBOOT_HW_MODULE_H
```

### 2.2 Hardware Categories

FerryBoot supports several hardware categories:

1. **Storage Controllers** - IDE, SATA, NVMe
2. **Network Adapters** - Ethernet, WiFi
3. **Input Devices** - Keyboard, Mouse, Touchscreen
4. **Display Adapters** - VGA, VESA, UEFI GOP
5. **USB Controllers** - UHCI, OHCI, EHCI, XHCI
6. **Audio Devices** - PC Speaker, Sound Cards
7. **Power Management** - ACPI, APM

## 3. Storage Controller Modules

### 3.1 NVMe Implementation

NVMe is a high-performance storage interface:

```c
// nvme.h - NVMe storage controller implementation
#ifndef FERRYBOOT_NVME_H
#define FERRYBOOT_NVME_H

#include "hw_module.h"
#include "pci.h"

// NVMe registers
#define NVME_CAP    0x0000  // Controller Capabilities
#define NVME_VS     0x0008  // Version
#define NVME_INTMS  0x000C  // Interrupt Mask Set
#define NVME_INTMC  0x0010  // Interrupt Mask Clear
#define NVME_CC     0x0014  // Controller Configuration
#define NVME_CSTS   0x001C  // Controller Status
#define NVME_AQA    0x0024  // Admin Queue Attributes
#define NVME_ASQ    0x0028  // Admin Submission Queue Base Address
#define NVME_ACQ    0x0030  // Admin Completion Queue Base Address

// NVMe command opcodes
#define NVME_ADMIN_DELETE_SQ    0x00
#define NVME_ADMIN_CREATE_SQ    0x01
#define NVME_ADMIN_GET_LOG_PAGE 0x02
#define NVME_ADMIN_DELETE_CQ    0x04
#define NVME_ADMIN_CREATE_CQ    0x05
#define NVME_ADMIN_IDENTIFY     0x06
#define NVME_ADMIN_ABORT        0x08
#define NVME_ADMIN_SET_FEATURES 0x09
#define NVME_ADMIN_GET_FEATURES 0x0A

// NVMe identify commands
#define NVME_IDENTIFY_CNS_NAMESPACE 0x00
#define NVME_IDENTIFY_CNS_CONTROLLER 0x01

// NVMe controller capabilities
typedef struct {
    uint64_t cap;
    uint32_t vs;
    uint32_t intms;
    uint32_t intmc;
    uint32_t cc;
    uint32_t csts;
    uint32_t aqa;
    uint64_t asq;
    uint64_t acq;
} nvme_registers_t;

// NVMe identify controller structure
typedef struct {
    uint16_t vid;
    uint16_t ssvid;
    char sn[20];
    char mn[40];
    char fr[8];
    uint8_t rab;
    uint8_t ieee[3];
    uint8_t cmic;
    uint8_t mdts;
    uint16_t cntlid;
    uint32_t ver;
    uint32_t rtd3r;
    uint32_t rtd3e;
    uint32_t oaes;
    uint32_t ctratt;
    uint8_t rsvd100[156];
    uint16_t oacs;
    uint8_t acl;
    uint8_t aerl;
    uint8_t frmw;
    uint8_t lpa;
    uint8_t elpe;
    uint8_t npss;
    uint8_t avscc;
    uint8_t apsta;
    uint16_t wctemp;
    uint16_t cctemp;
    uint16_t mtfa;
    uint32_t hmpre;
    uint32_t hmmin;
    uint8_t tnvmcap[16];
    uint8_t unvmcap[16];
    uint32_t rpmbs;
    uint16_t edstt;
    uint8_t dsto;
    uint8_t fwug;
    uint16_t kas;
    uint16_t hctma;
    uint16_t mntmt;
    uint16_t mxtmt;
    uint32_t sanicap;
    uint8_t rsvd332[180];
    uint8_t sqes;
    uint8_t cqes;
    uint16_t maxcmd;
    uint32_t nn;
    uint16_t oncs;
    uint16_t fuses;
    uint8_t fna;
    uint8_t vwc;
    uint16_t awun;
    uint16_t awupf;
    uint8_t nvscc;
    uint8_t rsvd531;
    uint16_t acwu;
    uint8_t rsvd534[2];
    uint32_t sgls;
    uint8_t rsvd540[228];
    uint8_t subnqn[256];
    uint8_t rsvd1024[768];
    uint8_t rsvd1792[3072];
} __attribute__((packed)) nvme_identify_controller_t;

// NVMe identify namespace structure
typedef struct {
    uint64_t nsze;
    uint64_t ncap;
    uint64_t nuse;
    uint8_t nsfeat;
    uint8_t nlbaf;
    uint8_t flbas;
    uint8_t mc;
    uint8_t dpc;
    uint8_t dps;
    uint8_t nmic;
    uint8_t rescap;
    uint8_t fpi;
    uint8_t rsvd33;
    uint16_t nawun;
    uint16_t nawupf;
    uint16_t nacwu;
    uint16_t nabsn;
    uint16_t nabo;
    uint16_t nabspf;
    uint16_t rsvd46;
    uint8_t nvmcap[16];
    uint8_t rsvd64[40];
    uint8_t nguid[16];
    uint8_t eui64[8];
    struct {
        uint16_t ms;
        uint8_t lbads;
        uint8_t rp;
    } __attribute__((packed)) lbaf[16];
    uint8_t rsvd192[192];
    uint8_t vs[3712];
} __attribute__((packed)) nvme_identify_namespace_t;

// NVMe command structure
typedef struct {
    uint8_t opcode;
    uint8_t flags;
    uint16_t command_id;
    uint32_t nsid;
    uint32_t cdw2[2];
    uint64_t metadata;
    uint64_t prp1;
    uint64_t prp2;
    uint32_t cdw10[6];
} __attribute__((packed)) nvme_command_t;

// NVMe completion structure
typedef struct {
    uint32_t result;
    uint32_t rsvd;
    uint16_t sq_head;
    uint16_t sq_id;
    uint16_t command_id;
    uint16_t status;
} __attribute__((packed)) nvme_completion_t;

// NVMe queue structures
typedef struct {
    nvme_command_t* sq;
    nvme_completion_t* cq;
    uint32_t sq_head;
    uint32_t sq_tail;
    uint32_t cq_head;
    uint32_t sq_size;
    uint32_t cq_size;
    uint32_t sq_phase;
} nvme_queue_t;

// NVMe device context
typedef struct {
    pci_device_t pci_dev;
    nvme_registers_t* regs;
    nvme_queue_t admin_queue;
    nvme_identify_controller_t controller_info;
    nvme_identify_namespace_t namespace_info;
    uint32_t page_size;
    uint32_t max_queue_entries;
    bool initialized;
} nvme_context_t;

#endif // FERRYBOOT_NVME_H
```

```c
// nvme.c - NVMe storage controller implementation
#include "nvme.h"
#include "hal.h"
#include "memory.h"
#include "pci.h"

// Global NVMe context
static nvme_context_t nvme_ctx;

// NVMe initialization
static int nvme_initialize(void) {
    // Find NVMe controller via PCI
    if (pci_find_device(0x0108, 0x0000, &nvme_ctx.pci_dev) != 0) {
        return -1;
    }
    
    // Enable PCI device
    pci_enable_device(&nvme_ctx.pci_dev);
    
    // Map PCI BAR0 to memory
    nvme_ctx.regs = (nvme_registers_t*)pci_map_bar(&nvme_ctx.pci_dev, 0);
    if (!nvme_ctx.regs) {
        return -1;
    }
    
    // Check controller version
    if (nvme_ctx.regs->vs < 0x010000) {
        return -1; // Unsupported version
    }
    
    // Reset controller
    nvme_ctx.regs->cc = 0;
    while (nvme_ctx.regs->csts & 0x00000001) {
        // Wait for reset to complete
        hal_sleep(1);
    }
    
    // Initialize admin queues
    if (nvme_init_admin_queues() != 0) {
        return -1;
    }
    
    // Enable controller
    nvme_ctx.regs->cc = 0x00000001; // Enable controller
    while (!(nvme_ctx.regs->csts & 0x00000001)) {
        // Wait for controller to become ready
        hal_sleep(1);
    }
    
    // Identify controller
    if (nvme_identify_controller() != 0) {
        return -1;
    }
    
    // Identify namespace
    if (nvme_identify_namespace(1) != 0) {
        return -1;
    }
    
    nvme_ctx.initialized = true;
    return 0;
}

// Initialize admin queues
static int nvme_init_admin_queues(void) {
    // Allocate admin submission queue
    nvme_ctx.admin_queue.sq_size = 16;
    nvme_ctx.admin_queue.sq = memory_alloc(nvme_ctx.admin_queue.sq_size * sizeof(nvme_command_t));
    if (!nvme_ctx.admin_queue.sq) {
        return -1;
    }
    
    // Allocate admin completion queue
    nvme_ctx.admin_queue.cq_size = 16;
    nvme_ctx.admin_queue.cq = memory_alloc(nvme_ctx.admin_queue.cq_size * sizeof(nvme_completion_t));
    if (!nvme_ctx.admin_queue.cq) {
        memory_free(nvme_ctx.admin_queue.sq);
        return -1;
    }
    
    // Configure admin queues
    nvme_ctx.regs->aqa = ((nvme_ctx.admin_queue.cq_size - 1) << 16) | 
                         (nvme_ctx.admin_queue.sq_size - 1);
    nvme_ctx.regs->asq = (uint64_t)nvme_ctx.admin_queue.sq;
    nvme_ctx.regs->acq = (uint64_t)nvme_ctx.admin_queue.cq;
    
    return 0;
}

// Send NVMe command
static int nvme_submit_command(nvme_command_t* cmd, nvme_completion_t* cpl) {
    // Copy command to submission queue
    memory_copy(&nvme_ctx.admin_queue.sq[nvme_ctx.admin_queue.sq_tail], cmd, sizeof(nvme_command_t));
    
    // Update submission queue tail
    nvme_ctx.admin_queue.sq_tail = (nvme_ctx.admin_queue.sq_tail + 1) % nvme_ctx.admin_queue.sq_size;
    nvme_ctx.regs->asq = (uint64_t)&nvme_ctx.admin_queue.sq[nvme_ctx.admin_queue.sq_tail];
    
    // Wait for completion
    uint32_t timeout = 1000; // 1 second timeout
    while (timeout > 0) {
        // Check completion queue head
        if (nvme_ctx.admin_queue.cq_head != nvme_ctx.regs->acq) {
            // Copy completion
            memory_copy(cpl, &nvme_ctx.admin_queue.cq[nvme_ctx.admin_queue.cq_head], sizeof(nvme_completion_t));
            
            // Update completion queue head
            nvme_ctx.admin_queue.cq_head = (nvme_ctx.admin_queue.cq_head + 1) % nvme_ctx.admin_queue.cq_size;
            nvme_ctx.regs->acq = (uint64_t)&nvme_ctx.admin_queue.cq[nvme_ctx.admin_queue.cq_head];
            
            // Check status
            if ((cpl->status >> 1) == 0) {
                return 0; // Success
            } else {
                return -1; // Error
            }
        }
        
        hal_sleep(1);
        timeout--;
    }
    
    return -1; // Timeout
}

// Identify controller
static int nvme_identify_controller(void) {
    nvme_command_t cmd;
    nvme_completion_t cpl;
    
    // Prepare identify command
    memory_set(&cmd, 0, sizeof(cmd));
    cmd.opcode = NVME_ADMIN_IDENTIFY;
    cmd.nsid = 0;
    cmd.cdw10[0] = NVME_IDENTIFY_CNS_CONTROLLER;
    cmd.prp1 = (uint64_t)&nvme_ctx.controller_info;
    
    // Submit command
    return nvme_submit_command(&cmd, &cpl);
}

// Identify namespace
static int nvme_identify_namespace(uint32_t nsid) {
    nvme_command_t cmd;
    nvme_completion_t cpl;
    
    // Prepare identify command
    memory_set(&cmd, 0, sizeof(cmd));
    cmd.opcode = NVME_ADMIN_IDENTIFY;
    cmd.nsid = nsid;
    cmd.cdw10[0] = NVME_IDENTIFY_CNS_NAMESPACE;
    cmd.prp1 = (uint64_t)&nvme_ctx.namespace_info;
    
    // Submit command
    return nvme_submit_command(&cmd, &cpl);
}

// Read sectors from NVMe device
static int nvme_read_sectors(uint64_t lba, void* buffer, size_t count) {
    nvme_command_t cmd;
    nvme_completion_t cpl;
    
    // Prepare read command
    memory_set(&cmd, 0, sizeof(cmd));
    cmd.opcode = 0x02; // Read command
    cmd.nsid = 1; // Namespace 1
    cmd.prp1 = (uint64_t)buffer;
    cmd.cdw10[0] = lba & 0xFFFFFFFF;
    cmd.cdw10[1] = (lba >> 32) & 0xFFFFFFFF;
    cmd.cdw10[2] = count - 1; // Number of logical blocks
    
    // Submit command
    return nvme_submit_command(&cmd, &cpl);
}

// Write sectors to NVMe device
static int nvme_write_sectors(uint64_t lba, const void* buffer, size_t count) {
    nvme_command_t cmd;
    nvme_completion_t cpl;
    
    // Prepare write command
    memory_set(&cmd, 0, sizeof(cmd));
    cmd.opcode = 0x01; // Write command
    cmd.nsid = 1; // Namespace 1
    cmd.prp1 = (uint64_t)buffer;
    cmd.cdw10[0] = lba & 0xFFFFFFFF;
    cmd.cdw10[1] = (lba >> 32) & 0xFFFFFFFF;
    cmd.cdw10[2] = count - 1; // Number of logical blocks
    
    // Submit command
    return nvme_submit_command(&cmd, &cpl);
}

// Reset NVMe controller
static int nvme_reset(void) {
    if (!nvme_ctx.initialized) {
        return -1;
    }
    
    // Disable controller
    nvme_ctx.regs->cc = 0;
    while (nvme_ctx.regs->csts & 0x00000001) {
        // Wait for reset to complete
        hal_sleep(1);
    }
    
    // Re-enable controller
    nvme_ctx.regs->cc = 0x00000001;
    while (!(nvme_ctx.regs->csts & 0x00000001)) {
        // Wait for controller to become ready
        hal_sleep(1);
    }
    
    return 0;
}

// Cleanup NVMe controller
static void nvme_cleanup(void) {
    if (nvme_ctx.admin_queue.sq) {
        memory_free(nvme_ctx.admin_queue.sq);
        nvme_ctx.admin_queue.sq = NULL;
    }
    
    if (nvme_ctx.admin_queue.cq) {
        memory_free(nvme_ctx.admin_queue.cq);
        nvme_ctx.admin_queue.cq = NULL;
    }
    
    memory_set(&nvme_ctx, 0, sizeof(nvme_ctx));
}

// NVMe operations
static hardware_ops_t nvme_ops = {
    .initialize = nvme_initialize,
    .detect = NULL, // Not needed for PCI devices
    .configure = NULL, // Configuration done during initialization
    .read = (int (*)(uint64_t, void*, size_t))nvme_read_sectors,
    .write = (int (*)(uint64_t, const void*, size_t))nvme_write_sectors,
    .reset = nvme_reset,
    .cleanup = nvme_cleanup
};

// Module metadata
static const module_header_t nvme_module_header = {
    .magic = MODULE_MAGIC,
    .version = 1,
    .type = MODULE_TYPE_HARDWARE,
    .flags = 0,
    .name = "nvme",
    .description = "NVMe storage controller support"
};

// Module interface
static hardware_module_t nvme_module = {
    .base = {
        .init = NULL, // No initialization needed
        .cleanup = NULL // No cleanup needed
    },
    .hw_ops = nvme_ops
};

// Module entry point
int module_main(module_header_t* header) {
    if (header->magic != MODULE_MAGIC) {
        return -1;
    }
    
    *header = nvme_module_header;
    return (int)&nvme_module;
}
```

## 4. Network Adapter Modules

### 4.1 Ethernet Implementation

Ethernet is the most common network interface:

```c
// ethernet.h - Ethernet network adapter implementation
#ifndef FERRYBOOT_ETHERNET_H
#define FERRYBOOT_ETHERNET_H

#include "hw_module.h"
#include "pci.h"

// Ethernet frame structure
typedef struct {
    uint8_t dest_mac[6];
    uint8_t src_mac[6];
    uint16_t ethertype;
    uint8_t payload[1500];
    uint32_t fcs;
} __attribute__((packed)) ethernet_frame_t;

// Ethernet device context
typedef struct {
    pci_device_t pci_dev;
    uint8_t mac_address[6];
    uint32_t io_base;
    uint32_t mem_base;
    uint32_t irq;
    bool initialized;
} ethernet_context_t;

// Supported Ethernet chipsets
#define ETHERNET_VENDOR_INTEL    0x8086
#define ETHERNET_VENDOR_REALTEK  0x10EC
#define ETHERNET_VENDOR_AMD      0x1022

#endif // FERRYBOOT_ETHERNET_H
```

```c
// ethernet.c - Ethernet network adapter implementation
#include "ethernet.h"
#include "hal.h"
#include "memory.h"
#include "pci.h"

// Global Ethernet context
static ethernet_context_t eth_ctx;

// Initialize Ethernet adapter
static int ethernet_initialize(void) {
    // Find Ethernet controller via PCI
    if (pci_find_device(0x0200, 0x0000, &eth_ctx.pci_dev) != 0) {
        // Try specific vendors
        if (pci_find_device_by_vendor(ETHERNET_VENDOR_INTEL, 0x0000, &eth_ctx.pci_dev) != 0 &&
            pci_find_device_by_vendor(ETHERNET_VENDOR_REALTEK, 0x0000, &eth_ctx.pci_dev) != 0 &&
            pci_find_device_by_vendor(ETHERNET_VENDOR_AMD, 0x0000, &eth_ctx.pci_dev) != 0) {
            return -1;
        }
    }
    
    // Enable PCI device
    pci_enable_device(&eth_ctx.pci_dev);
    
    // Get PCI resources
    eth_ctx.io_base = pci_get_bar(&eth_ctx.pci_dev, 0);
    eth_ctx.mem_base = pci_get_bar(&eth_ctx.pci_dev, 1);
    eth_ctx.irq = pci_get_irq(&eth_ctx.pci_dev);
    
    // Initialize specific chipset
    switch (eth_ctx.pci_dev.vendor_id) {
        case ETHERNET_VENDOR_INTEL:
            return intel_ethernet_init();
        case ETHERNET_VENDOR_REALTEK:
            return realtek_ethernet_init();
        case ETHERNET_VENDOR_AMD:
            return amd_ethernet_init();
        default:
            return -1;
    }
}

// Intel Ethernet initialization
static int intel_ethernet_init(void) {
    // Reset device
    pci_write_reg32(&eth_ctx.pci_dev, 0x00, 0xFFFFFFFF);
    
    // Enable memory and I/O access
    pci_write_reg16(&eth_ctx.pci_dev, 0x04, 0x0006);
    
    // Read MAC address
    uint32_t mac_low = pci_read_reg32(&eth_ctx.pci_dev, 0x00);
    uint32_t mac_high = pci_read_reg32(&eth_ctx.pci_dev, 0x04);
    
    eth_ctx.mac_address[0] = mac_low & 0xFF;
    eth_ctx.mac_address[1] = (mac_low >> 8) & 0xFF;
    eth_ctx.mac_address[2] = (mac_low >> 16) & 0xFF;
    eth_ctx.mac_address[3] = (mac_low >> 24) & 0xFF;
    eth_ctx.mac_address[4] = mac_high & 0xFF;
    eth_ctx.mac_address[5] = (mac_high >> 8) & 0xFF;
    
    // Configure device
    // (Implementation specific to Intel chipset)
    
    eth_ctx.initialized = true;
    return 0;
}

// Send Ethernet frame
static int ethernet_send_frame(const ethernet_frame_t* frame, size_t frame_size) {
    if (!eth_ctx.initialized) {
        return -1;
    }
    
    // Check frame size
    if (frame_size > sizeof(ethernet_frame_t)) {
        return -1;
    }
    
    // Send frame through hardware
    // (Implementation specific to chipset)
    
    return 0;
}

// Receive Ethernet frame
static int ethernet_receive_frame(ethernet_frame_t* frame, size_t* frame_size) {
    if (!eth_ctx.initialized) {
        return -1;
    }
    
    // Check for received frames
    // (Implementation specific to chipset)
    
    return 0;
}

// Configure Ethernet adapter
static int ethernet_configure(void* config) {
    // Configure adapter with provided settings
    // (Implementation specific to chipset)
    
    return 0;
}

// Reset Ethernet adapter
static int ethernet_reset(void) {
    if (!eth_ctx.initialized) {
        return -1;
    }
    
    // Reset device
    // (Implementation specific to chipset)
    
    return 0;
}

// Cleanup Ethernet adapter
static void ethernet_cleanup(void) {
    if (eth_ctx.initialized) {
        // Disable device
        // (Implementation specific to chipset)
    }
    
    memory_set(&eth_ctx, 0, sizeof(eth_ctx));
}

// Ethernet operations
static hardware_ops_t ethernet_ops = {
    .initialize = ethernet_initialize,
    .detect = NULL, // Not needed for PCI devices
    .configure = ethernet_configure,
    .read = NULL, // Not applicable for network devices
    .write = NULL, // Not applicable for network devices
    .reset = ethernet_reset,
    .cleanup = ethernet_cleanup
};

// Module metadata
static const module_header_t ethernet_module_header = {
    .magic = MODULE_MAGIC,
    .version = 1,
    .type = MODULE_TYPE_HARDWARE,
    .flags = 0,
    .name = "ethernet",
    .description = "Ethernet network adapter support"
};

// Module interface
static hardware_module_t ethernet_module = {
    .base = {
        .init = NULL, // No initialization needed
        .cleanup = NULL // No cleanup needed
    },
    .hw_ops = ethernet_ops
};

// Module entry point
int module_main(module_header_t* header) {
    if (header->magic != MODULE_MAGIC) {
        return -1;
    }
    
    *header = ethernet_module_header;
    return (int)&ethernet_module;
}
```

## 5. Input Device Modules

### 5.1 PS/2 Keyboard Implementation

PS/2 is a legacy but still widely supported input interface:

```c
// ps2_keyboard.h - PS/2 keyboard implementation
#ifndef FERRYBOOT_PS2_KEYBOARD_H
#define FERRYBOOT_PS2_KEYBOARD_H

#include "hw_module.h"

// PS/2 ports
#define PS2_DATA_PORT    0x60
#define PS2_COMMAND_PORT 0x64

// PS/2 commands
#define PS2_CMD_DISABLE_KEYBOARD 0xAD
#define PS2_CMD_ENABLE_KEYBOARD  0xAE
#define PS2_CMD_SELF_TEST        0xAA
#define PS2_CMD_TEST_KEYBOARD    0xAB

// PS/2 keyboard context
typedef struct {
    bool initialized;
    bool caps_lock;
    bool num_lock;
    bool scroll_lock;
    uint8_t scancode_set;
} ps2_keyboard_context_t;

// Scancode mappings (simplified)
typedef struct {
    uint8_t scancode;
    uint8_t ascii;
    uint8_t ascii_shift;
} scancode_map_t;

#endif // FERRYBOOT_PS2_KEYBOARD_H
```

```c
// ps2_keyboard.c - PS/2 keyboard implementation
#include "ps2_keyboard.h"
#include "hal.h"
#include "memory.h"

// Global PS/2 keyboard context
static ps2_keyboard_context_t ps2_ctx;

// Scancode mappings (US keyboard layout)
static const scancode_map_t scancode_map[] = {
    {0x01, 0x1B, 0x1B}, // ESC
    {0x02, '1', '!'},
    {0x03, '2', '@'},
    {0x04, '3', '#'},
    {0x05, '4', '$'},
    {0x06, '5', '%'},
    {0x07, '6', '^'},
    {0x08, '7', '&'},
    {0x09, '8', '*'},
    {0x0A, '9', '('},
    {0x0B, '0', ')'},
    {0x0C, '-', '_'},
    {0x0D, '=', '+'},
    {0x0E, 0x08, 0x08}, // Backspace
    {0x0F, 0x09, 0x09}, // Tab
    {0x10, 'q', 'Q'},
    {0x11, 'w', 'W'},
    {0x12, 'e', 'E'},
    {0x13, 'r', 'R'},
    {0x14, 't', 'T'},
    {0x15, 'y', 'Y'},
    {0x16, 'u', 'U'},
    {0x17, 'i', 'I'},
    {0x18, 'o', 'O'},
    {0x19, 'p', 'P'},
    {0x1A, '[', '{'},
    {0x1B, ']', '}'},
    {0x1C, 0x0D, 0x0D}, // Enter
    {0x1D, 0x00, 0x00}, // Left Control
    {0x1E, 'a', 'A'},
    {0x1F, 's', 'S'},
    // ... more mappings
};

// Wait for PS/2 input buffer to be empty
static void ps2_wait_input_empty(void) {
    while (hal_inportb(PS2_COMMAND_PORT) & 0x02) {
        hal_sleep(1);
    }
}

// Wait for PS/2 output buffer to be full
static void ps2_wait_output_full(void) {
    while (!(hal_inportb(PS2_COMMAND_PORT) & 0x01)) {
        hal_sleep(1);
    }
}

// Send command to PS/2 controller
static int ps2_send_command(uint8_t command) {
    ps2_wait_input_empty();
    hal_outportb(PS2_COMMAND_PORT, command);
    return 0;
}

// Send data to PS/2 device
static int ps2_send_data(uint8_t data) {
    ps2_wait_input_empty();
    hal_outportb(PS2_DATA_PORT, data);
    return 0;
}

// Read data from PS/2 device
static uint8_t ps2_read_data(void) {
    ps2_wait_output_full();
    return hal_inportb(PS2_DATA_PORT);
}

// Initialize PS/2 keyboard
static int ps2_keyboard_initialize(void) {
    // Disable keyboard
    ps2_send_command(PS2_CMD_DISABLE_KEYBOARD);
    
    // Flush output buffer
    while (hal_inportb(PS2_COMMAND_PORT) & 0x01) {
        hal_inportb(PS2_DATA_PORT);
    }
    
    // Run self test
    ps2_send_command(PS2_CMD_SELF_TEST);
    if (ps2_read_data() != 0x55) {
        return -1;
    }
    
    // Test keyboard
    ps2_send_command(PS2_CMD_TEST_KEYBOARD);
    if (ps2_read_data() != 0x00) {
        return -1;
    }
    
    // Enable keyboard
    ps2_send_command(PS2_CMD_ENABLE_KEYBOARD);
    
    // Reset keyboard
    ps2_send_data(0xFF);
    if (ps2_read_data() != 0xFA) { // ACK
        return -1;
    }
    
    // Get scancode set
    ps2_send_data(0xF0);
    ps2_send_data(0x00);
    if (ps2_read_data() == 0xFA) { // ACK
        ps2_ctx.scancode_set = ps2_read_data();
    }
    
    // Set default LED state
    ps2_send_data(0xED);
    if (ps2_read_data() == 0xFA) { // ACK
        ps2_send_data(0x00); // All LEDs off
        ps2_read_data(); // ACK
    }
    
    ps2_ctx.initialized = true;
    return 0;
}

// Read key from PS/2 keyboard
static uint32_t ps2_keyboard_read_key(void) {
    if (!ps2_ctx.initialized) {
        return 0;
    }
    
    // Check if data is available
    if (!(hal_inportb(PS2_COMMAND_PORT) & 0x01)) {
        return 0;
    }
    
    // Read scancode
    uint8_t scancode = hal_inportb(PS2_DATA_PORT);
    
    // Handle special keys
    switch (scancode) {
        case 0x3A: // Caps Lock
            ps2_ctx.caps_lock = !ps2_ctx.caps_lock;
            return 0;
        case 0x45: // Num Lock
            ps2_ctx.num_lock = !ps2_ctx.num_lock;
            return 0;
        case 0x46: // Scroll Lock
            ps2_ctx.scroll_lock = !ps2_ctx.scroll_lock;
            return 0;
        case 0xE0: // Extended key
            // Handle extended keys (arrow keys, function keys, etc.)
            return 0xE000 | hal_inportb(PS2_DATA_PORT);
    }
    
    // Convert scancode to ASCII
    for (int i = 0; i < sizeof(scancode_map) / sizeof(scancode_map[0]); i++) {
        if (scancode_map[i].scancode == scancode) {
            // Check for shift key (simplified)
            bool shift = false; // Would need to check for shift key state
            return shift ? scancode_map[i].ascii_shift : scancode_map[i].ascii;
        }
    }
    
    return 0;
}

// Check if key is pressed
static bool ps2_keyboard_key_pressed(void) {
    return (hal_inportb(PS2_COMMAND_PORT) & 0x01) != 0;
}

// Reset PS/2 keyboard
static int ps2_keyboard_reset(void) {
    if (!ps2_ctx.initialized) {
        return -1;
    }
    
    // Reset keyboard
    ps2_send_data(0xFF);
    return ps2_read_data() == 0xFA ? 0 : -1; // Check for ACK
}

// Cleanup PS/2 keyboard
static void ps2_keyboard_cleanup(void) {
    if (ps2_ctx.initialized) {
        ps2_send_command(PS2_CMD_DISABLE_KEYBOARD);
    }
    
    memory_set(&ps2_ctx, 0, sizeof(ps2_ctx));
}

// PS/2 keyboard operations
static hardware_ops_t ps2_keyboard_ops = {
    .initialize = ps2_keyboard_initialize,
    .detect = NULL, // Always detectable via ports
    .configure = NULL, // No configuration needed
    .read = NULL, // Use specialized read function
    .write = NULL, // Not applicable for input device
    .reset = ps2_keyboard_reset,
    .cleanup = ps2_keyboard_cleanup
};

// Module metadata
static const module_header_t ps2_keyboard_module_header = {
    .magic = MODULE_MAGIC,
    .version = 1,
    .type = MODULE_TYPE_HARDWARE,
    .flags = 0,
    .name = "ps2_keyboard",
    .description = "PS/2 keyboard support"
};

// Module interface
static hardware_module_t ps2_keyboard_module = {
    .base = {
        .init = NULL, // No initialization needed
        .cleanup = NULL // No cleanup needed
    },
    .hw_ops = ps2_keyboard_ops
};

// Specialized functions for keyboard
uint32_t ps2_keyboard_get_key(void) {
    return ps2_keyboard_read_key();
}

bool ps2_keyboard_is_pressed(void) {
    return ps2_keyboard_key_pressed();
}

// Module entry point
int module_main(module_header_t* header) {
    if (header->magic != MODULE_MAGIC) {
        return -1;
    }
    
    *header = ps2_keyboard_module_header;
    return (int)&ps2_keyboard_module;
}
```

## 6. Display Adapter Modules

### 6.1 VGA Implementation

VGA is the legacy display standard with universal support:

```c
// vga.h - VGA display adapter implementation
#ifndef FERRYBOOT_VGA_H
#define FERRYBOOT_VGA_H

#include "hw_module.h"

// VGA registers
#define VGA_CRTC_INDEX 0x3D4
#define VGA_CRTC_DATA  0x3D5
#define VGA_ATTR_INDEX 0x3C0
#define VGA_ATTR_DATA  0x3C1
#define VGA_MISC_OUTPUT 0x3C2
#define VGA_SEQUENCER_INDEX 0x3C4
#define VGA_SEQUENCER_DATA  0x3C5
#define VGA_GRAPHICS_INDEX  0x3CE
#define VGA_GRAPHICS_DATA   0x3CF

// VGA text mode dimensions
#define VGA_TEXT_WIDTH  80
#define VGA_TEXT_HEIGHT 25
#define VGA_TEXT_BUFFER 0xB8000

// VGA graphics mode dimensions
#define VGA_GRAPHICS_WIDTH  640
#define VGA_GRAPHICS_HEIGHT 480
#define VGA_GRAPHICS_BUFFER 0xA0000

// VGA color palette
#define VGA_PALETTE_INDEX 0x3C8
#define VGA_PALETTE_DATA  0x3C9

// VGA mode information
typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t buffer_address;
    bool text_mode;
} vga_mode_t;

// VGA context
typedef struct {
    vga_mode_t current_mode;
    uint8_t* text_buffer;
    uint8_t* graphics_buffer;
    bool initialized;
} vga_context_t;

#endif // FERRYBOOT_VGA_H
```

```c
// vga.c - VGA display adapter implementation
#include "vga.h"
#include "hal.h"
#include "memory.h"

// Global VGA context
static vga_context_t vga_ctx;

// Set VGA text mode
static int vga_set_text_mode(void) {
    // Set 80x25 text mode (mode 03h)
    hal_outportb(0x3C2, 0x67);
    hal_outportb(0x3C4, 0x01);
    hal_outportb(0x3C5, 0x01);
    hal_outportb(0x3C4, 0x04);
    hal_outportb(0x3C5, 0x06);
    hal_outportb(0x3CE, 0x05);
    hal_outportb(0x3CF, 0x00);
    hal_outportb(0x3CE, 0x06);
    hal_outportb(0x3CF, 0x05);
    
    // Clear screen
    memory_set((void*)VGA_TEXT_BUFFER, 0, VGA_TEXT_WIDTH * VGA_TEXT_HEIGHT * 2);
    
    // Set current mode
    vga_ctx.current_mode.width = VGA_TEXT_WIDTH;
    vga_ctx.current_mode.height = VGA_TEXT_HEIGHT;
    vga_ctx.current_mode.depth = 4; // 16 colors
    vga_ctx.current_mode.buffer_address = VGA_TEXT_BUFFER;
    vga_ctx.current_mode.text_mode = true;
    
    return 0;
}

// Set VGA graphics mode
static int vga_set_graphics_mode(uint32_t width, uint32_t height, uint32_t depth) {
    // Set 640x480x16 graphics mode (mode 12h)
    if (width == 640 && height == 480 && depth == 4) {
        // Implementation for mode 12h
        // (Simplified implementation)
        
        // Set current mode
        vga_ctx.current_mode.width = width;
        vga_ctx.current_mode.height = height;
        vga_ctx.current_mode.depth = depth;
        vga_ctx.current_mode.buffer_address = VGA_GRAPHICS_BUFFER;
        vga_ctx.current_mode.text_mode = false;
        
        return 0;
    }
    
    return -1; // Unsupported mode
}

// Set video mode
static int vga_set_video_mode(uint32_t width, uint32_t height, uint32_t depth) {
    if (width == VGA_TEXT_WIDTH && height == VGA_TEXT_HEIGHT && depth == 4) {
        return vga_set_text_mode();
    } else {
        return vga_set_graphics_mode(width, height, depth);
    }
}

// Put pixel in graphics mode
static void vga_put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (vga_ctx.current_mode.text_mode) {
        return; // Not applicable in text mode
    }
    
    if (x >= vga_ctx.current_mode.width || y >= vga_ctx.current_mode.height) {
        return; // Out of bounds
    }
    
    // Calculate pixel address
    uint32_t addr = vga_ctx.current_mode.buffer_address + (y * vga_ctx.current_mode.width) + x;
    *(uint8_t*)addr = color & 0x0F;
}

// Clear screen
static void vga_clear_screen(uint32_t color) {
    if (vga_ctx.current_mode.text_mode) {
        // Clear text mode screen
        uint16_t* buffer = (uint16_t*)vga_ctx.current_mode.buffer_address;
        uint16_t entry = (color << 8) | ' ';
        
        for (int i = 0; i < VGA_TEXT_WIDTH * VGA_TEXT_HEIGHT; i++) {
            buffer[i] = entry;
        }
    } else {
        // Clear graphics mode screen
        memory_set((void*)vga_ctx.current_mode.buffer_address, 
                   color & 0x0F, 
                   vga_ctx.current_mode.width * vga_ctx.current_mode.height);
    }
}

// Draw rectangle
static void vga_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    if (vga_ctx.current_mode.text_mode) {
        return; // Not implemented for text mode
    }
    
    for (uint32_t py = y; py < y + height && py < vga_ctx.current_mode.height; py++) {
        for (uint32_t px = x; px < x + width && px < vga_ctx.current_mode.width; px++) {
            vga_put_pixel(px, py, color);
        }
    }
}

// Initialize VGA adapter
static int vga_initialize(void) {
    // Initialize to text mode by default
    if (vga_set_text_mode() != 0) {
        return -1;
    }
    
    vga_ctx.text_buffer = (uint8_t*)VGA_TEXT_BUFFER;
    vga_ctx.graphics_buffer = (uint8_t*)VGA_GRAPHICS_BUFFER;
    vga_ctx.initialized = true;
    
    return 0;
}

// Reset VGA adapter
static int vga_reset(void) {
    if (!vga_ctx.initialized) {
        return -1;
    }
    
    // Reset to text mode
    return vga_set_text_mode();
}

// Cleanup VGA adapter
static void vga_cleanup(void) {
    if (vga_ctx.initialized) {
        vga_set_text_mode(); // Reset to text mode
    }
    
    memory_set(&vga_ctx, 0, sizeof(vga_ctx));
}

// VGA operations
static hardware_ops_t vga_ops = {
    .initialize = vga_initialize,
    .detect = NULL, // Always available
    .configure = NULL, // Configuration done through set_video_mode
    .read = NULL, // Not applicable
    .write = NULL, // Use specialized functions
    .reset = vga_reset,
    .cleanup = vga_cleanup
};

// Module metadata
static const module_header_t vga_module_header = {
    .magic = MODULE_MAGIC,
    .version = 1,
    .type = MODULE_TYPE_HARDWARE,
    .flags = 0,
    .name = "vga",
    .description = "VGA display adapter support"
};

// Module interface
static hardware_module_t vga_module = {
    .base = {
        .init = NULL, // No initialization needed
        .cleanup = NULL // No cleanup needed
    },
    .hw_ops = vga_ops
};

// Specialized functions for display
int vga_set_mode(uint32_t width, uint32_t height, uint32_t depth) {
    return vga_set_video_mode(width, height, depth);
}

void vga_put_pixel_at(uint32_t x, uint32_t y, uint32_t color) {
    vga_put_pixel(x, y, color);
}

void vga_clear(uint32_t color) {
    vga_clear_screen(color);
}

void vga_draw_rectangle(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    vga_draw_rect(x, y, width, height, color);
}

// Module entry point
int module_main(module_header_t* header) {
    if (header->magic != MODULE_MAGIC) {
        return -1;
    }
    
    *header = vga_module_header;
    return (int)&vga_module;
}
```

## 7. Module Registration and Management

### 7.1 Hardware Module Manager

```c
// hw_manager.c - Hardware module manager
#include "hw_module.h"
#include "module.h"
#include "hal.h"

// Registered hardware modules
static hardware_module_t* hw_modules[16];
static uint32_t hw_module_count = 0;

// Register hardware module
int hw_register_module(hardware_module_t* module) {
    if (hw_module_count >= 16) {
        return -1;
    }
    
    hw_modules[hw_module_count] = module;
    hw_module_count++;
    
    return 0;
}

// Initialize all hardware modules
int hw_initialize_all(void) {
    for (uint32_t i = 0; i < hw_module_count; i++) {
        if (hw_modules[i]->hw_ops.initialize) {
            int result = hw_modules[i]->hw_ops.initialize();
            if (result != 0) {
                return result;
            }
        }
    }
    
    return 0;
}

// Detect hardware modules
int hw_detect_modules(void) {
    // In a real implementation, this would scan for available hardware
    // and load appropriate modules
    
    return 0;
}

// Reset hardware module
int hw_reset_module(const char* module_name) {
    for (uint32_t i = 0; i < hw_module_count; i++) {
        if (strcmp(hw_modules[i]->base.name, module_name) == 0) {
            if (hw_modules[i]->hw_ops.reset) {
                return hw_modules[i]->hw_ops.reset();
            }
            return -1;
        }
    }
    
    return -1; // Module not found
}

// Cleanup all hardware modules
void hw_cleanup_all(void) {
    for (uint32_t i = 0; i < hw_module_count; i++) {
        if (hw_modules[i]->hw_ops.cleanup) {
            hw_modules[i]->hw_ops.cleanup();
        }
    }
}
```

## 8. Bootloader Integration

### 8.1 Hardware Detection During Boot

```c
// hw_detection.c - Hardware detection during boot
#include "hw_manager.h"
#include "pci.h"

// Detect storage controllers
int hw_detect_storage_controllers(void) {
    pci_device_t pci_dev;
    
    // Scan for NVMe controllers
    if (pci_find_device(0x0108, 0x0000, &pci_dev) == 0) {
        // Load NVMe module
        module_load("nvme.mod");
    }
    
    // Scan for AHCI controllers
    if (pci_find_device(0x0106, 0x0000, &pci_dev) == 0) {
        // Load AHCI module (not implemented in this example)
    }
    
    // Scan for IDE controllers
    if (pci_find_device(0x0101, 0x0000, &pci_dev) == 0) {
        // Load IDE module (not implemented in this example)
    }
    
    return 0;
}

// Detect network adapters
int hw_detect_network_adapters(void) {
    pci_device_t pci_dev;
    
    // Scan for Ethernet controllers
    if (pci_find_device(0x0200, 0x0000, &pci_dev) == 0) {
        // Load Ethernet module
        module_load("ethernet.mod");
    }
    
    // Scan for WiFi controllers
    if (pci_find_device(0x0280, 0x0000, &pci_dev) == 0) {
        // Load WiFi module (not implemented in this example)
    }
    
    return 0;
}

// Detect input devices
int hw_detect_input_devices(void) {
    // Try to detect PS/2 keyboard
    // This is a simplified check
    uint8_t status = hal_inportb(0x64);
    if ((status & 0x10) == 0) { // No timeout
        // Load PS/2 keyboard module
        module_load("ps2_keyboard.mod");
    }
    
    // Try to detect USB devices
    // (Implementation would go here)
    
    return 0;
}

// Detect display adapters
int hw_detect_display_adapters(void) {
    // VGA is always available
    module_load("vga.mod");
    
    // Try to detect VESA support
    // (Implementation would go here)
    
    return 0;
}

// Detect all hardware
int hw_detect_all(void) {
    hw_detect_storage_controllers();
    hw_detect_network_adapters();
    hw_detect_input_devices();
    hw_detect_display_adapters();
    
    return 0;
}
```

## 9. Performance Considerations

### 9.1 Hardware Access Optimization

1. **Port I/O Caching**: Cache frequently accessed I/O ports
2. **Memory Mapping**: Use memory-mapped I/O when available
3. **DMA Transfers**: Use DMA for large data transfers
4. **Interrupt Handling**: Efficient interrupt handling mechanisms

### 9.2 Resource Management

1. **Buffer Pooling**: Pre-allocate buffers for common operations
2. **Lazy Initialization**: Initialize hardware only when needed
3. **Power Management**: Implement power saving features
4. **Resource Sharing**: Share resources between similar devices

## 10. Error Handling

### 10.1 Error Codes

```c
// hw_errors.h - Hardware error codes
#define HW_ERROR_SUCCESS            0
#define HW_ERROR_INVALID_PARAM     -1
#define HW_ERROR_DEVICE_NOT_FOUND  -2
#define HW_ERROR_DEVICE_BUSY       -3
#define HW_ERROR_IO_ERROR          -4
#define HW_ERROR_TIMEOUT           -5
#define HW_ERROR_UNSUPPORTED       -6
#define HW_ERROR_OUT_OF_MEMORY     -7
#define HW_ERROR_CORRUPTED         -8
#define HW_ERROR_PERMISSION_DENIED -9
```

### 10.2 Error Recovery

1. **Retry Mechanism**: Retry failed operations with exponential backoff
2. **Fallback Strategies**: Use alternative hardware when primary fails
3. **Graceful Degradation**: Continue with reduced functionality
4. **Error Logging**: Log errors for debugging (in debug mode)

## 11. Testing

### 11.1 Unit Tests

```c
// test_nvme.c - NVMe unit tests
#include "nvme.h"
#include "test_framework.h"

void test_nvme_initialize(void) {
    // Test NVMe controller initialization
    int result = nvme_initialize();
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_TRUE(nvme_ctx.initialized);
}

void test_nvme_identify(void) {
    // Test NVMe identify commands
    int result = nvme_identify_controller();
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_TRUE(nvme_ctx.controller_info.vid != 0);
}

void test_nvme_read_write(void) {
    // Test NVMe read/write operations
    uint8_t buffer[512];
    int result = nvme_read_sectors(0, buffer, 1);
    TEST_ASSERT_EQUAL(0, result);
    
    // Test write (read-only in bootloader)
    // result = nvme_write_sectors(0, buffer, 1);
    // TEST_ASSERT_EQUAL(0, result);
}

TEST_LIST = {
    { "test_nvme_initialize", test_nvme_initialize },
    { "test_nvme_identify", test_nvme_identify },
    { "test_nvme_read_write", test_nvme_read_write },
    { NULL, NULL }
};
```

### 11.2 Integration Tests

```bash
# Test hardware modules with QEMU
qemu-system-x86_64 -drive format=raw,file=test_disk.img -netdev user,id=net0 -device e1000,netdev=net0 -m 512

# Test with specific hardware configurations
qemu-system-x86_64 -drive format=raw,file=test_disk.img -device nvme,serial=1234 -m 512
```

## 12. Conclusion

This hardware module implementation guide provides a comprehensive framework for implementing hardware support in FerryBoot. The modular design allows for easy addition of new hardware types while maintaining a consistent interface.

Key features of this implementation:

1. **Modular Architecture**: Each hardware type is implemented as a separate module
2. **Standard Interface**: All hardware modules implement the same operations interface
3. **Automatic Detection**: Hardware is automatically detected and initialized
4. **Performance Optimization**: Efficient access patterns and resource management
5. **Error Handling**: Robust error handling and recovery mechanisms
6. **Testing Support**: Unit tests and integration testing framework
7. **Cross-Platform Support**: Consistent interface across BIOS and UEFI platforms

By following this guide, developers can implement robust hardware support that enables FerryBoot to work with a wide variety of hardware configurations while maintaining its lightweight and fast characteristics.
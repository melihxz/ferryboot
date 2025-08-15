# FerryBoot Installation Guide

## Overview

This guide explains how to install FerryBoot on different systems and configurations.

## Prerequisites

Before installing FerryBoot, ensure you have:
- A backup of your current boot configuration
- Administrative privileges
- A USB drive (for testing)
- Appropriate installation tools

## Installation Methods

### 1. BIOS Systems (MBR)

#### Installing to Disk
```bash
# Install to MBR (first 512 bytes)
sudo dd if=bin/ferryboot_bios.bin of=/dev/sdX bs=512 count=1

# Install to partition (for chainloading)
sudo dd if=bin/ferryboot_bios.bin of=/dev/sdX1 bs=512
```

#### Installing with GRUB Chainloading
```bash
# Copy FerryBoot to boot partition
sudo cp bin/ferryboot_bios.bin /boot/ferryboot.bin

# Add to GRUB configuration
echo 'menuentry "FerryBoot" { chainloader /boot/ferryboot.bin }' | sudo tee -a /etc/grub.d/40_custom

# Update GRUB
sudo update-grub
```

### 2. UEFI Systems (GPT)

#### Installing to EFI System Partition
```bash
# Mount EFI System Partition
sudo mkdir -p /boot/efi/EFI/FERRYBOOT
sudo mount /dev/sdX1 /boot/efi

# Copy bootloader
sudo cp bin/ferryboot_uefi.efi /boot/efi/EFI/FERRYBOOT/ferryboot.efi

# Create boot entry
sudo efibootmgr -c -L "FerryBoot" -l "\EFI\FERRYBOOT\ferryboot.efi" -d /dev/sdX -p 1
```

#### Installing with Fallback
```bash
# Copy to fallback location
sudo cp bin/ferryboot_uefi.efi /boot/efi/EFI/BOOT/BOOTX64.EFI
```

## Creating Bootable Media

### USB Drive Installation

#### Using dd (Linux)
```bash
# Create bootable USB
sudo dd if=bin/ferryboot_bios.bin of=/dev/sdX bs=512
sync
```

#### Using Rufus (Windows)
1. Download Rufus
2. Select FerryBoot image
3. Choose USB drive
4. Click "Start"

#### Using balenaEtcher (Cross-platform)
1. Download balenaEtcher
2. Select FerryBoot image
3. Select USB drive
4. Click "Flash"

## Configuration After Installation

### First Boot

1. **Setup Wizard**: On first boot, FerryBoot will launch the setup wizard
2. **Quick Setup**: For most users, select Quick Setup
3. **Advanced Setup**: For custom configurations, select Advanced Setup
4. **Save Configuration**: Apply and save settings

### Manual Configuration

```bash
# Launch configuration tool
sudo ferryboot-config

# Set default boot entry
sudo ferryboot-config --set default_entry=0

# Set boot timeout
sudo ferryboot-config --set timeout=10

# Enable password protection
sudo ferryboot-config --set password_protected=true
```

## Dual-Boot Configuration

### Automatic Detection

FerryBoot automatically detects installed operating systems:
- Windows (NT 5.0+)
- Linux distributions
- BSD variants
- macOS (on compatible hardware)

### Manual Entry Addition

```bash
# Add Windows entry
sudo ferryboot-config --add-entry "Windows 11" \
  --path "(hd0,1)/Windows/System32/winload.efi" \
  --type=windows

# Add Linux entry
sudo ferryboot-config --add-entry "Ubuntu" \
  --path "(hd0,2)/boot/vmlinuz" \
  --initrd="(hd0,2)/boot/initrd.img" \
  --parameters="root=/dev/sda2 ro quiet splash" \
  --type=linux
```

## Migration from Other Bootloaders

### From GRUB

1. **Backup GRUB configuration**:
   ```bash
   sudo cp /etc/default/grub /etc/default/grub.backup
   sudo cp -r /etc/grub.d /etc/grub.d.backup
   ```

2. **Install FerryBoot** (see above)

3. **Import GRUB entries**:
   ```bash
   # Parse GRUB configuration
   sudo ferryboot-import-grub
   ```

### From rEFInd

1. **Backup rEFInd configuration**:
   ```bash
   sudo cp -r /boot/efi/EFI/refind /boot/efi/EFI/refind.backup
   ```

2. **Install FerryBoot** (see above)

3. **Import rEFInd entries**:
   ```bash
   # Parse rEFInd configuration
   sudo ferryboot-import-refind
   ```

## Secure Boot Configuration

### Enrolling FerryBoot Keys

1. **Enter UEFI firmware settings**
2. **Navigate to Secure Boot options**
3. **Enter Setup Mode**
4. **Enroll FerryBoot certificate**:
   ```bash
   # Enroll certificate
   sudo sbsign --cert /usr/share/ferryboot/ferryboot.cert \
     --sign bin/ferryboot_uefi.efi
   ```

### Using Shim

1. **Install shim**:
   ```bash
   sudo apt install shim-signed
   ```

2. **Copy FerryBoot with shim**:
   ```bash
   sudo cp /usr/lib/shim/shimx64.efi.signed /boot/efi/EFI/FERRYBOOT/ferryboot.efi
   sudo cp bin/ferryboot_uefi.efi /boot/efi/EFI/FERRYBOOT/ferryboot_real.efi
   ```

## Network Boot (PXE)

### Setting up PXE Server

1. **Install DHCP and TFTP servers**:
   ```bash
   sudo apt install isc-dhcp-server tftpd-hpa
   ```

2. **Configure DHCP**:
   ```bash
   # /etc/dhcp/dhcpd.conf
   next-server 192.168.1.100;
   filename "ferryboot_pxe.efi";
   ```

3. **Copy FerryBoot to TFTP directory**:
   ```bash
   sudo cp bin/ferryboot_pxe.efi /var/tftpboot/
   ```

## Troubleshooting

### Common Issues

#### Bootloader Not Starting
1. **Check installation**:
   ```bash
   # Verify MBR
   sudo hexdump -C /dev/sdX | head -n 1
   
   # Verify EFI entry
   efibootmgr -v
   ```

2. **Reinstall bootloader** (see installation methods above)

#### Missing Operating Systems
1. **Run detection**:
   ```bash
   sudo ferryboot-detect-os
   ```

2. **Manually add entries** (see dual-boot configuration)

#### Password Forgotten
1. **Boot from recovery media**
2. **Mount boot partition**
3. **Remove password hash**:
   ```bash
   sudo ferryboot-config --set password_protected=false
   ```

### Recovery Options

#### Built-in Recovery
1. **Hold Shift during boot** to access recovery menu
2. **Select "Recovery Console"**
3. **Use command-line tools to fix issues**

#### External Recovery
1. **Create recovery USB** with FerryBoot tools
2. **Boot from USB**
3. **Mount system partitions**
4. **Repair bootloader installation**

## Advanced Configuration

### Custom Modules
```bash
# Install custom module
sudo cp mymodule.mod /boot/ferryboot/modules/

# Load module at boot
sudo ferryboot-config --add-module mymodule
```

### Custom Themes
```bash
# Install custom theme
sudo cp mytheme.zip /boot/ferryboot/themes/

# Apply theme
sudo ferryboot-config --set theme=mytheme
```

### Boot Parameters
```bash
# Set global boot parameters
sudo ferryboot-config --set global_parameters="quiet splash"

# Set per-entry parameters
sudo ferryboot-config --set-entry-parameters 0 "nomodeset acpi=off"
```

## Verification

### Check Installation
```bash
# Verify BIOS installation
sudo ferryboot-verify --bios /dev/sdX

# Verify UEFI installation
sudo ferryboot-verify --uefi /dev/sdX1
```

### Test Boot
```bash
# Test in QEMU
qemu-system-x86_64 -drive format=raw,file=/dev/sdX -m 512
```

## Uninstallation

### BIOS Systems
```bash
# Restore previous bootloader
sudo dd if=/path/to/backup.mbr of=/dev/sdX bs=512 count=1

# Or reinstall GRUB
sudo grub-install /dev/sdX
```

### UEFI Systems
```bash
# Remove FerryBoot entry
sudo efibootmgr -b XXXX -B

# Remove files
sudo rm -rf /boot/efi/EFI/FERRYBOOT

# Reinstall another bootloader if needed
```

## Best Practices

### Before Installation
- Backup current boot configuration
- Document existing OS locations
- Verify system compatibility

### After Installation
- Test all boot entries
- Configure security settings
- Set up automatic updates

### Maintenance
- Regular configuration backups
- Update modules and themes
- Monitor for compatibility issues
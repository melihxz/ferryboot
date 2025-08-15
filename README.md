# FerryBoot

A cross-platform, advanced yet easy-to-use bootloader software.

## Current Status

FerryBoot is currently in early development. We have a basic BIOS bootloader working and are implementing UEFI support along with a modular architecture.

## Features (Planned)

- **Platform Compatibility**: Works on modern (UEFI/GPT) and legacy (BIOS/MBR) systems
- **Boot Modes**: UEFI Secure Boot support, classic BIOS mode, multi-OS menu, PXE network boot
- **Speed & Minimalism**: Extremely lightweight, ultra-fast boot times
- **Modular Architecture**: Core kernel with loadable plugins
- **Hardware Support**: IDE, SATA, NVMe, USB 3.0/3.1, PS/2, VGA, high-res GUI
- **Security**: Password protection, disk encryption support, signed kernel support
- **User Experience**: Setup wizard, CLI/GUI modes, keyboard/mouse navigation

## Architecture

FerryBoot follows a modular architecture with these components:

1. **Stage 1 Bootloader** - Initial hardware initialization
2. **Stage 2 Bootloader** - Main bootloader with UI and module loading
3. **Module System** - Loadable plugins for filesystems, hardware, etc.
4. **Configuration System** - Persistent settings and user preferences
5. **Security Subsystem** - Encryption, signatures, password protection
6. **Hardware Abstraction Layer** - Consistent interface across platforms

## Building

```bash
# For BIOS systems
make bios

# For UEFI systems
make uefi

# For all platforms
make all
```

## Roadmap

### Phase 1: Foundation (Complete)
- [x] Stage 1 bootloader (assembly)
- [x] Stage 2 bootloader (C)
- [x] Basic HAL implementation
- [x] Configuration system
- [x] Makefile-based build system

### Phase 2: UEFI Support (In Progress)
- [ ] UEFI application framework
- [ ] Secure Boot implementation
- [ ] Graphical interface (VGA/VESA)
- [ ] Mouse input support

### Phase 3: Module System (Planned)
- [ ] Module loading framework
- [ ] Filesystem modules (5+ formats)
- [ ] Hardware modules (storage, input, network)
- [ ] Compression modules (3+ formats)

See our detailed [Development Roadmap](docs/development.md) for complete plans.

## Installation

Detailed installation instructions coming soon.

## License

GPLv3
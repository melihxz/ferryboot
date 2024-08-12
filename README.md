# Ferryboot

Ferryboot is a customizable bootloader development project. This project is designed to perform basic bootloader functions, presenting the user with a list of available kernel options and allowing them to select and load one. The focus of the project is on loading kernels directly into memory without relying on standard boot devices.

## Features

- **User Interface**: Provides a simple menu interface to display boot options to the user.
- **Kernel Selection and Loading**: Allows the user to select a kernel and load it into memory.
- **Configuration**: Ability to configure some settings of the bootloader.

## Requirements

- `gcc` (GNU Compiler Collection)
- `nasm` (Netwide Assembler)
- An `x86` compatible platform or emulator

## Installation

1. **Install Dependencies**:
   ```bash
   sudo apt-get install build-essential nasm
   ```

2. **Clone the Project**:
   ```bash
   git clone <repository-url>
   cd Ferryboot
   ```

3. **Build the Project**:
   ```bash
   make
   ```

## Usage

- After building the `bootloader` binary, you can test it using an emulator (e.g., QEMU) or on actual hardware.

  Example:
  ```bash
  qemu-system-x86_64 -drive format=raw,file=bootloader
  ```

## Known Issues

- **Linker Errors**: Dependencies on standard library functions such as `puts`, `fopen`, `fread`, etc., are problematic in a bootloader context. These functions are typically not available at the bootloader stage. Consider implementing your own versions.
- **Stack Protection**: Missing symbols like `__stack_chk_fail_local` can cause linking issues. You may need to use the `-fno-stack-protector` flag to address these problems.
- **Entry Point**: When `_start` symbol is not found, the default entry point is used. You may need to provide an appropriate `_start` symbol.
- **File Operations**: Functions like `fopen`, `fclose`, `fread` are generally unsupported in bootloader contexts. Alternative methods should be used.

## Contributors

- **Melih Deniz** - Project Founder and Lead Developer

## License

This project is licensed under the [MIT License](LICENSE).

---

**Note:** This README provides an overview and lists known issues. For more details and ongoing updates, please refer to the project documentation.

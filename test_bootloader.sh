#!/bin/bash
# Test script for FerryBoot

echo "Building FerryBoot..."
make clean
make bios

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo "Build successful!"
echo "Binary size: $(ls -la bin/ferryboot_bios.bin | awk '{print $5}') bytes"

echo "Testing in QEMU (5 second timeout)..."
timeout 5s qemu-system-x86_64 -drive format=raw,file=bin/ferryboot_bios.bin -m 512 -serial stdio

echo ""
echo "Test completed!"
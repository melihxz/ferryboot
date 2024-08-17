BITS 16
ORG 0x7C00

; Disable interrupts, clear registers, and set up segment registers
start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    ; Print the boot message
    mov si, boot_message
    call print_string

    ; Load the kernel from disk
    call load_kernel

    ; Jump to the kernel entry point
    jmp 0x0000:0x0600

; Boot message
boot_message db "Booting... Please wait.", 0

; Error message for disk read failure
disk_err_msg db "Disk read error!", 0

; Function to print a null-terminated string
print_string:
    mov ah, 0x0E
.print_char:
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .print_char
.done:
    ret

; Load the kernel from the disk
load_kernel:
    mov bx, 0x1000      ; Load kernel to 0x1000 segment
    mov dh, 0x01        ; Number of sectors to read
    mov dl, 0x80        ; First hard drive
    mov ch, 0x00        ; Cylinder number
    mov cl, 0x02        ; Sector number
    mov ah, 0x02        ; BIOS read sectors function
    int 0x13
    jc disk_error       ; Jump if carry flag is set
    ret

disk_error:
    mov si, disk_err_msg
    call print_string
    hlt

; Function to get the memory map
get_memory_map:
    mov eax, 0xE820
    mov ecx, 24         ; Size of the buffer
    int 0x15            ; BIOS memory map function
    jc no_memory_map    ; Jump if carry flag is set
    ret

no_memory_map:
    mov si, memory_map_err_msg
    call print_string
    hlt

; Error message for memory map failure
memory_map_err_msg db "Memory map error!", 0

; Initialize GDT (Global Descriptor Table)
gdt_start:
    dq 0x0000000000000000 ; Null segment
    dq 0x00CF9A000000FFFF ; Code segment
    dq 0x00CF92000000FFFF ; Data segment

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

gdt_end:

; Set up GDT
setup_gdt:
    lgdt [gdt_descriptor]
    ret

; Set up protected mode
setup_protected_mode:
    cli
    mov eax, cr0
    or eax, 0x1          ; Set the PE bit in CR0
    mov cr0, eax
    jmp 0x08:protected_mode_start

; Start of protected mode code
protected_mode_start:
    mov ax, 0x10         ; Data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9000      ; Stack pointer
    call enable_a20      ; Enable A20 line
    ret

; Enable A20 line (allow access to memory above 1MB)
enable_a20:
    in al, 0x64
    or al, 0x02
    out 0x64, al
    ret

; Set up second stage loader
second_stage_loader:
    call setup_gdt
    call setup_protected_mode
    jmp 0x08:second_stage_main

; Main function of the second stage loader
second_stage_main:
    mov si, second_stage_message
    call print_string
    call load_kernel
    jmp 0x0000:0x0600

; Message indicating second stage loaded
second_stage_message db "Second stage loaded.", 0

; Function to draw a pixel on the screen
draw_pixel:
    mov ah, 0x0C
    mov al, 0x0F         ; Color value
    mov cx, 160          ; X coordinate
    mov dx, 100          ; Y coordinate
    int 0x10
    ret

; Set up graphics mode
graphics_mode_start:
    mov ax, 0x0013       ; 320x200 256-color graphics mode
    int 0x10
    call draw_pixel      ; Draw a pixel on the screen
    ret

; Extended memory map function
extended_memory_map:
    mov eax, 0xE820
    mov ecx, 24          ; Buffer size
    int 0x15             ; BIOS memory map function
    jc no_memory_map_extended ; Jump if carry flag is set
    ret

no_memory_map_extended:
    mov si, memory_map_err_msg
    call print_string
    hlt

; Simple file system check
simple_fs_check:
    mov si, fs_check_msg
    call print_string
    ; Here, you could add more complex file system handling
    ret

fs_check_msg db "Checking filesystem...", 0

; GDT segment loading
gdt_segment_loader:
    lgdt [gdt_descriptor]
    mov ax, 0x10         ; Load the data segment
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax
    ret

; Enter protected mode (another approach)
enter_protected_mode_code:
    cli
    mov eax, cr0
    or eax, 0x1          ; Set PE bit
    mov cr0, eax
    jmp 0x08:protected_mode_start_code

protected_mode_start_code:
    mov ax, 0x10         ; Set up segment registers
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9000
    ret

; Basic memory management
memory_management:
    mov eax, 0xE820
    mov ecx, 24
    int 0x15
    jc memory_error
    ret

memory_error:
    mov si, memory_map_err_msg
    call print_string
    hlt

; Set up segment registers
setup_segments:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax
    ret

; Extended setup for segment registers
extended_setup_segments:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax
    ret

; Debug messages
debug_setup_msg db "Debug: Setup complete.", 0
debug_error_msg db "Debug: Setup failed!", 0

; Kernel startup sequence
kernel_startup:
    call load_kernel
    jmp 0x08:kernel_entry

kernel_entry:
    ; Kernel main entry point
    ret

; Message for kernel loading
kernel_load_message db "Loading kernel...", 0

; Memory management success and error messages
memory_setup_success db "Memory setup complete.", 0
memory_setup_error db "Memory setup failed!", 0

times 510-($-$$) db 0
dw 0xAA55

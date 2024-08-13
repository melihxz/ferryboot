[BITS 16]
[ORG 0x7C00]

start:
    ; Set up segment registers
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Load Stage 2 bootloader from disk
    mov ah, 0x02         ; BIOS interrupt 13h - Read Sectors
    mov al, 1            ; Number of sectors to read
    mov ch, 0            ; Cylinder
    mov cl, 2            ; Sector number (2)
    mov dh, 0            ; Head
    mov dl, 0x80         ; Drive (first hard disk)
    int 0x13             ; Call BIOS interrupt

    ; Check if the read operation was successful
    jc disk_read_error   ; If carry flag is set, jump to disk read error handling

    ; Jump to Stage 2 bootloader
    jmp 0x1000:0x0000

disk_read_error:
    ; Display a disk read error message
    mov si, disk_error_msg
    call print_string
    jmp halt_system

print_string:
    ; Print a null-terminated string pointed to by SI
    mov ah, 0x0E
.print_char:
    lodsb                 ; Load next byte from string into AL
    cmp al, 0
    je .done              ; If end of string (null), we're done
    int 0x10              ; Otherwise, print character
    jmp .print_char
.done:
    ret

halt_system:
    hlt                   ; Halt the CPU

disk_error_msg db 'Disk Read Error!', 0

times 510-($-$$) db 0  ; Fill the rest of the boot sector with zeros
dw 0xAA55             ; Boot signature

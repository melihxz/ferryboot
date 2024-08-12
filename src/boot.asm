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
    jc read_error        ; If carry flag is set, jump to error handling

    ; Jump to Stage 2 bootloader
    jmp 0x1000:0x0000

read_error:
    ; Display error message (very basic)
    mov ah, 0x0E         ; BIOS interrupt 10h - Teletype output
    mov al, 'E'          ; Character to print
    int 0x10             ; Call BIOS interrupt
    hlt                  ; Halt the CPU

times 510-($-$$) db 0  ; Fill the rest of the boot sector with zeros
dw 0xAA55             ; Boot signature

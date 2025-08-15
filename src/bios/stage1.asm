; FerryBoot Stage 1 - BIOS Bootloader
; Loads stage 2 bootloader from disk

BITS 16
ORG 0x7C00

start:
    ; Initialize segments
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Reset disk system
    mov ah, 0
    mov dl, 0x80
    int 0x13
    jc error

    ; Load stage 2 from disk
    ; CHS: cylinder=0, head=0, sector=2
    mov ah, 0x02    ; Read sectors function
    mov al, 10      ; Number of sectors to read (5KB)
    mov ch, 0       ; Cylinder
    mov cl, 2       ; Sector (1-indexed)
    mov dh, 0       ; Head
    mov dl, 0x80    ; Drive number (first hard disk)
    mov bx, 0x7E00  ; Load address (after boot sector)
    int 0x13
    jc error

    ; Jump to stage 2
    jmp 0x7E00

error:
    ; Print error message
    mov si, error_msg
    call print_string
    jmp $

print_string:
    lodsb
    or al, al
    jz done
    mov ah, 0x0E
    mov bh, 0
    int 0x10
    jmp print_string
done:
    ret

error_msg db 'Error loading stage 2', 0

; Pad to 510 bytes and add boot signature
times 510-($-$$) db 0
dw 0xAA55
[BITS 16]
[ORG 0x1000]

start:
    mov ah, 0x0E
    mov si, msg
print_char:
    lodsb
    cmp al, 0
    je done
    int 0x10
    jmp print_char
done:
    hlt
msg db 'Assembly Kernel Loaded!', 0

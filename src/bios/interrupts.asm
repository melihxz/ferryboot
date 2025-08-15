; BIOS interrupt functions for FerryBoot
; Assembly implementations for BIOS calls

[BITS 16]

section .text

; Call INT 10h (Video services)
; Parameters:
;   ah = function number
;   al = subfunction/parameter
;   bx = parameter
;   cx = parameter
;   dx = parameter
global int10h_call
int10h_call:
    push bp
    mov bp, sp
    
    ; Save registers
    push ax
    push bx
    push cx
    push dx
    push si
    push di
    
    ; Load parameters
    mov ah, [bp + 4]   ; ah parameter
    mov al, [bp + 6]   ; al parameter
    mov bx, [bp + 8]   ; bx parameter
    mov cx, [bp + 10]  ; cx parameter
    mov dx, [bp + 12]  ; dx parameter
    
    ; Call BIOS interrupt
    int 0x10
    
    ; Restore registers
    pop di
    pop si
    pop dx
    pop cx
    pop bx
    pop ax
    
    pop bp
    ret

; Read sector using INT 13h
; Parameters:
;   drive = drive number
;   cylinder = cylinder number
;   sector = sector number
;   head = head number
;   count = number of sectors
;   buffer = buffer address
;   status = pointer to status byte
global int13h_read
int13h_read:
    push bp
    mov bp, sp
    
    ; Save registers
    push ax
    push bx
    push cx
    push dx
    push si
    push di
    push es
    
    ; Load parameters
    mov dl, [bp + 4]   ; drive number
    mov ch, [bp + 6]   ; cylinder (low 8 bits)
    mov cl, [bp + 8]   ; sector (bits 0-5), cylinder (bits 6-7)
    mov dh, [bp + 10]  ; head number
    mov al, [bp + 12]  ; number of sectors
    mov si, [bp + 14]  ; buffer address (low 16 bits)
    mov di, [bp + 16]  ; status pointer
    
    ; Set up ES:BX for buffer address
    mov bx, si
    xor si, si         ; ES = 0 (buffer in low memory)
    mov es, si
    
    ; Set up CHS parameters
    ; CL contains sector in bits 0-5 and high 2 bits of cylinder in bits 6-7
    and cl, 0x3F       ; Keep only sector bits
    mov ah, [bp + 6]   ; Get cylinder
    shl ah, 6          ; Move high 2 bits to position 6-7
    or cl, ah          ; Combine sector and high cylinder bits
    
    ; Set function number
    mov ah, 0x02       ; Read sectors
    
    ; Call BIOS interrupt
    int 0x13
    
    ; Store status
    mov si, di         ; Get status pointer
    mov [si], ah       ; Store status byte
    
    ; Restore registers
    pop es
    pop di
    pop si
    pop dx
    pop cx
    pop bx
    pop ax
    
    pop bp
    ret

; Check if key is pressed using INT 16h
; Parameters:
;   status = pointer to status byte
global int16h_check_key
int16h_check_key:
    push bp
    mov bp, sp
    
    ; Save registers
    push ax
    push bx
    
    ; Load parameter
    mov bx, [bp + 4]   ; status pointer
    
    ; Check for key
    mov ah, 0x01       ; Check for keystroke
    int 0x16
    
    ; Store status (ZF set if no key)
    mov [bx], ah       ; Store status
    
    ; Restore registers
    pop bx
    pop ax
    
    pop bp
    ret

; Get key using INT 16h
; Parameters:
;   key = pointer to key word
global int16h_get_key
int16h_get_key:
    push bp
    mov bp, sp
    
    ; Save registers
    push ax
    push bx
    
    ; Load parameter
    mov bx, [bp + 4]   ; key pointer
    
    ; Get key
    mov ah, 0x00       ; Get keystroke
    int 0x16
    
    ; Store key
    mov [bx], ax       ; Store key (scan code in AH, ASCII in AL)
    
    ; Restore registers
    pop bx
    pop ax
    
    pop bp
    ret
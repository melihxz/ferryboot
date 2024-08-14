[BITS 16]
[ORG 0x7C00]

start:
    cli                    ; Kesilmeleri devre dışı bırak
    xor ax, ax             ; AX register'ını sıfırla
    mov ds, ax             ; DS segment register'ını sıfırla
    mov es, ax             ; ES segment register'ını sıfırla
    mov ss, ax             ; SS segment register'ını sıfırla
    mov sp, 0x7C00         ; Stack pointer'ı yeniden ayarla

    ; Menü başlığı
    mov si, menu_msg
    call print_string

wait_key:
    ; Kullanıcıdan giriş bekle
    mov ah, 0
    int 0x16               ; Klavye girişi

    ; 1 tuşuna basıldıysa Kernel 1'i yükle
    cmp al, '1'
    je load_kernel1

    ; 2 tuşuna basıldıysa Kernel 2'yi yükle
    cmp al, '2'
    je load_kernel2

    ; Geçersiz tuş basıldıysa tekrar dene
    jmp wait_key

load_kernel1:
    mov bx, 0x1000         ; Kernel 1'in yükleneceği adres
    mov cl, 2              ; Kernel 1'in bulunduğu sektör
    jmp load_kernel

load_kernel2:
    mov bx, 0x1000         ; Kernel 2'nin yükleneceği adres
    mov cl, 3              ; Kernel 2'nin bulunduğu sektör
    jmp load_kernel

load_kernel:
    mov ah, 0x02           ; BIOS'un disk okuma fonksiyonu
    mov al, 1              ; 1 sektör oku
    mov ch, 0              ; Silindir 0
    mov dh, 0              ; Kafa 0
    mov dl, 0x80           ; Ana disk (ilk disk)
    int 0x13               ; Disk okuma kesmesi

    ; Yüklenen kernel'i çalıştır
    jmp 0x0000:0x1000      ; Kernel başlangıç adresine git

print_string:
    mov ah, 0x0E           ; BIOS teletype fonksiyonu
.next_char:
    lodsb                  ; SI'den sonraki karakteri yükle
    cmp al, 0              ; Karakter sonlandırıcı mı?
    je .done               ; Eğer öyleyse bitir
    int 0x10               ; Ekrana yaz
    jmp .next_char
.done:
    ret

menu_msg db '1: Kernel 1', 0x0D, 0x0A, '2: Kernel 2', 0x0D, 0x0A, 'Choose: ', 0
times 510-($-$$) db 0       ; Bootloader'ı 512 byte'a tamamla
dw 0xAA55                  ; Boot signature

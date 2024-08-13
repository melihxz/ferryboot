[BITS 16]          ; 16-bit kod
[ORG 0x7C00]       ; Boot sektörünün başlangıç adresi

; Bootloader kodu başlıyor

START:
    ; Ekranı temizle
    mov ax, 0xB800
    mov es, ax
    mov di, 0x0000
    mov al, 0x00
    mov ah, 0x07
    mov cx, 2000
    rep stosw

    ; Kernel'i bellek adresine yükle
    mov si, 0x1000    ; Kernel'i bellek adresinden yükle
    mov bx, 0x8000    ; Kernel verilerini okumak için bir bellek adresi
    mov cx, 0x10000   ; Okunacak veri miktarı (64 KB)

    ; Disk okuma işlemi
    mov ah, 0x02      ; Disk okuma fonksiyonu
    mov al, 0x01      ; Bir sektör oku
    mov ch, 0x00      ; Silindirin yüksek kısmı
    mov cl, 0x02      ; Sektör numarası (başlangıçta 2. sektör)
    mov dh, 0x00      ; Yüzey numarası
    mov dl, 0x80      ; İlk sabit disk
    int 0x13          ; BIOS Disk hizmet çağrısı

    ; Kernel'i yükledikten sonra çalıştır
    jmp 0x0000:0x1000 ; Kernel'in başlangıç adresine zıpla

; Boş bir döngü
HALT:
    jmp HALT

; Disk hatası durumunda burada dur
DISK_ERROR:
    mov ah, 0x0E
    mov al, 'E'
    int 0x10
    jmp HALT

; Bootloader sonu
TIMES 510 - ($ - $$) DB 0  ; 510 byte'a kadar boş yer
DW 0xAA55                 ; Boot sektör imzası

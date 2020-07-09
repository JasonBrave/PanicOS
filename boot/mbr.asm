; MBR partition table
times 446 db 0
; partition entry 1
db 0
times 3 db 0
db 0ch
times 3 db 0
dd 2048
dd 129024
; partition entry 2 to 4
times 48 db 0
; boot signature
db 0x55,0xaa

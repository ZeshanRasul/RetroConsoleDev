.psx
.create "TEXTURE.bin", 0x80010000

.org 0x80010000

;;; IO Port
IO_BASE_ADDR equ 0x1F80

;;; GPU Registers
GP0 equ 0x1810
GP1 equ 0x1814

IMG_WIDTH           equ 640
IMG_HEIGHT          equ 480
IMG_SIZE_IN_BYTES   equ 921600

Main:
    lui $a0, IO_BASE_ADDR 

    ;;; Display Setup
    
    ; Reset GPU
    li $t1, 0x00000000  ;Command to reset GPU
    sw $t1, GP1($a0)    ;Write packet containing command to GP1

    ; Enable Display
    li $t1, 0x03000000
    sw $t1, GP1($a0)

    ; Display Mode
    li $t1, 0x0800003F
    sw $t1, GP1($a0)

    ; Display Area Start (X=0, Y=0)
    li $t1, 0x05000000
    sw $t1, GP1($a0)

    ; Set Horizonal Range
    li $t1, 0x06C60260
    sw $t1, GP1($a0)

    ; Set Vertical Range
    li $t1, 0x07096018
    sw $t1, GP1($a0)

    ;;; Setup VRAM Access
    
    ; Draw Mode Settings
    li $t1, 0xE1000400
    sw $t1, GP0($a0)

    ; Drawing Area Top Left
    li $t1, 0xE3000000
    sw $t1, GP0($a0)

    ; Drawing Area Bottom Right
    li $t1, 0xE403BD3F
    sw $t1, GP0($a0)

    ; Drawing Offset
    li $t1, 0xE5000000
    sw $t1, GP0($a0)

    ;;; Clear Screen
    li $t1, 0x0280045A
    sw $t1, GP0($a0)
    li $t1, 0x00000000
    sw $t1, GP0($a0)
    li $t1, 0x00EF013F
    sw $t1, GP0($a0)

    ;;; Copy texture data from CPU to GPU
    li $t1, 0xA0000000
    sw $t1, GP0($a0)

    li $t1, 0x00000000
    sw $t1, GP0($a0)

    li $t1, 0x01E003C0
    sw $t1, GP0($a0)

    li $t0, IMG_SIZE_IN_BYTES
    srl $t0, 2

    la $t2, Image

TextureTransfer:
    lw $t1, 0($t2)
    nop
    sw $t1, GP0($a0)
    addiu $t2, $t2, 4
    addiu $t0, $t0, -1
    bnez $t0, TextureTransfer
    nop

LoopForever:
    j LoopForever
    nop

Image:
    .incbin "logo.bin"

.close
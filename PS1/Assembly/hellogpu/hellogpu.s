.psx
.create "HELLOGPU.bin", 0x80010000

.org 0x80010000

;;; IO Port
IO_BASE_ADDR equ 0x1F80

;;; GPU Registers
GP0 equ 0x1810
GP1 equ 0x1814

Main:
    lui $t0, IO_BASE_ADDR 

    la $sp, 0x00103CF0

    ;;; Display Setup
    
    ; Reset GPU
    li $t1, 0x00000000  ;Command to reset GPU
    sw $t1, GP1($t0)    ;Write packet containing command to GP1

    ; Enable Display
    li $t1, 0x03000000
    sw $t1, GP1($t0)

    ; Display Mode
    li $t1, 0x08000009
    sw $t1, GP1($t0)

    ; Display Area Start (X=0, Y=0)
    li $t1, 0x05000000
    sw $t1, GP1($t0)

    ; Set Horizonal Range
    li $t1, 0x06C60260
    sw $t1, GP1($t0)

    ; Set Vertical Range
    li $t1, 0x07042818
    sw $t1, GP1($t0)

    ;;; Setup VRAM Access
    
    ; Draw Mode Settings
    li $t1, 0xE1000400
    sw $t1, GP0($t0)

    ; Drawing Area Top Left
    li $t1, 0xE3000000
    sw $t1, GP0($t0)

    ; Drawing Area Bottom Right
    li $t1, 0xE403BD3F
    sw $t1, GP0($t0)

    ; Drawing Offset
    li $t1, 0xE5000000
    sw $t1, GP0($t0)

    ;;; Clear Screen
    li $t1, 0x0280045A
    sw $t1, GP0($t0)
    li $t1, 0x00000000
    sw $t1, GP0($t0)
    li $t1, 0x00EF013F
    sw $t1, GP0($t0)

    ;;; Draw Flat Shaded Quad
    li $t1, 0x2803DE33
    sw $t1, GP0($t0)
    li $t1, 0x00A00050
    sw $t1, GP0($t0)
    li $t1, 0x00A000F0
    sw $t1, GP0($t0)
    li $t1, 0x00F00050
    sw $t1, GP0($t0)
    li $t1, 0x00F000F0
    sw $t1, GP0($t0)

    ;;; Draw Flat Shaded Triangle (params stored in stack)

    lui $a0, IO_BASE_ADDR

    addiu $sp, -(4 * 7)

    li $t0, 0xFF4472
    sw $t0, 0($sp)

    li $t0, 200
    sw $t0, 4($sp)

    li $t0, 40
    sw $t0, 8($sp)

    li $t0, 288
    sw $t0, 12($sp)

    li $t0, 56
    sw $t0, 16($sp)

    li $t0, 224
    sw $t0, 20($sp)

    li $t0, 200
    sw $t0, 24($sp)

    jal DrawFlatShadedTriangle
    nop

LoopForever:
    j LoopForever
    nop

;;;;;;;;;;;;;;;;;;;;;;;;
;; Subroutine to draw flat shaded triangle
;;;;;;;;;;;;;;;;;;;;;;;;

DrawFlatShadedTriangle:

    lui $t0, 0x2000
    lw $t1, 0($sp)
    nop
    
    or $t8, $t0, $t1
    sw $t8, GP0($a0)

    lw $t1, 4($sp)
    lw $t2, 8($sp)
    nop

    sll $t2, $t2, 16
    andi $t1, $t1, 0xFFFF
    or $t8, $t1, $t2
    sw $t8, GP0($a0)

    lw $t1, 12($sp)
    lw $t2, 16($sp)
    nop

    sll $t2, $t2, 16
    andi $t1, $t1, 0xFFFF
    or $t8, $t1, $t2
    sw $t8, GP0($a0)

    lw $t1, 20($sp)
    lw $t2, 24($sp)
    nop

    sll $t2, $t2, 16
    andi $t1, $t1, 0xFFFF
    or $t8, $t1, $t2
    sw $t8, GP0($a0)

    addiu $sp, $sp, (4 * 7)

    jr $ra
    nop

.close
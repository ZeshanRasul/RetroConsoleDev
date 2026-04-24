.psx
.create "exercise2.bin", 0x80010000

.org 0x80010000

Main:
    li $t0, 0x01
    li $t1, 0x00
Loop:
    add $t1, $t1, $t0
    addi $t0, $t0, 0x01
    bne $t0, 0x0b, Loop
End:

.close
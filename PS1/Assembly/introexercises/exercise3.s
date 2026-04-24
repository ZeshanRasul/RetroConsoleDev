.psx
.create "exercise3.bin", 0x80010000

.org 0x80010000

Main:
    li $t0, 0x1b
    li $t1, 0x03
    li $t2, 0x00
Loop:
    subu $t0, $t0, $t1
    addi $t2, 0x01
    bge $t0, $t1, Loop
End:

.close
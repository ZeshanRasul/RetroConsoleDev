.psx
.create "factorialsubroutine.bin", 0x80010000

.org 0x80010000

Main:
    li $a0, 0x5 ; num
    jal Factorial
    nop

LoopForever:
    j LoopForever
    nop
End:

;;;; Subroutine for factorial calculation
;;;; Takes a single argument for num stored in $a0
Factorial:
    li $t1, 0x1 ; i
    li $t2, 0x1 ; j
    li $t3, 0x1 ; temp
    li $t4, 0x1 ; sum

OuterLoop:
    bge $t1, $a0, EndOuterLoop
    nop
    li $t4, 0x00
    li $t2, 0x00

InnerLoop:
    bge $t2, $t1, EndInnerLoop
    addu $t4, $t4, $t3
    addiu $t2, $t2, 0x1
    b InnerLoop
    nop
EndInnerLoop:
    li $t3, 0x0
    addu $t3, $t3, $t4
    addiu $t1, $t1, 0x1
    b OuterLoop
    nop
EndOuterLoop:
    move $v0, $t4 
    jr $ra

.close
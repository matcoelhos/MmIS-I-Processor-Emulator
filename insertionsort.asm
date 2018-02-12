# Find lowest element from vector MIS
# Arquitetura de Computadores
#
# Mateus Coelho Silva
#
#-----------------------------
#-----------------------------

.data

V = 6 5 7 8 2 1 4 3 9

.program

lw $s0, V
mainloop:
    add $t1, $s0, $zero
    add $t7, $t1, $zero
    add $t2, $t1, $zero
    add $t2, $t2, 1
    lw $t8, 0($s0)
    lw $t3, 0($t1)

    findlowestloop:
        add $t1, $t1, 1
        lw $t5, 0($t1)
        slt $t6, $t3, $t5
        bne $t6, $zero, conditiontrue

        add $t3, $t5, $zero
        add $t7, $t1, $zero
        
        conditiontrue:
        add $t2, $t2, 1
        slt $t4, $t2, 9
        bne $t4, $zero, findlowestloop

    sw $t3, 0($s0)
    sw $t8, 0($t7)
    add $s0, $s0, 1
    slt $t9, $s0, 8
    bne $t9, $zero, mainloop:
#end



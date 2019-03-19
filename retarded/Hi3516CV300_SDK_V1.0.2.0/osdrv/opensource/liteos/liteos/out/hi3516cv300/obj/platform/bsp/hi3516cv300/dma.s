# 1 "dma.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "dma.S"
# 34 "dma.S"
.global arm926_dma_inv_range
.global arm926_dma_clean_range
.global los_setjmp
.global los_longjmp

.equ CACHE_DLINESIZE, 32
arm926_dma_inv_range:
    tst r0, #CACHE_DLINESIZE - 1
    mcrne p15, 0, r0, c7, c10, 1
    tst r1, #CACHE_DLINESIZE -1
    mcrne p15, 0, r1, c7, c10, 1

    bic r0, r0, #CACHE_DLINESIZE - 1
1:
    mcr p15, 0, r0, c7, c6, 1
    add r0, r0, #CACHE_DLINESIZE
    cmp r0, r1
    blo 1b
    mcr p15, 0, r0, c7, c10, 4

    mov pc, lr

.type arm926_dma_inv_range, %function;

arm926_dma_clean_range:
    bic r0, r0, #CACHE_DLINESIZE - 1
1:
    mcr p15, 0, r0, c7, c10, 1
    add r0, r0, #CACHE_DLINESIZE
    cmp r0, r1
    blo 1b

    mcr p15, 0, r0, c7, c10, 4
    mov pc, lr
.type arm926_dma_clean_range,%function;

los_longjmp:

        ldmfd r0,{r4-r14}
        cmp r1,#0
        moveq r1,#1
        mov r0,r1
        mov pc,lr

.type los_longjmp,%function;
.size los_longjmp, .-los_longjmp

los_setjmp:
        stmea r0,{r4-r14}
        mov r0,#0

        mov pc,lr

.type los_setjmp,%function;
.size los_setjmp, .-los_setjmp

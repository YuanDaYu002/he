# 1 "reset_vector.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "reset_vector.S"
# 34 "reset_vector.S"
.equ CPSR_IRQ_DISABLE, 0x80
.equ CPSR_FIQ_DISABLE, 0x40
.equ CPSR_THUMB_ENABLE, 0x20
.equ CPSR_USER_MODE, 0x10
.equ CPSR_FIQ_MODE, 0x11
.equ CPSR_IRQ_MODE, 0x12
.equ CPSR_SVC_MODE, 0x13
.equ CPSR_ABT_MODE, 0x17
.equ CPSR_UNDEF_MODE, 0x1B

.global __exc_stack_top
.global __startup_stack_top
.global __startup_stack
.global __irq_stack_top
.global __fiq_stack_top
.global __svc_stack_top
.global __abt_stack_top
.global __undef_stack_top
.global __undef_stack
.extern __ram_data_start
.extern __ram_data_end
.extern __rom_data_start
.extern __sram_data_start
.extern __sram_data_end
.extern __srom_data_start
.extern __bss_start
.extern __bss_end
.extern hal_clock_initialize_start
.extern bss_init
.extern copy_vector
.extern board_config
.extern _osExceptFiqHdl
.extern _osExceptAddrAbortHdl
.extern _osExceptDataAbortHdl
.extern _osExceptPrefetchAbortHdl
.extern _osExceptSwiHdl
.extern _osExceptUndefInstrHdl
.extern _osExceptIrqHdl
.extern __stack_chk_guard_setup
.code 32
.section ".vectors", "ax"

.global __exception_handlers
__exception_handlers:


        ldr pc,_reset_vector
        ldr pc,osExceptUndefInstrHdl
        ldr pc,osExceptSwiHdl
        ldr pc,osExceptPrefetchAbortHdl
        ldr pc,osExceptDataAbortHdl
        ldr pc,osExceptAddrAbortHdl
        ldr pc,osExceptIrqHdl
        ldr pc,osExceptFiqHdl

_reset_vector: .word reset_vector
osExceptUndefInstrHdl: .word _osExceptUndefInstrHdl
osExceptSwiHdl: .word _osExceptSwiHdl
osExceptPrefetchAbortHdl: .word _osExceptPrefetchAbortHdl
osExceptDataAbortHdl: .word _osExceptDataAbortHdl
osExceptAddrAbortHdl: .word _osExceptAddrAbortHdl
osExceptIrqHdl: .word _osExceptIrqHdl
osExceptFiqHdl: .word _osExceptFiqHdl

.globl _armboot_start
_armboot_start: .word __exception_handlers
.globl _TEXT_BASE
_TEXT_BASE: .word 0x00000000

.text

.global reset_vector
.type reset_vector,function

reset_vector:
warm_reset:

        ldr sp, =__startup_stack_top

        mov r0,#(CPSR_IRQ_DISABLE |CPSR_FIQ_DISABLE|CPSR_IRQ_MODE)
        msr cpsr,r0
        ldr sp, =__irq_stack_top

        mov r0,#(CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE|CPSR_UNDEF_MODE)
        msr cpsr,r0
        ldr sp, =__undef_stack_top

        mov r0,#(CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE|CPSR_ABT_MODE)
        msr cpsr,r0
        ldr sp, =__abt_stack_top

        mov r0,#(CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE|CPSR_FIQ_MODE)
        msr cpsr,r0
        ldr sp, =__fiq_stack_top


        mov r0,#(CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE|CPSR_SVC_MODE)
        msr cpsr,r0


        msr spsr,r0


        ldr sp, =__svc_stack_top
        bl board_config
        bl hal_clock_initialize_start


        mrc p15,0,r0,c1,c0,0
        bic r0,r0,#0x1000
        bic r0,r0,#0x000f





        mcr p15,0,r0,c1,c0,0
        nop
        nop
        mov r0,#0

        mcr p15, 0, r0, c7, c7, 0



        bl hal_mmu_init

        mov r0, #0
        mcrne p15, 0, r0, c8, c7, 0

        mrc p15, 0, r0, c1, c0, 0
        orr r0, r0, #1<<12
        orr r0, r0, #1<<8
        orr r0, r0, #1<<2
        orr r0, r0, #1<<1
        orr r0, r0, #1

        mcr p15, 0, r0, c1, c0, 0


        ldr r0, =g_swResumeFromImg
        ldr r1, [r0]
        cmp r1, #1
        beq osSystemWakeup



        ldr r0, =__bss_start
        ldr r1, =__bss_end
        blx los_bss_init
# 197 "reset_vector.S"
        bl __stack_chk_guard_setup


        bl main

_start_hang:
        b _start_hang
        .code 32

        .global reset_platform
        .type reset_platform, function

reset_platform:
        mov r0,#0
        mov pc,r0


init_done:
        .long 0xDEADB00B


        .code 32
# 231 "reset_vector.S"
        .data

init_flag:
        .balign 4
        .long 0




    .section ".int_stack", "wa", %nobits
    .align 3

__undef_stack:
    .space 32
__undef_stack_top:

__abt_stack:
    .space 32
__abt_stack_top:

__irq_stack:
    .space 64
__irq_stack_top:

__fiq_stack:
    .space 64
__fiq_stack_top:

__svc_stack:
    .space 4096
__svc_stack_top:

__exc_stack:
    .space 512
__exc_stack_top:

    .section ".startup_stack", "wa", %nobits
    .align 3

__startup_stack:
    .space 512
__startup_stack_top:

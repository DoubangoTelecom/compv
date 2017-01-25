#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__arm__) && !defined(__aarch64__)
.include "compv_common_arm.S"

#########################################################################
.equ COMPV_GAS_REG_SZ_BYTES, 4
.equ prolog_bytes, (10 * COMPV_GAS_REG_SZ_BYTES)   @ push {r4-r12, lr}

# Stack Base Pointer register
bp .req r12


#########################################################################
# Macro to exit from the function (x86 'ret' equiv.)
.macro COMPV_GAS_FUNCTION_RETURN
    bx lr
.endm

#########################################################################
# Function Prolog.
.macro COMPV_GAS_FUNCTION_PROLOG
    push {r4-r12, lr}        @ (10 * COMPV_GAS_REG_SZ_BYTES) -> prolog_bytes
.endm

#########################################################################
# Function Epilog. More info:
.macro COMPV_GAS_FUNCTION_EPILOG
    pop {r4-r12, lr}
.endm

#########################################################################
.macro COMPV_GAS_SAVE_NEON_REGS
    vstmdb sp!, {d8-d15}    @ save VFP/Advanced SIMD registers D8,  (aka S16-S31, Q4-Q7)
.endm

#########################################################################
.macro COMPV_GAS_RESTORE_NEON_REGS
    vldmia sp!, {d8-d15}    @ restore VFP/Advanced SIMD registers
.endm

#########################################################################
# COMPV_GAS_ALIGN_STACK <alignment> <register>
# This macro aligns the stack to the given alignment (in bytes). The stack
# is left such that the previous value of the stack pointer is the first
# argument on the stack (ie, the inverse of this macro is 'pop sp.')
# This macro uses one temporary register, which is not preserved, and thus
# must be specified as an argument.
.macro COMPV_GAS_ALIGN_STACK alignment register
    mov         \register, sp
    and         sp, #-\alignment
    sub         sp, #(\alignment - COMPV_GAS_REG_SZ_BYTES)
    push        { \register }
.endm

#########################################################################
# COMPV_GAS_UNALIGN_STACK
# Undo COMPV_GAS_ALIGN_STACK. This function will a temporary register,
# which is not preserved, and thus must be specified as an argument.
# The temporary could be different than the one provide with COMPV_GAS_ALIGN_STACK.
.macro COMPV_GAS_UNALIGN_STACK register
    pop { \register } @ pop { sp } is illegal instruction
    mov sp, \register
.endm

#########################################################################
.macro COMPV_GAS_SHADOW_ARGS_TO_STACK nn
    sub sp, sp, #(COMPV_GAS_REG_SZ_BYTES * \nn)
    mov bp, sp
    .if \nn >= 1
        str r0, [sp, #(COMPV_GAS_REG_SZ_BYTES * 0)]
    .endif
    .if \nn >= 2
        str r1, [sp, #(COMPV_GAS_REG_SZ_BYTES * 1)]
    .endif
    .if \nn >= 3
        str r2, [sp, #(COMPV_GAS_REG_SZ_BYTES * 2)]
    .endif
    .if \nn >= 4
        str r3, [sp, #(COMPV_GAS_REG_SZ_BYTES * 3)]
    .endif
    add r8, sp, #(prolog_bytes + (COMPV_GAS_REG_SZ_BYTES * \nn))
    add r9, sp, #(COMPV_GAS_REG_SZ_BYTES * 4)
    .rept \nn - 4
        ldr r10, [r8], #COMPV_GAS_REG_SZ_BYTES
        str r10, [r9], #COMPV_GAS_REG_SZ_BYTES
    .endr
    #.set counter, 0
    #.rept \nn - 4
    #	add r4, sp, #(prolog_bytes + (COMPV_GAS_REG_SZ_BYTES * \nn) + (COMPV_GAS_REG_SZ_BYTES * counter))
    #	ldr r4, [r4]
    #	str r4, [sp, #(COMPV_GAS_REG_SZ_BYTES * (counter + 4))]
    #	.set counter, counter+1
    #.endr
.endm

#########################################################################
# requires COMPV_GAS_SHADOW_ARGS_TO_STACK
.macro COMPV_GAS_UNSHADOW_ARGS nn
    add sp, sp, #(\nn * COMPV_GAS_REG_SZ_BYTES)
.endm

#########################################################################
# requires COMPV_GAS_SHADOW_ARGS_TO_STACK
.macro ldr_arg nn rx
    ldr \rx, [bp, #(\nn * COMPV_GAS_REG_SZ_BYTES)]
.endm

#########################################################################
# requires COMPV_GAS_SHADOW_ARGS_TO_STACK
# Example: ldmia_args r0-r7
.macro ldm_args registers
    ldm bp, {\registers}
.endm

#endif /* defined(__arm__) && !defined(__aarch64__) */

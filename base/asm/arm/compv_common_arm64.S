#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__aarch64__)
.include "compv_common_arm.S"

#########################################################################
.equ COMPV_GAS_REG_SZ_BYTES, 8
.equ prolog_bytes, (12 * COMPV_GAS_REG_SZ_BYTES)

# Stack Base Pointer register (defined when the function is declared)
bp .req x30

#########################################################################
# Macro to exit from the function (x86 'ret' equiv.)
.macro COMPV_GAS_FUNCTION_RETURN
    ret
.endm

#########################################################################
# Function Prolog.
.macro COMPV_GAS_FUNCTION_PROLOG
    // x19 to x29 are callee-saved (12 * COMPV_GAS_REG_SZ_BYTES)
    stp x19, x20, [sp, #-(2*COMPV_GAS_REG_SZ_BYTES)]!
    stp x21, x22, [sp, #-(2*COMPV_GAS_REG_SZ_BYTES)]!
    stp x23, x24, [sp, #-(2*COMPV_GAS_REG_SZ_BYTES)]!
    stp x25, x26, [sp, #-(2*COMPV_GAS_REG_SZ_BYTES)]!
    stp x27, x28, [sp, #-(2*COMPV_GAS_REG_SZ_BYTES)]!
    stp x29, x30, [sp, #-(2*COMPV_GAS_REG_SZ_BYTES)]!
    // bp is x30 and must be saved first
    mov bp, sp
.endm

#########################################################################
# Function Epilog. More info:
.macro COMPV_GAS_FUNCTION_EPILOG
    mov sp, bp
    ldp x29, x30, [sp], #(2*COMPV_GAS_REG_SZ_BYTES)
    ldp x27, x28, [sp], #(2*COMPV_GAS_REG_SZ_BYTES)
    ldp x25, x26, [sp], #(2*COMPV_GAS_REG_SZ_BYTES)
    ldp x23, x24, [sp], #(2*COMPV_GAS_REG_SZ_BYTES)
    ldp x21, x22, [sp], #(2*COMPV_GAS_REG_SZ_BYTES)
    ldp x19, x20, [sp], #(2*COMPV_GAS_REG_SZ_BYTES)
.endm

#########################################################################
.macro COMPV_GAS_SAVE_NEON_REGS
    stp d8, d9, [sp, #-16]!
    stp d10, d11, [sp ,#-16]!
    stp d12, d13, [sp, #-16]!
    stp d14 ,d15, [sp, #-16]!
.endm

#########################################################################
.macro COMPV_GAS_RESTORE_NEON_REGS
    ldp d14, d15, [sp], #16
    ldp d12, d13, [sp], #16
    ldp d10, d11, [sp], #16
    ldp d8, d9, [sp], #16
.endm

#########################################################################
# COMPV_GAS_ALIGN_STACK <alignment> <register>
# This macro aligns the stack to the given alignment (in bytes). The stack
# is left such that the previous value of the stack pointer is the first
# argument on the stack (ie, the inverse of this macro is 'pop sp.')
# This macro uses one temporary register, which is not preserved, and thus
# must be specified as an argument.
.macro COMPV_GAS_ALIGN_STACK alignment register
    mov \register, sp
    and sp, \register, #-\alignment
    sub sp, sp, #(\alignment - (2*COMPV_GAS_REG_SZ_BYTES))
    stp \register, \register, [sp, #-(2*COMPV_GAS_REG_SZ_BYTES)]!
.endm

#########################################################################
# COMPV_GAS_UNALIGN_STACK
# Undo COMPV_GAS_ALIGN_STACK. This function will a temporary register,
# which is not preserved, and thus must be specified as an argument.
# The temporary could be different than the one provide with COMPV_GAS_ALIGN_STACK.
.macro COMPV_GAS_UNALIGN_STACK register0, register1
    ldp \register0, \register1, [sp], #(2*COMPV_GAS_REG_SZ_BYTES) // use same register (ldp \register0, \register0) crash on iOS
    mov sp, \register0
.endm

#########################################################################
.macro COMPV_GAS_SHADOW_ARGS_TO_STACK nn
    .set nna, ((\nn + 1) & -2) // align stack on 16 bytes (using even number of 8 bytes registers)
    .if \nn >= 1
         stp x0, x1, [sp, #-(2*COMPV_GAS_REG_SZ_BYTES)]!
    .endif
    .if \nn >= 3
        stp x2, x3, [sp, #-(2*COMPV_GAS_REG_SZ_BYTES)]!
    .endif
    .if \nn >= 5
        stp x4, x5, [sp, #-(2*COMPV_GAS_REG_SZ_BYTES)]!
    .endif
    .if \nn >= 7
        stp x6, x7, [sp, #-(2*COMPV_GAS_REG_SZ_BYTES)]!
    .endif
    .if \nn >= 9
         .set counter, 0
        .rept ((nna - 8) + 1) >> 1
             ldp x14, x15, [bp, #(prolog_bytes + (counter*COMPV_GAS_REG_SZ_BYTES))]
             stp x14, x15, [sp, #-(2*COMPV_GAS_REG_SZ_BYTES)]!
             .set counter, counter+2
        .endr
    .endif
.endm

#########################################################################
# requires COMPV_GAS_SHADOW_ARGS_TO_STACK
.macro COMPV_GAS_UNSHADOW_ARGS
    mov sp, bp
.endm

#########################################################################
# requires COMPV_GAS_SHADOW_ARGS_TO_STACK
.macro ldp_arg nn rx ry
    ldp \rx, \ry, [bp, #-((2 + \nn) * COMPV_GAS_REG_SZ_BYTES)]
.endm

#########################################################################
# requires COMPV_GAS_SHADOW_ARGS_TO_STACK
.macro ldr_arg nn rx
    ldr \rx, [bp, #-((2*((1 - (\nn & 1))) + \nn) * COMPV_GAS_REG_SZ_BYTES)]
.endm

#endif /* defined(__aarch64__) */

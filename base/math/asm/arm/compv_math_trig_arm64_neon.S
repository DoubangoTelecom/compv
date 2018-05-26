#########################################################################
# Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__aarch64__)
.include "compv_common_arm64.S"

.data

.extern

.text

#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) const compv_float32_t* x
# arg(1) -> COMPV_ALIGNED(NEON) const compv_float32_t* y
# arg(2) -> COMPV_ALIGNED(NEON) compv_float32_t* r
# arg(3) -> compv_uscalar_t width
# arg(4) -> compv_uscalar_t height
# arg(5) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
.macro CompVMathTrigHypotNaive_32f_Macro_NEON64 fusedMultiplyAdd
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Set arguments ##
	x .req r0
	y .req r1
	out .req r2
	width .req r3
	height .req r4
    stride .req r5

    width16 .req r6
    i .req r7

    prfm pldl1keep, [x, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [x, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [x, #(CACHE_LINE_SIZE*2)]
    prfm pldl1keep, [y, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [y, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [y, #(CACHE_LINE_SIZE*2)]

    and width16, width, #-16

    # Transform stride to padding then from samples to bytes #
	add r11, width, #3
	and r11, r11, #-4
	sub stride, stride, r11
    lsl stride, stride, #(COMPV_GAS_FLOAT32_SHIFT_BYTES)

    ####################################################
    # for (compv_uscalar_t j = 0; j < height; ++j)
    ####################################################
    LoopHeight_CompVMathTrigHypotNaive_32f_Macro_NEON64\@:
        mov i, #0 
        ####################################################
        # for (i = 0; i < width16; i += 16)
        ####################################################
        tst width16, width16
        beq EndOf_LoopWidth16_CompVMathTrigHypotNaive_32f_Macro_NEON64\@
        LoopWidth16_CompVMathTrigHypotNaive_32f_Macro_NEON64\@:
            ldp q0, q1, [x], #(2*COMPV_GAS_Q_SZ_BYTES)
            ldp q2, q3, [x], #(2*COMPV_GAS_Q_SZ_BYTES)
            ldp q4, q5, [y], #(2*COMPV_GAS_Q_SZ_BYTES)
            ldp q6, q7, [y], #(2*COMPV_GAS_Q_SZ_BYTES)
            prfm pldl1keep, [x, #(CACHE_LINE_SIZE*3)]
            prfm pldl1keep, [y, #(CACHE_LINE_SIZE*3)]
            fmul v0.4s, v0.4s, v0.4s
            fmul v1.4s, v1.4s, v1.4s
            fmul v2.4s, v2.4s, v2.4s
            fmul v3.4s, v3.4s, v3.4s
            .if \fusedMultiplyAdd
                fmla v0.4s, v4.4s, v4.4s
                fmla v1.4s, v5.4s, v5.4s
                fmla v2.4s, v6.4s, v6.4s
                fmla v3.4s, v7.4s, v7.4s
            .else
                fmul v4.4s, v4.4s, v4.4s
                fmul v5.4s, v5.4s, v5.4s
                fmul v6.4s, v6.4s, v6.4s
                fmul v7.4s, v7.4s, v7.4s
                fadd v0.4s, v0.4s, v4.4s
                fadd v1.4s, v1.4s, v5.4s
                fadd v2.4s, v2.4s, v6.4s
                fadd v3.4s, v3.4s, v7.4s
            .endif
            fsqrt v0.4s, v0.4s
            fsqrt v1.4s, v1.4s
            fsqrt v2.4s, v2.4s
            fsqrt v3.4s, v3.4s
            stp q0, q1, [out], #(2*COMPV_GAS_Q_SZ_BYTES)
            stp q2, q3, [out], #(2*COMPV_GAS_Q_SZ_BYTES)
            add i, i, #16
            cmp i, width16
            blt LoopWidth16_CompVMathTrigHypotNaive_32f_Macro_NEON64\@
        EndOf_LoopWidth16_CompVMathTrigHypotNaive_32f_Macro_NEON64\@:

        ####################################################
        # for (; i < width; i += 4)
        ####################################################
        LoopWidth4_CompVMathTrigHypotNaive_32f_Macro_NEON64\@:
            ldr q0, [x], #(1*COMPV_GAS_Q_SZ_BYTES)
            ldr q5, [y], #(1*COMPV_GAS_Q_SZ_BYTES)
            fmul v0.4s, v0.4s, v0.4s
            fmla v0.4s, v4.4s, v4.4s
            fsqrt v0.4s, v0.4s
            str q0, [out], #(1*COMPV_GAS_Q_SZ_BYTES)
            add i, i, #4
            cmp i, width
            blt LoopWidth4_CompVMathTrigHypotNaive_32f_Macro_NEON64\@
        EndOf_LoopWidth4_CompVMathTrigHypotNaive_32f_Macro_NEON64\@:

        subs height, height, #1
        add y, y, stride
        add x, x, stride
        add out, out, stride
        bne LoopHeight_CompVMathTrigHypotNaive_32f_Macro_NEON64\@
    EndOf_LoopHeight_CompVMathTrigHypotNaive_32f_Macro_NEON64\@:


    .unreq x
	.unreq y
	.unreq out
	.unreq width
	.unreq height
   .unreq  stride

    .unreq width16
    .unreq i

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVMathTrigHypotNaive_32f_Asm_NEON64
	CompVMathTrigHypotNaive_32f_Macro_NEON64 0

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVMathTrigHypotNaive_32f_Asm_FMA_NEON64
	CompVMathTrigHypotNaive_32f_Macro_NEON64 1

#endif /* defined(__aarch64__) */

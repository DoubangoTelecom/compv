#########################################################################
# Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__arm__) && !defined(__aarch64__)
.include "compv_common_arm32.S"

.data

.extern

.text

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const compv_float32_t* x
@ arg(1) -> COMPV_ALIGNED(NEON) const compv_float32_t* y
@ arg(2) -> COMPV_ALIGNED(NEON) compv_float32_t* r
@ arg(3) -> compv_uscalar_t width
@ arg(4) -> compv_uscalar_t height
@ arg(5) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVMathTrigHypotNaive_32f_Asm_NEON32
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 6
	COMPV_GAS_SAVE_NEON_REGS

	@@ Load arguments @@
	ldm_args r0-r5
	x .req r0
	y .req r1
	out .req r2
	width .req r3
	height .req r4
    stride .req r5

    width16 .req r6
    i .req r7

    pld [x, #(CACHE_LINE_SIZE*0)]
	pld [x, #(CACHE_LINE_SIZE*1)]
	pld [x, #(CACHE_LINE_SIZE*2)]
    pld [y, #(CACHE_LINE_SIZE*0)]
	pld [y, #(CACHE_LINE_SIZE*1)]
	pld [y, #(CACHE_LINE_SIZE*2)]

    and width16, width, #-16

    @ Transform stride to padding then from samples to bytes @
	add r11, width, #3
	and r11, r11, #-4
	sub stride, stride, r11
    lsl stride, stride, #(COMPV_GAS_FLOAT32_SHIFT_BYTES)

    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (compv_uscalar_t j = 0; j < height; ++j)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    LoopHeight_CompVMathTrigHypotNaive_32f_Asm_NEON32:
        mov i, #0 
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (i = 0; i < width16; i += 16)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        tst width16, width16
        beq EndOf_LoopWidth16_CompVMathTrigHypotNaive_32f_Asm_NEON32
        LoopWidth16_CompVMathTrigHypotNaive_32f_Asm_NEON32:
            vld1.u8 { q0, q1 }, [x]!
            vld1.u8 { q2, q3 }, [x]!
            vld1.u8 { q4, q5 }, [y]!
            vld1.u8 { q6, q7 }, [y]!
            pld [x, #(CACHE_LINE_SIZE*3)]
            pld [y, #(CACHE_LINE_SIZE*3)]
            vmul.f32 q0, q0, q0
            vmul.f32 q1, q1, q1
            vmul.f32 q2, q2, q2
            vmul.f32 q3, q3, q3
            vmla.f32 q0, q4, q4
            vmla.f32 q1, q5, q5
            vmla.f32 q2, q6, q6
            vmla.f32 q3, q7, q7
            @ TODO(dmi): 'vsqrt.f32' not vectorized and not using approx implementaion (to make sure MD5 match) -> !! PERF ISSUE!!
            vsqrt.f32 s0, s0
            vsqrt.f32 s1, s1
            vsqrt.f32 s2, s2
            vsqrt.f32 s3, s3
            vsqrt.f32 s4, s4
            vsqrt.f32 s5, s5
            vsqrt.f32 s6, s6
            vsqrt.f32 s7, s7
            vsqrt.f32 s8, s8
            vsqrt.f32 s9, s9
            vsqrt.f32 s10, s10
            vsqrt.f32 s11, s11
            vsqrt.f32 s12, s12
            vsqrt.f32 s13, s13
            vsqrt.f32 s14, s14
            vsqrt.f32 s15, s15
            vst1.f32 {q0, q1}, [out :128]!
            vst1.f32 {q2, q3}, [out :128]!
            add i, i, #16
            cmp i, width16
            blt LoopWidth16_CompVMathTrigHypotNaive_32f_Asm_NEON32
        EndOf_LoopWidth16_CompVMathTrigHypotNaive_32f_Asm_NEON32:

        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (; i < width; i += 4)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        LoopWidth4_CompVMathTrigHypotNaive_32f_Asm_NEON32:
            vld1.u8 { q0 }, [x]!
            vld1.u8 { q4 }, [y]!
            vmul.f32 q0, q0, q0
            vmla.f32 q0, q4, q4
            @ TODO(dmi): 'vsqrt.f32' not vectorized and not using approx implementaion (to make sure MD5 match) -> !! PERF ISSUE!!
            vsqrt.f32 s0, s0
            vsqrt.f32 s1, s1
            vsqrt.f32 s2, s2
            vsqrt.f32 s3, s3
            vst1.f32 {q0}, [out :128]!
            add i, i, #4
            cmp i, width
            blt LoopWidth4_CompVMathTrigHypotNaive_32f_Asm_NEON32
        EndOf_LoopWidth4_CompVMathTrigHypotNaive_32f_Asm_NEON32:

        subs height, height, #1
        add y, y, stride
        add x, x, stride
        add out, out, stride
        bne LoopHeight_CompVMathTrigHypotNaive_32f_Asm_NEON32
    EndOf_LoopHeight_CompVMathTrigHypotNaive_32f_Asm_NEON32:


    .unreq x
	.unreq y
	.unreq out
	.unreq width
	.unreq height
   .unreq  stride

    .unreq width16
    .unreq i

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 6
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__arm__) && !defined(__aarch64__) */

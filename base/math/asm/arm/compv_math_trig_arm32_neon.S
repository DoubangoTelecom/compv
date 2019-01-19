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

.align 4
kAtan2Eps_32f: .single 2.2204460492503131e-016, 2.2204460492503131e-016, 2.2204460492503131e-016, 2.2204460492503131e-016
kAtan2P1_32f: .single 57.2836266, 57.2836266, 57.2836266, 57.2836266
kAtan2P3_32f: .single -18.6674461, -18.6674461, -18.6674461, -18.6674461
kAtan2P5_32f: .single 8.91400051, 8.91400051, 8.91400051, 8.91400051
kAtan2P7_32f: .single -2.53972459, -2.53972459, -2.53972459, -2.53972459
k90_32f: .single 90.0, 90.0, 90.0, 90.0
k180_32f: .single 180.0, 180.0, 180.0, 180.0
k360_32f: .single 360.0, 360.0, 360.0, 360.0

.extern

.text

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const compv_float32_t* y
@ arg(1) -> COMPV_ALIGNED(NEON) const compv_float32_t* x
@ arg(2) -> COMPV_ALIGNED(NEON) compv_float32_t* r
@ arg(3) -> const compv_float32_t* scale1
@ arg(4) -> compv_uscalar_t width
@ arg(5) -> compv_uscalar_t height
@ arg(6) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
.macro CompVMathTrigFastAtan2_32f_Macro_NEON32 fusedMultiplyAdd
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 7
	COMPV_GAS_SAVE_NEON_REGS

    @@ Load arguments @@
	ldm_args r0-r6
	y .req r0
	x .req r1
	out .req r2
    scale1 .req r3
	width .req r4
	height .req r5
    stride .req r6

    i .req r7
	r_vecAtan2_p1 .req r8

    vec1 .req q0 @ MUST not change (used by vdiv.f32)
    vec2 .req q1 @ MUST not change (used by vdiv.f32)
    vecAx .req q2
    vecAy .req q3
    vecMask .req q4
    vec0 .req q5
    vecAtan2_scale .req q6
    vecAtan2_plus360 .req q7
    vecAtan2_plus180 .req q8
    vecAtan2_plus90 .req q9
    vecAtan2_p3 .req q10
    vecAtan2_p7 .req q11
    vecAtan2_p5 .req q12
    vecAtan2_eps .req q13
	vecAx0 .req q14
	vecAy0 .req q15

    pld [x, #(CACHE_LINE_SIZE*0)]
	pld [x, #(CACHE_LINE_SIZE*1)]
	pld [x, #(CACHE_LINE_SIZE*2)]
    pld [y, #(CACHE_LINE_SIZE*0)]
	pld [y, #(CACHE_LINE_SIZE*1)]
	pld [y, #(CACHE_LINE_SIZE*2)]

    ldr scale1, [scale1]
    ldr r7, =kAtan2Eps_32f
    ldr r8, =k90_32f
    ldr r9, =kAtan2P3_32f
    ldr r10, =kAtan2P5_32f
    ldr r11, =kAtan2P7_32f
    vld1.f32 { vecAtan2_eps }, [r7]
    vld1.f32 { vecAtan2_plus90 }, [r8]
    vld1.f32 { vecAtan2_p3 }, [r9]
    vld1.f32 { vecAtan2_p5 }, [r10]
    vld1.f32 { vecAtan2_p7 }, [r11]
    ldr r7, =k180_32f
    ldr r8, =k360_32f
    vld1.f32 { vecAtan2_plus180 }, [r7]
    vld1.f32 { vecAtan2_plus360 }, [r8]
    vdup.f32 vecAtan2_scale, scale1    

    @ Transform stride to padding then from samples to bytes @
	add r11, width, #3
	and r11, r11, #-4
	sub stride, stride, r11
    lsl stride, stride, #(COMPV_GAS_FLOAT32_SHIFT_BYTES)

	ldr r_vecAtan2_p1, =kAtan2P1_32f

    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (compv_uscalar_t j = 0; j < height; ++j)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    LoopHeight_CompVMathTrigFastAtan2_32f_Macro_NEON32\@:
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (compv_uscalar_t i = 0; i < width; i += 4)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        mov i, #0
        LoopWidth_CompVMathTrigFastAtan2_32f_Macro_NEON32\@:
            vld1.u8 { vecAx0 }, [x :128]!
            vld1.u8 { vecAy0 }, [y :128]!
            pld [x, #(CACHE_LINE_SIZE*3)]
            pld [y, #(CACHE_LINE_SIZE*3)]

            @ ax = std::abs(x[i]), ay = std::abs(y[i]);
            vabs.f32 vecAx, vecAx0
            vabs.f32 vecAy, vecAy0

            @ if (ax >= ay) vec1 = ay, vec2 = ax;
            @ else vec1 = ax, vec2 = ay;
            vcge.f32 vecMask, vecAx, vecAy
            vand.s32 vec1, vecAy, vecMask
            vand.s32 vec2, vecAx, vecMask
            vbic.s32 vecAx, vecAx, vecMask
            vbic.s32 vecAy, vecAy, vecMask
            vorr.s32 vec1, vec1, vecAx
            vorr.s32 vec2, vec2, vecAy

            @ c = vec1 / (vec2 + atan2_eps)
            @ c2 = c*c
            vadd.f32 vec2, vec2, vecAtan2_eps
            @ TODO(dmi): 'vdiv.f32' not vectorized and not using approx implementaion (to make sure MD5 match) -> !! PERF ISSUE!!
            @ vec1 = vecC
            @ vec2 = vecC2
            vdiv.f32 s0, s0, s4
            vdiv.f32 s1, s1, s5
            vdiv.f32 s2, s2, s6
            vdiv.f32 s3, s3, s7
            vmov.f32 vecAx, vecAtan2_p5
            vmov.f32 vecAy, vecAtan2_p3
			vld1.f32 { vec0 }, [r_vecAtan2_p1]
            vmul.f32 vec2, vec1, vec1

            @ a = (((atan2_p7*c2 + atan2_p5)*c2 + atan2_p3)*c2 + atan2_p1)*c
            .if \fusedMultiplyAdd
                vfma.f32 vecAx, vecAtan2_p7, vec2
                vfma.f32 vecAy, vecAx, vec2
                vfma.f32 vec0, vecAy, vec2
            .else
                vmla.f32 vecAx, vecAtan2_p7, vec2
                vmla.f32 vecAy, vecAx, vec2
                vmla.f32 vec0, vecAy, vec2
            .endif
            vmul.f32 vec0, vec0, vec1

            @ if (!(ax >= ay)) a = 90 - a
            vsub.f32 vec1, vecAtan2_plus90, vec0
            vand.s32 vec0, vec0, vecMask
            vbic.s32 vec1, vec1, vecMask
            vorr.s32 vec0, vec0, vec1

            @ if (x[i] < 0) a = 180.f - a
            vclt.f32 vecMask, vecAx0, #0
            vsub.f32 vec1, vecAtan2_plus180, vec0
            vbic.s32 vec0, vec0, vecMask
            vand.s32 vec1, vec1, vecMask
            vorr.s32 vec0, vec0, vec1

            @ if (y[i] < 0) a = 360.f - a
            vclt.f32 vecMask, vecAy0, #0
            vsub.f32 vec1, vecAtan2_plus360, vec0
            vbic.s32 vec0, vec0, vecMask
            vand.s32 vec1, vec1, vecMask
            vorr.s32 vec0, vec0, vec1

            @ r[i] = a * scale
            vmul.f32 vec0, vec0, vecAtan2_scale
            vst1.f32 { vec0 }, [out: 128]!

            add i, i, #4
            cmp i, width
            blt LoopWidth_CompVMathTrigFastAtan2_32f_Macro_NEON32\@
        EndOf_LoopWidth_CompVMathTrigFastAtan2_32f_Macro_NEON32\@:

        subs height, height, #1
        add y, y, stride
        add x, x, stride
        add out, out, stride
        bne LoopHeight_CompVMathTrigFastAtan2_32f_Macro_NEON32\@
    EndOf_LoopHeight_CompVMathTrigFastAtan2_32f_Macro_NEON32\@:

    .unreq y
    .unreq x
	.unreq out
    .unreq scale1
	.unreq width
	.unreq height
    .unreq stride

    .unreq i
	.unreq r_vecAtan2_p1

    .unreq vec1
    .unreq vec2
    .unreq vecAx
    .unreq vecAy
    .unreq vecMask
    .unreq vec0
    .unreq vecAtan2_scale
    .unreq vecAtan2_plus360
    .unreq vecAtan2_plus180
    .unreq vecAtan2_plus90
    .unreq vecAtan2_p3
    .unreq vecAtan2_p7
    .unreq vecAtan2_p5
    .unreq vecAtan2_eps
	.unreq vecAx0
	.unreq vecAy0

    COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 7
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVMathTrigFastAtan2_32f_Asm_NEON32
    CompVMathTrigFastAtan2_32f_Macro_NEON32 0

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVMathTrigFastAtan2_32f_Asm_FMA_NEON32
    CompVMathTrigFastAtan2_32f_Macro_NEON32 1

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const compv_float32_t* x
@ arg(1) -> COMPV_ALIGNED(NEON) const compv_float32_t* y
@ arg(2) -> COMPV_ALIGNED(NEON) compv_float32_t* r
@ arg(3) -> compv_uscalar_t width
@ arg(4) -> compv_uscalar_t height
@ arg(5) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
.macro CompVMathTrigHypotNaive_32f_Macro_NEON32 fusedMultiplyAdd
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
    LoopHeight_CompVMathTrigHypotNaive_32f_Macro_NEON32\@:
        mov i, #0 
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (i = 0; i < width16; i += 16)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        tst width16, width16
        beq EndOf_LoopWidth16_CompVMathTrigHypotNaive_32f_Macro_NEON32\@
        LoopWidth16_CompVMathTrigHypotNaive_32f_Macro_NEON32\@:
            vld1.u8 { q0, q1 }, [x :128]!
            vld1.u8 { q2, q3 }, [x :128]!
            vld1.u8 { q4, q5 }, [y :128]!
            vld1.u8 { q6, q7 }, [y :128]!
            pld [x, #(CACHE_LINE_SIZE*3)]
            pld [y, #(CACHE_LINE_SIZE*3)]
            vmul.f32 q0, q0, q0
            vmul.f32 q1, q1, q1
            vmul.f32 q2, q2, q2
            vmul.f32 q3, q3, q3
            .if \fusedMultiplyAdd
                vfma.f32 q0, q4, q4
                vfma.f32 q1, q5, q5
                vfma.f32 q2, q6, q6
                vfma.f32 q3, q7, q7
            .else
                vmla.f32 q0, q4, q4
                vmla.f32 q1, q5, q5
                vmla.f32 q2, q6, q6
                vmla.f32 q3, q7, q7
            .endif
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
            blt LoopWidth16_CompVMathTrigHypotNaive_32f_Macro_NEON32\@
        EndOf_LoopWidth16_CompVMathTrigHypotNaive_32f_Macro_NEON32\@:

        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (; i < width; i += 4)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		cmp i, width
		bge EndOf_LoopWidth4_CompVMathTrigHypotNaive_32f_Macro_NEON32\@
        LoopWidth4_CompVMathTrigHypotNaive_32f_Macro_NEON32\@:
            vld1.u8 { q0 }, [x :128]!
            vld1.u8 { q4 }, [y :128]!
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
            blt LoopWidth4_CompVMathTrigHypotNaive_32f_Macro_NEON32\@
        EndOf_LoopWidth4_CompVMathTrigHypotNaive_32f_Macro_NEON32\@:

        subs height, height, #1
        add y, y, stride
        add x, x, stride
        add out, out, stride
        bne LoopHeight_CompVMathTrigHypotNaive_32f_Macro_NEON32\@
    EndOf_LoopHeight_CompVMathTrigHypotNaive_32f_Macro_NEON32\@:


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
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVMathTrigHypotNaive_32f_Asm_NEON32
	CompVMathTrigHypotNaive_32f_Macro_NEON32 0

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVMathTrigHypotNaive_32f_Asm_FMA_NEON32
	CompVMathTrigHypotNaive_32f_Macro_NEON32 1

#endif /* defined(__arm__) && !defined(__aarch64__) */

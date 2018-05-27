#########################################################################
# Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__arm__) && !defined(__aarch64__)
.include "compv_common_arm32.S"

.equ normL1, 0
.equ normL1Sqrt, 1
.equ normL2, 2
.equ normL2Hys, 3

.data

.align 4
kOne_32f: .single 1.0
kZeroDotTwo_32f: .single 0.2

.extern

.text

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> compv_float32_t* inOutPtr
@ arg(1) -> const compv_float32_t* eps1
@ arg(2) -> const compv_uscalar_t count
.macro CompVHogCommonNormL1_9_32f_Macro_NEON32 norm
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	@@ Set arguments @@
	inOutPtr .req r0
	eps1 .req r1

    idx8 .req r2

    vec0 .req q0 @ Must not change
    vec0x .req q0x
    vec0y .req q0y
    veca .req q1 @ Must not change
    vecb .req q2 @ Must not change

    ldr r11, =kOne_32f
    add idx8, inOutPtr, #(8 * COMPV_GAS_FLOAT32_SZ_BYTES)
    vld1.f32 {d11[0]}, [r11] @ s22 = vecOne
    vld1.f32 {d11[1]}, [eps1] @ s23 = vecEps
    vld1.f32 {veca, vecb}, [inOutPtr]
    vld1.f32 {d12[0]}, [idx8] @ s24 = vvb
    
    vadd.f32 vec0, veca, vecb
    vadd.f32 vec0x, vec0x, vec0y
    vpadd.f32 vec0x, vec0x, vec0x
    vadd.f32 s0, s0, s24
    vadd.f32 s0, s0, s23
    vdiv.f32 s0, s22, s0
    vdup.f32 vec0, vec0x[0]
    vmul.f32 veca, veca, vec0
    vmul.f32 vecb, vecb, vec0
    vmul.f32 s24, s24, s0

    .if \norm == normL1Sqrt
        @ TODO(dmi): 'vsqrt.f32' not vectorized and not using approx implementaion (to make sure MD5 match) -> !! PERF ISSUE!!
        vsqrt.f32 s4, s4
        vsqrt.f32 s5, s5
        vsqrt.f32 s6, s6
        vsqrt.f32 s7, s7
        vsqrt.f32 s8, s8
        vsqrt.f32 s9, s9
        vsqrt.f32 s10, s10
        vsqrt.f32 s11, s11
        vsqrt.f32 s24, s24
    .endif

    vst1.f32 {veca, vecb}, [inOutPtr]
    vst1.f32 {d12[0]}, [idx8]

    .unreq inOutPtr
    .unreq eps1

    .unreq idx8

    .unreq vec0
    .unreq vec0x
    .unreq vec0y
    .unreq veca
    .unreq vecb

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVHogCommonNormL1_9_32f_Asm_NEON32
    CompVHogCommonNormL1_9_32f_Macro_NEON32 normL1

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVHogCommonNormL1Sqrt_9_32f_Asm_NEON32
    CompVHogCommonNormL1_9_32f_Macro_NEON32 normL1Sqrt

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.macro CompVHogCommonNormL2_9_32f_Macro_Round_NEON32
    vmul.f32 vec0, veca, veca
    vmul.f32 vec1, vecb, vecb
    vmul.f32 s25, s24, s24 @ s25 = vvb2
    vadd.f32 vec0, vec0, vec1
    vadd.f32 vec0x, vec0x, vec0y
    vpadd.f32 vec0x, vec0x, vec0x
    vadd.f32 s0, s0, s25
    vadd.f32 s0, s0, s23
    vsqrt.f32 s0, s0
    vdiv.f32 s0, s22, s0
    vdup.f32 vec0, vec0x[0]
    vmul.f32 veca, veca, vec0
    vmul.f32 vecb, vecb, vec0
    vmul.f32 s24, s24, s0
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> compv_float32_t* inOutPtr
@ arg(1) -> const compv_float32_t* eps1
@ arg(2) -> const compv_uscalar_t count
.macro CompVHogCommonNormL2_9_32f_Macro_NEON32 norm
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	@@ Set arguments @@
	inOutPtr .req r0
	eps1 .req r1

    idx8 .req r2

    vec0 .req q0 @ Must not change
    vec0x .req q0x
    vec0y .req q0y
    veca .req q1 @ Must not change
    vecb .req q2 @ Must not change
    vec1 .req q3

    ldr r11, =kOne_32f
    add idx8, inOutPtr, #(8 * COMPV_GAS_FLOAT32_SZ_BYTES)
    vld1.f32 {d11[0]}, [r11] @ s22 = vecOne
    vld1.f32 {d11[1]}, [eps1] @ s23 = vecEps
    vld1.f32 {veca, vecb}, [inOutPtr]
    vld1.f32 {d12[0]}, [idx8] @ s24 = vvb - ARM64: idx not needed, ldr accepts pre-index
    .if \norm == normL2Hys
        ldr r11, =kZeroDotTwo_32f
        ldr r11, [r11]
        vdup.f32 q7, r11 @ q7(d14, d15)(s28, s29, s30, s31) = vecMax
    .endif
    
    CompVHogCommonNormL2_9_32f_Macro_Round_NEON32
    .if \norm == normL2Hys
        vmin.f32 veca, veca, q7
        vmin.f32 vecb, vecb, q7
        vmin.f32 d12, d12, d14 @ vmin.f32 s24, s24, s28
        CompVHogCommonNormL2_9_32f_Macro_Round_NEON32
    .endif

    vst1.f32 {veca, vecb}, [inOutPtr]
    vst1.f32 {d12[0]}, [idx8] @ s24

    .unreq inOutPtr
    .unreq eps1

    .unreq idx8

    .unreq vec0
    .unreq vec0x
    .unreq vec0y
    .unreq veca
    .unreq vecb
    .unreq vec1

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVHogCommonNormL2_9_32f_Asm_NEON32
    CompVHogCommonNormL2_9_32f_Macro_NEON32 normL2

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVHogCommonNormL2Hys_9_32f_Asm_NEON32
    CompVHogCommonNormL2_9_32f_Macro_NEON32 normL2Hys

#endif /* defined(__arm__) && !defined(__aarch64__) */

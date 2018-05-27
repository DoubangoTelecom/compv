#########################################################################
# Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__aarch64__)
.include "compv_common_arm64.S"

.equ normL1, 0
.equ normL1Sqrt, 1
.equ normL2, 2
.equ normL2Hys, 3

.equ kOne_32f_0, 0x0000
.equ kOne_32f_1, 0x3f80
.equ kZeroDotTwo_32f_0, 0xcccd
.equ kZeroDotTwo_32f_1, 0x3e4c

.data

.extern

.text

#########################################################################
# arg(0) -> compv_float32_t* inOutPtr
# arg(1) -> const compv_float32_t* eps1
# arg(2) -> const compv_uscalar_t count
.macro CompVHogCommonNormL1_9_32f_Macro_NEON64 norm
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Set arguments ##
	inOutPtr .req r0
	eps1 .req r1
    inOutPtr8 .req r2

    vec0 .req v0 // Must not change
    veca .req v1 // Must not change
    vecb .req v2 // Must not change


    movz r11w, #kOne_32f_0
    movk r11w, #kOne_32f_1, lsl #16
    add inOutPtr8, inOutPtr, #(8 * COMPV_GAS_FLOAT32_SZ_BYTES)
    mov v5.s[0], r11w // s5 = vecOne
    ld1 {v6.s}[0], [eps1] // s6 = vecEps
    ldr q1, [inOutPtr] // q1 = veca
    ldr q2, [inOutPtr, #(4 * COMPV_GAS_FLOAT32_SZ_BYTES)] // q2 = vecb
    ld1 {v7.s}[0], [inOutPtr8] // s7 = vvb
    
    fadd vec0.4s, veca.4s, vecb.4s
    ext v3.16b, vec0.16b, vec0.16b, #8
    fadd vec0.2s, vec0.2s, v3.2s
    faddp vec0.2s, vec0.2s, vec0.2s
    fadd s0, s0, s7
    fadd s0, s0, s6
    fdiv s0, s5, s0
    dup vec0.4s, vec0.s[0]
    
    fmul veca.4s, veca.4s, vec0.4s
    fmul vecb.4s, vecb.4s, vec0.4s
    fmul s7, s7, s0

    .if \norm == normL1Sqrt
        fsqrt veca.4s, veca.4s
        fsqrt vecb.4s, vecb.4s
        fsqrt s7, s7
    .endif

    str q1, [inOutPtr]
    str q2, [inOutPtr, #(4 * COMPV_GAS_FLOAT32_SZ_BYTES)]
    st1 {v7.s}[0], [inOutPtr8]

    .unreq inOutPtr
    .unreq eps1
    .unreq inOutPtr8

    .unreq vec0
    .unreq vec0x
    .unreq vec0y
    .unreq veca
    .unreq vecb

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVHogCommonNormL1_9_32f_Asm_NEON64
    CompVHogCommonNormL1_9_32f_Macro_NEON64 normL1

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVHogCommonNormL1Sqrt_9_32f_Asm_NEON64
    CompVHogCommonNormL1_9_32f_Macro_NEON64 normL1Sqrt


#########################################################################
.macro CompVHogCommonNormL2_9_32f_Macro_Round_NEON64
    vmul.f32 vec0, veca, veca
    vmul.f32 vec1, vecb, vecb
    vmul.f32 s25, s24, s24 # s25 = vvb2
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

#########################################################################
# arg(0) -> compv_float32_t* inOutPtr
# arg(1) -> const compv_float32_t* eps1
# arg(2) -> const compv_uscalar_t count
.macro CompVHogCommonNormL2_9_32f_Macro_NEON64 norm
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Set arguments ##
	inOutPtr .req r0
	eps1 .req r1

    idx4 .req r2
    idx8 .req r3

    vec0 .req q0 # Must not change
    vec0x .req q0x
    vec0y .req q0y
    veca .req q1 # Must not change
    vecb .req q2 # Must not change
    vec1 .req q3

    ldr r11, =kOne_32f
    add idx4, inOutPtr, #(4 * COMPV_GAS_FLOAT32_SZ_BYTES)
    add idx8, inOutPtr, #(8 * COMPV_GAS_FLOAT32_SZ_BYTES)
    vld1.f32 {d11[0]}, [r11] # s22 = vecOne
    vld1.f32 {d11[1]}, [eps1] # s23 = vecEps
    vld1.f32 {veca}, [inOutPtr]
    vld1.f32 {vecb}, [idx4] # ARM64: idx not needed, ldr accepts pre-index
    vld1.f32 {d12[0]}, [idx8] # s24 = vvb - ARM64: idx not needed, ldr accepts pre-index
    .if \norm == normL2Hys
        ldr r11, =kZeroDotTwo_32f
        ldr r11, [r11]
        vdup.f32 q7, r11 # q7(d14, d15)(s28, s29, s30, s31) = vecMax
    .endif
    
    CompVHogCommonNormL2_9_32f_Macro_Round_NEON64
    .if \norm == normL2Hys
        vmin.f32 veca, veca, q7
        vmin.f32 vecb, vecb, q7
        vmin.f32 d12, d12, d14 # vmin.f32 s24, s24, s28
        CompVHogCommonNormL2_9_32f_Macro_Round_NEON64
    .endif

    vst1.f32 {veca}, [inOutPtr]
    vst1.f32 {vecb}, [idx4]
    vst1.f32 {d12[0]}, [idx8] # s24

    .unreq inOutPtr
    .unreq eps1

    .unreq idx4
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

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVHogCommonNormL2_9_32f_Asm_NEON64
    CompVHogCommonNormL2_9_32f_Macro_NEON64 normL2

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVHogCommonNormL2Hys_9_32f_Asm_NEON64
    CompVHogCommonNormL2_9_32f_Macro_NEON64 normL2Hys

#endif /* defined(__aarch64__) */

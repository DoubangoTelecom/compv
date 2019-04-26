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
    ldp q1, q2, [inOutPtr] // q1 = veca, q2 = vecb
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

    stp q1, q2, [inOutPtr]
    st1 {v7.s}[0], [inOutPtr8]

    .unreq inOutPtr
    .unreq eps1
    .unreq inOutPtr8

    .unreq vec0
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
    fmul vec0.4s, veca.4s, veca.4s
    fmul vec1.4s, vecb.4s, vecb.4s
    fmul s9, s7, s7 // s9 = vvb2    
    fadd vec0.4s, vec0.4s, vec1.4s
    ext vec1.16b, vec0.16b, vec0.16b, #8
    fadd vec0.2s, vec0.2s, vec1.2s
    faddp vec0.2s, vec0.2s, vec0.2s
    fadd s0, s0, s9
    fadd s0, s0, s6
    fsqrt s0, s0
    fdiv s0, s5, s0
    dup vec0.4s, vec0.s[0]
    fmul veca.4s, veca.4s, vec0.4s
    fmul vecb.4s, vecb.4s, vec0.4s
    fmul s7, s7, s0
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
    inOutPtr8 .req r2

    vec0 .req v0 // Must not change
    veca .req v1 // Must not change
    vecb .req v2 // Must not change
    vec1 .req v3

    movz r11w, #kOne_32f_0
    movk r11w, #kOne_32f_1, lsl #16
    .if \norm == normL2Hys
        movz r12w, #kZeroDotTwo_32f_0
        movk r12w, #kZeroDotTwo_32f_1, lsl #16
    .endif
    add inOutPtr8, inOutPtr, #(8 * COMPV_GAS_FLOAT32_SZ_BYTES)
    mov v5.s[0], r11w // s5 = vecOne
    .if \norm == normL2Hys
        dup v8.4s, r12w // s8 = vecMax
    .endif
    ld1 {v6.s}[0], [eps1] // s6 = vecEps
    ldp q1, q2, [inOutPtr] // q1 = veca, q2 = vecb
    ld1 {v7.s}[0], [inOutPtr8] // s7 = vvb

    CompVHogCommonNormL2_9_32f_Macro_Round_NEON64
    .if \norm == normL2Hys
        fmin veca.4s, veca.4s, v8.4s
        fmin vecb.4s, vecb.4s, v8.4s
        fmin s7, s7, s8
        CompVHogCommonNormL2_9_32f_Macro_Round_NEON64
    .endif

    stp q1, q2, [inOutPtr]
    st1 {v7.s}[0], [inOutPtr8]

    .unreq inOutPtr
    .unreq eps1
    .unreq inOutPtr8

    .unreq vec0
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

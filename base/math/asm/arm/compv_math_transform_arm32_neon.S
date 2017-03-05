#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__arm__) && !defined(__aarch64__)
.include "compv_common_arm32.S"

#if defined(__APPLE__)
#   define sym(funcname) _##funcname
#else
#   define sym(funcname) funcname
#endif

.data

.extern
 
.text


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(SSE) const compv_float64_t* srcX
@ arg(1) -> COMPV_ALIGNED(SSE) const compv_float64_t* srcY
@ arg(2) -> COMPV_ALIGNED(SSE) const compv_float64_t* srcZ
@ arg(3) -> COMPV_ALIGNED(SSE) compv_float64_t* dstX
@ arg(4) -> COMPV_ALIGNED(SSE) compv_float64_t* dstY
@ arg(5) -> compv_uscalar_t numPoints
COMPV_GAS_FUNCTION_DECLARE CompVMathTransformHomogeneousToCartesian2D_4_64f_Intrin_Asm_NEON32
    COMPV_GAS_FUNCTION_PROLOG
    COMPV_GAS_SHADOW_ARGS_TO_STACK 6
	COMPV_GAS_SAVE_NEON_REGS

	@@ Load arguments @@
    ldm_args r0-r5
	srcX .req r0
	srcY .req r1
	srcZ .req r2
	dstX .req r3
    dstY .req r4
    numPoints .req r5

    vmov.f64 q15x, #1.000000e+00
    vld1.64 { q13, q14 }, [srcZ :128]
    vdiv.f64 q0x, q15x, q13x
    vdiv.f64 q0y, q15x, q13y
    vdiv.f64 q1x, q15x, q14x
    vdiv.f64 q1y, q15x, q14y
    vld1.64 { q11, q12 }, [srcX :128]
    vmul.f64 q2x, q11x, q0x
    vmul.f64 q2y, q11y, q0y
    vmul.f64 q3x, q12x, q1x
    vmul.f64 q3y, q12y, q1y
    vld1.64 { q9, q10 }, [srcY :128]
    vmul.f64 q4x, q9x, q0x
    vmul.f64 q4y, q9y, q0y
    vmul.f64 q5x, q10x, q1x
    vmul.f64 q5y, q10y, q1y
    vst1.64 { q2, q3 }, [dstX :128]
    vst1.64 { q4, q5 }, [dstY :128]
    
    .unreq srcX
	.unreq srcY
	.unreq srcZ
	.unreq dstX
    .unreq dstY
    .unreq numPoints

    COMPV_GAS_RESTORE_NEON_REGS
    COMPV_GAS_UNSHADOW_ARGS 6
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#endif /* defined(__arm__) && !defined(__aarch64__) */
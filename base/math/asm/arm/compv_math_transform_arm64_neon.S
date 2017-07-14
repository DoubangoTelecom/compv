#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__aarch64__)
.include "compv_common_arm64.S"

#if defined(__APPLE__)
#   define sym(funcname) _##funcname
#else
#   define sym(funcname) funcname
#endif

.data

.extern

.text


#########################################################################
# arg(0) -> COMPV_ALIGNED(SSE) const compv_float64_t* srcX
# arg(1) -> COMPV_ALIGNED(SSE) const compv_float64_t* srcY
# arg(2) -> COMPV_ALIGNED(SSE) const compv_float64_t* srcZ
# arg(3) -> COMPV_ALIGNED(SSE) compv_float64_t* dstX
# arg(4) -> COMPV_ALIGNED(SSE) compv_float64_t* dstY
# arg(5) -> compv_uscalar_t numPoints
COMPV_GAS_FUNCTION_DECLARE CompVMathTransformHomogeneousToCartesian2D_4_64f_Asm_NEON64
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Load arguments ##
	srcX .req r0
	srcY .req r1
	srcZ .req r2
	dstX .req r3
    dstY .req r4
    numPoints .req r5

	fmov v15.2d, #1.00000000
    ldp q13, q14, [srcZ]
    fdiv v0.2d, v15.2d, v13.2d
    fdiv v1.2d, v15.2d, v14.2d
    ldp q11, q12, [srcX]
    fmul v2.2d, v11.2d, v0.2d
    fmul v3.2d, v12.2d, v1.2d
    ldp q9, q10, [srcY]
    fmul v4.2d, v9.2d, v0.2d
    fmul v5.2d, v10.2d, v1.2d
    stp q2, q3, [dstX]
    stp q4, q5, [dstY]
    
    .unreq srcX
	.unreq srcY
	.unreq srcZ
	.unreq dstX
    .unreq dstY
    .unreq numPoints

    COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#endif /* defined(__aarch64__) */


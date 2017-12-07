#########################################################################
# Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   #
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
# arg(0) -> COMPV_ALIGNED(NEON) const compv_float64_t* x
# arg(1) -> COMPV_ALIGNED(NEON) const compv_float64_t* y
# arg(2) -> compv_uscalar_t numPoints
# arg(3) -> compv_float64_t* tx1
# arg(4) -> compv_float64_t* ty1
# arg(5) -> compv_float64_t* sf1
COMPV_GAS_FUNCTION_DECLARE CompVMathStatsNormalize2DHartley_64f_Asm_NEON64
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Load arguments ##
	x .req r0
	y .req r1
	numPoints .req r2
	tx1 .req r3
    ty1 .req r4
    sf1 .req r5

    i .req r6

    vecSqrt2 .req v26
    vecTx .req v27
    vecTy .req v28
    vecMagnitude .req v29
    vecOneOverNumPoints .req v30

    movi vecTx.2d, #0
    movi vecTy.2d, #0
    movi vecMagnitude.2d, #0

    mov r11, #2
    ucvtf d20, numPoints
    ucvtf d21, r11
    fmov vecOneOverNumPoints.2d, #1.00000000
    dup vecSqrt2.2d, v21.d[0]
    dup v20.2d, v20.d[0]
    fdiv vecOneOverNumPoints.2d, vecOneOverNumPoints.2d, v20.2d
    fsqrt vecSqrt2.2d, vecSqrt2.2d

    mov r14, x
    mov r15, y
    

    #########################################################################
    // for (i = 0; i < numPoints_ - 7; i += 8)
    #########################################################################
    lsr i, numPoints, #3
    cbz i, EndOf_LoopTxTy8_CompVMathStatsNormalize2DHartley_64f_Asm_NEON64
    LoopTxTy8_CompVMathStatsNormalize2DHartley_64f_Asm_NEON64:
        subs i, i, #1
        ldp q0, q1, [r14], #32
        ldp q2, q3, [r14], #32
        fadd v0.2d, v0.2d, v1.2d
        fadd v2.2d, v2.2d, v3.2d
        ldp q4, q5, [r15], #32
        ldp q6, q7, [r15], #32
        fadd v4.2d, v4.2d, v5.2d
        fadd v6.2d, v6.2d, v7.2d
        fadd v0.2d, v0.2d, v2.2d
        fadd v4.2d, v4.2d, v6.2d
        fadd vecTx.2d, vecTx.2d, v0.2d
        fadd vecTy.2d, vecTy.2d, v4.2d
        bne LoopTxTy8_CompVMathStatsNormalize2DHartley_64f_Asm_NEON64
        EndOf_LoopTxTy8_CompVMathStatsNormalize2DHartley_64f_Asm_NEON64:
        ## EndOf_LoopTxTy8_CompVMathStatsNormalize2DHartley_64f_Asm_NEON64 ##

    and r8, numPoints, #7 // modulo 8
    and r9, numPoints, #3 // modulo 4


    #########################################################################
    # .if (i < numPoints_ - 3)
    #########################################################################
    lsr i, r8, #2
    cbz i, EndOf_LoopTxTy4_CompVMathStatsNormalize2DHartley_64f_Asm_NEON64
    LoopTxTy4_CompVMathStatsNormalize2DHartley_64f_Asm_NEON64:
        ldp q0, q1, [r14], #32
        fadd v0.2d, v0.2d, v1.2d
        ldp q4, q5, [r15], #32
        fadd v4.2d, v4.2d, v5.2d
        fadd vecTx.2d, vecTx.2d, v0.2d
        fadd vecTy.2d, vecTy.2d, v4.2d
        EndOf_LoopTxTy4_CompVMathStatsNormalize2DHartley_64f_Asm_NEON64:
        ## EndOf_LoopTxTy4_CompVMathStatsNormalize2DHartley_64f_Asm_NEON64 ##


    #########################################################################
    # .if (i < numPoints_ - 1)
    #########################################################################
    lsr i, r9, #1
    cbz i, EndOf_LoopTxTy2_CompVMathStatsNormalize2DHartley_64f_Asm_NEON64
    LoopTxTy2_CompVMathStatsNormalize2DHartley_64f_Asm_NEON64:
        ldr q0, [r14], #16
        ldr q4, [r15], #16
        fadd vecTx.2d, vecTx.2d, v0.2d
        fadd vecTy.2d, vecTy.2d, v4.2d
        EndOf_LoopTxTy2_CompVMathStatsNormalize2DHartley_64f_Asm_NEON64:
        ## EndOf_LoopTxTy2_CompVMathStatsNormalize2DHartley_64f_Asm_NEON64 ##

    mov d23, vecTx.d[1]
    mov d24, vecTy.d[1]
    fadd d27, d27, d23 // v27 = vecTx
    fadd d28, d28, d24 // v28 = vecTy

    #########################################################################
    # .if (numPoints_ & 1)
    #########################################################################
    ands i, numPoints, #1
    beq EndOf_LoopTxTy1_CompVMathStatsNormalize2DHartley_64f_Asm_NEON64
    LoopTxTy1_CompVMathStatsNormalize2DHartley_64f_Asm_NEON64:
        ld1 {v0.d}[0], [r14]
        ld1 {v4.d}[0], [r15]
        fadd d27, d27, d0
        fadd d28, d28, d4
        EndOf_LoopTxTy1_CompVMathStatsNormalize2DHartley_64f_Asm_NEON64:
        ## EndOf_LoopTxTy1_CompVMathStatsNormalize2DHartley_64f_Asm_NEON64 ##

    dup vecTx.2d, v27.d[0]
    dup vecTy.2d, v28.d[0]

    fmul vecTx.2d, vecTx.2d, vecOneOverNumPoints.2d
    fmul vecTy.2d, vecTy.2d, vecOneOverNumPoints.2d

    #########################################################################
    # for (i = 0; i < numPoints_ - 3; i += 4)
    #########################################################################
    lsr i, numPoints, #2
    cbz i, EndOf_LoopMagnitude4_CompVMathStatsNormalize2DHartley_64f_Asm_NEON64
    LoopMagnitude4_CompVMathStatsNormalize2DHartley_64f_Asm_NEON64:
        subs i, i, #1
        ldp q0, q1, [x], #32
        ldp q4, q5, [y], #32
        fsub v0.2d, v0.2d, vecTx.2d
        fsub v4.2d, v4.2d, vecTy.2d
        fsub v1.2d, v1.2d, vecTx.2d
        fsub v5.2d, v5.2d, vecTy.2d
        fmul v0.2d, v0.2d, v0.2d
        fmul v4.2d, v4.2d, v4.2d
        fmul v1.2d, v1.2d, v1.2d
        fmul v5.2d, v5.2d, v5.2d
        fadd v0.2d, v0.2d, v4.2d
        fadd v1.2d, v1.2d, v5.2d
        fsqrt v0.2d, v0.2d
        fsqrt v1.2d, v1.2d
        fadd vecMagnitude.2d, vecMagnitude.2d, v0.2d
        fadd vecMagnitude.2d, vecMagnitude.2d, v1.2d
        bne LoopMagnitude4_CompVMathStatsNormalize2DHartley_64f_Asm_NEON64
        EndOf_LoopMagnitude4_CompVMathStatsNormalize2DHartley_64f_Asm_NEON64:
        ## EndOf_LoopMagnitude4_CompVMathStatsNormalize2DHartley_64f_Asm_NEON64 ##

    #########################################################################
    # .if (i < numPoints_ - 1)
    #########################################################################
    lsr i, r9, #1
    cbz i, EndOf_LoopMagnitude2_CompVMathStatsNormalize2DHartley_64f_Asm_NEON64
    LoopMagnitude2_CompVMathStatsNormalize2DHartley_64f_Asm_NEON64:
        ldr q0, [x], #16
        ldr q4, [y], #16
        fsub v0.2d, v0.2d, vecTx.2d
        fsub v4.2d, v4.2d, vecTy.2d
        fmul v0.2d, v0.2d, v0.2d
        fmul v4.2d, v4.2d, v4.2d
        fadd v0.2d, v0.2d, v4.2d
        fsqrt v0.2d, v0.2d
        fadd vecMagnitude.2d, vecMagnitude.2d, v0.2d
        EndOf_LoopMagnitude2_CompVMathStatsNormalize2DHartley_64f_Asm_NEON64:
        ## EndOf_LoopMagnitude2_CompVMathStatsNormalize2DHartley_64f_Asm_NEON64 ##

    mov d23, vecMagnitude.d[1]
    fadd d29, d29, d23 // v29 = vecMagnitude

    #########################################################################
    # .if (numPoints_ & 1)
    #########################################################################
    ands i, numPoints, #1
    beq EndOf_LoopMagnitude1_CompVMathStatsNormalize2DHartley_64f_Asm_NEON64
    LoopMagnitude1_CompVMathStatsNormalize2DHartley_64f_Asm_NEON64:
        ld1 {v0.d}[0], [x]
        ld1 {v4.d}[0], [y]
        fsub d0, d0, d27  // v27 = vecTx
        fsub d4, d4, d28 // v28 = vecTy
        fmul d0, d0, d0
        fmul d4, d4, d4
        fadd d0, d0, d4
        fsqrt d0, d0
        fadd d29, d29, d0
        EndOf_LoopMagnitude1_CompVMathStatsNormalize2DHartley_64f_Asm_NEON64:
        ## EndOf_LoopMagnitude1_CompVMathStatsNormalize2DHartley_64f_Asm_NEON64 ##


    dup vecMagnitude.2d, v29.d[0]
    fmul vecMagnitude.2d, vecMagnitude.2d, vecOneOverNumPoints.2d
    fdiv vecMagnitude.2d, vecSqrt2.2d, vecMagnitude.2d
    
    st1 { vecTx.d }[0], [tx1]
    st1 { vecTy.d }[0], [ty1]
    st1 { vecMagnitude.d }[0], [sf1]
    
    .unreq x
	.unreq y
	.unreq numPoints
	.unreq tx1
    .unreq ty1
    .unreq sf1

    .unreq i

    .unreq vecSqrt2
    .unreq vecTx
    .unreq vecTy
    .unreq vecMagnitude
    .unreq vecOneOverNumPoints

    COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN



#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) const compv_float64_t* x
# arg(1) -> COMPV_ALIGNED(NEON) const compv_float64_t* y
# arg(2) -> compv_uscalar_t numPoints
# arg(3) -> compv_float64_t* tx1
# arg(4) -> compv_float64_t* ty1
# arg(5) -> compv_float64_t* sf1
COMPV_GAS_FUNCTION_DECLARE CompVMathStatsNormalize2DHartley_4_64f_Asm_NEON64
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Load arguments ##
	x .req r0
	y .req r1
	numPoints .req r2
	tx1 .req r3
    ty1 .req r4
    sf1 .req r5

    i .req r6

    vecSqrt2 .req v26
    vecTx .req v27
    vecTy .req v28
    vecMagnitude .req v29
    vecOneOverNumPoints .req v30
    mov r11, #2
    ucvtf d20, numPoints
    ucvtf d21, r11
    fmov vecOneOverNumPoints.2d, #1.00000000
    dup vecSqrt2.2d, v21.d[0]
    dup v20.2d, v20.d[0]
    fdiv vecOneOverNumPoints.2d, vecOneOverNumPoints.2d, v20.2d
    fsqrt vecSqrt2.2d, vecSqrt2.2d
    ldp q0, q1, [x]
    fadd vecTx.2d, v0.2d, v1.2d
    ldp q4, q5, [y]
    fadd vecTy.2d, v4.2d, v5.2d
    mov d23, vecTx.d[1]
    mov d24, vecTy.d[1]
    fadd d27, d27, d23 // v27 = vecTx
    fadd d28, d28, d24 // v28 = vecTy
    fmul d27, d27, d30 // v30 = vecOneOverNumPoints
    fmul d28, d28, d30 // v30 = vecOneOverNumPoints
    dup vecTx.2d, v27.d[0]
    dup vecTy.2d, v28.d[0]
    ldp q0, q1, [x]
    ldp q4, q5, [y]
    fsub v0.2d, v0.2d, vecTx.2d
    fsub v4.2d, v4.2d, vecTy.2d
    fsub v1.2d, v1.2d, vecTx.2d
    fsub v5.2d, v5.2d, vecTy.2d
    fmul v0.2d, v0.2d, v0.2d
    fmul v4.2d, v4.2d, v4.2d
    fmul v1.2d, v1.2d, v1.2d
    fmul v5.2d, v5.2d, v5.2d
    fadd v0.2d, v0.2d, v4.2d
    fadd v1.2d, v1.2d, v5.2d
    fsqrt vecMagnitude.2d, v0.2d
    fsqrt v1.2d, v1.2d
    fadd vecMagnitude.2d, vecMagnitude.2d, v1.2d
    mov d23, vecMagnitude.d[1]
    fadd d29, d29, d23 // v29 = vecMagnitude
    fmul d29, d29, d30 // v30 = vecOneOverNumPoints
    fdiv d29, d26, d29  // v26 = vecSqrt2
    st1 { vecTx.d }[0], [tx1]
    st1 { vecTy.d }[0], [ty1]
    st1 { vecMagnitude.d }[0], [sf1]
    
    .unreq x
	.unreq y
	.unreq numPoints
	.unreq tx1
    .unreq ty1
    .unreq sf1

    .unreq i

    .unreq vecSqrt2
    .unreq vecTx
    .unreq vecTy
    .unreq vecMagnitude
    .unreq vecOneOverNumPoints

    COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN



#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) const compv_float64_t* aX_h
# arg(1) -> COMPV_ALIGNED(NEON) const compv_float64_t* aY_h
# arg(2) -> COMPV_ALIGNED(NEON) const compv_float64_t* aZ_h
# arg(3) -> COMPV_ALIGNED(NEON) const compv_float64_t* bX
# arg(4) -> COMPV_ALIGNED(NEON) const compv_float64_t* bY
# arg(5) -> COMPV_ALIGNED(NEON) compv_float64_t* mse
# arg(6) -> compv_uscalar_t numPoints
COMPV_GAS_FUNCTION_DECLARE CompVMathStatsMSE2DHomogeneous_64f_Asm_NEON64
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

    ## Load arguments ##
	aX_h .req r0
	aY_h .req r1
	aZ_h .req r2
	bX .req r3
    bY .req r4
    mse .req r5
    numPoints .req r6

    i .req r7

    vecOne .req v30
    fmov vecOne.2d, #1.00000000

    #########################################################################
    # for (i = 0; i < numPoints_ - 7; i += 8)
    #########################################################################
    lsr i, numPoints, #3
    cbz i, EndOf_LoopNumPoints8_CompVMathStatsMSE2DHomogeneous_64f_Asm_NEON64
    LoopNumPoints8_CompVMathStatsMSE2DHomogeneous_64f_Asm_NEON64:
        subs i, i, #1
        ldp q0, q1, [aZ_h], #32
        fdiv v0.2d, vecOne.2d, v0.2d
        fdiv v1.2d, vecOne.2d, v1.2d
        ldp q2, q3, [aZ_h], #32
        fdiv v2.2d, vecOne.2d, v2.2d
        fdiv v3.2d, vecOne.2d, v3.2d
        ldp q4, q5, [aX_h], #32
        ldp q6, q7, [aY_h], #32
        fmul v4.2d, v4.2d, v0.2d
        fmul v5.2d, v5.2d, v1.2d
        fmul v6.2d, v6.2d, v0.2d
        fmul v7.2d, v7.2d, v1.2d
        ldp q8, q9, [aX_h], #32
        ldp q10, q11, [aY_h], #32
        fmul v8.2d, v8.2d, v2.2d
        fmul v9.2d, v9.2d, v3.2d
        fmul v10.2d, v10.2d, v2.2d
        fmul v11.2d, v11.2d, v3.2d
        ldp q0, q1, [bX], #32
        ldp q2, q3, [bY], #32
        fsub v4.2d, v4.2d, v0.2d
        fsub v5.2d, v5.2d, v1.2d
        fsub v6.2d, v6.2d, v2.2d
        fsub v7.2d, v7.2d, v3.2d
        ldp q0, q1, [bX], #32
        ldp q2, q3, [bY], #32
        fsub v8.2d, v8.2d, v0.2d
        fsub v9.2d, v9.2d, v1.2d
        fsub v10.2d, v10.2d, v2.2d
        fsub v11.2d, v11.2d, v3.2d
        fmul v4.2d, v4.2d, v4.2d
        fmul v6.2d, v6.2d, v6.2d
        fmul v5.2d, v5.2d, v5.2d
        fmul v7.2d, v7.2d, v7.2d
        fmul v8.2d, v8.2d, v8.2d
        fmul v10.2d, v10.2d, v10.2d
        fmul v9.2d, v9.2d, v9.2d
        fmul v11.2d, v11.2d, v11.2d
        fadd v4.2d, v4.2d, v6.2d
        fadd v5.2d, v5.2d, v7.2d
        fadd v8.2d, v8.2d, v10.2d
        fadd v9.2d, v9.2d, v11.2d
        stp q4, q5, [mse], #32
        stp q8, q9, [mse], #32
        bne LoopNumPoints8_CompVMathStatsMSE2DHomogeneous_64f_Asm_NEON64
        EndOf_LoopNumPoints8_CompVMathStatsMSE2DHomogeneous_64f_Asm_NEON64:
        ## EndOf_LoopNumPoints8_CompVMathStatsMSE2DHomogeneous_64f_Asm_NEON64 ##

    and r8, numPoints, #7 // modulo 8
    and r9, numPoints, #3 // modulo 4


    #########################################################################
    # .if (i < numPoints_ - 3)
    #########################################################################
    lsr i, r8, #2
    cbz i, EndOf_LoopNumPoints4_CompVMathStatsMSE2DHomogeneous_64f_Asm_NEON64
    LoopNumPoints4_CompVMathStatsMSE2DHomogeneous_64f_Asm_NEON64:
        ldp q0, q1, [aZ_h], #32
        fdiv v0.2d, vecOne.2d, v0.2d
        fdiv v1.2d, vecOne.2d, v1.2d
        ldp q4, q5, [aX_h], #32
        ldp q6, q7, [aY_h], #32
        fmul v4.2d, v4.2d, v0.2d
        fmul v5.2d, v5.2d, v1.2d
        fmul v6.2d, v6.2d, v0.2d
        fmul v7.2d, v7.2d, v1.2d
        ldp q0, q1, [bX], #32
        ldp q2, q3, [bY], #32
        fsub v4.2d, v4.2d, v0.2d
        fsub v5.2d, v5.2d, v1.2d
        fsub v6.2d, v6.2d, v2.2d
        fsub v7.2d, v7.2d, v3.2d
        fmul v4.2d, v4.2d, v4.2d
        fmul v6.2d, v6.2d, v6.2d
        fmul v5.2d, v5.2d, v5.2d
        fmul v7.2d, v7.2d, v7.2d
        fadd v4.2d, v4.2d, v6.2d
        fadd v5.2d, v5.2d, v7.2d
        stp q4, q5, [mse], #32
        EndOf_LoopNumPoints4_CompVMathStatsMSE2DHomogeneous_64f_Asm_NEON64:
        ## EndOf_LoopNumPoints4_CompVMathStatsMSE2DHomogeneous_64f_Asm_NEON64 ##

    #########################################################################
    # .if (i < numPoints_ - 1)
    #########################################################################
    lsr i, r9, #1
    cbz i, EndOf_LoopNumPoints2_CompVMathStatsMSE2DHomogeneous_64f_Asm_NEON64
    LoopNumPoints2_CompVMathStatsMSE2DHomogeneous_64f_Asm_NEON64:
        ldr q0, [aZ_h], #16
        fdiv v0.2d, vecOne.2d, v0.2d        
        ldr q4, [aX_h], #16
        ldr q6, [aY_h], #16
        fmul v4.2d, v4.2d, v0.2d
        fmul v6.2d, v6.2d, v0.2d
        ldr q0, [bX], #16
        ldr q2, [bY], #16
        fsub v4.2d, v4.2d, v0.2d
        fsub v6.2d, v6.2d, v2.2d
        fmul v4.2d, v4.2d, v4.2d
        fmul v6.2d, v6.2d, v6.2d
        fadd v4.2d, v4.2d, v6.2d
        str q4, [mse], #16
        EndOf_LoopNumPoints2_CompVMathStatsMSE2DHomogeneous_64f_Asm_NEON64:
        ## EndOf_LoopNumPoints2_CompVMathStatsMSE2DHomogeneous_64f_Asm_NEON64 ##

    #########################################################################
    # .if (numPoints_ & 1)
    #########################################################################
    ands i, numPoints, #1
    beq EndOf_LoopNumPoints1_CompVMathStatsMSE2DHomogeneous_64f_Asm_NEON64
    LoopNumPoints1_CompVMathStatsMSE2DHomogeneous_64f_Asm_NEON64:
        ld1 {v0.d}[0], [aZ_h]
        fdiv d0, d30, d0 // v30 = vecOne
        ld1 {v4.d}[0], [aX_h]
        ld1 {v6.d}[0], [aY_h]
        fmul d4, d4, d0
        fmul d6, d6, d0
        ld1 {v0.d}[0], [bX]
        ld1 {v2.d}[0], [bY]
        fsub d4, d4, d0
        fsub d6, d6, d2
        fmul d4, d4, d4
        fmul d6, d6, d6
        fadd d4, d4, d6
        st1 {v4.d}[0], [mse]
        EndOf_LoopNumPoints1_CompVMathStatsMSE2DHomogeneous_64f_Asm_NEON64:
        ## EndOf_LoopNumPoints1_CompVMathStatsMSE2DHomogeneous_64f_Asm_NEON64 ##

    .unreq aX_h
	.unreq aY_h
	.unreq aZ_h
	.unreq bX
    .unreq bY
    .unreq mse
    .unreq numPoints

    .unreq i

    .unreq vecOne

    COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) const compv_float64_t* aX_h
# arg(1) -> COMPV_ALIGNED(NEON) const compv_float64_t* aY_h
# arg(2) -> COMPV_ALIGNED(NEON) const compv_float64_t* aZ_h
# arg(3) -> COMPV_ALIGNED(NEON) const compv_float64_t* bX
# arg(4) -> COMPV_ALIGNED(NEON) const compv_float64_t* bY
# arg(5) -> COMPV_ALIGNED(NEON) compv_float64_t* mse
# arg(6) -> compv_uscalar_t numPoints
COMPV_GAS_FUNCTION_DECLARE CompVMathStatsMSE2DHomogeneous_4_64f_Asm_NEON64
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

    ## Load arguments ##
	aX_h .req r0
	aY_h .req r1
	aZ_h .req r2
	bX .req r3
    bY .req r4
    mse .req r5
    numPoints .req r6

    i .req r7

    vecOne .req v30

    fmov vecOne.2d, #1.00000000
    ldp q0, q1, [aZ_h]
    fdiv v0.2d, vecOne.2d, v0.2d
    fdiv v1.2d, vecOne.2d, v1.2d
    ldp q4, q5, [aX_h]
    ldp q6, q7, [aY_h]
    fmul v4.2d, v4.2d, v0.2d
    fmul v5.2d, v5.2d, v1.2d
    fmul v6.2d, v6.2d, v0.2d
    fmul v7.2d, v7.2d, v1.2d
    ldp q0, q1, [bX]
    ldp q2, q3, [bY]
    fsub v4.2d, v4.2d, v0.2d
    fsub v5.2d, v5.2d, v1.2d
    fsub v6.2d, v6.2d, v2.2d
    fsub v7.2d, v7.2d, v3.2d
    fmul v4.2d, v4.2d, v4.2d
    fmul v6.2d, v6.2d, v6.2d
    fmul v5.2d, v5.2d, v5.2d
    fmul v7.2d, v7.2d, v7.2d
    fadd v4.2d, v4.2d, v6.2d
    fadd v5.2d, v5.2d, v7.2d
    stp q4, q5, [mse]

    .unreq aX_h
	.unreq aY_h
	.unreq aZ_h
	.unreq bX
    .unreq bY
    .unreq mse
    .unreq numPoints

    .unreq i

    .unreq vecOne

    COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) const compv_float64_t* data
# arg(1) -> compv_uscalar_t count
# arg(2) -> const compv_float64_t* mean1
# arg(3) -> compv_float64_t* var1
COMPV_GAS_FUNCTION_DECLARE CompVMathStatsVariance_64f_Asm_NEON64
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

    ## Set arguments ##
	data .req r0
	count .req r1
	mean1 .req r2
	var1 .req r3

    i .req r4

    vecMean .req v28
    vecVar .req v29
    #define vecCountMinus1 d30

    movi vecVar.2d, #0
    sub r10, count, #1
    ucvtf vecCountMinus1, r10
    ld1 { vecMean.8b }, [mean1]
    dup vecMean.2d, vecMean.d[0]

    #########################################################################
    # for (i = 0; i < count - 7; i += 8)
    #########################################################################
    lsr i, count, #3
    cbz i, EndOf_LoopNumPoints8_CompVMathStatsVariance_64f_Asm_NEON64
    LoopNumPoints8_CompVMathStatsVariance_64f_Asm_NEON64:
        subs i, i, #1
        ldp q0, q1, [data], #32
        ldp q2, q3, [data], #32
        fsub v0.2d, v0.2d, vecMean.2d
        fsub v1.2d, v1.2d, vecMean.2d
        fsub v2.2d, v2.2d, vecMean.2d
        fsub v3.2d, v3.2d, vecMean.2d
        fmul v0.2d, v0.2d, v0.2d
        fmul v1.2d, v1.2d, v1.2d
        fmul v2.2d, v2.2d, v2.2d
        fmul v3.2d, v3.2d, v3.2d
        fadd v0.2d, v0.2d, v1.2d
        fadd v2.2d, v2.2d, v3.2d
        fadd vecVar.2d, vecVar.2d, v0.2d
        fadd vecVar.2d, vecVar.2d, v2.2d
        bne LoopNumPoints8_CompVMathStatsVariance_64f_Asm_NEON64
        EndOf_LoopNumPoints8_CompVMathStatsVariance_64f_Asm_NEON64:
        ## EndOf_LoopNumPoints8_CompVMathStatsVariance_64f_Asm_NEON64 ##

    and r8, count, #7 // modulo 8
    and r9, count, #3 // modulo 4

    #########################################################################
    # .if (i < count - 3)
    #########################################################################
    lsr i, r8, #2
    cbz i, EndOf_LoopNumPoints4_CompVMathStatsVariance_64f_Asm_NEON64
    LoopNumPoints4_CompVMathStatsVariance_64f_Asm_NEON64:
        ldp q0, q1, [data], #32
        fsub v0.2d, v0.2d, vecMean.2d
        fsub v1.2d, v1.2d, vecMean.2d
        fmul v0.2d, v0.2d, v0.2d
        fmul v1.2d, v1.2d, v1.2d
        fadd v0.2d, v0.2d, v1.2d
        fadd vecVar.2d, vecVar.2d, v0.2d
        EndOf_LoopNumPoints4_CompVMathStatsVariance_64f_Asm_NEON64:
        ## EndOf_LoopNumPoints4_CompVMathStatsVariance_64f_Asm_NEON64 ##

    #########################################################################
    # .if (i < count - 1)
    #########################################################################
    lsr i, r9, #1
    cbz i, EndOf_LoopNumPoints2_CompVMathStatsVariance_64f_Asm_NEON64
    LoopNumPoints2_CompVMathStatsVariance_64f_Asm_NEON64:
        ldr q0, [data], #16
        fsub v0.2d, v0.2d, vecMean.2d
        fmul v0.2d, v0.2d, v0.2d
        fadd vecVar.2d, vecVar.2d, v0.2d
        EndOf_LoopNumPoints2_CompVMathStatsVariance_64f_Asm_NEON64:
        ## EndOf_LoopNumPoints2_CompVMathStatsVariance_64f_Asm_NEON64 ##

    mov d23, vecVar.d[1]
    fadd d29, d29, d23 // v29 = vecVar

    #########################################################################
    # .if (count & 1)
    #########################################################################
    ands i, count, #1
    beq EndOf_LoopNumPoints1_CompVMathStatsVariance_64f_Asm_NEON64
    LoopNumPoints1_CompVMathStatsVariance_64f_Asm_NEON64:
        ld1 {v0.d}[0], [data]
        fsub d0, d0, d28 // v28 = vecMean
        fmul d0, d0, d0
        fadd d29, d29, d0
        EndOf_LoopNumPoints1_CompVMathStatsVariance_64f_Asm_NEON64:
        ## EndOf_LoopNumPoints1_CompVMathStatsVariance_64f_Asm_NEON64 ##

    fdiv d29, d29, vecCountMinus1
    st1 { v29.d }[0], [var1]

    .unreq data
	.unreq count
	.unreq mean1
	.unreq var1

    .unreq i

    .unreq vecMean
    .unreq vecVar
    #undef vecCountMinus1

    COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#endif /* defined(__aarch64__) */
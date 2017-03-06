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
@ arg(0) -> const COMPV_ALIGNED(NEON) compv_float64_t* x
@ arg(1) -> const COMPV_ALIGNED(NEON) compv_float64_t* y
@ arg(2) -> compv_uscalar_t numPoints
@ arg(3) -> compv_float64_t* tx1
@ arg(4) -> compv_float64_t* ty1
@ arg(5) -> compv_float64_t* sf1
COMPV_GAS_FUNCTION_DECLARE CompVMathStatsNormalize2DHartley_64f_Asm_NEON32
    COMPV_GAS_FUNCTION_PROLOG
    COMPV_GAS_SHADOW_ARGS_TO_STACK 6
	COMPV_GAS_SAVE_NEON_REGS

	@@ Load arguments @@
    ldm_args r0-r5
	x .req r0
	y .req r1
	numPoints .req r2
	tx1 .req r3
    ty1 .req r4
    sf1 .req r5

    i .req r6

    vecSqrt2 .req q13y
    vecTx .req q14x
    vecTy .req q14y
    vecMagnitude .req q15x
    vecOneOverNumPoints .req q15y

    veor vecTx, vecTx, vecTx
    veor vecTy, vecTy, vecTy
    veor vecMagnitude, vecMagnitude, vecMagnitude

    vmov s11, numPoints
    vcvt.f64.u32 q0x, s11
    vmov.f64 vecOneOverNumPoints, #1.000000e+00
    vmov.f64 vecSqrt2, #2.000000e+00
    vdiv.f64 vecOneOverNumPoints, vecOneOverNumPoints, q0x
    vsqrt.f64 vecSqrt2, vecSqrt2
    
    
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (i = 0; i < numPoints_ - 7; i += 8)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    lsrs i, numPoints, #3
    beq EndOf_LoopTxTy8_CompVMathStatsNormalize2DHartley_64f_Asm_NEON32
    LoopTxTy8_CompVMathStatsNormalize2DHartley_64f_Asm_NEON32:
        subs i, i, #1
        vld1.64 { q0, q1 }, [x :128]!
        vld1.64 { q2, q3 }, [x :128]!
        vadd.f64 q0x, q0x, q0y
        vadd.f64 q1x, q1x, q1y
        vadd.f64 q2x, q2x, q2y
        vadd.f64 q3x, q3x, q3y
        vld1.64 { q4, q5 }, [y :128]!
        vld1.64 { q6, q7 }, [y :128]!
        vadd.f64 q4x, q4x, q4y
        vadd.f64 q5x, q5x, q5y
        vadd.f64 q6x, q6x, q6y
        vadd.f64 q7x, q7x, q7y
        vadd.f64 q0x, q0x, q1x
        vadd.f64 q2x, q2x, q3x
        vadd.f64 q4x, q4x, q5x
        vadd.f64 q6x, q6x, q7x
        vadd.f64 q0x, q0x, q2x
        vadd.f64 q4x, q4x, q6x
        vadd.f64 vecTx, vecTx, q0x
        vadd.f64 vecTy, vecTy, q4x
        bne LoopTxTy8_CompVMathStatsNormalize2DHartley_64f_Asm_NEON32
        EndOf_LoopTxTy8_CompVMathStatsNormalize2DHartley_64f_Asm_NEON32:
        @@ EndOf_LoopTxTy8_CompVMathStatsNormalize2DHartley_64f_Asm_NEON32 @@

    and r8, numPoints, #7 @ modulo 8
    and r9, numPoints, #3 @ modulo 4

    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ .if (i < numPoints_ - 3)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    lsrs i, r8, #2
    beq EndOf_LoopTxTy4_CompVMathStatsNormalize2DHartley_64f_Asm_NEON32
    LoopTxTy4_CompVMathStatsNormalize2DHartley_64f_Asm_NEON32:
        vld1.64 { q0, q1 }, [x :128]!
        vadd.f64 q0x, q0x, q0y
        vadd.f64 q1x, q1x, q1y
        vld1.64 { q4, q5 }, [y :128]!
        vadd.f64 q4x, q4x, q4y
        vadd.f64 q5x, q5x, q5y
        vadd.f64 q0x, q0x, q1x
        vadd.f64 q4x, q4x, q5x
        vadd.f64 vecTx, vecTx, q0x
        vadd.f64 vecTy, vecTy, q4x
        EndOf_LoopTxTy4_CompVMathStatsNormalize2DHartley_64f_Asm_NEON32:
        @@ EndOf_LoopTxTy4_CompVMathStatsNormalize2DHartley_64f_Asm_NEON32 @@

    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ .if (i < numPoints_ - 1)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    lsrs i, r9, #1
    beq EndOf_LoopTxTy2_CompVMathStatsNormalize2DHartley_64f_Asm_NEON32
    LoopTxTy2_CompVMathStatsNormalize2DHartley_64f_Asm_NEON32:
        vld1.64 { q0 }, [x :128]!
        vadd.f64 q0x, q0x, q0y
        vld1.64 { q4 }, [y :128]!
        vadd.f64 q4x, q4x, q4y
        vadd.f64 vecTx, vecTx, q0x
        vadd.f64 vecTy, vecTy, q4x
        EndOf_LoopTxTy2_CompVMathStatsNormalize2DHartley_64f_Asm_NEON32:
        @@ EndOf_LoopTxTy2_CompVMathStatsNormalize2DHartley_64f_Asm_NEON32 @@

    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ .if (numPoints_ & 1)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    ands i, numPoints, #1
    beq EndOf_LoopTxTy1_CompVMathStatsNormalize2DHartley_64f_Asm_NEON32
    LoopTxTy1_CompVMathStatsNormalize2DHartley_64f_Asm_NEON32:
        vld1.64 { q0x }, [x :64]
        vld1.64 { q4x }, [y :64]
        vadd.f64 vecTx, vecTx, q0x
        vadd.f64 vecTy, vecTy, q4x
        EndOf_LoopTxTy1_CompVMathStatsNormalize2DHartley_64f_Asm_NEON32:
        @@ EndOf_LoopTxTy1_CompVMathStatsNormalize2DHartley_64f_Asm_NEON32 @@


    vmul.f64 vecTx, vecTx, vecOneOverNumPoints
    vmul.f64 vecTy, vecTy, vecOneOverNumPoints

    @ restore x and y @
    ldr_arg 0, x
    ldr_arg 1, y

    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (i = 0; i < numPoints_ - 3; i += 4)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    lsrs i, numPoints, #2
    beq EndOf_LoopMagnitude4_CompVMathStatsNormalize2DHartley_64f_Asm_NEON32
    LoopMagnitude4_CompVMathStatsNormalize2DHartley_64f_Asm_NEON32:
        subs i, i, #1
        vld1.64 { q0, q1 }, [x :128]!
        vld1.64 { q4, q5 }, [y :128]!
        vsub.f64 q0x, q0x, vecTx
        vsub.f64 q4x, q4x, vecTy
        vsub.f64 q0y, q0y, vecTx
        vsub.f64 q4y, q4y, vecTy
        vsub.f64 q1x, q1x, vecTx
        vsub.f64 q5x, q5x, vecTy
        vsub.f64 q1y, q1y, vecTx
        vsub.f64 q5y, q5y, vecTy
        vmul.f64 q0x, q0x, q0x
        vmul.f64 q4x, q4x, q4x
        vmul.f64 q0y, q0y, q0y
        vmul.f64 q4y, q4y, q4y
        vmul.f64 q1x, q1x, q1x
        vmul.f64 q5x, q5x, q5x
        vmul.f64 q1y, q1y, q1y
        vmul.f64 q5y, q5y, q5y
        vadd.f64 q0x, q0x, q4x
        vadd.f64 q0y, q0y, q4y
        vadd.f64 q1x, q1x, q5x
        vadd.f64 q1y, q1y, q5y
        vsqrt.f64 q0x, q0x
        vsqrt.f64 q0y, q0y
        vsqrt.f64 q1x, q1x
        vsqrt.f64 q1y, q1y
        vadd.f64 q0x, q0x, q0y
        vadd.f64 q1x, q1x, q1y
        vadd.f64 vecMagnitude, vecMagnitude, q0x
        vadd.f64 vecMagnitude, vecMagnitude, q1x
        bne LoopMagnitude4_CompVMathStatsNormalize2DHartley_64f_Asm_NEON32
        EndOf_LoopMagnitude4_CompVMathStatsNormalize2DHartley_64f_Asm_NEON32:
        @@ EndOf_LoopMagnitude4_CompVMathStatsNormalize2DHartley_64f_Asm_NEON32 @@


    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ .if (i < numPoints_ - 1)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    lsrs i, r9, #1
    beq EndOf_LoopLoopMagnitude2_CompVMathStatsNormalize2DHartley_64f_Asm_NEON32
    LoopLoopMagnitude2_CompVMathStatsNormalize2DHartley_64f_Asm_NEON32:
        subs i, i, #1
        vld1.64 { q0 }, [x :128]!
        vld1.64 { q4 }, [y :128]!
        vsub.f64 q0x, q0x, vecTx
        vsub.f64 q4x, q4x, vecTy
        vsub.f64 q0y, q0y, vecTx
        vsub.f64 q4y, q4y, vecTy
        vmul.f64 q0x, q0x, q0x
        vmul.f64 q4x, q4x, q4x
        vmul.f64 q0y, q0y, q0y
        vmul.f64 q4y, q4y, q4y
        vadd.f64 q0x, q0x, q4x
        vadd.f64 q0y, q0y, q4y
        vsqrt.f64 q0x, q0x
        vsqrt.f64 q0y, q0y
        vadd.f64 vecMagnitude, vecMagnitude, q0x
        vadd.f64 vecMagnitude, vecMagnitude, q0y
        EndOf_LoopLoopMagnitude2_CompVMathStatsNormalize2DHartley_64f_Asm_NEON32:
        @@ EndOf_LoopLoopMagnitude2_CompVMathStatsNormalize2DHartley_64f_Asm_NEON32 @@


    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ .if (numPoints_ & 1)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    ands i, numPoints, #1
    beq EndOf_LoopMagnitude1_CompVMathStatsNormalize2DHartley_64f_Asm_NEON32
    LoopMagnitude1_CompVMathStatsNormalize2DHartley_64f_Asm_NEON32:
        vld1.64 { q0x }, [x :64]
        vld1.64 { q4x }, [y :64]
        vsub.f64 q0x, q0x, vecTx
        vsub.f64 q4x, q4x, vecTy
        vmul.f64 q0x, q0x, q0x
        vmul.f64 q4x, q4x, q4x
        vadd.f64 q0x, q0x, q4x
        vsqrt.f64 q0x, q0x
        vadd.f64 vecMagnitude, vecMagnitude, q0x
        EndOf_LoopMagnitude1_CompVMathStatsNormalize2DHartley_64f_Asm_NEON32:
        @@ EndOf_LoopMagnitude1_CompVMathStatsNormalize2DHartley_64f_Asm_NEON32 @@

    vmul.f64 vecMagnitude, vecMagnitude, vecOneOverNumPoints
    vdiv.f64 vecMagnitude, vecSqrt2, vecMagnitude
    
    vst1.64 { vecTx }, [tx1 :64]
    vst1.64 { vecTy }, [ty1 :64]
    vst1.64 { vecMagnitude }, [sf1 :64]
    
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
    COMPV_GAS_UNSHADOW_ARGS 6
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> const COMPV_ALIGNED(NEON) compv_float64_t* x
@ arg(1) -> const COMPV_ALIGNED(NEON) compv_float64_t* y
@ arg(2) -> compv_uscalar_t numPoints
@ arg(3) -> compv_float64_t* tx1
@ arg(4) -> compv_float64_t* ty1
@ arg(5) -> compv_float64_t* sf1
COMPV_GAS_FUNCTION_DECLARE CompVMathStatsNormalize2DHartley_4_64f_Asm_NEON32
    COMPV_GAS_FUNCTION_PROLOG
    COMPV_GAS_SHADOW_ARGS_TO_STACK 6
	COMPV_GAS_SAVE_NEON_REGS

	@@ Load arguments @@
    ldm_args r0-r5
	x .req r0
	y .req r1
	numPoints .req r2
	tx1 .req r3
    ty1 .req r4
    sf1 .req r5

    i .req r6

    vecSqrt2 .req q13y
    vecTx .req q14x
    vecTy .req q14y
    vecMagnitude .req q15x
    vecOneOverNumPoints .req q15y

    vmov s11, numPoints
    vcvt.f64.u32 q0x, s11
    vmov.f64 vecOneOverNumPoints, #1.000000e+00
    vmov.f64 vecSqrt2, #2.000000e+00
    vdiv.f64 vecOneOverNumPoints, vecOneOverNumPoints, q0x
    vsqrt.f64 vecSqrt2, vecSqrt2
    vld1.64 { q0, q1 }, [x :128]!
    vadd.f64 q0x, q0x, q0y
    vadd.f64 q1x, q1x, q1y
    vld1.64 { q4, q5 }, [y :128]!
    vadd.f64 q4x, q4x, q4y
    vadd.f64 q5x, q5x, q5y
    vadd.f64 vecTx, q0x, q1x
    vadd.f64 vecTy, q4x, q5x
    vmul.f64 vecTx, vecTx, vecOneOverNumPoints
    vmul.f64 vecTy, vecTy, vecOneOverNumPoints
    ldr_arg 0, x
    ldr_arg 1, y    
    vld1.64 { q0, q1 }, [x :128]!
    vld1.64 { q4, q5 }, [y :128]!
    vsub.f64 q0x, q0x, vecTx
    vsub.f64 q4x, q4x, vecTy
    vsub.f64 q0y, q0y, vecTx
    vsub.f64 q4y, q4y, vecTy
    vsub.f64 q1x, q1x, vecTx
    vsub.f64 q5x, q5x, vecTy
    vsub.f64 q1y, q1y, vecTx
    vsub.f64 q5y, q5y, vecTy
    vmul.f64 q0x, q0x, q0x
    vmul.f64 q4x, q4x, q4x
    vmul.f64 q0y, q0y, q0y
    vmul.f64 q4y, q4y, q4y
    vmul.f64 q1x, q1x, q1x
    vmul.f64 q5x, q5x, q5x
    vmul.f64 q1y, q1y, q1y
    vmul.f64 q5y, q5y, q5y
    vadd.f64 q0x, q0x, q4x
    vadd.f64 q0y, q0y, q4y
    vadd.f64 q1x, q1x, q5x
    vadd.f64 q1y, q1y, q5y
    vsqrt.f64 q0x, q0x
    vsqrt.f64 q0y, q0y
    vsqrt.f64 q1x, q1x
    vsqrt.f64 q1y, q1y
    vadd.f64 q0x, q0x, q0y
    vadd.f64 q1x, q1x, q1y
    vadd.f64 vecMagnitude, q0x, q1x
    vmul.f64 vecMagnitude, vecMagnitude, vecOneOverNumPoints
    vst1.64 { vecTx }, [tx1 :64]
    vst1.64 { vecTy }, [ty1 :64]
    vdiv.f64 vecMagnitude, vecSqrt2, vecMagnitude    
    vst1.64 { vecMagnitude }, [sf1 :64]
    
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
    COMPV_GAS_UNSHADOW_ARGS 6
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#endif /* defined(__arm__) && !defined(__aarch64__) */

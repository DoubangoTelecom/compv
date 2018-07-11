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

########################################################################
.equ STEP_2, 2
.equ STEP_1, 1
.macro COMPV_MATH_EXP_64F64F_ASM_NEON64 step
	//vmov.f64 vecDI, vecB # Needed only for FMA or ARM32 
	fmin vecT.2d, vecT.2d, vecMax.2d
	fmax vecT.2d, vecT.2d, vecMin.2d

	// FMA:
	fmul vec0.2d, vecT.2d, vecCA.2d
	fadd vecDI.2d, vecB.2d, vec0.2d

	fsub vec0.2d, vecDI.2d, vecB.2d
	add vecU.2d, vecDI.2d, vecCADJ.2d
			
	// FMA:
	fmul vec0.2d, vec0.2d, vecCRA.2d
	fsub vecT.2d, vecT.2d, vec0.2d

	ushr vecU.2d, vecU.2d, #11
	shl vecU.2d, vecU.2d, #52
	fmul vecY.2d, vecT.2d, vecT.2d
	and vecDI.16b, vecDI.16b, vecMask.16b
	fadd vec0.2d, vecC30.2d, vecT.2d
	shl vecDI.2d, vecDI.2d, #(COMPV_GAS_UINT64_SHIFT_BYTES)
	fmul vecY.2d, vecY.2d, vec0.2d
	mov rt0, vecDI.d[0]
	.if \step == STEP_2
		mov rt1, vecDI.d[1]
	.endif
	add rt0, lut64u, rt0
	.if \step == STEP_2
		add rt1, lut64u, rt1
	.endif
	ld1 { vecLUT.d }[0], [rt0]
	.if \step == STEP_2
		ld1 { vecLUT.d }[1], [rt1]
	.endif

	// FMA:
	fmul vec0.2d, vecY.2d, vecC20.2d
	fadd vecT.2d, vecT.2d, vec0.2d
			
	orr vecU.16b, vecU.16b, vecLUT.16b
	fadd vecY.2d, vecC10.2d, vecT.2d
	fmul vecY.2d, vecY.2d, vecU.2d
.endm

########################################################################
# arg(0) -> const compv_float64_t* ptrIn
# arg(1) -> compv_float64_t* ptrOut
# arg(2) -> const compv_uscalar_t width
# arg(3) -> const compv_uscalar_t height
# arg(4) -> const compv_uscalar_t stride
# arg(5) -> const uint64_t* lut64u
# arg(6) -> const uint64_t* var64u
# arg(7) -> const compv_float64_t* var64f
COMPV_GAS_FUNCTION_DECLARE CompVMathExpExp_minpack1_64f64f_Asm_NEON64
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS
	## end prolog ##

	## Load arguments ##
	ptrIn .req r0
	ptrOut .req r1
	width .req r2
	height .req r3
	stride .req r4
	lut64u .req r5
	var64u .req r6
	var64f .req r7

	# Local Variables #
    i .req r8
	rt0 .req r9
	rt1 .req r10
	width2 .req r11
	vecC10 .req v0
	vecMax .req v1
	vecMin .req v2
	vecDI .req v3
	vecB .req v4
	vecCA .req v5
	vecT .req v6
	vec0 .req v7
	vecCRA .req v8
	vecCADJ .req v9
	vecU .req v10
	vecY .req v11
	vecMask .req v12
	vecC30 .req v13
	vecLUT .req v14
	vecC20 .req v15
	
	ld1 { vecMask.d }[0], [var64u], #(COMPV_GAS_FLOAT64_SZ_BYTES)
	ld1 { vecCADJ.d }[0], [var64u]
	
	ld1 { vecB.d }[0], [var64f], #(COMPV_GAS_FLOAT64_SZ_BYTES)
	ld1 { vecCA.d }[0], [var64f], #(COMPV_GAS_FLOAT64_SZ_BYTES)
	ld1 { vecCRA.d }[0], [var64f], #(COMPV_GAS_FLOAT64_SZ_BYTES)
	ld1 { vecC10.d }[0], [var64f], #(COMPV_GAS_FLOAT64_SZ_BYTES)
	ld1 { vecC20.d }[0], [var64f], #(COMPV_GAS_FLOAT64_SZ_BYTES)
	ld1 { vecC30.d }[0], [var64f], #(COMPV_GAS_FLOAT64_SZ_BYTES)
	ld1 { vecMin.d }[0], [var64f], #(COMPV_GAS_FLOAT64_SZ_BYTES)
	ld1 { vecMax.d }[0], [var64f]

	dup vecMask.2d, vecMask.d[0]
	dup vecCADJ.2d, vecCADJ.d[0]
	
	dup vecB.2d, vecB.d[0]
	dup vecCA.2d, vecCA.d[0]
	dup vecCRA.2d, vecCRA.d[0]
	dup vecC10.2d, vecC10.d[0]
	dup vecC20.2d, vecC20.d[0]
	dup vecC30.2d, vecC30.d[0]
	dup vecMin.2d, vecMin.d[0]
	dup vecMax.2d, vecMax.d[0]

	prfm pldl1keep, [ptrIn, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [ptrIn, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [ptrIn, #(CACHE_LINE_SIZE*2)]

	# Transform stride to padding
	and width2, width, #-2
	sub stride, stride, width
	lsl stride, stride, #(COMPV_GAS_FLOAT64_SHIFT_BYTES)

	#################################################
	# for (compv_uscalar_t j = 0; j < height; ++j)
	#################################################
	LoopHeight_CompVMathExpExp_minpack1_64f64f_Asm_NEON64:
		###################################################
		# for (compv_uscalar_t i = 0; i < width2; i += 2)
		###################################################
		mov i, width2
		cbz i, EndOf_LoopWidth2_CompVMathExpExp_minpack1_64f64f_Asm_NEON64
		LoopWidth2_CompVMathExpExp_minpack1_64f64f_Asm_NEON64:
			ld1 { vecT.2d }, [ptrIn], #(2*COMPV_GAS_FLOAT64_SZ_BYTES)
			prfm pldl1keep, [ptrIn, #(CACHE_LINE_SIZE*3)]
			COMPV_MATH_EXP_64F64F_ASM_NEON64 STEP_2
			subs i, i, #2
			st1 { vecY.2d }, [ptrOut], #(2*COMPV_GAS_FLOAT64_SZ_BYTES)
			bne LoopWidth2_CompVMathExpExp_minpack1_64f64f_Asm_NEON64
		EndOf_LoopWidth2_CompVMathExpExp_minpack1_64f64f_Asm_NEON64:

		###################################################
		# for (; i < width; i += 1)
		###################################################
		cmp width, width2
		beq EndOf_LoopWidth1_CompVMathExpExp_minpack1_64f64f_Asm_NEON64
		LoopWidth1_CompVMathExpExp_minpack1_64f64f_Asm_NEON64:
			ld1 { vecT.d }[0], [ptrIn], #(1*COMPV_GAS_FLOAT64_SZ_BYTES)
			COMPV_MATH_EXP_64F64F_ASM_NEON64 STEP_1
			st1 { vecY.d }[0], [ptrOut], #(1*COMPV_GAS_FLOAT64_SZ_BYTES)
		EndOf_LoopWidth1_CompVMathExpExp_minpack1_64f64f_Asm_NEON64:

		add ptrIn, ptrIn, stride
		add ptrOut, ptrOut, stride
		subs height, height, #1
		bne LoopHeight_CompVMathExpExp_minpack1_64f64f_Asm_NEON64 
	EndOf_LoopHeight_CompVMathExpExp_minpack1_64f64f_Asm_NEON64:

	.unreq ptrIn
	.unreq ptrOut
	.unreq width
	.unreq height
	.unreq stride
	.unreq lut64u
	.unreq var64u
	.unreq var64f

	.unreq i
	.unreq rt0
	.unreq rt1
	.unreq width2
	.unreq vecC10
	.unreq vecMax
	.unreq vecMin
	.unreq vecDI
	.unreq vecB
	.unreq vecCA
	.unreq vecT
	.unreq vec0
	.unreq vecCRA
	.unreq vecCADJ
	.unreq vecU
	.unreq vecY
	.unreq vecMask
	.unreq vecC30
	.unreq vecLUT
	.unreq vecC20

	## begin epilog ##
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__aarch64__) */

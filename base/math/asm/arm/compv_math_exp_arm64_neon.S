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

.equ STEP_2, 2
.equ STEP_1, 1
.equ FMA_YES, 1
.equ FMA_NO, 0

#########################################################
# arg(0) -> COMPV_ALIGNED(NEON) const compv_float32_t* ptrIn
# arg(1) -> COMPV_ALIGNED(NEON) compv_float32_t* ptrOut
# arg(2) -> const compv_uscalar_t width
# arg(3) -> const compv_uscalar_t height
# arg(4) -> COMPV_ALIGNED(NEON) const compv_uscalar_t stride
# arg(5) -> COMPV_ALIGNED(NEON) const uint32_t* lut32u
# arg(6) -> COMPV_ALIGNED(NEON) const compv_float32_t* var32f
.macro CompVMathExpExp_minpack1_32f32f_Macro_NEON64 fusedMultiplyAdd
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Load arguments ##
	ptrIn .req r0
	ptrOut .req r1
	width .req r2
	height .req r3
	stride .req r4
	lut32u .req r5
	var32f .req r6

	i .req r7
	t0 .req r8
	t0w .req r8w
	t1 .req r9
	t1w .req r9w
	t2 .req r10
	t2w .req r10w
	t3 .req r11
	t3w .req r11w

	vecMagic .req v0
	vecA0 .req v1
	vecB0 .req v2
	vecMaxX .req v3
	vecMinX .req v4
	vec130048 .req v5
	vec1023 .req v6
	vecX .req v7
	vecFi .req v8
	vecT .req v9
	vecU .req v10
	vecV .req v11

	prfm pldl1keep, [ptrIn, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [ptrIn, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [ptrIn, #(CACHE_LINE_SIZE*2)]
	prfm pldl1keep, [ptrIn, #(CACHE_LINE_SIZE*3)]

	# Transform stride to padding #
	add i, width, #3
	and i, i, #-4
	sub stride, stride, i
	lsl stride, stride, #(COMPV_GAS_FLOAT32_SHIFT_BYTES)


	ldr r12w, [var32f,  #(0 * COMPV_GAS_FLOAT32_SZ_BYTES)]
	ldr r13w, [var32f,  #(1 * COMPV_GAS_FLOAT32_SZ_BYTES)]
	ldr r14w, [var32f,  #(2 * COMPV_GAS_FLOAT32_SZ_BYTES)]
	ldr r15w, [var32f,  #(3 * COMPV_GAS_FLOAT32_SZ_BYTES)]
	ldr r16w, [var32f,  #(4 * COMPV_GAS_FLOAT32_SZ_BYTES)]
	dup vecMagic.4s, r12w
	dup vecA0.4s, r13w
	dup vecB0.4s, r14w
	dup vecMaxX.4s, r15w
	dup vecMinX.4s, r16w
	
	mov r12w, #130048
	mov r13w, #1023
	dup vec130048.4s, r12w
	dup vec1023.4s, r13w

	################################################
	# for (compv_uscalar_t j = 0; j < height; ++j)
	################################################
	LoopHeigth_CompVMathExpExp_minpack1_32f32f_Asm_NEON64\@:
		################################################
		# for (i = 0; i < width; i += 4)
		################################################
		mov i, #0
		LoopWidth_CompVMathExpExp_minpack1_32f32f_Asm_NEON64\@:
			ld1 { vecX.4s }, [ptrIn], #(1 * COMPV_GAS_V_SZ_BYTES)
			prfm pldl1keep, [ptrIn, #(CACHE_LINE_SIZE*4)]

			fmin vecX.4s, vecX.4s, vecMaxX.4s
			fmax vecX.4s, vecX.4s, vecMinX.4s
			.if \fusedMultiplyAdd == FMA_YES
				mov vecFi.16b, vecMagic.16b
				fmla vecFi.4s, vecX.4s, vecA0.4s
			.else
				fmul vecFi.4s, vecX.4s, vecA0.4s
				fadd vecFi.4s, vecFi.4s, vecMagic.4s
			.endif			
			fsub vecT.4s, vecFi.4s, vecMagic.4s
			.if \fusedMultiplyAdd == FMA_YES
				fmls vecX.4s, vecT.4s, vecB0.4s
			.else
				fmul vecT.4s, vecT.4s, vecB0.4s
				fsub vecX.4s, vecX.4s, vecT.4s // vecX = vecT from intrin code
			.endif

			add vecU.4s, vecFi.4s, vec130048.4s
			and vecV.16b, vecFi.16b, vec1023.16b
			ushr vecU.4s, vecU.4s, #10
			shl vecU.4s, vecU.4s, #23

			umov t0, vecV.d[0]
			umov t2, vecV.d[1]
			lsr t1, t0, #32
			and t0, t0, #0xffffffff
			lsr t3, t2, #32
			and t2, t2, #0xffffffff
			ldr t0w, [lut32u, t0, LSL #(COMPV_GAS_UINT32_SHIFT_BYTES)]
			ldr t1w, [lut32u, t1, LSL #(COMPV_GAS_UINT32_SHIFT_BYTES)]
			ldr t2w, [lut32u, t2, LSL #(COMPV_GAS_UINT32_SHIFT_BYTES)]
			ldr t3w, [lut32u, t3, LSL #(COMPV_GAS_UINT32_SHIFT_BYTES)]
			orr t0, t0, t1, LSL #32
			orr t2, t2, t3, LSL #32
			ins vecV.d[0], t0
			ins vecV.d[1], t2			
			orr vecV.16b, vecV.16b, vecU.16b

			.if \fusedMultiplyAdd == FMA_YES
				fmla vecV.4s, vecX.4s, vecV.4s
			.else
				fmul vecX.4s, vecX.4s, vecV.4s
				fadd vecV.4s, vecV.4s, vecX.4s
			.endif
			st1 { vecV.4s }, [ptrOut], #(1 * COMPV_GAS_V_SZ_BYTES)

			add i, i, #4
			cmp i, width
			blt LoopWidth_CompVMathExpExp_minpack1_32f32f_Asm_NEON64\@
		EndOf_LoopWidth_CompVMathExpExp_minpack1_32f32f_Asm_NEON64\@:

		subs height, height, #1
		add ptrIn, ptrIn, stride
		add ptrOut, ptrOut, stride
		bne LoopHeigth_CompVMathExpExp_minpack1_32f32f_Asm_NEON64\@
	EndOf_LoopHeigth_CompVMathExpExp_minpack1_32f32f_Asm_NEON64\@:

	.unreq ptrIn
	.unreq ptrOut
	.unreq width
	.unreq height
	.unreq stride
	.unreq lut32u
	.unreq var32f

	.unreq i
	.unreq t0
	.unreq t0w
	.unreq t1
	.unreq t1w
	.unreq t2
	.unreq t2w
	.unreq t3
	.unreq t3w

	.unreq vecMagic
	.unreq vecA0
	.unreq vecB0
	.unreq vecMaxX
	.unreq vecMinX
	.unreq vec130048
	.unreq vec1023
	.unreq vecX
	.unreq vecFi
	.unreq vecT
	.unreq vecU
	.unreq vecV

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVMathExpExp_minpack1_32f32f_Asm_NEON64
	CompVMathExpExp_minpack1_32f32f_Macro_NEON64 FMA_NO

########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVMathExpExp_minpack1_32f32f_Asm_FMA_NEON64
	CompVMathExpExp_minpack1_32f32f_Macro_NEON64 FMA_YES

########################################################################
.macro COMPV_MATH_EXP_64F64F_ASM_NEON64 step, fusedMultiplyAdd
	.if \fusedMultiplyAdd == FMA_YES
		mov vecDI.16b, vecB.16b // Needed only for FMA or ARM32
	.endif
	fmin vecT.2d, vecT.2d, vecMax.2d
	fmax vecT.2d, vecT.2d, vecMin.2d
	.if \fusedMultiplyAdd == FMA_YES
		fmla vecDI.2d, vecT.2d, vecCA.2d
	.else
		fmul vec0.2d, vecT.2d, vecCA.2d
		fadd vecDI.2d, vecB.2d, vec0.2d
	.endif
	fsub vec0.2d, vecDI.2d, vecB.2d
	add vecU.2d, vecDI.2d, vecCADJ.2d
	.if \fusedMultiplyAdd == FMA_YES
		fmls vecT.2d, vec0.2d, vecCRA.2d
	.else
		fmul vec0.2d, vec0.2d, vecCRA.2d
		fsub vecT.2d, vecT.2d, vec0.2d
	.endif
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

	.if \fusedMultiplyAdd == FMA_YES
		fmla vecT.2d, vecY.2d, vecC20.2d
	.else
		fmul vec0.2d, vecY.2d, vecC20.2d
		fadd vecT.2d, vecT.2d, vec0.2d
	.endif
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
.macro CompVMathExpExp_minpack1_64f64f_Macro_NEON64 fusedMultiplyAdd
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
	LoopHeight_CompVMathExpExp_minpack1_64f64f_Asm_NEON64\@:
		###################################################
		# for (compv_uscalar_t i = 0; i < width2; i += 2)
		###################################################
		mov i, width2
		cbz i, EndOf_LoopWidth2_CompVMathExpExp_minpack1_64f64f_Asm_NEON64\@
		LoopWidth2_CompVMathExpExp_minpack1_64f64f_Asm_NEON64\@:
			ld1 { vecT.2d }, [ptrIn], #(2*COMPV_GAS_FLOAT64_SZ_BYTES)
			prfm pldl1keep, [ptrIn, #(CACHE_LINE_SIZE*3)]
			COMPV_MATH_EXP_64F64F_ASM_NEON64 STEP_2, \fusedMultiplyAdd
			subs i, i, #2
			st1 { vecY.2d }, [ptrOut], #(2*COMPV_GAS_FLOAT64_SZ_BYTES)
			bne LoopWidth2_CompVMathExpExp_minpack1_64f64f_Asm_NEON64\@
		EndOf_LoopWidth2_CompVMathExpExp_minpack1_64f64f_Asm_NEON64\@:

		###################################################
		# for (; i < width; i += 1)
		###################################################
		cmp width, width2
		beq EndOf_LoopWidth1_CompVMathExpExp_minpack1_64f64f_Asm_NEON64\@
		LoopWidth1_CompVMathExpExp_minpack1_64f64f_Asm_NEON64\@:
			ld1 { vecT.d }[0], [ptrIn], #(1*COMPV_GAS_FLOAT64_SZ_BYTES)
			COMPV_MATH_EXP_64F64F_ASM_NEON64 STEP_1, \fusedMultiplyAdd
			st1 { vecY.d }[0], [ptrOut], #(1*COMPV_GAS_FLOAT64_SZ_BYTES)
		EndOf_LoopWidth1_CompVMathExpExp_minpack1_64f64f_Asm_NEON64\@:

		add ptrIn, ptrIn, stride
		add ptrOut, ptrOut, stride
		subs height, height, #1
		bne LoopHeight_CompVMathExpExp_minpack1_64f64f_Asm_NEON64\@ 
	EndOf_LoopHeight_CompVMathExpExp_minpack1_64f64f_Asm_NEON64\@:

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
.endm

########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVMathExpExp_minpack1_64f64f_Asm_NEON64
	CompVMathExpExp_minpack1_64f64f_Macro_NEON64 FMA_NO

########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVMathExpExp_minpack1_64f64f_Asm_FMA_NEON64
	CompVMathExpExp_minpack1_64f64f_Macro_NEON64 FMA_YES

#endif /* defined(__aarch64__) */

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

#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) const compv_float32_t* ptrIn
# arg(1) -> COMPV_ALIGNED(NEON) compv_float32_t* ptrOut
# arg(2) -> const compv_uscalar_t width
# arg(3) -> const compv_uscalar_t height
# arg(4) -> COMPV_ALIGNED(NEON) const compv_uscalar_t stride
# arg(5) -> const compv_float32_t* scale1
COMPV_GAS_FUNCTION_DECLARE CompVMathScaleScale_32f32f_Asm_NEON64
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Load arguments ##
	ptrIn .req r0
	ptrOut .req r1
	width .req r2
	height .req r3
	stride .req r4
	scale1 .req r5

	width32 .req r6
	i .req r7
	iw .req r7w

	prfm pldl1keep, [ptrIn, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [ptrIn, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [ptrIn, #(CACHE_LINE_SIZE*2)]
	prfm pldl1keep, [ptrIn, #(CACHE_LINE_SIZE*3)]

	and width32, width, #-32

	# Transform stride to padding #
	add i, width, #3
	and i, i, #-4
	sub stride, stride, i
	lsl stride, stride, #(COMPV_GAS_FLOAT32_SHIFT_BYTES)

	ldr iw, [scale1]
	dup v15.4s, iw

	################################################
	# for (compv_uscalar_t j = 0; j < height; ++j)
	################################################
	LoopHeigth_CompVMathScaleScale_32f32f_Asm_NEON64:
		################################################
		# for (i = 0; i < width32; i += 32)
		################################################
		mov i, #0
		tst width32, width32
		beq EndOf_LoopWidth32_CompVMathScaleScale_32f32f_Asm_NEON64
		LoopWidth32_CompVMathScaleScale_32f32f_Asm_NEON64:
			ldp q0, q1, [ptrIn], #(2 * COMPV_GAS_Q_SZ_BYTES)
			ldp q2, q3, [ptrIn], #(2 * COMPV_GAS_Q_SZ_BYTES)
			ldp q4, q5, [ptrIn], #(2 * COMPV_GAS_Q_SZ_BYTES)
			ldp q6, q7, [ptrIn], #(2 * COMPV_GAS_Q_SZ_BYTES)
			prfm pldl1keep, [ptrIn, #(CACHE_LINE_SIZE*4)]
			fmul v0.4s, v0.4s, v15.4s
			fmul v1.4s, v1.4s, v15.4s
			fmul v2.4s, v2.4s, v15.4s
			fmul v3.4s, v3.4s, v15.4s
			fmul v4.4s, v4.4s, v15.4s
			fmul v5.4s, v5.4s, v15.4s
			fmul v6.4s, v6.4s, v15.4s
			fmul v7.4s, v7.4s, v15.4s
			stp q0, q1, [ptrOut], #(2 * COMPV_GAS_Q_SZ_BYTES)
			stp q2, q3, [ptrOut], #(2 * COMPV_GAS_Q_SZ_BYTES)
			stp q4, q5, [ptrOut], #(2 * COMPV_GAS_Q_SZ_BYTES)
			stp q6, q7, [ptrOut], #(2 * COMPV_GAS_Q_SZ_BYTES)
			add i, i, #32
			cmp i, width32
			blt  LoopWidth32_CompVMathScaleScale_32f32f_Asm_NEON64
		EndOf_LoopWidth32_CompVMathScaleScale_32f32f_Asm_NEON64:
		
		################################################
		# for (; i < width; i += 4)
		################################################
		cmp i, width
		bge EndOf_LoopWidth_CompVMathScaleScale_32f32f_Asm_NEON64
		LoopWidth_CompVMathScaleScale_32f32f_Asm_NEON64:
			ldr q0, [ptrIn], #(1 * COMPV_GAS_Q_SZ_BYTES)
			fmul v0.4s, v0.4s, v15.4s
			str q0, [ptrOut], #(1 * COMPV_GAS_Q_SZ_BYTES)
			add i, i, #4
			cmp i, width
			blt LoopWidth_CompVMathScaleScale_32f32f_Asm_NEON64
		EndOf_LoopWidth_CompVMathScaleScale_32f32f_Asm_NEON64:

		subs height, height, #1
		add ptrIn, ptrIn, stride
		add ptrOut, ptrOut, stride
		bne  LoopHeigth_CompVMathScaleScale_32f32f_Asm_NEON64
	EndOf_LoopHeigth_CompVMathScaleScale_32f32f_Asm_NEON64:

	.unreq ptrIn
	.unreq ptrOut
	.unreq width
	.unreq height
	.unreq stride
	.unreq scale1

	.unreq width32
	.unreq i
	.unreq iw

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__aarch64__) */
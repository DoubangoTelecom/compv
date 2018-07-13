#########################################################################
# Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__arm__) && !defined(__aarch64__)
.include "compv_common_arm32.S" @

.data

.text

.equ DOT_TYPE_DOT, 1
.equ DOT_TYPE_SUB, 0
.equ FMA_YES, 1
.equ FMA_NO, 0

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> const compv_float64_t* ptrA
@ arg(1) -> const compv_float64_t* ptrB
@ arg(2) -> const compv_uscalar_t width
@ arg(3) -> const compv_uscalar_t height
@ arg(4) -> const compv_uscalar_t strideA
@ arg(5) -> const compv_uscalar_t strideB
@ arg(6) -> compv_float64_t* out
.macro CompVMathDotDotSub_64f64f_Macro_NEON32 dotType, fusedMultiplyAdd
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 7
	COMPV_GAS_SAVE_NEON_REGS
	@@ end prolog @@

	@ Load arguments @
	ldm_args r0-r6
	ptrA .req r0
	ptrB .req r1
	width .req r2
	height .req r3
	strideA .req r4
	strideB .req r5
	out .req r6

	@ Local Variables @
    i .req r7
	width16 .req r8
	width2 .req r9
	vecSum0 .req q12
	vecSum0x .req q12x
	vecSum0y .req q12y
	vecSum1 .req q13
	vecSum1x .req q13x
	vecSum1y .req q13y
	.if \fusedMultiplyAdd == FMA_YES
		vecSum2 .req q14
		vecSum2x .req q14x
		vecSum2y .req q14y
		vecSum3 .req q15
		vecSum3x .req q15x
		vecSum3y .req q15y
	.endif

	and width16, width, #-16
	and width2, width, #-2

	@ Reset vecSum0 and vecSum1 to zeros
	veor.f64  vecSum0, vecSum0, vecSum0
	veor.f64  vecSum1, vecSum1, vecSum1
	.if \fusedMultiplyAdd == FMA_YES
		veor.f64  vecSum2, vecSum2, vecSum2
		veor.f64  vecSum3, vecSum3, vecSum3
	.endif

	@ Transform stride to padding
	sub strideA, strideA, width
	sub strideB, strideB, width
	lsl strideA, strideA, #(COMPV_GAS_FLOAT64_SHIFT_BYTES)
	lsl strideB, strideB, #(COMPV_GAS_FLOAT64_SHIFT_BYTES)

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (compv_uscalar_t j = 0; j < height; ++j)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopHeight_CompVMathDotDotSub_64f64f_Asm_NEON32\@:
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (i = 0; i < width16; i += 16)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		movs i, width16
		beq EndOf_LoopWidth16_CompVMathDotDotSub_64f64f_Asm_NEON32\@
		LoopWidth16_CompVMathDotDotSub_64f64f_Asm_NEON32\@:
			vldm ptrA!, { q0-q3 }
			vldm ptrB!, { q8-q11 }
			.if \dotType == DOT_TYPE_SUB
				vsub.f64 q0x, q0x, q8x
				vsub.f64 q0y, q0y, q8y
				vsub.f64 q1x, q1x, q9x
				vsub.f64 q1y, q1y, q9y
				vsub.f64 q2x, q2x, q10x
				vsub.f64 q2y, q2y, q10y
				vsub.f64 q3x, q3x, q11x
				vsub.f64 q3y, q3y, q11y
			.else
				.if \fusedMultiplyAdd == FMA_YES
					vfma.f64 vecSum0x, q0x, q8x
					vfma.f64 vecSum0y, q0y, q8y
					vfma.f64 vecSum1x, q1x, q9x
					vfma.f64 vecSum1y, q1y, q9y
					vfma.f64 vecSum2x, q2x, q10x
					vfma.f64 vecSum2y, q2y, q10y
					vfma.f64 vecSum3x, q3x, q11x
					vfma.f64 vecSum3y, q3y, q11y
				.else
					vmul.f64 q0x, q0x, q8x
					vmul.f64 q0y, q0y, q8y
					vmul.f64 q1x, q1x, q9x
					vmul.f64 q1y, q1y, q9y
					vmul.f64 q2x, q2x, q10x
					vmul.f64 q2y, q2y, q10y
					vmul.f64 q3x, q3x, q11x
					vmul.f64 q3y, q3y, q11y
				.endif
			.endif
			vldm ptrA!, { q4-q7 }
			vldm ptrB!, { q8-q11 }
			.if \dotType == DOT_TYPE_SUB
				vsub.f64 q4x, q4x, q8x
				vsub.f64 q4y, q4y, q8y
				vsub.f64 q5x, q5x, q9x
				vsub.f64 q5y, q5y, q9y
				vsub.f64 q6x, q6x, q10x
				vsub.f64 q6y, q6y, q10y
				vsub.f64 q7x, q7x, q11x
				vsub.f64 q7y, q7y, q11y
				.if \fusedMultiplyAdd == FMA_YES
					vfma.f64 vecSum0x, q0x, q0x
					vfma.f64 vecSum0y, q0y, q0y
					vfma.f64 vecSum1x, q1x, q1x
					vfma.f64 vecSum1y, q1y, q1y
					vfma.f64 vecSum2x, q2x, q2x
					vfma.f64 vecSum2y, q2y, q2y
					vfma.f64 vecSum3x, q3x, q3x
					vfma.f64 vecSum3y, q3y, q3y
					vfma.f64 vecSum0x, q4x, q4x
					vfma.f64 vecSum0y, q4y, q4y
					vfma.f64 vecSum1x, q5x, q5x
					vfma.f64 vecSum1y, q5y, q5y
					vfma.f64 vecSum2x, q6x, q6x
					vfma.f64 vecSum2y, q6y, q6y
					vfma.f64 vecSum3x, q7x, q7x
					vfma.f64 vecSum3y, q7y, q7y
				.else
					vmul.f64 q0x, q0x, q0x
					vmul.f64 q0y, q0y, q0y
					vmul.f64 q2x, q2x, q2x
					vmul.f64 q2y, q2y, q2y
					vmul.f64 q4x, q4x, q4x
					vmul.f64 q4y, q4y, q4y
					vmul.f64 q6x, q6x, q6x
					vmul.f64 q6y, q6y, q6y
					vmul.f64 q1x, q1x, q1x
					vmul.f64 q1y, q1y, q1y
					vmul.f64 q3x, q3x, q3x
					vmul.f64 q3y, q3y, q3y
					vmul.f64 q5x, q5x, q5x
					vmul.f64 q5y, q5y, q5y
					vmul.f64 q7x, q7x, q7x
					vmul.f64 q7y, q7y, q7y
				.endif
			.else
				.if \fusedMultiplyAdd == FMA_YES
					vfma.f64 vecSum0x, q4x, q8x
					vfma.f64 vecSum0y, q4y, q8y
					vfma.f64 vecSum1x, q5x, q9x
					vfma.f64 vecSum1y, q5y, q9y
					vfma.f64 vecSum2x, q6x, q10x
					vfma.f64 vecSum2y, q6y, q10y
					vfma.f64 vecSum3x, q7x, q11x
					vfma.f64 vecSum3y, q7y, q11y
				.else
					vmul.f64 q4x, q4x, q8x
					vmul.f64 q4y, q4y, q8y
					vmul.f64 q5x, q5x, q9x
					vmul.f64 q5y, q5y, q9y
					vmul.f64 q6x, q6x, q10x
					vmul.f64 q6y, q6y, q10y
					vmul.f64 q7x, q7x, q11x
					vmul.f64 q7y, q7y, q11y
				.endif
			.endif
			.if \fusedMultiplyAdd == FMA_NO
				vadd.f64 q0x, q0x, q2x
				vadd.f64 q0y, q0y, q2y
				vadd.f64 q4x, q4x, q6x
				vadd.f64 q4y, q4y, q6y
				vadd.f64 q1x, q1x, q3x
				vadd.f64 q1y, q1y, q3y
				vadd.f64 q5x, q5x, q7x
				vadd.f64 q5y, q5y, q7y
				vadd.f64 q0x, q0x, q4x
				vadd.f64 q0y, q0y, q4y
				vadd.f64 q1x, q1x, q5x
				vadd.f64 q1y, q1y, q5y
				vadd.f64 vecSum0x, vecSum0x, q0x
				vadd.f64 vecSum0y, vecSum0y, q0y
				vadd.f64 vecSum1x, vecSum1x, q1x
				vadd.f64 vecSum1y, vecSum1y, q1y
			.endif
			subs i, i, #16
			bne LoopWidth16_CompVMathDotDotSub_64f64f_Asm_NEON32\@
		EndOf_LoopWidth16_CompVMathDotDotSub_64f64f_Asm_NEON32\@:

		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (; i < width2; i += 2)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		subs i, width2, width16
		beq EndOf_LoopWidth2_CompVMathDotDotSub_64f64f_Asm_NEON32\@
		LoopWidth2_CompVMathDotDotSub_64f64f_Asm_NEON32\@:
			vld1.f64 { q0 }, [ptrA]!
			vld1.f64 { q8 }, [ptrB]!
			.if \dotType == DOT_TYPE_SUB
				vsub.f64 q0x, q0x, q8x
				vsub.f64 q0y, q0y, q8y
				.if \fusedMultiplyAdd == FMA_YES
					vfma.f64 vecSum0x, q0x, q0x
					vfma.f64 vecSum0y, q0y, q0y
				.else
					vmul.f64 q0x, q0x, q0x
					vmul.f64 q0y, q0y, q0y
				.endif
			.else
				.if \fusedMultiplyAdd == FMA_YES
					vfma.f64 vecSum0x, q0x, q8x
					vfma.f64 vecSum0y, q0y, q8y
				.else
					vmul.f64 q0x, q0x, q8x
					vmul.f64 q0y, q0y, q8y
				.endif
			.endif
			.if \fusedMultiplyAdd == FMA_NO
				vadd.f64 vecSum0x, vecSum0x, q0x
				vadd.f64 vecSum0y, vecSum0y, q0y
			.endif
			subs i, i, #2
			bne LoopWidth2_CompVMathDotDotSub_64f64f_Asm_NEON32\@
		EndOf_LoopWidth2_CompVMathDotDotSub_64f64f_Asm_NEON32\@:

		#############################################
		# for (; i < width1; i += 1)
		#############################################
		cmp width, width2
		beq EndOf_LoopWidth1_CompVMathDotDotSub_64f64f_Asm_NEON32\@
		LoopWidth1_CompVMathDotDotSub_64f64f_Asm_NEON32\@:
			vld1.f64 { q0x }, [ptrA]!
			vld1.f64 { q8x }, [ptrB]!
			.if \dotType == DOT_TYPE_SUB
				vsub.f64 q0x, q0x, q8x
				.if \fusedMultiplyAdd == FMA_YES
					vfma.f64 vecSum0x, q0x, q0x
				.else
					vmul.f64 q0x, q0x, q0x
				.endif
			.else
				.if \fusedMultiplyAdd == FMA_YES
					vfma.f64 vecSum0x, q0x, q8x
				.else
					vmul.f64 q0x, q0x, q8x
				.endif
			.endif
			.if \fusedMultiplyAdd == FMA_NO
				vadd.f64 vecSum0x, vecSum0x, q0x
			.endif
		EndOf_LoopWidth1_CompVMathDotDotSub_64f64f_Asm_NEON32\@:

		subs height, height, #1
		add ptrA, ptrA, strideA
		add ptrB, ptrB, strideB
		bne LoopHeight_CompVMathDotDotSub_64f64f_Asm_NEON32\@
	EndOf_LoopHeight_CompVMathDotDotSub_64f64f_Asm_NEON32\@:

	vadd.f64 vecSum0x, vecSum0x, vecSum1x
	vadd.f64 vecSum0y, vecSum0y, vecSum1y
	.if \fusedMultiplyAdd == FMA_YES
		vadd.f64 vecSum2x, vecSum2x, vecSum3x
		vadd.f64 vecSum2y, vecSum2y, vecSum3y
		vadd.f64 vecSum0x, vecSum0x, vecSum2x
		vadd.f64 vecSum0y, vecSum0y, vecSum2y
	.endif
	vadd.f64 vecSum0x, vecSum0x, vecSum0y
	vst1.f64 { vecSum0x }, [out]

	.unreq ptrA
	.unreq ptrB
	.unreq width
	.unreq height
	.unreq strideA
	.unreq strideB
	.unreq out

    .unreq i
	.unreq width16
	.unreq width2
	.unreq vecSum0
	.unreq vecSum0x
	.unreq vecSum0y
	.unreq vecSum1
	.unreq vecSum1x
	.unreq vecSum1y
	.if \fusedMultiplyAdd == FMA_YES
		.unreq vecSum2
		.unreq vecSum2x
		.unreq vecSum2y
		.unreq vecSum3
		.unreq vecSum3x
		.unreq vecSum3y
	.endif

	@@ begin epilog @@
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 7
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVMathDotDotSub_64f64f_Asm_NEON32
	CompVMathDotDotSub_64f64f_Macro_NEON32 DOT_TYPE_SUB, FMA_NO

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVMathDotDotSub_64f64f_Asm_FMA_NEON32
	CompVMathDotDotSub_64f64f_Macro_NEON32 DOT_TYPE_SUB, FMA_YES

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVMathDotDot_64f64f_Asm_NEON32
	CompVMathDotDotSub_64f64f_Macro_NEON32 DOT_TYPE_DOT, FMA_NO

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVMathDotDot_64f64f_Asm_FMA_NEON32
	CompVMathDotDotSub_64f64f_Macro_NEON32 DOT_TYPE_DOT, FMA_YES

#endif /* defined(__arm__) && !defined(__aarch64__) */

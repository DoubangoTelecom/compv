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

.equ DOT_TYPE_DOT, 1
.equ DOT_TYPE_SUB, 0
.equ FMA_YES, 1
.equ FMA_NO, 0

################################################################
# arg(0) -> const compv_float64_t* ptrA
# arg(1) -> const compv_float64_t* ptrB
# arg(2) -> const compv_uscalar_t width
# arg(3) -> const compv_uscalar_t height
# arg(4) -> const compv_uscalar_t strideA
# arg(5) -> const compv_uscalar_t strideB
# arg(6) -> compv_float64_t* out
.macro CompVMathDotDotSub_64f64f_Macro_NEON64 dotType, fusedMultiplyAdd
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS
	## end prolog ##

	# Load arguments #
	ptrA .req r0
	ptrB .req r1
	width .req r2
	height .req r3
	strideA .req r4
	strideB .req r5
	out .req r6

	# Local Variables #
    i .req r7
	width16 .req r8
	width2 .req r9
	vecLowA .req v26
	vecLowAd .req d26
	vecLowB .req v27
	vecLowBd .req d27
	vecSum0 .req v28
	vecSum1 .req v29
	.if \fusedMultiplyAdd == FMA_YES
		vecSum2 .req v30
		vecSum3 .req v31
	.endif

	and width16, width, #-16
	and width2, width, #-2

	# Reset vecSum0 and vecSum1 to zeros
	eor vecLowA.16b, vecLowA.16b, vecLowA.16b
	eor vecLowB.16b, vecLowB.16b, vecLowB.16b
	eor vecSum0.16b, vecSum0.16b, vecSum0.16b
	eor vecSum1.16b, vecSum1.16b, vecSum1.16b
	.if \fusedMultiplyAdd == FMA_YES
		eor vecSum2.16b, vecSum2.16b, vecSum2.16b
		eor vecSum3.16b, vecSum3.16b, vecSum3.16b
	.endif

	# Transform stride to padding
	sub strideA, strideA, width
	sub strideB, strideB, width
	lsl strideA, strideA, #(COMPV_GAS_FLOAT64_SHIFT_BYTES)
	lsl strideB, strideB, #(COMPV_GAS_FLOAT64_SHIFT_BYTES)

	##################################################
	# for (compv_uscalar_t j = 0; j < height; ++j)
	##################################################
	LoopHeight_CompVMathDotDotSub_64f64f_Asm_NEON64\@:
		##################################################
		# for (i = 0; i < width16; i += 16)
		##################################################
		cbz width16, EndOf_LoopWidth16_CompVMathDotDotSub_64f64f_Asm_NEON64\@
		mov i, width16
		LoopWidth16_CompVMathDotDotSub_64f64f_Asm_NEON64\@:
			ldp q0, q1, [ptrA, #(0 * COMPV_GAS_Q_SZ_BYTES)]
			ldp q2, q3, [ptrA, #(2 * COMPV_GAS_Q_SZ_BYTES)]
			ldp q4, q5, [ptrA, #(4 * COMPV_GAS_Q_SZ_BYTES)]
			ldp q6, q7, [ptrA, #(6 * COMPV_GAS_Q_SZ_BYTES)]
			ldp q8, q9, [ptrB, #(0 * COMPV_GAS_Q_SZ_BYTES)]
			ldp q10, q11, [ptrB, #(2 * COMPV_GAS_Q_SZ_BYTES)]
			ldp q12, q13, [ptrB, #(4 * COMPV_GAS_Q_SZ_BYTES)]
			ldp q14, q15, [ptrB, #(6 * COMPV_GAS_Q_SZ_BYTES)]
			.if \dotType == DOT_TYPE_SUB
				fsub v0.2d, v0.2d, v8.2d
				fsub v2.2d, v2.2d, v10.2d
				fsub v4.2d, v4.2d, v12.2d
				fsub v6.2d, v6.2d, v14.2d
				fsub v1.2d, v1.2d, v9.2d
				fsub v3.2d, v3.2d, v11.2d
				fsub v5.2d, v5.2d, v13.2d
				fsub v7.2d, v7.2d, v15.2d
				.if \fusedMultiplyAdd == FMA_YES
					fmla vecSum0.2d, v0.2d, v0.2d
					fmla vecSum1.2d, v2.2d, v2.2d
					fmla vecSum2.2d, v4.2d, v4.2d
					fmla vecSum3.2d, v6.2d, v6.2d
					fmla vecSum0.2d, v1.2d, v1.2d
					fmla vecSum1.2d, v3.2d, v3.2d
					fmla vecSum2.2d, v5.2d, v5.2d
					fmla vecSum3.2d, v7.2d, v7.2d
				.else
					fmul v0.2d, v0.2d, v0.2d
					fmul v2.2d, v2.2d, v2.2d
					fmul v4.2d, v4.2d, v4.2d
					fmul v6.2d, v6.2d, v6.2d
					fmul v1.2d, v1.2d, v1.2d
					fmul v3.2d, v3.2d, v3.2d
					fmul v5.2d, v5.2d, v5.2d
					fmul v7.2d, v7.2d, v7.2d
				.endif
			.else
				.if \fusedMultiplyAdd == FMA_YES
					fmla vecSum0.2d, v0.2d, v8.2d
					fmla vecSum1.2d, v2.2d, v10.2d
					fmla vecSum2.2d, v4.2d, v12.2d
					fmla vecSum3.2d, v6.2d, v14.2d
					fmla vecSum0.2d, v1.2d, v9.2d
					fmla vecSum1.2d, v3.2d, v11.2d
					fmla vecSum2.2d, v5.2d, v13.2d
					fmla vecSum3.2d, v7.2d, v15.2d
				.else
					fmul v0.2d, v0.2d, v8.2d
					fmul v2.2d, v2.2d, v10.2d
					fmul v4.2d, v4.2d, v12.2d
					fmul v6.2d, v6.2d, v14.2d
					fmul v1.2d, v1.2d, v9.2d
					fmul v3.2d, v3.2d, v11.2d
					fmul v5.2d, v5.2d, v13.2d
					fmul v7.2d, v7.2d, v15.2d
				.endif
			.endif
			.if \fusedMultiplyAdd == FMA_NO
				fadd v0.2d, v0.2d, v2.2d
				fadd v4.2d, v4.2d, v6.2d
				fadd v1.2d, v1.2d, v3.2d
				fadd v5.2d, v5.2d, v7.2d
				fadd v0.2d, v0.2d, v4.2d
				fadd v1.2d, v1.2d, v5.2d
				fadd vecSum0.2d, vecSum0.2d, v0.2d
				fadd vecSum1.2d, vecSum1.2d, v1.2d
			.endif
			subs i, i, #16
			add ptrA, ptrA, #(16 * COMPV_GAS_FLOAT64_SZ_BYTES)
			add ptrB, ptrB, #(16 * COMPV_GAS_FLOAT64_SZ_BYTES)
			bne LoopWidth16_CompVMathDotDotSub_64f64f_Asm_NEON64\@
		EndOf_LoopWidth16_CompVMathDotDotSub_64f64f_Asm_NEON64\@:

		#############################################
		# for (; i < width2; i += 2)
		#############################################
		subs i, width2, width16
		beq EndOf_LoopWidth2_CompVMathDotDotSub_64f64f_Asm_NEON64\@
		LoopWidth2_CompVMathDotDotSub_64f64f_Asm_NEON64\@:
			ldr q0, [ptrA], #(1 * COMPV_GAS_Q_SZ_BYTES)
			ldr q8, [ptrB], #(1 * COMPV_GAS_Q_SZ_BYTES)
			.if \dotType == DOT_TYPE_SUB
				fsub v0.2d, v0.2d, v8.2d
				.if \fusedMultiplyAdd == FMA_YES
					fmla vecSum0.2d, v0.2d, v0.2d
				.else
					fmul v0.2d, v0.2d, v0.2d
				.endif
			.else
				.if \fusedMultiplyAdd == FMA_YES
					fmla vecSum0.2d, v0.2d, v8.2d
				.else
					fmul v0.2d, v0.2d, v8.2d
				.endif
			.endif
			.if \fusedMultiplyAdd == FMA_NO
				fadd vecSum0.2d, vecSum0.2d, v0.2d
			.endif
			subs i, i, #2
			bne LoopWidth2_CompVMathDotDotSub_64f64f_Asm_NEON64\@
		EndOf_LoopWidth2_CompVMathDotDotSub_64f64f_Asm_NEON64\@:

		#############################################
		# for (; i < width1; i += 1)
		#############################################
		cmp width, width2
		beq EndOf_LoopWidth1_CompVMathDotDotSub_64f64f_Asm_NEON64\@
		LoopWidth1_CompVMathDotDotSub_64f64f_Asm_NEON64\@:
			ldr vecLowAd, [ptrA], #(1 * COMPV_GAS_FLOAT64_SZ_BYTES)
			ldr vecLowBd, [ptrB], #(1 * COMPV_GAS_FLOAT64_SZ_BYTES)
			.if \dotType == DOT_TYPE_SUB
				fsub vecLowAd, vecLowAd, vecLowBd
				.if \fusedMultiplyAdd == FMA_YES
					fmla vecSum0.2d, vecLowA.2d, vecLowA.2d
				.else
					fmul vecLowAd, vecLowAd, vecLowAd
				.endif
			.else
				.if \fusedMultiplyAdd == FMA_YES
					fmla vecSum0.2d, vecLowA.2d, vecLowB.2d
				.else
					fmul vecLowAd, vecLowAd, vecLowBd
				.endif
			.endif
			.if \fusedMultiplyAdd == FMA_NO
				fadd vecSum0.2d, vecSum0.2d, vecLowA.2d
			.endif
		EndOf_LoopWidth1_CompVMathDotDotSub_64f64f_Asm_NEON64\@:

		subs height, height, #1
		add ptrA, ptrA, strideA
		add ptrB, ptrB, strideB
		bne LoopHeight_CompVMathDotDotSub_64f64f_Asm_NEON64\@
	EndOf_LoopHeight_CompVMathDotDotSub_64f64f_Asm_NEON64\@:


	fadd vecSum0.2d, vecSum0.2d, vecSum1.2d
	.if \fusedMultiplyAdd == FMA_YES
		fadd vecSum2.2d, vecSum2.2d, vecSum3.2d
		fadd vecSum0.2d, vecSum0.2d, vecSum2.2d
	.endif
	mov d0, vecSum0.d[0]
	mov d1, vecSum0.d[1]
	fadd d0, d0, d1
	str d0, [out]

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
	.unreq vecLowA
	.unreq vecLowAd
	.unreq vecLowB
	.unreq vecLowBd
	.unreq vecSum0
	.unreq vecSum1
	.if \fusedMultiplyAdd == FMA_YES
		.unreq vecSum2
		.unreq vecSum3
	.endif

	## begin epilog ##
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

################################################################
COMPV_GAS_FUNCTION_DECLARE CompVMathDotDotSub_64f64f_Asm_NEON64
	CompVMathDotDotSub_64f64f_Macro_NEON64 DOT_TYPE_SUB, FMA_NO

################################################################
COMPV_GAS_FUNCTION_DECLARE CompVMathDotDotSub_64f64f_Asm_FMA_NEON64
	CompVMathDotDotSub_64f64f_Macro_NEON64 DOT_TYPE_SUB, FMA_YES

################################################################
COMPV_GAS_FUNCTION_DECLARE CompVMathDotDot_64f64f_Asm_NEON64
	CompVMathDotDotSub_64f64f_Macro_NEON64 DOT_TYPE_DOT, FMA_NO

################################################################
COMPV_GAS_FUNCTION_DECLARE CompVMathDotDot_64f64f_Asm_FMA_NEON64
	CompVMathDotDotSub_64f64f_Macro_NEON64 DOT_TYPE_DOT, FMA_YES

#endif /* defined(__aarch64__) */
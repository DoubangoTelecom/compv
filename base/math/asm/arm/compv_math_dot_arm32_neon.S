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

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> const compv_float64_t* ptrA
@ arg(1) -> const compv_float64_t* ptrB
@ arg(2) -> const compv_uscalar_t width
@ arg(3) -> const compv_uscalar_t height
@ arg(4) -> const compv_uscalar_t strideA
@ arg(5) -> const compv_uscalar_t strideB
@ arg(6) -> compv_float64_t* ret
COMPV_GAS_FUNCTION_DECLARE CompVMathDotDotSub_64f64f_Asm_NEON32
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
	ret .req r6

	@ Local Variables @
    i .req r7
	width16 .req r8
	width2 .req r9
	vecSum0 .req q14
	vecSum1 .req q15

	pld [ptrA, #(CACHE_LINE_SIZE*0)]
	pld [ptrA, #(CACHE_LINE_SIZE*1)]
	pld [ptrA, #(CACHE_LINE_SIZE*2)]
	pld [ptrA, #(CACHE_LINE_SIZE*3)]
	pld [ptrB, #(CACHE_LINE_SIZE*0)]
	pld [ptrB, #(CACHE_LINE_SIZE*1)]
	pld [ptrB, #(CACHE_LINE_SIZE*2)]
	pld [ptrB, #(CACHE_LINE_SIZE*3)]

	and width16, width, #-16
	and width2, width, #-2

	@ Reset vecSum0 and vecSum1 to zeros
	veor.f64  vecSum0, vecSum0, vecSum0
	veor.f64  vecSum1, vecSum1, vecSum1

	@ Transform stride to padding
	sub strideA, strideA, width
	sub strideB, strideB, width
	lsl strideA, strideA, #(COMPV_GAS_FLOAT64_SHIFT_BYTES)
	lsl strideB, strideB, #(COMPV_GAS_FLOAT64_SHIFT_BYTES)

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (compv_uscalar_t j = 0; j < height; ++j)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopHeight_CompVMathDotDotSub_64f64f_Asm_NEON32:
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (i = 0; i < width16; i += 16)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		movs i, width16
		beq EndOf_LoopWidth16_CompVMathDotDotSub_64f64f_Asm_NEON32
		LoopWidth16_CompVMathDotDotSub_64f64f_Asm_NEON32:
			vldm ptrA!, { q0-q3 }
			vldm ptrB!, { q4-q7 }
			pld [ptrA, #(CACHE_LINE_SIZE*4)]
			pld [ptrB, #(CACHE_LINE_SIZE*4)]
			vsub.f64 q0x, q0x, q4x
			vsub.f64 q0y, q0y, q4y
			vsub.f64 q1x, q1x, q5x
			vsub.f64 q1y, q1y, q5y
			vsub.f64 q2x, q2x, q6x
			vsub.f64 q2y, q2y, q6y
			vsub.f64 q3x, q3x, q7x
			vsub.f64 q3y, q3y, q7y
			vldm ptrA!, { q4-q7 }
			vldm ptrB!, { q8-q11 }
			pld [ptrA, #(CACHE_LINE_SIZE*4)]
			pld [ptrB, #(CACHE_LINE_SIZE*4)]
			vsub.f64 q4x, q4x, q8x
			vsub.f64 q4y, q4y, q8y
			vsub.f64 q5x, q5x, q9x
			vsub.f64 q5y, q5y, q9y
			vsub.f64 q6x, q6x, q10x
			vsub.f64 q6y, q6y, q10y
			vsub.f64 q7x, q7x, q11x
			vsub.f64 q7y, q7y, q11y
			@ TODO(dmi): Add FMA implementation
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
			@vecSum0 = vaddq_f64(vecSum0, vec0);
			@vecSum1 = vaddq_f64(vecSum1, vec1);

			subs i, i, #16
			add ptrA, ptrA, #(16 * COMPV_GAS_FLOAT64_SZ_BYTES)
			add ptrB, ptrB, #(16 * COMPV_GAS_FLOAT64_SZ_BYTES)
			bne LoopWidth16_CompVMathDotDotSub_64f64f_Asm_NEON32
		EndOf_LoopWidth16_CompVMathDotDotSub_64f64f_Asm_NEON32:

		subs height, height, #1
		bne LoopHeight_CompVMathDotDotSub_64f64f_Asm_NEON32
	EndOf_LoopHeight_CompVMathDotDotSub_64f64f_Asm_NEON32:

	.unreq ptrA
	.unreq ptrB
	.unreq width
	.unreq height
	.unreq strideA
	.unreq strideB
	.unreq ret

    .unreq i
	.unreq width16
	.unreq width2
	.unreq vecSum0
	.unreq vecSum1

	@@ begin epilog @@
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 7
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__arm__) && !defined(__aarch64__) */

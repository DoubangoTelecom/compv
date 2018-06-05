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
# arg(0) -> COMPV_ALIGNED(NEON) const compv_float32_t* Aptr
# arg(1) -> COMPV_ALIGNED(NEON) const compv_float32_t* Bptr
# arg(2) -> COMPV_ALIGNED(NEON) compv_float32_t* Rptr
# arg(3) -> const compv_uscalar_t Bcols
# arg(4) -> const compv_uscalar_t Arows
# arg(5) -> const compv_uscalar_t Brows
# arg(6) -> COMPV_ALIGNED(NEON) const compv_uscalar_t Astride
# arg(7) -> COMPV_ALIGNED(NEON) const compv_uscalar_t Bstride
# arg(8) -> COMPV_ALIGNED(NEON) const compv_uscalar_t Rstride
.macro CompVMathOpMulMulABt_32f32f32f_Macro_NEON64 fusedMultiplyAdd
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Load arguments ##
	ldr r8, [bp, #(prolog_bytes + (0*COMPV_GAS_REG_SZ_BYTES))]
	Aptr .req r0
	Bptr .req r1
	Rptr .req r2
	Bcols .req r3
	Arows .req r4
	Brows .req r5
	Astride .req r6
	Bstride .req r7
	Rstride .req r8

	k .req r9
	j .req r10
	Bcols16 .req r11
	Bcols4 .req r12
	Bptr0 .req r13

	prfm pldl1keep, [Aptr, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [Aptr, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [Aptr, #(CACHE_LINE_SIZE*2)]
	prfm pldl1keep, [Aptr, #(CACHE_LINE_SIZE*3)]
	prfm pldl1keep, [Bptr, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [Bptr, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [Bptr, #(CACHE_LINE_SIZE*2)]
	prfm pldl1keep, [Bptr, #(CACHE_LINE_SIZE*3)]

	ands Bcols16, Bcols, #-16
	ands Bcols4, Bcols, #-4

	# Transform strides to padding #
	sub Bstride, Bstride, Bcols
	sub Rstride, Rstride, Brows
	lsl Astride, Astride, #(COMPV_GAS_FLOAT32_SHIFT_BYTES)
	lsl Bstride, Bstride, #(COMPV_GAS_FLOAT32_SHIFT_BYTES)
	lsl Rstride, Rstride, #(COMPV_GAS_FLOAT32_SHIFT_BYTES)	

	################################################
	# for (compv_uscalar_t i = 0; i < Arows; ++i)
	################################################
	LoopArows_CompVMathOpMulMulABt_32f32f32f_Asm_NEON64\@:
		mov Bptr0, Bptr
		################################################
		# for (compv_uscalar_t j = 0; j < Brows; ++j)
		################################################
		mov j, Brows
		LoopBrows_CompVMathOpMulMulABt_32f32f32f_Asm_NEON64\@:
			eor v0.16b, v0.16b, v0.16b
			eor v1.16b, v1.16b, v1.16b
			eor v2.16b, v2.16b, v2.16b
			eor v3.16b, v3.16b, v3.16b
			################################################
			# for (k = 0; k < Bcols16; k += 16)
			################################################
			mov k, #0
			cbz Bcols16, EndOf_LoopBcols16_CompVMathOpMulMulABt_32f32f32f_Asm_NEON64\@
			LoopBcols16_CompVMathOpMulMulABt_32f32f32f_Asm_NEON64\@:
				ldp q4, q5, [Aptr], #(2*COMPV_GAS_Q_SZ_BYTES)
				ldp q6, q7, [Aptr], #(2*COMPV_GAS_Q_SZ_BYTES)
				ldp q8, q9, [Bptr0], #(2*COMPV_GAS_Q_SZ_BYTES)
				ldp q10, q11, [Bptr0], #(2*COMPV_GAS_Q_SZ_BYTES)
				prfm pldl1keep, [Aptr, #(CACHE_LINE_SIZE*4)]
				prfm pldl1keep, [Bptr0, #(CACHE_LINE_SIZE*4)]
				.if \fusedMultiplyAdd
					fmla v0.4s, v4.4s, v8.4s
					fmla v1.4s, v5.4s, v9.4s
					fmla v2.4s, v6.4s, v10.4s
					fmla v3.4s, v7.4s, v11.4s
				.else
					fmul v4.4s, v4.4s, v8.4s
					fmul v5.4s, v5.4s, v9.4s
					fmul v6.4s, v6.4s, v10.4s
					fmul v7.4s, v7.4s, v11.4s
					fadd v0.4s, v0.4s, v4.4s
					fadd v1.4s, v1.4s, v5.4s
					fadd v2.4s, v2.4s, v6.4s
					fadd v3.4s, v3.4s, v7.4s
				.endif
				add k, k, #16
				cmp k, Bcols16
				blt LoopBcols16_CompVMathOpMulMulABt_32f32f32f_Asm_NEON64\@
			EndOf_LoopBcols16_CompVMathOpMulMulABt_32f32f32f_Asm_NEON64\@:

			################################################
			# for (; k < Bcols4; k += 4)
			################################################
			cmp k, Bcols4
			bge EndOf_LoopBcols4_CompVMathOpMulMulABt_32f32f32f_Asm_NEON64\@
			LoopBcols4_CompVMathOpMulMulABt_32f32f32f_Asm_NEON64\@:
				ldr q4, [Aptr], #(1*COMPV_GAS_Q_SZ_BYTES) 
				ldr q8, [Bptr0], #(1*COMPV_GAS_Q_SZ_BYTES)
				.if \fusedMultiplyAdd
					fmla v0.4s, v4.4s, v8.4s
				.else
					fmul v4.4s, v4.4s, v8.4s
					fadd v0.4s, v0.4s, v4.4s
				.endif
				add k, k, #4
				cmp k, Bcols4
				blt LoopBcols4_CompVMathOpMulMulABt_32f32f32f_Asm_NEON64\@
			EndOf_LoopBcols4_CompVMathOpMulMulABt_32f32f32f_Asm_NEON64\@:

			fadd v0.4s, v0.4s, v1.4s
			fadd v2.4s, v2.4s, v3.4s
			fadd v0.4s, v0.4s, v2.4s
			ext v3.16b, v0.16b, v0.16b, #8
			fadd v0.2s, v0.2s, v3.2s
			faddp v0.2s, v0.2s, v0.2s

			################################################
			# for (; k < Bcols; k += 1)
			################################################
			cmp k, Bcols
			bge EndOf_LoopBcols1_CompVMathOpMulMulABt_32f32f32f_Asm_NEON64\@
			LoopBcols1_CompVMathOpMulMulABt_32f32f32f_Asm_NEON64\@:
				ld1 {v4.s}[0], [Aptr], #(1*COMPV_GAS_FLOAT32_SZ_BYTES) // s4 = Aptr
				ld1 {v8.s}[0], [Bptr0], #(1*COMPV_GAS_FLOAT32_SZ_BYTES) // s8 = Bptr0
				.if \fusedMultiplyAdd
					fmadd s0, s4, s8, s0
				.else
					fmul s4, s4, s8
					fadd s0, s0, s4
				.endif
				add k, k, #1
				cmp k, Bcols
				blt LoopBcols1_CompVMathOpMulMulABt_32f32f32f_Asm_NEON64\@
			EndOf_LoopBcols1_CompVMathOpMulMulABt_32f32f32f_Asm_NEON64\@:

			st1 {v0.s}[0], [Rptr], #(1*COMPV_GAS_FLOAT32_SZ_BYTES)
			subs j, j, #1
			sub Aptr, Aptr, Bcols, LSL #(COMPV_GAS_FLOAT32_SHIFT_BYTES) // rollback
			add Bptr0, Bptr0, Bstride
			bne LoopBrows_CompVMathOpMulMulABt_32f32f32f_Asm_NEON64\@
		EndOf_LoopBrows_CompVMathOpMulMulABt_32f32f32f_Asm_NEON64\@:

		subs Arows, Arows, #1
		add Aptr, Aptr, Astride
		add Rptr, Rptr, Rstride
		bne LoopArows_CompVMathOpMulMulABt_32f32f32f_Asm_NEON64\@
	EndOf_LoopArows_CompVMathOpMulMulABt_32f32f32f_Asm_NEON64\@:

	.unreq Aptr
	.unreq Bptr
	.unreq Rptr
	.unreq Bcols
	.unreq Arows
	.unreq Brows
	.unreq Astride
	.unreq Bstride
	.unreq Rstride

	.unreq k
	.unreq j
	.unreq Bcols16
	.unreq Bcols4
	.unreq Bptr0

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVMathOpMulMulABt_32f32f32f_Asm_NEON64
	CompVMathOpMulMulABt_32f32f32f_Macro_NEON64 0

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVMathOpMulMulABt_32f32f32f_Asm_FMA_NEON64
	CompVMathOpMulMulABt_32f32f32f_Macro_NEON64 1

#endif /* defined(__aarch64__) */
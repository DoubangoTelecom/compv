#########################################################################
# Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__arm__) && !defined(__aarch64__)
.include "compv_common_arm32.S"

.data

.extern
 
.text

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const compv_float32_t* Aptr
@ arg(1) -> COMPV_ALIGNED(NEON) const compv_float32_t* Bptr
@ arg(2) -> COMPV_ALIGNED(NEON) compv_float32_t* Rptr
@ arg(3) -> const compv_uscalar_t Bcols
@ arg(4) -> const compv_uscalar_t Arows
@ arg(5) -> const compv_uscalar_t Brows
@ arg(6) -> COMPV_ALIGNED(NEON) const compv_uscalar_t Astride
@ arg(7) -> COMPV_ALIGNED(NEON) const compv_uscalar_t Bstride
@ arg(8) -> COMPV_ALIGNED(NEON) const compv_uscalar_t Rstride
COMPV_GAS_FUNCTION_DECLARE CompVMathOpMulMulABt_32f32f32f_Asm_NEON32
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 9
	COMPV_GAS_SAVE_NEON_REGS

	@@ Load arguments @@
	ldm_args r0-r8
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
	Bcolsx .req r11

	pld [Aptr, #(CACHE_LINE_SIZE*0)]
	pld [Aptr, #(CACHE_LINE_SIZE*1)]
	pld [Aptr, #(CACHE_LINE_SIZE*2)]
	pld [Aptr, #(CACHE_LINE_SIZE*3)]
	pld [Bptr, #(CACHE_LINE_SIZE*0)]
	pld [Bptr, #(CACHE_LINE_SIZE*1)]
	pld [Bptr, #(CACHE_LINE_SIZE*2)]
	pld [Bptr, #(CACHE_LINE_SIZE*3)]

	@ Transform strides to padding @
	sub Bstride, Bstride, Bcols
	sub Rstride, Rstride, Brows
	lsl Astride, Astride, #(COMPV_GAS_FLOAT32_SHIFT_BYTES)
	lsl Bstride, Bstride, #(COMPV_GAS_FLOAT32_SHIFT_BYTES)
	lsl Rstride, Rstride, #(COMPV_GAS_FLOAT32_SHIFT_BYTES)	

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (compv_uscalar_t i = 0; i < Arows; ++i)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopArows_CompVMathOpMulMulABt_32f32f32f_Asm_NEON32:
		ldr_arg 1, Bptr
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (compv_uscalar_t j = 0; j < Brows; ++j)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		mov j, #0
		LoopBrows_CompVMathOpMulMulABt_32f32f32f_Asm_NEON32:
			veor.f32 q0, q0, q0
			veor.f32 q1, q1, q1
			veor.f32 q2, q2, q2
			veor.f32 q3, q3, q3
			@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			@ for (k = 0; k < Bcols16; k += 16)
			@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			mov k, #0
			ands Bcolsx, Bcols, #-16
			beq EndOf_LoopBcols16_CompVMathOpMulMulABt_32f32f32f_Asm_NEON32
			LoopBcols16_CompVMathOpMulMulABt_32f32f32f_Asm_NEON32:
				vld1.f32 { q4, q5 }, [Aptr :128]!
				vld1.f32 { q6, q7 }, [Aptr :128]!
				vld1.f32 { q8, q9 }, [Bptr :128]!
				vld1.f32 { q10, q11 }, [Bptr :128]!
				pld [Aptr, #(CACHE_LINE_SIZE*4)]
				pld [Bptr, #(CACHE_LINE_SIZE*4)]
				vmla.f32 q0, q4, q8
				vmla.f32 q1, q5, q9
				vmla.f32 q2, q6, q10
				vmla.f32 q3, q7, q11
				add k, k, #16
				cmp k, Bcolsx
				blt LoopBcols16_CompVMathOpMulMulABt_32f32f32f_Asm_NEON32
			EndOf_LoopBcols16_CompVMathOpMulMulABt_32f32f32f_Asm_NEON32:

			@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			@ for (; k < Bcols4; k += 4)
			@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			and Bcolsx, Bcols, #-4
			cmp k, Bcolsx
			bge EndOf_LoopBcols4_CompVMathOpMulMulABt_32f32f32f_Asm_NEON32
			LoopBcols4_CompVMathOpMulMulABt_32f32f32f_Asm_NEON32:
				vld1.f32 { q4 }, [Aptr :128]!
				vld1.f32 { q8 }, [Bptr :128]!
				vmla.f32 q0, q4, q8
				add k, k, #4
				cmp k, Bcolsx
				blt LoopBcols4_CompVMathOpMulMulABt_32f32f32f_Asm_NEON32
			EndOf_LoopBcols4_CompVMathOpMulMulABt_32f32f32f_Asm_NEON32:

			vadd.f32 q0, q0, q1
			vadd.f32 q2, q2, q3
			vadd.f32 q0, q0, q2
			vadd.f32 q0x, q0x, q0y
			vpadd.f32 q0x, q0x, q0x

			@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			@ for (; k < Bcols; k += 1)
			@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			cmp k, Bcols
			bge EndOf_LoopBcols1_CompVMathOpMulMulABt_32f32f32f_Asm_NEON32
			LoopBcols1_CompVMathOpMulMulABt_32f32f32f_Asm_NEON32:
				vld1.f32 { d1[0] }, [Aptr]! @ s2 = Aptr
				vld1.f32 { d2[0] }, [Bptr]! @ s4 = Bptr
				vmla.f32 s0, s2, s4 @ s0 += (s2 * s4) 
				add k, k, #1
				cmp k, Bcols
				blt LoopBcols1_CompVMathOpMulMulABt_32f32f32f_Asm_NEON32
			EndOf_LoopBcols1_CompVMathOpMulMulABt_32f32f32f_Asm_NEON32:

			vst1.f32 { d0[0] }, [Rptr]! @ s0 = sum
			add j, j, #1
			sub Aptr, Aptr, Bcols, LSL #(COMPV_GAS_FLOAT32_SHIFT_BYTES) @ rollback
			add Bptr, Bptr, Bstride
			cmp j, Brows
			blt LoopBrows_CompVMathOpMulMulABt_32f32f32f_Asm_NEON32
		EndOf_LoopBrows_CompVMathOpMulMulABt_32f32f32f_Asm_NEON32:

		subs Arows, Arows, #1
		add Aptr, Aptr, Astride
		add Rptr, Rptr, Rstride
		bne LoopArows_CompVMathOpMulMulABt_32f32f32f_Asm_NEON32
	EndOf_LoopArows_CompVMathOpMulMulABt_32f32f32f_Asm_NEON32:

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
	.unreq Bcolsx

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 9
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__arm__) && !defined(__aarch64__) */

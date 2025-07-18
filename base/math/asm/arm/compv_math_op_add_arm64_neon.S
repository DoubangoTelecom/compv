#########################################################################
# Copyright (C) 2016-2025 Doubango Telecom <https://www.doubango.org>   #
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
# arg(3) -> const compv_uscalar_t width
# arg(4) -> const compv_uscalar_t height
# arg(5) -> COMPV_ALIGNED(NEON) const compv_uscalar_t Astride
# arg(6) -> COMPV_ALIGNED(NEON) const compv_uscalar_t Bstride
# arg(7) -> COMPV_ALIGNED(NEON) const compv_uscalar_t Rstride
COMPV_GAS_FUNCTION_DECLARE CompVMathOpAddAdd_32f32f32f_Asm_NEON64
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Load arguments ##
	Aptr .req r0
	Bptr .req r1
	Rptr .req r2
	width .req r3
	height .req r4
	Astride .req r5
	Bstride .req r6
	Rstride .req r7

	width16 .req r8
	i .req r9

	and width16, width, #-16

	# Transform strides to padding #
	add r11, width, #3
	and r11, r11, #-4
	sub Astride, Astride, r11
	sub Bstride, Bstride, r11
	sub Rstride, Rstride, r11
	lsl Astride, Astride, #(COMPV_GAS_FLOAT32_SHIFT_BYTES)
	lsl Bstride, Bstride, #(COMPV_GAS_FLOAT32_SHIFT_BYTES)
	lsl Rstride, Rstride, #(COMPV_GAS_FLOAT32_SHIFT_BYTES)

	prfm pldl1keep, [Aptr, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [Aptr, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [Aptr, #(CACHE_LINE_SIZE*2)]
	prfm pldl1keep, [Aptr, #(CACHE_LINE_SIZE*3)]
	prfm pldl1keep, [Bptr, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [Bptr, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [Bptr, #(CACHE_LINE_SIZE*2)]
	prfm pldl1keep, [Bptr, #(CACHE_LINE_SIZE*3)]

	################################################
	# for (compv_uscalar_t j = 0; j < height; ++j)
	################################################
	LoopHeigth_CompVMathOpAddAdd_32f32f32f_Asm_NEON64:
		################################################
		# for (i = 0; i < width16; i += 16)
		################################################
		mov i, #0
		tst width16, width16
		beq EndOf_LoopWidth16_CompVMathOpAddAdd_32f32f32f_Asm_NEON64
		LoopWidth16_CompVMathOpAddAdd_32f32f32f_Asm_NEON64:
			ldp q0, q1, [Aptr], #(2*COMPV_GAS_Q_SZ_BYTES)
			ldp q2, q3, [Aptr], #(2*COMPV_GAS_Q_SZ_BYTES)
			ldp q4, q5, [Bptr], #(2*COMPV_GAS_Q_SZ_BYTES)
			ldp q6, q7, [Bptr], #(2*COMPV_GAS_Q_SZ_BYTES)
			prfm pldl1keep, [Aptr, #(CACHE_LINE_SIZE*4)]
			prfm pldl1keep, [Bptr, #(CACHE_LINE_SIZE*4)]
			fadd v0.4s, v0.4s, v4.4s
			fadd v1.4s, v1.4s, v5.4s
			fadd v2.4s, v2.4s, v6.4s
			fadd v3.4s, v3.4s, v7.4s
			stp q0, q1, [Rptr], #(2*COMPV_GAS_Q_SZ_BYTES)
			stp q2, q3, [Rptr], #(2*COMPV_GAS_Q_SZ_BYTES)
			add i, i, #16
			cmp i, width16
			blt  LoopWidth16_CompVMathOpAddAdd_32f32f32f_Asm_NEON64
		EndOf_LoopWidth16_CompVMathOpAddAdd_32f32f32f_Asm_NEON64:
		
		################################################
		# for (; i < width; i += 4)
		################################################
		cmp i, width
		bge EndOf_LoopWidth4_CompVMathOpAddAdd_32f32f32f_Asm_NEON64
		LoopWidth4_CompVMathOpAddAdd_32f32f32f_Asm_NEON64:
			ldr q0, [Aptr], #(1*COMPV_GAS_Q_SZ_BYTES)
			ldr q4, [Bptr], #(1*COMPV_GAS_Q_SZ_BYTES)
			fadd v0.4s, v0.4s, v4.4s
			str q0, [Rptr], #(1*COMPV_GAS_Q_SZ_BYTES)
			add i, i, #4
			cmp i, width
			blt LoopWidth4_CompVMathOpAddAdd_32f32f32f_Asm_NEON64
		EndOf_LoopWidth4_CompVMathOpAddAdd_32f32f32f_Asm_NEON64:

		subs height, height, #1
		add Aptr, Aptr, Astride
		add Bptr, Bptr, Bstride
		add Rptr, Rptr, Rstride
		bne  LoopHeigth_CompVMathOpAddAdd_32f32f32f_Asm_NEON64
	EndOf_LoopHeigth_CompVMathOpAddAdd_32f32f32f_Asm_NEON64:

	.unreq Aptr
	.unreq Bptr
	.unreq Rptr
	.unreq width
	.unreq height
	.unreq Astride
	.unreq Bstride
	.unreq Rstride

	.unreq width16
	.unreq i

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#endif /* defined(__aarch64__) */
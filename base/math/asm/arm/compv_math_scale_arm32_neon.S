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
@ arg(0) -> COMPV_ALIGNED(NEON) const compv_float32_t* ptrIn
@ arg(1) -> COMPV_ALIGNED(NEON) compv_float32_t* ptrOut
@ arg(2) -> const compv_uscalar_t width
@ arg(3) -> const compv_uscalar_t height
@ arg(4) -> COMPV_ALIGNED(NEON) const compv_uscalar_t stride
@ arg(5) -> const compv_float32_t* scale1
COMPV_GAS_FUNCTION_DECLARE CompVMathScaleScale_32f32f_Asm_NEON32
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 6
	COMPV_GAS_SAVE_NEON_REGS

	@@ Load arguments @@
	ldm_args r0-r5
	ptrIn .req r0
	ptrOut .req r1
	width .req r2
	height .req r3
	stride .req r4
	scale1 .req r5

	width32 .req r6
	i .req r7

	pld [ptrIn, #(CACHE_LINE_SIZE*0)]
	pld [ptrIn, #(CACHE_LINE_SIZE*1)]
	pld [ptrIn, #(CACHE_LINE_SIZE*2)]
	pld [ptrIn, #(CACHE_LINE_SIZE*3)]

	and width32, width, #-32

	@ Transform stride to padding @
	add i, width, #3
	and i, i, #-4
	sub stride, stride, i
	lsl stride, stride, #(COMPV_GAS_FLOAT32_SHIFT_BYTES)

	vld1.32 {q15x[], q15y[]}, [scale1] @ duplicate

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (compv_uscalar_t j = 0; j < height; ++j)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopHeigth_CompVMathScaleScale_32f32f_Asm_NEON32:
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (i = 0; i < width32; i += 32)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		mov i, #0
		tst width32, width32
		beq EndOf_LoopWidth32_CompVMathScaleScale_32f32f_Asm_NEON32
		LoopWidth32_CompVMathScaleScale_32f32f_Asm_NEON32:
			vld1.f32 { q0, q1 }, [ptrIn :128]!
			vld1.f32 { q2, q3 }, [ptrIn :128]!
			vld1.f32 { q4, q5 }, [ptrIn :128]!
			vld1.f32 { q6, q7 }, [ptrIn :128]!
			pld [ptrIn, #(CACHE_LINE_SIZE*4)]
			vmul.f32 q0, q0, q15
			vmul.f32 q1, q1, q15
			vmul.f32 q2, q2, q15
			vmul.f32 q3, q3, q15
			vmul.f32 q4, q4, q15
			vmul.f32 q5, q5, q15
			vmul.f32 q6, q6, q15
			vmul.f32 q7, q7, q15
			vst1.f32 { q0, q1 }, [ptrOut :128]!
			vst1.f32 { q2, q3 }, [ptrOut :128]!
			vst1.f32 { q4, q5 }, [ptrOut :128]!
			vst1.f32 { q6, q7 }, [ptrOut :128]!
			add i, i, #32
			cmp i, width32
			blt  LoopWidth32_CompVMathScaleScale_32f32f_Asm_NEON32
		EndOf_LoopWidth32_CompVMathScaleScale_32f32f_Asm_NEON32:
		
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (; i < width; i += 4)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		cmp i, width
		bge EndOf_LoopWidth_CompVMathScaleScale_32f32f_Asm_NEON32
		LoopWidth_CompVMathScaleScale_32f32f_Asm_NEON32:
			vld1.f32 { q0 }, [ptrIn :128]!
			vmul.f32 q0, q0, q15
			vst1.f32 { q0 }, [ptrOut :128]!
			add i, i, #4
			cmp i, width
			blt LoopWidth_CompVMathScaleScale_32f32f_Asm_NEON32
		EndOf_LoopWidth_CompVMathScaleScale_32f32f_Asm_NEON32:

		subs height, height, #1
		add ptrIn, ptrIn, stride
		add ptrOut, ptrOut, stride
		bne  LoopHeigth_CompVMathScaleScale_32f32f_Asm_NEON32
	EndOf_LoopHeigth_CompVMathScaleScale_32f32f_Asm_NEON32:

	.unreq ptrIn
	.unreq ptrOut
	.unreq width
	.unreq height
	.unreq stride
	.unreq scale1

	.unreq width32
	.unreq i

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 6
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__arm__) && !defined(__aarch64__) */

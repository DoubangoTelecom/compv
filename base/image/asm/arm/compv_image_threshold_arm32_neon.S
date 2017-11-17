#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__arm__) && !defined(__aarch64__)
.include "compv_common_arm32.S"

.data
 
.text


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* inPtr,
@ arg(1) -> COMPV_ALIGNED(NEON) uint8_t* outPtr,
@ arg(2) -> compv_uscalar_t width, 
@ arg(3) -> compv_uscalar_t height, 
@ arg(4) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride,
@ arg(5) -> compv_uscalar_t threshold
COMPV_GAS_FUNCTION_DECLARE CompVImageThresholdGlobal_8u8u_Asm_NEON32
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 6
	COMPV_GAS_SAVE_NEON_REGS
	
	@@ Declare input arguments @@
	ldm_args r0-r5
	inPtr .req r0 
	outPtr .req r1
	width .req r2
	height .req r3
	stride .req r4
	threshold .req r5

	@@ Declare local vectors @@
	width1 .req r6
	pad .req r7

	add pad, width, #15
	and pad, pad, #-16
	sub pad, stride, pad

	vdup.u8 q4, threshold @ q4 = vecThreshold

	pld [inPtr, #(CACHE_LINE_SIZE*0)]
	pld [inPtr, #(CACHE_LINE_SIZE*1)]
	pld [inPtr, #(CACHE_LINE_SIZE*2)]
	
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (j = 0; j < height; ++j)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopHeight_CompVImageThresholdGlobal_8u8u_Asm_NEON32:

		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (i = 0; i < width1; i += 64)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		ands width1, width, #-64
		beq EndOf_LoopWidth64_CompVImageThresholdGlobal_8u8u_Asm_NEON32
		LoopWidth64_CompVImageThresholdGlobal_8u8u_Asm_NEON32:
			pld [inPtr, #(CACHE_LINE_SIZE*3)]
			subs width1, width1, #64
			vld1.u8 {q0,q1}, [inPtr: 128]!
			vld1.u8 {q2,q3}, [inPtr: 128]!
			vcgt.u8 q0, q0, q4
			vcgt.u8 q1, q1, q4
			vcgt.u8 q2, q2, q4
			vcgt.u8 q3, q3, q4
			vst1.u8 {q0,q1}, [outPtr: 128]!
			vst1.u8 {q2,q3}, [outPtr: 128]!
			bne LoopWidth64_CompVImageThresholdGlobal_8u8u_Asm_NEON32
		EndOf_LoopWidth64_CompVImageThresholdGlobal_8u8u_Asm_NEON32:

		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (; i < width; i += 16) 
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		ands width1, width, #63
		beq EndOf_LoopWidth16_CompVImageThresholdGlobal_8u8u_Asm_NEON32
		LoopWidth16_CompVImageThresholdGlobal_8u8u_Asm_NEON32:
			vld1.u8 {q0}, [inPtr: 128]!
			vcgt.u8 q0, q0, q4
			vst1.u8 {q0}, [outPtr: 128]!
			subs width1, width1, #16
			bgt LoopWidth16_CompVImageThresholdGlobal_8u8u_Asm_NEON32
		EndOf_LoopWidth16_CompVImageThresholdGlobal_8u8u_Asm_NEON32:
		
		subs height, height, #1
		add inPtr, inPtr, pad
		add outPtr, outPtr, pad
		bne LoopHeight_CompVImageThresholdGlobal_8u8u_Asm_NEON32

	EndOf_LoopHeight_CompVImageThresholdGlobal_8u8u_Asm_NEON32:
	
	.unreq inPtr
	.unreq outPtr
	.unreq width
	.unreq height
	.unreq stride
	.unreq threshold
	.unreq width1
	.unreq pad

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 6
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#endif /* defined(__arm__) && !defined(__aarch64__) */

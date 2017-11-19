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
 .align 4
data0123: .word 0x0, 0x1, 0x2, 0x3

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

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(1) -> CCOMPV_ALIGNED(NEON) const uint32_t* ptr32uHistogram
@ arg(2) -> CCOMPV_ALIGNED(NEON) uint32_t* sumA256
@ arg(3) -> Cuint32_t* sumB1
COMPV_GAS_FUNCTION_DECLARE CompVImageThresholdOtsuSum_32s32s_Asm_NEON32
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	ptr32uHistogram .req r0
	sumA256 .req r1
	sumB1 .req r2
	i .req r3
	vecIndicesInc .req q0
	vecIndices0 .req q1
	vecIndices1 .req q2
	vecIndices2 .req q3
	vecIndices3 .req q4
	vec0 .req q5
	vec1 .req q6
	vec2 .req q7
	vec3 .req q8
	vecSumB0 .req q9 @ must be q9 (used later)
	vecSumB1 .req q10
	vecSumB2 .req q11
	vecSumB3 .req q12

	mov r4, #16
	vdup.u32 vecIndicesInc, r4

	mov r4, #4
	ldr r5, =data0123
	vdup.u32 q14, r4
	vld1.u32 {vecIndices0}, [r5]
	vadd.u32 vecIndices1, vecIndices0, q14
	vadd.u32 vecIndices2, vecIndices1, q14
	vadd.u32 vecIndices3, vecIndices2, q14

	veor.u32 vecSumB0, vecSumB0, vecSumB0
	veor.u32 vecSumB1, vecSumB1, vecSumB1
	veor.u32 vecSumB2, vecSumB2, vecSumB2
	veor.u32 vecSumB3, vecSumB3, vecSumB3

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (size_t i = 0; i < 256; i += 16)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	mov i, #256
	LoopWidth_CompVImageThresholdOtsuSum_32s32s_Asm_NEON32:
		vld1.u32 {vec0,vec1}, [ptr32uHistogram: 128]!
		vld1.u32 {vec2,vec3}, [ptr32uHistogram: 128]!
		vmul.u32 vec0, vec0, vecIndices0
		vmul.u32 vec1, vec1, vecIndices1
		vmul.u32 vec2, vec2, vecIndices2
		vmul.u32 vec3, vec3, vecIndices3
		vadd.u32 vecIndices0, vecIndices0, vecIndicesInc
		vadd.u32 vecIndices1, vecIndices1, vecIndicesInc
		vadd.u32 vecIndices2, vecIndices2, vecIndicesInc
		vadd.u32 vecIndices3, vecIndices3, vecIndicesInc
		vadd.u32 vecSumB0, vecSumB0, vec0
		vadd.u32 vecSumB1, vecSumB1, vec1
		vadd.u32 vecSumB2, vecSumB2, vec2
		vadd.u32 vecSumB3, vecSumB3, vec3
		vst1.u8 {vec0,vec1}, [sumA256: 128]!
		vst1.u8 {vec2,vec3}, [sumA256: 128]!
		subs i, i, #16
		bne LoopWidth_CompVImageThresholdOtsuSum_32s32s_Asm_NEON32
	EndOf_LoopWidth_CompVImageThresholdOtsuSum_32s32s_Asm_NEON32:

	vadd.u32 vecSumB0, vecSumB0, vecSumB1
	vadd.u32 vecSumB2, vecSumB2, vecSumB3
	vadd.u32 vecSumB0, vecSumB0, vecSumB2
	vadd.u32 q9x, q9x, q9y
	vmov r4, q9x[0]
	vmov r5, q9x[1]
	add r4, r4, r5
	str r4, [sumB1]

	.unreq ptr32uHistogram
	.unreq sumA256
	.unreq sumB1
	.unreq i
	.unreq vecIndicesInc
	.unreq vecIndices0
	.unreq vecIndices1
	.unreq vecIndices2
	.unreq vecIndices3
	.unreq vec0
	.unreq vec1
	.unreq vec2
	.unreq vec3
	.unreq vecSumB0
	.unreq vecSumB1
	.unreq vecSumB2
	.unreq vecSumB3

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__arm__) && !defined(__aarch64__) */

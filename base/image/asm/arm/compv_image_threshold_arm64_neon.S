#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__aarch64__)
.include "compv_common_arm64.S"

.data
 
.text


#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* inPtr,
# arg(1) -> COMPV_ALIGNED(NEON) uint8_t* outPtr,
# arg(2) -> compv_uscalar_t width, 
# arg(3) -> compv_uscalar_t height, 
# arg(4) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride,
# arg(5) -> compv_uscalar_t threshold
COMPV_GAS_FUNCTION_DECLARE CompVImageThresholdGlobal_8u8u_Asm_NEON64
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS
	
	## Declare input arguments ##
	inPtr .req r0 
	outPtr .req r1
	width .req r2
	height .req r3
	stride .req r4
	threshold .req r5

	## Declare local vectors ##
	width1 .req r6
	pad .req r7

	add pad, width, #15
	and pad, pad, #-16
	sub pad, stride, pad

	dup v4.16b, w5 // v4 = vecThreshold

	prfm pldl1keep, [inPtr, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [inPtr, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [inPtr, #(CACHE_LINE_SIZE*2)]
	
	###########################################################
	# for (j = 0; j < height; ++j)
	###########################################################
	LoopHeight_CompVImageThresholdGlobal_8u8u_Asm_NEON64:

		###########################################################
		# for (i = 0; i < width1; i += 64)
		###########################################################
		ands width1, width, #-64
		beq EndOf_LoopWidth64_CompVImageThresholdGlobal_8u8u_Asm_NEON64
		LoopWidth64_CompVImageThresholdGlobal_8u8u_Asm_NEON64:
			prfm pldl1keep, [inPtr, #(CACHE_LINE_SIZE*3)]
			subs width1, width1, #64
			ld1 {v0.16b-v3.16b}, [inPtr], #(4*COMPV_GAS_V_SZ_BYTES)
			cmhi v0.16b, v0.16b, v4.16b
			cmhi v1.16b, v1.16b, v4.16b
			cmhi v2.16b, v2.16b, v4.16b
			cmhi v3.16b, v3.16b, v4.16b
			st1 {v0.16b-v3.16b}, [outPtr], #(4*COMPV_GAS_V_SZ_BYTES)
			bne LoopWidth64_CompVImageThresholdGlobal_8u8u_Asm_NEON64
		EndOf_LoopWidth64_CompVImageThresholdGlobal_8u8u_Asm_NEON64:

		###########################################################
		# for (; i < width; i += 16) 
		###########################################################
		ands width1, width, #63
		beq EndOf_LoopWidth16_CompVImageThresholdGlobal_8u8u_Asm_NEON64
		LoopWidth16_CompVImageThresholdGlobal_8u8u_Asm_NEON64:
			ld1 {v0.16b}, [inPtr], #(1*COMPV_GAS_V_SZ_BYTES)
			cmhi v0.16b, v0.16b, v4.16b
			st1 {v0.16b}, [outPtr], #(1*COMPV_GAS_V_SZ_BYTES)
			subs width1, width1, #16
			bgt LoopWidth16_CompVImageThresholdGlobal_8u8u_Asm_NEON64
		EndOf_LoopWidth16_CompVImageThresholdGlobal_8u8u_Asm_NEON64:
		
		subs height, height, #1
		add inPtr, inPtr, pad
		add outPtr, outPtr, pad
		bne LoopHeight_CompVImageThresholdGlobal_8u8u_Asm_NEON64

	EndOf_LoopHeight_CompVImageThresholdGlobal_8u8u_Asm_NEON64:
	
	.unreq inPtr
	.unreq outPtr
	.unreq width
	.unreq height
	.unreq stride
	.unreq threshold
	.unreq width1
	.unreq pad

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#########################################################################
# arg(1) -> CCOMPV_ALIGNED(NEON) const uint32_t* ptr32uHistogram
# arg(2) -> CCOMPV_ALIGNED(NEON) uint32_t* sumA256
# arg(3) -> Cuint32_t* sumB1
COMPV_GAS_FUNCTION_DECLARE CompVImageThresholdOtsuSum_32s32s_Asm_NEON64
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	ptr32uHistogram .req r0
	sumA256 .req r1
	sumB1 .req r2
	i .req r3
	vecIndicesInc .req v0
	vecIndices0 .req v1
	vecIndices1 .req v2
	vecIndices2 .req v3
	vecIndices3 .req v4
	vec0 .req v5
	vec1 .req v6
	vec2 .req v7
	vec3 .req v8
	vecSumB0 .req v9 // must be q9 (used later)
	vecSumB1 .req v10
	vecSumB2 .req v11
	vecSumB3 .req v12

	mov r4, #16
	dup vecIndicesInc.4s, w4

	mov r4, #4
	dup v14.4s, w4
	mov r5, #0
	mov r6, #1
	mov r7, #2
	mov r8, #3
	ins vecIndices0.4s[0], w5
	ins vecIndices0.4s[1], w6
	ins vecIndices0.4s[2], w7
	ins vecIndices0.4s[3], w8
	add vecIndices1.4s, vecIndices0.4s, v14.4s
	add vecIndices2.4s, vecIndices1.4s, v14.4s
	add vecIndices3.4s, vecIndices2.4s, v14.4s

	eor vecSumB0.16b, vecSumB0.16b, vecSumB0.16b
	eor vecSumB1.16b, vecSumB1.16b, vecSumB1.16b
	eor vecSumB2.16b, vecSumB2.16b, vecSumB2.16b
	eor vecSumB3.16b, vecSumB3.16b, vecSumB3.16b

	#######################################
	# for (size_t i = 0; i < 256; i += 16)
	#######################################
	mov i, #256
	LoopWidth_CompVImageThresholdOtsuSum_32s32s_Asm_NEON64:		
		ld1 {vec0.4s-vec3.4s}, [ptr32uHistogram], #(4*COMPV_GAS_V_SZ_BYTES)
		mul vec0.4s, vec0.4s, vecIndices0.4s
		mul vec1.4s, vec1.4s, vecIndices1.4s
		mul vec2.4s, vec2.4s, vecIndices2.4s
		mul vec3.4s, vec3.4s, vecIndices3.4s
		add vecIndices0.4s, vecIndices0.4s, vecIndicesInc.4s
		add vecIndices1.4s, vecIndices1.4s, vecIndicesInc.4s
		add vecIndices2.4s, vecIndices2.4s, vecIndicesInc.4s
		add vecIndices3.4s, vecIndices3.4s, vecIndicesInc.4s
		add vecSumB0.4s, vecSumB0.4s, vec0.4s
		add vecSumB1.4s, vecSumB1.4s, vec1.4s
		add vecSumB2.4s, vecSumB2.4s, vec2.4s
		add vecSumB3.4s, vecSumB3.4s, vec3.4s
		st1 {vec0.4s-vec3.4s}, [sumA256], #(4*COMPV_GAS_V_SZ_BYTES)
		subs i, i, #16
		bne LoopWidth_CompVImageThresholdOtsuSum_32s32s_Asm_NEON64
	EndOf_LoopWidth_CompVImageThresholdOtsuSum_32s32s_Asm_NEON64:

	add vecSumB0.4s, vecSumB0.4s, vecSumB1.4s
	add vecSumB2.4s, vecSumB2.4s, vecSumB3.4s
	add vecSumB0.4s, vecSumB0.4s, vecSumB2.4s
	addp vecSumB0.4s, vecSumB0.4s, vecSumB0.4s
	
	mov r4w, vecSumB0.s[0]
	mov r5w, vecSumB0.s[1]
	add r4w, r4w, r5w
	str r4w, [sumB1]

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


#endif /* defined(__aarch64__) */

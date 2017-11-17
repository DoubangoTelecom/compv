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


#endif /* defined(__aarch64__) */

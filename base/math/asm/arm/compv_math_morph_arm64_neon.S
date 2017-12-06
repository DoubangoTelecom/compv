#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__aarch64__)
.include "compv_common_arm64.S" //

.data

.text

#########################################################################
# arg(0) -> const compv_uscalar_t* strelInputPtrsPtr
# arg(1) -> const compv_uscalar_t strelInputPtrsCount
# arg(2) -> uint8_t* outPtr
# arg(3) -> const compv_uscalar_t width
# arg(4) -> const compv_uscalar_t height
# arg(5) -> const compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVMathMorphProcessErode_8u_Asm_NEON64
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS
    COMPV_GAS_ALIGN_STACK 16, r9
	COMPV_GAS_MEMALLOC (16*COMPV_GAS_UINT8_SZ_BYTES)

	## Load arguments ##
	strelInputPtrsPtr .req r0
	strelInputPtrsCount .req r1
	outPtr .req r2
	width .req r3
	height .req r4
	stride .req r5

	## Local variables ##
	.equ mem, 0
	i .req r6
	strelInputPtrsPtrv .req r7
	v .req r8
	k .req r9
	uint8 .req r10w
	pad .req r11

	# Compute padding #
	sub pad, stride, width // Not the same as intrin code we''re not reading the orphans as packed #16B but one-by-one

	# Convert strelInputPtrsCount to bytes #
	lsl strelInputPtrsCount, strelInputPtrsCount, #(COMPV_GAS_REG_SHIFT_BYTES)

	###########################################################
	# for (v = 0; v < strelInputPtrsCount; ++v)
	###########################################################
	mov v, #(0*COMPV_GAS_REG_SZ_BYTES)
	LoopCacheLoad_CompVMathMorphProcessOp_8u_Macro_NEON64:
		ldr strelInputPtrsPtrv, [strelInputPtrsPtr, v] // strelInputPtrsPtrv = (strelInputPtrsPtr[v])
		add v, v, #(1*COMPV_GAS_REG_SZ_BYTES)
		prfm pldl1keep, [strelInputPtrsPtrv, #(CACHE_LINE_SIZE*0)]
		prfm pldl1keep, [strelInputPtrsPtrv, #(CACHE_LINE_SIZE*1)]
		prfm pldl1keep, [strelInputPtrsPtrv, #(CACHE_LINE_SIZE*2)]
		prfm pldl1keep, [strelInputPtrsPtrv, #(CACHE_LINE_SIZE*3)]
		cmp v, strelInputPtrsCount
		blt LoopCacheLoad_CompVMathMorphProcessOp_8u_Macro_NEON64
	EndOf_LoopCacheLoad_CompVMathMorphProcessOp_8u_Macro_NEON64:

	########################################
	# for (j = 0, k = 0; j < height; ++j)
	########################################
	mov k, #0
	LoopHeight_CompVMathMorphProcessOp_8u_Macro_NEON64:
		#############################################
		# for (i = 0; i < width64; i += 64, k += 64)
		#############################################
		ands i, width, #-64
		beq EndOf_LoopWidth64_CompVMathMorphProcessOp_8u_Macro_NEON64
		LoopWidth64_CompVMathMorphProcessOp_8u_Macro_NEON64:
			ldr strelInputPtrsPtrv, [strelInputPtrsPtr, #(0*COMPV_GAS_REG_SZ_BYTES)] // strelInputPtrsPtrv = (strelInputPtrsPtr[v=0])
			add strelInputPtrsPtrv, strelInputPtrsPtrv, k//, LSL #(COMPV_YASM_UINT8_SHIFT_BYTES) // strelInputPtrsPtrv += k
			prfm pldl1keep, [strelInputPtrsPtrv, #(CACHE_LINE_SIZE*4)]
			ld1 {v0.16b-v3.16b}, [strelInputPtrsPtrv]
			###########################################################
			# for (v = 1; v < strelInputPtrsCount; ++v)
			###########################################################
			mov v, #(1*COMPV_GAS_REG_SZ_BYTES)
			LoopStrel64_CompVMathMorphProcessOp_8u_Macro_NEON64:
				ldr strelInputPtrsPtrv, [strelInputPtrsPtr, v] // strelInputPtrsPtrv = (strelInputPtrsPtr[v])
				add strelInputPtrsPtrv, strelInputPtrsPtrv, k//, LSL #(COMPV_YASM_UINT8_SHIFT_BYTES) # strelInputPtrsPtrv += k
				prfm pldl1keep, [strelInputPtrsPtrv, #(CACHE_LINE_SIZE*4)]
				add v, v, #(1*COMPV_GAS_REG_SZ_BYTES)
				ld1 {v4.16b-v7.16b}, [strelInputPtrsPtrv]
				#.if \CompVMathMorphProcessOp == CompVMathMorphProcessOpErode
					umin v0.16b, v0.16b, v4.16b
					umin v1.16b, v1.16b, v5.16b
					umin v2.16b, v2.16b, v6.16b
					umin v3.16b, v3.16b, v7.16b
				#.else
				#	umax v0.16b, v0.16b, v4.16b
				#	umax v1.16b, v1.16b, v5.16b
				#	umax v2.16b, v2.16b, v6.16b
				#	umax v3.16b, v3.16b, v7.16b
				#.endif
				cmp v, strelInputPtrsCount
				blt LoopStrel64_CompVMathMorphProcessOp_8u_Macro_NEON64
			EndOf_LoopStrel64_CompVMathMorphProcessOp_8u_Macro_NEON64:
			
			subs i, i, #64
			add k, k, #64//, LSL #(COMPV_YASM_UINT8_SHIFT_BYTES)
			st1 {v0.16b-v3.16b}, [outPtr], #(4*COMPV_GAS_V_SZ_BYTES)	
			bne LoopWidth64_CompVMathMorphProcessOp_8u_Macro_NEON64
		EndOf_LoopWidth64_CompVMathMorphProcessOp_8u_Macro_NEON64:

		ands i, width, #63
		beq EndOf_LoopWidth16_CompVMathMorphProcessOp_8u_Macro_NEON64

		###########################################
		# for (# i < width; i += 16, k += 16)
		###########################################
		LoopWidth16_CompVMathMorphProcessOp_8u_Macro_NEON64:
			ldr strelInputPtrsPtrv, [strelInputPtrsPtr, #(0*COMPV_GAS_REG_SZ_BYTES)] // strelInputPtrsPtrv = (strelInputPtrsPtr[v=0])
			add strelInputPtrsPtrv, strelInputPtrsPtrv, k//, LSL #(COMPV_YASM_UINT8_SHIFT_BYTES) # strelInputPtrsPtrv += k
			prfm pldl1keep, [strelInputPtrsPtrv, #(CACHE_LINE_SIZE*4)]
			ld1 {v0.16b}, [strelInputPtrsPtrv]
			###########################################################
			# for (v = 1; v < strelInputPtrsCount; ++v)
			###########################################################
			mov v, #(1*COMPV_GAS_REG_SZ_BYTES)
			LoopStrel16_CompVMathMorphProcessOp_8u_Macro_NEON64:
				ldr strelInputPtrsPtrv, [strelInputPtrsPtr, v] // strelInputPtrsPtrv = (strelInputPtrsPtr[v])
				add strelInputPtrsPtrv, strelInputPtrsPtrv, k//, LSL #(COMPV_YASM_UINT8_SHIFT_BYTES) # strelInputPtrsPtrv += k
				prfm pldl1keep, [strelInputPtrsPtrv, #(CACHE_LINE_SIZE*4)]
				add v, v, #(1*COMPV_GAS_REG_SZ_BYTES)
				ld1 {v4.16b}, [strelInputPtrsPtrv]
				cmp v, strelInputPtrsCount
				#.if \CompVMathMorphProcessOp == CompVMathMorphProcessOpErode
					umin v0.16b, v0.16b, v4.16b
				#.else
				#	umax v0.16b, v0.16b, v4.16b
				#.endif
				blt LoopStrel16_CompVMathMorphProcessOp_8u_Macro_NEON64
			EndOf_LoopStrel16_CompVMathMorphProcessOp_8u_Macro_NEON64:
			
			subs i, i, #16
			bmi MoreThanWidth16_CompVMathMorphProcessOp_8u_Macro_NEON64
				
			## if (i < width16) ##
			LessThanWidth16_CompVMathMorphProcessOp_8u_Macro_NEON64:
				st1 {v0.16b}, [outPtr], #(1*COMPV_GAS_V_SZ_BYTES)
				add k, k, #(16 << COMPV_GAS_UINT8_SHIFT_BYTES)
				b EndOf_MoreThanWidth16_CompVMathMorphProcessOp_8u_Macro_NEON64
			EndOf_LessThanWidth16_CompVMathMorphProcessOp_8u_Macro_NEON64:

			## if (i >= width16) ##
			MoreThanWidth16_CompVMathMorphProcessOp_8u_Macro_NEON64:
				add v, sp, #(mem) // v now contains mem address
				st1 {v0.16b}, [v]
				## for (v = 0; i < width; ++i, ++v) ##
				add i, i, #16 // was negative and now contains '(width - (width & -16))'
				add k, k, i//, LSL #(COMPV_GAS_UINT8_SHIFT_BYTES)
				LoopMoreThanWidth16_CompVMathMorphProcessOp_8u_Macro_NEON64:
					ldrb uint8, [v], #(1*COMPV_GAS_UINT8_SZ_BYTES)
					subs i, i, #1
					strb uint8, [outPtr], #(1*COMPV_GAS_UINT8_SZ_BYTES)
					bne LoopMoreThanWidth16_CompVMathMorphProcessOp_8u_Macro_NEON64
				EndOf_LoopMoreThanWidth16_CompVMathMorphProcessOp_8u_Macro_NEON64:
				b EndOf_LoopWidth16_CompVMathMorphProcessOp_8u_Macro_NEON64
			EndOf_MoreThanWidth16_CompVMathMorphProcessOp_8u_Macro_NEON64:

			bgt LoopWidth16_CompVMathMorphProcessOp_8u_Macro_NEON64 // branch for far above 'subs i, i, #16' instruction
		EndOf_LoopWidth16_CompVMathMorphProcessOp_8u_Macro_NEON64:
		
		subs height, height, #1
		add k, k, pad//, LSL #(COMPV_GAS_UINT8_SHIFT_BYTES)
		add outPtr, outPtr, pad//, LSL #(COMPV_GAS_UINT8_SHIFT_BYTES)
		bne LoopHeight_CompVMathMorphProcessOp_8u_Macro_NEON64
	EndOf_LoopHeight_CompVMathMorphProcessOp_8u_Macro_NEON64:

	.unreq strelInputPtrsPtr
	.unreq strelInputPtrsCount
	.unreq outPtr
	.unreq width
	.unreq height
	.unreq stride

	.unreq i
	.unreq strelInputPtrsPtrv
	.unreq v
	.unreq k
	.unreq uint8
	.unreq pad

	COMPV_GAS_MEMFREE (16*COMPV_GAS_UINT8_SZ_BYTES)
	COMPV_GAS_UNALIGN_STACK r9, r10 // *must not* use the same register
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__aarch64__) */
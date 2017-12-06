#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__arm__) && !defined(__aarch64__)
.include "compv_common_arm32.S"

#if defined(__APPLE__)
#   define sym(funcname) _##funcname
#else
#   define sym(funcname) funcname
#endif

.equ CompVMathMorphProcessOpErode, 0
.equ CompVMathMorphProcessOpDilate, 1

.data

.extern
 
.text

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> const compv_uscalar_t* strelInputPtrsPtr
@ arg(1) -> const compv_uscalar_t strelInputPtrsCount
@ arg(2) -> uint8_t* outPtr
@ arg(3) -> const compv_uscalar_t width
@ arg(4) -> const compv_uscalar_t height
@ arg(5) -> const compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVMathMorphProcessErode_8u_Asm_NEON32
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 6
	COMPV_GAS_SAVE_NEON_REGS
    COMPV_GAS_ALIGN_STACK 16, r11
	COMPV_GAS_MEMALLOC (16*COMPV_GAS_UINT8_SZ_BYTES)

	@@ Load arguments @@
	ldm_args r0-r5
	strelInputPtrsPtr .req r0
	strelInputPtrsCount .req r1
	outPtr .req r2
	width .req r3
	height .req r4
	stride .req r5

	@@ Local variables @@
	.equ mem, 0
	i .req r6
	strelInputPtrsPtrv .req r7
	v .req r8
	k .req r9
	uint8 .req r10
	pad .req r11

	@ Compute padding @
	sub pad, stride, width @ Not the same as intrin code we''re not reading the orphans as packed #16B but one-by-one

	@ Convert strelInputPtrsCount to bytes @
	lsl strelInputPtrsCount, strelInputPtrsCount, #(COMPV_GAS_REG_SHIFT_BYTES)

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (v = 0; v < strelInputPtrsCount; ++v)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	mov v, #(0*COMPV_GAS_REG_SZ_BYTES)
	LoopCacheLoad_CompVMathMorphProcessOp_8u_Macro_NEON32:
		ldr strelInputPtrsPtrv, [strelInputPtrsPtr, v] @ strelInputPtrsPtrv = (strelInputPtrsPtr[v])
		add v, v, #(1*COMPV_GAS_REG_SZ_BYTES)
		pld [strelInputPtrsPtrv, #(CACHE_LINE_SIZE*0)]
		pld [strelInputPtrsPtrv, #(CACHE_LINE_SIZE*1)]
		pld [strelInputPtrsPtrv, #(CACHE_LINE_SIZE*2)]
		pld [strelInputPtrsPtrv, #(CACHE_LINE_SIZE*3)]
		cmp v, strelInputPtrsCount
		blt LoopCacheLoad_CompVMathMorphProcessOp_8u_Macro_NEON32
	EndOf_LoopCacheLoad_CompVMathMorphProcessOp_8u_Macro_NEON32:

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (j = 0, k = 0; j < height; ++j)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	mov k, #0
	LoopHeight_CompVMathMorphProcessOp_8u_Macro_NEON32:
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (i = 0; i < width64; i += 64, k += 64)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		ands i, width, #-64
		beq EndOf_LoopWidth64_CompVMathMorphProcessOp_8u_Macro_NEON32
		LoopWidth64_CompVMathMorphProcessOp_8u_Macro_NEON32:
			ldr strelInputPtrsPtrv, [strelInputPtrsPtr, #(0*COMPV_GAS_REG_SZ_BYTES)] @ strelInputPtrsPtrv = (strelInputPtrsPtr[v=0])
			add strelInputPtrsPtrv, strelInputPtrsPtrv, k@, LSL #(COMPV_YASM_UINT8_SHIFT_BYTES) @ strelInputPtrsPtrv += k
			pld [strelInputPtrsPtrv, #(CACHE_LINE_SIZE*4)]
			vld1.u8 {q0,q1}, [strelInputPtrsPtrv]!
			vld1.u8 {q2,q3}, [strelInputPtrsPtrv]
			@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			@ for (v = 1; v < strelInputPtrsCount; ++v)
			@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			mov v, #(1*COMPV_GAS_REG_SZ_BYTES)
			LoopStrel64_CompVMathMorphProcessOp_8u_Macro_NEON32:
				ldr strelInputPtrsPtrv, [strelInputPtrsPtr, v] @ strelInputPtrsPtrv = (strelInputPtrsPtr[v])
				add strelInputPtrsPtrv, strelInputPtrsPtrv, k@, LSL #(COMPV_YASM_UINT8_SHIFT_BYTES) @ strelInputPtrsPtrv += k
				pld [strelInputPtrsPtrv, #(CACHE_LINE_SIZE*4)]
				add v, v, #(1*COMPV_GAS_REG_SZ_BYTES)
				vld1.u8 {q4,q5}, [strelInputPtrsPtrv]!
				vld1.u8 {q6,q7}, [strelInputPtrsPtrv]
				@.if \CompVMathMorphProcessOp == CompVMathMorphProcessOpErode
					vmin.u8 q0, q0, q4
					vmin.u8 q1, q1, q5
					vmin.u8 q2, q2, q6
					vmin.u8 q3, q3, q7
				@.else
				@	vmax.u8 q0, q0, q4
				@	vmax.u8 q1, q1, q5
				@	vmax.u8 q2, q2, q6
				@	vmax.u8 q3, q3, q7
				@.endif
				cmp v, strelInputPtrsCount
				blt LoopStrel64_CompVMathMorphProcessOp_8u_Macro_NEON32
			EndOf_LoopStrel64_CompVMathMorphProcessOp_8u_Macro_NEON32:
			
			subs i, i, #64
			add k, k, #64@, LSL #(COMPV_YASM_UINT8_SHIFT_BYTES)
			vst1.u8 {q0,q1}, [outPtr]!
			vst1.u8 {q2,q3}, [outPtr]!		
			bne LoopWidth64_CompVMathMorphProcessOp_8u_Macro_NEON32
		EndOf_LoopWidth64_CompVMathMorphProcessOp_8u_Macro_NEON32:

		ands i, width, #63
		beq EndOf_LoopWidth16_CompVMathMorphProcessOp_8u_Macro_NEON32

		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (@ i < width; i += 16, k += 16)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		LoopWidth16_CompVMathMorphProcessOp_8u_Macro_NEON32:
			ldr strelInputPtrsPtrv, [strelInputPtrsPtr, #(0*COMPV_GAS_REG_SZ_BYTES)] @ strelInputPtrsPtrv = (strelInputPtrsPtr[v=0])
			add strelInputPtrsPtrv, strelInputPtrsPtrv, k@, LSL #(COMPV_YASM_UINT8_SHIFT_BYTES) @ strelInputPtrsPtrv += k
			pld [strelInputPtrsPtrv, #(CACHE_LINE_SIZE*4)]
			vld1.u8 {q0}, [strelInputPtrsPtrv]
			@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			@ for (v = 1; v < strelInputPtrsCount; ++v)
			@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			mov v, #(1*COMPV_GAS_REG_SZ_BYTES)
			LoopStrel16_CompVMathMorphProcessOp_8u_Macro_NEON32:
				ldr strelInputPtrsPtrv, [strelInputPtrsPtr, v] @ strelInputPtrsPtrv = (strelInputPtrsPtr[v])
				add strelInputPtrsPtrv, strelInputPtrsPtrv, k@, LSL #(COMPV_YASM_UINT8_SHIFT_BYTES) @ strelInputPtrsPtrv += k
				pld [strelInputPtrsPtrv, #(CACHE_LINE_SIZE*4)]
				add v, v, #(1*COMPV_GAS_REG_SZ_BYTES)
				vld1.u8 {q4}, [strelInputPtrsPtrv]
				cmp v, strelInputPtrsCount
				@.if \CompVMathMorphProcessOp == CompVMathMorphProcessOpErode
					vmin.u8 q0, q0, q4
				@.else
				@	vmax.u8 q0, q0, q4
				@.endif
				blt LoopStrel16_CompVMathMorphProcessOp_8u_Macro_NEON32
			EndOf_LoopStrel16_CompVMathMorphProcessOp_8u_Macro_NEON32:
			
			subs i, i, #16
			bmi MoreThanWidth16_CompVMathMorphProcessOp_8u_Macro_NEON32
				
			@@ if (i < width16) @@
			LessThanWidth16_CompVMathMorphProcessOp_8u_Macro_NEON32:
				vst1.u8 {q0}, [outPtr]!
				add k, k, #(16 << COMPV_GAS_UINT8_SHIFT_BYTES)
				b EndOf_MoreThanWidth16_CompVMathMorphProcessOp_8u_Macro_NEON32
			EndOf_LessThanWidth16_CompVMathMorphProcessOp_8u_Macro_NEON32:

			@@ if (i >= width16) @@
			MoreThanWidth16_CompVMathMorphProcessOp_8u_Macro_NEON32:
				add v, sp, #(mem) @ v now contains mem address
				vst1.u8 {q0}, [v :128]
				@@ for (v = 0; i < width; ++i, ++v) @@
				add i, i, #16 @ was negative and now contains '(width - (width & -16))'
				add k, k, i@, LSL #(COMPV_GAS_UINT8_SHIFT_BYTES)
				LoopMoreThanWidth16_CompVMathMorphProcessOp_8u_Macro_NEON32:
					ldrb uint8, [v], #(1*COMPV_GAS_UINT8_SZ_BYTES)
					subs i, i, #1
					strb uint8, [outPtr], #(1*COMPV_GAS_UINT8_SZ_BYTES)
					bne LoopMoreThanWidth16_CompVMathMorphProcessOp_8u_Macro_NEON32
				EndOf_LoopMoreThanWidth16_CompVMathMorphProcessOp_8u_Macro_NEON32:
				b EndOf_LoopWidth16_CompVMathMorphProcessOp_8u_Macro_NEON32
			EndOf_MoreThanWidth16_CompVMathMorphProcessOp_8u_Macro_NEON32:

			bgt LoopWidth16_CompVMathMorphProcessOp_8u_Macro_NEON32 @ branch for far above 'subs i, i, #16' instruction
		EndOf_LoopWidth16_CompVMathMorphProcessOp_8u_Macro_NEON32:
		
		subs height, height, #1
		add k, k, pad@, LSL #(COMPV_GAS_UINT8_SHIFT_BYTES)
		add outPtr, outPtr, pad@, LSL #(COMPV_GAS_UINT8_SHIFT_BYTES)
		bne LoopHeight_CompVMathMorphProcessOp_8u_Macro_NEON32
	EndOf_LoopHeight_CompVMathMorphProcessOp_8u_Macro_NEON32:

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
	COMPV_GAS_UNALIGN_STACK r11
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 6
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#endif /* defined(__arm__) && !defined(__aarch64__) */

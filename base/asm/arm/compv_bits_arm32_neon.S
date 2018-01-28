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
@ arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* Aptr
@ arg(1) -> COMPV_ALIGNED(SSE) const uint8_t* Bptr
@ arg(2) -> uint8_t* Rptr
@ arg(3) -> compv_uscalar_t width
@ arg(4) -> COMPV_ALIGNED(SSE) compv_uscalar_t height
@ arg(5) -> COMPV_ALIGNED(SSE) compv_uscalar_t Astride
@ arg(6) -> COMPV_ALIGNED(SSE) compv_uscalar_t Bstride
@ arg(7) -> COMPV_ALIGNED(SSE) compv_uscalar_t Rstride
COMPV_GAS_FUNCTION_DECLARE CompVBitsLogicalAnd_8u_Asm_NEON32
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 8
	COMPV_GAS_SAVE_NEON_REGS

	@@ Parameters @@
	ldm_args r0-r7
	Aptr .req r0
	Bptr .req r1
	Rptr .req r2
	width .req r3
	height .req r4
	Astride .req r5
	Bstride .req r6
	Rstride .req r7

	@ Transform strides to padding @
	add r11, width, #15
	and r11, r11, #-16
	sub Astride, Astride, r11
	sub Bstride, Bstride, r11
	sub Rstride, Rstride, r11

	@@ Local vars @@
	i .req r8
	width64 .req r9
	width16 .req r10
	and width64, width, #-64
	and width16, width, #63
	add width16, width16, #15
	and width16, width16, #-16

	pld [Aptr, #(CACHE_LINE_SIZE*0)]
	pld [Aptr, #(CACHE_LINE_SIZE*1)]
	pld [Aptr, #(CACHE_LINE_SIZE*2)]
	pld [Bptr, #(CACHE_LINE_SIZE*0)]
	pld [Bptr, #(CACHE_LINE_SIZE*1)]
	pld [Bptr, #(CACHE_LINE_SIZE*2)]

    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (compv_uscalar_t j = 0; j < height; ++j)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopHeight_CompVBitsLogicalAnd_8u_Asm_NEON32:
		movs i, width64
		beq EndOf_LoopWidth64_CompVBitsLogicalAnd_8u_Asm_NEON32
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (i = 0; i < width64; i += 64)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		LoopWidth64_CompVBitsLogicalAnd_8u_Asm_NEON32:
			pld [Aptr, #(CACHE_LINE_SIZE*3)]
			pld [Bptr, #(CACHE_LINE_SIZE*3)]
			vldm Aptr!, { q0-q3 }
			vldm Bptr!, { q4-q7 }
			vand q0, q0, q4
			vand q1, q1, q5
			vand q2, q2, q6
			vand q3, q3, q7
			vstm Rptr!, { q0-q3 }
			subs i, i, #64
			bne LoopWidth64_CompVBitsLogicalAnd_8u_Asm_NEON32
		EndOf_LoopWidth64_CompVBitsLogicalAnd_8u_Asm_NEON32:

		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (; i < width; i += 16)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		movs i, width16
		beq EndOf_LoopWidth16_CompVBitsLogicalAnd_8u_Asm_NEON32
		LoopWidth16_CompVBitsLogicalAnd_8u_Asm_NEON32:
			vld1.u8 { q0 }, [Aptr :128]!
			vld1.u8 { q4 }, [Bptr :128]!
			vand q0, q0, q4
			vst1.u8 { q0 }, [Rptr :128]!
			subs i, i, #16
			bne LoopWidth16_CompVBitsLogicalAnd_8u_Asm_NEON32
		EndOf_LoopWidth16_CompVBitsLogicalAnd_8u_Asm_NEON32:

		add Rptr, Rptr, Rstride
		add Aptr, Aptr, Astride
		add Bptr, Bptr, Bstride
		subs height, height, #1
		bne LoopHeight_CompVBitsLogicalAnd_8u_Asm_NEON32	
	EndOf_LoopHeight_CompVBitsLogicalAnd_8u_Asm_NEON32:

	@@ UnDefines @@
	.unreq Aptr
	.unreq Bptr
	.unreq Rptr
	.unreq width
	.unreq height
	.unreq Astride
	.unreq Bstride
	.unreq Rstride
	.unreq i
	.unreq width64
	.unreq width16

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 8
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__arm__) && !defined(__aarch64__) */

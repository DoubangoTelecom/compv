#########################################################################
# Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   #
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
# arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* Aptr
# arg(1) -> COMPV_ALIGNED(SSE) const uint8_t* Bptr
# arg(2) -> uint8_t* Rptr
# arg(3) -> compv_uscalar_t width
# arg(4) -> COMPV_ALIGNED(SSE) compv_uscalar_t height
# arg(5) -> COMPV_ALIGNED(SSE) compv_uscalar_t Astride
# arg(6) -> COMPV_ALIGNED(SSE) compv_uscalar_t Bstride
# arg(7) -> COMPV_ALIGNED(SSE) compv_uscalar_t Rstride
COMPV_GAS_FUNCTION_DECLARE CompVBitsLogicalAnd_8u_Asm_NEON64
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Parameters ##
	Aptr .req r0
	Bptr .req r1
	Rptr .req r2
	width .req r3
	height .req r4
	Astride .req r5
	Bstride .req r6
	Rstride .req r7

	# Transform strides to padding #
	add r11, width, #15
	and r11, r11, #-16
	sub Astride, Astride, r11
	sub Bstride, Bstride, r11
	sub Rstride, Rstride, r11

	## Local vars ##
	i .req r8
	width64 .req r9
	width16 .req r10
	and width64, width, #-64
	and width16, width, #63
	add width16, width16, #15
	and width16, width16, #-16

	prfm pldl1keep, [Aptr, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [Aptr, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [Aptr, #(CACHE_LINE_SIZE*2)]
	prfm pldl1keep, [Bptr, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [Bptr, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [Bptr, #(CACHE_LINE_SIZE*2)]
	//prfm pstl1keep, [Rptr, #(CACHE_LINE_SIZE*0)]
	//prfm pstl1keep, [Rptr, #(CACHE_LINE_SIZE*1)]
	//prfm pstl1keep, [Rptr, #(CACHE_LINE_SIZE*2)]

    ###########################################################
    # for (compv_uscalar_t j = 0; j < height; ++j)
    ###########################################################
	LoopHeight_CompVBitsLogicalAnd_8u_Asm_NEON64:
		cbz width64, EndOf_LoopWidth64_CompVBitsLogicalAnd_8u_Asm_NEON64
		mov i, width64
		###########################################################
		# for (i = 0; i < width64; i += 64)
		###########################################################
		LoopWidth64_CompVBitsLogicalAnd_8u_Asm_NEON64:
			prfm pldl1keep, [Aptr, #(CACHE_LINE_SIZE*3)]
			prfm pldl1keep, [Bptr, #(CACHE_LINE_SIZE*3)]
			//prfm pstl1keep, [Rptr, #(CACHE_LINE_SIZE*3)]
			ldp q0, q1, [Aptr], #(2*COMPV_GAS_Q_SZ_BYTES)
			ldp q2, q3, [Aptr], #(2*COMPV_GAS_Q_SZ_BYTES)
			ldp q4, q5, [Bptr], #(2*COMPV_GAS_Q_SZ_BYTES)
			ldp q6, q7, [Bptr], #(2*COMPV_GAS_Q_SZ_BYTES)
			and v0.16b, v0.16b, v4.16b
			and v1.16b, v1.16b, v5.16b
			and v2.16b, v2.16b, v6.16b
			and v3.16b, v3.16b, v7.16b
			stp q0, q1, [Rptr], #(2*COMPV_GAS_Q_SZ_BYTES)
			stp q2, q3, [Rptr], #(2*COMPV_GAS_Q_SZ_BYTES)
			subs i, i, #64
			bne LoopWidth64_CompVBitsLogicalAnd_8u_Asm_NEON64
		EndOf_LoopWidth64_CompVBitsLogicalAnd_8u_Asm_NEON64:

		###########################################################
		# for (; i < width; i += 16)
		###########################################################
		cbz width16, EndOf_LoopWidth16_CompVBitsLogicalAnd_8u_Asm_NEON64
		mov i, width16
		LoopWidth16_CompVBitsLogicalAnd_8u_Asm_NEON64:
			ldr q0, [Aptr], #(1*COMPV_GAS_Q_SZ_BYTES)
			ldr q4, [Bptr], #(1*COMPV_GAS_Q_SZ_BYTES)
			and v0.16b, v0.16b, v4.16b
			str q0, [Rptr], #(1*COMPV_GAS_Q_SZ_BYTES)
			subs i, i, #16
			bne LoopWidth16_CompVBitsLogicalAnd_8u_Asm_NEON64
		EndOf_LoopWidth16_CompVBitsLogicalAnd_8u_Asm_NEON64:

		add Rptr, Rptr, Rstride
		add Aptr, Aptr, Astride
		add Bptr, Bptr, Bstride
		subs height, height, #1
		bne LoopHeight_CompVBitsLogicalAnd_8u_Asm_NEON64	
	EndOf_LoopHeight_CompVBitsLogicalAnd_8u_Asm_NEON64:

	## UnDefines ##
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
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#########################################################################
# arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* Aptr
# arg(1) -> COMPV_ALIGNED(SSE) const uint8_t* Bptr
# arg(2) -> uint8_t* Rptr
# arg(3) -> compv_uscalar_t width
# arg(4) -> COMPV_ALIGNED(SSE) compv_uscalar_t height
# arg(5) -> COMPV_ALIGNED(SSE) compv_uscalar_t Astride
# arg(6) -> COMPV_ALIGNED(SSE) compv_uscalar_t Bstride
# arg(7) -> COMPV_ALIGNED(SSE) compv_uscalar_t Rstride
COMPV_GAS_FUNCTION_DECLARE CompVBitsLogicalNotAnd_8u_Asm_NEON64
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Parameters ##
	Aptr .req r0
	Bptr .req r1
	Rptr .req r2
	width .req r3
	height .req r4
	Astride .req r5
	Bstride .req r6
	Rstride .req r7

	# Transform strides to padding #
	add r11, width, #15
	and r11, r11, #-16
	sub Astride, Astride, r11
	sub Bstride, Bstride, r11
	sub Rstride, Rstride, r11

	## Local vars ##
	i .req r8
	width64 .req r9
	width16 .req r10
	and width64, width, #-64
	and width16, width, #63
	add width16, width16, #15
	and width16, width16, #-16

	prfm pldl1keep, [Aptr, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [Aptr, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [Aptr, #(CACHE_LINE_SIZE*2)]
	prfm pldl1keep, [Bptr, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [Bptr, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [Bptr, #(CACHE_LINE_SIZE*2)]
	//prfm pstl1keep, [Rptr, #(CACHE_LINE_SIZE*0)]
	//prfm pstl1keep, [Rptr, #(CACHE_LINE_SIZE*1)]
	//prfm pstl1keep, [Rptr, #(CACHE_LINE_SIZE*2)]

    ###########################################################
    # for (compv_uscalar_t j = 0; j < height; ++j)
    ###########################################################
	LoopHeight_CompVBitsLogicalNotAnd_8u_Asm_NEON64:
		cbz width64, EndOf_LoopWidth64_CompVBitsLogicalNotAnd_8u_Asm_NEON64
		mov i, width64
		###########################################################
		# for (i = 0; i < width64; i += 64)
		###########################################################
		LoopWidth64_CompVBitsLogicalNotAnd_8u_Asm_NEON64:
			prfm pldl1keep, [Aptr, #(CACHE_LINE_SIZE*3)]
			prfm pldl1keep, [Bptr, #(CACHE_LINE_SIZE*3)]
			//prfm pstl1keep, [Rptr, #(CACHE_LINE_SIZE*3)]
			ldp q0, q1, [Aptr], #(2*COMPV_GAS_Q_SZ_BYTES)
			ldp q2, q3, [Aptr], #(2*COMPV_GAS_Q_SZ_BYTES)
			ldp q4, q5, [Bptr], #(2*COMPV_GAS_Q_SZ_BYTES)
			ldp q6, q7, [Bptr], #(2*COMPV_GAS_Q_SZ_BYTES)
			bic v0.16b, v4.16b, v0.16b
			bic v1.16b, v5.16b, v1.16b
			bic v2.16b, v6.16b, v2.16b
			bic v3.16b, v7.16b, v3.16b
			stp q0, q1, [Rptr], #(2*COMPV_GAS_Q_SZ_BYTES)
			stp q2, q3, [Rptr], #(2*COMPV_GAS_Q_SZ_BYTES)
			subs i, i, #64
			bne LoopWidth64_CompVBitsLogicalNotAnd_8u_Asm_NEON64
		EndOf_LoopWidth64_CompVBitsLogicalNotAnd_8u_Asm_NEON64:

		###########################################################
		# for (; i < width; i += 16)
		###########################################################
		cbz width16, EndOf_LoopWidth16_CompVBitsLogicalNotAnd_8u_Asm_NEON64
		mov i, width16
		LoopWidth16_CompVBitsLogicalNotAnd_8u_Asm_NEON64:
			ldr q0, [Aptr], #(1*COMPV_GAS_Q_SZ_BYTES)
			ldr q4, [Bptr], #(1*COMPV_GAS_Q_SZ_BYTES)
			bic v0.16b, v4.16b, v0.16b
			str q0, [Rptr], #(1*COMPV_GAS_Q_SZ_BYTES)
			subs i, i, #16
			bne LoopWidth16_CompVBitsLogicalNotAnd_8u_Asm_NEON64
		EndOf_LoopWidth16_CompVBitsLogicalNotAnd_8u_Asm_NEON64:

		add Rptr, Rptr, Rstride
		add Aptr, Aptr, Astride
		add Bptr, Bptr, Bstride
		subs height, height, #1
		bne LoopHeight_CompVBitsLogicalNotAnd_8u_Asm_NEON64	
	EndOf_LoopHeight_CompVBitsLogicalNotAnd_8u_Asm_NEON64:

	## UnDefines ##
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
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#########################################################################
# arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* Aptr
# arg(1) -> uint8_t* Rptr
# arg(2) -> compv_uscalar_t width
# arg(3) -> COMPV_ALIGNED(SSE) compv_uscalar_t height
# arg(4) -> COMPV_ALIGNED(SSE) compv_uscalar_t Astride
# arg(5) -> COMPV_ALIGNED(SSE) compv_uscalar_t Rstride
COMPV_GAS_FUNCTION_DECLARE CompVBitsLogicalNot_8u_Asm_NEON64
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Parameters ##
	Aptr .req r0
	Rptr .req r1
	width .req r2
	height .req r3
	Astride .req r4
	Rstride .req r5

	# Transform strides to padding #
	add r11, width, #15
	and r11, r11, #-16
	sub Astride, Astride, r11
	sub Rstride, Rstride, r11

	## Local vars ##
	i .req r8
	width64 .req r9
	width16 .req r10
	and width64, width, #-64
	and width16, width, #63
	add width16, width16, #15
	and width16, width16, #-16

	prfm pldl1keep, [Aptr, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [Aptr, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [Aptr, #(CACHE_LINE_SIZE*2)]
	//prfm pstl1keep, [Rptr, #(CACHE_LINE_SIZE*0)]
	//prfm pstl1keep, [Rptr, #(CACHE_LINE_SIZE*1)]
	//prfm pstl1keep, [Rptr, #(CACHE_LINE_SIZE*2)]

    ###########################################################
    # for (compv_uscalar_t j = 0; j < height; ++j)
    ###########################################################
	LoopHeight_CompVBitsLogicalNot_8u_Asm_NEON64:
		cbz width64, EndOf_LoopWidth64_CompVBitsLogicalNot_8u_Asm_NEON64
		mov i, width64
		###########################################################
		# for (i = 0; i < width64; i += 64)
		###########################################################
		LoopWidth64_CompVBitsLogicalNot_8u_Asm_NEON64:
			prfm pldl1keep, [Aptr, #(CACHE_LINE_SIZE*3)]
			//prfm pstl1keep, [Rptr, #(CACHE_LINE_SIZE*3)]
			ldp q0, q1, [Aptr], #(2*COMPV_GAS_Q_SZ_BYTES)
			ldp q2, q3, [Aptr], #(2*COMPV_GAS_Q_SZ_BYTES)
			mvn v0.16b, v0.16b
			mvn v1.16b, v1.16b
			mvn v2.16b, v2.16b
			mvn v3.16b, v3.16b
			stp q0, q1, [Rptr], #(2*COMPV_GAS_Q_SZ_BYTES)
			stp q2, q3, [Rptr], #(2*COMPV_GAS_Q_SZ_BYTES)
			subs i, i, #64
			bne LoopWidth64_CompVBitsLogicalNot_8u_Asm_NEON64
		EndOf_LoopWidth64_CompVBitsLogicalNot_8u_Asm_NEON64:

		###########################################################
		# for (; i < width; i += 16)
		###########################################################
		cbz width16, EndOf_LoopWidth16_CompVBitsLogicalNot_8u_Asm_NEON64
		mov i, width16
		LoopWidth16_CompVBitsLogicalNot_8u_Asm_NEON64:
			ldr q0, [Aptr], #(1*COMPV_GAS_Q_SZ_BYTES)
			mvn v0.16b, v0.16b
			str q0, [Rptr], #(1*COMPV_GAS_Q_SZ_BYTES)
			subs i, i, #16
			bne LoopWidth16_CompVBitsLogicalNot_8u_Asm_NEON64
		EndOf_LoopWidth16_CompVBitsLogicalNot_8u_Asm_NEON64:

		add Rptr, Rptr, Rstride
		add Aptr, Aptr, Astride
		subs height, height, #1
		bne LoopHeight_CompVBitsLogicalNot_8u_Asm_NEON64	
	EndOf_LoopHeight_CompVBitsLogicalNot_8u_Asm_NEON64:

	## UnDefines ##
	.unreq Aptr
	.unreq Rptr
	.unreq width
	.unreq height
	.unreq Astride
	.unreq Rstride
	.unreq i
	.unreq width64
	.unreq width16

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#########################################################################
# arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* Aptr
# arg(1) -> COMPV_ALIGNED(SSE) const uint8_t* A_Minus1_ptr
# arg(2) -> uint8_t* Rptr
# arg(3) -> compv_uscalar_t width
# arg(4) -> COMPV_ALIGNED(SSE) compv_uscalar_t height
# arg(5) -> COMPV_ALIGNED(SSE) compv_uscalar_t Astride
# arg(6) -> COMPV_ALIGNED(SSE) compv_uscalar_t Rstride
COMPV_GAS_FUNCTION_DECLARE CompVBitsLogicalXorVt_8u_Asm_NEON64
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Parameters ##
	Aptr .req r0
	A_Minus1_ptr .req r1
	Rptr .req r2
	width .req r3
	height .req r4
	Astride .req r5
	Rstride .req r6

	# Transform strides to padding #
	add r11, width, #15
	and r11, r11, #-16
	sub Astride, Astride, r11
	sub Rstride, Rstride, r11

	## Local vars ##
	i .req r8
	width64 .req r9
	width16 .req r10
	and width64, width, #-64
	and width16, width, #63
	add width16, width16, #15
	and width16, width16, #-16

	prfm pldl1keep, [Aptr, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [Aptr, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [Aptr, #(CACHE_LINE_SIZE*2)]
	prfm pldl1keep, [A_Minus1_ptr, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [A_Minus1_ptr, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [A_Minus1_ptr, #(CACHE_LINE_SIZE*2)]
	//prfm pstl1keep, [Rptr, #(CACHE_LINE_SIZE*0)]
	//prfm pstl1keep, [Rptr, #(CACHE_LINE_SIZE*1)]
	//prfm pstl1keep, [Rptr, #(CACHE_LINE_SIZE*2)]

    ###########################################################
    # for (compv_uscalar_t j = 0; j < height; ++j)
    ###########################################################
	LoopHeight_CompVBitsLogicalXorVt_8u_Asm_NEON64:
		cbz width64, EndOf_LoopWidth64_CompVBitsLogicalXorVt_8u_Asm_NEON64
		mov i, width64
		###########################################################
		# for (i = 0; i < width64; i += 64)
		###########################################################
		LoopWidth64_CompVBitsLogicalXorVt_8u_Asm_NEON64:
			prfm pldl1keep, [Aptr, #(CACHE_LINE_SIZE*3)]
			prfm pldl1keep, [A_Minus1_ptr, #(CACHE_LINE_SIZE*3)]
			//prfm pstl1keep, [Rptr, #(CACHE_LINE_SIZE*3)]
			ldp q0, q1, [Aptr], #(2*COMPV_GAS_Q_SZ_BYTES)
			ldp q2, q3, [Aptr], #(2*COMPV_GAS_Q_SZ_BYTES)
			ldp q4, q5, [A_Minus1_ptr], #(2*COMPV_GAS_Q_SZ_BYTES)
			ldp q6, q7, [A_Minus1_ptr], #(2*COMPV_GAS_Q_SZ_BYTES)
			eor v0.16b, v0.16b, v4.16b
			eor v1.16b, v1.16b, v5.16b
			eor v2.16b, v2.16b, v6.16b
			eor v3.16b, v3.16b, v7.16b
			stp q0, q1, [Rptr], #(2*COMPV_GAS_Q_SZ_BYTES)
			stp q2, q3, [Rptr], #(2*COMPV_GAS_Q_SZ_BYTES)
			subs i, i, #64
			bne LoopWidth64_CompVBitsLogicalXorVt_8u_Asm_NEON64
		EndOf_LoopWidth64_CompVBitsLogicalXorVt_8u_Asm_NEON64:

		###########################################################
		# for (; i < width; i += 16)
		###########################################################
		cbz width16, EndOf_LoopWidth16_CompVBitsLogicalXorVt_8u_Asm_NEON64
		mov i, width16
		LoopWidth16_CompVBitsLogicalXorVt_8u_Asm_NEON64:
			ldr q0, [Aptr], #(1*COMPV_GAS_Q_SZ_BYTES)
			ldr q4, [A_Minus1_ptr], #(1*COMPV_GAS_Q_SZ_BYTES)
			eor v0.16b, v0.16b, v4.16b
			str q0, [Rptr], #(1*COMPV_GAS_Q_SZ_BYTES)
			subs i, i, #16
			bne LoopWidth16_CompVBitsLogicalXorVt_8u_Asm_NEON64
		EndOf_LoopWidth16_CompVBitsLogicalXorVt_8u_Asm_NEON64:

		add Rptr, Rptr, Rstride
		add Aptr, Aptr, Astride
		add A_Minus1_ptr, A_Minus1_ptr, Astride
		subs height, height, #1
		bne LoopHeight_CompVBitsLogicalXorVt_8u_Asm_NEON64
	EndOf_LoopHeight_CompVBitsLogicalXorVt_8u_Asm_NEON64:

	## UnDefines ##
	.unreq Aptr
	.unreq A_Minus1_ptr
	.unreq Rptr
	.unreq width
	.unreq height
	.unreq Astride
	.unreq Rstride
	.unreq i
	.unreq width64
	.unreq width16

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__aarch64__) */

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

.extern
 
.text

#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) void* dataDstPtr
# arg(1) -> COMPV_ALIGNED(NEON) const void* dataSrcPtr
# arg(2) -> compv_uscalar_t size
COMPV_GAS_FUNCTION_DECLARE CompVMemCopy_Asm_NEON64
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Defines ##
	dataDstPtr .req r0
	dataSrcPtr .req r1
	size .req r2
    sizeNEON .req r3
	sizeDOUBLE .req r4
	sizeBYTE .req r5

	prfm pldl1keep, [dataSrcPtr, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [dataSrcPtr, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [dataSrcPtr, #(CACHE_LINE_SIZE*2)]

    ###########################################################
    # for (i = 0; i < sizeNEON; i += 64)
    ###########################################################
    ands sizeNEON, size, #-64
    cbz sizeNEON, EndOfLoopCountNEON_CompVMemCopy_Asm_NEON64
    LoopCountNEON_CompVMemCopy_Asm_NEON64:
		prfm pldl1keep, [dataSrcPtr, #(CACHE_LINE_SIZE*3)]
        ld1 {v0.16b-v3.16b}, [dataSrcPtr], #(4*COMPV_GAS_V_SZ_BYTES)
		st1 {v0.16b-v3.16b}, [dataDstPtr], #(4*COMPV_GAS_V_SZ_BYTES)
		subs sizeNEON, sizeNEON, #(4*COMPV_GAS_V_SZ_BYTES)
        bne LoopCountNEON_CompVMemCopy_Asm_NEON64
	EndOfLoopCountNEON_CompVMemCopy_Asm_NEON64:

	###########################################################
    # for (i = 0; i < sizeDOUBLE; i += 8)
    ###########################################################
	ands sizeDOUBLE, size, #63
	lsr sizeDOUBLE, sizeDOUBLE, #3
	cbz sizeDOUBLE, EndOfLoopCountDOUBLE_CompVMemCopy_Asm_NEON64
    LoopCountDOUBLE_CompVMemCopy_Asm_NEON64:
        ldr r11, [dataSrcPtr], #8
        subs sizeDOUBLE, sizeDOUBLE, #1
		str r11, [dataDstPtr], #8
        bne LoopCountDOUBLE_CompVMemCopy_Asm_NEON64
	EndOfLoopCountDOUBLE_CompVMemCopy_Asm_NEON64:

	###########################################################
    # for (i = 0; i < sizeBYTE; i += 1)
    ###########################################################
	ands sizeBYTE, size, #7
	cbz sizeBYTE, EndOfLoopCountBYTE_CompVMemCopy_Asm_NEON64
    LoopCountBYTE_CompVMemCopy_Asm_NEON64:
        ldrb r11w, [dataSrcPtr], #1
        subs sizeBYTE, sizeBYTE, #1
		strb r11w, [dataDstPtr], #1
        bne LoopCountBYTE_CompVMemCopy_Asm_NEON64
	EndOfLoopCountBYTE_CompVMemCopy_Asm_NEON64:

	## UnDefines ##
	.unreq dataDstPtr
	.unreq dataSrcPtr
	.unreq size
    .unreq sizeNEON
	.unreq sizeDOUBLE
	.unreq sizeBYTE

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) void* dataDstPtr
# arg(1) -> compv_uscalar_t size
COMPV_GAS_FUNCTION_DECLARE CompVMemZero_Asm_NEON64
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Defines ##
	dataDstPtr .req r0
	size .req r1
    sizeNEON .req r2
	sizeDOUBLE .req r3
	sizeBYTE .req r4
	zero .req r5
	zerow .req r5w

	mov zero, #0
	eor v0.16b, v0.16b, v0.16b
	eor v1.16b, v1.16b, v1.16b
	eor v2.16b, v2.16b, v2.16b
	eor v3.16b, v3.16b, v3.16b

    ###########################################################
    # for (i = 0; i < sizeNEON; i += 64)
    ###########################################################
    ands sizeNEON, size, #-64
    beq EndOfLoopCountNEON_CompVMemZero_Asm_NEON64
    LoopCountNEON_CompVMemZero_Asm_NEON64:
		st1 { v0.16b-v3.16b }, [dataDstPtr], #(4*COMPV_GAS_V_SZ_BYTES)
        subs sizeNEON, sizeNEON, #(4*COMPV_GAS_V_SZ_BYTES)
        bne LoopCountNEON_CompVMemZero_Asm_NEON64
	EndOfLoopCountNEON_CompVMemZero_Asm_NEON64:

	###########################################################
    # for (i = 0; i < sizeDOUBLE; i += 8)
    ###########################################################
	ands sizeDOUBLE, size, #63
	lsr sizeDOUBLE, sizeDOUBLE, #3
	cbz sizeDOUBLE, EndOfLoopCountDOUBLE_CompVMemZero_Asm_NEON64
    LoopCountDOUBLE_CompVMemZero_Asm_NEON64:
        str zero, [dataDstPtr], #8
        subs sizeDOUBLE, sizeDOUBLE, #1
        bne LoopCountDOUBLE_CompVMemZero_Asm_NEON64
	EndOfLoopCountDOUBLE_CompVMemZero_Asm_NEON64:

	###########################################################
    # for (i = 0; i < sizeBYTE; i += 1)
    ###########################################################
	ands sizeBYTE, size, #7
	cbz sizeBYTE, EndOfLoopCountBYTE_CompVMemZero_Asm_NEON64
    LoopCountBYTE_CompVMemZero_Asm_NEON64:
        strb zerow, [dataDstPtr], #1
        subs sizeBYTE, sizeBYTE, #1
        bne LoopCountBYTE_CompVMemZero_Asm_NEON64
	EndOfLoopCountBYTE_CompVMemZero_Asm_NEON64:

	## UnDefines ##
	.unreq dataDstPtr
	.unreq size
    .unreq sizeNEON
	.unreq sizeDOUBLE
	.unreq sizeBYTE
	.unreq zero
	.unreq zerow

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) uint8_t* dstPt0
# arg(1) -> COMPV_ALIGNED(NEON) uint8_t* dstPt1
# arg(2) -> COMPV_ALIGNED(NEON) uint8_t* dstPt2
# arg(3) -> COMPV_ALIGNED(NEON) const compv_uint8x3_t* srcPtr
# arg(4) -> compv_uscalar_t width
# arg(5) -> compv_uscalar_t height
# arg(6) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVMemCopy3_Asm_NEON64
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Declare input arguments ##
	dstPt0 .req r0
	dstPt1 .req r1
	dstPt2 .req r2
	srcPtr .req r3
	width .req r4
	height .req r5
	stride .req r6

	prfm pldl1keep, [srcPtr, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [srcPtr, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [srcPtr, #(CACHE_LINE_SIZE*2)]
	
	## Declare local vectors ##
	pad .req r7
	pad3 .req r8
	i .req r9

	add pad, width, #15
	and pad, pad, #-16
	sub pad, stride, pad
	add pad3, pad, pad, LSL #1

	#################################################
	# for (compv_uscalar_t j = 0; j < height; ++j)
	#################################################
	LoopHeight_CompVMemCopy3_Asm_NEON64:
		#################################################
		# for (compv_uscalar_t i = 0; i < width; i += 16)
		#################################################
		mov i, #0
		LoopWidth_CompVMemCopy3_Asm_NEON64:
			prfm pldl1keep, [srcPtr, #(CACHE_LINE_SIZE*4)]
			//prfm pstl1keep, [dstPt0, #(CACHE_LINE_SIZE*4)]
			//prfm pstl1keep, [dstPt1, #(CACHE_LINE_SIZE*4)]
			//prfm pstl1keep, [dstPt2, #(CACHE_LINE_SIZE*4)]
			ld3 {v0.16b, v1.16b, v2.16b}, [srcPtr], #((16*3)*COMPV_GAS_UINT8_SZ_BYTES)
			#if 0 // MediaPad2, this code is by faaar slower (2700ms vs 2300ms)
			st1 {v0.16b}, [dstPt0], #(1*COMPV_GAS_V_SZ_BYTES)
			st1 {v1.16b}, [dstPt1], #(1*COMPV_GAS_V_SZ_BYTES)
			st1 {v2.16b}, [dstPt2], #(1*COMPV_GAS_V_SZ_BYTES)
			#else
			str q0, [dstPt0, i]
			str q1, [dstPt1, i]
			str q2, [dstPt2, i]
			#endif
			add i, i, #16
			cmp i, width
			blt LoopWidth_CompVMemCopy3_Asm_NEON64
		EndOf_LoopWidth_CompVMemCopy3_Asm_NEON64:
		subs height, height, #1
		#if 0 // See above
		add dstPt0, dstPt0, pad
		add dstPt1, dstPt1, pad
		add dstPt2, dstPt2, pad
		#else
		add dstPt0, dstPt0, stride
		add dstPt1, dstPt1, stride
		add dstPt2, dstPt2, stride
		#endif
		add srcPtr, srcPtr, pad3
		bne LoopHeight_CompVMemCopy3_Asm_NEON64
	EndOf_LoopHeight_CompVMemCopy3_Asm_NEON64:

	.unreq dstPt0
	.unreq dstPt1
	.unreq dstPt2
	.unreq srcPtr
	.unreq width
	.unreq height
	.unreq stride
	.unreq pad
	.unreq pad3
	.unreq i

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__aarch64__) */

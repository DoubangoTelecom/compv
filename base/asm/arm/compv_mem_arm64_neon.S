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
# arg(3) -> COMPV_ALIGNED(NEON) uint8_t* dstPt3
# arg(4) -> COMPV_ALIGNED(NEON) const compv_uint8x4_t* srcPtr
# arg(5) -> compv_uscalar_t width
# arg(6) -> compv_uscalar_t height
# arg(7) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVMemUnpack4_Asm_NEON64
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Declare input arguments ##
	dstPt0 .req r0
	dstPt1 .req r1
	dstPt2 .req r2
	dstPt3 .req r3
	srcPtr .req r4
	width .req r5
	height .req r6
	stride .req r7

	prfm pldl1keep, [srcPtr, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [srcPtr, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [srcPtr, #(CACHE_LINE_SIZE*2)]
	prfm pldl1keep, [srcPtr, #(CACHE_LINE_SIZE*3)]
	
	## Declare local vectors ##
	pad .req r8
	pad4 .req r9
	i .req r10

	add pad, width, #15
	and pad, pad, #-16
	sub pad, stride, pad
	lsl pad4, pad, #2

	#################################################
	# for (compv_uscalar_t j = 0; j < height; ++j)
	#################################################
	LoopHeight_CompVMemUnpack4_Asm_NEON64:
		#################################################
		# for (compv_uscalar_t i = 0; i < width; i += 16)
		#################################################
		mov i, #0
		LoopWidth_CompVMemUnpack4_Asm_NEON64:
			ld4 {v0.16b, v1.16b, v2.16b, v3.16b}, [srcPtr], #((16*4)*COMPV_GAS_UINT8_SZ_BYTES)
			prfm pldl1keep, [srcPtr, #(CACHE_LINE_SIZE*4)]
			#if 0 // MediaPad2, this code is by faaar slower (2700ms vs 2300ms)
			st1 {v0.16b}, [dstPt0], #(1*COMPV_GAS_V_SZ_BYTES)
			st1 {v1.16b}, [dstPt1], #(1*COMPV_GAS_V_SZ_BYTES)
			st1 {v2.16b}, [dstPt2], #(1*COMPV_GAS_V_SZ_BYTES)
			st1 {v3.16b}, [dstPt3], #(1*COMPV_GAS_V_SZ_BYTES)
			#else
			str q0, [dstPt0, i]
			str q1, [dstPt1, i]
			str q2, [dstPt2, i]
			str q3, [dstPt3, i]
			#endif
			add i, i, #16
			cmp i, width
			blt LoopWidth_CompVMemUnpack4_Asm_NEON64
		EndOf_LoopWidth_CompVMemUnpack4_Asm_NEON64:
		subs height, height, #1
		#if 0 // See above
		add dstPt0, dstPt0, pad
		add dstPt1, dstPt1, pad
		add dstPt2, dstPt2, pad
		add dstPt3, dstPt3, pad
		#else
		add dstPt0, dstPt0, stride
		add dstPt1, dstPt1, stride
		add dstPt2, dstPt2, stride
		add dstPt3, dstPt3, stride
		#endif
		add srcPtr, srcPtr, pad4
		bne LoopHeight_CompVMemUnpack4_Asm_NEON64
	EndOf_LoopHeight_CompVMemUnpack4_Asm_NEON64:

	.unreq dstPt0
	.unreq dstPt1
	.unreq dstPt2
	.unreq dstPt3
	.unreq srcPtr
	.unreq width
	.unreq height
	.unreq stride
	.unreq pad
	.unreq pad4
	.unreq i

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
COMPV_GAS_FUNCTION_DECLARE CompVMemUnpack3_Asm_NEON64
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
	LoopHeight_CompVMemUnpack3_Asm_NEON64:
		#################################################
		# for (compv_uscalar_t i = 0; i < width; i += 16)
		#################################################
		mov i, #0
		LoopWidth_CompVMemUnpack3_Asm_NEON64:
			ld3 {v0.16b, v1.16b, v2.16b}, [srcPtr], #((16*3)*COMPV_GAS_UINT8_SZ_BYTES)
			prfm pldl1keep, [srcPtr, #(CACHE_LINE_SIZE*4)]
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
			blt LoopWidth_CompVMemUnpack3_Asm_NEON64
		EndOf_LoopWidth_CompVMemUnpack3_Asm_NEON64:
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
		bne LoopHeight_CompVMemUnpack3_Asm_NEON64
	EndOf_LoopHeight_CompVMemUnpack3_Asm_NEON64:

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

#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) uint8_t* dstPt0
# arg(1) -> COMPV_ALIGNED(NEON) uint8_t* dstPt1
# arg(2) -> COMPV_ALIGNED(NEON) const compv_uint8x2_t* srcPtr
# arg(3) -> compv_uscalar_t width
# arg(4) -> compv_uscalar_t height
# arg(5) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVMemUnpack2_Asm_NEON64
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Declare input arguments ##
	dstPt0 .req r0
	dstPt1 .req r1
	srcPtr .req r2
	width .req r3
	height .req r4
	stride .req r5

	prfm pldl1keep, [srcPtr, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [srcPtr, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [srcPtr, #(CACHE_LINE_SIZE*2)]
	
	## Declare local vectors ##
	pad .req r6
	pad2 .req r7
	i .req r8

	add pad, width, #15
	and pad, pad, #-16
	sub pad, stride, pad
	lsl pad2, pad, #1

	#################################################
	# for (compv_uscalar_t j = 0; j < height; ++j)
	#################################################
	LoopHeight_CompVMemUnpack2_Asm_NEON64:
		#################################################
		# for (compv_uscalar_t i = 0; i < width; i += 16)
		#################################################
		mov i, #0
		LoopWidth_CompVMemUnpack2_Asm_NEON64:
			ld2 {v0.16b, v1.16b}, [srcPtr], #((16*2)*COMPV_GAS_UINT8_SZ_BYTES)
			prfm pldl1keep, [srcPtr, #(CACHE_LINE_SIZE*4)]
			#if 0 // MediaPad2, this code is by faaar slower (2700ms vs 2300ms)
			st1 {v0.16b}, [dstPt0], #(1*COMPV_GAS_V_SZ_BYTES)
			st1 {v1.16b}, [dstPt1], #(1*COMPV_GAS_V_SZ_BYTES)
			#else
			str q0, [dstPt0, i]
			str q1, [dstPt1, i]
			#endif
			add i, i, #16
			cmp i, width
			blt LoopWidth_CompVMemUnpack2_Asm_NEON64
		EndOf_LoopWidth_CompVMemUnpack2_Asm_NEON64:
		subs height, height, #1
		#if 0 // See above
		add dstPt0, dstPt0, pad
		add dstPt1, dstPt1, pad
		#else
		add dstPt0, dstPt0, stride
		add dstPt1, dstPt1, stride
		#endif
		add srcPtr, srcPtr, pad2
		bne LoopHeight_CompVMemUnpack2_Asm_NEON64
	EndOf_LoopHeight_CompVMemUnpack2_Asm_NEON64:

	.unreq dstPt0
	.unreq dstPt1
	.unreq srcPtr
	.unreq width
	.unreq height
	.unreq stride
	.unreq pad
	.unreq pad2
	.unreq i

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) compv_uint8x4_t* dstPtr
# arg(1) -> COMPV_ALIGNED(NEON) const uint8_t* srcPt0
# arg(2) -> COMPV_ALIGNED(NEON) const uint8_t* srcPt1
# arg(3) -> COMPV_ALIGNED(NEON) const uint8_t* srcPt2
# arg(4) -> COMPV_ALIGNED(NEON) const uint8_t* srcPt3
# arg(5) -> compv_uscalar_t width
# arg(6) -> compv_uscalar_t height
# arg(7) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVMemPack4_Asm_NEON64
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Declare input arguments ##
	dstPtr .req r0
	srcPt0 .req r1
	srcPt1 .req r2
	srcPt2 .req r3
	srcPt3 .req r4
	width .req r5
	height .req r6
	stride .req r7

	prfm pldl1keep, [srcPt0, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [srcPt0, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [srcPt0, #(CACHE_LINE_SIZE*2)]
	prfm pldl1keep, [srcPt0, #(CACHE_LINE_SIZE*3)]
	prfm pldl1keep, [srcPt1, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [srcPt1, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [srcPt1, #(CACHE_LINE_SIZE*2)]
	prfm pldl1keep, [srcPt1, #(CACHE_LINE_SIZE*3)]
	prfm pldl1keep, [srcPt2, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [srcPt2, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [srcPt2, #(CACHE_LINE_SIZE*2)]
	prfm pldl1keep, [srcPt2, #(CACHE_LINE_SIZE*3)]
	prfm pldl1keep, [srcPt3, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [srcPt3, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [srcPt3, #(CACHE_LINE_SIZE*2)]
	prfm pldl1keep, [srcPt3, #(CACHE_LINE_SIZE*3)]
	
	## Declare local vectors ##
	pad .req r8
	pad4 .req r9
	i .req r10

	add pad, width, #15
	and pad, pad, #-16
	sub pad, stride, pad
	lsl pad4, pad, #2

	#################################################
	# for (compv_uscalar_t j = 0; j < height; ++j)
	#################################################
	LoopHeight_CompVMemPack4_Asm_NEON64:
		#################################################
		# for (compv_uscalar_t i = 0; i < width; i += 16)
		#################################################
		mov i, #0
		LoopWidth_CompVMemPack4_Asm_NEON64:
			ld1 {v0.16b}, [srcPt0], #(16*COMPV_GAS_UINT8_SZ_BYTES)
			ld1 {v1.16b}, [srcPt1], #(16*COMPV_GAS_UINT8_SZ_BYTES)
			ld1 {v2.16b}, [srcPt2], #(16*COMPV_GAS_UINT8_SZ_BYTES)
			ld1 {v3.16b}, [srcPt3], #(16*COMPV_GAS_UINT8_SZ_BYTES)
			prfm pstl1keep, [srcPt0, #(CACHE_LINE_SIZE*4)]
			prfm pstl1keep, [srcPt1, #(CACHE_LINE_SIZE*4)]
			prfm pstl1keep, [srcPt2, #(CACHE_LINE_SIZE*4)]
			prfm pstl1keep, [srcPt3, #(CACHE_LINE_SIZE*4)]
			st4 { v0.16b, v1.16b, v2.16b, v3.16b }, [dstPtr], #(16*4*COMPV_GAS_UINT8_SZ_BYTES)
			add i, i, #16
			cmp i, width
			blt LoopWidth_CompVMemPack4_Asm_NEON64
		EndOf_LoopWidth_CompVMemPack4_Asm_NEON64:
		subs height, height, #1
		add srcPt0, srcPt0, pad
		add srcPt1, srcPt1, pad
		add srcPt2, srcPt2, pad
		add srcPt3, srcPt3, pad
		add dstPtr, dstPtr, pad4
		bne LoopHeight_CompVMemPack4_Asm_NEON64
	EndOf_LoopHeight_CompVMemPack4_Asm_NEON64:

	.unreq srcPt0
	.unreq srcPt1
	.unreq srcPt2
	.unreq srcPt3
	.unreq dstPtr
	.unreq width
	.unreq height
	.unreq stride
	.unreq pad
	.unreq pad4
	.unreq i

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) compv_uint8x3_t* dstPtr
# arg(1) -> COMPV_ALIGNED(NEON) const uint8_t* srcPt0
# arg(2) -> COMPV_ALIGNED(NEON) const uint8_t* srcPt1
# arg(3) -> COMPV_ALIGNED(NEON) const uint8_t* srcPt2
# arg(4) -> compv_uscalar_t width
# arg(5) -> compv_uscalar_t height
# arg(6) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVMemPack3_Asm_NEON64
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Declare input arguments ##
	dstPtr .req r0
	srcPt0 .req r1
	srcPt1 .req r2
	srcPt2 .req r3
	width .req r4
	height .req r5
	stride .req r6

	prfm pldl1keep, [srcPt0, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [srcPt0, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [srcPt0, #(CACHE_LINE_SIZE*2)]
	prfm pldl1keep, [srcPt1, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [srcPt1, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [srcPt1, #(CACHE_LINE_SIZE*2)]
	prfm pldl1keep, [srcPt2, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [srcPt2, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [srcPt2, #(CACHE_LINE_SIZE*2)]
	
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
	LoopHeight_CompVMemPack3_Asm_NEON64:
		#################################################
		# for (compv_uscalar_t i = 0; i < width; i += 16)
		#################################################
		mov i, #0
		LoopWidth_CompVMemPack3_Asm_NEON64:
			ld1 {v0.16b}, [srcPt0], #(16*COMPV_GAS_UINT8_SZ_BYTES)
			ld1 {v1.16b}, [srcPt1], #(16*COMPV_GAS_UINT8_SZ_BYTES)
			ld1 {v2.16b}, [srcPt2], #(16*COMPV_GAS_UINT8_SZ_BYTES)
			prfm pstl1keep, [srcPt0, #(CACHE_LINE_SIZE*4)]
			prfm pstl1keep, [srcPt1, #(CACHE_LINE_SIZE*4)]
			prfm pstl1keep, [srcPt2, #(CACHE_LINE_SIZE*4)]
			st3 { v0.16b, v1.16b, v2.16b }, [dstPtr], #(16*3*COMPV_GAS_UINT8_SZ_BYTES)
			add i, i, #16
			cmp i, width
			blt LoopWidth_CompVMemPack3_Asm_NEON64
		EndOf_LoopWidth_CompVMemPack3_Asm_NEON64:
		subs height, height, #1
		add srcPt0, srcPt0, pad
		add srcPt1, srcPt1, pad
		add srcPt2, srcPt2, pad
		add dstPtr, dstPtr, pad3
		bne LoopHeight_CompVMemPack3_Asm_NEON64
	EndOf_LoopHeight_CompVMemPack3_Asm_NEON64:

	.unreq srcPt0
	.unreq srcPt1
	.unreq srcPt2
	.unreq dstPtr
	.unreq width
	.unreq height
	.unreq stride
	.unreq pad
	.unreq pad3
	.unreq i

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) compv_uint8x2_t* dstPtr
# arg(1) -> COMPV_ALIGNED(NEON) const uint8_t* srcPt0
# arg(2) -> COMPV_ALIGNED(NEON) const uint8_t* srcPt1
# arg(3) -> compv_uscalar_t width
# arg(4) -> compv_uscalar_t height
# arg(5) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVMemPack2_Asm_NEON64
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Declare input arguments ##
	dstPtr .req r0
	srcPt0 .req r1
	srcPt1 .req r2
	width .req r3
	height .req r4
	stride .req r5

	prfm pldl1keep, [srcPt0, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [srcPt0, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [srcPt0, #(CACHE_LINE_SIZE*2)]
	prfm pldl1keep, [srcPt1, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [srcPt1, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [srcPt1, #(CACHE_LINE_SIZE*2)]
	
	## Declare local vectors ##
	pad .req r6
	pad2 .req r7
	i .req r8

	add pad, width, #15
	and pad, pad, #-16
	sub pad, stride, pad
	lsl pad2, pad, #1

	#################################################
	# for (compv_uscalar_t j = 0; j < height; ++j)
	#################################################
	LoopHeight_CompVMemPack2_Asm_NEON64:
		#################################################
		# for (compv_uscalar_t i = 0; i < width; i += 16)
		#################################################
		mov i, #0
		LoopWidth_CompVMemPack2_Asm_NEON64:
			ld1 {v0.16b}, [srcPt0], #(16*COMPV_GAS_UINT8_SZ_BYTES)
			ld1 {v1.16b}, [srcPt1], #(16*COMPV_GAS_UINT8_SZ_BYTES)
			prfm pstl1keep, [srcPt0, #(CACHE_LINE_SIZE*4)]
			prfm pstl1keep, [srcPt1, #(CACHE_LINE_SIZE*4)]
			st2 { v0.16b, v1.16b }, [dstPtr], #(16*2*COMPV_GAS_UINT8_SZ_BYTES)
			add i, i, #16
			cmp i, width
			blt LoopWidth_CompVMemPack2_Asm_NEON64
		EndOf_LoopWidth_CompVMemPack2_Asm_NEON64:
		subs height, height, #1
		add srcPt0, srcPt0, pad
		add srcPt1, srcPt1, pad
		add dstPtr, dstPtr, pad2
		bne LoopHeight_CompVMemPack2_Asm_NEON64
	EndOf_LoopHeight_CompVMemPack2_Asm_NEON64:

	.unreq srcPt0
	.unreq srcPt1
	.unreq dstPtr
	.unreq width
	.unreq height
	.unreq stride
	.unreq pad
	.unreq pad2
	.unreq i

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__aarch64__) */

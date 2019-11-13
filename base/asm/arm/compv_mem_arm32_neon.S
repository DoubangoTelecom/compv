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
@ arg(0) -> COMPV_ALIGNED(NEON) void* dataDstPtr
@ arg(1) -> COMPV_ALIGNED(NEON) const void* dataSrcPtr
@ arg(2) -> compv_uscalar_t size
COMPV_GAS_FUNCTION_DECLARE CompVMemCopy_Asm_NEON32
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	@@ Defines @@
	dataDstPtr .req r0
	dataSrcPtr .req r1
	size .req r2
    sizeNEON .req r3
	sizeWORD .req r4
	sizeBYTE .req r5

	pld [dataSrcPtr, #(CACHE_LINE_SIZE*0)]
	pld [dataSrcPtr, #(CACHE_LINE_SIZE*1)]
	pld [dataSrcPtr, #(CACHE_LINE_SIZE*2)]

    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (i = 0; i < sizeNEON; i += 64)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    ands sizeNEON, size, #-64
    beq EndOfLoopCountNEON_CompVMemCopy_Asm_NEON32
    LoopCountNEON_CompVMemCopy_Asm_NEON32:
		pld [dataSrcPtr, #(CACHE_LINE_SIZE*3)]
        vldm dataSrcPtr!, { q0-q3 }
		vstm dataDstPtr!, { q0-q3 }
		subs sizeNEON, sizeNEON, #64
        bne LoopCountNEON_CompVMemCopy_Asm_NEON32
	EndOfLoopCountNEON_CompVMemCopy_Asm_NEON32:

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (i = 0; i < sizeWORD; i += 4)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	and sizeWORD, size, #63
	lsrs sizeWORD, sizeWORD, #2
	beq EndOfLoopCountWORD_CompVMemCopy_Asm_NEON32
    LoopCountWORD_CompVMemCopy_Asm_NEON32:
        ldr r11, [dataSrcPtr], #4
        subs sizeWORD, sizeWORD, #1
		str r11, [dataDstPtr], #4
        bne LoopCountWORD_CompVMemCopy_Asm_NEON32
	EndOfLoopCountWORD_CompVMemCopy_Asm_NEON32:

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (i = 0; i < sizeBYTE; i += 1)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	ands sizeBYTE, size, #3
	beq EndOfLoopCountBYTE_CompVMemCopy_Asm_NEON32
    LoopCountBYTE_CompVMemCopy_Asm_NEON32:
        ldrb r11, [dataSrcPtr], #1
        subs sizeBYTE, sizeBYTE, #1
		strb r11, [dataDstPtr], #1
        bne LoopCountBYTE_CompVMemCopy_Asm_NEON32
	EndOfLoopCountBYTE_CompVMemCopy_Asm_NEON32:

	@@ UnDefines @@
	.unreq dataDstPtr
	.unreq dataSrcPtr
	.unreq size
    .unreq sizeNEON
	.unreq sizeWORD
	.unreq sizeBYTE

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) void* dataDstPtr
@ arg(1) -> compv_uscalar_t size
COMPV_GAS_FUNCTION_DECLARE CompVMemZero_Asm_NEON32
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	@@ Defines @@
	dataDstPtr .req r0
	size .req r1
    sizeNEON .req r2
	sizeWORD .req r3
	sizeBYTE .req r4
	zero .req r5

	mov zero, #0
	veor.u8 q0, q0, q0
	veor.u8 q1, q1, q1
	veor.u8 q2, q2, q2
	veor.u8 q3, q3, q3

    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (i = 0; i < sizeNEON; i += 64)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    ands sizeNEON, size, #-64
    beq EndOfLoopCountNEON_CompVMemZero_Asm_NEON32
    LoopCountNEON_CompVMemZero_Asm_NEON32:
		vstm dataDstPtr!, { q0-q3 }
		subs sizeNEON, sizeNEON, #64
        bne LoopCountNEON_CompVMemZero_Asm_NEON32
	EndOfLoopCountNEON_CompVMemZero_Asm_NEON32:

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (i = 0; i < sizeWORD; i += 4)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	and sizeWORD, size, #63
	lsrs sizeWORD, sizeWORD, #2
	beq EndOfLoopCountWORD_CompVMemZero_Asm_NEON32
    LoopCountWORD_CompVMemZero_Asm_NEON32:
        str zero, [dataDstPtr], #4
        subs sizeWORD, sizeWORD, #1
        bne LoopCountWORD_CompVMemZero_Asm_NEON32
	EndOfLoopCountWORD_CompVMemZero_Asm_NEON32:

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (i = 0; i < sizeBYTE; i += 1)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	ands sizeBYTE, size, #3
	beq EndOfLoopCountBYTE_CompVMemZero_Asm_NEON32
    LoopCountBYTE_CompVMemZero_Asm_NEON32:
        strb zero, [dataDstPtr], #1
        subs sizeBYTE, sizeBYTE, #1
        bne LoopCountBYTE_CompVMemZero_Asm_NEON32
	EndOfLoopCountBYTE_CompVMemZero_Asm_NEON32:

	@@ UnDefines @@
	.unreq dataDstPtr
	.unreq size
    .unreq sizeNEON
	.unreq sizeWORD
	.unreq sizeBYTE
	.unreq zero

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) uint8_t* dstPt0
@ arg(1) -> COMPV_ALIGNED(NEON) uint8_t* dstPt1
@ arg(2) -> COMPV_ALIGNED(NEON) uint8_t* dstPt2
@ arg(3) -> COMPV_ALIGNED(NEON) uint8_t* dstPt3
@ arg(4) -> COMPV_ALIGNED(NEON) const compv_uint8x4_t* srcPtr
@ arg(5) -> compv_uscalar_t width
@ arg(6) -> compv_uscalar_t height
@ arg(7) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVMemUnpack4_Asm_NEON32
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 8
	COMPV_GAS_SAVE_NEON_REGS

	@@ Declare input arguments @@
	ldm_args r0-r7
	dstPt0 .req r0
	dstPt1 .req r1
	dstPt2 .req r2
	dstPt3 .req r3
	srcPtr .req r4
	width .req r5
	height .req r6
	stride .req r7

	pld [srcPtr, #(CACHE_LINE_SIZE*0)]
	pld [srcPtr, #(CACHE_LINE_SIZE*1)]
	pld [srcPtr, #(CACHE_LINE_SIZE*2)]
	pld [srcPtr, #(CACHE_LINE_SIZE*3)]
	
	@@ Declare local vectors @@
	pad .req r8
	pad4 .req r9
	width1 .req r10
	i .req r11

	add width1, width, #15
	and width1, width1, #-16
	sub pad, stride, width1
	lsl pad4, pad, #2

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (compv_uscalar_t j = 0; j < height; ++j)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopHeight_CompVMemUnpack4_Asm_NEON32:
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (compv_uscalar_t i = 0; i < width; i += 16)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		mov i, width1
		LoopWidth_CompVMemUnpack4_Asm_NEON32:
			vld4.u8 {q0x, q1x, q2x, q3x}, [srcPtr :64]!
			vld4.u8 {q0y, q1y, q2y, q3y}, [srcPtr :64]!
			pld [srcPtr, #(CACHE_LINE_SIZE*4)]
			vst1.u8 {q0}, [dstPt0: 128]!
			vst1.u8 {q1}, [dstPt1: 128]!
			vst1.u8 {q2}, [dstPt2: 128]!
			vst1.u8 {q3}, [dstPt3: 128]!
			subs i, i, #16
			bne LoopWidth_CompVMemUnpack4_Asm_NEON32
		EndOf_LoopWidth_CompVMemUnpack4_Asm_NEON32:
		subs height, height, #1
		add dstPt0, dstPt0, pad
		add dstPt1, dstPt1, pad
		add dstPt2, dstPt2, pad
		add dstPt3, dstPt3, pad
		add srcPtr, srcPtr, pad4
		bne LoopHeight_CompVMemUnpack4_Asm_NEON32
	EndOf_LoopHeight_CompVMemUnpack4_Asm_NEON32:

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
	.unreq width1
	.unreq i

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 8
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) uint8_t* dstPt0
@ arg(1) -> COMPV_ALIGNED(NEON) uint8_t* dstPt1
@ arg(2) -> COMPV_ALIGNED(NEON) uint8_t* dstPt2
@ arg(3) -> COMPV_ALIGNED(NEON) const compv_uint8x3_t* srcPtr
@ arg(4) -> compv_uscalar_t width
@ arg(5) -> compv_uscalar_t height
@ arg(6) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVMemUnpack3_Asm_NEON32
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 7
	COMPV_GAS_SAVE_NEON_REGS

	@@ Declare input arguments @@
	ldm_args r0-r6
	dstPt0 .req r0
	dstPt1 .req r1
	dstPt2 .req r2
	srcPtr .req r3
	width .req r4
	height .req r5
	stride .req r6

	pld [srcPtr, #(CACHE_LINE_SIZE*0)]
	pld [srcPtr, #(CACHE_LINE_SIZE*1)]
	pld [srcPtr, #(CACHE_LINE_SIZE*2)]
	
	@@ Declare local vectors @@
	pad .req r7
	pad3 .req r8
	width1 .req r9
	i .req r10

	add width1, width, #15
	and width1, width1, #-16
	sub pad, stride, width1
	add pad3, pad, pad, LSL #1

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (compv_uscalar_t j = 0; j < height; ++j)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopHeight_CompVMemUnpack3_Asm_NEON32:
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (compv_uscalar_t i = 0; i < width; i += 16)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		mov i, width1
		LoopWidth_CompVMemUnpack3_Asm_NEON32:
			vld3.u8 {q0x, q1x, q2x}, [srcPtr :64]!
			vld3.u8 {q0y, q1y, q2y}, [srcPtr :64]!
			pld [srcPtr, #(CACHE_LINE_SIZE*3)]
			vst1.u8 {q0}, [dstPt0: 128]!
			vst1.u8 {q1}, [dstPt1: 128]!
			vst1.u8 {q2}, [dstPt2: 128]!
			subs i, i, #16
			bne LoopWidth_CompVMemUnpack3_Asm_NEON32
		EndOf_LoopWidth_CompVMemUnpack3_Asm_NEON32:
		subs height, height, #1
		add dstPt0, dstPt0, pad
		add dstPt1, dstPt1, pad
		add dstPt2, dstPt2, pad
		add srcPtr, srcPtr, pad3
		bne LoopHeight_CompVMemUnpack3_Asm_NEON32
	EndOf_LoopHeight_CompVMemUnpack3_Asm_NEON32:

	.unreq dstPt0
	.unreq dstPt1
	.unreq dstPt2
	.unreq srcPtr
	.unreq width
	.unreq height
	.unreq stride
	.unreq pad
	.unreq pad3
	.unreq width1
	.unreq i

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 7
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) uint8_t* dstPt0
@ arg(1) -> COMPV_ALIGNED(NEON) uint8_t* dstPt1
@ arg(2) -> COMPV_ALIGNED(NEON) const compv_uint8x2_t* srcPtr
@ arg(3) -> compv_uscalar_t width
@ arg(4) -> compv_uscalar_t height
@ arg(5) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVMemUnpack2_Asm_NEON32
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 6
	COMPV_GAS_SAVE_NEON_REGS

	@@ Declare input arguments @@
	ldm_args r0-r5
	dstPt0 .req r0
	dstPt1 .req r1
	srcPtr .req r2
	width .req r3
	height .req r4
	stride .req r5

	pld [srcPtr, #(CACHE_LINE_SIZE*0)]
	pld [srcPtr, #(CACHE_LINE_SIZE*1)]
	pld [srcPtr, #(CACHE_LINE_SIZE*2)]
	
	@@ Declare local vectors @@
	pad .req r6
	pad2 .req r7
	width1 .req r8
	i .req r9

	add width1, width, #15
	and width1, width1, #-16
	sub pad, stride, width1
	lsl pad2, pad, #1

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (compv_uscalar_t j = 0; j < height; ++j)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopHeight_CompVMemUnpack2_Asm_NEON32:
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (compv_uscalar_t i = 0; i < width; i += 16)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		mov i, width1
		LoopWidth_CompVMemUnpack2_Asm_NEON32:
			vld2.u8 {q0x, q1x}, [srcPtr :64]!
			vld2.u8 {q0y, q1y}, [srcPtr :64]!
			pld [srcPtr, #(CACHE_LINE_SIZE*3)]
			vst1.u8 {q0}, [dstPt0: 128]!
			vst1.u8 {q1}, [dstPt1: 128]!
			subs i, i, #16
			bne LoopWidth_CompVMemUnpack2_Asm_NEON32
		EndOf_LoopWidth_CompVMemUnpack2_Asm_NEON32:
		subs height, height, #1
		add dstPt0, dstPt0, pad
		add dstPt1, dstPt1, pad
		add srcPtr, srcPtr, pad2
		bne LoopHeight_CompVMemUnpack2_Asm_NEON32
	EndOf_LoopHeight_CompVMemUnpack2_Asm_NEON32:

	.unreq dstPt0
	.unreq dstPt1
	.unreq srcPtr
	.unreq width
	.unreq height
	.unreq stride
	.unreq pad
	.unreq pad2
	.unreq width1
	.unreq i

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 6
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) compv_uint8x4_t* dstPtr
@ arg(1) -> COMPV_ALIGNED(NEON) const uint8_t* srcPt0
@ arg(2) -> COMPV_ALIGNED(NEON) const uint8_t* srcPt1
@ arg(3) -> COMPV_ALIGNED(NEON) const uint8_t* srcPt2
@ arg(4) -> COMPV_ALIGNED(NEON) const uint8_t* srcPt3
@ arg(5) -> compv_uscalar_t width
@ arg(6) -> compv_uscalar_t height
@ arg(7) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVMemPack4_Asm_NEON32
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 8
	COMPV_GAS_SAVE_NEON_REGS

	@@ Declare input arguments @@
	ldm_args r0-r7
	dstPtr .req r0
	srcPt0 .req r1
	srcPt1 .req r2
	srcPt2 .req r3
	srcPt3 .req r4
	width .req r5
	height .req r6
	stride .req r7

	pld [srcPt0, #(CACHE_LINE_SIZE*0)]
	pld [srcPt0, #(CACHE_LINE_SIZE*1)]
	pld [srcPt0, #(CACHE_LINE_SIZE*2)]
	pld [srcPt0, #(CACHE_LINE_SIZE*3)]
	pld [srcPt1, #(CACHE_LINE_SIZE*0)]
	pld [srcPt1, #(CACHE_LINE_SIZE*1)]
	pld [srcPt1, #(CACHE_LINE_SIZE*2)]
	pld [srcPt1, #(CACHE_LINE_SIZE*3)]
	pld [srcPt2, #(CACHE_LINE_SIZE*0)]
	pld [srcPt2, #(CACHE_LINE_SIZE*1)]
	pld [srcPt2, #(CACHE_LINE_SIZE*2)]
	pld [srcPt2, #(CACHE_LINE_SIZE*3)]
	pld [srcPt3, #(CACHE_LINE_SIZE*0)]
	pld [srcPt3, #(CACHE_LINE_SIZE*1)]
	pld [srcPt3, #(CACHE_LINE_SIZE*2)]
	pld [srcPt3, #(CACHE_LINE_SIZE*3)]
	
	@@ Declare local vectors @@
	pad .req r8
	pad4 .req r9
	width1 .req r10
	i .req r11

	add width1, width, #15
	and width1, width1, #-16
	sub pad, stride, width1
	lsl pad4, pad, #2

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (compv_uscalar_t j = 0; j < height; ++j)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopHeight_CompVMemPack4_Asm_NEON32:
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (compv_uscalar_t i = 0; i < width; i += 16)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		mov i, width1
		LoopWidth_CompVMemPack4_Asm_NEON32:
			vld1.u8 {q0}, [srcPt0 :128]!
			vld1.u8 {q1}, [srcPt1 :128]!
			vld1.u8 {q2}, [srcPt2 :128]!
			vld1.u8 {q3}, [srcPt3 :128]!
			pld [srcPt0, #(CACHE_LINE_SIZE*4)]
			pld [srcPt1, #(CACHE_LINE_SIZE*4)]
			pld [srcPt2, #(CACHE_LINE_SIZE*4)]
			pld [srcPt3, #(CACHE_LINE_SIZE*4)]
			vst4.u8 {q0x, q1x, q2x, q3x}, [dstPtr: 64]!
			vst4.u8 {q0y, q1y, q2y, q3y}, [dstPtr: 64]!
			subs i, i, #16
			bne LoopWidth_CompVMemPack4_Asm_NEON32
		EndOf_LoopWidth_CompVMemPack4_Asm_NEON32:
		subs height, height, #1
		add srcPt0, srcPt0, pad
		add srcPt1, srcPt1, pad
		add srcPt2, srcPt2, pad
		add srcPt3, srcPt3, pad
		add dstPtr, dstPtr, pad4
		bne LoopHeight_CompVMemPack4_Asm_NEON32
	EndOf_LoopHeight_CompVMemPack4_Asm_NEON32:

	.unreq dstPtr
	.unreq srcPt0
	.unreq srcPt1
	.unreq srcPt2
	.unreq srcPt3
	.unreq width
	.unreq height
	.unreq stride
	.unreq pad
	.unreq pad4
	.unreq width1
	.unreq i

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 8
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) compv_uint8x3_t* dstPtr
@ arg(1) -> COMPV_ALIGNED(NEON) const uint8_t* srcPt0
@ arg(2) -> COMPV_ALIGNED(NEON) const uint8_t* srcPt1
@ arg(3) -> COMPV_ALIGNED(NEON) const uint8_t* srcPt2
@ arg(4) -> compv_uscalar_t width
@ arg(5) -> compv_uscalar_t height
@ arg(6) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVMemPack3_Asm_NEON32
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 7
	COMPV_GAS_SAVE_NEON_REGS

	@@ Declare input arguments @@
	ldm_args r0-r6
	dstPtr .req r0
	srcPt0 .req r1
	srcPt1 .req r2
	srcPt2 .req r3
	width .req r4
	height .req r5
	stride .req r6

	pld [srcPt0, #(CACHE_LINE_SIZE*0)]
	pld [srcPt0, #(CACHE_LINE_SIZE*1)]
	pld [srcPt0, #(CACHE_LINE_SIZE*2)]
	pld [srcPt1, #(CACHE_LINE_SIZE*0)]
	pld [srcPt1, #(CACHE_LINE_SIZE*1)]
	pld [srcPt1, #(CACHE_LINE_SIZE*2)]
	pld [srcPt2, #(CACHE_LINE_SIZE*0)]
	pld [srcPt2, #(CACHE_LINE_SIZE*1)]
	pld [srcPt2, #(CACHE_LINE_SIZE*2)]
	
	@@ Declare local vectors @@
	pad .req r7
	pad3 .req r8
	width1 .req r9
	i .req r10

	add width1, width, #15
	and width1, width1, #-16
	sub pad, stride, width1
	add pad3, pad, pad, LSL #1

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (compv_uscalar_t j = 0; j < height; ++j)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopHeight_CompVMemPack3_Asm_NEON32:
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (compv_uscalar_t i = 0; i < width; i += 16)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		mov i, width1
		LoopWidth_CompVMemPack3_Asm_NEON32:
			vld1.u8 {q0}, [srcPt0 :128]!
			vld1.u8 {q1}, [srcPt1 :128]!
			vld1.u8 {q2}, [srcPt2 :128]!
			pld [srcPt0, #(CACHE_LINE_SIZE*3)]
			pld [srcPt1, #(CACHE_LINE_SIZE*3)]
			pld [srcPt2, #(CACHE_LINE_SIZE*3)]
			vst3.u8 {q0x, q1x, q2x}, [dstPtr: 64]!
			vst3.u8 {q0y, q1y, q2y}, [dstPtr: 64]!
			subs i, i, #16
			bne LoopWidth_CompVMemPack3_Asm_NEON32
		EndOf_LoopWidth_CompVMemPack3_Asm_NEON32:
		subs height, height, #1
		add srcPt0, srcPt0, pad
		add srcPt1, srcPt1, pad
		add srcPt2, srcPt2, pad
		add dstPtr, dstPtr, pad3
		bne LoopHeight_CompVMemPack3_Asm_NEON32
	EndOf_LoopHeight_CompVMemPack3_Asm_NEON32:

	.unreq dstPtr
	.unreq srcPt0
	.unreq srcPt1
	.unreq srcPt2
	.unreq width
	.unreq height
	.unreq stride
	.unreq pad
	.unreq pad3
	.unreq width1
	.unreq i

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 7
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) compv_uint8x2_t* dstPtr
@ arg(1) -> COMPV_ALIGNED(NEON) const uint8_t* srcPt0
@ arg(2) -> COMPV_ALIGNED(NEON) const uint8_t* srcPt1
@ arg(3) -> compv_uscalar_t width
@ arg(4) -> compv_uscalar_t height
@ arg(5) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVMemPack2_Asm_NEON32
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 6
	COMPV_GAS_SAVE_NEON_REGS

	@@ Declare input arguments @@
	ldm_args r0-r5
	dstPtr .req r0
	srcPt0 .req r1
	srcPt1 .req r2
	width .req r3
	height .req r4
	stride .req r5

	pld [srcPt0, #(CACHE_LINE_SIZE*0)]
	pld [srcPt0, #(CACHE_LINE_SIZE*1)]
	pld [srcPt0, #(CACHE_LINE_SIZE*2)]
	pld [srcPt1, #(CACHE_LINE_SIZE*0)]
	pld [srcPt1, #(CACHE_LINE_SIZE*1)]
	pld [srcPt1, #(CACHE_LINE_SIZE*2)]
	
	@@ Declare local vectors @@
	pad .req r6
	pad2 .req r7
	width1 .req r8
	i .req r9

	add width1, width, #15
	and width1, width1, #-16
	sub pad, stride, width1
	lsl pad2, pad, #1

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (compv_uscalar_t j = 0; j < height; ++j)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopHeight_CompVMemPack2_Asm_NEON32:
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (compv_uscalar_t i = 0; i < width; i += 16)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		mov i, width1
		LoopWidth_CompVMemPack2_Asm_NEON32:
			vld1.u8 {q0}, [srcPt0 :128]!
			vld1.u8 {q1}, [srcPt1 :128]!
			pld [srcPt0, #(CACHE_LINE_SIZE*3)]
			pld [srcPt1, #(CACHE_LINE_SIZE*3)]
			vst2.u8 {q0x, q1x}, [dstPtr: 64]!
			vst2.u8 {q0y, q1y}, [dstPtr: 64]!
			subs i, i, #16
			bne LoopWidth_CompVMemPack2_Asm_NEON32
		EndOf_LoopWidth_CompVMemPack2_Asm_NEON32:
		subs height, height, #1
		add srcPt0, srcPt0, pad
		add srcPt1, srcPt1, pad
		add dstPtr, dstPtr, pad2
		bne LoopHeight_CompVMemPack2_Asm_NEON32
	EndOf_LoopHeight_CompVMemPack2_Asm_NEON32:

	.unreq dstPtr
	.unreq srcPt0
	.unreq srcPt1
	.unreq width
	.unreq height
	.unreq stride
	.unreq pad
	.unreq pad2
	.unreq width1
	.unreq i

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 6
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__arm__) && !defined(__aarch64__) */

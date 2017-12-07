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
@ arg(3) -> COMPV_ALIGNED(NEON) const compv_uint8x3_t* srcPtr
@ arg(4) -> compv_uscalar_t width
@ arg(5) -> compv_uscalar_t height
@ arg(6) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVMemCopy3_Asm_NEON32
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
	LoopHeight_CompVMemCopy3_Asm_NEON32:
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (compv_uscalar_t i = 0; i < width; i += 16)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		mov i, width1
		LoopWidth_CompVMemCopy3_Asm_NEON32:
			pld [srcPtr, #(CACHE_LINE_SIZE*3)]
			vld3.u8 {q0x, q1x, q2x}, [srcPtr :64]!
			vld3.u8 {q0y, q1y, q2y}, [srcPtr :64]!
			vst1.u8 {q0}, [dstPt0: 128]!
			vst1.u8 {q1}, [dstPt1: 128]!
			vst1.u8 {q2}, [dstPt2: 128]!
			subs i, i, #16
			bne LoopWidth_CompVMemCopy3_Asm_NEON32
		EndOf_LoopWidth_CompVMemCopy3_Asm_NEON32:
		subs height, height, #1
		add dstPt0, dstPt0, pad
		add dstPt1, dstPt1, pad
		add dstPt2, dstPt2, pad
		add srcPtr, srcPtr, pad3
		bne LoopHeight_CompVMemCopy3_Asm_NEON32
	EndOf_LoopHeight_CompVMemCopy3_Asm_NEON32:

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

#endif /* defined(__arm__) && !defined(__aarch64__) */

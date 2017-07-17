#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
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


#endif /* defined(__arm__) && !defined(__aarch64__) */

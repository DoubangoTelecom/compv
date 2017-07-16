#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__aarch64__)
.include "compv_common_arm64.S"

#if defined(__APPLE__)
#   define sym(funcname) _##funcname
#else
#   define sym(funcname) funcname
#endif

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
    beq EndOfLoopCountNEON_CompVMemCopy_Asm_NEON64
    LoopCountNEON_CompVMemCopy_Asm_NEON64:
		prfm pldl1keep, [dataSrcPtr, #(CACHE_LINE_SIZE*3)]
        ld1 {v0.16b-v3.16b}, [dataSrcPtr], #(4*COMPV_GAS_V_SZ_BYTES)
		st1 {v0.16b-v3.16b}, [dataDstPtr], #(4*COMPV_GAS_V_SZ_BYTES)
		subs sizeNEON, sizeNEON, #64
        bne LoopCountNEON_CompVMemCopy_Asm_NEON64
	EndOfLoopCountNEON_CompVMemCopy_Asm_NEON64:

	###########################################################
    # for (i = 0; i < sizeDOUBLE; i += 8)
    ###########################################################
	and sizeDOUBLE, size, #63
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
	beq EndOfLoopCountBYTE_CompVMemCopy_Asm_NEON64
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

#endif /* defined(__aarch64__) */

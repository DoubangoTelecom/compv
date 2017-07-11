#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__arm__) && !defined(__aarch64__)
.include "compv_common_arm32.S" @"

.data

.text

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> const uint8_t* dataPtr
@ arg(1) -> compv_uscalar_t width
@ arg(2) -> compv_uscalar_t height
@ arg(3) -> compv_uscalar_t stride
@ arg(4) -> uint32_t *histogramPtr
COMPV_GAS_FUNCTION_DECLARE CompVMathHistogramProcess_8u32s_Asm_NEON32
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 5
	COMPV_GAS_SAVE_NEON_REGS
	@@ end prolog @@

    @ Load arguments @
	ldm_args r0-r4

	dataPtr .req r0
	width .req r1
	height .req r2
	stride .req r3
	histogramPtr .req r4
    i .req r5

    sub stride, stride, width @ stride now holds padding

    @ !! Important: Pre-indexing 'dataPtr' to read data then, adding #4 at the end 
    @ !! is faster than Post-incrementing

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (j = 0; j < height; ++j)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopHeight_CompVMathHistogramProcess_8u32s_Asm_NEON32:
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (i = 0; i < maxWidthStep1; i += 8)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        and i, width, #-4
		LoopWidth4_CompVMathHistogramProcess_8u32s_Asm_NEON32:
			ldrb.w r6, [dataPtr, #(0*COMPV_GAS_UINT8_SZ_BYTES)]
			add.w r6, histogramPtr, r6, LSL #2
            ldr.w r10, [r6]
            ldrb.w r7, [dataPtr, #(1*COMPV_GAS_UINT8_SZ_BYTES)]
            add.w r10, r10, #1
            add.w r7, histogramPtr, r7, LSL #2
            str.w r10, [r6]
            ldr.w r10, [r7]
            ldrb.w r8, [dataPtr, #(2*COMPV_GAS_UINT8_SZ_BYTES)]
            add.w r10, r10, #1
            add.w r8, histogramPtr, r8, LSL #2
            str.w r10, [r7]
            ldr.w r10, [r8]
            ldrb.w r9, [dataPtr, #(3*COMPV_GAS_UINT8_SZ_BYTES)]
            add.w r10, r10, #1
            add.w r9, histogramPtr, r9, LSL #2
            str.w r10, [r8]
            ldr.w r10, [r9]
            subs i, i, #4
            add.w r10, r10, #1
            add dataPtr, dataPtr, #(4*COMPV_GAS_UINT8_SZ_BYTES)
            str.w r10, [r9]
			bne LoopWidth4_CompVMathHistogramProcess_8u32s_Asm_NEON32
			@@ EndOf_LoopWidth4 @@

		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (@ i < width@ ++i)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		ands i, width, #3
		beq EndOf_LoopWidth1_CompVMathHistogramProcess_8u32s_Asm_NEON32
		LoopWidth1_CompVMathHistogramProcess_8u32s_Asm_NEON32:
			ldrb r6, [dataPtr], #COMPV_GAS_UINT8_SZ_BYTES
            add r6, histogramPtr, r6, LSL #2
            ldr r10, [r6]
			subs i, i, #1
			add r10, r10, #1
			str r10, [r6]
			bne LoopWidth1_CompVMathHistogramProcess_8u32s_Asm_NEON32
			EndOf_LoopWidth1_CompVMathHistogramProcess_8u32s_Asm_NEON32:
			@@ EndOf_LoopWidth1 @@

		subs height, height, #1
		add dataPtr, dataPtr, stride
		bne LoopHeight_CompVMathHistogramProcess_8u32s_Asm_NEON32
		@@ EndOf_LoopHeight @@

	.unreq dataPtr			
	.unreq width			 
	.unreq height			
	.unreq stride			
	.unreq	histogramPtr		
	.unreq i		

	@@ begin epilog @@
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 5
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__arm__) && !defined(__aarch64__) */

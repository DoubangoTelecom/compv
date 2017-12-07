#########################################################################
# Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__aarch64__)
.include "compv_common_arm64.S" //

.data

.text

#########################################################
# arg(0) -> const uint8_t* dataPtr
# arg(1) -> compv_uscalar_t width
# arg(2) -> compv_uscalar_t height
# arg(3) -> compv_uscalar_t stride
# arg(4) -> uint32_t *histogramPtr
COMPV_GAS_FUNCTION_DECLARE CompVMathHistogramProcess_8u32s_Asm_NEON64
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS
	## end prolog ##

	dataPtr .req r0
	width .req r1
	height .req r2
	stride .req r3
	histogramPtr .req r4
    i .req r5

    sub stride, stride, width // stride now holds padding

    # !! Important: Pre-indexing 'dataPtr' to read data then, adding #4 at the end 
    # !! is faster than Post-incrementing

	######################################
	# for (j = 0; j < height; ++j)
	######################################
	LoopHeight_CompVMathHistogramProcess_8u32s_Asm_NEON64:
		##############################################
		# for (i = 0; i < maxWidthStep1; i += 8)
		##############################################
        and i, width, #-8
		LoopWidth4_CompVMathHistogramProcess_8u32s_Asm_NEON64:
#if defined(__APPLE__) // on iOS, this code is faster. On Android (MediaPad2 and Galaxy Tab A6) this code is #2 slooooower
            ldrb r6w, [dataPtr, #(0*COMPV_GAS_UINT8_SZ_BYTES)]
            ldrb r7w, [dataPtr, #(1*COMPV_GAS_UINT8_SZ_BYTES)]
            add r6, histogramPtr, r6, LSL #2
            ldr r14w, [r6]
            add r7, histogramPtr, r7, LSL #2
            add r14w, r14w, #1
            str r14w, [r6]
            ldrb r8w, [dataPtr, #(2*COMPV_GAS_UINT8_SZ_BYTES)]
            ldr r15w, [r7]
            add r8, histogramPtr, r8, LSL #2
            add r15w, r15w, #1
            str r15w, [r7]
            ldr r16w, [r8]
            ldrb r9w, [dataPtr, #(3*COMPV_GAS_UINT8_SZ_BYTES)]
            add r16w, r16w, #1
            add r9, histogramPtr, r9, LSL #2
            str r16w, [r8]
            ldr r17w, [r9]
            ldrb r10w, [dataPtr, #(4*COMPV_GAS_UINT8_SZ_BYTES)]
            add r17w, r17w, #1
            add r10, histogramPtr, r10, LSL #2
            str r17w, [r9]
            ldrb r11w, [dataPtr, #(5*COMPV_GAS_UINT8_SZ_BYTES)]
            ldr r19w, [r10]
            add r11, histogramPtr, r11, LSL #2
            add r19w, r19w, #1
            str r19w, [r10]
            ldr r20w, [r11]
            ldrb r12w, [dataPtr, #(6*COMPV_GAS_UINT8_SZ_BYTES)]
            add r20w, r20w, #1
            add r12, histogramPtr, r12, LSL #2
            str r20w, [r11]
            ldr r21w, [r12]
            ldrb r13w, [dataPtr, #(7*COMPV_GAS_UINT8_SZ_BYTES)]
            add r21w, r21w, #1
            add r13, histogramPtr, r13, LSL #2
            str r21w, [r12]
            ldr r22w, [r13]
            subs i, i, #8
            add r22w, r22w, #1
            add dataPtr, dataPtr, #(8*COMPV_GAS_UINT8_SZ_BYTES)
            str r22w, [r13]
#else
			ldrb r6w, [dataPtr, #(0*COMPV_GAS_UINT8_SZ_BYTES)]
			ldrb r7w, [dataPtr, #(1*COMPV_GAS_UINT8_SZ_BYTES)]
			ldrb r8w, [dataPtr, #(2*COMPV_GAS_UINT8_SZ_BYTES)]
			ldrb r9w, [dataPtr, #(3*COMPV_GAS_UINT8_SZ_BYTES)]
            ldr r11w, [histogramPtr, r6, LSL #2]
            add r11w, r11w, #1
            str r11w, [histogramPtr, r6, LSL #2]
            ldr r10w, [histogramPtr, r7, LSL #2]
            add r10w, r10w, #1
            str r10w, [histogramPtr, r7, LSL #2]
            ldr r11w, [histogramPtr, r8, LSL #2]
			add dataPtr, dataPtr, #(4*COMPV_GAS_UINT8_SZ_BYTES)
            add r11w, r11w, #1
            str r11w, [histogramPtr, r8, LSL #2]
            ldr r10w, [histogramPtr, r9, LSL #2]
            subs i, i, #4
            add r10w, r10w, #1
            str r10w, [histogramPtr, r9, LSL #2]
#endif
			cbnz i, LoopWidth4_CompVMathHistogramProcess_8u32s_Asm_NEON64
			## EndOf_LoopWidth4 ##

		##############################################
		# for (# i < width# ++i)
		##############################################
		ands i, width, #7
		cbz i, EndOf_LoopWidth1_CompVMathHistogramProcess_8u32s_Asm_NEON64
		LoopWidth1_CompVMathHistogramProcess_8u32s_Asm_NEON64:
#if defined(__APPLE__) // on iOS, this code is faster. On Android (MediaPad2 and Galaxy Tab A6) this code is #2 slooooower
			ldrb r6w, [dataPtr], #COMPV_GAS_UINT8_SZ_BYTES
            ldr r10w, [histogramPtr, r6, LSL #2]
			subs i, i, #1
			add r10w, r10w, #1
			str r10w, [histogramPtr, r6, LSL #2]
#else
			ldrb r6w, [dataPtr, #(0*COMPV_GAS_UINT8_SZ_BYTES)]
			add dataPtr, dataPtr, #(1*COMPV_GAS_UINT8_SZ_BYTES)
            ldr r10w, [histogramPtr, r6, LSL #2]
			subs i, i, #1
			add r10w, r10w, #1
			str r10w, [histogramPtr, r6, LSL #2]
#endif
			cbnz i, LoopWidth1_CompVMathHistogramProcess_8u32s_Asm_NEON64
			EndOf_LoopWidth1_CompVMathHistogramProcess_8u32s_Asm_NEON64:
			## EndOf_LoopWidth1 ##

		subs height, height, #1
		add dataPtr, dataPtr, stride
		bne LoopHeight_CompVMathHistogramProcess_8u32s_Asm_NEON64
		## EndOf_LoopHeight ##

	.unreq dataPtr			
	.unreq width			 
	.unreq height			
	.unreq stride			
	.unreq	histogramPtr		
	.unreq i		

	## begin epilog ##
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#endif /* defined(__aarch64__) */
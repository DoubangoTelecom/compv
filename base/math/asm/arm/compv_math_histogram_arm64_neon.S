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

#########################################################
# arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* ptrIn
# arg(1) -> COMPV_ALIGNED(NEON) int32_t* ptrOut
# arg(2) -> const compv_uscalar_t width
# arg(3) -> const compv_uscalar_t height
# arg(4) -> COMPV_ALIGNED(NEON) const compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVMathHistogramBuildProjectionX_8u32s_Asm_NEON64
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS
	## end prolog ##

	# Load arguments #
	ptrIn .req r0
	ptrOut .req r1
	width .req r2
	height .req r3
	stride .req r4

	# Local Variables #
    i .req r5
	width16 .req r6

	prfm pldl1keep, [ptrIn, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [ptrIn, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [ptrIn, #(CACHE_LINE_SIZE*2)]
	prfm pldl1keep, [ptrIn, #(CACHE_LINE_SIZE*3)]

	and width16, width, #-16
	sub stride, stride, width // stride now contains padding

	########################################################
	# for (i = 0; i < width16; i += 16)
	########################################################
	mov i, #0
	LoopMemsetWidth16_CompVMathHistogramBuildProjectionX_8u32s_Asm_NEON64:
		ldr q1, [ptrIn], #(1*COMPV_GAS_Q_SZ_BYTES)
		prfm pldl1keep, [ptrIn, #(CACHE_LINE_SIZE*4)]
		uxtl2 v3.8h, v1.16b
		uxtl v1.8h, v1.8b
		uxtl v0.4s, v1.4h
		uxtl2 v1.4s, v1.8h
		uxtl v2.4s, v3.4h
		uxtl2 v3.4s, v3.8h
		stp q0, q1, [ptrOut], #(2*COMPV_GAS_Q_SZ_BYTES) 
		stp q2, q3, [ptrOut], #(2*COMPV_GAS_Q_SZ_BYTES) 
		add i, i, #16
		cmp i, width16
		blt LoopMemsetWidth16_CompVMathHistogramBuildProjectionX_8u32s_Asm_NEON64
	EndOf_LoopMemsetWidth16_CompVMathHistogramBuildProjectionX_8u32s_Asm_NEON64:

	########################################################
	# for (; i < width; ++i)
	########################################################
	cmp i, width
	bge EndOf_LoopMemsetWidth1_CompVMathHistogramBuildProjectionX_8u32s_Asm_NEON64
	LoopMemsetWidth1_CompVMathHistogramBuildProjectionX_8u32s_Asm_NEON64:
		ldrb r11w, [ptrIn], #(1*COMPV_GAS_UINT8_SZ_BYTES)
		str r11w, [ptrOut], #(1*COMPV_GAS_UINT32_SZ_BYTES)
		add i, i, #1
		cmp i, width
		blt LoopMemsetWidth1_CompVMathHistogramBuildProjectionX_8u32s_Asm_NEON64
	EndOf_LoopMemsetWidth1_CompVMathHistogramBuildProjectionX_8u32s_Asm_NEON64:

	sub ptrOut, ptrOut, width, LSL #(COMPV_GAS_UINT32_SHIFT_BYTES)
	add ptrIn, ptrIn, stride
	subs height, height, #1
	beq EndOf_LoopHeight_CompVMathHistogramBuildProjectionX_8u32s_Asm_NEON64

	########################################################
	# for (compv_uscalar_t j = 1; j < height; ++j) 
	########################################################
	LoopHeight_CompVMathHistogramBuildProjectionX_8u32s_Asm_NEON64:
		########################################################
		# for (i = 16; i < width16; i += 16)
		########################################################
		mov i, #0
		LoopWidth16_CompVMathHistogramBuildProjectionX_8u32s_Asm_NEON64:
			ldr q4, [ptrIn], #(1*COMPV_GAS_Q_SZ_BYTES)
			uxtl2 v5.8h, v4.16b
			uxtl v4.8h, v4.8b
			ldp q0, q1, [ptrOut, #(0*COMPV_GAS_Q_SZ_BYTES)]
			ldp q2, q3, [ptrOut, #(2*COMPV_GAS_Q_SZ_BYTES)]
			uaddw v0.4s, v0.4s, v4.4h
			uaddw2 v1.4s, v1.4s, v4.8h
			uaddw v2.4s, v2.4s, v5.4h
			uaddw2 v3.4s, v3.4s, v5.8h
			stp q0, q1, [ptrOut], #(2*COMPV_GAS_Q_SZ_BYTES)
			stp q2, q3, [ptrOut], #(2*COMPV_GAS_Q_SZ_BYTES)
			add i, i, #16
			cmp i, width16
			blt LoopWidth16_CompVMathHistogramBuildProjectionX_8u32s_Asm_NEON64
		EndOf_LoopWidth16_CompVMathHistogramBuildProjectionX_8u32s_Asm_NEON64:

		########################################################
		# for (; i < width; ++i)
		########################################################
		cmp i, width
		bge EndOf_LoopWidth1_CompVMathHistogramBuildProjectionX_8u32s_Asm_NEON64
		LoopWidth1_CompVMathHistogramBuildProjectionX_8u32s_Asm_NEON64:
			ldrb r11w, [ptrIn], #(1*COMPV_GAS_UINT8_SZ_BYTES)
			ldr r10w, [ptrOut]
			add r10, r10, r11
			add i, i, #1
			cmp i, width
			str r10w, [ptrOut], #(1*COMPV_GAS_UINT32_SZ_BYTES)
			blt LoopWidth1_CompVMathHistogramBuildProjectionX_8u32s_Asm_NEON64
		EndOf_LoopWidth1_CompVMathHistogramBuildProjectionX_8u32s_Asm_NEON64:

		add ptrIn, ptrIn, stride
		sub ptrOut, ptrOut, width, LSL #(COMPV_GAS_UINT32_SHIFT_BYTES)

		subs height, height, #1
		bne LoopHeight_CompVMathHistogramBuildProjectionX_8u32s_Asm_NEON64
	EndOf_LoopHeight_CompVMathHistogramBuildProjectionX_8u32s_Asm_NEON64:

	.unreq ptrIn
	.unreq ptrOut
	.unreq width
	.unreq height
	.unreq stride

    .unreq i
	.unreq width16

	## begin epilog ##
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#########################################################
# arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* ptrIn
# arg(1) -> COMPV_ALIGNED(NEON) int32_t* ptrOut
# arg(2) -> const compv_uscalar_t width
# arg(3) -> const compv_uscalar_t height
# arg(4) -> COMPV_ALIGNED(NEON) const compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVMathHistogramBuildProjectionY_8u32s_Asm_NEON64
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS
	## end prolog ##

	# Load arguments #
	ptrIn .req r0
	ptrOut .req r1
	width .req r2
	height .req r3
	stride .req r4

	# Local Variables #
    i .req r5
	sum .req r6w
	width16 .req r7

	prfm pldl1keep, [ptrIn, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [ptrIn, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [ptrIn, #(CACHE_LINE_SIZE*2)]
	prfm pldl1keep, [ptrIn, #(CACHE_LINE_SIZE*3)]

	and width16, width, #-16
	sub stride, stride, width // stride now contains padding

	########################################################
	# for (compv_uscalar_t j = 0; j < height; ++j) 
	########################################################
	LoopHeight_CompVMathHistogramBuildProjectionY_8u32s_Asm_NEON64:
		ldr q1, [ptrIn], #(1*COMPV_GAS_Q_SZ_BYTES)
		prfm pldl1keep, [ptrIn, #(CACHE_LINE_SIZE*4)]
		uxtl2 v3.8h, v1.16b
		uxtl v1.8h, v1.8b
		uxtl v0.4s, v1.4h
		uxtl2 v1.4s, v1.8h
		uxtl v2.4s, v3.4h
		uxtl2 v3.4s, v3.8h
		########################################################
		# for (i = 16; i < width16; i += 16)
		########################################################
		cmp width16, #16
		mov i, #16
		ble EndOf_LoopWidth16_CompVMathHistogramBuildProjectionY_8u32s_Asm_NEON64
		LoopWidth16_CompVMathHistogramBuildProjectionY_8u32s_Asm_NEON64:
			ldr q4, [ptrIn], #(1*COMPV_GAS_Q_SZ_BYTES)
			uxtl2 v5.8h, v4.16b
			uxtl v4.8h, v4.8b
			uaddw v0.4s, v0.4s, v4.4h
			uaddw2 v1.4s, v1.4s, v4.8h
			uaddw v2.4s, v2.4s, v5.4h
			uaddw2 v3.4s, v3.4s, v5.8h
			add i, i, #16
			cmp i, width16
			blt LoopWidth16_CompVMathHistogramBuildProjectionY_8u32s_Asm_NEON64
		EndOf_LoopWidth16_CompVMathHistogramBuildProjectionY_8u32s_Asm_NEON64:

		add v0.4s, v0.4s, v2.4s
		add v1.4s, v1.4s, v3.4s
		add v0.4s, v0.4s, v1.4s

		addp v0.4s, v0.4s, v0.4s
		addp v0.2s, v0.2s, v0.2s
		mov sum, v0.s[0]

		########################################################
		# for (; i < width; ++i)
		########################################################
		cmp i, width
		bge EndOf_LoopWidth1_CompVMathHistogramBuildProjectionY_8u32s_Asm_NEON64
		LoopWidth1_CompVMathHistogramBuildProjectionY_8u32s_Asm_NEON64:
			ldrb r11w, [ptrIn], #(1*COMPV_GAS_UINT8_SZ_BYTES)
			add sum, sum, r11w
			add i, i, #1
			cmp i, width
			blt LoopWidth1_CompVMathHistogramBuildProjectionY_8u32s_Asm_NEON64
		EndOf_LoopWidth1_CompVMathHistogramBuildProjectionY_8u32s_Asm_NEON64:

		add ptrIn, ptrIn, stride
		str sum, [ptrOut], #(1*COMPV_GAS_UINT32_SZ_BYTES)

		subs height, height, #1
		bne LoopHeight_CompVMathHistogramBuildProjectionY_8u32s_Asm_NEON64
	EndOf_LoopHeight_CompVMathHistogramBuildProjectionY_8u32s_Asm_NEON64:

	.unreq ptrIn
	.unreq ptrOut
	.unreq width
	.unreq height
	.unreq stride

    .unreq i
	.unreq sum
	.unreq width16

	## begin epilog ##
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__aarch64__) */
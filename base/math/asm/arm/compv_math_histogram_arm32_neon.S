#########################################################################
# Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__arm__) && !defined(__aarch64__)
.include "compv_common_arm32.S" @

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
		@ for (i = 0; i < maxWidthStep1; i += 4)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        and i, width, #-4
		LoopWidth4_CompVMathHistogramProcess_8u32s_Asm_NEON32:
#if defined(__APPLE__) // on iOS, this code is faster. On Android (MediaPad2 and Galaxy Tab A6) this code is #2 slooooower
			ldrb.w r6, [dataPtr, #(0*COMPV_GAS_UINT8_SZ_BYTES)]
			add.w r6, histogramPtr, r6, LSL #2
            ldr.w r11, [r6]
            ldrb.w r7, [dataPtr, #(1*COMPV_GAS_UINT8_SZ_BYTES)]
            add.w r11, r11, #1
            add.w r7, histogramPtr, r7, LSL #2
            str.w r11, [r6]
            ldr.w r10, [r7]
            ldrb.w r8, [dataPtr, #(2*COMPV_GAS_UINT8_SZ_BYTES)]
            add.w r10, r10, #1
            add.w r8, histogramPtr, r8, LSL #2
            str.w r10, [r7]
            ldr.w r11, [r8]
            ldrb.w r9, [dataPtr, #(3*COMPV_GAS_UINT8_SZ_BYTES)]
            add.w r11, r11, #1
            add.w r9, histogramPtr, r9, LSL #2
            str.w r11, [r8]
            ldr.w r10, [r9]
            subs i, i, #4
            add.w r10, r10, #1
            add dataPtr, dataPtr, #(4*COMPV_GAS_UINT8_SZ_BYTES)
            str.w r10, [r9]
#else
			ldrb r6, [dataPtr, #(0*COMPV_GAS_UINT8_SZ_BYTES)]
			ldrb r7, [dataPtr, #(1*COMPV_GAS_UINT8_SZ_BYTES)]
			ldrb r8, [dataPtr, #(2*COMPV_GAS_UINT8_SZ_BYTES)]
			ldrb r9, [dataPtr, #(3*COMPV_GAS_UINT8_SZ_BYTES)]
            ldr r11, [histogramPtr, r6, LSL #2]
            add r11, r11, #1
            str r11, [histogramPtr, r6, LSL #2]
            ldr r10, [histogramPtr, r7, LSL #2]
            add r10, r10, #1
            str r10, [histogramPtr, r7, LSL #2]
            ldr r11, [histogramPtr, r8, LSL #2]
			add dataPtr, dataPtr, #(4*COMPV_GAS_UINT8_SZ_BYTES)
            add r11, r11, #1
            str r11, [histogramPtr, r8, LSL #2]
            ldr r10, [histogramPtr, r9, LSL #2]
            subs i, i, #4
            add r10, r10, #1
            str r10, [histogramPtr, r9, LSL #2]
#endif
			bne LoopWidth4_CompVMathHistogramProcess_8u32s_Asm_NEON32
			@@ EndOf_LoopWidth4 @@

		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (@ i < width@ ++i)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		ands i, width, #3
		beq EndOf_LoopWidth1_CompVMathHistogramProcess_8u32s_Asm_NEON32
		LoopWidth1_CompVMathHistogramProcess_8u32s_Asm_NEON32:
#if defined(__APPLE__) // on iOS, this code is faster. On Android (MediaPad2 and Galaxy Tab A6) this code is #2 slooooower
			ldrb r6, [dataPtr], #COMPV_GAS_UINT8_SZ_BYTES
            add r6, histogramPtr, r6, LSL #2
            ldr r10, [r6]
			subs i, i, #1
			add r10, r10, #1
			str r10, [r6]
#else
			ldrb r6, [dataPtr, #(0*COMPV_GAS_UINT8_SZ_BYTES)]
			add dataPtr, dataPtr, #(1*COMPV_GAS_UINT8_SZ_BYTES)
            ldr r10, [histogramPtr, r6, LSL #2]
			subs i, i, #1
			add r10, r10, #1
			str r10, [histogramPtr, r6, LSL #2]
#endif
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


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* ptrIn
@ arg(1) -> COMPV_ALIGNED(NEON) int32_t* ptrOut
@ arg(2) -> const compv_uscalar_t width
@ arg(3) -> const compv_uscalar_t height
@ arg(4) -> COMPV_ALIGNED(NEON) const compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVMathHistogramBuildProjectionY_8u32s_Asm_NEON32
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 5
	COMPV_GAS_SAVE_NEON_REGS
	@@ end prolog @@

	@ Load arguments @
	ldm_args r0-r4
	ptrIn .req r0
	ptrOut .req r1
	width .req r2
	height .req r3
	stride .req r4

	@ Local Variables @
    i .req r5
	sum .req r6
	width16 .req r7
	vec0 .req q0
	vec0x .req q0x
	vec0y .req q0y
	vec1 .req q1
	vec1x .req q1x
	vec1y .req q1y
	vec2 .req q2
	vec2x .req q2x
	vec2y .req q2y
	vec3 .req q3
	vec3x .req q3x
	vec3y .req q3y
	vec4 .req q4
	vec4x .req q4x
	vec4y .req q4y
	vec5 .req q5
	vec5x .req q5x
	vec5y .req q5y

	pld [ptrIn, #(CACHE_LINE_SIZE*0)]
	pld [ptrIn, #(CACHE_LINE_SIZE*1)]
	pld [ptrIn, #(CACHE_LINE_SIZE*2)]
	pld [ptrIn, #(CACHE_LINE_SIZE*3)]

	and width16, width, #-16
	sub stride, stride, width @ stride now contains padding

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (compv_uscalar_t j = 0; j < height; ++j) 
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopHeight_CompVMathHistogramBuildProjectionY_8u32s_Asm_NEON32:
		vld1.u8 { vec1 }, [ptrIn :128]!
		pld [ptrIn, #(CACHE_LINE_SIZE*4)]
		vmovl.u8 vec3, vec1y
		vmovl.u8 vec1, vec1x
		vmovl.u16 vec0, vec1x
		vmovl.u16 vec1, vec1y
		vmovl.u16 vec2, vec3x
		vmovl.u16 vec3, vec3y
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (i = 16; i < width16; i += 16)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		cmp width16, #16
		mov i, #16
		ble EndOf_LoopWidth16_CompVMathHistogramBuildProjectionY_8u32s_Asm_NEON32
		LoopWidth16_CompVMathHistogramBuildProjectionY_8u32s_Asm_NEON32:
			vld1.u8 { vec4 }, [ptrIn :128]!
			vmovl.u8 vec5, vec4y
			vmovl.u8 vec4, vec4x
			vaddw.u16 vec0, vec4x
			vaddw.u16 vec1, vec4y
			vaddw.u16 vec2, vec5x
			vaddw.u16 vec3, vec5y
			add i, i, #16
			cmp i, width16
			blt LoopWidth16_CompVMathHistogramBuildProjectionY_8u32s_Asm_NEON32
		EndOf_LoopWidth16_CompVMathHistogramBuildProjectionY_8u32s_Asm_NEON32:

		vaddq.u32 vec0, vec0, vec2
		vaddq.u32 vec1, vec1, vec3
		vaddq.u32 vec0, vec0, vec1

		vpadd.u32 vec0x, vec0x, vec0y
		vpadd.u32 vec0x, vec0x, vec0x
		vmov.u32 sum, vec0x[0]

		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (; i < width; ++i)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		cmp i, width
		bge EndOf_LoopWidth1_CompVMathHistogramBuildProjectionY_8u32s_Asm_NEON32
		LoopWidth1_CompVMathHistogramBuildProjectionY_8u32s_Asm_NEON32:
			ldrb r11, [ptrIn], #(1*COMPV_GAS_UINT8_SZ_BYTES)
			add sum, sum, r11
			add i, i, #1
			cmp i, width
			blt LoopWidth1_CompVMathHistogramBuildProjectionY_8u32s_Asm_NEON32
		EndOf_LoopWidth1_CompVMathHistogramBuildProjectionY_8u32s_Asm_NEON32:

		add ptrIn, ptrIn, stride
		str sum, [ptrOut], #(1*COMPV_GAS_UINT32_SZ_BYTES)

		subs height, height, #1
		bne LoopHeight_CompVMathHistogramBuildProjectionY_8u32s_Asm_NEON32
	EndOf_LoopHeight_CompVMathHistogramBuildProjectionY_8u32s_Asm_NEON32:

	.unreq ptrIn
	.unreq ptrOut
	.unreq width
	.unreq height
	.unreq stride

    .unreq i
	.unreq sum
	.unreq width16
	.unreq vec0
	.unreq vec0x
	.unreq vec0y
	.unreq vec1
	.unreq vec1x
	.unreq vec1y
	.unreq vec2
	.unreq vec2x
	.unreq vec2y
	.unreq vec3
	.unreq vec3x
	.unreq vec3y
	.unreq vec4
	.unreq vec4x
	.unreq vec4y
	.unreq vec5
	.unreq vec5x
	.unreq vec5y

	@@ begin epilog @@
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 5
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
	

#endif /* defined(__arm__) && !defined(__aarch64__) */

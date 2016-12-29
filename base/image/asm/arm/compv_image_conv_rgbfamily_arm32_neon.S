@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   @
@ File author: Mamadou DIOP (Doubango Telecom, France).                 @
@ License: GPLv3. For commercial license please contact us.             @
@ Source code: https://github.com/DoubangoTelecom/compv                 @
@ WebSite: http://compv.org                                             @
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.include "compv_common_arm.S"

.section .data

.extern k16_i16, k128_i16
 
.section .text
  
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* rgb24Ptr
@ arg(1) -> COMPV_ALIGNED(NEON) uint8_t* outYPtr
@ arg(2) -> compv_uscalar_t width
@ arg(3) -> compv_uscalar_t height
@ arg(4) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
@ arg(5) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_YCoeffs8
COMPV_GAS_FUNCTION_DECLARE CompVImageConvRgb24family_to_y_Asm_NEON32
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 6
	COMPV_GAS_SAVE_NEON_REGS
	
	ldmia_args r0-r5
	rgb24Ptr .req r0
	outYPtr .req r1
	width .req r2
	height .req r3
	stride .req r4
	kRGBfamilyToYUV_YCoeffs8 .req r5

	pld [rgb24Ptr]
	pld [rgb24Ptr, #64*1]
	pld [rgb24Ptr, #64*2]
	pld [rgb24Ptr, #64*3]

	add r6, width, #15
	and r6, r6, #-16
	sub r7, stride, r6
	padY .req r7
	add r8, r7, r7, LSL #1
	padRGB .req r8

	mov r9, #2048
	vdup.u16 q0, r9
	xmm2048 .req q0
	vld4.8 {d2, d3, d4, d5}, [kRGBfamilyToYUV_YCoeffs8 : 128]
	xmmCoeff0 .req d2
	xmmCoeff1 .req d3
	xmmCoeff2 .req d4

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (j = 0; j < height; ++j)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopHeight_CompVImageConvRgb24family_to_y_Asm_NEON32:
		mov r9, #0
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (i = 0; i < width; i += 16)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		LoopWidth_CompVImageConvRgb24family_to_y_Asm_NEON32:
			add r9, r9, #16
			pld [rgb24Ptr, #64*3]
			vld3.8 {d5, d6, d7}, [rgb24Ptr : 64]! @ FIXME For RGB32: add alignment
			vld3.8 {d8, d9, d10}, [rgb24Ptr]! @ FIXME For RGB32: add alignment
			vmull.u8 q6, xmmCoeff0, d5
			vmull.u8 q7, xmmCoeff0, d8
			vmlal.u8 q6, xmmCoeff1, d6
			vmlal.u8 q7, xmmCoeff1, d9
			vmlal.u8 q6, xmmCoeff2, d7
			vmlal.u8 q7, xmmCoeff2, d10
			vadd.u16 q8, q6, xmm2048
			vadd.u16 q9, q7, xmm2048
			vqshrn.u16 d5, q8, #7
			vqshrn.u16 d6, q9, #7
			vst1.u8 {d5, d6}, [outYPtr : 128]! @ vstmia r1!, {d5, d6}
			cmp r9, width
			blt LoopWidth_CompVImageConvRgb24family_to_y_Asm_NEON32
			@End_of_LoopWidth

		add outYPtr, outYPtr, padY
		add rgb24Ptr, rgb24Ptr, padRGB
		subs height, height, #1
		bne LoopHeight_CompVImageConvRgb24family_to_y_Asm_NEON32	
		@End_of_LoopHeight

	@undef aliases
	.unreq rgb24Ptr
	.unreq outYPtr
	.unreq width
	.unreq height
	.unreq stride
	.unreq kRGBfamilyToYUV_YCoeffs8
	.unreq padY
	.unreq xmm2048
	.unreq xmmCoeff0
	.unreq xmmCoeff1
	.unreq xmmCoeff2
	
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 6
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* rgb24Ptr
@ arg(1) -> COMPV_ALIGNED(NEON) uint8_t* outUPtr
@ arg(2) -> COMPV_ALIGNED(NEON) uint8_t* outVPtr
@ arg(3) -> compv_uscalar_t width
@ arg(4) -> compv_uscalar_t height
@ arg(5) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
@ arg(6) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_UCoeffs8
@ arg(7) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_VCoeffs8
COMPV_GAS_FUNCTION_DECLARE CompVImageConvRgb24family_to_uv_planar_11_Asm_NEON32
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 8
	COMPV_GAS_SAVE_NEON_REGS

	ldmia_args r0-r7
	rgb24Ptr .req r0
	outUPtr .req r1
	outVPtr .req r2
	width .req r3
	height .req r4
	stride .req r5
	kRGBfamilyToYUV_UCoeffs8 .req r6
	kRGBfamilyToYUV_VCoeffs8 .req r7

	pld [rgb24Ptr]
	pld [rgb24Ptr, #64*1]
	pld [rgb24Ptr, #64*2]
	pld [rgb24Ptr, #64*3]

	add r8, width, #15
	and r8, r8, #-16
	sub r9, stride, r8
	padUV .req r9
	add r8, padUV, padUV, LSL #1
	padRGB .req r8
	
	vld4.s8 {q0low, q0high, q1low, q1high}, [kRGBfamilyToYUV_UCoeffs8 : 128]
	vmovl.s8 q2, q0low
	vmovl.s8 q3, q0high
	vmovl.s8 q4, q1low
	xmmCoeffU0 .req q2
	xmmCoeffU1 .req q3
	xmmCoeffU2 .req q4
	vld4.s8 {q0low, q0high, q1low, q1high}, [kRGBfamilyToYUV_VCoeffs8 : 128]
	vmovl.s8 q5, q0low
	vmovl.s8 q6, q0high
	vmovl.s8 q7, q1low
	xmmCoeffV0 .req q5
	xmmCoeffV1 .req q6
	xmmCoeffV2 .req q7

	ldr r10, =k128_i16
	vld1.u16 {q0low, q0high}, [r10 : 128] @ {q0low, q0high} = q0 = xmm128

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (j = 0; j < height; ++j)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopHeight_CompVImageConvRgb24family_to_uv_planar_11_Asm_NEON32:
		mov r6, #0
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (i = 0; i < width; i += 16)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		LoopWidth_CompVImageConvRgb24family_to_uv_planar_11_Asm_NEON32:
			xmm0 .req q11
			xmm1 .req q12
			add r6, r6, #16
			pld [rgb24Ptr, #64*3]
			vld3.8 {d16, d17, d18}, [rgb24Ptr : 64]! @ FIXME For RGB32: add alignment
			vld3.8 {d19, d20, d21}, [rgb24Ptr]! @ FIXME For RGB24: add alignment
			@ First part(R)
			vmovl.u8 xmm0, d16
			vmovl.u8 xmm1, d19
			vmul.i16 q13, xmmCoeffU0, xmm0
			vmul.s16 q14, xmmCoeffV0, xmm0
			vmovl.u8 xmm0, d17
			vmul.s16 q15, xmmCoeffU0, xmm1
			vmul.s16 q1, xmmCoeffV0, xmm1
			vmovl.u8 xmm1, d20
			@ Second part(G)
			vmla.s16 q13, xmmCoeffU1, xmm0
			vmla.s16 q14, xmmCoeffV1, xmm0
			vmovl.u8 xmm0, d18
			vmla.s16 q15, xmmCoeffU1, xmm1
			vmla.s16 q1, xmmCoeffV1, xmm1
			vmovl.u8 xmm1, d21
			@ Third part(B)
			vmla.s16 q13, xmmCoeffU2, xmm0
			vmla.s16 q14, xmmCoeffV2, xmm0
			vmla.s16 q15, xmmCoeffU2, xmm1
			vmla.s16 q1, xmmCoeffV2, xmm1
			@ >> 8
			vshr.s16 q13, q13, #8
			vshr.s16 q14, q14, #8
			vshr.s16 q15, q15, #8
			vshr.s16 q1, q1, #8
			@ + 128
			vadd.s16 q13, q13, q0
			vadd.s16 q14, q14, q0
			vadd.s16 q15, q15, q0
			vadd.s16 q1, q1, q0
			@ outPtr = concat(saturate(xmm0), saturate(xmm1))
			vqmovun.s16 d22, q13
			vqmovun.s16 d23, q15
			vqmovun.s16 d24, q14
			vqmovun.s16 d25, q1
			cmp r6, width
			vst1.u8 {d22, d23}, [outUPtr : 128]! @ vstmia outUPtr!, {d22, d23}
			vst1.u8 {d24, d25}, [outVPtr : 128]! @ vstmia outVPtr!, {d24, d25}			
			blt LoopWidth_CompVImageConvRgb24family_to_uv_planar_11_Asm_NEON32
			@End_of_LoopWidth

		add rgb24Ptr, rgb24Ptr, padRGB
		add outUPtr, outUPtr, padUV
		add outVPtr, outVPtr, padUV
		subs height, height, #1
		bne LoopHeight_CompVImageConvRgb24family_to_uv_planar_11_Asm_NEON32
		@End_of_LoopHeight

	@undef aliases
	.unreq rgb24Ptr
	.unreq outUPtr
	.unreq outVPtr
	.unreq width
	.unreq height
	.unreq stride
	.unreq kRGBfamilyToYUV_UCoeffs8
	.unreq kRGBfamilyToYUV_VCoeffs8
	.unreq padUV
	.unreq padRGB
	.unreq xmm0
	.unreq xmm1
	.unreq xmmCoeffU0
	.unreq xmmCoeffU1
	.unreq xmmCoeffU2
	.unreq xmmCoeffV0
	.unreq xmmCoeffV1
	.unreq xmmCoeffV2
	
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 8
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


	
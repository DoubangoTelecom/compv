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

.equ familyRGB24, 0
.equ familyRGB32, 1
.equ familyRGB565, 2
.equ isAlphaLastYES, YES @ alpha at the last position and R,G,B at in order: BGRA, RGBA...
.equ isAlphaLastNO, NO @ alpha at the first position and R,G,B at in order: ARGB, ABGR...

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* rgbPtr
@ arg(1) -> COMPV_ALIGNED(NEON) uint8_t* outYPtr
@ arg(2) -> compv_uscalar_t width
@ arg(3) -> compv_uscalar_t height
@ arg(4) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
@ arg(5) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_YCoeffs8
@ When alpha is at the last position then load first 3 coeffs only
.macro CompVImageConvRgbfamily_to_y_Macro_NEON32 family, isAlphaLast
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 6
	COMPV_GAS_SAVE_NEON_REGS
	
	ldmia_args r0-r5
	rgbPtr .req r0
	outYPtr .req r1
	width .req r2
	height .req r3
	stride .req r4
	kRGBfamilyToYUV_YCoeffs8 .req r5

	pld [rgbPtr]
	pld [rgbPtr, #64*1]
	pld [rgbPtr, #64*2]
	pld [rgbPtr, #64*3]

	add r6, width, #15
	and r6, r6, #-16
	sub r7, stride, r6
	padY .req r7
	.if \family == familyRGB24
		add r8, padY, padY, LSL #1
	.elseif \family == familyRGB32
		mov r8, padY, LSL #2
	.elseif
		.error
	.endif
	padRGB .req r8

	mov r9, #2048
	vdup.u16 q0, r9
	vec2048 .req q0

	vld4.8 {d20, d21, d22, d23}, [kRGBfamilyToYUV_YCoeffs8 : 128]
	vecCoeff0 .req d20
	vecCoeff1 .req d21
	vecCoeff2 .req d22
	vecCoeff3 .req d23

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (j = 0; j < height; ++j)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopHeight\@:
		mov r9, #0
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (i = 0; i < width; i += 16)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		LoopWidth\@:
			add r9, r9, #16
			pld [rgbPtr, #64*3]
			.if \family == familyRGB24
				vld3.u8 {d5, d6, d7}, [rgbPtr :64]!
				vld3.u8 {d16, d17, d18}, [rgbPtr]!
			.elseif \family == familyRGB32
				vld4.u8 {d5, d6, d7, d8}, [rgbPtr :128]!
				vld4.u8 {d16, d17, d18, d19}, [rgbPtr :128]!
			.elseif
				.error not implemented
			.endif
			vmull.u8 q6, vecCoeff0, d5
			vmull.u8 q7, vecCoeff0, d16
			vmlal.u8 q6, vecCoeff1, d6
			vmlal.u8 q7, vecCoeff1, d17
			vmlal.u8 q6, vecCoeff2, d7
			vmlal.u8 q7, vecCoeff2, d18
			.if \family == familyRGB32 && \isAlphaLast == isAlphaLastNO
				vmlal.u8 q6, vecCoeff3, d8
				vmlal.u8 q7, vecCoeff3, d19
			.endif
			vadd.u16 q6, q6, vec2048
			vadd.u16 q7, q7, vec2048
			vqshrn.u16 d5, q6, #7
			vqshrn.u16 d6, q7, #7
			vst1.u8 {d5, d6}, [outYPtr :128]! @ vstmia r1!, {d5, d6}
			cmp r9, width
			blt LoopWidth\@
			@End_of_LoopWidth

		add outYPtr, outYPtr, padY
		add rgbPtr, rgbPtr, padRGB
		subs height, height, #1
		bne LoopHeight\@	
		@End_of_LoopHeight

	@undef aliases
	.unreq rgbPtr
	.unreq outYPtr
	.unreq width
	.unreq height
	.unreq stride
	.unreq kRGBfamilyToYUV_YCoeffs8
	.unreq padY
	.unreq vec2048
	.unreq vecCoeff0
	.unreq vecCoeff1
	.unreq vecCoeff2
	.unreq vecCoeff3
	
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 6
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm
  
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVImageConvRgb24family_to_y_Asm_NEON32
    CompVImageConvRgbfamily_to_y_Macro_NEON32 familyRGB24, isAlphaLastNO

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVImageConvArgb32family_to_y_Asm_NEON32
    CompVImageConvRgbfamily_to_y_Macro_NEON32 familyRGB32, isAlphaLastNO

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVImageConvRgba32family_to_y_Asm_NEON32
    CompVImageConvRgbfamily_to_y_Macro_NEON32 familyRGB32, isAlphaLastYES


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* rgbPtr
@ arg(1) -> COMPV_ALIGNED(NEON) uint8_t* outUPtr
@ arg(2) -> COMPV_ALIGNED(NEON) uint8_t* outVPtr
@ arg(3) -> compv_uscalar_t width
@ arg(4) -> compv_uscalar_t height
@ arg(5) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
@ arg(6) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_UCoeffs8
@ arg(7) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_VCoeffs8
.macro CompVImageConvRgbfamily_to_uv_planar_11_Macro_NEON32 family, isAlphaLast
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 8
	COMPV_GAS_SAVE_NEON_REGS

	ldmia_args r0-r7
	rgbPtr .req r0
	outUPtr .req r1
	outVPtr .req r2
	width .req r3
	height .req r4
	stride .req r5
	kRGBfamilyToYUV_UCoeffs8 .req r6
	kRGBfamilyToYUV_VCoeffs8 .req r7

	pld [rgbPtr]
	pld [rgbPtr, #64*1]
	pld [rgbPtr, #64*2]
	pld [rgbPtr, #64*3]

	add r8, width, #15
	and r8, r8, #-16
	sub r9, stride, r8
	padUV .req r9
	.if \family == familyRGB24
		add r8, padUV, padUV, LSL #1
	.elseif \family == familyRGB32
		mov r8, padUV, LSL #2
	.elseif
		.error
	.endif
	padRGB .req r8
	
	vld4.s8 {q10x, q10y, q11x, q11y}, [kRGBfamilyToYUV_UCoeffs8 : 128]
	vmovl.s8 q0, q10x
	vmovl.s8 q1, q10y
	vmovl.s8 q2, q11x
	.if \family == familyRGB32 && \isAlphaLast == isAlphaLastNO
		vmovl.s8 q3, q11y
	.endif
	vecCoeffU0 .req q0
	vecCoeffU1 .req q1
	vecCoeffU2 .req q2
	vecCoeffU3 .req q3
	
	vld4.s8 {q10x, q10y, q11x, q11y}, [kRGBfamilyToYUV_VCoeffs8 : 128]
	vmovl.s8 q4, q10x
	vmovl.s8 q5, q10y
	vmovl.s8 q6, q11x
	.if \family == familyRGB32 && \isAlphaLast == isAlphaLastNO
		vmovl.s8 q7, q11y
	.endif
	vecCoeffV0 .req q4
	vecCoeffV1 .req q5
	vecCoeffV2 .req q6
	vecCoeffV3 .req q7

	ldr r10, =k128_i16
	vld1.u16 {q15x, q15y}, [r10 :128]
	vec128 .req q15

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (j = 0; j < height; ++j)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopHeight\@:
		mov r6, #0
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (i = 0; i < width; i += 16)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		LoopWidth\@:
			vec0 .req q12
			add r6, r6, #16
			pld [rgbPtr, #64*3]
			.if \family == familyRGB24
				vld3.u8 {q8x, q8y, q9x}, [rgbPtr :64]!
				vld3.u8 {q10x, q10y, q11x}, [rgbPtr]!
			.elseif \family == familyRGB32
				vld4.u8 {q8x, q8y, q9x, q9y}, [rgbPtr :128]!
				vld4.u8 {q10x, q10y, q11x, q11y}, [rgbPtr :128]!
			.elseif
				.error not implemented
			.endif
			vmovl.u8 vec0, q8x
			vmul.i16 q13, vecCoeffU0, vec0
			vmul.s16 q14, vecCoeffV0, vec0
			vmovl.u8 vec0, q8y
			vmla.s16 q13, vecCoeffU1, vec0
			vmla.s16 q14, vecCoeffV1, vec0
			vmovl.u8 vec0, q9x
			vmla.s16 q13, vecCoeffU2, vec0
			vmla.s16 q14, vecCoeffV2, vec0
			.if \family == familyRGB32 && \isAlphaLast == isAlphaLastNO
				vmovl.u8 vec0, q9y
				vmla.s16 q13, vecCoeffU3, vec0
				vmla.s16 q14, vecCoeffV3, vec0
			.endif
			vmovl.u8 vec0, q10x
			vmul.s16 q8, vecCoeffU0, vec0
			vmul.s16 q9, vecCoeffV0, vec0
			vmovl.u8 vec0, q10y
			vmla.s16 q8, vecCoeffU1, vec0
			vmla.s16 q9, vecCoeffV1, vec0
			vmovl.u8 vec0, q11x
			vmla.s16 q8, vecCoeffU2, vec0
			vmla.s16 q9, vecCoeffV2, vec0
			.if \family == familyRGB32 && \isAlphaLast == isAlphaLastNO
				vmovl.u8 vec0, q11y
				vmla.s16 q8, vecCoeffU3, vec0
				vmla.s16 q9, vecCoeffV3, vec0
			.endif
			vshr.s16 q13, q13, #8
			vshr.s16 q14, q14, #8
			vshr.s16 q8, q8, #8
			vshr.s16 q9, q9, #8
			vadd.s16 q13, q13, vec128
			vadd.s16 q14, q14, vec128
			vadd.s16 q8, q8, vec128
			vadd.s16 q9, q9, vec128
			vqmovun.s16 q11x, q13
			vqmovun.s16 q11y, q8
			vqmovun.s16 q12x, q14
			vqmovun.s16 q12y, q9
			vst1.u8 {q11x, q11y}, [outUPtr :128]! @ vstmia outUPtr!, {q11x, q11y}
			vst1.u8 {q12x, q12y}, [outVPtr :128]! @ vstmia outVPtr!, {q12x, q12y}
			cmp r6, width		
			blt LoopWidth\@
			@End_of_LoopWidth

		add rgbPtr, rgbPtr, padRGB
		add outUPtr, outUPtr, padUV
		add outVPtr, outVPtr, padUV
		subs height, height, #1
		bne LoopHeight\@
		@End_of_LoopHeight

	@undef aliases
	.unreq rgbPtr
	.unreq outUPtr
	.unreq outVPtr
	.unreq width
	.unreq height
	.unreq stride
	.unreq kRGBfamilyToYUV_UCoeffs8
	.unreq kRGBfamilyToYUV_VCoeffs8
	.unreq padUV
	.unreq padRGB
	.unreq vec0
	.unreq vecCoeffU0
	.unreq vecCoeffU1
	.unreq vecCoeffU2
	.unreq vecCoeffU3
	.unreq vecCoeffV0
	.unreq vecCoeffV1
	.unreq vecCoeffV2
	.unreq vecCoeffV3
	.unreq vec128
	
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 8
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVImageConvRgb24family_to_uv_planar_11_Asm_NEON32
	CompVImageConvRgbfamily_to_uv_planar_11_Macro_NEON32 familyRGB24, isAlphaLastNO

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVImageConvArgb32family_to_uv_planar_11_Asm_NEON32 
	CompVImageConvRgbfamily_to_uv_planar_11_Macro_NEON32 familyRGB32, isAlphaLastNO

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVImageConvRgba32family_to_uv_planar_11_Asm_NEON32
	CompVImageConvRgbfamily_to_uv_planar_11_Macro_NEON32 familyRGB32, isAlphaLastYES

	
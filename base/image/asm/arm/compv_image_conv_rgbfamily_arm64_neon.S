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

.extern sym(k16_i16), sym(k128_i16)
 
.text

.equ familyRGB24, 0
.equ familyRGB32, 1
.equ familyRGB565, 2
.equ endiannessBIG, 3
.equ endiannessLITTLE, 4
.equ isAlphaLastYES, YES // alpha at the last position and R,G,B at in order: BGRA, RGBA...
.equ isAlphaLastNO, NO // alpha at the first position and R,G,B at in order: ARGB, ABGR...

#############################################################################
# arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* rgbPtr
# arg(1) -> COMPV_ALIGNED(NEON) uint8_t* outYPtr
# arg(2) -> compv_uscalar_t width
# arg(3) -> compv_uscalar_t height
# arg(4) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
# arg(5) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_YCoeffs8
# When alpha is at the last position then load first 3 coeffs only
.macro CompVImageConvRgbfamily_to_y_Macro_NEON64 family, isAlphaLast
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 6
	COMPV_GAS_SAVE_NEON_REGS
	
	ldp_arg 0, r0, r1
    ldp_arg 2, r2, r3
    ldp_arg 4, r4, r5
	rgbPtr .req r0
	outYPtr .req r1
	width .req r2
	height .req r3
	stride .req r4
	kRGBfamilyToYUV_YCoeffs8 .req r5

	prfm pldl1keep, [rgbPtr, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [rgbPtr, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [rgbPtr, #(CACHE_LINE_SIZE*2)]

	add r6, width, #15
	and r6, r6, #-16
	sub r7, stride, r6
	padY .req r7
	.if \family == familyRGB24
		add r8, padY, padY, LSL #1
	.elseif \family == familyRGB32
		lsl r8, padY, #2
	.elseif
		.error Not implemented
	.endif
	padRGB .req r8

	mov r9, #2048
	dup v0.8h, r9w
	#define vec2048 v0

	ld4.8b {v12, v13, v14, v15}, [kRGBfamilyToYUV_YCoeffs8]
    dup v12.2d, v12.d[0]
    dup v13.2d, v13.d[0]
    dup v14.2d, v14.d[0]
    dup v15.2d, v15.d[0]
	#define vecCoeff0 v12
	#define vecCoeff1 v13
	#define vecCoeff2 v14
	#define vecCoeff3 v15

	########################################
	# for (j = 0; j < height; ++j)
	########################################
	LoopHeight\@:
		mov r9, #0
		########################################
		# for (i = 0; i < width; i += 16)
		########################################
		LoopWidth\@:
			add r9, r9, #16
			prfm pldl1keep, [rgbPtr, #(CACHE_LINE_SIZE*3)]
            prfm pstl1keep, [outYPtr, #(CACHE_LINE_SIZE*3)]
			.if \family == familyRGB24
				ld3.16b {v2, v3, v4}, [rgbPtr], #16*3
			.elseif \family == familyRGB32
				ld4.16b {v2, v3, v4, v5}, [rgbPtr], #16*4
			.elseif
				.error not implemented
			.endif
            
			umull v6.8h, vecCoeff0.8b, v2.8b
            umull2 v7.8h, vecCoeff0.16b, v2.16b
            umlal v6.8h, vecCoeff1.8b, v3.8b
			umlal2 v7.8h, vecCoeff1.16b, v3.16b
            umlal v6.8h, vecCoeff2.8b, v4.8b
			umlal2 v7.8h, vecCoeff2.16b, v4.16b
			.if \family == familyRGB32 && \isAlphaLast == isAlphaLastNO
				umlal v6.8h, vecCoeff3.8b, v5.8b
			    umlal2 v7.8h, vecCoeff3.16b, v5.16b
			.endif
			add v6.8h, v6.8h, vec2048.8h
			add v7.8h, v7.8h, vec2048.8h
			uqshrn v6.8b, v6.8h, #7
			uqshrn2 v6.16b, v7.8h, #7
			st1 {v6.16b}, [outYPtr], #16
			cmp r9, width
			blt LoopWidth\@
			#End_of_LoopWidth#

		add outYPtr, outYPtr, padY
		add rgbPtr, rgbPtr, padRGB
		subs height, height, #1
		bne LoopHeight\@	
		#End_of_LoopHeight#

	//undef aliases
	.unreq rgbPtr
	.unreq outYPtr
	.unreq width
	.unreq height
	.unreq stride
	.unreq kRGBfamilyToYUV_YCoeffs8
	.unreq padY
	#undef vec2048
	#undef vecCoeff0
	#undef vecCoeff1
	#undef vecCoeff2
	#undef vecCoeff3
	
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm
  
#############################################################################
COMPV_GAS_FUNCTION_DECLARE CompVImageConvRgb24family_to_y_Asm_NEON64
    CompVImageConvRgbfamily_to_y_Macro_NEON64 familyRGB24, isAlphaLastNO

#############################################################################
COMPV_GAS_FUNCTION_DECLARE CompVImageConvArgb32family_to_y_Asm_NEON64
    CompVImageConvRgbfamily_to_y_Macro_NEON64 familyRGB32, isAlphaLastNO

#############################################################################
COMPV_GAS_FUNCTION_DECLARE CompVImageConvRgba32family_to_y_Asm_NEON64
    CompVImageConvRgbfamily_to_y_Macro_NEON64 familyRGB32, isAlphaLastYES


#############################################################################
# arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* rgbPtr
# arg(1) -> COMPV_ALIGNED(NEON) uint8_t* outYPtr
# arg(2) -> compv_uscalar_t width
# arg(3) -> compv_uscalar_t height
# arg(4) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
# arg(5) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_YCoeffs8
.macro CompVImageConvRgb565family_to_y_Macro_NEON64 endianness
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 6
	COMPV_GAS_SAVE_NEON_REGS
	
	ldp_arg 0, r0, r1
    ldp_arg 2, r2, r3
    ldp_arg 4, r4, r5
	rgbPtr .req r0
	outYPtr .req r1
	width .req r2
	height .req r3
	stride .req r4
	kRGBfamilyToYUV_YCoeffs8 .req r5

	prfm pldl1keep, [rgbPtr, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [rgbPtr, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [rgbPtr, #(CACHE_LINE_SIZE*2)]

	add r6, width, #15
	and r6, r6, #-16
	sub r7, stride, r6
	padY .req r7
	lsl r8, padY, #1
	padRGB .req r8

	mov r9, #2048
	dup.8h v12, r9w
	#define vec2048 v12
	mov r9, #0xF800
	dup.8h v11, r9w
	#define vecMaskR v11
	mov r9, #0x07E0
	dup.8h v10, r9w
	#define vecMaskG v10
	mov r9, #0x001F
	dup.8h v9, r9w
	#define vecMaskB v9

	ld4.8b {v0, v1, v2, v3}, [kRGBfamilyToYUV_YCoeffs8]
	uxtl v13.8h, v0.8b
	uxtl v14.8h, v1.8b
	uxtl v15.8h, v2.8b
	#define vecCoeff0 v13
	#define vecCoeff1 v14
	#define vecCoeff2 v15

	########################################
	# for (j = 0; j < height; ++j)
	########################################
	LoopHeight\@:
		mov r9, #0
		########################################
		# for (i = 0; i < width; i += 16)
		########################################
		LoopWidth\@:
			add r9, r9, #16
			prfm pldl1keep, [rgbPtr, #(CACHE_LINE_SIZE*3)]
            prfm pstl1keep, [outYPtr, #(CACHE_LINE_SIZE*3)]
			ld1 {v0.16b, v1.16b}, [rgbPtr], #16*2
			#define vec0 v0
			#define vec1 v1
			.if \endianness == endiannessBIG
				rev16 vec0.16b, vec0.16b
				rev16 vec1.16b, vec1.16b
			.endif
			and v2.16b, vec0.16b, vecMaskR.16b
			and v3.16b, vec1.16b, vecMaskR.16b
			and v4.16b, vec0.16b, vecMaskG.16b
			and v5.16b, vec1.16b, vecMaskG.16b
			and v6.16b, vec0.16b, vecMaskB.16b
			and v7.16b, vec1.16b, vecMaskB.16b
			ushr v2.8h, v2.8h, #8
			ushr v3.8h, v3.8h, #8
			ushr v4.8h, v4.8h, #3
			ushr v5.8h, v5.8h, #3
			shl v6.8h, v6.8h, #3
			shl v7.8h, v7.8h, #3
			usra v2.8h, v2.8h, #5
			usra v3.8h, v3.8h, #5
			usra v4.8h, v4.8h, #6
			usra v5.8h, v5.8h, #6
			usra v6.8h, v6.8h, #5
			usra v7.8h, v7.8h, #5
			mul v0.8h, vecCoeff0.8h, v2.8h
			mul v1.8h, vecCoeff0.8h, v3.8h
			mla v0.8h, vecCoeff1.8h, v4.8h
			mla v1.8h, vecCoeff1.8h, v5.8h
			mla v0.8h, vecCoeff2.8h, v6.8h
			mla v1.8h, vecCoeff2.8h, v7.8h
			add v0.8h, v0.8h, vec2048.8h
			add v1.8h, v1.8h, vec2048.8h
			uqshrn v2.8b, v0.8h, #7
			uqshrn2 v2.16b, v1.8h, #7
			st1 {v2.16b}, [outYPtr], #16
			cmp r9, width
			blt LoopWidth\@
			#End_of_LoopWidth#

		add outYPtr, outYPtr, padY
		add rgbPtr, rgbPtr, padRGB
		subs height, height, #1
		bne LoopHeight\@
		#End_of_LoopHeight#

	//undef aliases
	.unreq rgbPtr
	.unreq outYPtr
	.unreq width
	.unreq height
	.unreq stride
	.unreq kRGBfamilyToYUV_YCoeffs8
	.unreq padY
	#undef vec0
	#undef vec1
	#undef vec2048
	#undef vecCoeff0
	#undef vecCoeff1
	#undef vecCoeff2
	#undef vecMaskR
	#undef vecMaskG
	#undef vecMaskB
	
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 6
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

#############################################################################
COMPV_GAS_FUNCTION_DECLARE CompVImageConvRgb565lefamily_to_y_Asm_NEON64
	CompVImageConvRgb565family_to_y_Macro_NEON64 endiannessLITTLE

#############################################################################
COMPV_GAS_FUNCTION_DECLARE CompVImageConvRgb565befamily_to_y_Asm_NEON64
	CompVImageConvRgb565family_to_y_Macro_NEON64 endiannessBIG

#############################################################################
# arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* rgbPtr
# arg(1) -> COMPV_ALIGNED(NEON) uint8_t* outUPtr
# arg(2) -> COMPV_ALIGNED(NEON) uint8_t* outVPtr
# arg(3) -> compv_uscalar_t width
# arg(4) -> compv_uscalar_t height
# arg(5) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
# arg(6) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_UCoeffs8
# arg(7) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_VCoeffs8
.macro CompVImageConvRgbfamily_to_uv_planar_11_Macro_NEON64 family, isAlphaLast
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 8
	COMPV_GAS_SAVE_NEON_REGS

	ldm_args r0-r7
	rgbPtr .req r0
	outUPtr .req r1
	outVPtr .req r2
	width .req r3
	height .req r4
	stride .req r5
	kRGBfamilyToYUV_UCoeffs8 .req r6
	kRGBfamilyToYUV_VCoeffs8 .req r7

	pld [rgbPtr, #(CACHE_LINE_SIZE*0)]
	pld [rgbPtr, #(CACHE_LINE_SIZE*1)]
	pld [rgbPtr, #(CACHE_LINE_SIZE*2)]

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

	ldr r10, =sym(k128_i16)
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
			pld [rgbPtr, #(CACHE_LINE_SIZE*3)]
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

#############################################################################
COMPV_GAS_FUNCTION_DECLARE CompVImageConvRgb24family_to_uv_planar_11_Asm_NEON64
	CompVImageConvRgbfamily_to_uv_planar_11_Macro_NEON64 familyRGB24, isAlphaLastNO

#############################################################################
COMPV_GAS_FUNCTION_DECLARE CompVImageConvArgb32family_to_uv_planar_11_Asm_NEON64 
	CompVImageConvRgbfamily_to_uv_planar_11_Macro_NEON64 familyRGB32, isAlphaLastNO

#############################################################################
COMPV_GAS_FUNCTION_DECLARE CompVImageConvRgba32family_to_uv_planar_11_Asm_NEON64
	CompVImageConvRgbfamily_to_uv_planar_11_Macro_NEON64 familyRGB32, isAlphaLastYES

#endif /* defined(__aarch64__) */

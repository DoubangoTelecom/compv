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

	ld4 {v12.8b, v13.8b, v14.8b, v15.8b}, [kRGBfamilyToYUV_YCoeffs8]
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
				ld3 {v2.16b, v3.16b, v4.16b}, [rgbPtr], #16*3
			.elseif \family == familyRGB32
				ld4 {v2.16b, v3.16b, v4.16b, v5.16b}, [rgbPtr], #16*4
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
	dup v12.8h, r9w
	#define vec2048 v12
	mov r9, #0xF800
	dup v11.8h, r9w
	#define vecMaskR v11
	mov r9, #0x07E0
	dup v10.8h, r9w
	#define vecMaskG v10
	mov r9, #0x001F
	dup v9.8h, r9w
	#define vecMaskB v9

	ld4 {v0.8b, v1.8b, v2.8b, v3.8b}, [kRGBfamilyToYUV_YCoeffs8]
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
	COMPV_GAS_UNSHADOW_ARGS
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

    ldp_arg 0, r0, r1
    ldp_arg 2, r2, r3
    ldp_arg 4, r4, r5
    ldp_arg 6, r6, r7
	rgbPtr .req r0
	outUPtr .req r1
	outVPtr .req r2
	width .req r3
	height .req r4
	stride .req r5
	kRGBfamilyToYUV_UCoeffs8 .req r6
	kRGBfamilyToYUV_VCoeffs8 .req r7

	prfm pldl1keep, [rgbPtr, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [rgbPtr, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [rgbPtr, #(CACHE_LINE_SIZE*2)]

	add r8, width, #15
	and r8, r8, #-16
	sub r9, stride, r8
	padUV .req r9
	.if \family == familyRGB24
		add r8, padUV, padUV, LSL #1
	.elseif \family == familyRGB32
		lsl r8, padUV, #2
	.elseif
		.error
	.endif
	padRGB .req r8
	
	ld4 {v10.8b, v11.8b, v12.8b, v13.8b}, [kRGBfamilyToYUV_UCoeffs8]
    sxtl v0.8h, v10.8b
	sxtl v1.8h, v11.8b
	sxtl v2.8h, v12.8b
	.if \family == familyRGB32 && \isAlphaLast == isAlphaLastNO
		sxtl v3.8h, v13.8b
	.endif
	#define vecCoeffU0 v0
	#define vecCoeffU1 v1
	#define vecCoeffU2 v2
	#define vecCoeffU3 v3
	
	ld4 {v10.8b, v11.8b, v12.8b, v13.8b}, [kRGBfamilyToYUV_VCoeffs8]
	sxtl v4.8h, v10.8b
	sxtl v5.8h, v11.8b
	sxtl v6.8h, v12.8b
	.if \family == familyRGB32 && \isAlphaLast == isAlphaLastNO
		sxtl v7.8h, v13.8b
	.endif
	#define vecCoeffV0 v4
	#define vecCoeffV1 v5
	#define vecCoeffV2 v6
	#define vecCoeffV3 v7

    mov r10, #128
	dup v30.8h, r10w
	#define vec128 v30

	########################################
	# for (j = 0; j < height; ++j)
	########################################
	LoopHeight\@:
		mov r6, #0
		########################################
		# for (i = 0; i < width; i += 16)
		########################################
		LoopWidth\@:
			add r6, r6, #16
			prfm pldl1keep, [rgbPtr, #(CACHE_LINE_SIZE*3)]
            prfm pstl1keep, [outUPtr, #(CACHE_LINE_SIZE*3)]
            prfm pstl1keep, [outVPtr, #(CACHE_LINE_SIZE*3)]
			.if \family == familyRGB24
				ld3 {v8.16b, v9.16b, v10.16b}, [rgbPtr], #3*16
			.elseif \family == familyRGB32
				ld4 {v8.16b, v9.16b, v10.16b, v11.16b}, [rgbPtr], #4*16
			.elseif
				.error not implemented
			.endif

            uxtl v16.8h, v8.8b
            uxtl v17.8h, v9.8b
            uxtl v18.8h, v10.8b
            .if \family == familyRGB32 && \isAlphaLast == isAlphaLastNO
				uxtl v19.8h, v11.8b
			.endif
            uxtl2 v20.8h, v8.16b
            uxtl2 v21.8h, v9.16b
            uxtl2 v22.8h, v10.16b
            .if \family == familyRGB32 && \isAlphaLast == isAlphaLastNO
				uxtl2 v23.8h, v11.16b
			.endif

			mul v13.8h, vecCoeffU0.8h, v16.8h
			mul v14.8h, vecCoeffV0.8h, v16.8h
            mul v8.8h, vecCoeffU0.8h, v20.8h
			mul v15.8h, vecCoeffV0.8h, v20.8h
			mla v13.8h, vecCoeffU1.8h, v17.8h
			mla v14.8h, vecCoeffV1.8h, v17.8h
            mla v8.8h, vecCoeffU1.8h, v21.8h
			mla v15.8h, vecCoeffV1.8h, v21.8h
			mla v13.8h, vecCoeffU2.8h, v18.8h
			mla v14.8h, vecCoeffV2.8h, v18.8h
            mla v8.8h, vecCoeffU2.8h, v22.8h
			mla v15.8h, vecCoeffV2.8h, v22.8h
			.if \family == familyRGB32 && \isAlphaLast == isAlphaLastNO
				mla v13.8h, vecCoeffU3.8h, v19.8h
				mla v14.8h, vecCoeffV3.8h, v19.8h
                mla v8.8h, vecCoeffU3.8h, v23.8h
				mla v15.8h, vecCoeffV3.8h, v23.8h
			.endif

			sshr v13.8h, v13.8h, #8
			sshr v14.8h, v14.8h, #8
			sshr v8.8h, v8.8h, #8
			sshr v15.8h, v15.8h, #8
			add v13.8h, v13.8h, vec128.8h
			add v14.8h, v14.8h, vec128.8h
			add v8.8h, v8.8h, vec128.8h
			add v15.8h, v15.8h, vec128.8h
			sqxtun v16.8b, v13.8h
            sqxtun v17.8b, v14.8h
			sqxtun2 v16.16b, v8.8h
			sqxtun2 v17.16b, v15.8h
			st1 {v16.16b}, [outUPtr], #16
			st1 {v17.16b}, [outVPtr], #16
			cmp r6, width		
			blt LoopWidth\@
			#End_of_LoopWidth#

		add rgbPtr, rgbPtr, padRGB
		add outUPtr, outUPtr, padUV
		add outVPtr, outVPtr, padUV
		subs height, height, #1
		bne LoopHeight\@
		#End_of_LoopHeight#

	//undef aliases
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
	#undef vecCoeffU0
	#undef vecCoeffU1
	#undef vecCoeffU2
	#undef vecCoeffU3
	#undef vecCoeffV0
	#undef vecCoeffV1
	#undef vecCoeffV2
	#undef vecCoeffV3
	#undef vec128
	
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS
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


#############################################################################
# arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* rgbPtr
# arg(1) -> COMPV_ALIGNED(NEON) uint8_t* outUPtr
# arg(2) -> COMPV_ALIGNED(NEON) uint8_t* outVPtr
# arg(3) -> compv_uscalar_t width
# arg(4) -> compv_uscalar_t height
# arg(5) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
# arg(6) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_UCoeffs8
# arg(7) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_VCoeffs8
.macro CompVImageConvRgb565family_to_uv_planar_11_Macro_NEON64 endianness
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 8
	COMPV_GAS_SAVE_NEON_REGS

	ldp_arg 0, r0, r1
    ldp_arg 2, r2, r3
    ldp_arg 4, r4, r5
    ldp_arg 6, r6, r7
	rgbPtr .req r0
	outUPtr .req r1
	outVPtr .req r2
	width .req r3
	height .req r4
	stride .req r5
	kRGBfamilyToYUV_UCoeffs8 .req r6
	kRGBfamilyToYUV_VCoeffs8 .req r7

	prfm pldl1keep, [rgbPtr, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [rgbPtr, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [rgbPtr, #(CACHE_LINE_SIZE*2)]

	add r8, width, #15
	and r8, r8, #-16
	sub r9, stride, r8
	padUV .req r9
	lsl r8, padUV, #1
	padRGB .req r8
	
	ld4 {v10.8b, v11.8b, v12.8b, v13.8b}, [kRGBfamilyToYUV_UCoeffs8]
	sxtl v0.8h, v10.8b
	sxtl v1.8h, v11.8b
	sxtl v2.8h, v12.8b
	#define vecCoeffU0 v0
	#define vecCoeffU1 v1
	#define vecCoeffU2 v2
	
	ld4 {v10.8b, v11.8b, v12.8b, v13.8b}, [kRGBfamilyToYUV_VCoeffs8]
	sxtl v3.8h, v10.8b
	sxtl v4.8h, v11.8b
	sxtl v5.8h, v12.8b
	#define vecCoeffV0 v3
	#define vecCoeffV1 v4
	#define vecCoeffV2 v5

	mov r10, #128
	dup v6.8h, r10w
	#define vec128 v6

	########################################
	# for (j = 0; j < height; ++j)
	########################################
	LoopHeight\@:
		mov r6, #0
		########################################
		# for (i = 0; i < width; i += 16)
		########################################
		LoopWidth\@:
			add r6, r6, #16
			prfm pldl1keep, [rgbPtr, #(CACHE_LINE_SIZE*3)]
            prfm pstl1keep, [outUPtr, #(CACHE_LINE_SIZE*3)]
            prfm pstl1keep, [outVPtr, #(CACHE_LINE_SIZE*3)]
			ld1 {v7.16b, v8.16b}, [rgbPtr], #2*16
			#define vec0 v7
			#define vec1 v8
			.if \endianness == endiannessBIG
				rev16 vec0.16b, vec0.16b
				rev16 vec1.16b, vec1.16b
			.endif
			ushr v9.8h, vec0.8h, #11
			ushr v10.8h, vec1.8h, #11
			shl v11.8h, vec0.8h, #5
			shl v12.8h, vec1.8h, #5
			shl v13.8h, vec0.8h, #11
			shl v14.8h, vec1.8h, #11
			shl v9.8h, v9.8h, #3
			shl v10.8h, v10.8h, #3
			ushr v11.8h, v11.8h, #10
			ushr v12.8h, v12.8h, #10
			ushr v13.8h, v13.8h, #8
			ushr v14.8h, v14.8h, #8
			shl v11.8h, v11.8h, #2
			shl v12.8h, v12.8h, #2
			usra v9.8h, v9.8h, #5
			usra v10.8h, v10.8h, #5
			usra v11.8h, v11.8h, #6
			usra v12.8h, v12.8h, #6
			usra v13.8h, v13.8h, #5
			usra v14.8h, v14.8h, #5
			mul v7.8h, vecCoeffU0.8h, v9.8h
			mul v8.8h, vecCoeffV0.8h, v9.8h
			mul v9.8h, vecCoeffU0.8h, v10.8h
			mul v10.8h, vecCoeffV0.8h, v10.8h
			mla v7.8h, vecCoeffU1.8h, v11.8h
			mla v8.8h, vecCoeffV1.8h, v11.8h
			mla v9.8h, vecCoeffU1.8h, v12.8h
			mla v10.8h, vecCoeffV1.8h, v12.8h
			mla v7.8h, vecCoeffU2.8h, v13.8h
			mla v8.8h, vecCoeffV2.8h, v13.8h
			mla v9.8h, vecCoeffU2.8h, v14.8h
			mla v10.8h, vecCoeffV2.8h, v14.8h
			sshr v7.8h, v7.8h, #8
			sshr v8.8h, v8.8h, #8
			sshr v9.8h, v9.8h, #8
			sshr v10.8h, v10.8h, #8
			add v7.8h, v7.8h, vec128.8h
			add v8.8h, v8.8h, vec128.8h
			add v9.8h, v9.8h, vec128.8h
			add v10.8h, v10.8h, vec128.8h
			sqxtun v11.8b, v7.8h
            sqxtun v12.8b, v8.8h
			sqxtun2 v11.16b, v9.8h
			sqxtun2 v12.16b, v10.8h
			st1 {v11.16b}, [outUPtr], #16
			st1 {v12.16b}, [outVPtr], #16
			cmp r6, width		
			blt LoopWidth\@
			#End_of_LoopWidth#

		add rgbPtr, rgbPtr, padRGB
		add outUPtr, outUPtr, padUV
		add outVPtr, outVPtr, padUV
		subs height, height, #1
		bne LoopHeight\@
		#End_of_LoopHeight#

	//undef aliases
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
	#undef vec0
	#undef vec1
	#undef vecCoeffU0
	#undef vecCoeffU1
	#undef vecCoeffU2
	#undef vecCoeffV0
	#undef vecCoeffV1
	#undef vecCoeffV2
	#undef vec128
	
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm


#############################################################################
COMPV_GAS_FUNCTION_DECLARE CompVImageConvRgb565lefamily_to_uv_planar_11_Asm_NEON64
	CompVImageConvRgb565family_to_uv_planar_11_Macro_NEON64 endiannessLITTLE

#############################################################################
COMPV_GAS_FUNCTION_DECLARE CompVImageConvRgb565befamily_to_uv_planar_11_Asm_NEON64
	CompVImageConvRgb565family_to_uv_planar_11_Macro_NEON64 endiannessBIG

#endif /* defined(__aarch64__) */

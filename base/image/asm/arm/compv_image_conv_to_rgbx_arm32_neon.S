#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__arm__) && !defined(__aarch64__)
.include "compv_common_arm32.S" @

.data

.text

.equ rgb24Family,	0
.equ rgba32Family,	1

.equ yuv420pFamily,	2
.equ yuv422pFamily,	3
.equ yuv444pFamily,	4   @ there is intrin implementation but no asm (for now useless)
.equ nv12Family,	5
.equ nv21Family,	6
.equ yuyv422Family,	7
.equ uyvy422Family,	8

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* yPtr
@ arg(x) -> COMPV_ALIGNED(NEON) const uint8_t* uPtr
@ arg(x) -> COMPV_ALIGNED(NEON) const uint8_t* vPtr
@ arg(x) -> COMPV_ALIGNED(NEON) uint8_t* rgbPtr
@ arg(x) -> compv_uscalar_t width
@ arg(x) -> compv_uscalar_t height
@ arg(x) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
.macro CompVImageConvYuv_to_Rgbx_Macro_NEON32 rgbxFamily, yuvFamily
    COMPV_GAS_FUNCTION_PROLOG
    .if \yuvFamily == yuv420pFamily || \yuvFamily == yuv422pFamily
		COMPV_GAS_SHADOW_ARGS_TO_STACK 7
	.elseif \yuvFamily == nv12Family || \yuvFamily == nv21Family
		COMPV_GAS_SHADOW_ARGS_TO_STACK 6
	.elseif \yuvFamily == yuyv422Family || \yuvFamily == uyvy422Family
		COMPV_GAS_SHADOW_ARGS_TO_STACK 5
	.else
		.error "Not implemented"
	.endif
	COMPV_GAS_SAVE_NEON_REGS

	@ Load arguments @
    .if \yuvFamily == yuv420pFamily || \yuvFamily == yuv422pFamily
        ldm_args r0-r6
	    yPtr .req r0
		uPtr .req r1
	    vPtr .req r2
        rgbPtr .req r3
        width .req r4
        height .req r5
        stride .req r6
        i .req r7
        padY .req r8
        padRGBx .req r9
        j .req r10
        padUV .req r11
	.elseif \yuvFamily == nv12Family || \yuvFamily == nv21Family
        ldm_args r0-r5
	    yPtr .req r0
		uvPtr .req r1
        rgbPtr .req r2
        width .req r3
        height .req r4
        stride .req r5
        i .req r6
        padY .req r7
        padRGBx .req r8
        j .req r9
        padUV .req r10
	.elseif \yuvFamily == yuyv422Family || \yuvFamily == uyvy422Family
        ldm_args r0-r4
	    yPtr .req r0
		rgbPtr .req r1
        width .req r2
        height .req r3
        stride .req r4
        i .req r5
        padY .req r6
        padRGBx .req r7
        j .req r8
        padUV .req r9
	.else
		.error "Not implemented"
	.endif

    .if \yuvFamily == uyvy422Family
        vecYlow .req q1
        vecYlowx .req q1x
        vecYlowy .req q1y
        vecYhigh .req q0
        vecYhighx .req q0x
        vecYhighy .req q0y
    .else
        vecYlow .req q0
        vecYlowx .req q0x
        vecYlowy .req q0y
        vecYhigh .req q1
        vecYhighx .req q1x
        vecYhighy .req q1y
    .endif
    .if \yuvFamily == nv21Family
        vecV .req q2
        vecVn .req q2x
        vecU .req q3
        vecUn .req q3x
    .else
        vecU .req q2
        vecUn .req q2x
        vecV .req q3
        vecVn .req q3x
    .endif
    vec16n .req q4x
    vec127n .req q4y
    vec37 .req q5
    vec51 .req q6
    vec65 .req q7
    vec13 .req q8
    vec26 .req q9
    vec0 .req q10
    vec1 .req q11
    vecR .req q12
    vecRx .req q12x
    vecRy .req q12y
    vecG .req q13
    vecGx .req q13x
    vecGy .req q13y
    vecB .req q14
    vecBx .req q14x
    vecBy .req q14y
    vecA .req q15
    vecAx .req q15x
    vecAy .req q15y

    mov i, #16
    mov j, #127
    mov padY, #37
    mov padUV, #51
    mov padRGBx, #65
    vdup.u8 vec16n, i
    vdup.u8 vec127n, j
    vdup.s16 vec37, padY
    vdup.s16 vec51, padUV
    vdup.s16 vec65, padRGBx
    mov i, #13
    mov j, #26
    vdup.s16 vec13, i
    vdup.s16 vec26, j
    vceq.u8 vecA, vecA, vecA

	pld [yPtr, #(CACHE_LINE_SIZE*0)]
	pld [yPtr, #(CACHE_LINE_SIZE*1)]
	pld [yPtr, #(CACHE_LINE_SIZE*2)]
	.if \yuvFamily == yuv420pFamily || \yuvFamily == yuv422pFamily
		pld [uPtr, #(CACHE_LINE_SIZE*0)]
		pld [uPtr, #(CACHE_LINE_SIZE*1)]
		pld [uPtr, #(CACHE_LINE_SIZE*2)]
		pld [vPtr, #(CACHE_LINE_SIZE*0)]
		pld [vPtr, #(CACHE_LINE_SIZE*1)]
		pld [vPtr, #(CACHE_LINE_SIZE*2)]
	.elseif \yuvFamily == nv12Family || \yuvFamily == nv21Family
		pld [uvPtr, #(CACHE_LINE_SIZE*0)]
		pld [uvPtr, #(CACHE_LINE_SIZE*1)]
		pld [uvPtr, #(CACHE_LINE_SIZE*2)]
	.elseif \yuvFamily == yuyv422Family || \yuvFamily == uyvy422Family
		; yuvPtr = yPtr
	.else
		.error "Not implemented"
	.endif

    .if \yuvFamily == yuv420pFamily
        add padY, width, #15
        and padY, padY, #-16
        sub padY, stride, padY
        add padUV, padY, #1
        lsr padUV, padUV, #1
        .equ y_step, 16
	.elseif \yuvFamily == yuv422pFamily
		add padY, width, #15
	    and padY, padY, #-16
	    sub padY, stride, padY
        add padUV, padY, #1
        lsr padUV, padUV, #1
        .equ y_step, 16
	.elseif \yuvFamily == nv12Family || \yuvFamily == nv21Family
        add padY, width, #15
	    and padY, padY, #-16
	    sub padY, stride, padY
        mov padUV, padY
        .equ y_step, 16
	.elseif \yuvFamily == yuyv422Family || \yuvFamily == uyvy422Family
        lsl width, width, #1
        lsl stride, stride, #1
        add padY, width, #31
        and padY, padY, #-32
        sub padY, stride, padY
        .equ y_step, 32
	.else
		.error "Not implemented"
	.endif
	
    .if \rgbxFamily == rgb24Family
        .if \yuvFamily == yuyv422Family || \yuvFamily == uyvy422Family
            add padRGBx, padY, padY, LSR #1 @ padRGBx = ((padY * 3) / 2)
        .else
		    add padRGBx, padY, padY, LSL #1 @ padRGBx = (padY * 3)
        .endif
	.elseif \rgbxFamily == rgba32Family
        .if \yuvFamily == yuyv422Family || \yuvFamily == uyvy422Family
            lsl padRGBx, padY, #1 @ padRGBx = (padY * 2)
        .else
		    lsl padRGBx, padY, #2 @ padRGBx = (padY * 4)
        .endif
	.else
		.error "Not implemented"
	.endif

    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (j = 0; j < height; ++j)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    mov j, #0
    LoopHeight_CompVImageConvYuv420p_to_Rgb24_Asm_NEON32\rgbxFamily\yuvFamily:
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (i = 0, k = 0, l = 0; i < width; i += y_step, k += rgbx_step, l += uv_step)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        mov i, #0
        LoopWidth_CompVImageConvYuv420p_to_Rgb24_Asm_NEON32\rgbxFamily\yuvFamily:
            @ Load samples @
			pld [yPtr, #(CACHE_LINE_SIZE*3)]
			.if \yuvFamily == yuv420pFamily || \yuvFamily == yuv422pFamily
				pld [uPtr, #(CACHE_LINE_SIZE*3)]
				pld [vPtr, #(CACHE_LINE_SIZE*3)]
			.elseif \yuvFamily == nv12Family || \yuvFamily == nv21Family
				pld [uvPtr, #(CACHE_LINE_SIZE*3)]
			.elseif \yuvFamily == yuyv422Family || \yuvFamily == uyvy422Family
				@ yuvPtr = yPtr
			.else
				.error "Not implemented"
			.endif
            .if \yuvFamily == yuv420pFamily || \yuvFamily == yuv422pFamily
                vld1.u8 {vecYlow}, [yPtr :128]! @ #16 Y samples
                vld1.u8 {vecUn}, [uPtr :64]! @ #8 U samples, low mem
                vld1.u8 {vecVn}, [vPtr :64]! @ #8 V samples, low mem
            .elseif \yuvFamily == nv12Family
                vld1.u8 {vecYlow}, [yPtr :128]! @ #16 Y samples
                vld2.u8 {vecUn, vecVn}, [uvPtr :64]! @ #8 U samples and #8 V samples, low mem
            .elseif \yuvFamily == nv21Family
                vld1.u8 {vecYlow}, [yPtr :128]! @ #16 Y samples
                vld2.u8 {vecVn, vecUn}, [uvPtr :64]! @ #8 U samples and #8 V samples, low mem
            .elseif \yuvFamily == yuyv422Family
                vld2.u8 {vecYlowx, vecYhighx}, [yPtr :64]!
                vld2.u8 {vecYlowy, vecYhighy}, [yPtr :64]!
                vuzp.u8 vecYhighx, vecYhighy
                vmov vecUn, vecYhighx
                vmov vecVn, vecYhighy
            .elseif \yuvFamily == uyvy422Family
                vld2.u8 {vecYhighx, vecYlowx}, [yPtr :64]!
                vld2.u8 {vecYhighy, vecYlowy}, [yPtr :64]!
                vuzp.u8 vecYhighx, vecYhighy
                vmov vecUn, vecYhighx
                vmov vecVn, vecYhighy
            .else
				.error "Not implemented"
			.endif

            vsubl.u8 vecV, vecVn, vec127n
            vsubl.u8 vecU, vecUn, vec127n
            vsubl.u8 vecYhigh, vecYlowy, vec16n
            vsubl.u8 vecYlow, vecYlowx, vec16n
            vmul.s16 vec0, vecV, vec51
            vmul.s16 vec1, vecU, vec65
            vmul.s16 vecU, vecU, vec13
            vmul.s16 vecYlow, vecYlow, vec37
            vmul.s16 vecYhigh, vecYhigh, vec37
            vmov vecR, vec0
            vzip.s16 vecR, vec0 @ duplicate UV because of 1/2 sampling
            vmov vecB, vec1
            vadd.s16 vecR, vecYlow, vecR
            vzip.s16 vecB, vec1 @ duplicate UV because of 1/2 sampling
            vadd.s16 vec0, vecYhigh, vec0
            vmla.s16 vecU, vecV, vec26
            vqshrun.s16 vecRx, vecR, #5
            vqshrun.s16 vecRy, vec0, #5
            vadd.s16 vecB, vecYlow, vecB
            vmov vecV, vecU
            vadd.s16 vec1, vecYhigh, vec1
            vzip.s16 vecU, vecV @ duplicate UV because of 1/2 sampling
            vqshrun.s16 vecBx, vecB, #5
            vsub.s16 vecU, vecYlow, vecU
            vsub.s16 vecV, vecYhigh, vecV
            vqshrun.s16 vecGx, vecU, #5
            vqshrun.s16 vecBy, vec1, #5
            vqshrun.s16 vecGy, vecV, #5
            
            .if \rgbxFamily == rgb24Family
                vst3.u8 {vecRx, vecGx, vecBx}, [rgbPtr :64]!
                vst3.u8 {vecRy, vecGy, vecBy}, [rgbPtr :64]!
            .elseif \rgbxFamily == rgba32Family
                vst4.u8 {vecRx, vecGx, vecBx, vecAx}, [rgbPtr :64]!
                vst4.u8 {vecRy, vecGy, vecBy, vecAy}, [rgbPtr :64]!
            .else
                .error "Not implemented"
            .endif

            add i, i, #y_step
            cmp i, width
			blt LoopWidth_CompVImageConvYuv420p_to_Rgb24_Asm_NEON32\rgbxFamily\yuvFamily
            @End_of_LoopWidth@

        add yPtr, yPtr, padY
        add rgbPtr, rgbPtr, padRGBx
        .if \yuvFamily == yuv420pFamily
            lsr i, i, #1 @ div i by #2 because UV sub-sampled at 1/2
            neg i, i @ negate i for rollbacking uPtr and vPtr
            tst j, #1 @ check whether j is odd
            movne i, padUV @ set i to padUV to move forward if j is odd
            add uPtr, uPtr, i @ rollback or move forward
            add vPtr, vPtr, i @ rollback or move forward
        .elseif \yuvFamily == yuv422pFamily
            add uPtr, uPtr, padUV
            add vPtr, vPtr, padUV
        .elseif \yuvFamily == nv12Family || \yuvFamily == nv21Family
            neg i, i @ negate i for rollbacking uvPtr (without 1/2 sub-sampling)
            tst j, #1 @ check whether j is odd
            movne i, padUV @ set i to padUV to move forward if j is odd
            add uvPtr, uvPtr, i @ rollback or move forward
        .elseif \yuvFamily == yuyv422Family || \yuvFamily == uyvy422Family
            @ yuvPtr = yPtr (already incremented, above)
        .else
            .error "Not implemented"
        .endif

        add j, j, #1
        cmp j, height
		blt LoopHeight_CompVImageConvYuv420p_to_Rgb24_Asm_NEON32\rgbxFamily\yuvFamily	
		@End_of_LoopHeight@

    @ undefs @
    .unreq yPtr
	 .if \yuvFamily == yuv420pFamily || \yuvFamily == yuv422pFamily
        .unreq uPtr
		.unreq vPtr
	.elseif \yuvFamily == nv12Family || \yuvFamily == nv21Family
        .unreq uvPtr
	.elseif \yuvFamily == yuyv422Family || \yuvFamily == uyvy422Family
        @ yPtr
	.else
		.error "Not implemented"
	.endif
	.unreq rgbPtr
	.unreq width
	.unreq height
	.unreq stride
    .unreq i
    .unreq padY
    .unreq padRGBx
    .unreq j
    .unreq padUV

    .unreq vecYlow
    .unreq vecYlowx
    .unreq vecYlowy
    .unreq vecYhigh
    .unreq vecYhighx
    .unreq vecYhighy
    .unreq vecU
    .unreq vecUn
    .unreq vecV
    .unreq vecVn
    .unreq vec16n
    .unreq vec127n
    .unreq vec37
    .unreq vec51
    .unreq vec65
    .unreq vec13
    .unreq vec26
    .unreq vec0
    .unreq vec1
    .unreq vecR
    .unreq vecRx
    .unreq vecRy
    .unreq vecG
    .unreq vecGx
    .unreq vecGy
    .unreq vecB
    .unreq vecBx
    .unreq vecBy
    .unreq vecA
    .unreq vecAx
    .unreq vecAy

	COMPV_GAS_RESTORE_NEON_REGS
    .if \yuvFamily == yuv420pFamily || \yuvFamily == yuv422pFamily
		COMPV_GAS_UNSHADOW_ARGS 7
	.elseif \yuvFamily == nv12Family || \yuvFamily == nv21Family
		COMPV_GAS_UNSHADOW_ARGS 6
	.elseif \yuvFamily == yuyv422Family || \yuvFamily == uyvy422Family
		COMPV_GAS_UNSHADOW_ARGS 5
	.else
		.error "Not implemented"
	.endif
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVImageConvYuv420p_to_Rgb24_Asm_NEON32
	CompVImageConvYuv_to_Rgbx_Macro_NEON32 rgb24Family, yuv420pFamily

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVImageConvYuv420p_to_Rgba32_Asm_NEON32
	CompVImageConvYuv_to_Rgbx_Macro_NEON32 rgba32Family, yuv420pFamily

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVImageConvYuv422p_to_Rgb24_Asm_NEON32
	CompVImageConvYuv_to_Rgbx_Macro_NEON32 rgb24Family, yuv422pFamily

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVImageConvYuv422p_to_Rgba32_Asm_NEON32
	CompVImageConvYuv_to_Rgbx_Macro_NEON32 rgba32Family, yuv422pFamily

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVImageConvNv12_to_Rgb24_Asm_NEON32
	CompVImageConvYuv_to_Rgbx_Macro_NEON32 rgb24Family, nv12Family

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVImageConvNv12_to_Rgba32_Asm_NEON32
	CompVImageConvYuv_to_Rgbx_Macro_NEON32 rgba32Family, nv12Family

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVImageConvNv21_to_Rgb24_Asm_NEON32
	CompVImageConvYuv_to_Rgbx_Macro_NEON32 rgb24Family, nv21Family

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVImageConvNv21_to_Rgba32_Asm_NEON32
	CompVImageConvYuv_to_Rgbx_Macro_NEON32 rgba32Family, nv21Family

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVImageConvYuyv422_to_Rgb24_Asm_NEON32
	CompVImageConvYuv_to_Rgbx_Macro_NEON32 rgb24Family, yuyv422Family

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVImageConvYuyv422_to_Rgba32_Asm_NEON32
	CompVImageConvYuv_to_Rgbx_Macro_NEON32 rgba32Family, yuyv422Family

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVImageConvUyvy422_to_Rgb24_Asm_NEON32
	CompVImageConvYuv_to_Rgbx_Macro_NEON32 rgb24Family, uyvy422Family

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVImageConvUyvy422_to_Rgba32_Asm_NEON32
	CompVImageConvYuv_to_Rgbx_Macro_NEON32 rgba32Family, uyvy422Family

#endif /* defined(__arm__) && !defined(__aarch64__) */

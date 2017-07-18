#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__aarch64__)
.include "compv_common_arm64.S" //"

.data

.text

.equ rgb24Family,	0
.equ rgba32Family,	1

.equ yuv420pFamily,	2
.equ yuv422pFamily,	3
.equ yuv444pFamily,	4   // there is intrin implementation but no asm (for now useless)
.equ nv12Family,	5
.equ nv21Family,	6
.equ yuyv422Family,	7
.equ uyvy422Family,	8

#####################################################################
# arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* yPtr
# arg(x) -> COMPV_ALIGNED(NEON) const uint8_t* uPtr
# arg(x) -> COMPV_ALIGNED(NEON) const uint8_t* vPtr
# arg(x) -> COMPV_ALIGNED(NEON) uint8_t* rgbPtr
# arg(x) -> compv_uscalar_t width
# arg(x) -> compv_uscalar_t height
# arg(x) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
.macro CompVImageConvYuv_to_Rgbx_Macro_NEON64 rgbxFamily, yuvFamily
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	# set arguments #
    .if \yuvFamily == yuv420pFamily || \yuvFamily == yuv422pFamily
        yPtr .req r0
        uPtr .req r1
        vPtr .req r2
        rgbPtr .req r3
        width .req r4
        height .req r5
        stride .req r6
	.elseif \yuvFamily == nv12Family || \yuvFamily == nv21Family
        yPtr .req r0
        uvPtr .req r1
        rgbPtr .req r2
        width .req r3
        height .req r4
        stride .req r5
	.elseif \yuvFamily == yuyv422Family || \yuvFamily == uyvy422Family
        yPtr .req r0
        rgbPtr .req r1
        width .req r2
        height .req r3
        stride .req r4
	.else
		.error "Not implemented"
	.endif
    i .req r7
    iw .req r7w
    padY .req r8
    padYw .req r8w
    padRGBx .req r9
    padRGBxw .req r9w
    j .req r10
    jw .req r10w
    padUV .req r11
    padUVw .req r11w

    .if \yuvFamily == uyvy422Family
        vecYlow .req v1
        vecYhigh .req v0
    .else
        vecYlow .req v0
        vecYhigh .req v1
    .endif
    .if \yuvFamily == nv21Family
        vecU .req v3
        vecUn .req v3
        vecV .req v2
        vecVn .req v2
    .else
        vecU .req v2
        vecUn .req v2
        vecV .req v3
        vecVn .req v3
    .endif
    vec16 .req v4
    vec127 .req v5
    vec37 .req v6
    vec51 .req v7
    vec65 .req v8
    vec13 .req v9
    vec26 .req v10
    vec0 .req v11
    vec1 .req v12
    vecR .req v13
    vecG .req v14
    vecB .req v15
    vecA .req v16

    prfm pldl1keep, [yPtr, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [yPtr, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [yPtr, #(CACHE_LINE_SIZE*2)]
	.if \yuvFamily == yuv420pFamily || \yuvFamily == yuv422pFamily
		prfm pldl1keep, [uPtr, #(CACHE_LINE_SIZE*0)]
		prfm pldl1keep, [uPtr, #(CACHE_LINE_SIZE*1)]
		prfm pldl1keep, [uPtr, #(CACHE_LINE_SIZE*2)]
		prfm pldl1keep, [vPtr, #(CACHE_LINE_SIZE*0)]
		prfm pldl1keep, [vPtr, #(CACHE_LINE_SIZE*1)]
		prfm pldl1keep, [vPtr, #(CACHE_LINE_SIZE*2)]
	.elseif \yuvFamily == nv12Family || \yuvFamily == nv21Family
		prfm pldl1keep, [uvPtr, #(CACHE_LINE_SIZE*0)]
		prfm pldl1keep, [uvPtr, #(CACHE_LINE_SIZE*1)]
		prfm pldl1keep, [uvPtr, #(CACHE_LINE_SIZE*2)]
	.elseif \yuvFamily == yuyv422Family || \yuvFamily == uyvy422Family
		; yuvPtr = yPtr
	.else
		.error "Not implemented"
	.endif

    mov iw, #16
    mov jw, #127
    mov padYw, #37
    mov padUVw, #51
    mov padRGBxw, #65
    dup vec16.16b, iw
    dup vec127.16b, jw
    dup vec37.8h, padYw
    dup vec51.8h, padUVw
    dup vec65.8h, padRGBxw
    mov iw, #13
    mov jw, #26
    dup vec13.8h, iw
    dup vec26.8h, jw
    cmeq.16b vecA, vecA, vecA

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
            add padRGBx, padY, padY, LSR #1 // padRGBx = ((padY * 3) / 2)
        .else
		    add padRGBx, padY, padY, LSL #1 // padRGBx = (padY * 3)
        .endif
	.elseif \rgbxFamily == rgba32Family
        .if \yuvFamily == yuyv422Family || \yuvFamily == uyvy422Family
            lsl padRGBx, padY, #1 // padRGBx = (padY * 2)
        .else
		    lsl padRGBx, padY, #2 // padRGBx = (padY * 4)
        .endif
	.else
		.error "Not implemented"
	.endif
	
    ########################################
    # for (j = 0; j < height; ++j)
    ########################################
    mov j, #0
    LoopHeight_CompVImageConvYuv420p_to_Rgb24_Asm_NEON64\rgbxFamily\yuvFamily:
        ###################################################################
        # for (i = 0, k = 0, l = 0; i < width; i += 16, k += 48, l += 8)
        ###################################################################
        mov i, #0
        LoopWidth_CompVImageConvYuv420p_to_Rgb24_Asm_NEON64\rgbxFamily\yuvFamily:
            # Load samples #
			prfm pldl1keep, [yPtr, #(CACHE_LINE_SIZE*3)]
			.if \yuvFamily == yuv420pFamily || \yuvFamily == yuv422pFamily
				prfm pldl1keep, [uPtr, #(CACHE_LINE_SIZE*3)]
			    prfm pldl1keep, [vPtr, #(CACHE_LINE_SIZE*3)]
			.elseif \yuvFamily == nv12Family || \yuvFamily == nv21Family
				prfm pldl1keep, [uvPtr, #(CACHE_LINE_SIZE*3)]
			.elseif \yuvFamily == yuyv422Family || \yuvFamily == uyvy422Family
				// yuvPtr = yPtr
			.else
				.error "Not implemented"
			.endif
            .if \yuvFamily == yuv420pFamily || \yuvFamily == yuv422pFamily
                ld1 {vecYlow.16b}, [yPtr], #(16*COMPV_GAS_UINT8_SZ_BYTES) // #16 Y samples
                ld1 {vecUn.8b}, [uPtr], #(8*COMPV_GAS_UINT8_SZ_BYTES) // #8 U samples, low mem
                ld1 {vecVn.8b}, [vPtr], #(8*COMPV_GAS_UINT8_SZ_BYTES) // #8 V samples, low mem
            .elseif \yuvFamily == nv12Family
                ld1 {vecYlow.16b}, [yPtr], #(16*COMPV_GAS_UINT8_SZ_BYTES) // #16 Y samples
                ld2 {vecU.8b, vecV.8b}, [uvPtr], #(16*COMPV_GAS_UINT8_SZ_BYTES) // #8 U samples and #8 V samples, low mem
            .elseif \yuvFamily == nv21Family
                ld1 {vecYlow.16b}, [yPtr], #(16*COMPV_GAS_UINT8_SZ_BYTES) // #16 Y samples
                ld2 {vecV.8b, vecU.8b}, [uvPtr], #(16*COMPV_GAS_UINT8_SZ_BYTES) // #8 U samples and #8 V samples, low mem
            .elseif \yuvFamily == yuyv422Family
                ld2 {vecYlow.16b, vecYhigh.16b}, [yPtr], #((2*16)*COMPV_GAS_UINT8_SZ_BYTES) // #32 YUV samples
                uzp1 vecUn.16b, vecYhigh.16b, vecYhigh.16b
                uzp2 vecVn.16b, vecYhigh.16b, vecYhigh.16b
            .elseif \yuvFamily == uyvy422Family
                ld2 {vecYhigh.16b, vecYlow.16b}, [yPtr], #((2*16)*COMPV_GAS_UINT8_SZ_BYTES) // #32 YUV samples
                uzp1 vecUn.16b, vecYhigh.16b, vecYhigh.16b
                uzp2 vecVn.16b, vecYhigh.16b, vecYhigh.16b
            .else
				.error "Not implemented"
			.endif

            usubl vecV.8h, vecVn.8b, vec127.8b
            usubl vecU.8h, vecUn.8b, vec127.8b
            usubl2 vecYhigh.8h, vecYlow.16b, vec16.16b
            usubl vecYlow.8h, vecYlow.8b, vec16.8b
            mul vec0.8h, vecV.8h, vec51.8h
            mul vec1.8h, vecU.8h, vec65.8h
            mul vecU.8h, vecU.8h, vec13.8h
            mul vecYlow.8h, vecYlow.8h, vec37.8h
            mul vecYhigh.8h, vecYhigh.8h, vec37.8h
            zip1 vecR.8h, vec0.8h, vec0.8h // duplicate UV because of 1/2 sampling
            zip2 vec0.8h, vec0.8h, vec0.8h // duplicate UV because of 1/2 sampling
            zip1 vecB.8h, vec1.8h, vec1.8h // duplicate UV because of 1/2 sampling
            zip2 vec1.8h, vec1.8h, vec1.8h // duplicate UV because of 1/2 sampling
            add vecR.8h, vecYlow.8h, vecR.8h
            add vec0.8h, vecYhigh.8h, vec0.8h
            mla vecU.8h, vecV.8h, vec26.8h
            sqshrun vecR.8b, vecR.8h, #5
            sqshrun2 vecR.16b, vec0.8h, #5
            add vecB.8h, vecYlow.8h, vecB.8h
            zip1 vecV.8h, vecU.8h, vecU.8h // duplicate UV because of 1/2 sampling
            zip2 vecU.8h, vecU.8h, vecU.8h // duplicate UV because of 1/2 sampling
            add vec1.8h, vecYhigh.8h, vec1.8h
            sqshrun vecB.8b, vecB.8h, #5
            sub vecV.8h, vecYlow.8h, vecV.8h
            sub vecU.8h, vecYhigh.8h, vecU.8h
            sqshrun vecG.8b, vecV.8h, #5
            sqshrun2 vecB.16b, vec1.8h, #5
            sqshrun2 vecG.16b, vecU.8h, #5
            
            .if \rgbxFamily == rgb24Family
                st3 {vecR.16b, vecG.16b, vecB.16b}, [rgbPtr], #((16*3)*COMPV_GAS_UINT8_SZ_BYTES)
            .elseif \rgbxFamily == rgba32Family
                st4 {vecR.16b, vecG.16b, vecB.16b, vecA.16b}, [rgbPtr], #((16*4)*COMPV_GAS_UINT8_SZ_BYTES)
            .else
                .error "Not implemented"
            .endif
            
            add i, i, #y_step
            cmp i, width
			blt LoopWidth_CompVImageConvYuv420p_to_Rgb24_Asm_NEON64\rgbxFamily\yuvFamily
            #End_of_LoopWidth#

        add yPtr, yPtr, padY
        add rgbPtr, rgbPtr, padRGBx
        
        .if \yuvFamily == yuv420pFamily
            lsr i, i, #1 // div i by #2 because UV sampled at 1/2
            neg i, i // negate i for rollbacking uPtr and vPtr
            tst j, #1 // check whether j is odd
            csel i, padUV, i, ne // set i to padUV to move forward if j is odd
            add uPtr, uPtr, i // rollback or move forward
            add vPtr, vPtr, i // rollback or move forward
        .elseif \yuvFamily == yuv422pFamily
            add uPtr, uPtr, padUV
            add vPtr, vPtr, padUV
        .elseif \yuvFamily == nv12Family || \yuvFamily == nv21Family
            neg i, i // negate i for rollbacking uvPtr (without 1/2 sub-sampling)
            tst j, #1 // check whether j is odd
            csel i, padUV, i, ne // set i to padUV to move forward if j is odd
            add uvPtr, uvPtr, i // rollback or move forward
        .elseif \yuvFamily == yuyv422Family || \yuvFamily == uyvy422Family
            // yuvPtr = yPtr (already incremented, above)
        .else
            .error "Not implemented"
        .endif

        add j, j, #1
        cmp j, height
		blt LoopHeight_CompVImageConvYuv420p_to_Rgb24_Asm_NEON64\rgbxFamily\yuvFamily	
		#End_of_LoopHeight#

    # undefs #
    .unreq yPtr
	.unreq uPtr
	.unreq vPtr
	.unreq rgbPtr
	.unreq width
	.unreq height
	.unreq stride
    .unreq i
    .unreq iw
    .unreq padY
    .unreq padYw
    .unreq padRGBx
    .unreq padRGBxw
    .unreq j
    .unreq jw
    .unreq padUV
    .unreq padUVw

    .unreq vecYlow
    .unreq vecYhigh
    .unreq vecU
    .unreq vecUn
    .unreq vecV
    .unreq vecVn
    .unreq vec16
    .unreq vec127
    .unreq vec37
    .unreq vec51
    .unreq vec65
    .unreq vec13
    .unreq vec26
    .unreq vec0
    .unreq vec1
    .unreq vecR
    .unreq vecG
    .unreq vecB
    .unreq vecA

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVImageConvYuv420p_to_Rgb24_Asm_NEON64
    CompVImageConvYuv_to_Rgbx_Macro_NEON64 rgb24Family, yuv420pFamily

########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVImageConvYuv420p_to_Rgba32_Asm_NEON64
    CompVImageConvYuv_to_Rgbx_Macro_NEON64 rgba32Family, yuv420pFamily

########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVImageConvYuv422p_to_Rgb24_Asm_NEON64
    CompVImageConvYuv_to_Rgbx_Macro_NEON64 rgb24Family, yuv422pFamily

########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVImageConvYuv422p_to_Rgba32_Asm_NEON64
    CompVImageConvYuv_to_Rgbx_Macro_NEON64 rgba32Family, yuv422pFamily

########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVImageConvNv12_to_Rgb24_Asm_NEON64
	CompVImageConvYuv_to_Rgbx_Macro_NEON64 rgb24Family, nv12Family

########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVImageConvNv12_to_Rgba32_Asm_NEON64
	CompVImageConvYuv_to_Rgbx_Macro_NEON64 rgba32Family, nv12Family

########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVImageConvNv21_to_Rgb24_Asm_NEON64
	CompVImageConvYuv_to_Rgbx_Macro_NEON64 rgb24Family, nv21Family

########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVImageConvNv21_to_Rgba32_Asm_NEON64
	CompVImageConvYuv_to_Rgbx_Macro_NEON64 rgba32Family, nv21Family

########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVImageConvYuyv422_to_Rgb24_Asm_NEON64
	CompVImageConvYuv_to_Rgbx_Macro_NEON64 rgb24Family, yuyv422Family

########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVImageConvYuyv422_to_Rgba32_Asm_NEON64
	CompVImageConvYuv_to_Rgbx_Macro_NEON64 rgba32Family, yuyv422Family

########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVImageConvUyvy422_to_Rgb24_Asm_NEON64
	CompVImageConvYuv_to_Rgbx_Macro_NEON64 rgb24Family, uyvy422Family

########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVImageConvUyvy422_to_Rgba32_Asm_NEON64
	CompVImageConvYuv_to_Rgbx_Macro_NEON64 rgba32Family, uyvy422Family

#endif /* defined(__aarch64__) */

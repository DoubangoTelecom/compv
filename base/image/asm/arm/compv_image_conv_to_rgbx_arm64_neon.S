#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__aarch64__)
.include "compv_common_arm64.S"

.data

.text

########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* yPtr
# arg(1) -> COMPV_ALIGNED(NEON) const uint8_t* uPtr
# arg(2) -> COMPV_ALIGNED(NEON) const uint8_t* vPtr
# arg(3) -> COMPV_ALIGNED(NEON) uint8_t* rgbPtr
# arg(4) -> compv_uscalar_t width
# arg(5) -> compv_uscalar_t height
# arg(6) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVImageConvYuv420_to_Rgb24_Asm_NEON64
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	# set arguments #
	yPtr .req r0
	uPtr .req r1
	vPtr .req r2
	rgbPtr .req r3
	width .req r4
	height .req r5
	stride .req r6
    i .req r7
    iw .req r7w
    padY .req r8
    padYw .req r8w
    padRGB .req r9
    padRGBw .req r9w
    j .req r10
    jw .req r10w
    padUV .req r11
    padUVw .req r11w

    vecYlow .req v0
    vecYhigh .req v1
    vecU .req v2
    vecUn .req v2
    vecV .req v3
    vecVn .req v3
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

    mov iw, #16
    mov jw, #127
    mov padYw, #37
    mov padUVw, #51
    mov padRGBw, #65
    dup vec16.16b, iw
    dup vec127.16b, jw
    dup vec37.8h, padYw
    dup vec51.8h, padUVw
    dup vec65.8h, padRGBw
    mov iw, #13
    mov jw, #26
    dup vec13.8h, iw
    dup vec26.8h, jw

    add padY, width, #15
	and padY, padY, #-16
	sub padY, stride, padY
    add padRGB, padY, padY, LSL #1 // padRGB = (padY * 3)
    add padUV, padY, #1
    lsr padUV, padUV, #1 // padUV = ((padY + 1)) >>1, to get rid of odd padY values
	
    ########################################
    # for (j = 0; j < height; ++j)
    ########################################
    mov j, #0
    LoopHeight_CompVImageConvYuv420_to_Rgb24_Asm_NEON64:
        ###################################################################
        # for (i = 0, k = 0, l = 0; i < width; i += 16, k += 48, l += 8)
        ###################################################################
        mov i, #0
        LoopWidth_CompVImageConvYuv420_to_Rgb24_Asm_NEON64:
            # Load samples #
            ld1 {vecYlow.16b}, [yPtr], #(16*COMPV_GAS_UINT8_SZ_BYTES) // #16 Y samples
            ld1 {vecUn.8b}, [uPtr], #(8*COMPV_GAS_UINT8_SZ_BYTES) // #8 U samples, low mem
            ld1 {vecVn.8b}, [vPtr], #(8*COMPV_GAS_UINT8_SZ_BYTES) // #8 V samples, low mem

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
            
            st3 {vecR.16b, vecG.16b, vecB.16b}, [rgbPtr], #((16*3)*COMPV_GAS_UINT8_SZ_BYTES)
            add i, i, #16
            cmp i, width
			blt LoopWidth_CompVImageConvYuv420_to_Rgb24_Asm_NEON64
            #End_of_LoopWidth#

        add yPtr, yPtr, padY
        add rgbPtr, rgbPtr, padRGB
        lsr i, i, #1 // div i by #2 because UV sampled at 1/2
        neg i, i // negate i for rollbacking uPtr and vPtr
        tst j, #1 // check whether j is odd
        csel i, padUV, i, ne // set i to padUV to move forward if j is odd
        add uPtr, uPtr, i // rollback or move forward
        add vPtr, vPtr, i // rollback or move forward
        add j, j, #1
        cmp j, height
		blt LoopHeight_CompVImageConvYuv420_to_Rgb24_Asm_NEON64	
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
    .unreq padRGB
    .unreq padRGBw
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

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#endif /* defined(__aarch64__) */

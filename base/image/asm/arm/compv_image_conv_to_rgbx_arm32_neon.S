#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__arm__) && !defined(__aarch64__)
.include "compv_common_arm32.S"

.data

.text


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* yPtr
@ arg(1) -> COMPV_ALIGNED(NEON) const uint8_t* uPtr
@ arg(2) -> COMPV_ALIGNED(NEON) const uint8_t* vPtr
@ arg(3) -> COMPV_ALIGNED(NEON) uint8_t* rgbPtr
@ arg(4) -> compv_uscalar_t width
@ arg(5) -> compv_uscalar_t height
@ arg(6) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVImageConvYuv420_to_Rgb24_Asm_NEON32
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 7
	COMPV_GAS_SAVE_NEON_REGS

	@ Load arguments @
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
    padRGB .req r9
    j .req r10
    padUV .req r11

    vecYlow .req q0
    vecYlowx .req q0x
    vecYlowy .req q0y
    vecYhigh .req q1
    vecYhighx .req q1x
    vecYhighy .req q1y
    vecU .req q2
    vecUn .req q2x
    vecV .req q3
    vecVn .req q3x
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

    mov i, #16
    mov j, #127
    mov padY, #37
    mov padUV, #51
    mov padRGB, #65
    vdup.u8 vec16n, i
    vdup.u8 vec127n, j
    vdup.s16 vec37, padY
    vdup.s16 vec51, padUV
    vdup.s16 vec65, padRGB
    mov i, #13
    mov j, #26
    vdup.s16 vec13, i
    vdup.s16 vec26, j

    add padY, width, #15
	and padY, padY, #-16
	sub padY, stride, padY
    add padRGB, padY, padY, LSL #1 @ padRGB = (padY * 3)
    add padUV, padY, #1
    lsr padUV, padUV, #1 @ padUV = ((padY + 1)) >>1, to get rid of odd padY values
	
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (j = 0; j < height; ++j)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    mov j, #0
    LoopHeight_CompVImageConvYuv420_to_Rgb24_Asm_NEON32:
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (i = 0, k = 0, l = 0; i < width; i += 16, k += 48, l += 8)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        mov i, #0
        LoopWidth_CompVImageConvYuv420_to_Rgb24_Asm_NEON32:
            @ Load samples @
            vld1.u8 vecYlow, [yPtr :128]! @ #16 Y samples
            vld1.u8 vecUn, [uPtr :64]! @ #8 U samples, low mem
            vld1.u8 vecVn, [vPtr :64]! @ #8 V samples, low mem

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
            
            vst3.u8 {vecRx, vecGx, vecBx}, [rgbPtr :64]!
            vst3.u8 {vecRy, vecGy, vecBy}, [rgbPtr :64]!
            add i, i, #16
            cmp i, width
			blt LoopWidth_CompVImageConvYuv420_to_Rgb24_Asm_NEON32
            @End_of_LoopWidth@

        add yPtr, yPtr, padY
        add rgbPtr, rgbPtr, padRGB
        lsr i, i, #1 @ div i by #2 because UV sampled at 1/2
        neg i, i @ negate i for rollbacking uPtr and vPtr
        tst j, #1 @ check whether j is odd
        movne i, padUV @ set i to padUV to move forward if j is odd
        add uPtr, uPtr, i @ rollback or move forward
        add vPtr, vPtr, i @ rollback or move forward
        add j, j, #1
        cmp j, height
		blt LoopHeight_CompVImageConvYuv420_to_Rgb24_Asm_NEON32	
		@End_of_LoopHeight@

    @ undefs @
    .unreq yPtr
	.unreq uPtr
	.unreq vPtr
	.unreq rgbPtr
	.unreq width
	.unreq height
	.unreq stride
    .unreq i
    .unreq padY
    .unreq padRGB
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

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 7
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#endif /* defined(__arm__) && !defined(__aarch64__) */

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

.equ familyRGB24, 0
.equ familyRGBA32, 1

#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* rgbxPtr
# arg(1) -> COMPV_ALIGNED(NEON) uint8_t* hsvPtr
# arg(2) -> compv_uscalar_t width
# arg(3) -> compv_uscalar_t height
# arg(4) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
# family -> familyRGB24 or familyRGBA32
.macro CompVImageConvRgbxToHsv_Macro_NEON64 family
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	# set arguments #
	rgbxPtr .req r0
	hsvPtr .req r1
	width .req r2
	height .req r3
    stride .req r4
    i .req r5
    padHsv .req r6
	padRgbx .req r7

    vecZero .req v0
    vec255f .req v1
    vec0 .req v2
    vec1 .req v3
    vec2 .req v4
    vec3 .req v5
    vec5 .req v6
    vec6 .req v7
    vec9 .req v8
    vec8 .req v9
    vec4 .req v10
    vec7 .req v11
    vec0f .req v12
    vec1f .req v13
    vec2f .req v14
    vec3f .req v15
    vec43n .req v16
    vec85 .req v17
    vec171 .req v18
    vecFF .req v19
    // upto v23 (included) are free

    add padHsv, width, #15
	and padHsv, padHsv, #-16
	sub padHsv, stride, padHsv
	.if \family == familyRGB24
		add padRgbx, padHsv, padHsv, LSL #1 // from samples to bytes "padRgbx = (pad * 3)"
	.elseif \family == familyRGBA32
		lsl padRgbx, padHsv, #2 // from samples to bytes "padRgbx = (pad * 4)"
	.elseif
		.error not implemented
	.endif
    add padHsv, padHsv, padHsv, LSL #1 // from samples to bytes (pad * 3)
    add width, width, width, LSL #1 // from samples to bytes (width * 3)

	prfm pldl1keep, [rgbxPtr, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [rgbxPtr, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [rgbxPtr, #(CACHE_LINE_SIZE*2)]

    movz r10w, #0x437f, lsl #16
    mov r11, #85
    mov r12, #171
    mov r13, #43
    dup vec255f.4s, r10w
    dup vec85.16b, r11w
    dup vec171.16b, r12w
    dup vec43n.16b, r13w
    cmeq vecFF.16b, vecFF.16b, vecFF.16b
    eor vecZero.16b, vecZero.16b, vecZero.16b

    ########################################
    # for (j = 0; j < height; ++j)
    ########################################
    LoopHeight_CompVImageConvRgbxToHsv_Asm_NEON64\family:
        ########################################################
        # for (i = 0; i < width; i += 48) {  // 48 = (16 * 3)
        ########################################################
        mov i, #0
        LoopWidth_CompVImageConvRgbxToHsv_Asm_NEON64\family:
			prfm pldl1keep, [rgbxPtr, #(CACHE_LINE_SIZE*3)]
			.if \family == familyRGB24
				ld3 {vec0f.16b, vec1f.16b, vec2f.16b}, [rgbxPtr], #((16*3)*COMPV_GAS_UINT8_SZ_BYTES)
			.elseif \family == familyRGBA32
				ld4 {vec0f.16b, vec1f.16b, vec2f.16b, vec3f.16b}, [rgbxPtr], #((16*4)*COMPV_GAS_UINT8_SZ_BYTES)
			.elseif
				.error not implemented
			.endif

            umin vec3.16b, vec0f.16b, vec1f.16b
            umax vec4.16b, vec0f.16b, vec1f.16b
            umin vec3.16b, vec3.16b, vec2f.16b
            umax vec4.16b, vec4.16b, vec2f.16b
            cmeq vec5.16b, vec4.16b, vec0f.16b
            usubl vec1.8h, vec4.8b, vec3.8b
            usubl2 vec3.8h, vec4.16b, vec3.16b            
            cmeq vec6.16b, vec4.16b, vec1f.16b
            orr vec8.16b, vec5.16b, vec6.16b
            bic vec6.16b, vec6.16b, vec5.16b
            bic vec7.16b, vecFF.16b, vec8.16b
            sub vec9.16b, vec0f.16b, vec1f.16b
            sub vec3f.16b, vec1f.16b, vec2f.16b
            sub vec8.16b, vec2f.16b, vec0f.16b
            and vec9.16b, vec7.16b, vec9.16b
            and vec5.16b, vec5.16b, vec3f.16b
            and vec8.16b, vec6.16b, vec8.16b
            orr vec5.16b, vec5.16b, vec8.16b
            uxtl vec0.4s, vec1.4h
            orr vec5.16b, vec5.16b, vec9.16b
            uxtl2 vec1.4s, vec1.8h
            uxtl vec2.4s, vec3.4h
            uxtl2 vec3.4s, vec3.8h
            ucvtf vec0.4s, vec0.4s
            ucvtf vec1.4s, vec1.4s
            ucvtf vec2.4s, vec2.4s
            ucvtf vec3.4s, vec3.4s
            uxtl vec1f.8h, vec4.8b
            uxtl2 vec3f.8h, vec4.16b
            uxtl vec0f.4s, vec1f.4h
            uxtl2 vec1f.4s, vec1f.8h
            uxtl vec2f.4s, vec3f.4h
            uxtl2 vec3f.4s, vec3f.8h
            ucvtf vec0f.4s, vec0f.4s
            ucvtf vec1f.4s, vec1f.4s
            ucvtf vec2f.4s, vec2f.4s
            ucvtf vec3f.4s, vec3f.4s
            cmeq v24.4s, vec0f.4s, vecZero.4s
            cmeq v25.4s, vec1f.4s, vecZero.4s
            cmeq v26.4s, vec2f.4s, vecZero.4s
            cmeq v27.4s, vec3f.4s, vecZero.4s
            frecpe vec0f.4s, vec0f.4s
            frecpe vec1f.4s, vec1f.4s
            frecpe vec2f.4s, vec2f.4s
            frecpe vec3f.4s, vec3f.4s
            cmeq v28.4s, vec0.4s, vecZero.4s
            cmeq v29.4s, vec1.4s, vecZero.4s
            bic vec0f.16b, vec0f.16b, v24.16b
            bic vec1f.16b, vec1f.16b, v25.16b
            fmul vec0f.4s, vec0f.4s, vec255f.4s
            fmul vec1f.4s, vec1f.4s, vec255f.4s
            bic vec2f.16b, vec2f.16b, v26.16b
            bic vec3f.16b, vec3f.16b, v27.16b
            fmul vec2f.4s, vec2f.4s, vec255f.4s
            fmul vec3f.4s, vec3f.4s, vec255f.4s
            cmeq v30.4s, vec2.4s, vecZero.4s
            cmeq v31.4s, vec3.4s, vecZero.4s
            fmul vec0f.4s, vec0f.4s, vec0.4s
            fmul vec1f.4s, vec1f.4s, vec1.4s
            fmul vec2f.4s, vec2f.4s, vec2.4s
            fmul vec3f.4s, vec3f.4s, vec3.4s
            frecpe vec0.4s, vec0.4s
            frecpe vec1.4s, vec1.4s
            fcvtzu vec0f.4s, vec0f.4s
            fcvtzu vec1f.4s, vec1f.4s
            fcvtzu vec2f.4s, vec2f.4s
            fcvtzu vec3f.4s, vec3f.4s
            frecpe vec2.4s, vec2.4s
            frecpe vec3.4s, vec3.4s
            xtn vec0f.4h, vec0f.4s
            xtn2 vec0f.8h, vec1f.4s
            xtn vec2f.4h, vec2f.4s
            xtn2 vec2f.8h, vec3f.4s
            sqxtun vec8.8b, vec0f.8h
            smull vec1f.8h, vec5.8b, vec43n.8b
            smull2 vec3f.8h, vec5.16b, vec43n.16b
            sqxtun2 vec8.16b, vec2f.8h // vec8 = hsv[1].u8
            bic vec0.16b, vec0.16b, v28.16b
            bic vec1.16b, vec1.16b, v29.16b
            sxtl vec0f.4s, vec1f.4h
            sxtl2 vec1f.4s, vec1f.8h
            sxtl vec2f.4s, vec3f.4h
            sxtl2 vec3f.4s, vec3f.8h
            scvtf vec0f.4s, vec0f.4s
            scvtf vec1f.4s, vec1f.4s
            scvtf vec2f.4s, vec2f.4s
            scvtf vec3f.4s, vec3f.4s
            fmul vec0f.4s, vec0f.4s, vec0.4s
            fmul vec1f.4s, vec1f.4s, vec1.4s
            fmul vec2f.4s, vec2f.4s, vec2.4s
            fmul vec3f.4s, vec3f.4s, vec3.4s
            bic vec2.16b, vec2.16b, v30.16b
            bic vec3.16b, vec3.16b, v31.16b
            and vec6.16b, vec6.16b, vec85.16b
            and vec7.16b, vec7.16b, vec171.16b
            fcvtas vec0f.4s, vec0f.4s
            fcvtas vec1f.4s, vec1f.4s
            fcvtas vec2f.4s, vec2f.4s
            fcvtas vec3f.4s, vec3f.4s
            xtn vec0f.4h, vec0f.4s
            xtn2 vec0f.8h, vec1f.4s
            xtn vec2f.4h, vec2f.4s
            xtn2 vec2f.8h, vec3f.4s
            sqxtn vec9.8b, vec0f.8h
            orr vec6.16b, vec6.16b, vec7.16b
            sqxtn2 vec9.16b, vec2f.8h
            sqadd vec9.16b, vec9.16b, vec6.16b
			
            st3 {vec9.16b, vec8.16b, vec4.16b}, [hsvPtr], #((16*3)*COMPV_GAS_UINT8_SZ_BYTES)
            
            add i, i, #((16*3)*COMPV_GAS_UINT8_SZ_BYTES)
            cmp i, width
            blt LoopWidth_CompVImageConvRgbxToHsv_Asm_NEON64\family
            #End_of_LoopWidth#

        add rgbxPtr, rgbxPtr, padRgbx
        add hsvPtr, hsvPtr, padHsv
        subs height, height, #1
		bne LoopHeight_CompVImageConvRgbxToHsv_Asm_NEON64\family
		#End_of_LoopHeight#


    # undefs #
    .unreq rgbxPtr
	.unreq hsvPtr
	.unreq width
	.unreq height
	.unreq stride
    .unreq i
    .unreq padHsv
	.unreq padRgbx

    .unreq vecZero
    .unreq vec255f 
    .unreq vec0 
    .unreq vec1 
    .unreq vec2 
    .unreq vec3 
    .unreq vec5 
    .unreq vec6 
    .unreq vec9 
    .unreq vec8 
    .unreq vec4 
    .unreq vec7 
    .unreq vec0f 
    .unreq vec1f 
    .unreq vec2f 
    .unreq vec3f 
    .unreq vec43n 
    .unreq vec85 
    .unreq vec171 
    .unreq vecFF 

    COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVImageConvRgb24ToHsv_Asm_NEON64
    CompVImageConvRgbxToHsv_Macro_NEON64 familyRGB24

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVImageConvRgba32ToHsv_Asm_NEON64
    CompVImageConvRgbxToHsv_Macro_NEON64 familyRGBA32


#endif /* defined(__aarch64__) */
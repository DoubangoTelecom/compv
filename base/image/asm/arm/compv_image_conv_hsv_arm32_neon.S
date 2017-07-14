#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__arm__) && !defined(__aarch64__)
.include "compv_common_arm32.S" @"

.data
.align 4
data255f: .word 0x437f0000, 0x437f0000, 0x437f0000, 0x437f0000

.text

.equ familyRGB24, 0
.equ familyRGBA32, 1

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* rgbxPtr
@ arg(1) -> COMPV_ALIGNED(NEON) uint8_t* hsvPtr
@ arg(2) -> compv_uscalar_t width
@ arg(3) -> compv_uscalar_t height
@ arg(4) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
@ family -> familyRGB24 or familyRGBA32
.macro CompVImageConvRgbxToHsv_Macro_NEON32 family
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 5
	COMPV_GAS_SAVE_NEON_REGS

    @ Load arguments @
	ldm_args r0-r4
	rgbxPtr .req r0
	hsvPtr .req r1
	width .req r2
	height .req r3
	stride .req r4
    i .req r5
    val43 .req r6
    val85 .req r7
    val171 .req r8
    valHalf .req r9
    padHsv .req r10
	padRgbx .req r11

    vecZero .req q0
    vec255f .req q1
    vec0 .req q2
    vec0x .req q2x
    vec0y .req q2y
    vec1 .req q3
    vec1x .req q3x
    vec1y .req q3y
    vec2 .req q4
    vec2x .req q4x
    vec2y .req q4y
    vec3 .req q5
    vec3x .req q5x
    vec3y .req q5y
    vec5 .req q6
    vec5x .req q6x
    vec5y .req q6y
    vec6 .req q7
    vec6x .req q7x
    vec6y .req q7y
    vec9 .req q8
    vec9x .req q8x
    vec9y .req q8y
    vec8 .req q9
    vec8x .req q9x
    vec8y .req q9y
    vec4 .req q10
    vec4x .req q10x
    vec4y .req q10y
    vec7 .req q11
    vec7x .req q11x
    vec7y .req q11y
    vec0f .req q12
    vec0fx .req q12x
    vec0fy .req q12y
    vec1f .req q13
    vec1fx .req q13x
    vec1fy .req q13y
    vec2f .req q14
    vec2fx .req q14x
    vec2fy .req q14y
    vec3f .req q15
    vec3fx .req q15x
    vec3fy .req q15y

    add padHsv, width, #15
	and padHsv, padHsv, #-16
	sub padHsv, stride, padHsv
	.if \family == familyRGB24
		add padRgbx, padHsv, padHsv, LSL #1 @ from samples to bytes "padRgbx = (pad * 3)"
	.elseif \family == familyRGBA32
		lsl padRgbx, padHsv, #2 @ from samples to bytes "padRgbx = (pad * 4)"
	.else
		.error not implemented
	.endif
    add padHsv, padHsv, padHsv, LSL #1 @ from samples to bytes "padHsv = (pad * 3)"
    add width, width, width, LSL #1 @ from samples to bytes "width = (width * 3)"

	pld [rgbxPtr, #(CACHE_LINE_SIZE*0)]
	pld [rgbxPtr, #(CACHE_LINE_SIZE*1)]
	pld [rgbxPtr, #(CACHE_LINE_SIZE*2)]

    ldr i, =data255f
    vld1.s32 {vec255f}, [i]
    veor.s32 vecZero, vecZero, vecZero
    mov valHalf, #0x3f000000 @ 0.5f
    mov val85, #85
    mov val171, #171
    mov val43, #43

    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (j = 0; j < height; ++j)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    LoopHeight_CompVImageConvRgbxToHsv_Asm_NEON32\family:
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (i = 0; i < width; i += 48) {  // 48 = (16 * 3)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        mov i, #0
        LoopWidth_CompVImageConvRgbxToHsv_Asm_NEON32\family:
			pld [rgbxPtr, #(CACHE_LINE_SIZE*3)]
			.if \family == familyRGB24
				vld3.u8 {vec0fx, vec1fx, vec2fx}, [rgbxPtr :64]!
				vld3.u8 {vec0fy, vec1fy, vec2fy}, [rgbxPtr :64]!
			.elseif \family == familyRGBA32
				vld4.u8 {vec0fx, vec1fx, vec2fx, vec3fx}, [rgbxPtr :64]!
				vld4.u8 {vec0fy, vec1fy, vec2fy, vec3fy}, [rgbxPtr :64]!
			.elseif
				.error not implemented
			.endif

            vmin.u8 vec3, vec0f, vec1f
            vmax.u8 vec4, vec0f, vec1f
            vmin.u8 vec3, vec3, vec2f
            vmax.u8 vec4, vec4, vec2f
            vceq.u8 vec7, vec7, vec7 @ vec7 = vecFF
            vsubl.u8 vec1, vec4x, vec3x
            vsubl.u8 vec3, vec4y, vec3y
            vceq.u8 vec5, vec4, vec0f
            vceq.u8 vec6, vec4, vec1f
            vorr.u8 vec8, vec5, vec6
            vbic.u8 vec6, vec6, vec5
            vbic.u8 vec7, vec7, vec8
            vsub.s8 vec9, vec0f, vec1f
            vsub.s8 vec3f, vec1f, vec2f
            vsub.s8 vec8, vec2f, vec0f
            vand.s8 vec9, vec7, vec9
            vand.s8 vec5, vec5, vec3f
            vand.s8 vec8, vec6, vec8
            vorr.s8 vec5, vec5, vec8
            vmovl.u16 vec0, vec1x
            vorr.s8 vec5, vec5, vec9
            vmovl.u16 vec1, vec1y
            vmovl.u16 vec2, vec3x
            vmovl.u16 vec3, vec3y
            vmovl.u8 vec1f, vec4x
            vmovl.u8 vec3f, vec4y
            vmovl.u16 vec0f, vec1fx
            vmovl.u16 vec1f, vec1fy
            vmovl.u16 vec2f, vec3fx
            vmovl.u16 vec3f, vec3fy
            vcvt.f32.u32 vec0f, vec0f
            vcvt.f32.u32 vec1f, vec1f
            vcvt.f32.u32 vec2f, vec2f
            vcvt.f32.u32 vec3f, vec3f
            vcvt.f32.u32 vec0, vec0
            vcvt.f32.u32 vec1, vec1
            vceqq.s32 vec8, vec0f, vecZero
            vrecpeq.f32 vec0f, vec0f
            vceqq.s32 vec9, vec1f, vecZero
            vrecpeq.f32 vec1f, vec1f
            vcvt.f32.u32 vec2, vec2
            vbicq.u32 vec0f, vec0f, vec8
            vceqq.s32 vec8, vec2f, vecZero
            vmul.f32 vec0f, vec0f, vec255f
            vrecpeq.f32 vec2f, vec2f
            vbicq.u32 vec1f, vec1f, vec9
            vceqq.s32 vec9, vec3f, vecZero
            vmul.f32 vec1f, vec1f, vec255f
            vrecpeq.f32 vec3f, vec3f
            vcvt.f32.u32 vec3, vec3
            vbicq.u32 vec2f, vec2f, vec8
            vmul.f32 vec2f, vec2f, vec255f
            vbicq.u32 vec3f, vec3f, vec9
            vmul.f32 vec3f, vec3f, vec255f
            vmul.f32 vec0f, vec0f, vec0
            vmul.f32 vec1f, vec1f, vec1
            vmul.f32 vec2f, vec2f, vec2
            vmul.f32 vec3f, vec3f, vec3
            vdup.s8 vec9x, val43 @ vec9 = vec43n
            vcvt.u32.f32 vec0f, vec0f
            vcvt.u32.f32 vec1f, vec1f
            vcvt.u32.f32 vec2f, vec2f
            vcvt.u32.f32 vec3f, vec3f
            vmovn.s32 vec0fx, vec0f
            vmovn.s32 vec0fy, vec1f
            vmull.s8 vec1f, vec5x, vec9x
            vmovn.s32 vec2fx, vec2f
            vmovn.s32 vec2fy, vec3f
            vmull.s8 vec3f, vec5y, vec9x
            vqmovun.s16 vec8x, vec0f
            vqmovun.s16 vec8y, vec2f @ vec8 = hsv[1].u8
            vceqq.s32 vec5, vec0, vecZero
            vrecpeq.f32 vec0, vec0
            vmovl.s16 vec0f, vec1fx
            vceqq.s32 vec9, vec1, vecZero
            vcvt.f32.s32 vec0f, vec0f
            vrecpeq.f32 vec1, vec1
            vmovl.s16 vec1f, vec1fy
            vbicq.u32 vec0, vec0, vec5
            vceqq.s32 vec5, vec2, vecZero
            vcvt.f32.s32 vec1f, vec1f
            vrecpeq.f32 vec2, vec2
            vmovl.s16 vec2f, vec3fx
            vbicq.u32 vec1, vec1, vec9
            vcvt.f32.s32 vec2f, vec2f
            vceqq.s32 vec9, vec3, vecZero
            vrecpeq.f32 vec3, vec3
            vmovl.s16 vec3f, vec3fy
            vbicq.u32 vec2, vec2, vec5
            vcvt.f32.s32 vec3f, vec3f
            vbicq.u32 vec3, vec3, vec9
            vmul.f32 vec0f, vec0f, vec0
            vmul.f32 vec1f, vec1f, vec1
            vmul.f32 vec2f, vec2f, vec2
            vmul.f32 vec3f, vec3f, vec3
            vdup.s32 vec5, valHalf @ vec5 = vecHalf
            vshr.u32 vec0, vec0f, #31
            vshr.u32 vec2, vec1f, #31
            vcvt.f32.u32 vec0, vec0
            vcvt.f32.u32 vec2, vec2
            vadd.f32 vec1, vec0f, vec5
            vadd.f32 vec3, vec1f, vec5
            vsubq.f32 vec1, vec1, vec0
            vshr.u32 vec0, vec2f, #31
            vsubq.f32 vec3, vec3, vec2
            vshr.u32 vec2, vec3f, #31
            vcvtq.s32.f32 vec0f, vec1
            vcvtq.s32.f32 vec1f, vec3            
            vcvt.f32.u32 vec0, vec0
            vcvt.f32.u32 vec2, vec2
            vadd.f32 vec1, vec2f, vec5
            vadd.f32 vec3, vec3f, vec5
            vsubq.f32 vec1, vec1, vec0
            vsubq.f32 vec3, vec3, vec2
            vcvtq.s32.f32 vec2f, vec1
            vcvtq.s32.f32 vec3f, vec3
            vdup.s8 vec0, val85 @ vec0 = vec85
            vdup.s8 vec1, val171 @ vec1 = vec171
            vmovn.s32 vec0fx, vec0f
            vmovn.s32 vec0fy, vec1f
            vmovn.s32 vec2fx, vec2f
            vmovn.s32 vec2fy, vec3f
            vqmovn.s16 vec9x, vec0f
            vand.s8 vec6, vec6, vec0
            vqmovn.s16 vec9y, vec2f
            vand.s8 vec7, vec7, vec1
            vorr.s8 vec6, vec6, vec7
            vqadd.s8 vec9, vec9, vec6

            vst3.u8 {vec9x, vec8x, vec4x}, [hsvPtr :64]!
            vst3.u8 {vec9y, vec8y, vec4y}, [hsvPtr :64]!

            add i, i, #48
            cmp i, width
            blt LoopWidth_CompVImageConvRgbxToHsv_Asm_NEON32\family
            @End_of_LoopWidth@

        add rgbxPtr, rgbxPtr, padRgbx
        add hsvPtr, hsvPtr, padHsv
        subs height, height, #1
		bne LoopHeight_CompVImageConvRgbxToHsv_Asm_NEON32\family
		@End_of_LoopHeight@


    @ undefs @
    .unreq rgbxPtr
	.unreq hsvPtr
	.unreq width
	.unreq height
	.unreq stride
    .unreq i
    .unreq val43
    .unreq val85
    .unreq val171
    .unreq valHalf
    .unreq padHsv
	.unreq padRgbx

    .unreq vecZero
    .unreq vec255f
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
    .unreq vec6
    .unreq vec6x
    .unreq vec6y
    .unreq vec7
    .unreq vec7x
    .unreq vec7y
    .unreq vec8
    .unreq vec8x
    .unreq vec8y
    .unreq vec9
    .unreq vec9x
    .unreq vec9y
    .unreq vec0f
    .unreq vec0fx
    .unreq vec0fy
    .unreq vec1f
    .unreq vec1fx
    .unreq vec1fy
    .unreq vec2f
    .unreq vec2fx
    .unreq vec2fy
    .unreq vec3f
    .unreq vec3fx
    .unreq vec3fy

    COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 5
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVImageConvRgb24ToHsv_Asm_NEON32
	CompVImageConvRgbxToHsv_Macro_NEON32 familyRGB24

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVImageConvRgba32ToHsv_Asm_NEON32
	CompVImageConvRgbxToHsv_Macro_NEON32 familyRGBA32


#endif /* defined(__arm__) && !defined(__aarch64__) */

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
.align 4
data255f: .word 0x437f0000, 0x437f0000, 0x437f0000, 0x437f0000

.text

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* rgb24Ptr
@ arg(1) -> COMPV_ALIGNED(NEON) uint8_t* hsvPtr
@ arg(2) -> compv_uscalar_t width
@ arg(3) -> compv_uscalar_t height
@ arg(4) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVImageConvRgb24ToHsv_Asm_NEON32
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 5
	COMPV_GAS_SAVE_NEON_REGS

    @ Load arguments @
	ldm_args r0-r4
	rgb24Ptr .req r0
	hsvPtr .req r1
	width .req r2
	height .req r3
	stride .req r4
    i .req r5
    val43 .req r6
    val85 .req r7
    val171 .req r8
    valHalf .req r9
    pad .req r10

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

    add pad, width, #15
	and pad, pad, #-16
	sub pad, stride, pad
    add pad, pad, pad, LSL #1 @ from samples to bytes (pad * 3)
    add width, width, widtth, LSL #1 @ from samples to bytes (width * 3)

    ldr val85, =data255f
    vld1.s32 {vec255f}, [val85]
    veor.s32 vecZero, vecZero, vecZero
    mov valHalf, #0x3f000000 @ 0.5f
    mov val85, #85
    mov val171, #171
    mov val43, #43

    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (j = 0; j < height; ++j)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    LoopHeight_CompVImageConvRgb24ToHsv_Asm_NEON32:
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (i = 0; i < width; i += 48) {  // 48 = (16 * 3)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        mov i, #0
        LoopWidth_CompVImageConvRgb24ToHsv_Asm_NEON32:
            vld3.u8 {vec0fx, vec1fx, vec2fx}, [rgb24Ptr :64]!
			vld3.u8 {vec0fy, vec1fy, vec2fy}, [rgb24Ptr]!

            vmin.u8 vec3, vec0f, vec1f
            vmax.u8 vec4, vec0f, vec1f
            vmin.u8 vec3, vec3, vec2f
            vmax.u8 vec4, vec4, vec2f
            vsubl.u8 vec1, vec4x, vec3x
            vsubl.u8 vec3, vec4y, vec3y

            vceq.u8 vec5, vec4, vec0f
            vceq.u8 vec6, vec4, vec1f
            vceq.u8 vec7, vec7, vec7 @ vec7 = vecFF
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
            vorr.s8 vec5, vec5, vec9

            @ minus = ToFloat32(ToUInt32(ToUInt16(vec4))) @
            vmovl.u16 vec0, vec1x
            vmovl.u16 vec1, vec1y
            vmovl.u16 vec2, vec3x
            vmovl.u16 vec3, vec3y
            vcvt.f32.u32 vec0, vec0
            vcvt.f32.u32 vec1, vec1
            vcvt.f32.u32 vec2, vec2
            vcvt.f32.u32 vec3, vec3

            @ maxVal = ToFloat32(ToUInt32(ToUInt16(ToUInt8(vec4)))) @
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

            @ scale = maxVal ? (1.f / maxVal) : 0.f @
            vceqq.s32 vec8, vec0f, vecZero
            vrecpeq.f32 vec0f, vec0f
            vceqq.s32 vec9, vec1f, vecZero
            vrecpeq.f32 vec1f, vec1f
            vbicq.u32 vec0f, vec0f, vec8
            vceqq.s32 vec8, vec2f, vecZero
            vrecpeq.f32 vec2f, vec2f
            vbicq.u32 vec1f, vec1f, vec9
            vceqq.s32 vec9, vec3f, vecZero
            vrecpeq.f32 vec3f, vec3f
            vbicq.u32 vec2f, vec2f, vec8
            vbicq.u32 vec3f, vec3f, vec9

            @ scales255 = (255 * scale) @
            vmul.f32 vec0f, vec0f, vec255f
            vmul.f32 vec1f, vec1f, vec255f
            vmul.f32 vec2f, vec2f, vec255f
            vmul.f32 vec3f, vec3f, vec255f

            @ hsv[1].float = static_cast<uint8_t>((scales255 * minus)) - unsigned @
            vmul.f32 vec0f, vec0f, vec0
            vmul.f32 vec1f, vec1f, vec1
            vmul.f32 vec2f, vec2f, vec2
            vmul.f32 vec3f, vec3f, vec3
            vcvt.u32.f32 vec0f, vec0f
            vcvt.u32.f32 vec1f, vec1f
            vcvt.u32.f32 vec2f, vec2f
            vcvt.u32.f32 vec3f, vec3f
            vmovn.s32 vec0fx, vec0f
            vmovn.s32 vec0fy, vec1f
            vmovn.s32 vec2fx, vec2f
            vmovn.s32 vec2fy, vec3f
            vqmovun.s16 vec8x, vec0f
            vqmovun.s16 vec8y, vec2f @ vec8 = hsv[1].u8

            @ B = ToFloat32(ToInt32(ToInt16(diff * 43))) @
            vdup.s8 vec9x, val43 @ vec9 = vec43n
            vmull.s8 vec1f, vec5x, vec9x
            vmull.s8 vec3f, vec5y, vec9x
            vmovl.s16 vec0f, vec1fx
            vmovl.s16 vec1f, vec1fy
            vmovl.s16 vec2f, vec3fx
            vmovl.s16 vec3f, vec3fy
            vcvt.f32.s32 vec0f, vec0f
            vcvt.f32.s32 vec1f, vec1f
            vcvt.f32.s32 vec2f, vec2f
            vcvt.f32.s32 vec3f, vec3f

            @ scale = minus ? (1.f / minus) : 0.f @
            vceqq.s32 vec5, vec0, vecZero
            vrecpeq.f32 vec0, vec0
            vceqq.s32 vec9, vec1, vecZero
            vrecpeq.f32 vec1, vec1
            vbicq.u32 vec0, vec0, vec5
            vceqq.s32 vec5, vec2, vecZero
            vrecpeq.f32 vec2, vec2
            vbicq.u32 vec1, vec1, vec9
            vceqq.s32 vec9, vec3, vecZero
            vrecpeq.f32 vec3, vec3
            vbicq.u32 vec2, vec2, vec5
            vbicq.u32 vec3, vec3, vec9

            @ compute static_cast<uint8_t>(round(B * scale) + ((85 & m1) | (171 & m2)) @
            vmul.f32 vec0f, vec0f, vec0
            vmul.f32 vec1f, vec1f, vec1
            vmul.f32 vec2f, vec2f, vec2
            vmul.f32 vec3f, vec3f, vec3
            vdup.s32 vec5, valHalf @ vec5 = vecHalf
            COMPV_ARM_NEON32_MATH_ROUNDF_2_NEAREST_INT vec0f, vec5, vec0, vec1
            COMPV_ARM_NEON32_MATH_ROUNDF_2_NEAREST_INT vec1f, vec5, vec2, vec3
            COMPV_ARM_NEON32_MATH_ROUNDF_2_NEAREST_INT vec2f, vec5, vec0, vec1
            COMPV_ARM_NEON32_MATH_ROUNDF_2_NEAREST_INT vec3f, vec5, vec2, vec3
            vmovn.s32 vec0fx, vec0f
            vmovn.s32 vec0fy, vec1f
            vmovn.s32 vec2fx, vec2f
            vmovn.s32 vec2fy, vec3f
            vdup.s8 vec0, val85 @ vec0 = vec85
            vdup.s8 vec1, val171 @ vec1 = vec171
            vqmovn.s16 vec9x, vec0f
            vqmovn.s16 vec9y, vec2f
            vand.s8 vec6, vec6, vec0
            vand.s8 vec7, vec7, vec1
            vorr.s8 vec6, vec6, vec7
            vqadd.s8 vec9, vec9, vec6
            vst3.u8 {vec9x, vec8x, vec4x}, [hsvPtr :64]!
            vst3.u8 {vec9y, vec8y, vec4y}, [hsvPtr :64]!

            add i, i, #48
            cmp i, width
            blt LoopWidth_CompVImageConvRgb24ToHsv_Asm_NEON32
            @End_of_LoopWidth@

        add rgb24Ptr, rgb24Ptr, pad
        add hsvPtr, hsvPtr, pad
        subs height, height, #1
		bne LoopHeight_CompVImageConvRgb24ToHsv_Asm_NEON32	
		@End_of_LoopHeight@


    @ undefs @
    .unreq rgb24Ptr
	.unreq hsvPtr
	.unreq width
	.unreq height
	.unreq stride
    .unreq i
    .unreq val43
    .unreq val85
    .unreq val171
    .unreq valHalf
    .unreq pad

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


#endif /* defined(__arm__) && !defined(__aarch64__) */

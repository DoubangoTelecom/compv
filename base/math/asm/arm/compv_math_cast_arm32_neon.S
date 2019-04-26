#########################################################################
# Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__arm__) && !defined(__aarch64__)
.include "compv_common_arm32.S"

.data

.extern
 
.text

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* src,
@ arg(1) -> COMPV_ALIGNED(NEON) compv_float32_t* dst,
@ arg(2) -> const compv_uscalar_t width,
@ arg(3) -> const compv_uscalar_t height,
@ arg(4) -> COMPV_ALIGNED(NEON) const compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVMathCastProcess_static_8u32f_Asm_NEON32
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 5
	COMPV_GAS_SAVE_NEON_REGS

	@@ Load arguments @@
	ldm_args r0-r4
	src .req r0
	dst .req r1
	width .req r2
	height .req r3
	stride .req r4

    @@ Local variables @@
	stridef .req r5
    i .req r6

	pld [src, #(CACHE_LINE_SIZE*0)]
	pld [src, #(CACHE_LINE_SIZE*1)]
	pld [src, #(CACHE_LINE_SIZE*2)]
	pld [src, #(CACHE_LINE_SIZE*3)]

	@ Convert stride to padding
	add r11, width, #15
	and r11, r11, #-16
	sub stride, stride, r11

    @ Change stridef = sizeof(float) * stride
    lsl stridef, stride, #COMPV_GAS_FLOAT32_SHIFT_BYTES
	
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (compv_uscalar_t j = 0; j < height; ++j)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    LoopHeight_CompVMathCastProcess_static_8u32f_Asm_NEON32:
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (compv_uscalar_t i = 0; i < width; i += 16)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        mov i, #0
        LoopWidth_CompVMathCastProcess_static_8u32f_Asm_NEON32:
            vld1.u8 { q1 }, [src :128]!
			pld [src, #(CACHE_LINE_SIZE*4)]
			vmovl.u8 q3, q1y
			vmovl.u8 q1, q1x
			vmovl.u16 q0, q1x
			vmovl.u16 q1, q1y
			vmovl.u16 q2, q3x
			vmovl.u16 q3, q3y
			vcvt.f32.s32 q0, q0
			vcvt.f32.s32 q1, q1
			vcvt.f32.s32 q2, q2
			vcvt.f32.s32 q3, q3
			vst1.f32 { q0, q1 }, [dst :128]!
			vst1.f32 { q2, q3 }, [dst :128]!
            add i, i, #16
			cmp i, width
			blt LoopWidth_CompVMathCastProcess_static_8u32f_Asm_NEON32
        EndOf_LoopWidth_CompVMathCastProcess_static_8u32f_Asm_NEON32:

        subs height, height, #1
        add src, src, stride
        add dst, dst, stridef
        bne LoopHeight_CompVMathCastProcess_static_8u32f_Asm_NEON32
    EndOf_LoopHeight_CompVMathCastProcess_static_8u32f_Asm_NEON32:

    .unreq src
	.unreq dst
	.unreq width
	.unreq height
	.unreq stride

	.unreq stridef
    .unreq i

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 5
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const compv_float32_t* src,
@ arg(1) -> COMPV_ALIGNED(NEON) uint8_t* dst,
@ arg(2) -> const compv_uscalar_t width,
@ arg(3) -> const compv_uscalar_t height,
@ arg(4) -> COMPV_ALIGNED(NEON) const compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVMathCastProcess_static_pixel8_32f_Asm_NEON32
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 5
	COMPV_GAS_SAVE_NEON_REGS

	@@ Load arguments @@
	ldm_args r0-r4
	src .req r0
	dst .req r1
	width .req r2
	height .req r3
	stride .req r4

    @@ Local variables @@
	stridef .req r5
    i .req r6

	pld [src, #(CACHE_LINE_SIZE*0)]
	pld [src, #(CACHE_LINE_SIZE*1)]
	pld [src, #(CACHE_LINE_SIZE*2)]
	pld [src, #(CACHE_LINE_SIZE*3)]

	@ Convert stride to padding
	add r11, width, #15
	and r11, r11, #-16
	sub stride, stride, r11

    @ Change stridef = sizeof(float) * stride
    lsl stridef, stride, #COMPV_GAS_FLOAT32_SHIFT_BYTES
	
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (compv_uscalar_t j = 0; j < height; ++j)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    LoopHeight_CompVMathCastProcess_static_pixel8_32f_Asm_NEON32:
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (compv_uscalar_t i = 0; i < width; i += 16)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        mov i, #0
        LoopWidth_CompVMathCastProcess_static_pixel8_32f_Asm_NEON32:
            vld1.u8 { q0, q1 }, [src :128]!
			vld1.u8 { q2, q3 }, [src :128]!
			pld [src, #(CACHE_LINE_SIZE*4)]
			vcvt.s32.f32 q0, q0
			vcvt.s32.f32 q1, q1
			vcvt.s32.f32 q2, q2
			vcvt.s32.f32 q3, q3
			vqmovn.s32 q4x, q0
			vqmovn.s32 q4y, q1
			vqmovn.s32 q5x, q2
			vqmovn.s32 q5y, q3
			vqmovun.s16 q6x, q4
			vqmovun.s16 q6y, q5
			vst1.f32 { q6 }, [dst :128]!
            add i, i, #16
			cmp i, width
			blt LoopWidth_CompVMathCastProcess_static_pixel8_32f_Asm_NEON32
        EndOf_LoopWidth_CompVMathCastProcess_static_pixel8_32f_Asm_NEON32:

        subs height, height, #1
        add src, src, stridef
        add dst, dst, stride
        bne LoopHeight_CompVMathCastProcess_static_pixel8_32f_Asm_NEON32
    EndOf_LoopHeight_CompVMathCastProcess_static_pixel8_32f_Asm_NEON32:

    .unreq src
	.unreq dst
	.unreq width
	.unreq height
	.unreq stride

	.unreq stridef
    .unreq i

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 5
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__arm__) && !defined(__aarch64__) */

#########################################################################
# Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__arm__) && !defined(__aarch64__)
.include "compv_common_arm32.S"

.equ GRAD_X, 0
.equ GRAD_Y, 1
.equ OUTPUT_INT16, 2
.equ OUTPUT_FLOAT32, 3

.data

.extern
 
.text

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* input
@ arg(1) -> COMPV_ALIGNED(NEON) int16_t/float32_* output
@ arg(2) -> compv_uscalar_t width
@ arg(3) -> compv_uscalar_t height
@ arg(4) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
.macro CompVGradientFastGrad_8u_Macro_NEON32 grad_type, output_type
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 5
	COMPV_GAS_SAVE_NEON_REGS

	@@ Load arguments @@
	ldm_args r0-r4
	input .req r0
	output .req r1
	width .req r2
	height .req r3
	stride .req r4
	
    i .req r5
    inputPlus .req r6
    inputMinus .req r7
    strideOut .req r8

    .if \grad_type == GRAD_X
        sub inputMinus, input, #1
        add inputPlus, input, #1
    .elseif \grad_type == GRAD_Y
        sub inputMinus, input, stride
        add inputPlus, input, stride
    .else
        .error Not supported
    .endif

    @ Transform stride to padding @
	add r11, width, #15
	and r11, r11, #-16
	sub stride, stride, r11

    @ Transform strideOut unit from samples to bytes @
    .if \output_type == OUTPUT_INT16
        lsl strideOut, stride, #(COMPV_GAS_INT16_SHIFT_BYTES)
    .elseif \output_type == OUTPUT_FLOAT32
        lsl strideOut, stride, #(COMPV_GAS_FLOAT32_SHIFT_BYTES)
    .else
        .error Not supported
    .endif

    pld [inputMinus, #(CACHE_LINE_SIZE*0)]
	pld [inputMinus, #(CACHE_LINE_SIZE*1)]
	pld [inputMinus, #(CACHE_LINE_SIZE*2)]
    pld [inputMinus, #(CACHE_LINE_SIZE*3)]
    pld [inputPlus, #(CACHE_LINE_SIZE*0)]
	pld [inputPlus, #(CACHE_LINE_SIZE*1)]
	pld [inputPlus, #(CACHE_LINE_SIZE*2)]
    pld [inputPlus, #(CACHE_LINE_SIZE*3)]

    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (compv_uscalar_t j = 0; j < height; ++j)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    LoopHeight_CompVGradientFastGrad_8u_Macro_NEON32\@:
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (compv_uscalar_t i = 0; i < width; i += 16)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        mov i, #0
        LoopWidth_CompVGradientFastGrad_8u_Macro_NEON32\@:
            .if \grad_type == GRAD_X
                vld1.u8 { q0 }, [inputMinus]!
                vld1.u8 { q1 }, [inputPlus]!
            .elseif \grad_type == GRAD_Y
                vld1.u8 { q0 }, [inputMinus :128]!
                vld1.u8 { q1 }, [inputPlus :128]!
            .else
                .error Not supported
            .endif
            pld [inputMinus, #(CACHE_LINE_SIZE*4)]
            pld [inputPlus, #(CACHE_LINE_SIZE*4)]
            vsubl.u8 q2, q1x, q0x
            vsubl.u8 q3, q1y, q0y
            .if \output_type == OUTPUT_INT16
                vst1.u8 { q2, q3 }, [output :128]!
            .elseif \output_type == OUTPUT_FLOAT32
                vmovl.s16 q4, q2x
                vmovl.s16 q5, q2y
                vmovl.s16 q6, q3x
                vmovl.s16 q7, q3y
                vcvt.f32.s32 q4, q4
                vcvt.f32.s32 q5, q5
                vcvt.f32.s32 q6, q6
                vcvt.f32.s32 q7, q7
                vst1.f32 { q4, q5 }, [output :128]!
                vst1.f32 { q6, q7 }, [output :128]!
            .else
                .error Not supported
            .endif
            add i, i, #16
            cmp i, width
            blt LoopWidth_CompVGradientFastGrad_8u_Macro_NEON32\@
        EndOf_LoopWidth_CompVGradientFastGrad_8u_Macro_NEON32\@:

        subs height, height, #1
        add inputMinus, inputMinus, stride
        add inputPlus, inputPlus, stride
        add output, output, strideOut
        bne LoopHeight_CompVGradientFastGrad_8u_Macro_NEON32\@
    EndOf_LoopHeight_CompVGradientFastGrad_8u_Macro_NEON32\@:

    .unreq input
	.unreq output
	.unreq width
	.unreq height
	.unreq stride
	
    .unreq i
    .unreq inputPlus
    .unreq inputMinus
    .unreq strideOut

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 5
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVGradientFastGradX_8u16s_Asm_NEON32
	CompVGradientFastGrad_8u_Macro_NEON32 GRAD_X, OUTPUT_INT16

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVGradientFastGradX_8u32f_Asm_NEON32
	CompVGradientFastGrad_8u_Macro_NEON32 GRAD_X, OUTPUT_FLOAT32

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVGradientFastGradY_8u16s_Asm_NEON32
	CompVGradientFastGrad_8u_Macro_NEON32 GRAD_Y, OUTPUT_INT16

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVGradientFastGradY_8u32f_Asm_NEON32
	CompVGradientFastGrad_8u_Macro_NEON32 GRAD_Y, OUTPUT_FLOAT32

#endif /* defined(__arm__) && !defined(__aarch64__) */

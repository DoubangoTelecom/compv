#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__arm__) && !defined(__aarch64__)
.include "compv_common_arm32.S"

#if defined(__APPLE__)
#   define sym(funcname) _##funcname
#else
#   define sym(funcname) funcname
#endif

.data

.extern
 
.text

.equ yuyv422Family, 0
.equ uyvy422Family, 1

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* yuv422Ptr
@ arg(1) -> COMPV_ALIGNED(NEON) uint8_t* outYPtr
@ arg(2) -> compv_uscalar_t width
@ arg(3) -> compv_uscalar_t height
@ arg(4) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
.macro CompVImageConvYuv422family_to_y_Macro_NEON32 yuv422Family
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 5
	COMPV_GAS_SAVE_NEON_REGS
	
	ldm_args r0-r4
	yuv422Ptr .req r0
	outYPtr .req r1
	width .req r2
	height .req r3
	stride .req r4

	pld [yuv422Ptr, #(CACHE_LINE_SIZE*0)]
	pld [yuv422Ptr, #(CACHE_LINE_SIZE*1)]
	pld [yuv422Ptr, #(CACHE_LINE_SIZE*2)]

	add r5, width, #15
	and r5, r5, #-16
	sub r5, stride, r5
	padY .req r5
	mov r6, padY, LSL #1
	padYUV .req r6

	
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (j = 0; j < height; ++j)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopHeight\@:
		mov r9, #0
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (i = 0; i < width; i += 16)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		LoopWidth\@:
			add r9, r9, #16
			pld [yuv422Ptr, #(CACHE_LINE_SIZE*3)]
			vld1.u8 {q1x, q1y, q2x, q2y}, [yuv422Ptr :128]!

            .if \yuv422Family == yuyv422Family
                @ldr r7, =sym(kShuffleEpi8_Yuyv422ToYuv_i32)
                vuzp.u8 q1x, q1y
                vuzp.u8 q2x, q2y
                vswp q1y, q2x
                vst1.u8 {q1}, [outYPtr :128]! @ vstmia outYPtr!, {q1x, q1y}
            .elseif \yuv422Family == uyvy422Family
                vuzp.u8 q1x, q1y
                vuzp.u8 q2x, q2y
                vswp q1y, q2x
                vst1.u8 {q2}, [outYPtr :128]! @ vstmia outYPtr!, {q2x, q2y}
            .elseif
                .error Not implemented
            .endif

			cmp r9, width
			blt LoopWidth\@
			@End_of_LoopWidth@

		add outYPtr, outYPtr, padY
		add yuv422Ptr, yuv422Ptr, padYUV
		subs height, height, #1
		bne LoopHeight\@
		@End_of_LoopHeight@

	.unreq yuv422Ptr
	.unreq outYPtr
	.unreq width
	.unreq height
	.unreq stride
	.unreq padY
	.unreq padYUV

	
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 5
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVImageConvYuyv422_to_y_Asm_NEON32
	CompVImageConvYuv422family_to_y_Macro_NEON32 yuyv422Family

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVImageConvUyvy422_to_y_Asm_NEON32
	CompVImageConvYuv422family_to_y_Macro_NEON32 uyvy422Family

#endif /* defined(__arm__) && !defined(__aarch64__) */

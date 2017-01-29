#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__aarch64__)
.include "compv_common_arm64.S"

#if defined(__APPLE__)
#   define sym(funcname) _##funcname
#else
#   define sym(funcname) funcname
#endif

//.data
//kShuffleEpi8_Yuyv422ToYuv_i32: .word 0x6040200, 0xe0c0a08, 0xd090501, 0xf0b0703
//kShuffleEpi8_Uyvy422ToYuv_i32: .word 0x7050301, 0xf0d0b09, 0x4080c, 0x2060a0e

//.extern sym(kShuffleEpi8_Yuyv422ToYuv_i32), sym(kShuffleEpi8_Uyvy422ToYuv_i32)

.text

.equ yuyv422Family, 0
.equ uyvy422Family, 1

#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* yuv422Ptr
# arg(1) -> COMPV_ALIGNED(NEON) uint8_t* outYPtr
# arg(2) -> compv_uscalar_t width
# arg(3) -> compv_uscalar_t height
# arg(4) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
.macro CompVImageConvYuv422family_to_y_Macro_NEON64 yuv422Family
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 5
	COMPV_GAS_SAVE_NEON_REGS

	ldp_arg 0, r0, r1
    ldp_arg 2, r2, r3
    ldr_arg 4, r4
	yuv422Ptr .req r0
	outYPtr .req r1
	width .req r2
	height .req r3
	stride .req r4

    prfm pldl1keep, [yuv422Ptr, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [yuv422Ptr, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [yuv422Ptr, #(CACHE_LINE_SIZE*2)]

	add r5, width, #15
	and r5, r5, #-16
	sub r5, stride, r5
	padY .req r5
    lsl r6, padY, #1
	padYUV .req r6
	
	########################################
	# for (j = 0; j < height; ++j)
	########################################
	LoopHeight\@:
		mov r9, #0
		########################################
		# for (i = 0; i < width; i += 16)
		########################################
		LoopWidth\@:
			add r9, r9, #16
			prfm pldl1keep, [yuv422Ptr, #(CACHE_LINE_SIZE*3)]
			prfm pstl1keep, [outYPtr, #(CACHE_LINE_SIZE*3)]
			ld1 {v1.16b, v2.16b}, [yuv422Ptr], #32
			
            .if \yuv422Family == yuyv422Family
                uzp1 v3.16b, v1.16b, v2.16b
            .elseif \yuv422Family == uyvy422Family
                uzp2 v3.16b, v1.16b, v2.16b
            .elseif
                .error Not implemented
            .endif

			st1 {v3.16b}, [outYPtr], #16 // vstmia outYPtr!, {q3x, q3y}
			cmp r9, width
			blt LoopWidth\@
			#End_of_LoopWidth#

		add outYPtr, outYPtr, padY
		add yuv422Ptr, yuv422Ptr, padYUV
		subs height, height, #1
		bne LoopHeight\@
		#End_of_LoopHeight#

	.unreq yuv422Ptr
	.unreq outYPtr
	.unreq width
	.unreq height
	.unreq stride
	.unreq padY
	.unreq padYUV
	
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVImageConvYuyv422_to_y_Asm_NEON64
	CompVImageConvYuv422family_to_y_Macro_NEON64 yuyv422Family

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVImageConvUyvy422_to_y_Asm_NEON64
	CompVImageConvYuv422family_to_y_Macro_NEON64 uyvy422Family

#endif /* defined(__aarch64__) */

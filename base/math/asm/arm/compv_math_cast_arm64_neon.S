#########################################################################
# Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__aarch64__)
.include "compv_common_arm64.S" //

.data

.extern
 
.text

#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* src,
# arg(1) -> COMPV_ALIGNED(NEON) compv_float32_t* dst,
# arg(2) -> const compv_uscalar_t width,
# arg(3) -> const compv_uscalar_t height,
# arg(4) -> COMPV_ALIGNED(NEON) const compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVMathCastProcess_static_8u32f_Asm_NEON64
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Load arguments ##
	src .req r0
	dst .req r1
	width .req r2
	height .req r3
	stride .req r4

    ## Local variables ##
	stridef .req r5
    i .req r6

	prfm pldl1keep, [src, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [src, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [src, #(CACHE_LINE_SIZE*2)]
	prfm pldl1keep, [src, #(CACHE_LINE_SIZE*3)]

	# Convert stride to padding
	add r11, width, #15
	and r11, r11, #-16
	sub stride, stride, r11

    # Change stridef = sizeof(float) * stride
    lsl stridef, stride, #COMPV_GAS_FLOAT32_SHIFT_BYTES
	
    ###########################################################
    # for (compv_uscalar_t j = 0; j < height; ++j)
    ###########################################################
    LoopHeight_CompVMathCastProcess_static_8u32f_Asm_NEON64:
        ###########################################################
        # for (compv_uscalar_t i = 0; i < width; i += 16)
        ###########################################################
        mov i, #0
        LoopWidth_CompVMathCastProcess_static_8u32f_Asm_NEON64:
            ldr q1, [src], #(COMPV_GAS_Q_SZ_BYTES)
			prfm pldl1keep, [src, #(CACHE_LINE_SIZE*4)]
			uxtl2 v3.8h, v1.16b
			uxtl v1.8h, v1.8b
			uxtl v0.4s, v1.4h
			uxtl2 v1.4s, v1.8h
			uxtl v2.4s, v3.4h
			uxtl2 v3.4s, v3.8h
			ucvtf v0.4s, v0.4s
			ucvtf v1.4s, v1.4s
			ucvtf v2.4s, v2.4s
			ucvtf v3.4s, v3.4s
			stp q0, q1, [dst], #(2 * COMPV_GAS_Q_SZ_BYTES)
			stp q2, q3, [dst], #(2 * COMPV_GAS_Q_SZ_BYTES)
            add i, i, #16
			cmp i, width
			blt LoopWidth_CompVMathCastProcess_static_8u32f_Asm_NEON64
        EndOf_LoopWidth_CompVMathCastProcess_static_8u32f_Asm_NEON64:

        subs height, height, #1
        add src, src, stride
        add dst, dst, stridef
        bne LoopHeight_CompVMathCastProcess_static_8u32f_Asm_NEON64
    EndOf_LoopHeight_CompVMathCastProcess_static_8u32f_Asm_NEON64:

    .unreq src
	.unreq dst
	.unreq width
	.unreq height
	.unreq stride

	.unreq stridef
    .unreq i

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) const compv_float32_t* src,
# arg(1) -> COMPV_ALIGNED(NEON) uint8_t* dst,
# arg(2) -> const compv_uscalar_t width,
# arg(3) -> const compv_uscalar_t height,
# arg(4) -> COMPV_ALIGNED(NEON) const compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVMathCastProcess_static_pixel8_32f_Asm_NEON64
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Load arguments ##
	src .req r0
	dst .req r1
	width .req r2
	height .req r3
	stride .req r4

    ## Local variables ##
	stridef .req r5
    i .req r6

	prfm pldl1keep, [src, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [src, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [src, #(CACHE_LINE_SIZE*2)]
	prfm pldl1keep, [src, #(CACHE_LINE_SIZE*3)]

	# Convert stride to padding
	add r11, width, #15
	and r11, r11, #-16
	sub stride, stride, r11

    # Change stridef = sizeof(float) * stride
    lsl stridef, stride, #COMPV_GAS_FLOAT32_SHIFT_BYTES
	
    ###########################################################
    # for (compv_uscalar_t j = 0; j < height; ++j)
    ###########################################################
    LoopHeight_CompVMathCastProcess_static_pixel8_32f_Asm_NEON64:
        ###########################################################
        # for (compv_uscalar_t i = 0; i < width; i += 16)
        ###########################################################
        mov i, #0
        LoopWidth_CompVMathCastProcess_static_pixel8_32f_Asm_NEON64:
            ldp q0, q1, [src], #(2 * COMPV_GAS_Q_SZ_BYTES)
			ldp q2, q3, [src], #(2 * COMPV_GAS_Q_SZ_BYTES)
			prfm pldl1keep, [src, #(CACHE_LINE_SIZE*4)]
			fcvtzs v0.4s, v0.4s
			fcvtzs v1.4s, v1.4s
			fcvtzs v2.4s, v2.4s
			fcvtzs v3.4s, v3.4s
			sqxtn v4.4h, v0.4s
			sqxtn2 v4.8h, v1.4s
			sqxtn v5.4h, v2.4s
			sqxtn2 v5.8h, v3.4s
			sqxtun v6.8b, v4.8h
			sqxtun2 v6.16b, v5.8h
			str q6, [dst], #(1 * COMPV_GAS_Q_SZ_BYTES)
            add i, i, #16
			cmp i, width
			blt LoopWidth_CompVMathCastProcess_static_pixel8_32f_Asm_NEON64
        EndOf_LoopWidth_CompVMathCastProcess_static_pixel8_32f_Asm_NEON64:

        subs height, height, #1
        add src, src, stridef
        add dst, dst, stride
        bne LoopHeight_CompVMathCastProcess_static_pixel8_32f_Asm_NEON64
    EndOf_LoopHeight_CompVMathCastProcess_static_pixel8_32f_Asm_NEON64:

    .unreq src
	.unreq dst
	.unreq width
	.unreq height
	.unreq stride

	.unreq stridef
    .unreq i

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__aarch64__) */

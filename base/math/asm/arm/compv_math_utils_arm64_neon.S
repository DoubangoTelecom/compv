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

.data

.extern

.text

#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) const uint16_t* data
# arg(1) -> compv_uscalar_t width
# arg(2) -> compv_uscalar_t height
# arg(3) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
# arg(4) -> uint16_t *max
COMPV_GAS_FUNCTION_DECLARE CompVMathUtilsMax_16u_Asm_NEON64
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Load arguments ##
	data .req r0
	width .req r1
	height .req r2
	stride .req r3
	max .req r4

    i .req r5
    pad .req r6
    widthModulo32Div8 .req r7

    vecMax .req v5
    vecOrphansSuppress .req v6

    movi vecMax.8h, #0

    add pad, width, #7
    and pad, pad, #-8
    sub pad, stride, pad
    lsl pad, pad, #1 // convert from shorts to bytes

    and widthModulo32Div8, width, #31 // modulo 32
    lsr widthModulo32Div8, widthModulo32Div8, #3 // div 8

    ## compute vecOrphansSuppress for orphans ##
    ands r8, width, #7
    beq NoOrphans_CompVMathDistanceHamming_Asm_NEON64
        lsl r8, r8, #1 // convert to bytes
		cmeq vecOrphansSuppress.16b, vecOrphansSuppress.16b, vecOrphansSuppress.16b
		mov r9, #-(16<<3)
		mov r10, #0
		add r8, r9, r8, LSL #3 // ((orphans - 16) << 3) = (-16<<3) + (orphans << 3)
		adds r11, r8, #64
		csel r9, r11, r10, mi
		mov v15.d[0], r9
		mov v15.d[1], r8
		ushl vecOrphansSuppress.2d, vecOrphansSuppress.2d, v15.2d
		NoOrphans_CompVMathDistanceHamming_Asm_NEON64:

    #########################################################################
    # for (compv_uscalar_t j = 0; j < height; ++j)
    #########################################################################
    LoopHeight_CompVMathUtilsMax_16u_Asm_NEON64:
        #########################################################################
        # for (i = 0; i < widthSigned - 31; i += 32)
        #########################################################################
        lsr i, width, #5
        cbz i, EndOf_LoopWidth32_CompVMathUtilsMax_16u_Asm_NEON64
        LoopWidth32_CompVMathUtilsMax_16u_Asm_NEON64:
            prfm pldl1keep, [data, #(CACHE_LINE_SIZE*3)]
            subs i, i, #1
            ldp q0, q1, [data], #32
            ldp q2, q3, [data], #32
            umax v0.8h, v0.8h, v1.8h
            umax v2.8h, v2.8h, v3.8h
            umax vecMax.8h, vecMax.8h, v0.8h
            umax vecMax.8h, vecMax.8h, v2.8h
            bne LoopWidth32_CompVMathUtilsMax_16u_Asm_NEON64
            EndOf_LoopWidth32_CompVMathUtilsMax_16u_Asm_NEON64:
            ## EndOf_LoopWidth32_CompVMathUtilsMax_16u_Asm_NEON64 ##

        #########################################################################
        # for (; i < widthSigned - 7; i += 8)
        #########################################################################
        cbz widthModulo32Div8, EndOf_LoopWidth8_CompVMathUtilsMax_16u_Asm_NEON64
        mov i, widthModulo32Div8
        LoopWidth8_CompVMathUtilsMax_16u_Asm_NEON64:
            prfm pldl1keep, [data, #(CACHE_LINE_SIZE*3)]
            subs i, i, #1
            ldr q0, [data], #16
            umax vecMax.8h, vecMax.8h, v0.8h
            bne LoopWidth8_CompVMathUtilsMax_16u_Asm_NEON64
            EndOf_LoopWidth8_CompVMathUtilsMax_16u_Asm_NEON64:
            ## EndOf_LoopWidth8_CompVMathUtilsMax_16u_Asm_NEON64 ##

        #########################################################################
        # .if (orphans)
        #########################################################################
        tst width, #7
        beq EndOf_IfOrphansCompVMathUtilsMax_16u_Asm_NEON64
            ldr q0, [data], #16
            and v0.16b, v0.16b, vecOrphansSuppress.16b
            umax vecMax.8h, vecMax.8h, v0.8h
            EndOf_IfOrphansCompVMathUtilsMax_16u_Asm_NEON64:

        subs height, height, #1
        add data, data, pad
        bne LoopHeight_CompVMathUtilsMax_16u_Asm_NEON64
        ## EndOf_LoopHeight_CompVMathUtilsMax_16u_Asm_NEON64 ##

    umaxp vecMax.8h, vecMax.8h, vecMax.8h
    umaxp vecMax.4h, vecMax.4h, vecMax.4h
    umaxp vecMax.4h, vecMax.4h, vecMax.4h
    st1 {vecMax.h}[0], [max]
    
    .unreq data
	.unreq width
	.unreq height
	.unreq stride
	.unreq max

    .unreq i
    .unreq pad
    .unreq widthModulo32Div8

    .unreq vecMax
    .unreq vecOrphansSuppress

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__aarch64__) */

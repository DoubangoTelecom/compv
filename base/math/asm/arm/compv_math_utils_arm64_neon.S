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

	## Set arguments ##
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
            subs i, i, #1
            ld1 {v0.8h, v1.8h}, [data], #(2*COMPV_GAS_Q_SZ_BYTES)
            ld1 {v2.8h, v3.8h}, [data], #(2*COMPV_GAS_Q_SZ_BYTES)
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
            subs i, i, #1
            ld1 {v0.8h}, [data], #(1*COMPV_GAS_Q_SZ_BYTES)
            umax vecMax.8h, vecMax.8h, v0.8h
            bne LoopWidth8_CompVMathUtilsMax_16u_Asm_NEON64
            EndOf_LoopWidth8_CompVMathUtilsMax_16u_Asm_NEON64:
            ## EndOf_LoopWidth8_CompVMathUtilsMax_16u_Asm_NEON64 ##

        #########################################################################
        # .if (orphans)
        #########################################################################
        tst width, #7
        beq EndOf_IfOrphansCompVMathUtilsMax_16u_Asm_NEON64
            ldr q0, [data], #(1*COMPV_GAS_Q_SZ_BYTES)
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


#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* data
# arg(1) -> compv_uscalar_t width
# arg(2) -> compv_uscalar_t height
# arg(3) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
# arg(4) -> uint32_t *sum1
COMPV_GAS_FUNCTION_DECLARE CompVMathUtilsSum_8u32u_Asm_NEON64
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Set arguments ##
	data .req r0
	width .req r1
	height .req r2
	stride .req r3
	sum1 .req r4

    i .req r5
    widthModulo64Div16 .req r6
    pad .req r7

    vecOrphansSuppress .req v13
    vecSuml .req v14
    vecSumh .req v15

    ## compute vecOrphansSuppress for orphans ##
    ands r8, width, #15
    beq NoOrphans_CompVMathUtilsSum_8u32u_Asm_NEON64
		cmeq vecOrphansSuppress.16b, vecOrphansSuppress.16b, vecOrphansSuppress.16b
		mov r9, #-(16<<3)
		mov r10, #0
		add r8, r9, r8, LSL #3 // ((orphans - 16) << 3) = (-16<<3) + (orphans << 3)
		adds r11, r8, #64
		csel r9, r11, r10, mi
		mov v30.d[0], r9
		mov v30.d[1], r8
		ushl vecOrphansSuppress.2d, vecOrphansSuppress.2d, v30.2d
		NoOrphans_CompVMathUtilsSum_8u32u_Asm_NEON64:

    and widthModulo64Div16, width, #63
    lsr widthModulo64Div16, widthModulo64Div16, #4

    add pad, width, #15
    and pad, pad, #-16
    sub pad, stride, pad

    movi vecSuml.4s, #0
    movi vecSumh.4s, #0
	
    #########################################################################
    # for (compv_uscalar_t j = 0; j < height; ++j)
    #########################################################################
    LoopHeight_CompVMathUtilsSum_8u32u_Asm_NEON64:
        #########################################################################
        # for (i = 0; i < widthSigned - 63; i += 64)
        #########################################################################
        lsr i, width, #6 // div 64
        cbz i, EndOf_LoopWidth64_CompVMathUtilsSum_8u32u_Asm_NEON64
        LoopWidth64_CompVMathUtilsSum_8u32u_Asm_NEON64:
            prfm pldl1keep, [data, #(CACHE_LINE_SIZE*3)]
            subs i, i, #1
            ldp q0, q1, [data], #(2*COMPV_GAS_Q_SZ_BYTES)
            ldp q2, q3, [data], #(2*COMPV_GAS_Q_SZ_BYTES)
            uaddl v4.8h, v0.8b, v1.8b
            uaddl2 v5.8h, v0.16b, v1.16b
            uaddl v6.8h, v2.8b, v3.8b
            uaddl2 v7.8h, v2.16b, v3.16b
            add v4.8h, v4.8h, v5.8h
            add v6.8h, v6.8h, v7.8h
            uaddl v8.4s, v4.4h, v6.4h
            uaddl2 v9.4s, v4.8h, v6.8h            
            add vecSuml.4s, vecSuml.4s, v8.4s
            add vecSumh.4s, vecSumh.4s, v9.4s
            bne LoopWidth64_CompVMathUtilsSum_8u32u_Asm_NEON64
            EndOf_LoopWidth64_CompVMathUtilsSum_8u32u_Asm_NEON64:
            ## EndOf_LoopWidth64_CompVMathUtilsSum_8u32u_Asm_NEON64 ##

        #########################################################################
        # for (; i < widthSigned - 15; i += 16)
        #########################################################################
        mov i, widthModulo64Div16
        cbz i, EndOf_LoopWidth16_CompVMathUtilsSum_8u32u_Asm_NEON64
        LoopWidth16_CompVMathUtilsSum_8u32u_Asm_NEON64:
            subs i, i, #1
            ldr q0, [data], #(1*COMPV_GAS_Q_SZ_BYTES)
            uxtl v1.8h, v0.8b
            uxtl2 v2.8h, v0.16b
            add v1.8h, v1.8h, v2.8h
            uaddw vecSuml.4s, vecSuml.4s, v1.4h
            uaddw2 vecSumh.4s, vecSumh.4s, v1.8h
            bne LoopWidth16_CompVMathUtilsSum_8u32u_Asm_NEON64
            EndOf_LoopWidth16_CompVMathUtilsSum_8u32u_Asm_NEON64:
            ## EndOf_LoopWidth16_CompVMathUtilsSum_8u32u_Asm_NEON64 ##

        #########################################################################
        # .if (orphans)
        #########################################################################
        tst width, #15
        beq EndOf_IfOrphansCompVMathUtilsSum_8u32u_Asm_NEON64
            ldr q0, [data], #(1*COMPV_GAS_Q_SZ_BYTES)
            and v0.16b, v0.16b, vecOrphansSuppress.16b
            uxtl v1.8h, v0.8b
            uxtl2 v2.8h, v0.16b
            add v1.8h, v1.8h, v2.8h
            uaddw vecSuml.4s, vecSuml.4s, v1.4h
            uaddw2 vecSumh.4s, vecSumh.4s, v1.8h
            EndOf_IfOrphansCompVMathUtilsSum_8u32u_Asm_NEON64:

        subs height, height, #1
        add data, data, pad
        bne LoopHeight_CompVMathUtilsSum_8u32u_Asm_NEON64
        ## EndOf_LoopHeight_CompVMathUtilsSum_8u32u_Asm_NEON64 ##

    add vecSuml.4s, vecSuml.4s, vecSumh.4s
    addp vecSuml.4s, vecSuml.4s, vecSuml.4s
    addp vecSuml.4s, vecSuml.4s, vecSuml.4s
    st1 {vecSuml.s}[0], [sum1]

	.unreq data
    .unreq width
    .unreq height
    .unreq stride
    .unreq sum1

    .unreq i
    .unreq widthModulo64Div16
    .unreq pad

    .unreq vecOrphansSuppress
    .unreq vecSuml
    .unreq vecSumh

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) const int16_t* a
# arg(1) -> COMPV_ALIGNED(NEON) const int16_t* b
# arg(2) -> COMPV_ALIGNED(NEON) uint16_t* r
# arg(3) -> compv_uscalar_t width
# arg(4) -> compv_uscalar_t height
# arg(5) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVMathUtilsSumAbs_16s16u_Asm_NEON64
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Set arguments ##
	aPtr .req r0
	bPtr .req r1
	rPtr .req r2
	width .req r3
	height .req r4
	stride .req r5

    i .req r6
    pad .req r7
    widthModulo32Div8 .req r8
    widthDiv32 .req r9
	
    add pad, width, #7
    and pad, pad, #-8
    sub pad, stride, pad
    lsl pad, pad, #1 // convert from shorts to bytes

    and widthModulo32Div8, width, #31 // modulo 32
    add widthModulo32Div8, widthModulo32Div8, #7 // reading beyond the width (data must be strided) - no orphans
    lsr widthModulo32Div8, widthModulo32Div8, #3 // div 8

    lsr widthDiv32, width, #5

    #########################################################################
    # for (j = 0; j < height; ++j)
    #########################################################################
    LoopHeight_CompVMathUtilsSumAbs_16s16u_Asm_NEON64:
        #########################################################################
        # for (i = 0; i < widthSigned - 31; i += 32)
        #########################################################################
        lsr i, width, #5
        cbz i, EndOf_LoopWidth32_CompVMathUtilsSumAbs_16s16u_Asm_NEON64
        LoopWidth32_CompVMathUtilsSumAbs_16s16u_Asm_NEON64:
            prfm pldl1keep, [aPtr, #(CACHE_LINE_SIZE*3)]
            prfm pldl1keep, [bPtr, #(CACHE_LINE_SIZE*3)]
            subs i, i, #1
            ld1 {v0.8h, v1.8h}, [aPtr], #32
            ld1 {v2.8h, v3.8h}, [bPtr], #32
            abs v0.8h, v0.8h
            abs v1.8h, v1.8h
            abs v2.8h, v2.8h
            abs v3.8h, v3.8h
            ld1 {v4.8h, v5.8h}, [aPtr], #32
            ld1 {v6.8h, v7.8h}, [bPtr], #32
            abs v4.8h, v4.8h
            abs v5.8h, v5.8h
            abs v6.8h, v6.8h
            abs v7.8h, v7.8h
            uqadd v0.8h, v0.8h, v2.8h
            uqadd v1.8h, v1.8h, v3.8h
            uqadd v4.8h, v4.8h, v6.8h
            uqadd v5.8h, v5.8h, v7.8h
            st1 {v0.8h, v1.8h}, [rPtr], #32
            st1 {v4.8h, v5.8h}, [rPtr], #32
            bne LoopWidth32_CompVMathUtilsSumAbs_16s16u_Asm_NEON64
            EndOf_LoopWidth32_CompVMathUtilsSumAbs_16s16u_Asm_NEON64:
            ## EndOf_LoopWidth32_CompVMathUtilsSumAbs_16s16u_Asm_NEON64 ##

        #########################################################################
        # for (; i < widthSigned; i += 8)
        #########################################################################
        cbz widthModulo32Div8, EndOf_LoopWidth8_CompVMathUtilsSumAbs_16s16u_Asm_NEON64
        mov i, widthModulo32Div8
        LoopWidth8_CompVMathUtilsSumAbs_16s16u_Asm_NEON64:
            prfm pldl1keep, [aPtr, #(CACHE_LINE_SIZE*3)]
            prfm pldl1keep, [bPtr, #(CACHE_LINE_SIZE*3)]
            subs i, i, #1
            ld1 {v0.8h}, [aPtr], #16
            ld1 {v2.8h}, [bPtr], #16
            abs v0.8h, v0.8h
            abs v2.8h, v2.8h
            uqadd v0.8h, v0.8h, v2.8h
            st1 {v0.8h}, [rPtr], #16
            bne LoopWidth8_CompVMathUtilsSumAbs_16s16u_Asm_NEON64
            EndOf_LoopWidth8_CompVMathUtilsSumAbs_16s16u_Asm_NEON64:
            ## EndOf_LoopWidth8_CompVMathUtilsSumAbs_16s16u_Asm_NEON64 ##

        subs height, height, #1
        add rPtr, rPtr, pad
        add aPtr, aPtr, pad
        add bPtr, bPtr, pad
        bne LoopHeight_CompVMathUtilsSumAbs_16s16u_Asm_NEON64
        EndOf_LoopHeight_CompVMathUtilsSumAbs_16s16u_Asm_NEON64:
        ## EndOf_LoopHeight_CompVMathUtilsSumAbs_16s16u_Asm_NEON64 ##

	.unreq aPtr
    .unreq bPtr
    .unreq rPtr
    .unreq width
    .unreq height
    .unreq stride

    .unreq i
    .unreq pad
    .unreq widthModulo32Div8
    .unreq widthDiv32

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) const uint16_t* in
# arg(1) -> const compv_float32_t* scale1
# arg(2) -> COMPV_ALIGNED(NEON) uint8_t* out
# arg(3) -> compv_uscalar_t width
# arg(4) -> compv_uscalar_t height
# arg(5) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON64
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Set arguments ##
	in .req r0
	scale1 .req r1
	out .req r2
	width .req r3
	height .req r4
	stride .req r5

    pad .req r6
    i .req r7
    widthModulo32Div8 .req r8

    add pad, width, #7
    and pad, pad, #-8
    sub pad, stride, pad

    and widthModulo32Div8, width, #31 // modulo 32
    add widthModulo32Div8, widthModulo32Div8, #7 // reading beyond the width (data must be strided) - no orphans
    lsr widthModulo32Div8, widthModulo32Div8, #3 // div 8

    ldr r11w, [scale1]
    dup v0.4s, r11w

    #########################################################################
    # for (compv_uscalar_t j = 0; j < height; ++j)
    #########################################################################
    LoopHeight_CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON64:
        #########################################################################
        # for (i = 0; i < widthSigned - 31; i += 32)
        #########################################################################
        lsr i, width, #5
        cbz i, EndOf_LoopWidth32_CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON64
        LoopWidth32_CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON64:
            prfm pldl1keep, [in, #(CACHE_LINE_SIZE*3)]
            subs i, i, #1
            ld1 { v1.8h, v2.8h }, [in], #32
            ld1 { v3.8h, v4.8h }, [in], #32
            uxtl v5.4s, v1.4h
            uxtl2 v6.4s, v1.8h
            uxtl v7.4s, v2.4h
            uxtl2 v8.4s, v2.8h
            uxtl v9.4s, v3.4h
            uxtl2 v10.4s, v3.8h
            uxtl v11.4s, v4.4h
            uxtl2 v12.4s, v4.8h
            ucvtf v5.4s, v5.4s
            ucvtf v6.4s, v6.4s
            ucvtf v7.4s, v7.4s
            ucvtf v8.4s, v8.4s
            ucvtf v9.4s, v9.4s
            ucvtf v10.4s, v10.4s
            ucvtf v11.4s, v11.4s
            ucvtf v12.4s, v12.4s
            fmul v5.4s, v5.4s, v0.s[0]
            fmul v6.4s, v6.4s, v0.s[0]
            fmul v7.4s, v7.4s, v0.s[0]
            fmul v8.4s, v8.4s, v0.s[0]
            fmul v9.4s, v9.4s, v0.s[0]
            fmul v10.4s, v10.4s, v0.s[0]
            fmul v11.4s, v11.4s, v0.s[0]
            fmul v12.4s, v12.4s, v0.s[0]
            fcvtzu v5.4s, v5.4s
            fcvtzu v6.4s, v6.4s
            fcvtzu v7.4s, v7.4s
            fcvtzu v8.4s, v8.4s
            fcvtzu v9.4s, v9.4s
            fcvtzu v10.4s, v10.4s
            fcvtzu v11.4s, v11.4s
            fcvtzu v12.4s, v12.4s
            uqxtn v1.4h, v5.4s
            uqxtn v2.4h, v7.4s
            uqxtn v3.4h, v9.4s
            uqxtn v4.4h, v11.4s
            uqxtn2 v1.8h, v6.4s
            uqxtn2 v2.8h, v8.4s
            uqxtn2 v3.8h, v10.4s
            uqxtn2 v4.8h, v12.4s
            uqxtn v13.8b, v1.8h
            uqxtn v14.8b, v3.8h
            uqxtn2 v13.16b, v2.8h
            uqxtn2 v14.16b, v4.8h
            st1 { v13.16b, v14.16b }, [out], #32
            bne LoopWidth32_CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON64
            EndOf_LoopWidth32_CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON64:
            ## EndOf_LoopWidth32_CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON64 ##

        #########################################################################
        # for (; i < widthSigned; i += 8)
        #########################################################################
        cbz widthModulo32Div8, EndOf_LoopWidth8_CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON64
        mov i, widthModulo32Div8
        LoopWidth8_CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON64:
            prfm pldl1keep, [in, #(CACHE_LINE_SIZE*3)]
            subs i, i, #1
            ld1 { v1.8h }, [in], #16
            uxtl v5.4s, v1.4h
            uxtl2 v6.4s, v1.8h
            ucvtf v5.4s, v5.4s
            ucvtf v6.4s, v6.4s
            fmul v5.4s, v5.4s, v0.s[0]
            fmul v6.4s, v6.4s, v0.s[0]
            fcvtzu v5.4s, v5.4s
            fcvtzu v6.4s, v6.4s
            uqxtn v1.4h, v5.4s
            uqxtn2 v1.8h, v6.4s
            uqxtn v13.8b, v1.8h
            st1 { v13.8b }, [out], #8
            bne LoopWidth8_CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON64
            EndOf_LoopWidth8_CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON64:
            ## EndOf_LoopWidth8_CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON64 ##

        subs height, height, #1
        add in, in, pad, LSL #1 // uint16_t
        add out, out, pad // uint8_t
        bne LoopHeight_CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON64
        EndOf_CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON64:
        ## EndOf_CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON64 ##

    .unreq in
	.unreq scale1
	.unreq out
	.unreq width
	.unreq height
	.unreq stride

    .unreq pad
    .unreq i
    .unreq widthModulo32Div8

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__aarch64__) */

#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__arm__) && !defined(__aarch64__)
.include "compv_common_arm32.S" @

#if defined(__APPLE__)
#   define sym(funcname) _##funcname
#else
#   define sym(funcname) funcname
#endif

.data

.extern

.text

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const uint16_t* data
@ arg(1) -> compv_uscalar_t width
@ arg(2) -> compv_uscalar_t height
@ arg(3) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
@ arg(4) -> uint16_t *max
COMPV_GAS_FUNCTION_DECLARE CompVMathUtilsMax_16u_Asm_NEON32
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 5
	COMPV_GAS_SAVE_NEON_REGS

	@@ Load arguments @@
	ldm_args r0-r4
	data .req r0
	width .req r1
	height .req r2
	stride .req r3
	max .req r4

    i .req r5
    pad .req r6
    widthModulo32Div8 .req r7

    vecMax .req q5
    vecMaxx .req q5x
    vecMaxy .req q5y
    vecOrphansSuppress .req q6

    veor.u8 vecMax, vecMax, vecMax

    add pad, width, #7
    and pad, pad, #-8
    sub pad, stride, pad
    lsl pad, pad, #1 @ convert from shorts to bytes

    and widthModulo32Div8, width, #31 @ modulo 32
    lsr widthModulo32Div8, widthModulo32Div8, #3 @ div 8

    @@ compute vecOrphansSuppress for orphans @@
	ands r8, width, #7
    beq NoOrphans_CompVMathUtilsMax_16u_Asm_NEON32
        lsl r8, r8, #1 @ convert to bytes
		mov r9, #-(16<<3)
		vceq.u8 vecOrphansSuppress, vecOrphansSuppress, vecOrphansSuppress
		veor.u8 q15, q15, q15
		add r8, r9, r8, LSL #3 @ ((orphans - 16) << 3) = (-16<<3) + (orphans << 3)
		mov r9, #0
		cmp r8, #-64
		addlt r9, r8, #64 @ r9 = 0 if (t0 < -64) otherwise unchanged (#0)
		vmov.s32 q15y[0], r8
		vmov.s32 q15x[0], r9
		vshl.u64 vecOrphansSuppress, vecOrphansSuppress, q15
		NoOrphans_CompVMathUtilsMax_16u_Asm_NEON32:

    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (compv_uscalar_t j = 0; j < height; ++j)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    LoopHeight_CompVMathUtilsMax_16u_Asm_NEON32:
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (i = 0; i < widthSigned - 31; i += 32)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        lsrs i, width, #5
        beq EndOf_LoopWidth32_CompVMathUtilsMax_16u_Asm_NEON32
        LoopWidth32_CompVMathUtilsMax_16u_Asm_NEON32:
            pld [data, #(CACHE_LINE_SIZE*3)]
            subs i, i, #1
            vld1.u16 { q0, q1 }, [data :128]!
            vld1.u16 { q2, q3 }, [data :128]!
            vmax.u16 q0, q0, q1
            vmax.u16 q2, q2, q3
            vmax.u16 vecMax, vecMax, q0
            vmax.u16 vecMax, vecMax, q2
            bne LoopWidth32_CompVMathUtilsMax_16u_Asm_NEON32
            EndOf_LoopWidth32_CompVMathUtilsMax_16u_Asm_NEON32:
            @@ EndOf_LoopWidth32_CompVMathUtilsMax_16u_Asm_NEON32 @@

        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (; i < widthSigned - 7; i += 8)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        movs i, widthModulo32Div8
        beq EndOf_LoopWidth8_CompVMathUtilsMax_16u_Asm_NEON32
        LoopWidth8_CompVMathUtilsMax_16u_Asm_NEON32:
            pld [data, #(CACHE_LINE_SIZE*3)]
            subs i, i, #1
            vld1.u16 { q0 }, [data :128]!
            vmax.u16 vecMax, vecMax, q0
            bne LoopWidth8_CompVMathUtilsMax_16u_Asm_NEON32
            EndOf_LoopWidth8_CompVMathUtilsMax_16u_Asm_NEON32:
            @@ EndOf_LoopWidth8_CompVMathUtilsMax_16u_Asm_NEON32 @@

        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ if (orphans)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        tst width, #7
        beq EndOf_IfOrphansCompVMathUtilsMax_16u_Asm_NEON32
            vld1.u16 { q0 }, [data :128]!
            vand.u8 q0, q0, vecOrphansSuppress
            vmax.u16 vecMax, vecMax, q0
            EndOf_IfOrphansCompVMathUtilsMax_16u_Asm_NEON32:

        subs height, height, #1
        add data, data, pad
        bne LoopHeight_CompVMathUtilsMax_16u_Asm_NEON32
        @@ EndOf_LoopHeight_CompVMathUtilsMax_16u_Asm_NEON32 @@

    vpmax.u16 vecMaxx, vecMaxx, vecMaxy @ 8 -> 4
    vpmax.u16 vecMaxx, vecMaxx, vecMaxx @ 4 -> 2
    vpmax.u16 vecMaxx, vecMaxx, vecMaxx @ 2 -> 1
    vmov.u16 r11, vecMaxx[0]
    strh r11, [max]
    
    .unreq data
	.unreq width
	.unreq height
	.unreq stride
	.unreq max

    .unreq i
    .unreq pad
    .unreq widthModulo32Div8

    .unreq vecMax
    .unreq vecMaxx
    .unreq vecMaxy
    .unreq vecOrphansSuppress

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 5
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* data
@ arg(1) -> compv_uscalar_t width
@ arg(2) -> compv_uscalar_t height
@ arg(3) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
@ arg(4) -> uint32_t *sum1
COMPV_GAS_FUNCTION_DECLARE CompVMathUtilsSum_8u32u_Asm_NEON32
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 5
	COMPV_GAS_SAVE_NEON_REGS

	@@ Load arguments @@
	ldm_args r0-r4
	data .req r0
	width .req r1
	height .req r2
	stride .req r3
	sum1 .req r4

    i .req r5
    widthModulo64Div16 .req r6
    pad .req r7

    vecOrphansSuppress .req q13
    vecSuml .req q14
    vecSumlx .req q14x
    vecSumly .req q14y
    vecSumh .req q15

    @@ compute vecOrphansSuppress for orphans @@
	ands r8, width, #15
    beq NoOrphans_CompVMathUtilsSum_8u32u_Asm_NEON32
		mov r9, #-(16<<3)
		vceq.u8 vecOrphansSuppress, vecOrphansSuppress, vecOrphansSuppress
		veor.u8 q15, q15, q15
		add r8, r9, r8, LSL #3 @ ((orphans - 16) << 3) = (-16<<3) + (orphans << 3)
		mov r9, #0
		cmp r8, #-64
		addlt r9, r8, #64 @ r9 = 0 if (t0 < -64) otherwise unchanged (#0)
		vmov.s32 q15y[0], r8
		vmov.s32 q15x[0], r9
		vshl.u64 vecOrphansSuppress, vecOrphansSuppress, q15
		NoOrphans_CompVMathUtilsSum_8u32u_Asm_NEON32:


    and widthModulo64Div16, width, #63
    lsr widthModulo64Div16, widthModulo64Div16, #4

    add pad, width, #15
    and pad, pad, #-16
    sub pad, stride, pad

    veor.u8 vecSuml, vecSuml, vecSuml
    veor.u8 vecSumh, vecSumh, vecSumh
	
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (compv_uscalar_t j = 0; j < height; ++j)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    LoopHeight_CompVMathUtilsSum_8u32u_Asm_NEON32:
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (i = 0; i < widthSigned - 63; i += 64)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        lsrs i, width, #6 @ div 64
        beq EndOf_LoopWidth64_CompVMathUtilsSum_8u32u_Asm_NEON32
        LoopWidth64_CompVMathUtilsSum_8u32u_Asm_NEON32:
            pld [data, #(CACHE_LINE_SIZE*3)]
            subs i, i, #1
            vld1.u8 { q0, q1 }, [data :128]!
            vld1.u8 { q2, q3 }, [data :128]!
            vaddl.u8 q0, q0x, q0y
            vaddl.u8 q1, q1x, q1y
            vaddl.u8 q2, q2x, q2y
            vaddl.u8 q3, q3x, q3y
            vadd.u16 q0, q0, q1
            vadd.u16 q2, q2, q3
            vaddl.u16 q0, q0x, q0y
            vaddl.u16 q2, q2x, q2y
            vadd.u32 vecSuml, vecSuml, q0
            vadd.u32 vecSumh, vecSumh, q2
            bne LoopWidth64_CompVMathUtilsSum_8u32u_Asm_NEON32
            EndOf_LoopWidth64_CompVMathUtilsSum_8u32u_Asm_NEON32:
            @@ EndOf_LoopWidth64_CompVMathUtilsSum_8u32u_Asm_NEON32 @@

        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (; i < widthSigned - 15; i += 16)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        movs i, widthModulo64Div16
        beq EndOf_LoopWidth16_CompVMathUtilsSum_8u32u_Asm_NEON32
        LoopWidth16_CompVMathUtilsSum_8u32u_Asm_NEON32:
            subs i, i, #1
            vld1.u8 { q0 }, [data :128]!
            vaddl.u8 q0, q0x, q0y
            vaddl.u16 q0, q0x, q0y
            vadd.u32 vecSuml, vecSuml, q0
            bne LoopWidth16_CompVMathUtilsSum_8u32u_Asm_NEON32
            EndOf_LoopWidth16_CompVMathUtilsSum_8u32u_Asm_NEON32:
            @@ EndOf_LoopWidth16_CompVMathUtilsSum_8u32u_Asm_NEON32 @@

        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ if (orphans)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        tst width, #15
        beq EndOf_IfOrphansCompVMathUtilsSum_8u32u_Asm_NEON32
            vld1.u8 { q0 }, [data :128]!
            vand.u8 q0, q0, vecOrphansSuppress
            vaddl.u8 q0, q0x, q0y
            vaddl.u16 q0, q0x, q0y
            vadd.u32 vecSuml, vecSuml, q0
            EndOf_IfOrphansCompVMathUtilsSum_8u32u_Asm_NEON32:

        subs height, height, #1
        add data, data, pad
        bne LoopHeight_CompVMathUtilsSum_8u32u_Asm_NEON32
        @@ EndOf_LoopHeight_CompVMathUtilsSum_8u32u_Asm_NEON32 @@

    vadd.u32 vecSuml, vecSuml, vecSumh
    vadd.u32 vecSumlx, vecSumlx, vecSumly
    vpadd.u32 vecSumlx, vecSumlx, vecSumlx
    vmov.u32 r11, vecSumlx[0]
    str r11, [sum1]

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
    .unreq vecSumlx
    .unreq vecSumly
    .unreq vecSumh

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 5
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const int32_t* aPtr
@ arg(1) -> COMPV_ALIGNED(NEON) const int32_t* bPtr
@ arg(2) -> COMPV_ALIGNED(NEON) int32_t* sPtr
@ arg(3) -> compv_uscalar_t width
@ arg(4) -> compv_uscalar_t height
@ arg(5) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVMathUtilsSum2_32s32s_Asm_NEON32
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 6
	COMPV_GAS_SAVE_NEON_REGS
	@@ end prolog @@

    @ Load arguments @
	ldm_args r0-r5

    aPtr .req r0
	bPtr .req r1
	sPtr .req r2
	width .req r3
	height .req r4
    stride .req r5
    i .req r6
    pad .req r7

    add pad, width, #3
    and pad, pad, #-4
    sub pad, stride, pad
    lsl pad, pad, #2 @ convert to bytes

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (j = 0; j < height; ++j)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopRows_CompVMathUtilsSum2_32s32s_Asm_NEON32:
		ands i, width, #-16
		beq EndOf_LoopCols16_CompVMathUtilsSum2_32s32s_Asm_NEON32
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (i = 0; i < width_ - 15; i += 16) 
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		LoopCols16_CompVMathUtilsSum2_32s32s_Asm_NEON32:
			vld1.s32 {q0, q1}, [aPtr :128]!
            vld1.s32 {q2, q3}, [aPtr :128]!
            vld1.s32 {q4, q5}, [bPtr :128]!
            vld1.s32 {q6, q7}, [bPtr :128]!
            vadd.s32 q0, q0, q4
            vadd.s32 q1, q1, q5
            vadd.s32 q2, q2, q6
            vadd.s32 q3, q3, q7
			vst1.s32 {q0, q1}, [sPtr :128]!
            vst1.s32 {q2, q3}, [sPtr :128]!
            subs i, i, #16
			bne LoopCols16_CompVMathUtilsSum2_32s32s_Asm_NEON32
		EndOf_LoopCols16_CompVMathUtilsSum2_32s32s_Asm_NEON32:
		
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (; i < width_; i += 4)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        ands i, width, #15
		beq EndOf_LoopCols4_CompVMathUtilsSum2_32s32s_Asm_NEON32
		LoopCols4_CompVMathUtilsSum2_32s32s_Asm_NEON32:
			vld1.s32 {q0}, [aPtr :128]!
            vld1.s32 {q4}, [bPtr :128]!
            vadd.s32 q0, q0, q4
            vst1.s32 {q0}, [sPtr :128]!
            subs i, i, #4	
			bgt LoopCols4_CompVMathUtilsSum2_32s32s_Asm_NEON32
		EndOf_LoopCols4_CompVMathUtilsSum2_32s32s_Asm_NEON32:
		
		subs height, height, #1
		add aPtr, aPtr, pad
		add bPtr, bPtr, pad
		add sPtr, sPtr, pad
		bne LoopRows_CompVMathUtilsSum2_32s32s_Asm_NEON32

    .unreq aPtr
	.unreq bPtr
	.unreq sPtr
	.unreq width
	.unreq height
    .unreq stride
    .unreq i
    .unreq pad

	@@ begin epilog @@
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 6
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const int32_t* aPtr
@ arg(1) -> COMPV_ALIGNED(NEON) const int32_t* bPtr
@ arg(2) -> COMPV_ALIGNED(NEON) int32_t* bPtr
@ arg(3) -> compv_uscalar_t width
@ arg(4) -> compv_uscalar_t height
@ arg(5) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVMathUtilsSum2_32s32s_256x1_Asm_NEON32
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 6
	COMPV_GAS_SAVE_NEON_REGS
	@@ end prolog @@

    @ Load arguments @
	ldm_args r0-r2

    aPtr .req r0
	bPtr .req r1
	sPtr .req r2

    .rep 8
        vld1.s32 {q0, q1}, [aPtr :128]!
        vld1.s32 {q2, q3}, [aPtr :128]!
        vld1.s32 {q4, q5}, [aPtr :128]!
        vld1.s32 {q6, q7}, [aPtr :128]!
        vld1.s32 {q8, q9}, [bPtr :128]!
        vld1.s32 {q10, q11}, [bPtr :128]!
        vld1.s32 {q12, q13}, [bPtr :128]!
        vld1.s32 {q14, q15}, [bPtr :128]!
        vadd.s32 q0, q0, q8
        vadd.s32 q1, q1, q9
        vadd.s32 q2, q2, q10
        vadd.s32 q3, q3, q11
        vadd.s32 q4, q4, q12
        vadd.s32 q5, q5, q13
        vadd.s32 q6, q6, q14
        vadd.s32 q7, q7, q15
        vst1.s32 {q0, q1}, [sPtr :128]!
        vst1.s32 {q2, q3}, [sPtr :128]!
        vst1.s32 {q4, q5}, [sPtr :128]!
        vst1.s32 {q6, q7}, [sPtr :128]!
    .endr

    .unreq aPtr
	.unreq bPtr
	.unreq sPtr

	@@ begin epilog @@
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 6
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const int16_t* a
@ arg(1) -> COMPV_ALIGNED(NEON) const int16_t* b
@ arg(2) -> COMPV_ALIGNED(NEON) uint16_t* r
@ arg(3) -> compv_uscalar_t width
@ arg(4) -> compv_uscalar_t height
@ arg(5) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVMathUtilsSumAbs_16s16u_Asm_NEON32
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 6
	COMPV_GAS_SAVE_NEON_REGS

	@@ Load arguments @@
	ldm_args r0-r5
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
    lsl pad, pad, #1 @ convert from shorts to bytes

    and widthModulo32Div8, width, #31 @ modulo 32
    add widthModulo32Div8, widthModulo32Div8, #7 @ reading beyond the width (data must be strided) - no orphans
    lsr widthModulo32Div8, widthModulo32Div8, #3 @ div 8

    lsr widthDiv32, width, #5

    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (j = 0; j < height; ++j)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    LoopHeight_CompVMathUtilsSumAbs_16s16u_Asm_NEON32:
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (i = 0; i < widthSigned - 31; i += 32)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        lsrs i, width, #5
        beq EndOf_LoopWidth32_CompVMathUtilsSumAbs_16s16u_Asm_NEON32
        LoopWidth32_CompVMathUtilsSumAbs_16s16u_Asm_NEON32:
            pld [aPtr, #(CACHE_LINE_SIZE*3)]
            pld [bPtr, #(CACHE_LINE_SIZE*3)]
            subs i, i, #1
            vld1.s16 { q0, q1 }, [aPtr :128]!
            vld1.s16 { q2, q3 }, [bPtr :128]!
            vabs.s16 q0, q0
            vabs.s16 q1, q1
            vabs.s16 q2, q2
            vabs.s16 q3, q3
            vld1.s16 { q4, q5 }, [aPtr :128]!
            vld1.s16 { q6, q7 }, [bPtr :128]!
            vabs.s16 q4, q4
            vabs.s16 q5, q5
            vabs.s16 q6, q6
            vabs.s16 q7, q7
            vqadd.u16 q0, q0, q2
            vqadd.u16 q1, q1, q3
            vqadd.u16 q4, q4, q6
            vqadd.u16 q5, q5, q7
            vst1.u16 { q0, q1 }, [rPtr :128]!
            vst1.u16 { q4, q5 }, [rPtr :128]!
            bne LoopWidth32_CompVMathUtilsSumAbs_16s16u_Asm_NEON32
            EndOf_LoopWidth32_CompVMathUtilsSumAbs_16s16u_Asm_NEON32:
            @@ EndOf_LoopWidth32_CompVMathUtilsSumAbs_16s16u_Asm_NEON32 @@

        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (; i < widthSigned; i += 8)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        movs i, widthModulo32Div8
        beq EndOf_LoopWidth8_CompVMathUtilsSumAbs_16s16u_Asm_NEON32
        LoopWidth8_CompVMathUtilsSumAbs_16s16u_Asm_NEON32:
            pld [aPtr, #(CACHE_LINE_SIZE*3)]
            pld [bPtr, #(CACHE_LINE_SIZE*3)]
            subs i, i, #1
            vld1.s16 { q0 }, [aPtr :128]!
            vld1.s16 { q2 }, [bPtr :128]!
            vabs.s16 q0, q0
            vabs.s16 q2, q2
            vqadd.u16 q0, q0, q2
            vst1.u16 { q0 }, [rPtr :128]!
            bne LoopWidth8_CompVMathUtilsSumAbs_16s16u_Asm_NEON32
            EndOf_LoopWidth8_CompVMathUtilsSumAbs_16s16u_Asm_NEON32:
            @@ EndOf_LoopWidth8_CompVMathUtilsSumAbs_16s16u_Asm_NEON32 @@

        subs height, height, #1
        add rPtr, rPtr, pad
        add aPtr, aPtr, pad
        add bPtr, bPtr, pad
        bne LoopHeight_CompVMathUtilsSumAbs_16s16u_Asm_NEON32
        EndOf_LoopHeight_CompVMathUtilsSumAbs_16s16u_Asm_NEON32:
        @@ EndOf_LoopHeight_CompVMathUtilsSumAbs_16s16u_Asm_NEON32 @@



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
	COMPV_GAS_UNSHADOW_ARGS 6
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const uint16_t* in
@ arg(1) -> const compv_float32_t* scale1
@ arg(2) -> COMPV_ALIGNED(NEON) uint8_t* out
@ arg(3) -> compv_uscalar_t width
@ arg(4) -> compv_uscalar_t height
@ arg(5) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON32
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 6
	COMPV_GAS_SAVE_NEON_REGS

	@@ Load arguments @@
	ldm_args r0-r5
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

    and widthModulo32Div8, width, #31 @ modulo 32
    add widthModulo32Div8, widthModulo32Div8, #7 @ reading beyond the width (data must be strided) - no orphans
    lsr widthModulo32Div8, widthModulo32Div8, #3 @ div 8

    ldr r11, [scale1]
    vdup.f32 q0, r11

    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (compv_uscalar_t j = 0; j < height; ++j)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    LoopHeight_CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON32:
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (i = 0; i < widthSigned - 31; i += 32)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        lsrs i, width, #5
        beq EndOf_LoopWidth32_CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON32
        LoopWidth32_CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON32:
            pld [in, #(CACHE_LINE_SIZE*3)]
            subs i, i, #1
            vld1.s16 { q1, q2 }, [in :128]!
            vld1.s16 { q3, q4 }, [in :128]!
            vmovl.u16 q5, q1x
            vmovl.u16 q6, q1y
            vmovl.u16 q7, q2x
            vmovl.u16 q8, q2y
            vmovl.u16 q9, q3x
            vmovl.u16 q10, q3y
            vmovl.u16 q11, q4x
            vmovl.u16 q12, q4y
            vcvtq.f32.u32 q5, q5
            vcvtq.f32.u32 q6, q6
            vcvtq.f32.u32 q7, q7
            vcvtq.f32.u32 q8, q8
            vcvtq.f32.u32 q9, q9
            vcvtq.f32.u32 q10, q10
            vcvtq.f32.u32 q11, q11
            vcvtq.f32.u32 q12, q12
            vmul.f32 q5, q5, q0x[0]
            vmul.f32 q6, q6, q0x[0]
            vmul.f32 q7, q7, q0x[0]
            vmul.f32 q8, q8, q0x[0]
            vmul.f32 q9, q9, q0x[0]
            vmul.f32 q10, q10, q0x[0]
            vmul.f32 q11, q11, q0x[0]
            vmul.f32 q12, q12, q0x[0]
            vcvtq.u32.f32 q5, q5
            vcvtq.u32.f32 q6, q6
            vcvtq.u32.f32 q7, q7
            vcvtq.u32.f32 q8, q8
            vcvtq.u32.f32 q9, q9
            vcvtq.u32.f32 q10, q10
            vcvtq.u32.f32 q11, q11
            vcvtq.u32.f32 q12, q12
            vqmovn.u32 q1x, q5
            vqmovn.u32 q1y, q6
            vqmovn.u32 q2x, q7
            vqmovn.u32 q2y, q8
            vqmovn.u32 q3x, q9
            vqmovn.u32 q3y, q10
            vqmovn.u32 q4x, q11
            vqmovn.u32 q4y, q12
            vqmovn.u16 q13x, q1
            vqmovn.u16 q13y, q2
            vqmovn.u16 q14x, q3
            vqmovn.u16 q14y, q4            
            vst1.u16 { q13, q14 }, [out :128]!
            bne LoopWidth32_CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON32
            EndOf_LoopWidth32_CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON32:
            @@ EndOf_LoopWidth32_CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON32 @@

        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (; i < widthSigned; i += 8)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        movs i, widthModulo32Div8
        beq EndOf_LoopWidth8_CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON32
        LoopWidth8_CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON32:
            pld [in, #(CACHE_LINE_SIZE*3)]
            subs i, i, #1
            vld1.s16 { q1 }, [in :128]!
            vmovl.u16 q5, q1x
            vmovl.u16 q6, q1y
            vcvtq.f32.u32 q5, q5
            vcvtq.f32.u32 q6, q6
            vmul.f32 q5, q5, q0x[0]
            vmul.f32 q6, q6, q0x[0]
            vcvtq.u32.f32 q5, q5
            vcvtq.u32.f32 q6, q6
            vqmovn.u32 q1x, q5
            vqmovn.u32 q1y, q6
            vqmovn.u16 q13x, q1
            vst1.u16 { q13x }, [out :64]!
            bne LoopWidth8_CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON32
            EndOf_LoopWidth8_CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON32:
            @@ EndOf_LoopWidth8_CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON32 @@

        subs height, height, #1
        add in, in, pad, LSL #1 @ uint16_t
        add out, out, pad @ uint8_t
        bne LoopHeight_CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON32
        EndOf_CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON32:
        @@ EndOf_CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_NEON32 @@

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
	COMPV_GAS_UNSHADOW_ARGS 6
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__arm__) && !defined(__aarch64__) */

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

    vpmax.u16 vecMaxx, vecMaxx, vecMaxy
    vpmax.u16 vecMaxx, vecMaxx, vecMaxx
    vmov.u16 r11, vecMaxx[0]
    str r11, [max]
    
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
    add widthModulo32Div8, widthModulo32Div8, #7 @ reading beyond the width (data must be strided)
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


#endif /* defined(__arm__) && !defined(__aarch64__) */

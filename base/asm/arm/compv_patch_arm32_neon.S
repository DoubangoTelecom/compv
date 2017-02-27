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
@ arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* top
@ arg(1) -> COMPV_ALIGNED(NEON) const uint8_t* bottom
@ arg(2) -> COMPV_ALIGNED(NEON) const int16_t* x
@ arg(3) -> COMPV_ALIGNED(NEON) const int16_t* y
@ arg(4) -> compv_uscalar_t count
@ arg(5) -> compv_scalar_t* sum01
@ arg(6) -> compv_scalar_t* sum10 - s10 is reserved register name
COMPV_GAS_FUNCTION_DECLARE CompVPatchMoments0110_Asm_NEON32
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 7
	COMPV_GAS_SAVE_NEON_REGS

	@@ Load arguments @@
	ldm_args r0-r6
	top .req r0
	bottom .req r1
	x .req r2
	y .req r3
	count .req r4
	sum01 .req r5
	sum10 .req r6
	
    i .req r7

    veor.u8 q8, q8, q8
    veor.u8 q9, q9, q9
    veor.u8 q10, q10, q10
    veor.u8 q11, q11, q11
    veor.u8 q12, q12, q12
    veor.u8 q13, q13, q13
    veor.u8 q14, q14, q14
    veor.u8 q15, q15, q15

    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (compv_uscalar_t i = 0; i < count; i += 16)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    add i, count, #15
    and i, i, #-16
    LoopCount_CompVPatchMoments0110_Asm_NEON32:
        vld1.u8 { q0 }, [top :128]!
        vld1.u8 { q1 }, [bottom :128]!
        vaddl.u8 q2, q0x, q1x
        vaddl.u8 q3, q0y, q1y
        vsubl.u8 q4, q0x, q1x
        vsubl.u8 q5, q0y, q1y
        vld1.u8 { q6, q7 }, [y :128]!
        vld1.u8 { q0, q1 }, [x :128]!
        subs i, i, #16
        vmlal.s16 q8, q2x, q0x
        vmlal.s16 q9, q2y, q0y
        vmlal.s16 q10, q3x, q1x
        vmlal.s16 q11, q3y, q1y
        vmlal.s16 q12, q4x, q6x
        vmlal.s16 q13, q4y, q6y
        vmlal.s16 q14, q5x, q7x
        vmlal.s16 q15, q5y, q7y
        bne LoopCount_CompVPatchMoments0110_Asm_NEON32
        @@ EndOf_LoopCount_CompVPatchMoments0110_Asm_NEON32 @@

    vadd.s32 q8, q8, q9
    vadd.s32 q10, q10, q11
    vadd.s32 q12, q12, q13
    vadd.s32 q14, q14, q15
    vadd.s32 q8, q8, q10
    vadd.s32 q12, q12, q14

    vpadd.s32 q0x, q8x, q8y
    vpadd.s32 q1x, q12x, q12y
    vpadd.s32 q2x, q0x, q1x

    vmov.s32 r8, q2x[0]
    vmov.s32 r9, q2x[1]
    ldr r10, [sum10]
    ldr r11, [sum01]
    add r10, r10, r8
    add r11, r11, r9
    str r10, [sum10]
    str r11, [sum01]

    .unreq top
	.unreq bottom
	.unreq x
	.unreq y
	.unreq count
	.unreq s01
	.unreq s10

    .unreq i


	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 7
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#endif /* defined(__arm__) && !defined(__aarch64__) */

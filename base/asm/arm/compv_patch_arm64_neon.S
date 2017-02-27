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
# arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* top
# arg(1) -> COMPV_ALIGNED(NEON) const uint8_t* bottom
# arg(2) -> COMPV_ALIGNED(NEON) const int16_t* x
# arg(3) -> COMPV_ALIGNED(NEON) const int16_t* y
# arg(4) -> compv_uscalar_t count
# arg(5) -> compv_scalar_t* sum01
# arg(6) -> compv_scalar_t* sum10 - s10 is reserved register name
COMPV_GAS_FUNCTION_DECLARE CompVPatchMoments0110_Asm_NEON64
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Load arguments ##
	top .req r0
	bottom .req r1
	x .req r2
	y .req r3
	count .req r4
	sum01 .req r5
	sum10 .req r6
	
    i .req r7

    movi v8.4s, #0
    movi v9.4s, #0
    movi v10.4s, #0
    movi v11.4s, #0
    movi v12.4s, #0
    movi v13.4s, #0
    movi v14.4s, #0
    movi v15.4s, #0

    ###########################################################
    # for (compv_uscalar_t i = 0; i < count; i += 16)
    ###########################################################
    add i, count, #15
    and i, i, #-16
    LoopCount_CompVPatchMoments0110_Asm_NEON64:
        ldr q0, [top], #16
        ldr q1, [bottom], #16
        ldp q16, q17, [y], #32
        ldp q18, q19, [x], #32
        subs i, i, #16
        uaddl v2.8h, v0.8b, v1.8b
        uaddl2 v3.8h, v0.16b, v1.16b
        usubl v4.8h, v0.8b, v1.8b
        usubl2 v5.8h, v0.16b, v1.16b
        smlal v8.4s, v2.4h, v18.4h
        smlal2 v9.4s, v2.8h, v18.8h
        smlal v10.4s, v3.4h, v19.4h
        smlal2 v11.4s, v3.8h, v19.8h
        smlal v12.4s, v4.4h, v16.4h
        smlal2 v13.4s, v4.8h, v16.8h
        smlal v14.4s, v5.4h, v17.4h
        smlal2 v15.4s, v5.8h, v17.8h
        bne LoopCount_CompVPatchMoments0110_Asm_NEON64
        ## EndOf_LoopCount_CompVPatchMoments0110_Asm_NEON64 ##

    add v8.4s, v8.4s, v9.4s
    add v10.4s, v10.4s, v11.4s
    add v12.4s, v12.4s, v13.4s
    add v14.4s, v14.4s, v15.4s
    add v8.4s, v8.4s, v10.4s
    add v12.4s, v12.4s, v14.4s

    addp v0.4s, v8.4s, v12.4s
    addp v2.4s, v0.4s, v0.4s

    mov r8w, v2.s[0]
    mov r9w, v2.s[1]
    ldr r10, [sum10]
    ldr r11, [sum01]
    add r10, r10, r8w, SXTW
    add r11, r11, r9w, SXTW
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
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__aarch64__) */

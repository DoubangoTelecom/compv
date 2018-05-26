#########################################################################
# Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__aarch64__)
.include "compv_common_arm64.S"

.equ kAtan2Eps_32f_0, 0x0000
.equ kAtan2Eps_32f_1, 0x2580
.equ kAtan2P1_32f_0, 0x226f
.equ kAtan2P1_32f_1, 0x4265
.equ kAtan2P3_32f_0, 0x56ee
.equ kAtan2P3_32f_1, 0xc195
.equ kAtan2P5_32f_0, 0x9fbf
.equ kAtan2P5_32f_1, 0x410e
.equ kAtan2P7_32f_0, 0x8ad9
.equ kAtan2P7_32f_1, 0xc022
.equ k90_32f_0, 0x0000
.equ k90_32f_1, 0x42b4
.equ k180_32f_0, 0x0000
.equ k180_32f_1, 0x4334
.equ k360_32f_0, 0x0000
.equ k360_32f_1, 0x43b4

.data

.extern

.text

#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) const compv_float32_t* y
# arg(1) -> COMPV_ALIGNED(NEON) const compv_float32_t* x
# arg(2) -> COMPV_ALIGNED(NEON) compv_float32_t* r
# arg(3) -> const compv_float32_t* scale1
# arg(4) -> compv_uscalar_t width
# arg(5) -> compv_uscalar_t height
# arg(6) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
.macro CompVMathTrigFastAtan2_32f_Macro_NEON64 fusedMultiplyAdd
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

    ## Set arguments ##
	y .req r0
	x .req r1
	out .req r2
    scale1 .req r3 // must not change (r3w alias used later)
	width .req r4
	height .req r5
    stride .req r6

    i .req r7

    vec1 .req v0
    vec2 .req v1
    vecAx .req v2
    vecAy .req v3
    vecMask .req v4
    vec0 .req v5
    vecAtan2_scale .req v6
    vecAtan2_plus360 .req v7
    vecAtan2_plus180 .req v8
    vecAtan2_zero .req v9
    vecAtan2_plus90 .req v10
    vecAtan2_p1 .req v11
    vecAtan2_p3 .req v12
    vecAtan2_p7 .req v13
    vecAtan2_p5 .req v14
    vecAtan2_eps .req v15
    vec3 .req v16

    prfm pldl1keep, [x, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [x, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [x, #(CACHE_LINE_SIZE*2)]
    prfm pldl1keep, [y, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [y, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [y, #(CACHE_LINE_SIZE*2)]

    
    movk r7w, #kAtan2Eps_32f_1, lsl #16
    dup vecAtan2_eps.4s, r7w

    ldr scale1, [scale1]
    movz r7w, #kAtan2Eps_32f_0
    movz r8w, #kAtan2P1_32f_0
    movz r9w, #kAtan2P3_32f_0
    movz r10w, #kAtan2P5_32f_0
    movz r11w, #kAtan2P7_32f_0
    movk r7w, #kAtan2Eps_32f_1, lsl #16
    movk r8w, #kAtan2P1_32f_1, lsl #16
    movk r9w, #kAtan2P3_32f_1, lsl #16
    movk r10w, #kAtan2P5_32f_1, lsl #16
    movk r11w, #kAtan2P7_32f_1, lsl #16
    dup vecAtan2_eps.4s, r7w
    dup vecAtan2_p1.4s, r8w
    dup vecAtan2_p3.4s, r9w
    dup vecAtan2_p5.4s, r10w
    dup vecAtan2_p7.4s, r11w
    eor vecAtan2_zero.16b, vecAtan2_zero.16b, vecAtan2_zero.16b
    movz r7w, #k90_32f_0
    movz r8w, #k180_32f_0
    movz r9w, #k360_32f_0
    movk r7w, #k90_32f_1, lsl #16
    movk r8w, #k180_32f_1, lsl #16
    movk r9w, #k360_32f_1, lsl #16
    dup vecAtan2_plus90.4s, r7w
    dup vecAtan2_plus180.4s, r8w
    dup vecAtan2_plus360.4s, r9w
    dup vecAtan2_scale.4s, r3w // r3w is scale1    

    # Transform stride to padding then from samples to bytes #
	add r11, width, #3
	and r11, r11, #-4
	sub stride, stride, r11
    lsl stride, stride, #(COMPV_GAS_FLOAT32_SHIFT_BYTES)

    #####################################################
    # for (compv_uscalar_t j = 0; j < height; ++j)
    #####################################################
    LoopHeight_CompVMathTrigFastAtan2_32f_Macro_NEON64\@:
        #####################################################
        # for (compv_uscalar_t i = 0; i < width; i += 4)
        #####################################################
        mov i, #0
        LoopWidth_CompVMathTrigFastAtan2_32f_Macro_NEON64\@:
            ld1 { vecAx.4s }, [x] // ARM64: increment now and remove later load
            ld1 { vecAy.4s }, [y] // ARM64: increment now and remove later load
            prfm pldl1keep, [x, #(CACHE_LINE_SIZE*3)]
            prfm pldl1keep, [y, #(CACHE_LINE_SIZE*3)]

            // ax = std::abs(x[i]), ay = std::abs(y[i]);
            fabs vecAx.4s, vecAx.4s
            fabs vecAy.4s, vecAy.4s

            // if (ax >= ay) vec1 = ay, vec2 = ax;
            // else vec1 = ax, vec2 = ay;
            fcmge vecMask.4s, vecAx.4s, vecAy.4s
            and vec1.16b, vecAy.16b, vecMask.16b
            and vec2.16b, vecAx.16b, vecMask.16b
            bic vecAx.16b, vecAx.16b, vecMask.16b
            bic vecAy.16b, vecAy.16b, vecMask.16b
            orr vec1.16b, vec1.16b, vecAx.16b
            orr vec2.16b, vec2.16b, vecAy.16b

            // c = vec1 / (vec2 + atan2_eps)
            // c2 = c*c
            fadd vec2.4s, vec2.4s, vecAtan2_eps.4s
            // vec1 = vecC
            // vec2 = vecC2
            fdiv vec1.4s, vec1.4s, vec2.4s
            .if \fusedMultiplyAdd
                mov vecAx.4s, vecAtan2_p5.4s
                mov vecAy.4s, vecAtan2_p3.4s
                mov vec0.4s, vecAtan2_p1.4s
            .endif
            fmul vec2.4s, vec1.4s, vec1.4s

            // a = (((atan2_p7*c2 + atan2_p5)*c2 + atan2_p3)*c2 + atan2_p1)*c
            .if \fusedMultiplyAdd
                fmla vecAx.4s, vecAtan2_p7.4s, vec2.4s
                fmla vecAy.4s, vecAx.4s, vec2.4s
                fmla vec0.4s, vecAy.4s, vec2.4s
            .else
                fmul vec3.4s, vecAtan2_p7.4s, vec2.4s
                fadd vec0.4s, vecAtan2_p5.4s, vec3.4s
                fmul vec3.4s, vec0.4s, vec2.4s
                fadd vec0.4s, vecAtan2_p3.4s, vec3.4s
                fmul vec3.4s, vec0.4s, vec2.4s
                fadd vec0.4s, vecAtan2_p1.4s, vec3.4s
            .endif
            fmul vec0.4s, vec0.4s, vec1.4s

            // if (!(ax >= ay)) a = 90 - a
            fsub vec1.4s, vecAtan2_plus90.4s, vec0.4s
            and vec0.16b, vec0.16b, vecMask.16b
            bic vec1.16b, vec1.16b, vecMask.16b
            orr vec0.16b, vec0.16b, vec1.16b

            // TODO(dmi): not needed for ARM64 -> save values
            ld1 { vecAx.4s }, [x], #(COMPV_GAS_V_SZ_BYTES)
            ld1 { vecAy.4s }, [y], #(COMPV_GAS_V_SZ_BYTES)

            // if (x[i] < 0) a = 180.f - a
            fcmlt vecMask.4s, vecAx.4s, vecAtan2_zero.4s
            fsub vec1.4s, vecAtan2_plus180.4s, vec0.4s
            bic vec0.16b, vec0.16b, vecMask.16b
            and vec1.16b, vec1.16b, vecMask.16b
            orr vec0.16b, vec0.16b, vec1.16b

            // if (y[i] < 0) a = 360.f - a
            fcmlt vecMask.4s, vecAy.4s, vecAtan2_zero.4s
            fsub vec1.4s, vecAtan2_plus360.4s, vec0.4s
            bic vec0.16b, vec0.16b, vecMask.16b
            and vec1.16b, vec1.16b, vecMask.16b
            orr vec0.16b, vec0.16b, vec1.16b

            // r[i] = a * scale
            fmul vec0.4s, vec0.4s, vecAtan2_scale.4s
            st1 { vec0.4s }, [out], #(COMPV_GAS_V_SZ_BYTES)

            add i, i, #4
            cmp i, width
            blt LoopWidth_CompVMathTrigFastAtan2_32f_Macro_NEON64\@
        EndOf_LoopWidth_CompVMathTrigFastAtan2_32f_Macro_NEON64\@:

        subs height, height, #1
        add y, y, stride
        add x, x, stride
        add out, out, stride
        bne LoopHeight_CompVMathTrigFastAtan2_32f_Macro_NEON64\@
    EndOf_LoopHeight_CompVMathTrigFastAtan2_32f_Macro_NEON64\@:

    .unreq y
    .unreq x
	.unreq out
    .unreq scale1
	.unreq width
	.unreq height
    .unreq stride

    .unreq i

    .unreq vec1
    .unreq vec2
    .unreq vecAx
    .unreq vecAy
    .unreq vecMask
    .unreq vec0
    .unreq vecAtan2_scale
    .unreq vecAtan2_plus360
    .unreq vecAtan2_plus180
    .unreq vecAtan2_zero
    .unreq vecAtan2_plus90
    .unreq vecAtan2_p1
    .unreq vecAtan2_p3
    .unreq vecAtan2_p7
    .unreq vecAtan2_p5
    .unreq vecAtan2_eps
    .unreq vec3

    COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVMathTrigFastAtan2_32f_Asm_NEON64
    CompVMathTrigFastAtan2_32f_Macro_NEON64 0

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVMathTrigFastAtan2_32f_Asm_FMA_NEON64
    CompVMathTrigFastAtan2_32f_Macro_NEON64 1

#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) const compv_float32_t* x
# arg(1) -> COMPV_ALIGNED(NEON) const compv_float32_t* y
# arg(2) -> COMPV_ALIGNED(NEON) compv_float32_t* r
# arg(3) -> compv_uscalar_t width
# arg(4) -> compv_uscalar_t height
# arg(5) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
.macro CompVMathTrigHypotNaive_32f_Macro_NEON64 fusedMultiplyAdd
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Set arguments ##
	x .req r0
	y .req r1
	out .req r2
	width .req r3
	height .req r4
    stride .req r5

    width16 .req r6
    i .req r7

    prfm pldl1keep, [x, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [x, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [x, #(CACHE_LINE_SIZE*2)]
    prfm pldl1keep, [y, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [y, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [y, #(CACHE_LINE_SIZE*2)]

    and width16, width, #-16

    # Transform stride to padding then from samples to bytes #
	add r11, width, #3
	and r11, r11, #-4
	sub stride, stride, r11
    lsl stride, stride, #(COMPV_GAS_FLOAT32_SHIFT_BYTES)

    ####################################################
    # for (compv_uscalar_t j = 0; j < height; ++j)
    ####################################################
    LoopHeight_CompVMathTrigHypotNaive_32f_Macro_NEON64\@:
        mov i, #0 
        ####################################################
        # for (i = 0; i < width16; i += 16)
        ####################################################
        tst width16, width16
        beq EndOf_LoopWidth16_CompVMathTrigHypotNaive_32f_Macro_NEON64\@
        LoopWidth16_CompVMathTrigHypotNaive_32f_Macro_NEON64\@:
            ldp q0, q1, [x], #(2*COMPV_GAS_Q_SZ_BYTES)
            ldp q2, q3, [x], #(2*COMPV_GAS_Q_SZ_BYTES)
            ldp q4, q5, [y], #(2*COMPV_GAS_Q_SZ_BYTES)
            ldp q6, q7, [y], #(2*COMPV_GAS_Q_SZ_BYTES)
            prfm pldl1keep, [x, #(CACHE_LINE_SIZE*3)]
            prfm pldl1keep, [y, #(CACHE_LINE_SIZE*3)]
            fmul v0.4s, v0.4s, v0.4s
            fmul v1.4s, v1.4s, v1.4s
            fmul v2.4s, v2.4s, v2.4s
            fmul v3.4s, v3.4s, v3.4s
            .if \fusedMultiplyAdd
                fmla v0.4s, v4.4s, v4.4s
                fmla v1.4s, v5.4s, v5.4s
                fmla v2.4s, v6.4s, v6.4s
                fmla v3.4s, v7.4s, v7.4s
            .else
                fmul v4.4s, v4.4s, v4.4s
                fmul v5.4s, v5.4s, v5.4s
                fmul v6.4s, v6.4s, v6.4s
                fmul v7.4s, v7.4s, v7.4s
                fadd v0.4s, v0.4s, v4.4s
                fadd v1.4s, v1.4s, v5.4s
                fadd v2.4s, v2.4s, v6.4s
                fadd v3.4s, v3.4s, v7.4s
            .endif
            fsqrt v0.4s, v0.4s
            fsqrt v1.4s, v1.4s
            fsqrt v2.4s, v2.4s
            fsqrt v3.4s, v3.4s
            stp q0, q1, [out], #(2*COMPV_GAS_Q_SZ_BYTES)
            stp q2, q3, [out], #(2*COMPV_GAS_Q_SZ_BYTES)
            add i, i, #16
            cmp i, width16
            blt LoopWidth16_CompVMathTrigHypotNaive_32f_Macro_NEON64\@
        EndOf_LoopWidth16_CompVMathTrigHypotNaive_32f_Macro_NEON64\@:

        ####################################################
        # for (; i < width; i += 4)
        ####################################################
        LoopWidth4_CompVMathTrigHypotNaive_32f_Macro_NEON64\@:
            ldr q0, [x], #(1*COMPV_GAS_Q_SZ_BYTES)
            ldr q5, [y], #(1*COMPV_GAS_Q_SZ_BYTES)
            fmul v0.4s, v0.4s, v0.4s
            fmla v0.4s, v4.4s, v4.4s
            fsqrt v0.4s, v0.4s
            str q0, [out], #(1*COMPV_GAS_Q_SZ_BYTES)
            add i, i, #4
            cmp i, width
            blt LoopWidth4_CompVMathTrigHypotNaive_32f_Macro_NEON64\@
        EndOf_LoopWidth4_CompVMathTrigHypotNaive_32f_Macro_NEON64\@:

        subs height, height, #1
        add y, y, stride
        add x, x, stride
        add out, out, stride
        bne LoopHeight_CompVMathTrigHypotNaive_32f_Macro_NEON64\@
    EndOf_LoopHeight_CompVMathTrigHypotNaive_32f_Macro_NEON64\@:


    .unreq x
	.unreq y
	.unreq out
	.unreq width
	.unreq height
   .unreq  stride

    .unreq width16
    .unreq i

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVMathTrigHypotNaive_32f_Asm_NEON64
	CompVMathTrigHypotNaive_32f_Macro_NEON64 0

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVMathTrigHypotNaive_32f_Asm_FMA_NEON64
	CompVMathTrigHypotNaive_32f_Macro_NEON64 1

#endif /* defined(__aarch64__) */

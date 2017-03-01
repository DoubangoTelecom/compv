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
# arg(0) -> COMPV_ALIGNED(NEON) compv_float64_t* ri
# arg(1) -> COMPV_ALIGNED(NEON) compv_float64_t* rj
# arg(2) -> const compv_float64_t* cos1
# arg(3) -> const compv_float64_t* sin1 @ s1 is reserved name
# arg(4) -> compv_uscalar_t count
COMPV_GAS_FUNCTION_DECLARE CompVMathMatrixMulGA_64f_Asm_NEON64
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS
	
	ri .req r0
	rj .req r1
	cos1 .req r2
	sin1 .req r3
	count .req r4

    #define vecC v31.d[0]
    #define vecS v31.d[1]

    vecRI0 .req v0
    vecRI1 .req v1
    vecRI2 .req v2
    vecRI3 .req v3
    vecRJ0 .req v4
    vecRJ1 .req v5
    vecRJ2 .req v6
    vecRJ3 .req v7

    ld1 { v31.d }[0], [cos1]
    ld1 { v31.d }[1], [sin1]
    
    #########################################################################
	# for (i = 0; i < countSigned - 7; i += 8)
    #########################################################################
    lsr r5, count, #3 // div 8
    cbz r5, EndOf_Loop8_CompVMathMatrixMulGA_64f_Asm_NEON64
    Loop8_CompVMathMatrixMulGA_64f_Asm_NEON64:
        subs r5, r5, #1
        ldp q0, q1, [ri]
        ldp q2, q3, [ri]
        ldp q4, q5, [rj]
        ldp q6, q7, [rj]
        fmul v9.2d, vecRI0.2d, vecC
        fmul v17.2d, vecRJ0.2d, vecS
        fmul v10.2d, vecRI1.2d, vecC
        fmul v18.2d, vecRJ1.2d, vecS
        fmul v11.2d, vecRI2.2d, vecC
        fmul v19.2d, vecRJ2.2d, vecS
        fmul v12.2d, vecRI3.2d, vecC
        fmul v20.2d, vecRJ3.2d, vecS
        fmul v13.2d, vecRJ0.2d, vecC
        fmul v21.2d, vecRI0.2d, vecS
        fmul v14.2d, vecRJ1.2d, vecC
        fmul v22.2d, vecRI1.2d, vecS
        fmul v15.2d, vecRJ2.2d, vecC
        fmul v23.2d, vecRI2.2d, vecS
        fmul v16.2d, vecRJ3.2d, vecC
        fmul v24.2d, vecRI3.2d, vecS
        fadd v9.2d, v9.2d, v17.2d
        fadd v10.2d, v10.2d, v18.2d
        fadd v11.2d, v11.2d, v19.2d
        fadd v12.2d, v12.2d, v20.2d
        fsub v13.2d, v13.2d, v21.2d
        fsub v14.2d, v14.2d, v22.2d
        fsub v15.2d, v15.2d, v23.2d
        fsub v16.2d, v16.2d, v24.2d
        stp q9, q10, [ri], #32
        stp q11, q12, [ri], #32
        stp q13, q14, [rj], #32
        stp q15, q16, [rj], #32
        bne Loop8_CompVMathMatrixMulGA_64f_Asm_NEON64
        EndOf_Loop8_CompVMathMatrixMulGA_64f_Asm_NEON64:
        ## EndOf_Loop8_CompVMathMatrixMulGA_64f_Asm_NEON64 ##

    #########################################################################
    # .if (i < countSigned - 3)
    #########################################################################
    and r5, count, #7 // modulo 8
    lsr r5, r5, #2 // div 4
    cbz r5, EndOf_Loop4_CompVMathMatrixMulGA_64f_Asm_NEON64
    Loop4_CompVMathMatrixMulGA_64f_Asm_NEON64:
        ldp q0, q1, [ri]
        ldp q4, q5, [rj]
        fmul v9.2d, vecRI0.2d, vecC
        fmul v17.2d, vecRJ0.2d, vecS
        fmul v10.2d, vecRI1.2d, vecC
        fmul v18.2d, vecRJ1.2d, vecS
        fmul v13.2d, vecRJ0.2d, vecC
        fmul v21.2d, vecRI0.2d, vecS
        fmul v14.2d, vecRJ1.2d, vecC
        fmul v22.2d, vecRI1.2d, vecS
        fadd v9.2d, v9.2d, v17.2d
        fadd v10.2d, v10.2d, v18.2d
        fsub v13.2d, v13.2d, v21.2d
        fsub v14.2d, v14.2d, v22.2d
        stp q9, q10, [ri], #32
        stp q13, q14, [rj], #32
        EndOf_Loop4_CompVMathMatrixMulGA_64f_Asm_NEON64:
        ## EndOf_Loop4_CompVMathMatrixMulGA_64f_Asm_NEON64 ##


    #########################################################################
    # .for (; i < countSigned; i += 2)
    #########################################################################
    and r5, count, #3 // modulo 4
    cbz r5, EndOf_Loop2_CompVMathMatrixMulGA_64f_Asm_NEON64
    add r5, r5, #1
    lsr r5, r5, #1
    Loop2_CompVMathMatrixMulGA_64f_Asm_NEON64:
        subs r5, r5, #1
        ldr q0, [ri]
        ldr q4, [rj]
        fmul v9.2d, vecRI0.2d, vecC
        fmul v17.2d, vecRJ0.2d, vecS
        fmul v13.2d, vecRJ0.2d, vecC
        fmul v21.2d, vecRI0.2d, vecS
        fadd v9.2d, v9.2d, v17.2d
        fsub v13.2d, v13.2d, v21.2d
        str q9, [ri], #16
        str q13, [rj], #16
        bne Loop2_CompVMathMatrixMulGA_64f_Asm_NEON64
        EndOf_Loop2_CompVMathMatrixMulGA_64f_Asm_NEON64:
        ## EndOf_Loop2_CompVMathMatrixMulGA_64f_Asm_NEON64 ##

    .unreq ri
	.unreq rj
	.unreq cos1
	.unreq sin1
	.unreq count

    .unreq vecRI0
    .unreq vecRI1
    .unreq vecRI2
    .unreq vecRI3
    .unreq vecRJ0
    .unreq vecRJ1
    .unreq vecRJ2
    .unreq vecRJ3

    #undef vecC
    #undef vecS


	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
    

#endif /* defined(__aarch64__) */

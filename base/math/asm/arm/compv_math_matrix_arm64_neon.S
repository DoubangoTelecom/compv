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
# arg(0) -> const COMPV_ALIGNED(NEON) compv_float64_t* A
# arg(1) -> compv_uscalar_t aRows
# arg(2) -> COMPV_ALIGNED(NEON) compv_uscalar_t aStrideInBytes
# arg(3) -> const COMPV_ALIGNED(NEON) compv_float64_t* B
# arg(4) -> compv_uscalar_t bRows
# arg(5) -> compv_uscalar_t bCols
# arg(6) -> COMPV_ALIGNED(NEON) compv_uscalar_t bStrideInBytes
# arg(7) -> COMPV_ALIGNED(NEON) compv_float64_t* R
# arg(8) -> COMPV_ALIGNED(NEON) compv_uscalar_t rStrideInBytes
.macro CompVMathMatrixMulABt_64f_Macro_NEON64 fusedMultiplyAdd
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Load arguments ##
    ldr r8, [bp, #(prolog_bytes + (0*COMPV_GAS_REG_SZ_BYTES))]
    A .req r0
	aRows .req r1
	aStrideInBytes .req r2
	B .req r3
    bRows .req r4
    bCols .req r5
    bStrideInBytes .req r6
    R .req r7
    rStrideInBytes .req r8

    BZero .req r12 // B0 is reserved name
    j .req r13
    k .req r14
    bColsMinus15 .req r15
    bColsMinus7 .req r16
    bColsMinus3 .req r17

    vecSum .req v0
    vecSumx .req d0

    sub bColsMinus15, bCols, #15
    sub bColsMinus7, bCols, #7
    sub bColsMinus3, bCols, #3

    lsl r9, bCols, #3 // bColsInBytes
    sub bStrideInBytes, bStrideInBytes, r9 // Now bStrideInBytes holds padInBytes

    #########################################################################
    # for (i = 0; i < aRows; ++i)
    #########################################################################
    LoopARows_CompVMathMatrixMulABt_64f_Asm_NEON64\@:
        mov BZero, B
        mov j, bRows
        mov r11, R
        #########################################################################
        # for (j = 0; j < bRows; ++j)
        #########################################################################
        LoopBRows_CompVMathMatrixMulABt_64f_Asm_NEON64\@:
            movi vecSum.2d, #0
            mov r10, A
            #########################################################################
            # for (k = 0; k < bColsSigned - 15; k += 16)
            #########################################################################
            mov k, #0
            cmp bColsMinus15, #0
            ble EndOf_LoopBCols16_CompVMathMatrixMulABt_64f_Asm_NEON64\@
            LoopBCols16_CompVMathMatrixMulABt_64f_Asm_NEON64\@:
                add k, k, #16
                .if \fusedMultiplyAdd
                    ldp q1, q2, [r10], #32
                    ldp q5, q6, [BZero], #32
                    fmul v1.2d, v1.2d, v5.2d
                    fmul v2.2d, v2.2d, v6.2d
                    ldp q3, q4, [r10], #32
                    ldp q7, q8, [BZero], #32
                    fmul v3.2d, v3.2d, v7.2d
                    fmul v4.2d, v4.2d, v8.2d
                    ldp q5, q6, [r10], #32
                    ldp q9, q10, [BZero], #32
                    fmla v1.2d, v5.2d, v9.2d
                    fmla v2.2d, v6.2d, v10.2d
                    ldp q7, q8, [r10], #32
                    ldp q11, q12, [BZero], #32
                    fmla v3.2d, v7.2d, v11.2d
                    fmla v4.2d, v8.2d, v12.2d


                    fadd v1.2d, v1.2d, v2.2d
                    fadd v3.2d, v3.2d, v4.2d
                    fadd vecSum.2d, vecSum.2d, v1.2d
                    fadd vecSum.2d, vecSum.2d, v3.2d
                .else                
                    ldp q1, q2, [r10], #32
                    ldp q5, q6, [BZero], #32
                    fmul v1.2d, v1.2d, v5.2d
                    fmul v2.2d, v2.2d, v6.2d
                    ldp q3, q4, [r10], #32
                    ldp q7, q8, [BZero], #32
                    fmul v3.2d, v3.2d, v7.2d
                    fmul v4.2d, v4.2d, v8.2d
                    ldp q5, q6, [r10], #32
                    ldp q9, q10, [BZero], #32
                    fmul v5.2d, v5.2d, v9.2d
                    fmul v6.2d, v6.2d, v10.2d
                    ldp q7, q8, [r10], #32
                    ldp q11, q12, [BZero], #32
                    fmul v7.2d, v7.2d, v11.2d
                    fmul v8.2d, v8.2d, v12.2d
                    fadd v1.2d, v1.2d, v2.2d
                    fadd v3.2d, v3.2d, v4.2d
                    fadd v5.2d, v5.2d, v6.2d
                    fadd v7.2d, v7.2d, v8.2d
                    fadd v1.2d, v1.2d, v3.2d
                    fadd v5.2d, v5.2d, v7.2d
                    fadd vecSum.2d, vecSum.2d, v1.2d
                    fadd vecSum.2d, vecSum.2d, v5.2d
                .endif
                cmp k, bColsMinus15
                blt LoopBCols16_CompVMathMatrixMulABt_64f_Asm_NEON64\@
                EndOf_LoopBCols16_CompVMathMatrixMulABt_64f_Asm_NEON64\@:
                ## EndOf_LoopBCols16_CompVMathMatrixMulABt_64f_Asm_NEON64 ##

            #########################################################################
            # .if (k < bColsSigned - 7)
            #########################################################################
            cmp k, bColsMinus7
            bge EndOf_LoopBCols8_CompVMathMatrixMulABt_64f_Asm_NEON64\@
            LoopBCols8_CompVMathMatrixMulABt_64f_Asm_NEON64\@:
                add k, k, #8
                ldp q1, q2, [r10], #32
                ldp q5, q6, [BZero], #32
                fmul v1.2d, v1.2d, v5.2d
                fmul v2.2d, v2.2d, v6.2d
                ldp q3, q4, [r10], #32
                ldp q7, q8, [BZero], #32
                fmul v3.2d, v3.2d, v7.2d
                fmul v4.2d, v4.2d, v8.2d
                fadd v1.2d, v1.2d, v2.2d
                fadd v3.2d, v3.2d, v4.2d
                fadd vecSum.2d, vecSum.2d, v1.2d
                fadd vecSum.2d, vecSum.2d, v3.2d
                EndOf_LoopBCols8_CompVMathMatrixMulABt_64f_Asm_NEON64\@:
                ## EndOf_LoopBCols8_CompVMathMatrixMulABt_64f_Asm_NEON64 ##

            #########################################################################
            # .if (k < bColsSigned - 3)
            #########################################################################
            cmp k, bColsMinus3
            bge EndOf_LoopBCols4_CompVMathMatrixMulABt_64f_Asm_NEON64\@
            LoopBCols4_CompVMathMatrixMulABt_64f_Asm_NEON64\@:
                add k, k, #4
                ldp q1, q2, [r10], #32
                ldp q3, q4, [BZero], #32
                fmul v1.2d, v1.2d, v3.2d
                fmul v2.2d, v2.2d, v4.2d
                fadd vecSum.2d, vecSum.2d, v1.2d
                fadd vecSum.2d, vecSum.2d, v2.2d
                EndOf_LoopBCols4_CompVMathMatrixMulABt_64f_Asm_NEON64\@:
                ## EndOf_LoopBCols4_CompVMathMatrixMulABt_64f_Asm_NEON64 ##

            #########################################################################
            # for (; k < bColsSigned; k += 1)
            #########################################################################
            cmp k, bCols
            bge EndOf_LoopBCols1_CompVMathMatrixMulABt_64f_Asm_NEON64\@
            mov d17, vecSum.d[0] // intermediate sum
            LoopBCols1_CompVMathMatrixMulABt_64f_Asm_NEON64\@:
                add k, k, #1
                ldr d15, [r10], #8
                ldr d16, [BZero], #8
                fmul d15, d15, d16
                fadd d17, d17, d15

                cmp k, bCols
                blt LoopBCols1_CompVMathMatrixMulABt_64f_Asm_NEON64\@
                // insert intermediate sum
                ins vecSum.d[0], v17.d[0]
                EndOf_LoopBCols1_CompVMathMatrixMulABt_64f_Asm_NEON64\@:
                ## EndOf_LoopBCols1_CompVMathMatrixMulABt_64f_Asm_NEON64 ##

            faddp vecSum.2d, vecSum.2d, vecSum.2d
            st1 {vecSum.d}[0], [r11], #COMPV_GAS_FLOAT64_SZ_BYTES
            subs j, j, #1
            add BZero, BZero, bStrideInBytes // bStrideInBytes holds padInBytes
            bne LoopBRows_CompVMathMatrixMulABt_64f_Asm_NEON64\@
            ## EndOf_LoopBRows_CompVMathMatrixMulABt_64f_Asm_NEON64 ##


        subs aRows, aRows, #1
        add A, A, aStrideInBytes
        add R, R, rStrideInBytes
        bne LoopARows_CompVMathMatrixMulABt_64f_Asm_NEON64\@
        ## EndOf_LoopARows_CompVMathMatrixMulABt_64f_Asm_NEON64 ##

    .unreq A
	.unreq aRows
	.unreq aStrideInBytes
	.unreq B
    .unreq bRows
    .unreq bCols
    .unreq bStrideInBytes
    .unreq R
    .unreq rStrideInBytes

    .unreq BZero
    .unreq j
    .unreq k
    .unreq bColsMinus15
    .unreq bColsMinus7
    .unreq bColsMinus3

    .unreq vecSum
    .unreq vecSumx

    COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVMathMatrixMulABt_64f_Asm_NEON64
    CompVMathMatrixMulABt_64f_Macro_NEON64 0

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVMathMatrixMulABt_64f_Asm_FMA_NEON64
    CompVMathMatrixMulABt_64f_Macro_NEON64 1

#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) compv_float64_t* ri
# arg(1) -> COMPV_ALIGNED(NEON) compv_float64_t* rj
# arg(2) -> const compv_float64_t* cos1
# arg(3) -> const compv_float64_t* sin1 # s1 is reserved name
# arg(4) -> compv_uscalar_t count
.macro CompVMathMatrixMulGA_64f_Macro_NEON64 fusedMultiplyAdd
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
    cbz r5, EndOf_Loop8_CompVMathMatrixMulGA_64f_Asm_NEON64\@
    Loop8_CompVMathMatrixMulGA_64f_Asm_NEON64\@:
        subs r5, r5, #1
        ldp q0, q1, [ri]
        ldp q2, q3, [ri, #32]
        ldp q4, q5, [rj]
        ldp q6, q7, [rj, #32]
        .if \fusedMultiplyAdd
            fmul v9.2d, vecRI0.2d, vecC
            fmul v10.2d, vecRI1.2d, vecC
            fmul v11.2d, vecRI2.2d, vecC
            fmul v12.2d, vecRI3.2d, vecC
            fmul v13.2d, vecRJ0.2d, vecC
            fmul v14.2d, vecRJ1.2d, vecC
            fmul v15.2d, vecRJ2.2d, vecC
            fmul v16.2d, vecRJ3.2d, vecC
            fmla v9.2d, vecRJ0.2d, vecS
            fmla v10.2d, vecRJ1.2d, vecS
            fmla v11.2d, vecRJ2.2d, vecS
            fmla v12.2d, vecRJ3.2d, vecS
            fmls v13.2d, vecRI0.2d, vecS
            fmls v14.2d, vecRI1.2d, vecS
            fmls v15.2d, vecRI2.2d, vecS
            fmls v16.2d, vecRI3.2d, vecS
        .else
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
        .endif
        stp q9, q10, [ri], #32
        stp q11, q12, [ri], #32
        stp q13, q14, [rj], #32
        stp q15, q16, [rj], #32
        bne Loop8_CompVMathMatrixMulGA_64f_Asm_NEON64\@
        EndOf_Loop8_CompVMathMatrixMulGA_64f_Asm_NEON64\@:
        ## EndOf_Loop8_CompVMathMatrixMulGA_64f_Asm_NEON64 ##

    #########################################################################
    # .if (i < countSigned - 3)
    #########################################################################
    and r5, count, #7 // modulo 8
    lsr r5, r5, #2 // div 4
    cbz r5, EndOf_Loop4_CompVMathMatrixMulGA_64f_Asm_NEON64\@
    Loop4_CompVMathMatrixMulGA_64f_Asm_NEON64\@:
        ldp q0, q1, [ri]
        ldp q4, q5, [rj]
        .if \fusedMultiplyAdd
            fmul v9.2d, vecRI0.2d, vecC
            fmul v10.2d, vecRI1.2d, vecC
            fmul v13.2d, vecRJ0.2d, vecC
            fmul v14.2d, vecRJ1.2d, vecC
            fmla v9.2d, vecRJ0.2d, vecS
            fmla v10.2d, vecRJ1.2d, vecS
            fmls v13.2d, vecRI0.2d, vecS
            fmls v14.2d, vecRI1.2d, vecS
        .else
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
        .endif
        stp q9, q10, [ri], #32
        stp q13, q14, [rj], #32
        EndOf_Loop4_CompVMathMatrixMulGA_64f_Asm_NEON64\@:
        ## EndOf_Loop4_CompVMathMatrixMulGA_64f_Asm_NEON64 ##


    #########################################################################
    # .for (; i < countSigned; i += 2)
    #########################################################################
    and r5, count, #3 // modulo 4
    cbz r5, EndOf_Loop2_CompVMathMatrixMulGA_64f_Asm_NEON64\@
    add r5, r5, #1
    lsr r5, r5, #1
    Loop2_CompVMathMatrixMulGA_64f_Asm_NEON64\@:
        subs r5, r5, #1
        ldr q0, [ri]
        ldr q4, [rj]
        .if \fusedMultiplyAdd
            fmul v9.2d, vecRI0.2d, vecC
            fmul v13.2d, vecRJ0.2d, vecC
            fmla v9.2d, vecRJ0.2d, vecS
            fmls v13.2d, vecRI0.2d, vecS
        .else
            fmul v9.2d, vecRI0.2d, vecC
            fmul v17.2d, vecRJ0.2d, vecS
            fmul v13.2d, vecRJ0.2d, vecC
            fmul v21.2d, vecRI0.2d, vecS
            fadd v9.2d, v9.2d, v17.2d
            fsub v13.2d, v13.2d, v21.2d
        .endif
        str q9, [ri], #16
        str q13, [rj], #16
        bne Loop2_CompVMathMatrixMulGA_64f_Asm_NEON64\@
        EndOf_Loop2_CompVMathMatrixMulGA_64f_Asm_NEON64\@:
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
.endm

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVMathMatrixMulGA_64f_Asm_NEON64
    CompVMathMatrixMulGA_64f_Macro_NEON64 0

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVMathMatrixMulGA_64f_Asm_FMA_NEON64
    CompVMathMatrixMulGA_64f_Macro_NEON64 1

#endif /* defined(__aarch64__) */
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
@ arg(0) -> const COMPV_ALIGNED(NEON) compv_float64_t* A
@ arg(1) -> compv_uscalar_t aRows
@ arg(2) -> COMPV_ALIGNED(NEON) compv_uscalar_t aStrideInBytes
@ arg(3) -> const COMPV_ALIGNED(NEON) compv_float64_t* B
@ arg(4) -> compv_uscalar_t bRows
@ arg(5) -> compv_uscalar_t bCols
@ arg(6) -> COMPV_ALIGNED(NEON) compv_uscalar_t bStrideInBytes
@ arg(7) -> COMPV_ALIGNED(NEON) compv_float64_t* R
@ arg(8) -> COMPV_ALIGNED(NEON) compv_uscalar_t rStrideInBytes
.macro CompVMathMatrixMulABt_64f_Macro_NEON32 fusedMultiplyAdd
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 9
	COMPV_GAS_SAVE_NEON_REGS

	@@ Load arguments @@
	ldm_args r0-r8
    A .req r0
	aRows .req r1
	aStrideInBytes .req r2
	B_ .req r3 @ B is reserved name
    bRows .req r4
    bCols .req r5
    bStrideInBytes .req r6
    R .req r7
    rStrideInBytes .req r8

    vecSum .req q0
    vecSumx .req q0x
    vecSumy .req q0y

    .equ argi_B, 3
    .equ argi_bRows, 4

    lsl r9, bCols, #3 @ bColsInBytes
    sub bStrideInBytes, bStrideInBytes, r9 @ Now bStrideInBytes holds padInBytes

    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (i = 0; i < aRows; ++i)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    LoopARows_CompVMathMatrixMulABt_64f_Asm_NEON32\@:
        ldr_arg argi_B, B_
        ldr_arg argi_bRows, bRows
        mov r11, R
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (j = 0; j < bRows; ++j)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        LoopBRows_CompVMathMatrixMulABt_64f_Asm_NEON32\@:
            veor.64 vecSum, vecSum, vecSum
            mov r10, A
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            @ for (k = 0; k < bColsSigned - 7; k += 8)
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            lsrs r9, bCols, #3
            beq EndOf_LoopBCols8_CompVMathMatrixMulABt_64f_Asm_NEON32\@
            LoopBCols8_CompVMathMatrixMulABt_64f_Asm_NEON32\@:
                subs r9, r9, #1
                vld1.64 {q1x, q1y, q2x, q2y}, [r10 :128]!
                vld1.64 {q3x, q3y, q4x, q4y}, [B_ :128]!
                vmul.f64 q5x, q1x, q3x
                vmul.f64 q5y, q1y, q3y
                vmul.f64 q6x, q2x, q4x
                vmul.f64 q6y, q2y, q4y
                vld1.64 {q1x, q1y, q2x, q2y}, [r10 :128]!
                vld1.64 {q3x, q3y, q4x, q4y}, [B_ :128]!
                .if \fusedMultiplyAdd
                    vfma.f64 q5x, q1x, q3x
                    vfma.f64 q5y, q1y, q3y
                    vfma.f64 q6x, q2x, q4x
                    vfma.f64 q6y, q2y, q4y
                .else
                    vmla.f64 q5x, q1x, q3x
                    vmla.f64 q5y, q1y, q3y
                    vmla.f64 q6x, q2x, q4x
                    vmla.f64 q6y, q2y, q4y
                .endif
                vadd.f64 q5x, q5x, q5y
                vadd.f64 q6x, q6x, q6y
                vadd.f64 vecSumx, vecSumx, q5x
                vadd.f64 vecSumy, vecSumy, q6x
                bne LoopBCols8_CompVMathMatrixMulABt_64f_Asm_NEON32\@
                EndOf_LoopBCols8_CompVMathMatrixMulABt_64f_Asm_NEON32\@:
                @@ EndOf_LoopBCols8_CompVMathMatrixMulABt_64f_Asm_NEON32 @@

            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            @ if (k < bColsSigned - 3)
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            and r9, bCols, #7 @ modulo 8
            lsrs r9, r9, #2
            beq EndOf_LoopBCols4_CompVMathMatrixMulABt_64f_Asm_NEON32\@
            LoopBCols4_CompVMathMatrixMulABt_64f_Asm_NEON32\@:
                vld1.64 {q1x, q1y, q2x, q2y}, [r10 :128]!
                vld1.64 {q3x, q3y, q4x, q4y}, [B_ :128]!
                vmul.f64 q5x, q1x, q3x
                vmul.f64 q5y, q1y, q3y
                .if \fusedMultiplyAdd
                    vfma.f64 q5x, q2x, q4x
                    vfma.f64 q5y, q2y, q4y
                .else
                    vmla.f64 q5x, q2x, q4x
                    vmla.f64 q5y, q2y, q4y
                .endif
                vadd.f64 vecSumx, vecSumx, q5x
                vadd.f64 vecSumy, vecSumy, q5y
                EndOf_LoopBCols4_CompVMathMatrixMulABt_64f_Asm_NEON32\@:
                @@ EndOf_LoopBCols4_CompVMathMatrixMulABt_64f_Asm_NEON32 @@

            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            @ for (; k < bColsSigned; k += 1)
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            ands r9, bCols, #3 @ modulo 4
            beq EndOf_LoopBCols1_CompVMathMatrixMulABt_64f_Asm_NEON32\@
            LoopBCols1_CompVMathMatrixMulABt_64f_Asm_NEON32\@:
                subs r9, r9, #1
                vld1.64 {q1x}, [r10 :64]!
                vld1.64 {q3x}, [B_ :64]!
                .if \fusedMultiplyAdd
                    vfma.f64 vecSumx, q1x, q3x
                .else
                    vmla.f64 vecSumx, q1x, q3x
                .endif
                bne LoopBCols1_CompVMathMatrixMulABt_64f_Asm_NEON32\@
                EndOf_LoopBCols1_CompVMathMatrixMulABt_64f_Asm_NEON32\@:
                @@ EndOf_LoopBCols1_CompVMathMatrixMulABt_64f_Asm_NEON32 @@

            vadd.f64 vecSumx, vecSumx, vecSumy
            vst1.64 {vecSumx}, [r11 :64]!
            subs bRows, bRows, #1
            add B_, B_, bStrideInBytes @ bStrideInBytes holds padInBytes
            bne LoopBRows_CompVMathMatrixMulABt_64f_Asm_NEON32\@
            @@ EndOf_LoopBRows_CompVMathMatrixMulABt_64f_Asm_NEON32 @@


        subs aRows, aRows, #1
        add A, A, aStrideInBytes
        add R, R, rStrideInBytes
        bne LoopARows_CompVMathMatrixMulABt_64f_Asm_NEON32\@
        @@ EndOf_LoopARows_CompVMathMatrixMulABt_64f_Asm_NEON32 @@

    .unreq A
	.unreq aRows
	.unreq aStrideInBytes
	.unreq B_
    .unreq bRows
    .unreq bCols
    .unreq bStrideInBytes
    .unreq R
    .unreq rStrideInBytes

    .unreq vecSum
    .unreq vecSumx
    .unreq vecSumy

    COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 9
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVMathMatrixMulABt_64f_Asm_NEON32
    CompVMathMatrixMulABt_64f_Macro_NEON32 0

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVMathMatrixMulABt_64f_Asm_FMA_NEON32
    CompVMathMatrixMulABt_64f_Macro_NEON32 1

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) compv_float64_t* ri
@ arg(1) -> COMPV_ALIGNED(NEON) compv_float64_t* rj
@ arg(2) -> const compv_float64_t* cos1
@ arg(3) -> const compv_float64_t* sin1 @ s1 is reserved name
@ arg(4) -> compv_uscalar_t count
.macro CompVMathMatrixMulGA_64f_Macro_NEON32 fusedMultiplyAdd
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 5
	COMPV_GAS_SAVE_NEON_REGS

	@@ Load arguments @@
	ldm_args r0-r4
	ri .req r0
	rj .req r1
	cos1 .req r2
	sin1 .req r3
    count .req r4

    vecRI0  .req q0
    vecRI0x .req q0x
    vecRI0y .req q0y
    vecRI1  .req q1
    vecRI1x .req q1x
    vecRI1y .req q1y
    vecRJ0  .req q2
    vecRJ0x .req q2x
    vecRJ0y .req q2y
    vecRJ1  .req q3
    vecRJ1x .req q3x
    vecRJ1y .req q3y
    vecC   .req q4x
    vecS   .req q4y

    vld1.64 {vecC}, [cos1]
    vld1.64 {vecS}, [sin1]

    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (i = 0; i < countSigned - 3; i += 4)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    lsrs r5, count, #2
    beq EndOf_Loop4_CompVMathMatrixMulGA_64f_Asm_NEON32\@
    Loop4_CompVMathMatrixMulGA_64f_Asm_NEON32\@:
        subs r5, r5, #1
        vld1.64 {vecRI0x, vecRI0y, vecRI1x, vecRI1y}, [ri :128]
        vld1.64 {vecRJ0x, vecRJ0y, vecRJ1x, vecRJ1y}, [rj :128]
        vmul.f64 q5x, vecRI0x, vecC
        vmul.f64 q5y, vecRI0y, vecC
        vmul.f64 q6x, vecRI1x, vecC
        vmul.f64 q6y, vecRI1y, vecC
        vmul.f64 q7x, vecRJ0x, vecC
        vmul.f64 q7y, vecRJ0y, vecC
        vmul.f64 q8x, vecRJ1x, vecC
        vmul.f64 q8y, vecRJ1y, vecC
        .if \fusedMultiplyAdd
            vfma.f64 q5x, vecRJ0x, vecS
            vfma.f64 q5y, vecRJ0y, vecS
            vfma.f64 q6x, vecRJ1x, vecS
            vfma.f64 q6y, vecRJ1y, vecS
            vfms.f64 q7x, vecRI0x, vecS
            vfms.f64 q7y, vecRI0y, vecS
            vfms.f64 q8x, vecRI1x, vecS
            vfms.f64 q8y, vecRI1y, vecS
        .else
            vmla.f64 q5x, vecRJ0x, vecS
            vmla.f64 q5y, vecRJ0y, vecS
            vmla.f64 q6x, vecRJ1x, vecS
            vmla.f64 q6y, vecRJ1y, vecS
            vmls.f64 q7x, vecRI0x, vecS
            vmls.f64 q7y, vecRI0y, vecS
            vmls.f64 q8x, vecRI1x, vecS
            vmls.f64 q8y, vecRI1y, vecS
        .endif
        vst1.64 {q5, q6}, [ri :128]!
        vst1.64 {q7, q8}, [rj :128]!
        bne Loop4_CompVMathMatrixMulGA_64f_Asm_NEON32\@
        EndOf_Loop4_CompVMathMatrixMulGA_64f_Asm_NEON32\@:
        @@ EndOf_Loop4_CompVMathMatrixMulGA_64f_Asm_NEON32 @@


    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (; i < countSigned; ++i)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    ands r5, count, #3 @ modulo 4
    beq EndOf_Loop1_CompVMathMatrixMulGA_64f_Asm_NEON32\@
    Loop1_CompVMathMatrixMulGA_64f_Asm_NEON32\@:
        subs r5, r5, #1
        vld1.64 {vecRI0x}, [ri :64]
        vld1.64 {vecRJ0x}, [rj :64]
        vmul.f64 q5x, vecRI0x, vecC
        vmul.f64 q7x, vecRJ0x, vecC
        .if \fusedMultiplyAdd
            vfma.f64 q5x, vecRJ0x, vecS
            vfms.f64 q7x, vecRI0x, vecS
        .else
            vmla.f64 q5x, vecRJ0x, vecS
            vmls.f64 q7x, vecRI0x, vecS
        .endif
        vst1.64 {q5x}, [ri :64]!
        vst1.64 {q7x}, [rj :64]!
        bne Loop1_CompVMathMatrixMulGA_64f_Asm_NEON32\@
        EndOf_Loop1_CompVMathMatrixMulGA_64f_Asm_NEON32\@:
        @@ EndOf_Loop1_CompVMathMatrixMulGA_64f_Asm_NEON32 @@
    
	.unreq ri
    .unreq rj
    .unreq cos1
    .unreq sin1
    .unreq count

    .unreq vecRI0
    .unreq vecRI0x
    .unreq vecRI0y
    .unreq vecRI1
    .unreq vecRI1x
    .unreq vecRI1y
    .unreq vecRJ0
    .unreq vecRJ0x
    .unreq vecRJ0y
    .unreq vecRJ1
    .unreq vecRJ1x
    .unreq vecRJ1y
    .unreq vecC
    .unreq vecS

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 5
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVMathMatrixMulGA_64f_Asm_NEON32
    CompVMathMatrixMulGA_64f_Macro_NEON32 0

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVMathMatrixMulGA_64f_Asm_FMA_NEON32
    CompVMathMatrixMulGA_64f_Macro_NEON32 1

#endif /* defined(__arm__) && !defined(__aarch64__) */

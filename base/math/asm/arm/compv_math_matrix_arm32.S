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

    vldr vecC, [cos1]
    vldr vecS, [sin1]

    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (i = 0; i < countSigned - 3; i += 4)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    lsrs r5, count, #2
    beq EndOf_Loop4_CompVMathMatrixMulGA_64f_Asm_NEON32\@
    Loop4_CompVMathMatrixMulGA_64f_Asm_NEON32\@:
        subs r5, r5, #1
        vld1.u8 {vecRI0, vecRI1}, [ri :128]
        vld1.u8 {vecRJ0, vecRJ1}, [rj :128]
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
        vld1.u8 {vecRI0x}, [ri :64]
        vld1.u8 {vecRJ0x}, [rj :64]
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

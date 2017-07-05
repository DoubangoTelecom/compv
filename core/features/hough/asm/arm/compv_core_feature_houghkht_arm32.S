#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__arm__) && !defined(__aarch64__)
.include "compv_common_arm32.S" @"

.data
.align 16
twoPi: .quad 0x401921fb54442d18, 0x401921fb54442d18
one: .quad 0x3ff0000000000000, 0x3ff0000000000000
four: .quad 0x4010000000000000, 0x4010000000000000
zeroDotOne: .quad 0x3fb999999999999a, 0x3fb999999999999a

.text

.equ vfpv4Enabled, 1
.equ vfpv4Disabled, 0

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const double* M_Eq14_r0
@ arg(1) -> COMPV_ALIGNED(NEON) const double* M_Eq14_0
@ arg(2) -> COMPV_ALIGNED(NEON) const double* M_Eq14_2
@ arg(3) -> COMPV_ALIGNED(NEON) const double* n_scale,
@ arg(4) -> COMPV_ALIGNED(NEON) double* sigma_rho_square
@ arg(5) -> COMPV_ALIGNED(NEON) double* sigma_rho_times_theta
@ arg(6) -> COMPV_ALIGNED(NEON) double* m2
@ arg(7) -> COMPV_ALIGNED(NEON) double* sigma_theta_square
@ arg(8) -> COMPV_ALIGNED(NEON) double* height
@ arg(9) -> COMPV_ALIGNED(NEON) double* heightMax1
@ arg(10) -> COMPV_ALIGNED(NEON) compv_uscalar_t count
.macro CompVHoughKhtKernelHeight_4mpq_Macro_NEON32 vfpv4
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 11
	COMPV_GAS_SAVE_NEON_REGS
	@@ end prolog @@

    @ Load arguments @
	ldm_args r0-r10

	M_Eq14_r0 .req r0
	M_Eq14_0 .req r1
	M_Eq14_2 .req r2
	n_scale .req r3
	sigma_rho_square .req r4
	sigma_rho_times_theta .req r5
	m2 .req r6
	sigma_theta_square .req r7
	height .req r8
	heightMax1 .req r9
	count .req r10
	t0 .req r11

	vecTwoPi .req d0
	vecOne .req d1
	vecFour .req d2
	vecZeroDotOne .req d3
	vecheightMax1 .req d4
	vecM_Eq14_0 .req d5
	vecM_Eq14_2 .req d6
	vecSigma_rho_square .req d7
	vecSigma_rho_times_sigma_theta .req d8
	vecSigma_rho_times_theta .req d9
	vecSigma_theta_square .req d10
	vecOne_minus_r_square .req d11
	vecHeight .req d12
	vecMaskEqZero .req d13
	vecTmp0 .req d14

    ldr t0, =twoPi
    vld1.f64 {vecTwoPi}, [t0 :64]
	ldr t0, =one
	vld1.f64 {vecOne}, [t0 :64]
    ldr t0, =four
	vld1.f64 {vecFour}, [t0 :64]
    ldr t0, =zeroDotOne
	vld1.f64 {vecZeroDotOne}, [t0 :64]

	lsl count, count, #3 @ convert from float64 to bytes

	vld1.f64 {vecheightMax1}, [heightMax1 :64]
	
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (compv_uscalar_t i = 0@ i < count@ i += 1)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopCount_CompVHoughKhtKernelHeight_4mpq_Asm_NEON32\vfpv4:
        vld1.f64 {vecTmp0}, [M_Eq14_r0 :64]!
		vdiv.f64 vecSigma_theta_square, vecOne, vecTmp0
		vld1.f64 {vecM_Eq14_0}, [M_Eq14_0 :64]!
		vld1.f64 {vecM_Eq14_2}, [M_Eq14_2 :64]!
		vmul.f64 vecSigma_rho_times_theta, vecSigma_theta_square, vecM_Eq14_0
		vmul.f64 vecSigma_theta_square, vecSigma_theta_square, vecM_Eq14_2
		.if \vfpv4 == vfpv4Disabled
            vld1.f64 {vecTmp0}, [n_scale :64]!
			vmul.f64 vecSigma_rho_square, vecSigma_rho_times_theta, vecM_Eq14_0
		.else
            vld1.f64 {vecSigma_rho_square}, [n_scale :64]!
            vfma.f64 vecSigma_rho_square, vecSigma_rho_times_theta, vecM_Eq14_0
		.endif
		vmul.f64 vecSigma_rho_times_theta, vecSigma_rho_times_theta, vecM_Eq14_2
		vmul.f64 vecM_Eq14_0, vecM_Eq14_0, vecSigma_theta_square
		vmul.f64 vecSigma_theta_square, vecSigma_theta_square, vecM_Eq14_2
		.if \vfpv4 == vfpv4Disabled
			vadd.f64 vecSigma_rho_square, vecSigma_rho_square, vecTmp0
		.endif
		vcmp.f64 vecSigma_theta_square, #0 @ AArch64, use 'vceqq_f64 vecMaskEqZero, vecSigma_theta_square, #0'
        vmrs   APSR_nzcv, fpscr
        vmoveq.f64 vecSigma_theta_square, vecZeroDotOne
		vmul.f64 vecSigma_rho_square, vecSigma_rho_square, vecFour
		vmul.f64 vecSigma_theta_square, vecSigma_theta_square, vecFour
		vsqrt.f64 vecSigma_rho_times_sigma_theta, vecSigma_rho_square
		vsqrt.f64 vecTmp0, vecSigma_theta_square
		vst1.f64 {vecSigma_rho_square}, [sigma_rho_square :64]!
		vst1.f64 {vecSigma_theta_square}, [sigma_theta_square :64]!
		vmul.f64 vecSigma_rho_times_sigma_theta, vecSigma_rho_times_sigma_theta, vecTmp0
		.if \vfpv4 == vfpv4Enabled
            vmov vecOne_minus_r_square, vecOne
			vdiv.f64 vectmp0, vecSigma_rho_times_theta, vecSigma_rho_times_sigma_theta
			vfms.f64 vecOne_minus_r_square, vectmp0, vectmp0
		.else
			vdiv.f64 vecTmp0, vecSigma_rho_times_theta, vecSigma_rho_times_sigma_theta
			vmul.f64 vecTmp0, vecTmp0, vecTmp0
			vsub.f64 vecOne_minus_r_square, vecOne, vecTmp0
		.endif
		vsqrt.f64 vecOne_minus_r_square, vecOne_minus_r_square
		vst1.f64 {vecSigma_rho_times_theta}, [sigma_rho_times_theta :64]!
		vst1.f64 {vecM_Eq14_0}, [m2 :64]!
		vmul.f64 vecOne_minus_r_square, vecOne_minus_r_square, vecSigma_rho_times_sigma_theta
		vmul.f64 vecOne_minus_r_square, vecOne_minus_r_square, vecTwoPi
		vdiv.f64 vecHeight, vecOne, vecOne_minus_r_square
        vcmp.f64 vecheightMax1, vecHeight
        vmrs   APSR_nzcv, fpscr
        vmovmi.f64 vecheightMax1, vecHeight
        subs count, count, #(1*COMPV_GAS_FLOAT64_SZ_BYTES) @ AArch64, move after the vdiv
		vst1.f64 {vecHeight}, [height :64]!
		
		@@ EndOf_LoopCount @@
		bne LoopCount_CompVHoughKhtKernelHeight_4mpq_Asm_NEON32\vfpv4
	
	vst1.f64 {vecheightMax1}, [heightMax1: 64]

	.unreq M_Eq14_r0				
	.unreq M_Eq14_0				
	.unreq M_Eq14_2				
	.unreq n_scale					
	.unreq sigma_rho_square		
	.unreq sigma_rho_times_theta	
	.unreq m2						
	.unreq sigma_theta_square		
	.unreq height					
	.unreq heightMax1				
	.unreq count					
	.unreq t0						

	.unreq vecTwoPi						
	.unreq vecOne							
	.unreq vecFour							
	.unreq vecZeroDotOne									
	.unreq vecheightMax1					
	.unreq vecM_Eq14_0						
	.unreq vecM_Eq14_2						
	.unreq vecSigma_rho_square				
	.unreq vecSigma_rho_times_sigma_theta	
	.unreq vecSigma_rho_times_theta		
	.unreq vecSigma_theta_square			
	.unreq vecOne_minus_r_square			
	.unreq vecHeight						
	.unreq vecMaskEqZero				
	.unreq vecTmp0					

	@@ begin epilog @@
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 11
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVHoughKhtKernelHeight_4mpq_Asm_NEON32
    CompVHoughKhtKernelHeight_4mpq_Macro_NEON32 vfpv4Enabled

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVHoughKhtKernelHeight_4mpq_Asm_VFPV4_NEON32
    CompVHoughKhtKernelHeight_4mpq_Macro_NEON32 vfpv4Disabled

#endif /* defined(__arm__) && !defined(__aarch64__) */

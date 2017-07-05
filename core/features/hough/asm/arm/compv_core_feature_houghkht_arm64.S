#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__aarch64__)
.include "compv_common_arm64.S" //"

.data

.text

.equ vfpv4Enabled, 1
.equ vfpv4Disabled, 0

#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) const double* M_Eq14_r0
# arg(1) -> COMPV_ALIGNED(NEON) const double* M_Eq14_0
# arg(2) -> COMPV_ALIGNED(NEON) const double* M_Eq14_2
# arg(3) -> COMPV_ALIGNED(NEON) const double* n_scale,
# arg(4) -> COMPV_ALIGNED(NEON) double* sigma_rho_square
# arg(5) -> COMPV_ALIGNED(NEON) double* sigma_rho_times_theta
# arg(6) -> COMPV_ALIGNED(NEON) double* m2
# arg(7) -> COMPV_ALIGNED(NEON) double* sigma_theta_square
# arg(8) -> COMPV_ALIGNED(NEON) double* height
# arg(9) -> COMPV_ALIGNED(NEON) double* heightMax1
# arg(10) -> COMPV_ALIGNED(NEON) compv_uscalar_t count
COMPV_GAS_FUNCTION_DECLARE CompVHoughKhtKernelHeight_4mpq_Asm_NEON64
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 11
	COMPV_GAS_SAVE_NEON_REGS
	## end prolog ##

    # Load arguments #
	ldp_arg 0, r0, r1
    ldp_arg 2, r2, r3
    ldp_arg 4, r4, r5
    ldp_arg 6, r6, r7
    ldp_arg 8, r8, r9
    ldr_arg 10, r10

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

	vecTwoPi .req v0
	vecOne .req v1
	vecFour .req v2
	vecZeroDotOne .req v3
	vecheightMax1 .req v4
    vecheightMax1n .req d4
	vecM_Eq14_0 .req v5
	vecM_Eq14_2 .req v6
	vecSigma_rho_square .req v7
	vecSigma_rho_times_sigma_theta .req v8
	vecSigma_rho_times_theta .req v9
	vecSigma_theta_square .req v10
	vecOne_minus_r_square .req v11
	vecHeight .req v12
	vecMaskEqZero .req v13
	vecTmp0 .req v14

	lsl count, count, #3 // convert from float64 to bytes

	ldr r11, [heightMax1]
    movz r12, #0x4019, lsl #48
    movz r13, #0x3ff0, lsl #48
    movz r14, #0x4010, lsl #48
    movz r15, #0x3fb9, lsl #48
    movk r12, #0x21fb, lsl #32
    movk r15, #0x9999, lsl #32
    movk r12, #0x5444, lsl #16
    movk r15, #0x9999, lsl #16
    movk r12, #0x2d18
    movk r15, #0x999a
    dup vecheightMax1.2d, r11
    dup vecTwoPi.2d, r12
    dup vecOne.2d, r13
    dup vecFour.2d, r14
    dup vecZeroDotOne.2d, r15
	
	##################################################
	# for (compv_uscalar_t i = 0# i < count# i += 1)
	##################################################
	LoopCount_CompVHoughKhtKernelHeight_4mpq_Asm_NEON64:
        ld1 {vecTmp0.2d}, [M_Eq14_r0], #(2*COMPV_GAS_FLOAT64_SZ_BYTES)
		fdiv vecSigma_theta_square.2d, vecOne.2d, vecTmp0.2d
		ld1 {vecM_Eq14_0.2d}, [M_Eq14_0], #(2*COMPV_GAS_FLOAT64_SZ_BYTES)
		ld1 {vecM_Eq14_2.2d}, [M_Eq14_2],#(2*COMPV_GAS_FLOAT64_SZ_BYTES)
		fmul vecSigma_rho_times_theta.2d, vecSigma_theta_square.2d, vecM_Eq14_0.2d
		fmul vecSigma_theta_square.2d, vecSigma_theta_square.2d, vecM_Eq14_2.2d
        ld1 {vecSigma_rho_square.2d}, [n_scale], #(2*COMPV_GAS_FLOAT64_SZ_BYTES)
        fmla vecSigma_rho_square.2d, vecSigma_rho_times_theta.2d, vecM_Eq14_0.2d
		fmul vecSigma_rho_times_theta.2d, vecSigma_rho_times_theta.2d, vecM_Eq14_2.2d
		fmul vecM_Eq14_0.2d, vecM_Eq14_0.2d, vecSigma_theta_square.2d
        cmeq vecMaskEqZero.2d, vecSigma_theta_square.2d, #0
        fmul vecSigma_rho_square.2d, vecSigma_rho_square.2d, vecFour.2d
		fmul vecSigma_theta_square.2d, vecSigma_theta_square.2d, vecM_Eq14_2.2d	
		bic vecSigma_theta_square.16b, vecSigma_theta_square.16b, vecMaskEqZero.16b
		and vecMaskEqZero.16b, vecMaskEqZero.16b, vecZeroDotOne.16b
		orr vecSigma_theta_square.16b, vecSigma_theta_square.16b, vecMaskEqZero.16b
		fmul vecSigma_theta_square.2d, vecSigma_theta_square.2d, vecFour.2d
		fsqrt vecSigma_rho_times_sigma_theta.2d, vecSigma_rho_square.2d
		fsqrt vecTmp0.2d, vecSigma_theta_square.2d
		st1 {vecSigma_rho_square.2d}, [sigma_rho_square], #(2*COMPV_GAS_FLOAT64_SZ_BYTES)
		st1 {vecSigma_theta_square.2d}, [sigma_theta_square], #(2*COMPV_GAS_FLOAT64_SZ_BYTES)
		fmul vecSigma_rho_times_sigma_theta.2d, vecSigma_rho_times_sigma_theta.2d, vecTmp0.2d
        fdiv vecTmp0.2d, vecSigma_rho_times_theta.2d, vecSigma_rho_times_sigma_theta.2d
        mov vecOne_minus_r_square.2d, vecOne.2d
        fmls vecOne_minus_r_square.2d, vecTmp0.2d, vecTmp0.2d
        st1 {vecSigma_rho_times_theta.2d}, [sigma_rho_times_theta], #(2*COMPV_GAS_FLOAT64_SZ_BYTES)
		fsqrt vecOne_minus_r_square.2d, vecOne_minus_r_square.2d
		st1 {vecM_Eq14_0.2d}, [m2], #(2*COMPV_GAS_FLOAT64_SZ_BYTES)
		fmul vecOne_minus_r_square.2d, vecOne_minus_r_square.2d, vecSigma_rho_times_sigma_theta.2d
		fmul vecOne_minus_r_square.2d, vecOne_minus_r_square.2d, vecTwoPi.2d
		fdiv vecHeight.2d, vecOne.2d, vecOne_minus_r_square.2d
        subs count, count, #(2*COMPV_GAS_FLOAT64_SZ_BYTES)
		fmax vecheightMax1.2d, vecheightMax1.2d, vecHeight.2d
        
		st1 {vecHeight.2d}, [height], #(2*COMPV_GAS_FLOAT64_SZ_BYTES)
		
		## EndOf_LoopCount ##
		bne LoopCount_CompVHoughKhtKernelHeight_4mpq_Asm_NEON64
	
	fmaxp vecheightMax1.2d, vecheightMax1.2d, vecheightMax1.2d
	st1 {vecheightMax1.1d}, [heightMax1]

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

	.unreq vecTwoPi						
	.unreq vecOne							
	.unreq vecFour							
	.unreq vecZeroDotOne									
	.unreq vecheightMax1	
    .unreq vecheightMax1n				
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

	## begin epilog ##
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#endif /* defined(__aarch64__) */
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(CompVHoughKhtKernelHeight_2mpq_Asm_X64_SSE2)

section .data
align 16
twoPi dq 0x401921fb54442d18, 0x401921fb54442d18
one dq 0x3ff0000000000000, 0x3ff0000000000000
four dq 0x4010000000000000, 0x4010000000000000
zeroDotOne dq 0x3fb999999999999a, 0x3fb999999999999a
	
section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const double* M_Eq14_r0
; arg(1) -> COMPV_ALIGNED(SSE) const double* M_Eq14_0
; arg(2) -> COMPV_ALIGNED(SSE) const double* M_Eq14_2
; arg(3) -> COMPV_ALIGNED(SSE) const double* n_scale,
; arg(4) -> COMPV_ALIGNED(SSE) double* sigma_rho_square
; arg(5) -> COMPV_ALIGNED(SSE) double* sigma_rho_times_theta
; arg(6) -> COMPV_ALIGNED(SSE) double* m2
; arg(7) -> COMPV_ALIGNED(SSE) double* sigma_theta_square
; arg(8) -> COMPV_ALIGNED(SSE) double* height
; arg(9) -> COMPV_ALIGNED(SSE) double* heightMax1
; arg(10) -> COMPV_ALIGNED(SSE) compv_uscalar_t count
sym(CompVHoughKhtKernelHeight_2mpq_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 11
	COMPV_YASM_SAVE_XMM 15
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	;; end prolog ;;

	%define M_Eq14_r0				rax
	%define M_Eq14_0				rcx
	%define M_Eq14_2				rdx
	%define n_scale					rbx
	%define sigma_rho_square		rsi
	%define sigma_rho_times_theta	rdi
	%define m2						r8
	%define sigma_theta_square		r9
	%define height					r10
	%define heightMax1				r11
	%define count					r12
	%define i						r13

	%define vecTwoPi						xmm0
	%define vecOne							xmm1
	%define vecFour							xmm2
	%define vecZeroDotOne					xmm3
	%define vecZero							xmm4
	%define vecheightMax1					xmm5
	%define vecM_Eq14_0						xmm6
	%define vecM_Eq14_2						xmm7
	%define vecSigma_rho_square				xmm8
	%define vecSigma_rho_times_sigma_theta	xmm9
	%define vecSigma_rho_times_theta		xmm10
	%define vecSigma_theta_square			xmm11
	%define vecOne_minus_r_square			xmm12
	%define vecHeight						xmm13
	%define vecMaskEqZero					xmm14
	%define vecTmp0							xmm15

	movapd vecTwoPi, [twoPi]
	movapd vecOne, [one]
	movapd vecFour, [four]
	movapd vecZeroDotOne, [zeroDotOne]
	xorpd vecZero, vecZero

	mov M_Eq14_r0, arg(0)				
	mov M_Eq14_0, arg(1)				
	mov M_Eq14_2, arg(2)				
	mov n_scale, arg(3)				
	mov sigma_rho_square, arg(4)		
	mov sigma_rho_times_theta, arg(5)	
	mov m2, arg(6)						
	mov sigma_theta_square, arg(7)		
	mov height, arg(8)					
	mov heightMax1, arg(9)				
	mov count, arg(10)

	lea count, [count*COMPV_YASM_FLOAT64_SZ_BYTES] ; convert from float64 to bytes

	movsd vecheightMax1, [heightMax1]
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t i = 0; i < count; i += 2)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	xor i, i
	.LoopCount:
		movapd vecSigma_theta_square, vecOne
		divpd vecSigma_theta_square, [M_Eq14_r0 + i]
		movapd vecM_Eq14_0, [M_Eq14_0 + i]
		movapd vecM_Eq14_2, [M_Eq14_2 + i]
		movapd vecSigma_rho_times_theta, vecSigma_theta_square
		mulpd vecSigma_rho_times_theta, vecM_Eq14_0
		mulpd vecSigma_theta_square, vecM_Eq14_2
		movapd vecSigma_rho_square, vecSigma_rho_times_theta
		mulpd vecSigma_rho_square, vecM_Eq14_0
		addpd vecSigma_rho_square, [n_scale + i]
		mulpd vecSigma_rho_times_theta, vecM_Eq14_2
		mulpd vecM_Eq14_0, vecSigma_theta_square
		mulpd vecSigma_theta_square, vecM_Eq14_2
		movapd vecMaskEqZero, vecZero
		cmpneqpd vecMaskEqZero, vecSigma_theta_square
		andpd vecSigma_theta_square, vecMaskEqZero
		andnpd vecMaskEqZero, vecZeroDotOne
		orpd vecSigma_theta_square, vecMaskEqZero
		mulpd vecSigma_rho_square, vecFour
		mulpd vecSigma_theta_square, vecFour
		sqrtpd vecSigma_rho_times_sigma_theta, vecSigma_rho_square
		sqrtpd vecTmp0, vecSigma_theta_square
		mulpd vecSigma_rho_times_sigma_theta, vecTmp0
		movapd vecTmp0, vecSigma_rho_times_theta
		divpd vecTmp0, vecSigma_rho_times_sigma_theta
		movapd vecOne_minus_r_square, vecOne
		mulpd vecTmp0, vecTmp0
		subpd vecOne_minus_r_square, vecTmp0
		sqrtpd vecOne_minus_r_square, vecOne_minus_r_square
		mulpd vecOne_minus_r_square, vecSigma_rho_times_sigma_theta
		mulpd vecOne_minus_r_square, vecTwoPi
		movapd vecHeight, vecOne
		divpd vecHeight, vecOne_minus_r_square

		movapd [sigma_rho_square + i], vecSigma_rho_square
		movapd [sigma_rho_times_theta + i], vecSigma_rho_times_theta
		movapd [m2 + i], vecM_Eq14_0
		movapd [sigma_theta_square + i], vecSigma_theta_square
		movapd [height + i], vecHeight

		maxpd vecheightMax1, vecHeight
		
		add i, (2*COMPV_YASM_FLOAT64_SZ_BYTES)
		cmp i, count
		;; EndOf_LoopCount ;;
		jl .LoopCount

	movapd vecTmp0, vecheightMax1
	shufpd vecTmp0, vecTmp0, 0x11
	maxsd vecheightMax1, vecTmp0
	movsd [heightMax1], vecheightMax1

	;; begin epilog ;;
	pop r13
	pop r12
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret


	

%endif ; COMPV_YASM_ABI_IS_64BIT
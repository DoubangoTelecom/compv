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

global sym(CompVHoughKhtKernelHeight_4mpq_Asm_X64_AVX)
global sym(CompVHoughKhtKernelHeight_4mpq_Asm_X64_FMA3_AVX)

section .data
align 32
twoPi dq 0x401921fb54442d18, 0x401921fb54442d18, 0x401921fb54442d18, 0x401921fb54442d18
one dq 0x3ff0000000000000, 0x3ff0000000000000, 0x3ff0000000000000, 0x3ff0000000000000
four dq 0x4010000000000000, 0x4010000000000000, 0x4010000000000000, 0x4010000000000000
zeroDotOne dq 0x3fb999999999999a, 0x3fb999999999999a, 0x3fb999999999999a, 0x3fb999999999999a
	
section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(AVX) const double* M_Eq14_r0
; arg(1) -> COMPV_ALIGNED(AVX) const double* M_Eq14_0
; arg(2) -> COMPV_ALIGNED(AVX) const double* M_Eq14_2
; arg(3) -> COMPV_ALIGNED(AVX) const double* n_scale,
; arg(4) -> COMPV_ALIGNED(AVX) double* sigma_rho_square
; arg(5) -> COMPV_ALIGNED(AVX) double* sigma_rho_times_theta
; arg(6) -> COMPV_ALIGNED(AVX) double* m2
; arg(7) -> COMPV_ALIGNED(AVX) double* sigma_theta_square
; arg(8) -> COMPV_ALIGNED(AVX) double* height
; arg(9) -> COMPV_ALIGNED(AVX) double* heightMax1
; arg(10) -> COMPV_ALIGNED(AVX) compv_uscalar_t count
; %1 -> 0: FMA3 supported, 0: FMA3 not supported
%macro CompVHoughKhtKernelHeight_4mpq_Macro_X64_AVX 1
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 11
	COMPV_YASM_SAVE_YMM 14
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

	%define vecTwoPi						ymm0
	%define vecOne							ymm1
	%define vecFour							ymm2
	%define vecZeroDotOne					ymm3
	%define vecheightMax1					ymm4
	%define vecheightMax1n					xmm4
	%define vecM_Eq14_0						ymm5
	%define vecM_Eq14_2						ymm6
	%define vecSigma_rho_square				ymm7
	%define vecSigma_rho_times_sigma_theta	ymm8
	%define vecSigma_rho_times_theta		ymm9
	%define vecSigma_theta_square			ymm10
	%define vecOne_minus_r_square			ymm11
	%define vecHeight						ymm12
	%define vecMaskEqZero					ymm13
	%define vecTmp0							ymm14
	%define vecTmp0n						xmm14

	vmovapd vecTwoPi, [twoPi]
	vmovapd vecOne, [one]
	vmovapd vecFour, [four]
	vmovapd vecZeroDotOne, [zeroDotOne]

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

	vbroadcastsd vecheightMax1, [heightMax1]
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t i = 0; i < count; i += 4)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	xor i, i
	.LoopCount:
		vdivpd vecSigma_theta_square, vecOne, [M_Eq14_r0 + i]
		vxorpd vecMaskEqZero, vecMaskEqZero
		vmovapd vecM_Eq14_0, [M_Eq14_0 + i]
		vmovapd vecM_Eq14_2, [M_Eq14_2 + i]
		vmulpd vecSigma_rho_times_theta, vecSigma_theta_square, vecM_Eq14_0
		vmulpd vecSigma_theta_square, vecSigma_theta_square, vecM_Eq14_2
		%if %1 == 0
			vmulpd vecSigma_rho_square, vecSigma_rho_times_theta, vecM_Eq14_0
		%else
			vmovapd vecSigma_rho_square, vecSigma_rho_times_theta
		%endif

		vmulpd vecSigma_rho_times_theta, vecSigma_rho_times_theta, vecM_Eq14_2
		%if %1 == 1
			vfmadd213pd vecSigma_rho_square, vecM_Eq14_0, [n_scale + i]
		%endif
		vmulpd vecM_Eq14_0, vecM_Eq14_0, vecSigma_theta_square
		vmulpd vecSigma_theta_square, vecSigma_theta_square, vecM_Eq14_2
		%if %1 == 0
			vaddpd vecSigma_rho_square, vecSigma_rho_square, [n_scale + i]
		%endif	
		vcmppd vecMaskEqZero, vecMaskEqZero, vecSigma_theta_square, 0xc
		vandpd vecSigma_theta_square, vecSigma_theta_square, vecMaskEqZero
		vmulpd vecSigma_rho_square, vecSigma_rho_square, vecFour
		vandnpd vecMaskEqZero, vecMaskEqZero, vecZeroDotOne
		vorpd vecSigma_theta_square, vecSigma_theta_square, vecMaskEqZero
		vmulpd vecSigma_theta_square, vecSigma_theta_square, vecFour
		vsqrtpd vecSigma_rho_times_sigma_theta, vecSigma_rho_square
		vsqrtpd vecTmp0, vecSigma_theta_square
		vmovapd [sigma_rho_square + i], vecSigma_rho_square
		vmovapd [sigma_theta_square + i], vecSigma_theta_square
		vmulpd vecSigma_rho_times_sigma_theta, vecSigma_rho_times_sigma_theta, vecTmp0
		%if %1 == 1
			vdivpd vecOne_minus_r_square, vecSigma_rho_times_theta, vecSigma_rho_times_sigma_theta
			vfnmadd213pd vecOne_minus_r_square, vecOne_minus_r_square, vecOne
		%else
			vdivpd vecTmp0, vecSigma_rho_times_theta, vecSigma_rho_times_sigma_theta
			vmulpd vecTmp0, vecTmp0, vecTmp0
			vsubpd vecOne_minus_r_square, vecOne, vecTmp0
		%endif
		vsqrtpd vecOne_minus_r_square, vecOne_minus_r_square
		vmovapd [sigma_rho_times_theta + i], vecSigma_rho_times_theta
		vmovapd [m2 + i], vecM_Eq14_0
		vmulpd vecOne_minus_r_square, vecOne_minus_r_square, vecSigma_rho_times_sigma_theta
		vmulpd vecOne_minus_r_square, vecOne_minus_r_square, vecTwoPi
		vdivpd vecHeight, vecOne, vecOne_minus_r_square
		add i, (4*COMPV_YASM_FLOAT64_SZ_BYTES)
		cmp i, count
		vmaxpd vecheightMax1, vecheightMax1, vecHeight
		vmovapd [height + i - (4*COMPV_YASM_FLOAT64_SZ_BYTES)], vecHeight
		
		;; EndOf_LoopCount ;;
		jl .LoopCount

	vextractf128 vecTmp0n, vecheightMax1, 0x1
	vmaxpd vecheightMax1n, vecheightMax1n, vecTmp0n
	vshufpd vecTmp0n, vecheightMax1n, vecheightMax1n, 0x11
	vmaxsd vecheightMax1n, vecheightMax1n, vecTmp0n
	vmovsd [heightMax1], vecheightMax1n

	%undef M_Eq14_r0				
	%undef M_Eq14_0				
	%undef M_Eq14_2				
	%undef n_scale					
	%undef sigma_rho_square		
	%undef sigma_rho_times_theta	
	%undef m2						
	%undef sigma_theta_square		
	%undef height					
	%undef heightMax1				
	%undef count					
	%undef i						

	%undef vecTwoPi						
	%undef vecOne							
	%undef vecFour							
	%undef vecZeroDotOne									
	%undef vecheightMax1		
	%undef vecheightMax1n			
	%undef vecM_Eq14_0						
	%undef vecM_Eq14_2						
	%undef vecSigma_rho_square				
	%undef vecSigma_rho_times_sigma_theta	
	%undef vecSigma_rho_times_theta		
	%undef vecSigma_theta_square			
	%undef vecOne_minus_r_square			
	%undef vecHeight						
	%undef vecMaskEqZero					
	%undef vecTmp0
	%undef vecTmp0n						

	;; begin epilog ;;
	pop r13
	pop r12
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_YMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVHoughKhtKernelHeight_4mpq_Asm_X64_AVX):
	CompVHoughKhtKernelHeight_4mpq_Macro_X64_AVX 0

sym(CompVHoughKhtKernelHeight_4mpq_Asm_X64_FMA3_AVX)
	CompVHoughKhtKernelHeight_4mpq_Macro_X64_AVX 1

%endif ; COMPV_YASM_ABI_IS_64BIT
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(CompVMathTrigFastAtan2_32f_Asm_X64_AVX)
global sym(CompVMathTrigFastAtan2_32f_Asm_X64_FMA3_AVX)

section .data
	extern sym(kAtan2Eps_32f)
	extern sym(kAtan2P1_32f)
	extern sym(kAtan2P3_32f)
	extern sym(kAtan2P5_32f)
	extern sym(kAtan2P7_32f)
	extern sym(k90_32f)
	extern sym(k180_32f)
	extern sym(k360_32f)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(AVX) const compv_float32_t* y
; arg(1) -> COMPV_ALIGNED(AVX) const compv_float32_t* x
; arg(2) -> COMPV_ALIGNED(AVX) compv_float32_t* r
; arg(3) -> const compv_float32_t* scale1
; arg(4) -> compv_uscalar_t width
; arg(5) -> compv_uscalar_t height
; arg(6) -> COMPV_ALIGNED(AVX) compv_uscalar_t stride
; %1 -> 0: FMA3 not supported, 1: FMA3 supported
%macro CompVMathTrigFastAtan2_32f_Macro_X64_AVX 1
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_YMM 15
	;; end prolog ;;

	%define y				rax
	%define x				rcx
	%define r				rdx
	%define width			r8
	%define height			r9
	%define stride			r10
	%define i				r11

	%define vecAtan2_zero		ymm0
	%define vecAtan2_sign		ymm1
	%define vecAtan2_signn		xmm1
	%define vecAtan2_scale		ymm2
	%define vecAtan2_scalen		xmm2
	%define vecAtan2_plus90		ymm3
	%define vecAtan2_plus180	ymm4
	%define vecAtan2_plus360	ymm5
	%define vecAtan2_eps		ymm6
	%define vecAtan2_p7			ymm7
	%define vecC				ymm8
	%define vecC2				ymm9
	%define vecAx				ymm10
	%define vecAy				ymm11
	%define vecMask				ymm12
	%define vec0				ymm13
	%define vec1				ymm14
	%define vec2				ymm15

	vxorps vecAtan2_zero, vecAtan2_zero

	mov eax, 0x7fffffff
	vmovd vecAtan2_signn, eax
	vbroadcastss vecAtan2_sign, vecAtan2_signn

	mov rax, arg(3)
	vmovss vecAtan2_scalen, [rax]
	vbroadcastss vecAtan2_scale, vecAtan2_scalen

	vmovaps vecAtan2_plus90, [sym(k90_32f)]
	vmovaps vecAtan2_plus180, [sym(k180_32f)]
	vmovaps vecAtan2_plus360, [sym(k360_32f)]
	vmovaps vecAtan2_eps, [sym(kAtan2Eps_32f)]
	vmovaps vecAtan2_p7, [sym(kAtan2P7_32f)]

	mov y, arg(0)
	mov x, arg(1)
	mov r, arg(2)
	mov width, arg(4)
	mov height, arg(5)
	mov stride, arg(6)

	; Convert stride from float-unit to byte-unit
	lea stride, [stride * COMPV_YASM_FLOAT32_SZ_BYTES]
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (compv_uscalar_t i = 0; i < width; i += 8)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		.LoopWidth:
			;; ax = std::abs(x[i]), ay = std::abs(y[i]) ;;
			vandps vecAx, vecAtan2_sign, [x + i * COMPV_YASM_FLOAT32_SZ_BYTES]
			vandps vecAy, vecAtan2_sign, [y + i * COMPV_YASM_FLOAT32_SZ_BYTES]

			;; if (ax >= ay) vec1 = ay, vec2 = ax ;;
			;; else vec1 = ax, vec2 = ay ;;
			vcmpgeps vecMask, vecAx, vecAy
			vandps vec1, vecMask, vecAy
			vandps vec2, vecMask, vecAx
			vandnps vecC, vecMask, vecAx
			vandnps vecC2, vecMask, vecAy
			vorps vecC, vecC, vec1
			vorps vecC2, vecC2, vec2

			;; c = vec1 / (vec2 + atan2_eps) ;;
			;; c2 = c*c ;;
			vaddps vecC2, vecC2, vecAtan2_eps
			vdivps vecC, vecC, vecC2
			vmulps vecC2, vecC, vecC

			;; a = (((atan2_p7*c2 + atan2_p5)*c2 + atan2_p3)*c2 + atan2_p1)*c ;;
			%if %1
				vmovaps vec0, vecAtan2_p7
				vfmadd213ps vec0, vecC2, [sym(kAtan2P5_32f)]
				vfmadd213ps vec0, vecC2, [sym(kAtan2P3_32f)]
				vfmadd213ps vec0, vecC2, [sym(kAtan2P1_32f)]
				vmulps vec0, vec0, vecC
			%else
				vmulps vec0, vecC2, vecAtan2_p7
				vaddps vec0, vec0, [sym(kAtan2P5_32f)]
				vmulps vec0, vec0, vecC2
				vaddps vec0, vec0, [sym(kAtan2P3_32f)]
				vmulps vec0, vec0, vecC2
				vaddps vec0, vec0, [sym(kAtan2P1_32f)]
				vmulps vec0, vec0, vecC
			%endif

			;; if (!(ax >= ay)) a = 90 - a ;;
			vsubps vec2, vecAtan2_plus90, vec0
			vandps vec0, vec0, vecMask
			vandnps vecMask, vecMask, vec2
			vorps vec0, vec0, vecMask
			
			;; if (x[i] < 0) a = 180.f - a ;;
			vcmpgtps vecMask, vecAtan2_zero, [x + i * COMPV_YASM_FLOAT32_SZ_BYTES]
			vsubps vec2, vecAtan2_plus180, vec0
			vandps vec1, vecMask, vec2
			vandnps vecMask, vecMask, vec0
			vorps vec0, vec1, vecMask

			;; if (y[i + k] < 0) a = 360.f - a ;;
			vcmpgtps vecMask, vecAtan2_zero, [y + i * COMPV_YASM_FLOAT32_SZ_BYTES]
			vsubps vec2, vecAtan2_plus360, vec0
			vandps vec1, vecMask, vec2
			vandnps vecMask, vecMask, vec0
			vorps vec0, vec1, vecMask

			;; r[i] = a * scale ;;
			vmulps vec0, vec0, vecAtan2_scale
			vmovaps [r + i*COMPV_YASM_FLOAT32_SZ_BYTES], vec0
			
			add i, 8
			cmp i, width
			jl .LoopWidth
		.EndOf_LoopWidth:

		dec height
		lea y, [y + stride]
		lea x, [x + stride]
		lea r, [r + stride]
		jnz .LoopHeight
	.EndOf_LoopHeight:

	%undef y				
	%undef x				
	%undef r				
	%undef width			
	%undef height			
	%undef stride			
	%undef i

	%undef vecAtan2_zero		
	%undef vecAtan2_sign
	%undef vecAtan2_signn	
	%undef vecAtan2_scale
	%undef vecAtan2_scalen
	%undef vecAtan2_plus90		
	%undef vecAtan2_plus180	
	%undef vecAtan2_plus360	
	%undef vecAtan2_eps		
	%undef vecAtan2_p7			
	%undef vecC				
	%undef vecC2				
	%undef vecAx				
	%undef vecAy				
	%undef vecMask				
	%undef vec0				
	%undef vec1				
	%undef vec2				

	;; begin epilog ;;
	COMPV_YASM_RESTORE_YMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVMathTrigFastAtan2_32f_Asm_X64_AVX)
	CompVMathTrigFastAtan2_32f_Macro_X64_AVX 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVMathTrigFastAtan2_32f_Asm_X64_FMA3_AVX)
	CompVMathTrigFastAtan2_32f_Macro_X64_AVX 1


%endif ; COMPV_YASM_ABI_IS_64BIT

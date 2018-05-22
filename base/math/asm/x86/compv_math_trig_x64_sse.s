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

global sym(CompVMathTrigFastAtan2_32f_Asm_X64_SSE2)

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
; arg(0) -> COMPV_ALIGNED(SSE) const compv_float32_t* y
; arg(1) -> COMPV_ALIGNED(SSE) const compv_float32_t* x
; arg(2) -> COMPV_ALIGNED(SSE) compv_float32_t* r
; arg(3) -> const compv_float32_t* scale1
; arg(4) -> compv_uscalar_t width
; arg(5) -> compv_uscalar_t height
; arg(6) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
sym(CompVMathTrigFastAtan2_32f_Asm_X64_SSE2)
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_XMM 15
	;; end prolog ;;

	%define y				rax
	%define x				rcx
	%define r				rdx
	%define width			r8
	%define height			r9
	%define stride			r10
	%define i				r11

	%define vecAtan2_zero		xmm0
	%define vecAtan2_sign		xmm1
	%define vecAtan2_scale		xmm2
	%define vecAtan2_plus90		xmm3
	%define vecAtan2_plus180	xmm4
	%define vecAtan2_plus360	xmm5
	%define vecAtan2_eps		xmm6
	%define vecAtan2_p1			xmm7
	%define vecC				xmm8
	%define vecC2				xmm9
	%define vecAx				xmm10
	%define vecAy				xmm11
	%define vecMask				xmm12
	%define vec0				xmm13
	%define vec1				xmm14
	%define vec2				xmm15

	xorps vecAtan2_zero, vecAtan2_zero

	mov rax, 0x7fffffff
	movd vecAtan2_sign, rax
	shufps vecAtan2_sign, vecAtan2_sign, 0x0

	mov rax, arg(3)
	movss vecAtan2_scale, [rax]
	shufps vecAtan2_scale, vecAtan2_scale, 0x0	

	movaps vecAtan2_plus90, [sym(k90_32f)]
	movaps vecAtan2_plus180, [sym(k180_32f)]
	movaps vecAtan2_plus360, [sym(k360_32f)]
	movaps vecAtan2_eps, [sym(kAtan2Eps_32f)]
	movaps vecAtan2_p1, [sym(kAtan2P1_32f)]

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
		; for (compv_uscalar_t i = 0; i < width; i += 4)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		.LoopWidth:
			;; ax = std::abs(x[i]), ay = std::abs(y[i]) ;;
			movaps vecAx, [x + i * COMPV_YASM_FLOAT32_SZ_BYTES]
			movaps vecAy, [y + i * COMPV_YASM_FLOAT32_SZ_BYTES]
			andps vecAx, vecAtan2_sign
			andps vecAy, vecAtan2_sign

			;; if (ax >= ay) vec1 = ay, vec2 = ax ;;
			;; else vec1 = ax, vec2 = ay ;;
			movaps vecMask, vecAy
			cmpleps vecMask, vecAx ; using LE (LessEqual) to emulate GE (GreaterEqual) - args swapped
			movaps vec1, vecMask
			movaps vec2, vecMask
			movaps vecC, vecMask
			movaps vecC2, vecMask
			andps vec1, vecAy
			andps vec2, vecAx
			andnps vecC, vecAx
			andnps vecC2, vecAy
			orps vecC, vec1
			orps vecC2, vec2

			;; c = vec1 / (vec2 + atan2_eps) ;;
			;; c2 = c*c ;;
			addps vecC2, vecAtan2_eps
			divps vecC, vecC2
			movaps vecC2, vecC
			mulps vecC2, vecC2

			;; a = (((atan2_p7*c2 + atan2_p5)*c2 + atan2_p3)*c2 + atan2_p1)*c ;;
			movaps vec0, vecC2
			mulps vec0, [sym(kAtan2P7_32f)]
			addps vec0, [sym(kAtan2P5_32f)]
			mulps vec0, vecC2
			addps vec0, [sym(kAtan2P3_32f)]
			mulps vec0, vecC2
			addps vec0, vecAtan2_p1
			mulps vec0, vecC

			;; if (!(ax >= ay)) a = 90 - a ;;
			movaps vec2, vecAtan2_plus90
			subps vec2, vec0
			andps vec0, vecMask
			andnps vecMask, vec2
			orps vec0, vecMask
			
			;; if (x[i] < 0) a = 180.f - a ;;
			movaps vecMask, [x + i * COMPV_YASM_FLOAT32_SZ_BYTES]
			cmpltps vecMask, vecAtan2_zero
			movaps vec2, vecAtan2_plus180
			movaps vec1, vecMask
			subps vec2, vec0
			andnps vecMask, vec0
			andps vec1, vec2
			orps vec1, vecMask ; vec1 now contains result(vec0)

			;; if (y[i + k] < 0) a = 360.f - a ;;
			movaps vecMask, [y + i * COMPV_YASM_FLOAT32_SZ_BYTES]
			cmpltps vecMask, vecAtan2_zero
			movaps vec2, vecAtan2_plus360
			movaps vec0, vecMask
			subps vec2, vec1
			andnps vecMask, vec1
			andps vec0, vec2
			orps vec0, vecMask

			;; r[i] = a * scale ;;
			mulps vec0, vecAtan2_scale
			movaps [r + i*COMPV_YASM_FLOAT32_SZ_BYTES], vec0
			
			add i, 4
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
	%undef vecAtan2_scale
	%undef vecAtan2_plus90
	%undef vecAtan2_plus180
	%undef vecAtan2_plus360
	%undef vecAtan2_eps
	%undef vecAtan2_p1
	%undef vecC				
	%undef vecC2				
	%undef vecAx				
	%undef vecAy				
	%undef vecMask				
	%undef vec0				
	%undef vec1				
	%undef vec2				

	;; begin epilog ;;
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret


%endif ; COMPV_YASM_ABI_IS_64BIT

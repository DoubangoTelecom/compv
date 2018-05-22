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

global sym(CompVHogCommonNormL1_9_32f_Asm_X64_SSE2)
global sym(CompVHogCommonNormL1Sqrt_9_32f_Asm_X64_SSE2)
global sym(CompVHogCommonNormL2_9_32f_Asm_X64_SSE2)
global sym(CompVHogCommonNormL2Hys_9_32f_Asm_X64_SSE2)

section .data
	One			dd 1.0
	ZeroDot2	dd 0.2

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> compv_float32_t* inOutPtr
; arg(1) -> const compv_float32_t* eps1
; arg(2) -> const compv_uscalar_t count
; %1 -> 0: L1, 1: L1Sqrt
%macro CompVHogCommonNormL1_9_32f_Macro_X64_SSE2 1
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 3
	COMPV_YASM_SAVE_XMM 7
	;; end prolog ;;

	%define inOutPtr		rax
	%define eps1			rcx

	mov inOutPtr, arg(0)
	mov eps1, arg(1)

	movups xmm5, [inOutPtr + 0*COMPV_YASM_FLOAT32_SZ_BYTES]
	movups xmm6, [inOutPtr + 4*COMPV_YASM_FLOAT32_SZ_BYTES]
	movss xmm7, [inOutPtr + 8*COMPV_YASM_FLOAT32_SZ_BYTES]

	movaps xmm0, xmm5
	addps xmm0, xmm6
	movss xmm2, [sym(One)]
	movaps xmm3, xmm0
	shufps xmm3, xmm3, 0x0E
	addps xmm0, xmm3
	movaps xmm4, xmm0
	shufps xmm4, xmm4, 0x01
	addps xmm0, xmm4
	addss xmm0, xmm7
	addss xmm0, [eps1]
	divss xmm2, xmm0
	shufps xmm2, xmm2, 0x00

	mulps xmm5, xmm2
	mulps xmm6, xmm2
	mulss xmm7, xmm2

	%if %1
		sqrtps xmm5, xmm5
		sqrtps xmm6, xmm6
		sqrtss xmm7, xmm7
	%endif

	movups [inOutPtr + 0*COMPV_YASM_FLOAT32_SZ_BYTES], xmm5
	movups [inOutPtr + 4*COMPV_YASM_FLOAT32_SZ_BYTES], xmm6
	movss [inOutPtr + 8*COMPV_YASM_FLOAT32_SZ_BYTES], xmm7

	%undef inOutPtr
	%undef eps1

	;; begin epilog ;;
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVHogCommonNormL1_9_32f_Asm_X64_SSE2)
	CompVHogCommonNormL1_9_32f_Macro_X64_SSE2 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVHogCommonNormL1Sqrt_9_32f_Asm_X64_SSE2)
	CompVHogCommonNormL1_9_32f_Macro_X64_SSE2 1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> compv_float32_t* inOutPtr
; arg(1) -> const compv_float32_t* eps_square1
; arg(2) -> const compv_uscalar_t count
%macro CompVHogCommonNormL2_9_32f_Macro_X64_SSE2 0
	movaps xmm0, xmm5
	mulps xmm0, xmm0
	movaps xmm1, xmm6
	mulps xmm1, xmm1
	movss xmm2, [sym(One)]
	addps xmm0, xmm1
	movaps xmm3, xmm0
	shufps xmm3, xmm3, 0x0E
	addps xmm0, xmm3
	movaps xmm3, xmm7
	movaps xmm4, xmm0
	mulss xmm3, xmm3
	shufps xmm4, xmm4, 0x01
	addps xmm0, xmm4
	addss xmm0, xmm3
	addss xmm0, [eps_square1]
	sqrtss xmm0, xmm0
	divss xmm2, xmm0
	shufps xmm2, xmm2, 0x00
		
	mulps xmm5, xmm2
	mulps xmm6, xmm2
	mulss xmm7, xmm2
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> compv_float32_t* inOutPtr
; arg(1) -> const compv_float32_t* eps_square1
; arg(2) -> const compv_uscalar_t count
sym(CompVHogCommonNormL2_9_32f_Asm_X64_SSE2)
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 3
	COMPV_YASM_SAVE_XMM 7
	;; end prolog ;;

	%define inOutPtr		rax
	%define eps_square1		rcx

	mov inOutPtr, arg(0)
	mov eps_square1, arg(1)

	movups xmm5, [inOutPtr + 0*COMPV_YASM_FLOAT32_SZ_BYTES]
	movups xmm6, [inOutPtr + 4*COMPV_YASM_FLOAT32_SZ_BYTES]
	movss xmm7, [inOutPtr + 8*COMPV_YASM_FLOAT32_SZ_BYTES]
	
	CompVHogCommonNormL2_9_32f_Macro_X64_SSE2

	movups [inOutPtr + 0*COMPV_YASM_FLOAT32_SZ_BYTES], xmm5
	movups [inOutPtr + 4*COMPV_YASM_FLOAT32_SZ_BYTES], xmm6
	movss [inOutPtr + 8*COMPV_YASM_FLOAT32_SZ_BYTES], xmm7

	%undef inOutPtr
	%undef eps_square1

	;; begin epilog ;;
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> compv_float32_t* inOutPtr
; arg(1) -> const compv_float32_t* eps_square1
; arg(2) -> const compv_uscalar_t count
sym(CompVHogCommonNormL2Hys_9_32f_Asm_X64_SSE2)
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 3
	COMPV_YASM_SAVE_XMM 7
	;; end prolog ;;

	%define inOutPtr		rax
	%define eps_square1		rcx

	mov inOutPtr, arg(0)
	mov eps_square1, arg(1)

	movups xmm5, [inOutPtr + 0*COMPV_YASM_FLOAT32_SZ_BYTES]
	movups xmm6, [inOutPtr + 4*COMPV_YASM_FLOAT32_SZ_BYTES]
	movss xmm7, [inOutPtr + 8*COMPV_YASM_FLOAT32_SZ_BYTES]
	
	CompVHogCommonNormL2_9_32f_Macro_X64_SSE2
	movss xmm0, [sym(ZeroDot2)]
	shufps xmm0, xmm0, 0x00
	minps xmm5, xmm0
	minps xmm6, xmm0
	minss xmm7, xmm0
	CompVHogCommonNormL2_9_32f_Macro_X64_SSE2

	movups [inOutPtr + 0*COMPV_YASM_FLOAT32_SZ_BYTES], xmm5
	movups [inOutPtr + 4*COMPV_YASM_FLOAT32_SZ_BYTES], xmm6
	movss [inOutPtr + 8*COMPV_YASM_FLOAT32_SZ_BYTES], xmm7

	%undef inOutPtr
	%undef eps_square1

	;; begin epilog ;;
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

%endif ; COMPV_YASM_ABI_IS_64BIT

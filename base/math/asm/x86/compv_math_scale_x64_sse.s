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

global sym(CompVMathScaleScale_64f64f_Asm_X64_SSE2)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const compv_float64_t* ptrIn
; arg(1) -> compv_float64_t* ptrOut
; arg(2) -> const compv_uscalar_t width
; arg(3) -> const compv_uscalar_t height
; arg(4) -> const compv_uscalar_t stride
; arg(5) -> const compv_float64_t* s1
sym(CompVMathScaleScale_64f64f_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	COMPV_YASM_SAVE_XMM 8
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	%define ptrIn		rax
	%define ptrOut		rcx
	%define width		rdx
	%define height		rbx
	%define stride		rsi
	%define s1			rdi

	%define width16		r8
	%define width2		r9
	%define i			r10

	mov ptrIn, arg(0)
	mov ptrOut, arg(1)
	mov width, arg(2)
	mov height, arg(3)
	mov stride, arg(4)
	mov s1, arg(5)

	mov width16, width
	mov width2, width
	and width16, -16
	and width2, -2

	shl stride, COMPV_YASM_FLOAT64_SHIFT_BYTES

	movsd xmm8, [s1]
	shufpd xmm8, xmm8, 0x00

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width16; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		test width16, width16
		jz .EndOf_LoopWidth16
		.LoopWidth16:
			movupd xmm0, [ptrIn + ((i + 0) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			movupd xmm1, [ptrIn + ((i + 2) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			movupd xmm2, [ptrIn + ((i + 4) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			movupd xmm3, [ptrIn + ((i + 6) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			movupd xmm4, [ptrIn + ((i + 8) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			movupd xmm5, [ptrIn + ((i + 10) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			movupd xmm6, [ptrIn + ((i + 12) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			movupd xmm7, [ptrIn + ((i + 14) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			mulpd xmm0, xmm8
			mulpd xmm1, xmm8
			mulpd xmm2, xmm8
			mulpd xmm3, xmm8
			mulpd xmm4, xmm8
			mulpd xmm5, xmm8
			mulpd xmm6, xmm8
			mulpd xmm7, xmm8
			add i, 16
			cmp i, width16
			movupd [ptrOut + ((i + 0 - 16) * COMPV_YASM_FLOAT64_SZ_BYTES)], xmm0
			movupd [ptrOut + ((i + 2 - 16) * COMPV_YASM_FLOAT64_SZ_BYTES)], xmm1
			movupd [ptrOut + ((i + 4 - 16) * COMPV_YASM_FLOAT64_SZ_BYTES)], xmm2
			movupd [ptrOut + ((i + 6 - 16) * COMPV_YASM_FLOAT64_SZ_BYTES)], xmm3
			movupd [ptrOut + ((i + 8 - 16) * COMPV_YASM_FLOAT64_SZ_BYTES)], xmm4
			movupd [ptrOut + ((i + 10 - 16) * COMPV_YASM_FLOAT64_SZ_BYTES)], xmm5
			movupd [ptrOut + ((i + 12 - 16) * COMPV_YASM_FLOAT64_SZ_BYTES)], xmm6
			movupd [ptrOut + ((i + 14 - 16) * COMPV_YASM_FLOAT64_SZ_BYTES)], xmm7
			jl .LoopWidth16
		.EndOf_LoopWidth16:

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width2; i += 2)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width2
		jge .EndOf_LoopWidth2
		.LoopWidth2:
			movupd xmm0, [ptrIn + ((i + 0) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			mulpd xmm0, xmm8
			add i, 2
			cmp i, width2
			movupd [ptrOut + ((i + 0 - 2) * COMPV_YASM_FLOAT64_SZ_BYTES)], xmm0
			jl .LoopWidth2
		.EndOf_LoopWidth2:

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width; i += 1)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_LoopWidth1
		.LoopWidth1:
			movsd xmm0, [ptrIn + ((i + 0) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			mulsd xmm0, xmm8
			inc i
			cmp i, width
			movsd [ptrOut + ((i + 0 - 1) * COMPV_YASM_FLOAT64_SZ_BYTES)], xmm0
			jl .LoopWidth1
		.EndOf_LoopWidth1:
		
		dec height
		lea ptrIn, [ptrIn + stride]
		lea ptrOut, [ptrOut + stride]
		jnz .LoopHeight
	.EndOf_LoopHeight:

	%undef ptrIn		
	%undef ptrOut		
	%undef width		
	%undef height		
	%undef stride		
	%undef s1			

	%undef width16		
	%undef width2		
	%undef i			

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret


%endif ; COMPV_YASM_ABI_IS_64BIT

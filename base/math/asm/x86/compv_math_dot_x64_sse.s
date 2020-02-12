;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(CompVMathDotDotSub_64f64f_Asm_X64_SSE2)
global sym(CompVMathDotDot_64f64f_Asm_X64_SSE2)

section .data

section .text

%define DOT_TYPE_DOT		0
%define DOT_TYPE_DOTSUB		1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const compv_float64_t* ptrA
; arg(1) -> const compv_float64_t* ptrB
; arg(2) -> const compv_uscalar_t width
; arg(3) -> const compv_uscalar_t height
; arg(4) -> const compv_uscalar_t strideA
; arg(5) -> const compv_uscalar_t strideB
; arg(6) -> compv_float64_t* sum
; %1 -> DOT_TYPE_DOT
%macro CompVMathDot_64f64f_Macro_X64_SSE2	1
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_XMM 13
	push rsi
	push rdi
	push rbx
	;; end prolog ;;
	
	%define ptrA		rax
	%define ptrB		rcx
	%define width		rdx
	%define height		rbx
	%define strideA		rsi
	%define strideB		rdi
	%define sum			r8

	%define width16		r9
	%define width2		r10
	%define i			r11

	mov ptrA, arg(0)
	mov ptrB, arg(1)
	mov width, arg(2)
	mov height, arg(3)
	mov strideA, arg(4)
	mov strideB, arg(5)
	mov sum, arg(6)

	mov width16, width
	mov width2, width
	and width16, -16
	and width2, -2

	shl strideA, COMPV_YASM_FLOAT64_SHIFT_BYTES
	shl strideB, COMPV_YASM_FLOAT64_SHIFT_BYTES

	pxor xmm12, xmm12
	pxor xmm13, xmm13

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
			movupd xmm0, [ptrA + ((i + 0) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			movupd xmm1, [ptrA + ((i + 2) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			movupd xmm2, [ptrA + ((i + 4) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			movupd xmm3, [ptrA + ((i + 6) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			movupd xmm4, [ptrA + ((i + 8) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			movupd xmm5, [ptrA + ((i + 10) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			movupd xmm6, [ptrA + ((i + 12) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			movupd xmm7, [ptrA + ((i + 14) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			
			movupd xmm8, [ptrB + ((i + 0) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			movupd xmm9, [ptrB + ((i + 2) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			movupd xmm10, [ptrB + ((i + 4) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			movupd xmm11, [ptrB + ((i + 6) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			%if %1 == DOT_TYPE_DOTSUB
				subpd xmm0, xmm8
			%else
				mulpd xmm0, xmm8
			%endif
			movupd xmm8, [ptrB + ((i + 8) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			%if %1 == DOT_TYPE_DOTSUB
				subpd xmm1, xmm9
				subpd xmm4, xmm8
			%else
				mulpd xmm1, xmm9
				mulpd xmm4, xmm8
			%endif
			movupd xmm9, [ptrB + ((i + 10) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			%if %1 == DOT_TYPE_DOTSUB
				subpd xmm2, xmm10
				subpd xmm5, xmm9
			%else
				mulpd xmm2, xmm10
				mulpd xmm5, xmm9
			%endif
			movupd xmm10, [ptrB + ((i + 12) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			%if %1 == DOT_TYPE_DOTSUB
				subpd xmm3, xmm11
				subpd xmm6, xmm10
			%else
				mulpd xmm3, xmm11
				mulpd xmm6, xmm10
			%endif
			movupd xmm11, [ptrB + ((i + 14) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			%if %1 == DOT_TYPE_DOTSUB
				subpd xmm7, xmm11
			%else
				mulpd xmm7, xmm11
			%endif

			%if %1 == DOT_TYPE_DOTSUB
				mulpd xmm0, xmm0
				mulpd xmm2, xmm2
				mulpd xmm4, xmm4
				mulpd xmm6, xmm6
				mulpd xmm1, xmm1
				mulpd xmm3, xmm3
				mulpd xmm5, xmm5
				mulpd xmm7, xmm7
			%endif
			add i, 16
			cmp i, width16
			addpd xmm0, xmm2
			addpd xmm4, xmm6
			addpd xmm1, xmm3
			addpd xmm5, xmm7
			addpd xmm0, xmm4
			addpd xmm1, xmm5
			addpd xmm12, xmm0
			addpd xmm13, xmm1
			jl .LoopWidth16
		.EndOf_LoopWidth16:

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width2; i += 2)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width2
		jge .EndOf_LoopWidth2
		.LoopWidth2:
			movupd xmm0, [ptrA + ((i + 0) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			movupd xmm1, [ptrB + ((i + 0) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			%if %1 == DOT_TYPE_DOTSUB
				subpd xmm0, xmm1
				mulpd xmm0, xmm0
			%else
				mulpd xmm0, xmm1
			%endif
			add i, 2
			cmp i, width2
			addpd xmm12, xmm0 
			jl .LoopWidth2
		.EndOf_LoopWidth2:

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width; i += 1)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_LoopWidth1
		.LoopWidth1:
			movsd xmm0, [ptrA + ((i + 0) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			movsd xmm1, [ptrB + ((i + 0) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			%if %1 == DOT_TYPE_DOTSUB
				subsd xmm0, xmm1
				mulsd xmm0, xmm0
			%else
				mulsd xmm0, xmm1
			%endif
			inc i
			cmp i, width
			addsd xmm12, xmm0
			jl .LoopWidth1
		.EndOf_LoopWidth1:
		
		dec height
		lea ptrA, [ptrA + strideA]
		lea ptrB, [ptrB + strideB]
		jnz .LoopHeight
	.EndOf_LoopHeight:

	addpd xmm12, xmm13
	movapd xmm0, xmm12
	shufpd xmm0, xmm0, 0xff
	addsd xmm12, xmm0
	movsd [sum], xmm12

	%undef ptrA
	%undef ptrB
	%undef width
	%undef height
	%undef strideA
	%undef strideB
	%undef sum
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
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVMathDotDotSub_64f64f_Asm_X64_SSE2):
	CompVMathDot_64f64f_Macro_X64_SSE2 DOT_TYPE_DOTSUB

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVMathDotDot_64f64f_Asm_X64_SSE2):
	CompVMathDot_64f64f_Macro_X64_SSE2 DOT_TYPE_DOT

%endif ; COMPV_YASM_ABI_IS_64BIT

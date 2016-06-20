;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "../compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(MathStatsNormalize2DHartley_float64_Asm_X86_SSE2)

section .data
	extern sym(ksqrt2_f64)
	extern sym(k1_f64)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const COMPV_ALIGNED(SSE) compv_float64_t* x;
; arg(1) -> const COMPV_ALIGNED(SSE) compv_float64_t* y
; arg(2) -> compv_uscalar_t numPoints;
; arg(3) -> compv_float64_t* tx1
; arg(4) -> compv_float64_t* ty1
; arg(5) -> compv_float64_t* s1
; void MathStatsNormalize2DHartley_float64_Asm_X86_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* x, const COMPV_ALIGNED(SSE) compv_float64_t* y, compv_uscalar_t numPoints, compv_float64_t* tx1, compv_float64_t* ty1, compv_float64_t* s1)
sym(MathStatsNormalize2DHartley_float64_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	COMPV_YASM_SAVE_XMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	mov rbx, arg(0) ; rbx = x
	mov rdx, arg(1) ; rdx = y

	;-------------------------------------
	; TX and TY
	;-------------------------------------

	pxor xmm0, xmm0 ; xmm0 = xmmTx
	pxor xmm1, xmm1 ; xmm1 = xmmTy
	pxor xmm7, xmm7 ; xmm7 = xmmMagnitude
	xor rsi, rsi ; rsi = i

	mov rax, arg(2) ; rax = numPoints
	lea rcx, [rax - 7] ; rcx = (numPoints - 7)
	lea rdi, [rax - 3] ; rdx = (numPoints - 3)

	movd xmm2, rax
	pshufd xmm2, xmm2, 0x0
	movapd xmm6, [sym(k1_f64)]
	cvtdq2pd xmm2, xmm2
	divpd xmm6, xmm2 ; xmm6 = xmmOneOverNumPoints

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (i = 0; i < numPoints_ - 7; i += 8)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rsi, rcx
	jge .EndOfLoop8TxTy
	.Loop8TxTy
		movapd xmm2, [rbx + rsi * 8 + 0*8]
		movapd xmm3, [rbx + rsi * 8 + 4*8]
		movapd xmm4, [rdx + rsi * 8 + 0*8]
		movapd xmm5, [rdx + rsi * 8 + 4*8]
		addpd xmm2, [rbx + rsi * 8 + 2*8]
		addpd xmm3, [rbx + rsi * 8 + 6*8]
		addpd xmm4, [rdx + rsi * 8 + 2*8]
		addpd xmm5, [rdx + rsi * 8 + 6*8]
		lea rsi, [rsi + 8] ; i += 8
		addpd xmm2, xmm3
		addpd xmm4, xmm5
		addpd xmm0, xmm2
		addpd xmm1, xmm4
		cmp rsi, rcx
		jl .Loop8TxTy
	.EndOfLoop8TxTy

	lea rcx, [rax - 1] ; rcx = (numPoints - 1)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; if (i < numPoints_ - 3)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rsi, rdi
	jge .EndOfMoreThanFourTxTyRemains
	.MoreThanFourTxTyRemains
		movapd xmm2, [rbx + rsi * 8 + 0*8]
		movapd xmm3, [rdx + rsi * 8 + 0*8]
		addpd xmm2, [rbx + rsi * 8 + 2*8]
		addpd xmm3, [rdx + rsi * 8 + 2*8]
		lea rsi, [rsi + 4] ; i += 4
		addpd xmm0, xmm2
		addpd xmm1, xmm3
	.EndOfMoreThanFourTxTyRemains

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; if (i < numPoints_ - 1)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rsi, rcx
	jge .EndOfMoreThanTwoTxTyRemains
	.MoreThanTwoTxTyRemains
		lea rsi, [rsi + 2]
		addpd xmm0, [rbx + rsi * 8 + 0*8 - 16]
		addpd xmm1, [rdx + rsi * 8 + 0*8 - 16]
	.EndOfMoreThanTwoTxTyRemains

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; if (numPoints_ & 1)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	test rax, 1
	jz .EndOfMoreThanOneTxTyRemains
	.MoreThanOneTxTyRemains
		addsd xmm0, [rbx + rsi * 8 + 0*8]
		addsd xmm1, [rdx + rsi * 8 + 0*8]
	.EndOfMoreThanOneTxTyRemains

	xor rsi, rsi ; i = 0

	movapd xmm2, xmm0
	movapd xmm3, xmm1
	shufpd xmm2, xmm2, 0x1
	shufpd xmm3, xmm3, 0x1
	addsd xmm0, xmm2
	addsd xmm1, xmm3
	mulsd xmm0, xmm6
	mulsd xmm1, xmm6
	shufpd xmm0, xmm0, 0x0
	shufpd xmm1, xmm1, 0x0

	;-------------------------------------
	; Magnitude
	;-------------------------------------

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (i = 0; i < numPoints_ - 3; i += 4)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rsi, rdi
	jge .EndOfLoop4Magnitude
	.Loop4Magnitude
		movapd xmm2, [rbx + rsi * 8 + 0*8]
		movapd xmm3, [rbx + rsi * 8 + 2*8]
		movapd xmm4, [rdx + rsi * 8 + 0*8]
		movapd xmm5, [rdx + rsi * 8 + 2*8]
		subpd xmm2, xmm0
		subpd xmm3, xmm0
		subpd xmm4, xmm1
		subpd xmm5, xmm1
		mulpd xmm2, xmm2
		mulpd xmm4, xmm4
		mulpd xmm3, xmm3
		mulpd xmm5, xmm5
		addpd xmm2, xmm4
		addpd xmm3, xmm5
		lea rsi, [rsi + 4] ; i += 4
		sqrtpd xmm2, xmm2
		sqrtpd xmm3, xmm3
		addpd xmm7, xmm2
		cmp rsi, rdi
		addpd xmm7, xmm3
		jl .Loop4Magnitude
	.EndOfLoop4Magnitude

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; if (i < numPoints_ - 1)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rsi, rcx
	jge .EndOfMoreThanTwoMagnitudeRemains
	.MoreThanTwoMagnitudeRemains
		movapd xmm2, [rbx + rsi * 8 + 0*8]
		movapd xmm3, [rdx + rsi * 8 + 0*8]
		lea rsi, [rsi + 2] ; i += 2
		subpd xmm2, xmm0
		subpd xmm3, xmm1
		mulpd xmm2, xmm2
		mulpd xmm3, xmm3
		addpd xmm2, xmm3
		sqrtpd xmm2, xmm2
		addpd xmm7, xmm2
	.EndOfMoreThanTwoMagnitudeRemains

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; if (numPoints_ & 1)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	test rax, 1
	jz .EndOfMoreThanOneMagnitudeRemains
	.MoreThanOneMagnitudeRemains
		movsd xmm2, [rbx + rsi * 8 + 0*8]
		movsd xmm3, [rdx + rsi * 8 + 0*8]
		subsd xmm2, xmm0
		subsd xmm3, xmm1
		mulsd xmm2, xmm2
		mulsd xmm3, xmm3
		addsd xmm2, xmm3
		sqrtsd xmm2, xmm2
		addsd xmm7, xmm2
	.EndOfMoreThanOneMagnitudeRemains

	movapd xmm2, xmm7
	movapd xmm3, [sym(ksqrt2_f64)] ; xmm3 = xmmSqrt2
	shufpd xmm2, xmm2, 0x01
	addsd xmm7, xmm2
	mov rax, arg(3) ; tx1
	mulsd xmm7, xmm6
	mov rbx, arg(4) ; ty1
	divsd xmm3, xmm7 ; now xmm3 = xmmMagnitude
	mov rcx, arg(5) ; s1

	movsd [rax], xmm0
	movsd [rbx], xmm1
	movsd [rcx], xmm3

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
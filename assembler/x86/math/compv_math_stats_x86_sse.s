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
global sym(MathStatsNormalize2DHartley_4_float64_Asm_X86_SSE2)
global sym(MathStatsMSE2DHomogeneous_float64_Asm_X86_SSE2)
global sym(MathStatsMSE2DHomogeneous_4_float64_Asm_X86_SSE2)

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


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const COMPV_ALIGNED(SSE) compv_float64_t* x;
; arg(1) -> const COMPV_ALIGNED(SSE) compv_float64_t* y
; arg(2) -> compv_uscalar_t numPoints;
; arg(3) -> compv_float64_t* tx1
; arg(4) -> compv_float64_t* ty1
; arg(5) -> compv_float64_t* s1
; void MathStatsNormalize2DHartley_4_float64_Asm_X86_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* x, const COMPV_ALIGNED(SSE) compv_float64_t* y, compv_uscalar_t numPoints, compv_float64_t* tx1, compv_float64_t* ty1, compv_float64_t* s1)
sym(MathStatsNormalize2DHartley_4_float64_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	COMPV_YASM_SAVE_XMM 7
	;; end prolog ;;

	mov rcx, arg(0) ; rcx = x
	mov rdx, arg(1) ; rdx = y

	movd xmm2, arg(2)
	pshufd xmm2, xmm2, 0x0
	movapd xmm6, [sym(k1_f64)]
	cvtdq2pd xmm2, xmm2
	
	movapd xmm0, [rcx + 0*8]
	movapd xmm1, [rdx + 0*8]
	divpd xmm6, xmm2 ; xmm6 = xmmOneOverNumPoints
	addpd xmm0, [rcx + 2*8]
	addpd xmm1, [rdx + 2*8]
	movapd xmm2, xmm0
	movapd xmm3, xmm1
	shufpd xmm2, xmm2, 0x1
	shufpd xmm3, xmm3, 0x1
	addsd xmm0, xmm2
	addsd xmm1, xmm3
	mulsd xmm0, xmm6
	mulsd xmm1, xmm6
	movapd xmm2, [rcx + 0*8]
	movapd xmm3, [rcx + 2*8]
	shufpd xmm0, xmm0, 0x0
	shufpd xmm1, xmm1, 0x0	
	movapd xmm4, [rdx + 0*8]
	movapd xmm5, [rdx + 2*8]
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
	movapd xmm4, [sym(ksqrt2_f64)] ; xmm4 = xmmSqrt2
	sqrtpd xmm7, xmm2
	sqrtpd xmm3, xmm3
	mov rax, arg(3)
	mov rcx, arg(4)
	mov rdx, arg(5)
	addpd xmm7, xmm3
	movapd xmm2, xmm7
	shufpd xmm2, xmm2, 0x01
	addsd xmm7, xmm2
	mulsd xmm7, xmm6
	divsd xmm4, xmm7 ; now xmm4 = xmmMagnitude
	movsd [rax], xmm0
	movsd [rcx], xmm1
	movsd [rdx], xmm4

	;; begin epilog ;;
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const COMPV_ALIGNED(SSE) compv_float64_t* aX_h
; arg(1) -> const COMPV_ALIGNED(SSE) compv_float64_t* aY_h
; arg(2) -> const COMPV_ALIGNED(SSE) compv_float64_t* aZ_h
; arg(3) -> const COMPV_ALIGNED(SSE) compv_float64_t* bX
; arg(4) -> const COMPV_ALIGNED(SSE) compv_float64_t* bY
; arg(5) -> COMPV_ALIGNED(SSE) compv_float64_t* mse
; arg(6) -> compv_uscalar_t numPoints
; void MathStatsMSE2DHomogeneous_float64_Asm_X86_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* aX_h, const COMPV_ALIGNED(SSE) compv_float64_t* aY_h, const COMPV_ALIGNED(SSE) compv_float64_t* aZ_h, const COMPV_ALIGNED(SSE) compv_float64_t* bX, const COMPV_ALIGNED(SSE) compv_float64_t* bY, COMPV_ALIGNED(SSE) compv_float64_t* mse, compv_uscalar_t numPoints)
sym(MathStatsMSE2DHomogeneous_float64_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_XMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; alloc memory
	sub rsp, 8 + 8
	; [rsp + 0] = numPointsSigned - 3
	; [rsp + 8] = numPointsSigned - 1

	mov rax, arg(6)
	lea rax, [rax - 3]
	mov [rsp + 0], rax
	lea rax, [rax + 2]
	mov [rsp + 8], rax

	xor rcx, rcx ; rcx = i
	mov rsi, arg(0) ; aX_h
	mov rdi, arg(1) ; aY_h
	mov rax, arg(2) ; aZ_h
	mov rbx, arg(3) ; bX
	mov rdx, arg(4) ; bY

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (i = 0; i < numPointsSigned - 3; i += 4)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rcx, [rsp + 0]
	jge .EndOfLoop4
	.Loop4
		movapd xmm0, [sym(k1_f64)]
		movapd xmm1, [sym(k1_f64)]
		divpd xmm0, [rax + rcx*8]
		divpd xmm1, [rax + rcx*8 + 2*8]
		mov rax, arg(5) ; mse
		movapd xmm2, [rsi + rcx*8]
		movapd xmm3, [rbx + rcx*8]
		movapd xmm4, [rsi + rcx*8 + 2*8]
		movapd xmm5, [rbx + rcx*8 + 2*8]
		mulpd xmm2, xmm0
		mulpd xmm4, xmm1
		movapd xmm6, [rdi + rcx*8 + 2*8]
		movapd xmm7, [rdx + rcx*8 + 2*8]
		subpd xmm2, xmm3 ; xmm2 = xmmEX0
		subpd xmm4, xmm5 ; xmm4 = xmmEX1		
		movapd xmm3, [rdi + rcx*8]
		movapd xmm5, [rdx + rcx*8]
		mulpd xmm3, xmm0
		mulpd xmm6, xmm1
		lea rcx, [rcx + 4]
		subpd xmm3, xmm5
		subpd xmm6, xmm7
		mulpd xmm2, xmm2
		mulpd xmm4, xmm4
		mulpd xmm3, xmm3
		mulpd xmm6, xmm6
		addpd xmm2, xmm3
		addpd xmm4, xmm6
		movapd [rax + rcx*8 - 4*8], xmm2
		movapd [rax + rcx*8 + 2*8 - 4*8], xmm4
		mov rax, arg(2) ; aZ_h
		cmp rcx, [rsp + 0]
		jl .Loop4
	.EndOfLoop4

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; if (i < numPointsSigned - 1)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rcx, [rsp + 8]
	jge .EndOfMoreThanTwoRemains
	.MoreThanTwoRemains
		movapd xmm0, [sym(k1_f64)]
		divpd xmm0, [rax + rcx*8]
		mov rax, arg(5) ; mse
		movapd xmm1, [rsi + rcx*8]
		movapd xmm2, [rdi + rcx*8]
		mulpd xmm1, xmm0
		mulpd xmm2, xmm0
		subpd xmm1, [rbx + rcx*8]
		subpd xmm2, [rdx + rcx*8]
		mulpd xmm1, xmm1
		mulpd xmm2, xmm2
		addpd xmm1, xmm2
		movapd [rax + rcx*8], xmm1
		lea rcx, [rcx + 2]
		mov rax, arg(2) ; aZ_h
	.EndOfMoreThanTwoRemains

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; if (numPointsSigned & 1)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rcx, arg(6)
	jge .EndOfMoreThanOneRemains
	.MoreThanOneRemains
		movsd xmm0, [sym(k1_f64)]
		divsd xmm0, [rax + rcx*8]
		mov rax, arg(5) ; mse
		movsd xmm1, [rsi + rcx*8]
		movsd xmm2, [rdi + rcx*8]
		mulsd xmm1, xmm0
		mulsd xmm2, xmm0
		subsd xmm1, [rbx + rcx*8]
		subsd xmm2, [rdx + rcx*8]
		mulsd xmm1, xmm1
		mulsd xmm2, xmm2
		addsd xmm1, xmm2
		movsd [rax + rcx*8], xmm1
	.EndOfMoreThanOneRemains

	; free memory
	add rsp, 8 + 8

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const COMPV_ALIGNED(SSE) compv_float64_t* aX_h
; arg(1) -> const COMPV_ALIGNED(SSE) compv_float64_t* aY_h
; arg(2) -> const COMPV_ALIGNED(SSE) compv_float64_t* aZ_h
; arg(3) -> const COMPV_ALIGNED(SSE) compv_float64_t* bX
; arg(4) -> const COMPV_ALIGNED(SSE) compv_float64_t* bY
; arg(5) -> COMPV_ALIGNED(SSE) compv_float64_t* mse
; arg(6) -> compv_uscalar_t numPoints
; void MathStatsMSE2DHomogeneous_4_float64_Asm_X86_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* aX_h, const COMPV_ALIGNED(SSE) compv_float64_t* aY_h, const COMPV_ALIGNED(SSE) compv_float64_t* aZ_h, const COMPV_ALIGNED(SSE) compv_float64_t* bX, const COMPV_ALIGNED(SSE) compv_float64_t* bY, COMPV_ALIGNED(SSE) compv_float64_t* mse, compv_uscalar_t numPoints)
sym(MathStatsMSE2DHomogeneous_4_float64_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_XMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;
	
	movapd xmm0, [sym(k1_f64)]
	movapd xmm1, [sym(k1_f64)]
	mov rax, arg(2) ; aZ_h
	mov rsi, arg(0) ; aX_h
	mov rbx, arg(3) ; bX
	mov rdi, arg(1) ; aY_h	
	mov rdx, arg(4) ; bY
	mov rcx, arg(5) ; mse	
	divpd xmm0, [rax]
	divpd xmm1, [rax + 2*8]
	movapd xmm2, [rsi]
	movapd xmm3, [rbx]
	movapd xmm4, [rsi + 2*8]
	movapd xmm5, [rbx + 2*8]
	mulpd xmm2, xmm0
	mulpd xmm4, xmm1
	movapd xmm6, [rdi + 2*8]
	movapd xmm7, [rdx + 2*8]
	subpd xmm2, xmm3
	subpd xmm4, xmm5		
	movapd xmm3, [rdi]
	movapd xmm5, [rdx]
	mulpd xmm3, xmm0
	mulpd xmm6, xmm1
	subpd xmm3, xmm5
	subpd xmm6, xmm7
	mulpd xmm2, xmm2
	mulpd xmm4, xmm4
	mulpd xmm3, xmm3
	mulpd xmm6, xmm6
	addpd xmm2, xmm3
	addpd xmm4, xmm6
	movapd [rcx], xmm2
	movapd [rcx + 2*8], xmm4

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
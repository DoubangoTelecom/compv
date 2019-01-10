;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(CompVMathStatsNormalize2DHartley_64f_Asm_X86_SSE2)
global sym(CompVMathStatsNormalize2DHartley_4_64f_Asm_X86_SSE2)
global sym(CompVMathStatsMSE2DHomogeneous_64f_Asm_X86_SSE2)
global sym(CompVMathStatsMSE2DHomogeneous_4_64f_Asm_X86_SSE2)
global sym(CompVMathStatsVariance_64f_Asm_X86_SSE2)

section .data
	extern sym(ksqrt2_64f)
	extern sym(k1_64f)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const compv_float64_t* x;
; arg(1) -> COMPV_ALIGNED(SSE) const compv_float64_t* y
; arg(2) -> compv_uscalar_t numPoints;
; arg(3) -> compv_float64_t* tx1
; arg(4) -> compv_float64_t* ty1
; arg(5) -> compv_float64_t* s1
sym(CompVMathStatsNormalize2DHartley_64f_Asm_X86_SSE2):
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

	pxor xmm0, xmm0 ; xmm0 = vecTx
	pxor xmm1, xmm1 ; xmm1 = vecTy
	pxor xmm7, xmm7 ; xmm7 = vecMagnitude
	xor rsi, rsi ; rsi = i

	mov rax, arg(2) ; rax = numPoints
	lea rcx, [rax - 7] ; rcx = (numPoints - 7)
	lea rdi, [rax - 3] ; rdx = (numPoints - 3)

	movd xmm2, rax
	pshufd xmm2, xmm2, 0x0
	movapd xmm6, [sym(k1_64f)]
	cvtdq2pd xmm2, xmm2
	divpd xmm6, xmm2 ; xmm6 = xmmOneOverNumPoints

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (i = 0; i < numPoints_ - 7; i += 8)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rsi, rcx
	jge .EndOfLoop8TxTy
	.Loop8TxTy:
		movapd xmm2, [rbx + rsi * COMPV_YASM_FLOAT64_SZ_BYTES + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm3, [rbx + rsi * COMPV_YASM_FLOAT64_SZ_BYTES + 4*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm4, [rdx + rsi * COMPV_YASM_FLOAT64_SZ_BYTES + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm5, [rdx + rsi * COMPV_YASM_FLOAT64_SZ_BYTES + 4*COMPV_YASM_FLOAT64_SZ_BYTES]
		addpd xmm2, [rbx + rsi * COMPV_YASM_FLOAT64_SZ_BYTES + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
		addpd xmm3, [rbx + rsi * COMPV_YASM_FLOAT64_SZ_BYTES + 6*COMPV_YASM_FLOAT64_SZ_BYTES]
		addpd xmm4, [rdx + rsi * COMPV_YASM_FLOAT64_SZ_BYTES + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
		addpd xmm5, [rdx + rsi * COMPV_YASM_FLOAT64_SZ_BYTES + 6*COMPV_YASM_FLOAT64_SZ_BYTES]
		add rsi, 8 ; i += 8
		addpd xmm2, xmm3
		addpd xmm4, xmm5
		cmp rsi, rcx
		addpd xmm0, xmm2
		addpd xmm1, xmm4
		jl .Loop8TxTy
	.EndOfLoop8TxTy

	lea rcx, [rax - 1] ; rcx = (numPoints - 1)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; if (i < numPoints_ - 3)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rsi, rdi
	jge .EndOfMoreThanFourTxTyRemains
	.MoreThanFourTxTyRemains:
		movapd xmm2, [rbx + rsi * COMPV_YASM_FLOAT64_SZ_BYTES + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm3, [rdx + rsi * COMPV_YASM_FLOAT64_SZ_BYTES + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
		addpd xmm2, [rbx + rsi * COMPV_YASM_FLOAT64_SZ_BYTES + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
		addpd xmm3, [rdx + rsi * COMPV_YASM_FLOAT64_SZ_BYTES + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
		add rsi, 4 ; i += 4
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
		addpd xmm0, [rbx + rsi * COMPV_YASM_FLOAT64_SZ_BYTES + 0*COMPV_YASM_FLOAT64_SZ_BYTES - 16]
		addpd xmm1, [rdx + rsi * COMPV_YASM_FLOAT64_SZ_BYTES + 0*COMPV_YASM_FLOAT64_SZ_BYTES - 16]
	.EndOfMoreThanTwoTxTyRemains

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; if (numPoints_ & 1)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	test rax, 1
	jz .EndOfMoreThanOneTxTyRemains
	.MoreThanOneTxTyRemains:
		addsd xmm0, [rbx + rsi * COMPV_YASM_FLOAT64_SZ_BYTES + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
		addsd xmm1, [rdx + rsi * COMPV_YASM_FLOAT64_SZ_BYTES + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
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
	.Loop4Magnitude:
		movapd xmm2, [rbx + rsi * COMPV_YASM_FLOAT64_SZ_BYTES + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm3, [rbx + rsi * COMPV_YASM_FLOAT64_SZ_BYTES + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm4, [rdx + rsi * COMPV_YASM_FLOAT64_SZ_BYTES + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm5, [rdx + rsi * COMPV_YASM_FLOAT64_SZ_BYTES + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
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
		sqrtpd xmm2, xmm2
		sqrtpd xmm3, xmm3
		add rsi, 4
		cmp rsi, rdi
		addpd xmm7, xmm2
		addpd xmm7, xmm3
		jl .Loop4Magnitude
	.EndOfLoop4Magnitude

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; if (i < numPoints_ - 1)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rsi, rcx
	jge .EndOfMoreThanTwoMagnitudeRemains
	.MoreThanTwoMagnitudeRemains:
		movapd xmm2, [rbx + rsi * COMPV_YASM_FLOAT64_SZ_BYTES + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm3, [rdx + rsi * COMPV_YASM_FLOAT64_SZ_BYTES + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
		subpd xmm2, xmm0
		subpd xmm3, xmm1
		mulpd xmm2, xmm2
		mulpd xmm3, xmm3
		addpd xmm2, xmm3
		sqrtpd xmm2, xmm2
		add rsi, 2 ; i += 2
		addpd xmm7, xmm2
	.EndOfMoreThanTwoMagnitudeRemains

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; if (numPoints_ & 1)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	test rax, 1
	jz .EndOfMoreThanOneMagnitudeRemains
	.MoreThanOneMagnitudeRemains
		movsd xmm2, [rbx + rsi * COMPV_YASM_FLOAT64_SZ_BYTES + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
		movsd xmm3, [rdx + rsi * COMPV_YASM_FLOAT64_SZ_BYTES + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
		subsd xmm2, xmm0
		subsd xmm3, xmm1
		mulsd xmm2, xmm2
		mulsd xmm3, xmm3
		addsd xmm2, xmm3
		sqrtsd xmm2, xmm2
		addsd xmm7, xmm2
	.EndOfMoreThanOneMagnitudeRemains

	movapd xmm2, xmm7
	movapd xmm3, [sym(ksqrt2_64f)] ; xmm3 = vecSqrt2
	shufpd xmm2, xmm2, 0x01
	addsd xmm7, xmm2
	mov rax, arg(3) ; tx1
	mulsd xmm7, xmm6
	mov rbx, arg(4) ; ty1
	divsd xmm3, xmm7 ; now xmm3 = vecMagnitude
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
; arg(0) -> COMPV_ALIGNED(SSE) const compv_float64_t* x;
; arg(1) -> COMPV_ALIGNED(SSE) const compv_float64_t* y
; arg(2) -> compv_uscalar_t numPoints;
; arg(3) -> compv_float64_t* tx1
; arg(4) -> compv_float64_t* ty1
; arg(5) -> compv_float64_t* s1
sym(CompVMathStatsNormalize2DHartley_4_64f_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	COMPV_YASM_SAVE_XMM 7
	;; end prolog ;;

	mov rcx, arg(0) ; rcx = x
	mov rdx, arg(1) ; rdx = y

	movd xmm2, arg(2)
	pshufd xmm2, xmm2, 0x0
	movapd xmm6, [sym(k1_64f)]
	cvtdq2pd xmm2, xmm2
	
	movapd xmm0, [rcx + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
	movapd xmm1, [rdx + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
	divpd xmm6, xmm2 ; xmm6 = vecOneOverNumPoints
	addpd xmm0, [rcx + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
	addpd xmm1, [rdx + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
	movapd xmm2, xmm0
	movapd xmm3, xmm1
	shufpd xmm2, xmm2, 0x1
	shufpd xmm3, xmm3, 0x1
	addsd xmm0, xmm2
	addsd xmm1, xmm3
	mulsd xmm0, xmm6
	mulsd xmm1, xmm6
	movapd xmm2, [rcx + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
	movapd xmm3, [rcx + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
	shufpd xmm0, xmm0, 0x0
	shufpd xmm1, xmm1, 0x0	
	movapd xmm4, [rdx + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
	movapd xmm5, [rdx + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
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
	movapd xmm4, [sym(ksqrt2_64f)] ; xmm4 = vecSqrt2
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
; arg(0) -> COMPV_ALIGNED(SSE) const compv_float64_t* aX_h
; arg(1) -> COMPV_ALIGNED(SSE) const compv_float64_t* aY_h
; arg(2) -> COMPV_ALIGNED(SSE) const compv_float64_t* aZ_h
; arg(3) -> COMPV_ALIGNED(SSE) const compv_float64_t* bX
; arg(4) -> COMPV_ALIGNED(SSE) const compv_float64_t* bY
; arg(5) -> COMPV_ALIGNED(SSE) compv_float64_t* mse
; arg(6) -> compv_uscalar_t numPoints
sym(CompVMathStatsMSE2DHomogeneous_64f_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_XMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; alloc memory
	sub rsp, COMPV_YASM_REG_SZ_BYTES + COMPV_YASM_REG_SZ_BYTES

	%define numPointsMinus3		rsp + 0
	%define numPointsMinus1		numPointsMinus3 + COMPV_YASM_REG_SZ_BYTES

	mov rax, arg(6)
	lea rax, [rax - 3]
	mov [numPointsMinus3], rax
	lea rax, [rax + 2]
	mov [numPointsMinus1], rax

	xor rcx, rcx ; rcx = i
	mov rsi, arg(0) ; aX_h
	mov rdi, arg(1) ; aY_h
	mov rax, arg(2) ; aZ_h
	mov rbx, arg(3) ; bX
	mov rdx, arg(4) ; bY

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (i = 0; i < numPointsSigned - 3; i += 4)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rcx, [numPointsMinus3]
	jge .EndOfLoop4
	.Loop4:
		movapd xmm0, [sym(k1_64f)]
		movapd xmm1, [sym(k1_64f)]
		divpd xmm0, [rax + rcx*COMPV_YASM_FLOAT64_SZ_BYTES + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
		divpd xmm1, [rax + rcx*COMPV_YASM_FLOAT64_SZ_BYTES + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
		mov rax, arg(5) ; mse
		movapd xmm2, [rsi + rcx*COMPV_YASM_FLOAT64_SZ_BYTES + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm3, [rbx + rcx*COMPV_YASM_FLOAT64_SZ_BYTES + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm4, [rsi + rcx*COMPV_YASM_FLOAT64_SZ_BYTES + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm5, [rbx + rcx*COMPV_YASM_FLOAT64_SZ_BYTES + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
		mulpd xmm2, xmm0
		mulpd xmm4, xmm1
		movapd xmm6, [rdi + rcx*COMPV_YASM_FLOAT64_SZ_BYTES + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm7, [rdx + rcx*COMPV_YASM_FLOAT64_SZ_BYTES + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
		subpd xmm2, xmm3 ; xmm2 = vecEX0
		subpd xmm4, xmm5 ; xmm4 = vecEX1		
		movapd xmm3, [rdi + rcx*COMPV_YASM_FLOAT64_SZ_BYTES + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm5, [rdx + rcx*COMPV_YASM_FLOAT64_SZ_BYTES + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
		mulpd xmm3, xmm0
		mulpd xmm6, xmm1
		add rcx, 4
		subpd xmm3, xmm5
		subpd xmm6, xmm7
		mulpd xmm2, xmm2
		mulpd xmm4, xmm4
		mulpd xmm3, xmm3
		mulpd xmm6, xmm6
		cmp rcx, [numPointsMinus3]
		addpd xmm2, xmm3
		addpd xmm4, xmm6
		movapd [rax + rcx*COMPV_YASM_FLOAT64_SZ_BYTES - 4*COMPV_YASM_FLOAT64_SZ_BYTES], xmm2
		movapd [rax + rcx*COMPV_YASM_FLOAT64_SZ_BYTES + 2*COMPV_YASM_FLOAT64_SZ_BYTES - 4*COMPV_YASM_FLOAT64_SZ_BYTES], xmm4
		mov rax, arg(2) ; aZ_h
		jl .Loop4
	.EndOfLoop4

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; if (i < numPointsSigned - 1)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rcx, [numPointsMinus1]
	jge .EndOfMoreThanTwoRemains
	.MoreThanTwoRemains
		movapd xmm0, [sym(k1_64f)]
		divpd xmm0, [rax + rcx*COMPV_YASM_FLOAT64_SZ_BYTES]
		mov rax, arg(5) ; mse
		movapd xmm1, [rsi + rcx*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm2, [rdi + rcx*COMPV_YASM_FLOAT64_SZ_BYTES]
		mulpd xmm1, xmm0
		mulpd xmm2, xmm0
		subpd xmm1, [rbx + rcx*COMPV_YASM_FLOAT64_SZ_BYTES]
		subpd xmm2, [rdx + rcx*COMPV_YASM_FLOAT64_SZ_BYTES]
		mulpd xmm1, xmm1
		mulpd xmm2, xmm2
		addpd xmm1, xmm2
		movapd [rax + rcx*COMPV_YASM_FLOAT64_SZ_BYTES], xmm1
		add rcx, 2
		mov rax, arg(2) ; aZ_h
	.EndOfMoreThanTwoRemains

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; if (numPointsSigned & 1)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rcx, arg(6)
	jge .EndOfMoreThanOneRemains
	.MoreThanOneRemains
		movsd xmm0, [sym(k1_64f)]
		divsd xmm0, [rax + rcx*COMPV_YASM_FLOAT64_SZ_BYTES]
		mov rax, arg(5) ; mse
		movsd xmm1, [rsi + rcx*COMPV_YASM_FLOAT64_SZ_BYTES]
		movsd xmm2, [rdi + rcx*COMPV_YASM_FLOAT64_SZ_BYTES]
		mulsd xmm1, xmm0
		mulsd xmm2, xmm0
		subsd xmm1, [rbx + rcx*COMPV_YASM_FLOAT64_SZ_BYTES]
		subsd xmm2, [rdx + rcx*COMPV_YASM_FLOAT64_SZ_BYTES]
		mulsd xmm1, xmm1
		mulsd xmm2, xmm2
		addsd xmm1, xmm2
		movsd [rax + rcx*COMPV_YASM_FLOAT64_SZ_BYTES], xmm1
	.EndOfMoreThanOneRemains

	; free memory
	add rsp, COMPV_YASM_REG_SZ_BYTES + COMPV_YASM_REG_SZ_BYTES

	%undef numPointsMinus3
	%undef numPointsMinus1

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
; arg(0) -> COMPV_ALIGNED(SSE) const compv_float64_t* aX_h
; arg(1) -> COMPV_ALIGNED(SSE) const compv_float64_t* aY_h
; arg(2) -> COMPV_ALIGNED(SSE) const compv_float64_t* aZ_h
; arg(3) -> COMPV_ALIGNED(SSE) const compv_float64_t* bX
; arg(4) -> COMPV_ALIGNED(SSE) const compv_float64_t* bY
; arg(5) -> COMPV_ALIGNED(SSE) compv_float64_t* mse
; arg(6) -> compv_uscalar_t numPoints
sym(CompVMathStatsMSE2DHomogeneous_4_64f_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_XMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;
	
	movapd xmm0, [sym(k1_64f)]
	movapd xmm1, [sym(k1_64f)]
	mov rax, arg(2) ; aZ_h	
	divpd xmm0, [rax + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
	divpd xmm1, [rax + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
	mov rsi, arg(0) ; aX_h
	mov rbx, arg(3) ; bX
	mov rdi, arg(1) ; aY_h	
	mov rdx, arg(4) ; bY
	mov rcx, arg(5) ; mse
	movapd xmm2, [rsi]
	movapd xmm3, [rbx]
	movapd xmm4, [rsi + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
	movapd xmm5, [rbx + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
	mulpd xmm2, xmm0
	mulpd xmm4, xmm1
	movapd xmm6, [rdi + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
	movapd xmm7, [rdx + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
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
	movapd [rcx + 2*COMPV_YASM_FLOAT64_SZ_BYTES], xmm4

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
; arg(0) -> COMPV_ALIGNED(SSE) const compv_float64_t* data
; arg(1) -> compv_uscalar_t count
; arg(2) -> const compv_float64_t* mean1
; arg(3) -> compv_float64_t* var1
sym(CompVMathStatsVariance_64f_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 4
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	mov rax, arg(2)
	mov rcx, arg(1) ; rcx = count
	xor rsi, rsi ; rsi = i = 0
	movsd xmm0, [rax]
	lea rbx, [rcx - 1] ; rbx = (count - 1)
	lea rdx, [rcx - 3] ; rdx = (count - 3)
	xorpd xmm5, xmm5 ; xmm5 = vecVar
	movd xmm1, rbx
	shufpd xmm0, xmm0, 0x0 ; xmm0 = vecMean
	pshufd xmm1, xmm1, 0x0
	mov rax, arg(0) ; rax = data
	mov rdi, arg(3) ; rdi = var1
	cvtdq2pd xmm1, xmm1 ; xmm1 = vecCountMinus1

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (i = 0; i < countSigned - 3; i += 4)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rsi, rdx
	jge .EndOfLoop4
	.Loop4:
		movapd xmm2, [rax + rsi*COMPV_YASM_FLOAT64_SZ_BYTES + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm3, [rax + rsi*COMPV_YASM_FLOAT64_SZ_BYTES + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
		subpd xmm2, xmm0
		subpd xmm3, xmm0
		mulpd xmm2, xmm2
		add rsi, 4
		mulpd xmm3, xmm3
		cmp rsi, rdx
		addpd xmm5, xmm2
		addpd xmm5, xmm3
		jl .Loop4
	.EndOfLoop4

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; if (i < countSigned - 1)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rsi, rbx
	jge .EnOfMoreThanTwoRemains
		movapd xmm2, [rax + rsi*COMPV_YASM_FLOAT64_SZ_BYTES]
		subpd xmm2, xmm0
		mulpd xmm2, xmm2
		add rsi, 2
		addpd xmm5, xmm2
	.EnOfMoreThanTwoRemains:

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; if (countSigned & 1)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	test rcx, 1
	jz .EndOfMoreThanOneRamains
		movsd xmm2, [rax + rsi*COMPV_YASM_FLOAT64_SZ_BYTES]
		subsd xmm2, xmm0
		mulsd xmm2, xmm2
		addsd xmm5, xmm2
	.EndOfMoreThanOneRamains:

	movapd xmm2, xmm5
	shufpd xmm2, xmm2, 0x1
	addsd xmm5, xmm2
	divsd xmm5, xmm1
	movsd [rdi], xmm5

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
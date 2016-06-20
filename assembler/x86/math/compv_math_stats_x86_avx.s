;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "../compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(MathStatsNormalize2DHartley_float64_Asm_X86_AVX)
global sym(MathStatsNormalize2DHartley_4_float64_Asm_X86_AVX)

section .data
	extern sym(ksqrt2_f64)
	extern sym(k1_f64)
	extern sym(kAVXMaskstore_0_u64)
	extern sym(kAVXMaskstore_0_1_u64)
	extern sym(kAVXMaskzero_2_3_u64)
	extern sym(kAVXMaskzero_1_2_3_u64)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const COMPV_ALIGNED(AVX) compv_float64_t* x;
; arg(1) -> const COMPV_ALIGNED(AVX) compv_float64_t* y
; arg(2) -> compv_uscalar_t numPoints;
; arg(3) -> compv_float64_t* tx1
; arg(4) -> compv_float64_t* ty1
; arg(5) -> compv_float64_t* s1
; void MathStatsNormalize2DHartley_float64_Asm_X86_AVX(const COMPV_ALIGNED(AVX) compv_float64_t* x, const COMPV_ALIGNED(AVX) compv_float64_t* y, compv_uscalar_t numPoints, compv_float64_t* tx1, compv_float64_t* ty1, compv_float64_t* s1)
sym(MathStatsNormalize2DHartley_float64_Asm_X86_AVX):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	COMPV_YASM_SAVE_YMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	mov rbx, arg(0) ; rbx = x
	mov rdx, arg(1) ; rdx = y

	;-------------------------------------
	; TX and TY
	;-------------------------------------

	vxorpd ymm0, ymm0 ; ymm0 = ymmTx
	vxorpd ymm1, ymm1 ; ymm1 = ymmTy
	vxorpd ymm7, ymm7 ; ymm7 = ymmMagnitude
	xor rsi, rsi ; rsi = i

	mov rax, arg(2) ; rax = numPoints
	lea rcx, [rax - 15] ; rcx = (numPoints - 15)
	lea rdi, [rax - 7] ; rdx = (numPoints - 7)

	vmovd xmm2, eax
	vpshufd xmm2, xmm2, 0x0
	vmovapd ymm6, [sym(k1_f64)]
	vcvtdq2pd ymm2, xmm2
	vdivpd ymm6, ymm6, ymm2 ; ymm6 = ymmOneOverNumPoints

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (i = 0; i < numPoints_ - 15; i += 16)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rsi, rcx
	jge .EndOfLoop16TxTy
	.Loop16TxTy
		vmovapd ymm2, [rbx + rsi * 8 + 0*8]
		vmovapd ymm3, [rbx + rsi * 8 + 8*8]
		vmovapd ymm4, [rdx + rsi * 8 + 0*8]
		vmovapd ymm5, [rdx + rsi * 8 + 8*8]
		vaddpd ymm2, ymm2, [rbx + rsi * 8 + 4*8]
		vaddpd ymm3, ymm3, [rbx + rsi * 8 + 12*8]
		vaddpd ymm4, ymm4, [rdx + rsi * 8 + 4*8]
		vaddpd ymm5, ymm5, [rdx + rsi * 8 + 12*8]
		lea rsi, [rsi + 16] ; i += 16
		vaddpd ymm2, ymm2, ymm3
		vaddpd ymm4, ymm4, ymm5
		vaddpd ymm0, ymm0, ymm2
		vaddpd ymm1, ymm1, ymm4
		cmp rsi, rcx
		jl .Loop16TxTy
	.EndOfLoop16TxTy

	lea rcx, [rax - 3] ; rcx = (numPoints - 3)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; if (i < numPoints_ - 7)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rsi, rdi
	jge .EndOfMoreThanEightTxTyRemains
	.MoreThanEightTxTyRemains
		vmovapd ymm2, [rbx + rsi * 8 + 0*8]
		vmovapd ymm3, [rdx + rsi * 8 + 0*8]
		vaddpd ymm2, ymm2, [rbx + rsi * 8 + 4*8]
		vaddpd ymm3, ymm3, [rdx + rsi * 8 + 4*8]
		lea rsi, [rsi + 8] ; i += 8
		vaddpd ymm0, ymm0, ymm2
		vaddpd ymm1, ymm1, ymm3
	.EndOfMoreThanEightTxTyRemains

	lea rdi, [rax - 1] ; rdi = (numPoints - 1)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; if (i < numPoints_ - 3)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rsi, rcx
	jge .EndOfMoreThanFourTxTyRemains
	.MoreThanFourTxTyRemains
		lea rsi, [rsi + 4] ; i += 4
		vaddpd ymm0, ymm0, [rbx + rsi * 8 + 0*8 - 32]
		vaddpd ymm1, ymm1, [rdx + rsi * 8 + 0*8 - 32]
	.EndOfMoreThanFourTxTyRemains

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; if (i < numPoints_ - 1)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rsi, rdi
	jge .EndOfMoreThanTwoTxTyRemains
	.MoreThanTwoTxTyRemains
		vmovapd xmm2, [rbx + rsi * 8 + 0*8] ; load 128 bits only and pad with zeros
		vmovapd xmm3, [rdx + rsi * 8 + 0*8] ; load 128 bits only and pad with zeros
		lea rsi, [rsi + 2]
		vaddpd ymm0, ymm2
		vaddpd ymm1, ymm3
	.EndOfMoreThanTwoTxTyRemains

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; if (numPoints_ & 1)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	test rax, 1
	jz .EndOfMoreThanOneTxTyRemains
	.MoreThanOneTxTyRemains
		vmovsd xmm2, [rbx + rsi * 8 + 0*8]
		vmovsd xmm3, [rdx + rsi * 8 + 0*8]
		vaddpd ymm0, ymm2
		vaddpd ymm1, ymm3
	.EndOfMoreThanOneTxTyRemains

	xor rsi, rsi ; i = 0

	lea rdi, [rax - 7] ; rdi = (numPoints - 7)

	vhaddpd ymm0, ymm0, ymm0
	vhaddpd ymm1, ymm1, ymm1
	vperm2f128 ymm2, ymm0, ymm0, 0x11
	vperm2f128 ymm3, ymm1, ymm1, 0x11
	vaddpd ymm0, ymm0, ymm2
	vaddpd ymm1, ymm1, ymm3
	vmulpd ymm0, ymm0, ymm6
	vmulpd ymm1, ymm1, ymm6
	vperm2f128 ymm0, ymm0, ymm0, 0x00
	vperm2f128 ymm1, ymm1, ymm1, 0x00

	;-------------------------------------
	; Magnitude
	;-------------------------------------

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (i = 0; i < numPoints_ - 7; i += 8)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rsi, rdi
	jge .EndOfLoop8Magnitude
	.Loop8Magnitude
		vmovapd ymm2, [rbx + rsi * 8 + 0*8]
		vmovapd ymm3, [rbx + rsi * 8 + 4*8]
		vmovapd ymm4, [rdx + rsi * 8 + 0*8]
		vmovapd ymm5, [rdx + rsi * 8 + 4*8]
		vsubpd ymm2, ymm2, ymm0
		vsubpd ymm3, ymm3, ymm0
		vsubpd ymm4, ymm4, ymm1
		vsubpd ymm5, ymm5, ymm1
		vmulpd ymm2, ymm2, ymm2
		vmulpd ymm4, ymm4, ymm4
		vmulpd ymm3, ymm3, ymm3
		vmulpd ymm5, ymm5, ymm5
		vaddpd ymm2, ymm2, ymm4
		vaddpd ymm3, ymm3, ymm5
		vsqrtpd ymm2, ymm2
		vsqrtpd ymm3, ymm3
		lea rsi, [rsi + 8] ; i += 8
		vaddpd ymm7, ymm7, ymm2
		vaddpd ymm7, ymm7, ymm3
		cmp rsi, rdi
		jl .Loop8Magnitude
	.EndOfLoop8Magnitude

	lea rdi, [rax - 1] ; rdi = (numPoints - 1)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; if (i < numPoints_ - 3)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rsi, rcx
	jge .EndOfMoreThanFourMagnitudeRemains
	.MoreThanFourMagnitudeRemains
		vmovapd ymm2, [rbx + rsi * 8 + 0*8]
		vmovapd ymm3, [rdx + rsi * 8 + 0*8]
		vsubpd ymm2, ymm2, ymm0
		vsubpd ymm3, ymm3, ymm1
		vmulpd ymm2, ymm2, ymm2
		vmulpd ymm3, ymm3, ymm3
		vaddpd ymm2, ymm2, ymm3
		vsqrtpd ymm2, ymm2
		lea rsi, [rsi + 4] ; i += 4
		vaddpd ymm7, ymm7, ymm2
	.EndOfMoreThanFourMagnitudeRemains

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; if (i < numPoints_ - 1)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rsi, rdi
	jge .EndOfMoreThanTwoMagnitudeRemains
	.MoreThanTwoMagnitudeRemains
		movapd xmm2, [rbx + rsi * 8 + 0*8] ; load 128b
		movapd xmm3, [rdx + rsi * 8 + 0*8] ; load 128b
		vsubpd xmm2, xmm2, xmm0
		vsubpd xmm3, xmm3, xmm1
		vmulpd xmm2, xmm2, xmm2
		vmulpd xmm3, xmm3, xmm3
		vaddpd xmm2, xmm2, xmm3
		vsqrtpd xmm2, xmm2
		lea rsi, [rsi + 2] ; i += 2
		vaddpd ymm7, ymm7, ymm2
	.EndOfMoreThanTwoMagnitudeRemains

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; if (numPoints_ & 1)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	test rax, 1
	jz .EndOfMoreThanOneMagnitudeRemains
	.MoreThanOneMagnitudeRemains
		vmovsd xmm2, [rbx + rsi * 8 + 0*8]
		vmovsd xmm3, [rdx + rsi * 8 + 0*8]
		vsubsd xmm2, xmm2, xmm0
		vsubsd xmm3, xmm3, xmm1
		vmulsd xmm2, xmm2, xmm2
		vmulsd xmm3, xmm3, xmm3
		vaddsd xmm2, xmm2, xmm3
		vsqrtsd xmm2, xmm2
		vaddpd ymm7, ymm7, ymm2
	.EndOfMoreThanOneMagnitudeRemains
	
	vhaddpd ymm7, ymm7, ymm7
	vmovapd ymm3, [sym(ksqrt2_f64)] ; ymm3 = ymmSqrt2
	vperm2f128 ymm4, ymm7, ymm7, 0x11
	vaddpd ymm7, ymm7, ymm4
	mov rax, arg(3) ; tx1
	vmulsd xmm7, xmm7, xmm6
	mov rbx, arg(4) ; ty1
	vdivsd xmm7, xmm3, xmm7
	mov rcx, arg(5) ; s1

	vmovsd [rax], xmm0
	vmovsd [rbx], xmm1
	vmovsd [rcx], xmm7

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_YMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const COMPV_ALIGNED(AVX) compv_float64_t* x;
; arg(1) -> const COMPV_ALIGNED(AVX) compv_float64_t* y
; arg(2) -> compv_uscalar_t numPoints;
; arg(3) -> compv_float64_t* tx1
; arg(4) -> compv_float64_t* ty1
; arg(5) -> compv_float64_t* s1
; void MathStatsNormalize2DHartley_4_float64_Asm_X86_AVX(const COMPV_ALIGNED(AVX) compv_float64_t* x, const COMPV_ALIGNED(AVX) compv_float64_t* y, compv_uscalar_t numPoints, compv_float64_t* tx1, compv_float64_t* ty1, compv_float64_t* s1)
sym(MathStatsNormalize2DHartley_4_float64_Asm_X86_AVX):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	COMPV_YASM_SAVE_YMM 7 ; FIXME: needed?
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
	COMPV_YASM_RESTORE_YMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret
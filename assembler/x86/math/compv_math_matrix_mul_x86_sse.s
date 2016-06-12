;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "../compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(MatrixMulGA_float64_Asm_X86_SSE2)
global sym(MatrixMulGA_float32_Asm_X86_SSE2)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) compv_float64_t* ri
; arg(1) -> COMPV_ALIGNED(SSE) compv_float64_t* rj
; arg(2) -> const compv_float64_t* c1
; arg(3) -> const compv_float64_t* s1
; arg(4) -> compv_uscalar_t count
; void MatrixMulGA_float64_Asm_X86_SSE2(COMPV_ALIGNED(SSE) compv_float64_t* ri, COMPV_ALIGNED(SSE) compv_float64_t* rj, const compv_float64_t* c1, const compv_float64_t* s1, compv_uscalar_t count)
sym(MatrixMulGA_float64_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	push rbx
	;; end prolog ;;

	xor rcx, rcx ; rcx = i
	mov rbx, arg(4) ; rbx = count
	imul rbx, 8

	mov rax, arg(2)
	mov rdx, arg(3)
	movsd xmm0, [rax]
	movsd xmm1, [rdx]
	shufpd xmm0, xmm0, 0x0 ; xmm0 = xmmC
	shufpd xmm1, xmm1, 0x0 ; xmm1 = xmmS

	mov rax, arg(0) ; ri
	mov rdx, arg(1) ; rj 

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; while (i < count)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopProcess
		movapd xmm2, [rax + rcx] ; xmmRI
		movapd xmm3, [rdx + rcx] ; XmmRJ
		movapd xmm4, xmm1
		movapd xmm5, xmm0

		mulpd xmm4, xmm2
		mulpd xmm2, xmm0
		mulpd xmm5, xmm3
		mulpd xmm3, xmm1

		add rcx, 16

		subpd xmm5, xmm4
		addpd xmm2, xmm3
		
		movapd [rdx + rcx - 16], xmm5
		movapd [rax + rcx - 16], xmm2
		
		cmp rcx, rbx
		jl .LoopProcess

	;; begin epilog ;;
	pop rbx
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) compv_float32_t* ri
; arg(1) -> COMPV_ALIGNED(SSE) compv_float32_t* rj
; arg(2) -> const compv_float32_t* c1
; arg(3) -> const compv_float32_t* s1
; arg(4) -> compv_uscalar_t count
; void MatrixMulGA_float32_Asm_X86_SSE2(COMPV_ALIGNED(SSE) compv_float32_t* ri, COMPV_ALIGNED(SSE) compv_float32_t* rj, const compv_float32_t* c1, const compv_float32_t* s1, compv_uscalar_t count)
sym(MatrixMulGA_float32_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	push rbx
	;; end prolog ;;

	xor rcx, rcx ; rcx = i
	mov rbx, arg(4) ; rbx = count
	imul rbx, 4

	mov rax, arg(2)
	mov rdx, arg(3)
	movss xmm0, [rax]
	movss xmm1, [rdx]
	shufps xmm0, xmm0, 0x0 ; xmm0 = xmmC
	shufps xmm1, xmm1, 0x0 ; xmm1 = xmmS

	mov rax, arg(0) ; ri
	mov rdx, arg(1) ; rj 

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; while (i < count)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopProcess
		movaps xmm2, [rax + rcx] ; xmmRI
		movaps xmm3, [rdx + rcx] ; XmmRJ
		movaps xmm4, xmm1
		movaps xmm5, xmm0

		mulps xmm4, xmm2
		mulps xmm2, xmm0
		mulps xmm5, xmm3
		mulps xmm3, xmm1

		add rcx, 16

		subps xmm5, xmm4
		addps xmm2, xmm3
		
		movaps [rdx + rcx - 16], xmm5
		movaps [rax + rcx - 16], xmm2
		
		cmp rcx, rbx
		jl .LoopProcess

	;; begin epilog ;;
	pop rbx
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

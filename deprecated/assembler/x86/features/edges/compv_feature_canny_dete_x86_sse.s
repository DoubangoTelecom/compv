;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "../../compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(CannyNMSApply_Asm_X86_SSE2)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) uint16_t* grad
; arg(1) -> COMPV_ALIGNED(SSE) uint8_t* nms
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
sym(CannyNMSApply_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; alloc mem
	sub rsp, 8

	mov rcx, arg(4)
	shl rcx, 1
	pxor xmm0, xmm0
	mov [rsp + 0], rcx ; [rsp + 0] = strideInBytesTimes2

	mov rax, arg(0) ; rax = grad
	mov rdx, arg(1) ; rdx = nms
	mov rbx, arg(2) ; rbx = width
	mov rcx, arg(3) ; rcx = height
	dec rcx ; row start at 1

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (row_ = 1; row_ < height; ++row_)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopRows		
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (col_ = 0; col_ < width; col_ += 8)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor rdi, rdi
		.LoopCols
			movq xmm1, [rdx + rdi]
			pcmpeqb xmm1, xmm0
			pmovmskb rsi, xmm1
			xor rsi, 0xffff
			lea rdi, [rdi + 8]
			jz .AllZeros
				punpcklbw xmm1, xmm1
				pand xmm1, [rax + rdi*2 - 16]
				movq [rdx + rdi - 8], xmm0
				movdqa [rax + rdi*2 - 16], xmm1
			.AllZeros
			cmp rdi, rbx
			jl .LoopCols

		add rdx, arg(4)
		add rax, [rsp + 0]
		dec rcx
		jnz .LoopRows

	; free mem
	add rsp, 8

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

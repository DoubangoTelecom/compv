;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "../../compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(CannyNMSApply_Asm_X64_SSE2)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) uint16_t* grad
; arg(1) -> COMPV_ALIGNED(SSE) uint8_t* nms
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
sym(CannyNMSApply_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	push rdi
	;; end prolog ;;
	
	pxor xmm0, xmm0

	mov rax, arg(0) ; rax = grad
	mov rdx, arg(1) ; rdx = nms
	mov r10, arg(2) ; r10 = width
	mov rcx, arg(3) ; rcx = height
	dec rcx ; row start at 1
	mov r8, arg(4) ; r8 = stride
	lea r9, [r8*2] ; r9 = strideInBytesTimes2

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
			pmovmskb r11, xmm1
			xor r11, 0xffff
			lea rdi, [rdi + 8]
			jz .AllZeros
				punpcklbw xmm1, xmm1
				pand xmm1, [rax + rdi*2 - 16]
				movq [rdx + rdi - 8], xmm0
				movdqa [rax + rdi*2 - 16], xmm1
			.AllZeros
			cmp rdi, r10
			jl .LoopCols

		dec rcx
		lea rdx, [rdx + r8]
		lea rax, [rax + r9]
		jnz .LoopRows

	;; begin epilog ;;
	pop rdi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

%endif ; COMPV_YASM_ABI_IS_64BIT
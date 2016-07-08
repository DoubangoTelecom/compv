;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "../compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(MathUtilsAddAbs_16i16u_Asm_X86_SSSE3)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const COMPV_ALIGNED(SSE) int16_t* a
; arg(1) -> const COMPV_ALIGNED(SSE) int16_t* b
; arg(2) -> COMPV_ALIGNED(SSE) uint16_t* r
; arg(3) -> compv_uscalar_t width
; arg(4) -> compv_uscalar_t height
; arg(5) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
sym(MathUtilsAddAbs_16i16u_Asm_X86_SSSE3):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	push rsi
	push rdi
	push rbx
	;; end prolog ;;
	
	; alloc memory
	sub rsp, 8

	mov rcx, arg(5)
	mov rax, arg(0) ; rax = a
	mov rdx, arg(1) ; rdx = b
	shl rcx, 1
	mov rbx, arg(2) ; rbx = r
	mov rdi, arg(3) ; rdi = width
	mov rsi, arg(4) ; rsi = height
	mov [rsp + 0], rcx ; [rsp + 0] = strideInBytes

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopRows
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width; i += 8)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor rcx, rcx ; rcx = i = 0
		.LoopCols
			pabsw xmm0, [rax + rcx*2]
			pabsw xmm1, [rdx + rcx*2]
			lea rcx, [rcx + 8]
			paddusw xmm0, xmm1
			cmp rcx, rdi
			movdqa [rbx + rcx*2 - 16], xmm0
			jl .LoopCols

		add rax, [rsp + 0]
		add rdx, [rsp + 0]
		add rbx, [rsp + 0]
		dec rsi
		jnz .LoopRows

	; free memory
	add rsp, 8

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
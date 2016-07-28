;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(MemSetDword_Asm_X86)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> void* dstPtr
; arg(1) -> compv_scalar_t val
; arg(2) -> compv_uscalar_t count
sym(MemSetDword_Asm_X86):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 3
	push rdi
	; end prolog
	;mov rdi, arg(0)
	;mov rax, arg(1)
	;mov rcx, arg(2)
	;rep stosd
	
	mov rax, arg(1)
	movd xmm0, rax
	pshufd xmm0, xmm0, 0x0
	mov rcx, arg(2)
	mov rax, arg(0)
	.Loop
		movd [rax], xmm0
		dec rcx
		lea rax, [rax + 4]
		jnz .Loop
	
	; begin epilog
	pop rdi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret


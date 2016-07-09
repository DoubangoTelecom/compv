;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "../compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(MathUtilsSumAbs_16i16u_Asm_X86_SSSE3)
global sym(MathUtilsSum_8u32u_Asm_X86_SSSE3)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const COMPV_ALIGNED(SSE) int16_t* a
; arg(1) -> const COMPV_ALIGNED(SSE) int16_t* b
; arg(2) -> COMPV_ALIGNED(SSE) uint16_t* r
; arg(3) -> compv_uscalar_t width
; arg(4) -> compv_uscalar_t height
; arg(5) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
sym(MathUtilsSumAbs_16i16u_Asm_X86_SSSE3):
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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* data
; arg(1) -> compv_uscalar_t count
; arg(2) -> uint8_t *mean1
sym(MathUtilsSum_8u32u_Asm_X86_SSSE3):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 3
	push rsi
	push rdi
	push rbx
	;; end prolog ;;
	
	pxor xmm0, xmm0 ; xmm0 = xmmZer0
	pxor xmm3, xmm3 ; xmm3 = xmmSum
	mov rsi, arg(0) ; rsi = data
	mov rdx, arg(1)
	; rcx = i
	lea rbx, [rdx - 15] ; rbx = count - 15
	lea rax, [rdx - 7] ; rax = count - 7
	lea rdi, [rdx - 3] ; rdi = count - 3
	dec rdx ; rdx = count - 1

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (i = 0; i < count_ - 15; i += 16)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	xor rcx, rcx
	.LoopRows 
		movdqa xmm1, [rsi + rcx]
		lea rcx, [rcx + 16]
		movdqa xmm2, xmm1
		punpcklbw xmm1, xmm0
		punpckhbw xmm2, xmm0
		paddw xmm1, xmm2
		movdqa xmm2, xmm1
		cmp rcx, rbx
		punpcklwd xmm1, xmm0
		punpckhwd xmm2, xmm0
		paddd xmm1, xmm2
		paddd xmm3, xmm1		
		jl .LoopRows

	;;;;;;;;;;;;;;;;;;;;;;;;;;
	; if (i < count_ - 7)
	;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rcx, rax
	jge .EndOfMoreThanSevenRemains
		movq xmm1, [rsi + rcx]
		lea rcx, [rcx + 8]
		punpcklbw xmm1, xmm0
		movdqa xmm2, xmm1
		punpcklwd xmm1, xmm0
		punpckhwd xmm2, xmm0
		paddd xmm1, xmm2
		paddd xmm3, xmm1
	.EndOfMoreThanSevenRemains

	;;;;;;;;;;;;;;;;;;;;;;;;;;
	; if (i < count_ - 3)
	;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rcx, rdi
	jge .EndOfMoreThanThreeRemains
		movd xmm1, [rsi + rcx]
		lea rcx, [rcx + 4]
		punpcklbw xmm1, xmm0
		punpcklwd xmm1, xmm0
		paddd xmm3, xmm1
	.EndOfMoreThanThreeRemains

	mov rdi, arg(1) ; rdi = count

	phaddd xmm3, xmm3
	phaddd xmm3, xmm3
	movd rbx, xmm3

	;;;;;;;;;;;;;;;;;;;;;;;;
	; if (i < count_ - 1)
	;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rcx, rdx
	jge .EndOfMoreThanOneRemains
		movzx rdx, byte [rsi + rcx]
		movzx rax, byte [rsi + rcx + 1]
		lea rcx, [rcx + 2]
		add rax, rdx
		add rbx, rax
	.EndOfMoreThanOneRemains

	;;;;;;;;;;;;;;;;;;;;
	; if (count_ & 1)
	;;;;;;;;;;;;;;;;;;;;
	test rdi, 1
	jz .EndOfMoreThanZeroRemains
		movzx rdx, byte [rsi + rcx]
		add rbx, rdx
	.EndOfMoreThanZeroRemains

	mov rax, arg(2)
	mov [rax], dword ebx

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
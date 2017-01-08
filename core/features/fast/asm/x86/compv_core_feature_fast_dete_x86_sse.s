;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(CompVFastNmsGather_Asm_X86_SSE2)
global sym(CompVFastNmsApply_Asm_X86_SSE2)

section .data
	

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* pcStrengthsMap
; arg(1) -> uint8_t* pNMS
; arg(2) -> const compv_uscalar_t width
; arg(3) -> compv_uscalar_t heigth
; arg(4) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
sym(CompVFastNmsGather_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	COMPV_YASM_SAVE_XMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	mov rcx, arg(4)
	imul rcx, 3
	mov rax, arg(0) ; rax = pcStrengthsMap
	mov rdx, arg(1) ; rdx = pNMS
	lea rax, [rax + rcx]
	lea rdx, [rdx + rcx]

	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*0]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*1]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*2]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]

	mov rsi, arg(3)
	lea rsi, [rsi - 3] ; rsi = j
	pxor xmm0, xmm0
	pcmpeqb xmm1, xmm1
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 3; j < heigth - 3; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		mov rdi, 3 ; rdi = i
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 3; i < width - 3; i += 16) ; stride aligned on (width + 3) which means we can ignore the '-3' guard
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopWidth
			prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]
			movdqu xmm2, [rax + rdi]
			movdqa xmm3, xmm2
			pcmpeqb xmm3, xmm0
			pandn xmm3, xmm1
			pmovmskb rcx, xmm3
			test rcx, rcx
			jz .Next
			mov rbx, rdi
			mov rcx, rdi
			sub rbx, arg(4)
			add rcx, arg(4)
			movdqu xmm4, [rax + rdi - 1] ; left
			movdqu xmm5, [rax + rdi + 1] ; right
			movdqu xmm6, [rax + rbx - 1] ; left-top
			movdqu xmm7, [rax + rbx] ; top
			pminub xmm4, xmm2
			pminub xmm5, xmm2
			pminub xmm6, xmm2
			pminub xmm7, xmm2
			pcmpeqb xmm4, xmm2
			pcmpeqb xmm5, xmm2
			pcmpeqb xmm6, xmm2
			pcmpeqb xmm7, xmm2
			por xmm4, xmm5
			movdqu xmm5, [rax + rbx + 1] ; right-top
			por xmm4, xmm6
			movdqu xmm6, [rax + rcx - 1] ; left-bottom
			por xmm4, xmm7			
			movdqu xmm7, [rax + rcx] ; bottom
			pminub xmm5, xmm2
			pminub xmm6, xmm2
			pminub xmm7, xmm2
			pcmpeqb xmm5, xmm2
			pcmpeqb xmm6, xmm2
			pcmpeqb xmm7, xmm2
			por xmm4, xmm5
			movdqu xmm5, [rax + rcx + 1] ; right-bottom
			por xmm4, xmm6
			pminub xmm5, xmm2
			por xmm4, xmm7			
			pcmpeqb xmm5, xmm2
			por xmm4, xmm5
			pand xmm4, xmm2
			movdqu [rdx + rdi], xmm4
			.Next
			
			; end-of-LoopWidth
			lea rdi, [rdi + 16]
			cmp rdi, arg(2)
			jl .LoopWidth

		add rax, arg(4)
		add rdx, arg(4)
		; end-of-LoopHeight
		dec rsi
		jnz .LoopHeight

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
; arg(0) -> COMPV_ALIGNED(SSE) uint8_t* pcStrengthsMap
; arg(1) -> COMPV_ALIGNED(SSE) uint8_t* pNMS
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t heigth
; arg(4) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
sym(CompVFastNmsApply_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	COMPV_YASM_SAVE_XMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	mov rcx, arg(4)
	imul rcx, 3
	mov rax, arg(0) ; rax = pcStrengthsMap
	mov rdx, arg(1) ; rdx = pNMS
	lea rax, [rax + rcx]
	lea rdx, [rdx + rcx]

	prefetcht0 [rdx + COMPV_YASM_CACHE_LINE_SIZE*0]
	prefetcht0 [rdx + COMPV_YASM_CACHE_LINE_SIZE*1]
	prefetcht0 [rdx + COMPV_YASM_CACHE_LINE_SIZE*2]
	prefetcht0 [rdx + COMPV_YASM_CACHE_LINE_SIZE*3]

	mov rsi, arg(3)
	lea rsi, [rsi - 3] ; rsi = j
	mov rbx, arg(2) ; rbx = width
	pxor xmm0, xmm0
	pcmpeqb xmm1, xmm1
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 3; j < heigth - 3; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		xor rdi, rdi ; rdi = i
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopWidth
			prefetcht0 [rdx + COMPV_YASM_CACHE_LINE_SIZE*4]
			movdqa xmm2, [rdx + rdi]
			pcmpeqb xmm2, xmm0
			pandn xmm2, xmm1
			pmovmskb rcx, xmm2
			test rcx, rcx
			jz .Next
			pandn xmm2, [rax + rdi]
			movdqa [rdx + rdi], xmm0
			movdqa [rax + rdi], xmm2
			.Next
			
			; end-of-LoopWidth
			lea rdi, [rdi + 16]
			cmp rdi, rbx
			jl .LoopWidth

		add rax, arg(4)
		add rdx, arg(4)
		; end-of-LoopHeight
		dec rsi
		jnz .LoopHeight

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
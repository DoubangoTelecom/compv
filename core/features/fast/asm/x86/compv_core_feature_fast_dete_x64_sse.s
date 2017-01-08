;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"
%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(CompVFastNmsGather_Asm_X64_SSE2)
global sym(CompVFastNmsApply_Asm_X64_SSE2)

section .data
	

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* pcStrengthsMap
; arg(1) -> uint8_t* pNMS
; arg(2) -> const compv_uscalar_t width
; arg(3) -> compv_uscalar_t heigth
; arg(4) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
sym(CompVFastNmsGather_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	COMPV_YASM_SAVE_XMM 11
	push r12
	push r13
	push r14
	push r15
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

	mov r15, arg(2) ; r15 = width
	mov r8, arg(3)
	lea r8, [r8 - 3] ; r8 = j
	mov r11, arg(4) ; r11 = stride
	mov r13, 3
	mov r14, 3
	sub r13, r11 ; r13 = (i - stride)
	add r14, r11 ; r14 = (i + stride)
	pxor xmm0, xmm0
	pcmpeqb xmm1, xmm1
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 3; j < heigth - 3; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		mov r9, 3 ; r9 = i
		mov r10, r13 ; r10 = (i - stride)
		mov r12, r14 ; r12 = (i + stride)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 3; i < width - 3; i += 16) ; stride aligned on (width + 3) which means we can ignore the '-3' guard
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopWidth
			prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]
			movdqu xmm2, [rax + r9]
			movdqa xmm3, xmm2
			pcmpeqb xmm3, xmm0
			pandn xmm3, xmm1
			pmovmskb rcx, xmm3
			test rcx, rcx
			jz .Next
			movdqu xmm4, [rax + r9 - 1] ; left
			movdqu xmm5, [rax + r9 + 1] ; right
			movdqu xmm6, [rax + r10 - 1] ; left-top
			movdqu xmm7, [rax + r10] ; top
			movdqu xmm8, [rax + r10 + 1] ; right-top
			movdqu xmm9, [rax + r12 - 1] ; left-bottom
			movdqu xmm10, [rax + r12] ; bottom
			movdqu xmm11, [rax + r12 + 1] ; right-bottom
			pminub xmm4, xmm2
			pminub xmm5, xmm2
			pminub xmm6, xmm2
			pminub xmm7, xmm2
			pminub xmm8, xmm2
			pminub xmm9, xmm2
			pminub xmm10, xmm2
			pminub xmm11, xmm2
			pcmpeqb xmm4, xmm2
			pcmpeqb xmm5, xmm2
			pcmpeqb xmm6, xmm2
			pcmpeqb xmm7, xmm2
			pcmpeqb xmm8, xmm2
			pcmpeqb xmm9, xmm2
			pcmpeqb xmm10, xmm2
			pcmpeqb xmm11, xmm2
			por xmm4, xmm5
			por xmm4, xmm6
			por xmm4, xmm7
			por xmm4, xmm8
			por xmm4, xmm9
			por xmm4, xmm10
			por xmm4, xmm11
			pand xmm4, xmm2
			movdqu [rdx + r9], xmm4
			.Next
			
			lea r9, [r9 + 16]
			cmp r9, r15
			lea r10, [r10 + 16]
			lea r12, [r12 + 16]
			jl .LoopWidth
			; end-of-LoopWidth

		dec r8
		lea rax, [rax + r11]
		lea rdx, [rdx + r11]
		jnz .LoopHeight
		; end-of-LoopHeight

	;; begin epilog ;;
	pop r15
	pop r14
	pop r13
	pop r12
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
sym(CompVFastNmsApply_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	push r8
	push r9
	push r10
	;; end prolog ;;


	;; begin epilog ;;
	pop r10
	pop r9
	pop r8
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endif ; COMPV_YASM_ABI_IS_64BIT
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"


COMPV_YASM_DEFAULT_REL

global sym(MemSetDword_Asm_X86_SSE2)
global sym(MemSetQword_Asm_X86_SSE2)
global sym(MemSetDQword_Asm_X86_SSE2)
global sym(MemCopyNTA_Asm_Aligned11_X86_SSE2)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; %1 -> sizeOfEltmInBytes
; %2 -> instruction
; arg(0) -> void* dstPtr
; arg(1) -> compv_scalar_t val
; arg(2) -> compv_uscalar_t count
%macro MemSet_Asm_X86_SSE2 2
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 3	
	mov rax, arg(1)
	movd xmm0, rax
	pshufd xmm0, xmm0, 0x0
	mov rcx, arg(2)
	mov rax, arg(0)
	.Loop
		%2 [rax], xmm0
		dec rcx
		lea rax, [rax + %1]
		jnz .Loop
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endmacro
sym(MemSetDword_Asm_X86_SSE2):
	MemSet_Asm_X86_SSE2 4, movd
sym(MemSetQword_Asm_X86_SSE2):
	MemSet_Asm_X86_SSE2 8, movq
sym(MemSetDQword_Asm_X86_SSE2):
	MemSet_Asm_X86_SSE2 16, movdqa

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; size must be > 16 and it's up to the caller to check it
; size should be multiple of 16, if not the remaining will be ignored
; arg(0) -> COMPV_ALIGNED(SSE) void* dataDstPtr
; arg(1) ->COMPV_ALIGNED(SSE) const void* dataSrcPtr
; arg(2) ->compv_uscalar_t size
; void MemCopyNTA_Asm_Aligned11_X86_SSE2(COMPV_ALIGNED(SSE) void* dataDstPtr, COMPV_ALIGNED(SSE) const void* dataSrcPtr, compv_uscalar_t size)
sym(MemCopyNTA_Asm_Aligned11_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 3
	COMPV_YASM_SAVE_XMM 7 ;XMM[6-n]
	push rsi
	push rdi
	push rbx
	; end prolog

	mov rax, arg(2) ; size
	mov rdi, arg(0) ; dataDstPtr
	mov rsi, arg(1) ; dataSrcPtr
	mov rcx, rax
	mov rdx, rax
	shr rcx, 8 ; rcx = count16x16
	shr rdx, 4 ; rdx = count16x1

	; Check if we have to execute Count16x16Loop
	test rcx, rcx
	jz .EndOfCount16x16Loop
	mov rbx, rcx
	shl rbx, 8
	sub rax, rbx
	shr rax, 4
	mov rdx, rax

	; Begin Count16x16Loop
	align 16
	.Count16x16Loop
		prefetchnta [rsi + COMPV_YASM_CACHE_LINE_SIZE*5]
		prefetchnta [rsi + COMPV_YASM_CACHE_LINE_SIZE*6]
		prefetchnta [rsi + COMPV_YASM_CACHE_LINE_SIZE*7]
		prefetchnta [rsi + COMPV_YASM_CACHE_LINE_SIZE*8]

		%assign i 0
		%rep 2
			%assign j 0
			%rep 8
				movdqa xmm %+ j, [rsi + i + j*16]
				%assign j j+1
			%endrep
			%assign j 0
			%rep 8
				movntdq [rdi + i + j*16], xmm %+ j
				%assign j j+1
			%endrep
			%assign i i+8*16
		%endrep
			
		add rsi, 16*16
		add rdi, 16*16

		dec rcx
		jnz .Count16x16Loop
	; End Count16x16Loop
	.EndOfCount16x16Loop

	; Check if we have to execute Count16x1Loop
	test rdx, rdx
	jz .EndOfCount16x1Loop

	; Begin Count16x1Loop
	align 16
	.Count16x1Loop
		movdqa xmm0, [rsi]
		add rsi, 16
		movntdq [rdi], xmm0
		add rdi, 16
	
		dec rdx
		jnz .Count16x1Loop
	; End Count16x1Loop
	.EndOfCount16x1Loop

	mfence ; flush latest WC (Write Combine) buffers to memory

	; begin epilog
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
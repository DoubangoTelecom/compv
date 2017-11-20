;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"


COMPV_YASM_DEFAULT_REL

global sym(CompVMemCopyNTA_Asm_Aligned11_X86_AVX)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; size must be > 32 and it's up to the caller to check it
; size should be multiple of 32, if not the remaining will be ignored
; arg(0) -> COMPV_ALIGNED(AVX) void* dataDstPtr
; arg(1) ->COMPV_ALIGNED(AVX) const void* dataSrcPtr
; arg(2) ->compv_uscalar_t size
; void CompVMemCopyNTA_Asm_Aligned11_X86_SSE2(COMPV_ALIGNED(AVX) void* dataDstPtr, COMPV_ALIGNED(AVX) const void* dataSrcPtr, compv_uscalar_t size)
sym(CompVMemCopyNTA_Asm_Aligned11_X86_AVX):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 3
	COMPV_YASM_SAVE_YMM 7 ;YMM[6-n]
	push rsi
	push rdi
	push rbx
	; end prolog

	%define cache_line_size 64

	mov rax, arg(2) ; size
	mov rdi, arg(0) ; dataDstPtr
	mov rsi, arg(1) ; dataSrcPtr
	mov rcx, rax
	mov rdx, rax
	shr rcx, 9 ; rcx = count16x32
	shr rdx, 5 ; rdx = count32x1

	; Check if we have to execute Count32x16Loop
	test rcx, rcx
	jz .EndOfCount32x16Loop
	mov rbx, rcx
	shl rbx, 9
	sub rax, rbx
	shr rax, 5
	mov rdx, rax

	; Begin Count32x16Loop
	align 16
	.Count32x16Loop

	%assign i 0
	%rep 2
		%assign j 0
		%rep 8
			vmovdqa ymm %+ j, [rsi + i + j*32]
			%assign j j+1
		%endrep
		%assign j 0
		%rep 8
			vmovntdq [rdi + i + j*32], ymm %+ j
			%assign j j+1
		%endrep
		%assign i i+8*32
	%endrep
			
	add rsi, 32*16
	add rdi, 32*16

	dec rcx
	jnz .Count32x16Loop
	; End Count32x16Loop
	.EndOfCount32x16Loop

	; Check if we have to execute Count32x1Loop
	test rdx, rdx
	jz .EndOfCount32x1Loop

	; Begin Count32x1Loop
	align 16
	.Count32x1Loop
	vmovdqa ymm0, [rsi]
	add rsi, 32
	vmovntdq [rdi], ymm0
	add rdi, 32
	
	dec rdx
	jnz .Count32x1Loop
	; End Count32x1Loop
	.EndOfCount32x1Loop

	; begin epilog
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_YMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
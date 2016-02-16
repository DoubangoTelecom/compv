; Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
; Copyright (C) 2016 Mamadou DIOP
;
; This file is part of Open Source ComputerVision (a.k.a CompV) project.
; Source code hosted at https://github.com/DoubangoTelecom/compv
; Website hosted at http://compv.org
;
; CompV is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; CompV is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with CompV.
;
%include "compv_common_x86.S"

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(MemCopyNTA_Asm_Aligned11_X64_AVX)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; size must be > 32 and it's up to the caller to check it
; size should be multiple of 32, if not the remaining will be ignored
; arg(0) -> COMPV_ALIGNED(AVX) void* dataDstPtr
; arg(1) ->COMPV_ALIGNED(AVX) const void* dataSrcPtr
; arg(2) ->compv_uscalar_t size
; void MemCopyNTA_Asm_Aligned11_X64_AVX(COMPV_ALIGNED(AVX) void* dataDstPtr, COMPV_ALIGNED(AVX) const void* dataSrcPtr, compv_uscalar_t size)
sym(MemCopyNTA_Asm_Aligned11_X64_AVX):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 3
	COMPV_YASM_SAVE_XMM 15 ;XMM[6-n]
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
		prefetchnta [rsi + cache_line_size*5]
		prefetchnta [rsi + cache_line_size*6]
		prefetchnta [rsi + cache_line_size*7]
		prefetchnta [rsi + cache_line_size*8]

		%assign j 0
		%rep 16
			vmovdqa ymm %+ j, [rsi + j*32]
			%assign j j+1
		%endrep
		%assign j 0
		%rep 16
			vmovntdq [rdi + j*32], ymm %+ j
			%assign j j+1
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
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

%endif ; COMPV_YASM_ABI_IS_64BIT
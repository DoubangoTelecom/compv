; Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
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
%include "compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(HammingDistance_Asm_POPCNT_X86_SSE42)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* dataPtr
; arg(1) -> compv_scalar_t width
; arg(2) -> compv_scalar_t stride
; arg(3) -> compv_scalar_t height
; arg(4) -> COMPV_ALIGNED(SSE) const uint8_t* patch1xnPtr
; arg(5) -> int32_t* distPtr
; void HammingDistance_Asm_POPCNT_X86_SSE42(COMPV_ALIGNED(SSE) const uint8_t* dataPtr, compv_scalar_t width, compv_scalar_t stride, compv_scalar_t height, COMPV_ALIGNED(SSE) const uint8_t* patch1xnPtr, int32_t* distPtr)
sym(HammingDistance_Asm_POPCNT_X86_SSE42):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; alloc memory
	sub rsp, 8 + 8 + 8 + 8
	%define i_sub16		rsp + 0
	%define i_sub4		rsp + 8
	%define i_sub2		rsp + 16
	%define i_sub1		rsp + 24

	mov rax, arg(1)
	sub rax, 1
	mov [i_sub1], rax
	sub rax, 1
	mov [i_sub2], rax
	sub rax, 2
	mov [i_sub4], rax
	sub rax, 12
	mov [i_sub16], rax

	; rdi = j = 0
	xor rdi, rdi

	; rcx = dataPtr
	mov rcx, arg(0)

	; rdx = patch1xnPtr
	mov rdx, arg(4)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopRows
		xor rbx, rbx ; rbx = cnt
		xor rsi, rsi ; rsi = i

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i <= width - 16; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp rsi, [i_sub16]
		jg .EndOfLoopCols16
		.LoopCols16
			movdqa xmm0, [rcx + rsi]
			movdqa xmm1, [rdx + rsi]
			pxor xmm0, xmm1
			movd eax, xmm0
			popcnt eax, eax
			add ebx, eax
			pextrd eax, xmm0, 1
			popcnt eax, eax
			add ebx, eax
			pextrd eax, xmm0, 2
			popcnt eax, eax
			add ebx, eax
			pextrd eax, xmm0, 3
			popcnt eax, eax
			add ebx, eax
		
			add rsi, 16
			cmp rsi, [i_sub16]
			jle .LoopCols16
			.EndOfLoopCols16

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i <= width - 4; i += 4)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp rsi, [i_sub4]
		jg .EndOfLoopCols4
		.LoopCols4
			mov eax, dword [rcx + rsi]
			xor eax, dword [rdx + rsi]
			popcnt eax, eax
			add ebx, eax

			add rsi, 4
			cmp rsi, [i_sub4]
			jle .LoopCols4
			.EndOfLoopCols4

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i <= width - 2)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp rsi, [i_sub2]
		jg .EndOfIf2
			mov ax, word [rcx + rsi]
			xor ax, word [rdx + rsi]
			popcnt ax, ax
			add ebx, eax

			add rsi, 2
			.EndOfIf2

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i <= width - 1)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp rsi, [i_sub1]
		jg .EndOfIf1
			mov al, byte [rcx + rsi]
			xor al, byte [rdx + rsi]
			popcnt ax, ax
			add ebx, eax

			add rsi, 1
			.EndOfIf1

		mov rax, arg(5) ; distPtr
		add rcx, arg(2) ; dataPtr += stride
		mov [rax + rdi*4], ebx ; distPtr[j] = (int32_t)(cnt)

		inc rdi
		cmp rdi, arg(3)
		jl .LoopRows

	; free memory
	add rsp, 8 + 8 + 8 + 8
	%undef i_sub16
	%undef i_sub4
	%undef i_sub2
	%undef i_sub1

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
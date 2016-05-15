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

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(HammingDistance_Asm_POPCNT_X64_SSE42)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* dataPtr
; arg(1) -> compv_scalar_t width
; arg(2) -> compv_scalar_t stride
; arg(3) -> compv_scalar_t height
; arg(4) -> COMPV_ALIGNED(SSE) const uint8_t* patch1xnPtr
; arg(5) -> int32_t* distPtr
; void HammingDistance_Asm_POPCNT_X64_SSE42(COMPV_ALIGNED(SSE) const uint8_t* dataPtr, compv_scalar_t width, compv_scalar_t stride, compv_scalar_t height, COMPV_ALIGNED(SSE) const uint8_t* patch1xnPtr, int32_t* distPtr)
sym(HammingDistance_Asm_POPCNT_X64_SSE42):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	push r14
	push r15
	;; end prolog ;;

	mov rax, arg(1)
	sub rax, 1
	mov r11, rax ; r11 = (width - 1)
	sub rax, 1
	mov r12, rax ; r12 = (width - 2)
	sub rax, 2
	mov r13, rax ; r13 = (width - 4)
	sub rax, 12
	mov r10, rax ; r10 = (width - 16)
	sub rax, 16 ; rax = (width - 32)

	; rdi = j = 0
	xor rdi, rdi

	; rcx = dataPtr
	mov rcx, arg(0)

	; rdx = patch1xnPtr
	mov rdx, arg(4)

	; r8 = stride
	mov r8, arg(2)

	; r9 = distPtr
	mov r9, arg(5)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopRows
		xor rbx, rbx ; rbx = cnt
		xor rsi, rsi ; rsi = i

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i <= width - 32; i += 32)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp rsi, rax
		jg .EndOfLoopCols32
		.LoopCols32
			movdqa xmm0, [rcx + rsi]
			movdqa xmm2, [rcx + rsi + 16]
			movdqa xmm1, [rdx + rsi]
			movdqa xmm3, [rdx + rsi + 16]
			pxor xmm0, xmm1
			pxor xmm2, xmm3
			movq r14, xmm0
			pextrq r15, xmm0, 1
			popcnt r14, r14
			popcnt r15, r15
			add rbx, r14
			add rbx, r15
			movq r14, xmm2
			pextrq r15, xmm2, 1
			popcnt r14, r14
			popcnt r15, r15
			add rbx, r14
			add rbx, r15
					
			add rsi, 32
			cmp rsi, rax
			jle .LoopCols32
			.EndOfLoopCols32

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i <= width - 16; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp rsi, r10
		jg .EndOfLoopCols16
		.LoopCols16
			movdqa xmm0, [rcx + rsi]
			movdqa xmm1, [rdx + rsi]
			pxor xmm0, xmm1
			movd r14, xmm0
			pextrq r15, xmm0, 1
			popcnt r14, r14
			popcnt r15, r15
			add rbx, r14
			add rbx, r15
					
			add rsi, 16
			cmp rsi, r10
			jle .LoopCols16
			.EndOfLoopCols16

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i <= width - 4; i += 4)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp rsi, r13
		jg .EndOfLoopCols4
		.LoopCols4
			mov r14d, dword [rcx + rsi]
			xor r14d, dword [rdx + rsi]
			popcnt r14d, r14d
			add rbx, r14

			add rsi, 4
			cmp rsi, r13
			jle .LoopCols4
			.EndOfLoopCols4

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i <= width - 2)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp rsi, r12
		jg .EndOfIf2
			mov r14w, word [rcx + rsi]
			xor r14w, word [rdx + rsi]
			popcnt r14w, r14w
			add rbx, r14

			add rsi, 2
			.EndOfIf2

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i <= width - 1)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp rsi, r11
		jg .EndOfIf1
			mov r14b, byte [rcx + rsi]
			xor r14b, byte [rdx + rsi]
			popcnt r14w, r14w
			add rbx, r14

			inc rsi
			.EndOfIf1
		
		add rcx, r8 ; dataPtr += stride
		mov [r9 + rdi*4], dword ebx ; distPtr[j] = (int32_t)(cnt)

		inc rdi
		cmp rdi, arg(3)
		jl .LoopRows

	;; begin epilog ;;
	pop r15
	pop r14
	pop r13
	pop r12
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

%endif ; COMPV_YASM_ABI_IS_64BIT
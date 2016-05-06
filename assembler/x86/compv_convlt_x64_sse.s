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

global sym(Convlt1_verthz_float32_minpack4_Asm_X64_SSE2)
global sym(Convlt1_verthz_fxpq16_minpack4_Asm_X64_SSE2)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; This function requires sizeof(float) = 4byte = 32bits
; arg(0) -> const uint8_t* in_ptr
; arg(1) -> uint8_t* out_ptr
; arg(2) -> compv_scalar_t width
; arg(3) -> compv_scalar_t height
; arg(4) -> compv_scalar_t stride
; arg(5) -> compv_scalar_t pad
; arg(6) -> const float* hkern_ptr
; arg(7) -> compv_scalar_t kern_size
; void Convlt1_verthz_float32_minpack4_Asm_X64_SSE2(const uint8_t* in_ptr, uint8_t* out_ptr, compv_scalar_t width, compv_scalar_t height, compv_scalar_t stride, compv_scalar_t pad, const float* hkern_ptr, compv_scalar_t kern_size)
sym(Convlt1_verthz_float32_minpack4_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_XMM 9 ;XMM[6-9]
	push rsi
	push rdi
	push rbx
	push r12
	;; end prolog ;;

	%define COMPV_SIZE_OF_FLOAT 4 ; up to the caller to make sure sizeof(float)=4

	; i = rdi
	; xor rdi, rdi

	; rcx = col

	; rbx = out_ptr
	mov rbx, arg(1)

	; j = rsi = height
	mov rsi, arg(3)

	; xmm7 = xmmZero
	pxor xmm7, xmm7

	; r9 = (pad += (width & 3))
	mov rdx, arg(2) ; width
	mov r9, arg(5) ; pad
	and rdx, 3
	add r9, rdx

	; rax = in_ptr
	mov rax, arg(0)

	; rdx = hkern_ptr
	mov rdx, arg(6)

	; r8 = kern_size
	mov r8, arg(7)

	; r10 = width
	mov r10, arg(2)

	; r11 = stride
	mov r11, arg(4)

	; r12 reserved
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopRows
		mov rdi, r10 ; i = width
		cmp r10, 16
		jl .EndOfLoopColumns16
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; while (i > 15)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopColumns16
			mov r12, rax ; in_ptr
			xor rcx, rcx ; col = 0
			movaps xmm5, xmm7 ; xmm5 = xmmSF0 = xmmZero
			movaps xmm6, xmm7 ; xmm6 = xmmSF1 = xmmZero
			movaps xmm4, xmm7 ; xmm4 = xmmSF2 = xmmZero
			movaps xmm9, xmm7 ; xmm9 = xmmSF3 = xmmZero
			
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (col = 0; col < kern_size; ++col)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			.LoopColumns16Kern16
				movdqu xmm0, [r12] ; xmm0 = xmmI0
				movss xmm1, [rdx + rcx*COMPV_SIZE_OF_FLOAT]
				movdqa xmm2, xmm0
				movdqa xmm3, xmm0
				movdqa xmm8, xmm0
				shufps xmm1, xmm1, 0x0 ; xmm1 = xmmCoeff
				
				punpcklbw xmm2, xmm7
				punpcklbw xmm3, xmm7
				punpckhbw xmm8, xmm7
				punpckhbw xmm0, xmm7
				punpcklwd xmm2, xmm7
				punpckhwd xmm3, xmm7
				punpcklwd xmm8, xmm7
				punpckhwd xmm0, xmm7
				cvtdq2ps xmm2, xmm2
				cvtdq2ps xmm3, xmm3
				cvtdq2ps xmm8, xmm8
				cvtdq2ps xmm0, xmm0
				mulps xmm2, xmm1
				mulps xmm3, xmm1
				mulps xmm8, xmm1
				mulps xmm0, xmm1
				addps xmm5, xmm2
				addps xmm6, xmm3
				addps xmm4, xmm8
				addps xmm9, xmm0

				inc rcx
				add r12, r11 ; += stride
				cmp rcx, r8 ; ==? kern_size
				jl .LoopColumns16Kern16		

			cvtps2dq xmm5, xmm5
			cvtps2dq xmm6, xmm6
			cvtps2dq xmm4, xmm4
			cvtps2dq xmm9, xmm9
			packssdw xmm5, xmm6
			packssdw xmm4, xmm9
			packuswb xmm5, xmm4
			lea rax, [rax + 16] ; in_ptr += 16
			movdqu [rbx], xmm5
			lea rbx, [rbx + 16] ; out_ptr += 16

			sub rdi, 16 ; i -= 16
			cmp rdi, 16
			jge .LoopColumns16
			.EndOfLoopColumns16

		cmp rdi, 4
		jl .EndOfLoopColumns4
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; while (i > 3)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopColumns4
			mov r12, rax ; in_ptr
			xor rcx, rcx ; col = 0
			movaps xmm4, xmm7 ; xmm4 = xmmSF0 = xmmZero
			
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (col = 0; col < kern_size; ++col)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			.LoopColumns4Kern16
				movd xmm0, [r12] ; xmm0 = xmmI0
				movss xmm1, [rdx + rcx*COMPV_SIZE_OF_FLOAT]
				punpcklbw xmm0, xmm7
				shufps xmm1, xmm1, 0x0 ; xmm1 = xmmCoeff
				punpcklwd xmm0, xmm7
				cvtdq2ps xmm0, xmm0
				mulps xmm0, xmm1
				addps xmm4, xmm0

				inc rcx
				add r12, r11 ; += stride
				cmp rcx, r8 ; ==? kern_size
				jl .LoopColumns4Kern16

			cvtps2dq xmm4, xmm4
			packssdw xmm4, xmm4
			packuswb xmm4, xmm4
			movd [rbx], xmm4

			lea rbx, [rbx + 4] ; out_ptr += 4
			lea rax, [rax + 4] ; in_ptr += 4

			sub rdi, 4 ; i -= 4
			cmp rdi, 4
			jge .LoopColumns4
			.EndOfLoopColumns4
		
		lea rbx, [rbx + r9] ; out_ptr += pad
		lea rax, [rax + r9] ; in_ptr += pad

		dec rsi ; --j
		test rsi, rsi
		jnz .LoopRows

	%undef COMPV_SIZE_OF_FLOAT

	;; begin epilog ;;
	pop r12
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; This function requires sizeof(float) = 4byte = 32bits
; arg(0) -> const uint8_t* in_ptr
; arg(1) -> uint8_t* out_ptr
; arg(2) -> compv_scalar_t width
; arg(3) -> compv_scalar_t height
; arg(4) -> compv_scalar_t stride
; arg(5) -> compv_scalar_t pad
; arg(6) -> const uint16_t* hkern_ptr
; arg(7) -> compv_scalar_t kern_size
; void Convlt1_verthz_fxpq16_minpack4_Asm_X64_SSE2(const uint8_t* in_ptr, uint8_t* out_ptr, compv_scalar_t width, compv_scalar_t height, compv_scalar_t stride, compv_scalar_t pad, const uint16_t* hkern_ptr, compv_scalar_t kern_size)
sym(Convlt1_verthz_fxpq16_minpack4_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	push rsi
	push rdi
	push rbx
	push r12
	;; end prolog ;;

	%define COMPV_SIZE_OF_INT16 2

	; i = rdi
	; xor rdi, rdi

	; rcx = col

	; rbx = out_ptr
	mov rbx, arg(1)

	; j = rsi = height
	mov rsi, arg(3)

	; xmm6 = xmmZero
	pxor xmm6, xmm6

	; r9 = (pad += (width & 3))
	mov rdx, arg(2) ; width
	mov r9, arg(5) ; pad
	and rdx, 3
	add r9, rdx

	; rax = in_ptr
	mov rax, arg(0)

	; rdx = hkern_ptr
	mov rdx, arg(6)

	; r8 = kern_size
	mov r8, arg(7)

	; r10 = width
	mov r10, arg(2)

	; r11 = stride
	mov r11, arg(4)

	; r12 reserved
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopRows
		mov rdi, r10 ; i = width
		cmp r10, 16
		jl .EndOfLoopColumns16
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; while (i > 15)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopColumns16
			mov r12, rax ; in_ptr
			xor rcx, rcx ; col = 0
			pxor xmm4, xmm4 ; xmm4 = xmmS0
			pxor xmm5, xmm5 ; xmm5 = xmmS1
			
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (col = 0; col < kern_size; ++col)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			.LoopColumns16Kern16
				movd xmm1, [rdx + rcx*COMPV_SIZE_OF_INT16]
				movdqu xmm0, [r12] ; xmm0 = xmmI0
				punpcklwd xmm1, xmm1			
				movdqa xmm2, xmm0
				pshufd xmm1, xmm1, 0 ; xmm1 = xmmCoeff
				punpcklbw xmm0, xmm6
				punpckhbw xmm2, xmm6
				pmulhuw xmm0, xmm1
				pmulhuw xmm2, xmm1
				paddw xmm4, xmm0
				paddw xmm5, xmm2

				inc rcx
				add r12, r11 ; += stride
				cmp rcx, r8 ; ==? kern_size
				jl .LoopColumns16Kern16		

			packuswb xmm4, xmm5
			lea rax, [rax + 16] ; in_ptr += 16
			movdqu [rbx], xmm4
			lea rbx, [rbx + 16] ; out_ptr += 16

			sub rdi, 16 ; i -= 16
			cmp rdi, 16
			jge .LoopColumns16
			.EndOfLoopColumns16

		cmp rdi, 4
		jl .EndOfLoopColumns4
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; while (i > 3)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopColumns4
			mov r12, rax ; in_ptr
			xor rcx, rcx ; col = 0
			pxor xmm4, xmm4 ; xmm4 = xmmS0
			
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (col = 0; col < kern_size; ++col)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			.LoopColumns4Kern16
				movd xmm1, [rdx + rcx*COMPV_SIZE_OF_INT16]
				movdqu xmm0, [r12] ; xmm0 = xmmI0
				punpcklwd xmm1, xmm1
				punpcklbw xmm0, xmm6		
				pshufd xmm1, xmm1, 0 ; xmm1 = xmmCoeff				
				pmulhuw xmm0, xmm1
				paddw xmm4, xmm0

				inc rcx
				add r12, r11 ; += stride
				cmp rcx, r8 ; ==? kern_size
				jl .LoopColumns4Kern16

			packuswb xmm4, xmm4
			movd [rbx], xmm4

			lea rbx, [rbx + 4] ; out_ptr += 4
			lea rax, [rax + 4] ; in_ptr += 4

			sub rdi, 4 ; i -= 4
			cmp rdi, 4
			jge .LoopColumns4
			.EndOfLoopColumns4
		
		lea rbx, [rbx + r9] ; out_ptr += pad
		lea rax, [rax + r9] ; in_ptr += pad

		dec rsi ; --j
		test rsi, rsi
		jnz .LoopRows

	%undef COMPV_SIZE_OF_INT16

	;; begin epilog ;;
	pop r12
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

%endif ; COMPV_YASM_ABI_IS_64BIT
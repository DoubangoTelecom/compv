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
%include "../../compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(ScaleBilinear_Asm_X86_SSE41)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* inPtr
; arg(1) -> COMPV_ALIGNED(SSE) uint8_t* outPtr
; arg(2) -> compv_scalar_t inHeight
; arg(3) -> compv_scalar_t inWidth
; arg(4) -> compv_scalar_t inStride
; arg(5) -> compv_scalar_t outHeight
; arg(6) -> compv_scalar_t outWidth
; arg(7) -> COMPV_ALIGNED(SSE) compv_scalar_t outStride
; arg(8) -> compv_scalar_t sf_x
; arg(9) -> compv_scalar_t sf_y
; void ScaleBilinear_Asm_X86_SSE41(const uint8_t* inPtr, COMPV_ALIGNED(SSE) uint8_t* outPtr, compv_scalar_t inHeight, compv_scalar_t inWidth, compv_scalar_t inStride, compv_scalar_t outHeight, compv_scalar_t outWidth, COMPV_ALIGNED(SSE) compv_scalar_t outStride, compv_scalar_t sf_x, compv_scalar_t sf_y)
sym(ScaleBilinear_Asm_X86_SSE41):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 10
	COMPV_YASM_SAVE_XMM 7 ; XMM[6-7]
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	%define i_xmmSFX			rsp + 0
	%define i_xmmOnes			rsp + 16
	%define i_xmmZeros			rsp + 32
	%define i_xmmInStride		rsp + 48
	%define i_xmmFF				rsp + 64
	%define i_xmmXStart			rsp + 80
	%define i_xmmTempNearestX	rsp + 96
	%define i_xmmNearestX		rsp + 112
	%define i_xmmTempNearestX	rsp + 128
	%define i_xmmNeighb			rsp + 144

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 16, rax
	sub rsp, 16*1 + 16*1 + 16*1 + 16*1 + 16*1 + 16*1 + 16*1 + 4*4 + 4*4 + 1*16
	; [rsp + 0] -> __m128i xmmSFX
	; [rsp + 16] -> __m128i xmmOnes
	; [rsp + 32] -> __m128i xmmZeros
	; [rsp + 48] -> __m128i xmmInStride
	; [rsp + 64] -> __m128i xmmFF
	; [rsp + 80] -> __m128i xmmXStart
	; [rsp + 96] -> __m128i xmmTempNearestX
	; [rsp + 112] -> int32_t xmmNearestX[4]
	; [rsp + 128] -> int32_t xmmTempNearestX[4]
	; [rsp + 144] -> uint8_t xmmNeighb[16]

	mov rax, arg(8) ; sf_x
	mov rcx, 1
	mov rdx, 0xFF
	mov rbx, arg(4) ; inStride
	shl rax, 2
	movd xmm0, rcx
	movd xmm1, rdx
	movd xmm2, rax
	movd xmm3, rbx
	pxor xmm4, xmm4
	pshufd xmm0, xmm0, 0x0
	pshufd xmm1, xmm1, 0x0
	pshufd xmm2, xmm2, 0x0
	pshufd xmm3, xmm3, 0x0
	movdqa [i_xmmOnes], xmm0
	movdqa [i_xmmFF], xmm1
	movdqa [i_xmmSFX], xmm2
	movdqa [i_xmmInStride], xmm3
	movdqa [i_xmmZeros], xmm4
	xor rax, rax ; sf_x * 0
	mov rbx, arg(8) ; sf_x * 1
	mov rdx, rbx
	mov rcx, rbx
	add rdx, rbx ; sf_x * 2
	add rcx, rdx ; sf_x * 3
	movd xmm0, rax
	movd xmm1, rbx
	movd xmm2, rdx
	movd xmm3, rcx
	punpckldq xmm0, xmm1
	punpckldq xmm2, xmm3
	punpcklqdq xmm0, xmm2
	movdqa [i_xmmXStart], xmm0

	; rcx = outPad
	mov rax, arg(6) ; outWidth
	mov rcx, arg(7) ; outStride
	add rax, 3
	and rax, -4
	sub rcx, rax

	; rsi = outHeight
	mov rsi, arg(5)

	; rbx = y = 0
	xor rbx, rbx

	; rdi = i

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0, y = 0; j < outHeight; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopRows
		xor rdi, rdi ; rdi = i = 0
		mov rax, rbx
		mov rdx, arg(4) ; inStride
		shr rax, 8 ; nearestY
		imul rax, rdx
		mov rdx, arg(0) ; inPtr
		add rdx, rax ; rdx = _inPtr
		movd xmm0, rbx
		pshufd xmm0, xmm0, 0x0 ; xmm0 = xmmY
		movdqa xmm1, [i_xmmXStart] ; xmm1 = xmmX

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < outWidth; i+= 4)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopColumns
			movdqa xmm2, xmm1
			psrld xmm2, 8
			movdqa [i_xmmNearestX], xmm2

			; neighb0 = inPtr_[nearestX]
			mov eax, dword [i_xmmNearestX + 0*4]
			lea rax, [rdx + rax]
			movzx rax, byte [rax]
			mov [i_xmmNeighb + 0*1], ax
			mov eax, dword [i_xmmNearestX + 1*4]
			lea rax, [rdx + rax]
			movzx rax, byte [rax]
			mov [i_xmmNeighb + 1*1], ax
			mov eax, dword [i_xmmNearestX + 2*4]
			lea rax, [rdx + rax]
			movzx rax, byte [rax]
			mov [i_xmmNeighb + 2*1], ax
			mov eax, dword [i_xmmNearestX + 3*4]
			lea rax, [rdx + rax]
			movzx rax, byte [rax]
			mov [i_xmmNeighb + 3*1], ax
			movdqa xmm7, [i_xmmNeighb]
			punpcklbw xmm7, [i_xmmZeros]
			punpcklwd xmm7, [i_xmmZeros]

			; neighb1 = inPtr_[nearestX + 1]
			movdqa xmm2, [i_xmmNearestX]
			paddd xmm2, [i_xmmOnes]
			movdqa [i_xmmTempNearestX], xmm2
			mov eax, dword [i_xmmTempNearestX + 0*4]
			lea rax, [rdx + rax]
			movzx rax, byte [rax]
			mov [i_xmmNeighb + 0*1], ax
			mov eax, dword [i_xmmTempNearestX + 1*4]
			lea rax, [rdx + rax]
			movzx rax, byte [rax]
			mov [i_xmmNeighb + 1*1], ax
			mov eax, dword [i_xmmTempNearestX + 2*4]
			lea rax, [rdx + rax]
			movzx rax, byte [rax]
			mov [i_xmmNeighb + 2*1], ax
			mov eax, dword [i_xmmTempNearestX + 3*4]
			lea rax, [rdx + rax]
			movzx rax, byte [rax]
			mov [i_xmmNeighb + 3*1], ax
			movdqa xmm6, [i_xmmNeighb]
			punpcklbw xmm6, [i_xmmZeros]
			punpcklwd xmm6, [i_xmmZeros]

			; neighb2 = inPtr_[nearestX + inStride]
			movdqa xmm2, [i_xmmNearestX]
			paddd xmm2, [i_xmmInStride]
			movdqa [i_xmmTempNearestX], xmm2
			mov eax, dword [i_xmmTempNearestX + 0*4]
			lea rax, [rdx + rax]
			movzx rax, byte [rax]
			mov [i_xmmNeighb + 0*1], ax
			mov eax, dword [i_xmmTempNearestX + 1*4]
			lea rax, [rdx + rax]
			movzx rax, byte [rax]
			mov [i_xmmNeighb + 1*1], ax
			mov eax, dword [i_xmmTempNearestX + 2*4]
			lea rax, [rdx + rax]
			movzx rax, byte [rax]
			mov [i_xmmNeighb + 2*1], ax
			mov eax, dword [i_xmmTempNearestX + 3*4]
			lea rax, [rdx + rax]
			movzx rax, byte [rax]
			mov [i_xmmNeighb + 3*1], ax
			movdqa xmm5, [i_xmmNeighb]
			punpcklbw xmm5, [i_xmmZeros]
			punpcklwd xmm5, [i_xmmZeros]

			; neighb3 = inPtr_[nearestX + inStride + 1]
			movdqa xmm2, [i_xmmTempNearestX]
			paddd xmm2, [i_xmmOnes]
			movdqa [i_xmmTempNearestX], xmm2
			mov eax, dword [i_xmmTempNearestX + 0*4]
			lea rax, [rdx + rax]
			movzx rax, byte [rax]
			mov [i_xmmNeighb + 0*1], ax
			mov eax, dword [i_xmmTempNearestX + 1*4]
			lea rax, [rdx + rax]
			movzx rax, byte [rax]
			mov [i_xmmNeighb + 1*1], ax
			mov eax, dword [i_xmmTempNearestX + 2*4]
			lea rax, [rdx + rax]
			movzx rax, byte [rax]
			mov [i_xmmNeighb + 2*1], ax
			mov eax, dword [i_xmmTempNearestX + 3*4]
			lea rax, [rdx + rax]
			movzx rax, byte [rax]
			mov [i_xmmNeighb + 3*1], ax
			movdqa xmm4, [i_xmmNeighb]
			punpcklbw xmm4, [i_xmmZeros]
			punpcklwd xmm4, [i_xmmZeros]

			; x0 = x & 0xff
			movdqa xmm2, xmm1
			pand xmm2, [i_xmmFF]
			pmulld xmm6, xmm2
			pmulld xmm4, xmm2
			; x1 = 255 - x0
			movdqa xmm3, [i_xmmFF]
			psubd xmm3, xmm2
			pmulld xmm7, xmm3
			pmulld xmm5, xmm3
			; y0 = y & 0xff
			movdqa xmm2, xmm0
			pand xmm2, [i_xmmFF]
			; y1 = 255 - y0
			movdqa xmm3, [i_xmmFF]
			psubd xmm3, xmm2

			paddd xmm7, xmm6
			pmulld xmm7, xmm3
			paddd xmm5, xmm4
			pmulld xmm5, xmm2
			paddd xmm7, xmm5
			psrld xmm7, 16
			packssdw xmm7, xmm7
			packuswb xmm7, xmm7
			mov rax, arg(1) ; FIXME
			movd dword [rax], xmm7

			; x += sf_x;
			paddd xmm1, [i_xmmSFX]

			; outPtr += 4
			add rax, 4
			mov arg(1), rax
			
			add rdi, 4
			cmp rdi, arg(6)
			jl .LoopColumns

		; y += sf_y
		add rbx, arg(9)

		; outPtr += outPad
		mov rax, arg(1)
		add rax, rcx
		mov arg(1), rax

		dec rsi
		test rsi, rsi
		jnz .LoopRows


	; unalign stack and free memory
	add rsp, 16*1 + 16*1 + 16*1 + 16*1 + 16*1 + 16*1 + 16*1 + 4*4 + 4*4 + 1*16
	COMPV_YASM_UNALIGN_STACK

	%undef i_xmmSFX
	%undef i_xmmOnes
	%undef i_xmmZeros
	%undef i_xmmInStride
	%undef i_xmmFF
	%undef i_xmmXStart
	%undef i_xmmTempNearestX
	%undef i_xmmNearestX
	%undef i_xmmTempNearestX
	%undef i_xmmNeighb

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
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

global sym(Brief256_31_Asm_X86_SSE41)

section .data
	;extern sym(kBrief256Pattern31AX)
	;extern sym(kBrief256Pattern31AY)
	;extern sym(kBrief256Pattern31BX)
	;extern sym(kBrief256Pattern31BY)
	extern sym(k128_u8)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; agr(0) -> const uint8_t* img_center
; agr(1) -> compv_scalar_t img_stride
; agr(2) -> const float* cos1
; agr(3) -> const float* sin1
; agr(4) -> COMPV_ALIGNED(SSE) void* out
; void Brief256_31_Asm_X86_SSE41(const uint8_t* img_center, compv_scalar_t img_stride, float cosT, float sinT, COMPV_ALIGNED(SSE) void* out)
sym(Brief256_31_Asm_X86_SSE41):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	COMPV_YASM_SAVE_XMM 7 ;XMM[6-7]
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	%define COMPV_SIZE_OF_FLOAT 4 ; up to the caller to make sure sizeof(float)=4
	%define i_xmmIndex	0
	%define i_xmmA		16
	%define i_xmmB		32

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 16, rax
	sub rsp, 4*4 + 16*1 + 16*1
	; [rsp + 0] = int32_t xmmIndex[4]
	; [rsp + 16] = uint8_t xmmA[16]
	; [rsp + 32] = uint8_t xmmB[16]

	; rdi = max
	mov rdi, 256
	; rsi = i
	xor rsi, rsi
	; rcx = u8_index
	xor rcx, rcx

	; xmm7 = xmmStride = _mm_set1_epi32((int)img_stride)
	mov rax, arg(1)
	movd xmm7, rax
	pshufd xmm7, xmm7, 0x0

	; xmm6 = xmmCosT = _mm_set1_ps(*cos1)
	mov rax, arg(2)
	movss xmm6, [rax]
	shufps xmm6, xmm6, 0x0

	; xmm5 = xmmSinT = _mm_set1_ps(*sin1)
	mov rax, arg(3)
	movss xmm5, [rax]
	shufps xmm5, xmm5, 0x0

	; xf = xmm0 = (kBrief256Pattern31AX[i] * cosT - kBrief256Pattern31AY[i] * sinT)
	;lea rax, [sym(kBrief256Pattern31AX)]
	;lea rbx, [sym(kBrief256Pattern31AY)]
	movaps xmm3, [rax + rsi*COMPV_SIZE_OF_FLOAT] ; xmm3 = kBrief256Pattern31AX
	movaps xmm4, [rbx + rsi*COMPV_SIZE_OF_FLOAT] ; xmm4 = kBrief256Pattern31AY
	movaps xmm0, xmm3
	movaps xmm1, xmm4
	mulps xmm0, xmm6
	mulps xmm1, xmm5
	subps xmm0, xmm1
	; yf = xmm3 = (kBrief256Pattern31AX[i] * sinT + kBrief256Pattern31AY[i] * cosT);
	mulps xmm3, xmm5
	mulps xmm4, xmm6
	addps xmm3, xmm4
	; x = xmm0 = COMPV_MATH_ROUNDF_2_INT(xf, int);
	cvtps2dq xmm0, xmm0
	; y = xmm3 = COMPV_MATH_ROUNDF_2_INT(yf, int);
	cvtps2dq xmm3, xmm3
	; xmmIndex = ((y * img_stride) + x)
	pmulld xmm3, xmm7
	paddd xmm3, xmm0
	movdqa [i_xmmIndex], xmm3
	mov rax, arg(0) ; rax = img_center
	movsxd rdx, dword [i_xmmIndex + 0*4] ; rdx = xmmIndex[0]
	movzx rbx, byte [rax + rdx] ; rbx = img_center[xmmIndex[0]]
	mov [i_xmmA + rcx + 0], bx ; xmmA[u8_index + 0] = img_center[xmmIndex[0]]
	movsxd rdx, dword [i_xmmIndex + 1*4] ; rdx = xmmIndex[1]
	movzx rbx, byte [rax + rdx] ; rbx = img_center[xmmIndex[1]]
	mov [i_xmmA + rcx + 1], bx ; xmmA[u8_index + 1] = img_center[xmmIndex[1]]
	movsxd rdx, dword [i_xmmIndex + 2*4] ; rdx = xmmIndex[2]
	movzx rbx, byte [rax + rdx] ; rbx = img_center[xmmIndex[2]]
	mov [i_xmmA + rcx + 2], bx ; xmmA[u8_index + 2] = img_center[xmmIndex[2]]
	movsxd rdx, dword [i_xmmIndex + 3*4] ; rdx = xmmIndex[3]
	movzx rbx, byte [rax + rdx] ; rbx = img_center[xmmIndex[3]]
	mov [i_xmmA + rcx + 3], bx ; xmmA[u8_index + 3] = img_center[xmmIndex[3]]
	

	; FIXME
	mov rax, arg(4)
	;movdqa [rax], xmm6
	;movaps [rax], xmm0
	movdqa xmm4, [i_xmmA]
	movdqa [rax], xmm4



	; unalign stack and free memory
	add rsp, 4*4 + 16*1 + 16*1
	COMPV_YASM_UNALIGN_STACK

	%undef COMPV_SIZE_OF_FLOAT
	%undef i_xmmIndex
	%undef i_xmmA
	%undef i_xmmB

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
	
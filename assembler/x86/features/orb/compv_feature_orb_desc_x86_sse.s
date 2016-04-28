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
	extern sym(kBrief256Pattern31AX)
	extern sym(kBrief256Pattern31AY)
	extern sym(kBrief256Pattern31BX)
	extern sym(kBrief256Pattern31BY)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; agr(0) -> const uint8_t* img_center
; agr(1) -> compv_scalar_t img_stride
; agr(2) -> float cosT
; agr(3) -> float sinT
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

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 16, rax
	sub rsp, 4*4 + 16*1 + 16*1
	; [rsp + 0] = int32_t xmmIndex[4]
	; [rsp + 16] = uint8_t xmmA[16]
	; [rsp + 32] = uint8_t xmmB[16];

	; xmm7 = xmmStride = _mm_set1_epi32((int)img_stride)
	mov rax, arg(1)
	movd xmm7, rax
	pshufd xmm7, xmm7, 0x0

	; xmm6 = xmmCosT = _mm_set1_ps(cosT)
	;mov rax, arg(4)
	;mov rcx, arg(2)
	;mov rcx, 1983
	;mov [rax], rcx
	;mov [rax + 4], rcx
	;mov [rax + 8], rcx
	movss xmm6, arg(2)
	shufps xmm6, xmm6, 0x0

	; xmm5 = xmmSinT = _mm_set1_ps(sinT)
	movss xmm5, arg(3)
	shufps xmm5, xmm5, 0x0

	; FIXME
	mov rax, arg(4)
	;movdqa [rax], xmm6
	movaps [rax], xmm6


	; unalign stack and free memory
	add rsp, 4*4 + 16*1 + 16*1
	COMPV_YASM_UNALIGN_STACK

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
	
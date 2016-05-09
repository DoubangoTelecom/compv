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

global sym(Moments0110_Asm_X86_SSE41)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* top
; arg(1) -> COMPV_ALIGNED(SSE)const uint8_t* bottom
; arg(2) -> COMPV_ALIGNED(SSE)const int16_t* x
; arg(3) -> COMPV_ALIGNED(SSE) const int16_t* y
; arg(4) -> compv_scalar_t count
; arg(5) -> compv_scalar_t* s01
; arg(6) -> compv_scalar_t* s10
; void Moments0110_Asm_X86_SSE41(COMPV_ALIGNED(SSE) const uint8_t* top, COMPV_ALIGNED(SSE)const uint8_t* bottom, COMPV_ALIGNED(SSE)const int16_t* x, COMPV_ALIGNED(SSE) const int16_t* y, compv_scalar_t count, compv_scalar_t* s01, compv_scalar_t* s10)
sym(Moments0110_Asm_X86_SSE41):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_XMM 7 ;XMM[6-7]
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	%define COMPV_SIZE_OF_INT16 2

	; xmm7 = xmmZero
	pxor xmm7, xmm7

	; rsi = i
	xor rsi, rsi

	mov rax, arg(5)
	mov rbx, arg(6)

	; rcx = [s01]
	mov rcx, [rax]

	; rdx = [s10]
	mov rdx, [rbx]

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_scalar_t i = 0; i < count; i += 16)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopRows
		mov rdi, arg(0) ; top
		mov rbx, arg(1) ; bottom
		movdqa xmm0, [rdi + rsi] ; xmm0 = xmmTop
		movdqa xmm1, [rbx + rsi] ; xmm1 = xmmBottom
		movdqa xmm2, xmm0 ; xmm2 = xmmTop
		movdqa xmm3, xmm1 ; xmm3 = xmmBottom

		mov rdi, arg(2) ; x
		mov rbx, arg(3) ; y

		punpcklbw xmm0, xmm7
		punpcklbw xmm1, xmm7
		movdqa xmm4, xmm0
		movdqa xmm5, xmm1

		paddw xmm0, xmm1
		pxor xmm1, xmm1
		pmullw xmm0, [rdi + rsi*COMPV_SIZE_OF_INT16]
		pxor xmm6, xmm6
		punpcklwd xmm1, xmm0
		punpckhwd xmm6, xmm0
		psrad xmm1, 16
		psrad xmm6, 16
		psubw xmm4, xmm5
		phaddd xmm1, xmm6
		pmullw xmm4, [rbx + rsi*COMPV_SIZE_OF_INT16]
		pxor xmm0, xmm0
		pxor xmm5, xmm5
		punpcklwd xmm0, xmm4
		punpckhwd xmm5, xmm4
		psrad xmm0, 16
		psrad xmm5, 16
		punpckhbw xmm2, xmm7
		punpckhbw xmm3, xmm7
		phaddd xmm0, xmm5
		movdqa xmm4, xmm2
		movdqa xmm5, xmm3		
		paddw xmm2, xmm3
		phaddd xmm1, xmm0
		pmullw xmm2, [rdi + rsi*COMPV_SIZE_OF_INT16 + 8*COMPV_SIZE_OF_INT16]
		pxor xmm0, xmm0
		pxor xmm6, xmm6
		punpcklwd xmm0, xmm2
		punpckhwd xmm6, xmm2
		psubw xmm4, xmm5
		psrad xmm0, 16
		psrad xmm6, 16
		pmullw xmm4, [rbx + rsi*COMPV_SIZE_OF_INT16 + 8*COMPV_SIZE_OF_INT16]
		pxor xmm2, xmm2
		pxor xmm5, xmm5
		punpcklwd xmm2, xmm4
		punpckhwd xmm5, xmm4
		psrad xmm2, 16
		psrad xmm5, 16
		phaddd xmm0, xmm6
		phaddd xmm2, xmm5
		phaddd xmm0, xmm2
		phaddd xmm1, xmm0

		pextrd dword edi, xmm1, 1
		pextrd dword ebx, xmm1, 3
		add dword ecx, edi
		add dword ecx, ebx
		pextrd dword edi, xmm1, 0
		pextrd dword ebx, xmm1, 2
		add dword edx, edi
		add dword edx, ebx

		add rsi, 16
		cmp rsi, arg(4)
		jl .LoopRows

	mov rax, arg(5)
	mov rbx, arg(6)
	mov [rax], rcx
	mov [rbx], rdx

	%undef COMPV_SIZE_OF_INT16

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
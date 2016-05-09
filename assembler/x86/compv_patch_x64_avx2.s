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

global sym(Moments0110_Asm_X64_AVX2)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(AVX2) const uint8_t* top
; arg(1) -> COMPV_ALIGNED(AVX2)const uint8_t* bottom
; arg(2) -> COMPV_ALIGNED(AVX2)const int16_t* x
; arg(3) -> COMPV_ALIGNED(AVX2) const int16_t* y
; arg(4) -> compv_scalar_t count
; arg(5) -> compv_scalar_t* s01
; arg(6) -> compv_scalar_t* s10
; void Moments0110_Asm_X64_AVX2(COMPV_ALIGNED(AVX2) const uint8_t* top, COMPV_ALIGNED(AVX2)const uint8_t* bottom, COMPV_ALIGNED(AVX2)const int16_t* x, COMPV_ALIGNED(AVX2) const int16_t* y, compv_scalar_t count, compv_scalar_t* s01, compv_scalar_t* s10)
sym(Moments0110_Asm_X64_AVX2):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_YMM 7 ;YMM[6-7]
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	;; end prolog ;;

	%define COMPV_SIZE_OF_INT16 2

	; ymm7 = ymmZero
	vpxor ymm7, ymm7

	; rsi = i
	xor rsi, rsi

	; rdi = top
	mov rdi, arg(0)
	
	; rbx = bottom
	mov rbx, arg(1)

	; rcx = x
	mov rcx, arg(2)

	; rdx = y
	mov rdx, arg(3)

	mov r8, arg(5) ; s01
	mov r9, arg(6) ; s10

	; r11 = *s01
	mov r11, [r8]

	; r12 = *s10
	mov r12, [r9]

	; r13 = count
	mov r13, arg(4)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_scalar_t i = 0; i < count; i += 32)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopRows
		vmovdqa ymm0, [rdi + rsi] ; ymm0 = ymmTop
		vmovdqa ymm1, [rbx + rsi] ; ymm1 = ymmBottom
		vpermq ymm0, ymm0, 0xD8
		vpermq ymm1, ymm1, 0xD8
		vmovdqa ymm2, ymm0 ; ymm2 = ymmTop
		vmovdqa ymm3, ymm1 ; ymm3 = ymmBottom

		vpunpcklbw ymm4, ymm0, ymm7
		vpunpcklbw ymm5, ymm1, ymm7
		vpaddw ymm0, ymm4, ymm5
		vpmullw ymm0, [rcx + rsi*COMPV_SIZE_OF_INT16]
		vpunpcklwd ymm1, ymm7, ymm0
		vpunpckhwd ymm6, ymm7, ymm0
		vpsrad ymm1, ymm1, 16
		vpsrad ymm6, ymm6, 16
		vpsubw ymm4, ymm4, ymm5
		vphaddd ymm1, ymm1, ymm6
		vpmullw ymm4, ymm4, [rdx + rsi*COMPV_SIZE_OF_INT16]
		vpunpcklwd ymm0, ymm7, ymm4
		vpunpckhwd ymm5, ymm7, ymm4
		vpsrad ymm0, ymm0, 16
		vpsrad ymm5, ymm5, 16
		vpunpckhbw ymm4, ymm2, ymm7
		vphaddd ymm0, ymm0, ymm5
		vpunpckhbw ymm5, ymm3, ymm7
		vpaddw ymm2, ymm4, ymm5
		vphaddd ymm1, ymm1, ymm0
		vpmullw ymm2, ymm2, [rcx + rsi*COMPV_SIZE_OF_INT16 + 16*COMPV_SIZE_OF_INT16]
		vpunpcklwd ymm0, ymm7, ymm2
		vpunpckhwd ymm6, ymm7, ymm2
		vpsubw ymm4, ymm4, ymm5
		vpsrad ymm0, ymm0, 16
		vpsrad ymm6, ymm6, 16
		vpmullw ymm4, ymm4, [rdx + rsi*COMPV_SIZE_OF_INT16 + 16*COMPV_SIZE_OF_INT16]
		vpunpcklwd ymm2, ymm7, ymm4
		vpunpckhwd ymm5, ymm7, ymm4
		vpsrad ymm2, ymm2, 16
		vpsrad ymm5, ymm5, 16
		vphaddd ymm0, ymm0, ymm6
		vphaddd ymm2, ymm2, ymm5
		vphaddd ymm0, ymm0, ymm2
		vphaddd ymm1, ymm1, ymm0

		vextracti128 xmm0, ymm1, 0
		vextracti128 xmm1, ymm1, 1

		vpextrd dword r8d, xmm0, 1
		vpextrd dword r9d, xmm0, 3
		vpextrd dword r10d, xmm1, 1
		vpextrd dword eax, xmm1, 3
		add dword r11d, r8d
		add dword r11d, r9d
		add dword r11d, r10d
		add dword r11d, eax

		vpextrd dword r8d, xmm0, 0
		vpextrd dword r9d, xmm0, 2
		vpextrd dword r10d, xmm1, 0
		vpextrd dword eax, xmm1, 2
		add dword r12d, r8d
		add dword r12d, r9d
		add dword r12d, r10d
		add dword r12d, eax

		add rsi, 32
		cmp rsi, r13
		jl .LoopRows

	mov r8, arg(5) ; s01
	mov r9, arg(6) ; s10

	mov [r8], r11
	mov [r9], r12

	%undef COMPV_SIZE_OF_INT16

	;; begin epilog ;;
	pop r13
	pop r12
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_YMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret
%endif ; COMPV_YASM_ABI_IS_64BIT
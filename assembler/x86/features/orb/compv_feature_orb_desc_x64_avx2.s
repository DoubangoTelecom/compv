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

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(Brief256_31_Asm_X64_AVX2)
global sym(Brief256_31_Asm_X64_FMA3_AVX2)

section .data
	extern sym(kBrief256Pattern31AX)
	extern sym(kBrief256Pattern31AY)
	extern sym(kBrief256Pattern31BX)
	extern sym(kBrief256Pattern31BY)
	extern sym(k128_u8)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; agr(0) -> const uint8_t* img_center
; agr(1) -> compv_scalar_t img_stride
; agr(2) -> const float* cos1
; agr(3) -> const float* sin1
; agr(4) -> COMPV_ALIGNED(SSE) void* out
; void Brief256_31_Asm_X64_AVX2(const uint8_t* img_center, compv_scalar_t img_stride, float cosT, float sinT, COMPV_ALIGNED(SSE) void* out)
; %1 -> 1: FMA enabled, 0: FMA disabled
%macro Brief256_31_Macro_X64_AVX2 1
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	COMPV_YASM_SAVE_YMM 7 ;XMM[6-7]
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	push r14
	push r15
	;; end prolog ;;

	%define COMPV_SIZE_OF_FLOAT 4 ; up to the caller to make sure sizeof(float)=4
	%define COMPV_SIZE_OF_UIN32	4
	%define i_ymmIndex	rsp + 0
	%define i_ymmA		rsp + 32
	%define i_ymmB		rsp + 64

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 32, rax
	sub rsp, 4*8 + 32*1 + 32*1
	; [rsp + 0] = int32_t ymmIndex[8]
	; [rsp + 32] = uint8_t ymmA[32]
	; [rsp + 64] = uint8_t ymmB[32]
	
	; rsi = i
	xor rsi, rsi
	; rcx = u8_index
	xor rcx, rcx
	; rdi = outPtr
	mov rdi, arg(4)
	; r8 = &kBrief256Pattern31AX
	lea r8, [sym(kBrief256Pattern31AX)]
	; r9 = &kBrief256Pattern31AY
	lea r9, [sym(kBrief256Pattern31AY)]
	; r10 = &kBrief256Pattern31BX
	lea r10, [sym(kBrief256Pattern31BX)]
	; r11 = &kBrief256Pattern31BY
	lea r11, [sym(kBrief256Pattern31BY)]
	; r12 = img_center
	mov r12, arg(0)

	vzeroupper

	
	; ymm7 = ymmStride = _mm256_set1_epi32((int)img_stride)
	mov rax, arg(1)
	vmovd xmm7, eax
	vpbroadcastd ymm7, xmm7
	%if %1 == 1 ; FMA3
		vcvtdq2ps ymm7, ymm7 ; ymmStride = floats
	%endif

	; ymm6 = xmmCosT = _mm256_set1_ps(*cos1)
	mov rax, arg(2)
	vmovss xmm6, [rax]
	vbroadcastss ymm6, xmm6

	; ymm5 = ymmSinT = _mm256_set1_ps(*sin1)
	mov rax, arg(3)
	vmovss xmm5, [rax]
	vbroadcastss ymm5, xmm5

	; ymm4 = ymm128
	vmovdqa ymm4, [sym(k128_u8)]

	;;;;;;;;;
	;	Loop
	;;;;;;;;
	.LoopStart
		;; ymmA ;;
		%if %1 == 1 ; FMA3
			vmulps ymm0, ymm5, [r9]
			vmulps ymm2, ymm5, [r8]
			vfmsub231ps ymm0, ymm6, [r8]
			vfmadd231ps ymm2, ymm6, [r9]
			vroundps ymm0, ymm0, 0x8
			vroundps ymm2, ymm2, 0x8
			vfmadd231ps ymm0, ymm2, ymm7
			vcvtps2dq ymm2, ymm0
		%else
			vmulps ymm0, ymm6, [r8]
			vmulps ymm1, ymm5, [r9]
			vmulps ymm2, ymm5, [r8]
			vmulps ymm3, ymm6, [r9]
			vsubps ymm0, ymm1
			vaddps ymm2, ymm3
			vcvtps2dq ymm0, ymm0
			vcvtps2dq ymm2, ymm2
			vpmulld ymm2, ymm7
			vpaddd ymm2, ymm0
		%endif
		vmovdqa [i_ymmIndex], ymm2
		movsxd rax, dword [i_ymmIndex + 0*4] ; rax = ymmIndex[0]
		movsxd rdx, dword [i_ymmIndex + 1*4] ; rdx = ymmIndex[1]
		movsxd r13, dword [i_ymmIndex + 2*4] ; r13 = ymmIndex[2]
		movsxd r14, dword [i_ymmIndex + 3*4] ; r14 = ymmIndex[3]
		movzx rbx, byte [r12 + rax] ; rbx = img_center[ymmIndex[0]]
		movzx r15, byte [r12 + rdx] ; r15 = img_center[ymmIndex[1]]
		movzx rax, byte [r12 + r13] ; rax = img_center[ymmIndex[2]]
		movzx rdx, byte [r12 + r14] ; rdx = img_center[ymmIndex[3]]
		mov [i_ymmA + rcx + 0], byte bl ; ymmA[u8_index + 0] = img_center[ymmIndex[0]]
		mov [i_ymmA + rcx + 1], byte r15b ; ymmA[u8_index + 1] = img_center[ymmIndex[1]]
		mov [i_ymmA + rcx + 2], byte al ; ymmA[u8_index + 2] = img_center[ymmIndex[2]]
		mov [i_ymmA + rcx + 3], byte dl ; ymmA[u8_index + 3] = img_center[ymmIndex[3]]
		movsxd rax, dword [i_ymmIndex + 4*4] ; rax = ymmIndex[4]
		movsxd rdx, dword [i_ymmIndex + 5*4] ; rdx = ymmIndex[5]
		movsxd r13, dword [i_ymmIndex + 6*4] ; r13 = ymmIndex[6]
		movsxd r14, dword [i_ymmIndex + 7*4] ; r14 = ymmIndex[7]
		movzx rbx, byte [r12 + rax] ; rbx = img_center[ymmIndex[4]]
		movzx r15, byte [r12 + rdx] ; r15 = img_center[ymmIndex[5]]
		movzx rax, byte [r12 + r13] ; rax = img_center[ymmIndex[6]]
		movzx rdx, byte [r12 + r14] ; rdx = img_center[ymmIndex[7]]
		mov [i_ymmA + rcx + 4], byte bl ; ymmA[u8_index + 4] = img_center[ymmIndex[4]]
		mov [i_ymmA + rcx + 5], byte r15b ; ymmA[u8_index + 5] = img_center[ymmIndex[5]]
		mov [i_ymmA + rcx + 6], byte al ; ymmA[u8_index + 6] = img_center[ymmIndex[6]]
		mov [i_ymmA + rcx + 7], byte dl ; ymmA[u8_index + 7] = img_center[ymmIndex[7]]

		;; ymmB ;;
		%if %1 == 1 ; FMA3
			vmulps ymm0, ymm5, [r11]
			vmulps ymm2, ymm5, [r10]
			vfmsub231ps ymm0, ymm6, [r10]
			vfmadd231ps ymm2, ymm6, [r11]
			vroundps ymm0, ymm0, 0x8
			vroundps ymm2, ymm2, 0x8
			vfmadd231ps ymm0, ymm2, ymm7
			vcvtps2dq ymm2, ymm0
		%else
			vmulps ymm0, ymm6, [r10]
			vmulps ymm1, ymm5, [r11]
			vmulps ymm2, ymm5, [r10]
			vmulps ymm3, ymm6, [r11]
			vsubps ymm0, ymm1
			vaddps ymm2, ymm3
			vcvtps2dq ymm0, ymm0
			vcvtps2dq ymm2, ymm2
			vpmulld ymm2, ymm7
			vpaddd ymm2, ymm0
		%endif
		vmovdqa [i_ymmIndex], ymm2
		movsxd rax, dword [i_ymmIndex + 0*4] ; rax = ymmIndex[0]
		movsxd rdx, dword [i_ymmIndex + 1*4] ; rdx = ymmIndex[1]
		movsxd r13, dword [i_ymmIndex + 2*4] ; r13 = ymmIndex[2]
		movsxd r14, dword [i_ymmIndex + 3*4] ; r14 = ymmIndex[3]
		movzx rbx, byte [r12 + rax] ; rbx = img_center[ymmIndex[0]]
		movzx r15, byte [r12 + rdx] ; r15 = img_center[ymmIndex[1]]
		movzx rax, byte [r12 + r13] ; rax = img_center[ymmIndex[2]]
		movzx rdx, byte [r12 + r14] ; rdx = img_center[ymmIndex[3]]
		mov [i_ymmB + rcx + 0], byte bl ; ymmB[u8_index + 0] = img_center[ymmIndex[0]]
		mov [i_ymmB + rcx + 1], byte r15b ; ymmB[u8_index + 1] = img_center[ymmIndex[1]]
		mov [i_ymmB + rcx + 2], byte al ; ymmB[u8_index + 2] = img_center[ymmIndex[2]]
		mov [i_ymmB + rcx + 3], byte dl ; ymmB[u8_index + 3] = img_center[ymmIndex[3]]
		movsxd rax, dword [i_ymmIndex + 4*4] ; rax = ymmIndex[4]
		movsxd rdx, dword [i_ymmIndex + 5*4] ; rdx = ymmIndex[5]
		movsxd r13, dword [i_ymmIndex + 6*4] ; r13 = ymmIndex[6]
		movsxd r14, dword [i_ymmIndex + 7*4] ; r14 = ymmIndex[7]
		movzx rbx, byte [r12 + rax] ; rbx = img_center[ymmIndex[4]]
		movzx r15, byte [r12 + rdx] ; r15 = img_center[ymmIndex[5]]
		movzx rax, byte [r12 + r13] ; rax = img_center[ymmIndex[6]]
		movzx rdx, byte [r12 + r14] ; rdx = img_center[ymmIndex[7]]
		mov [i_ymmB + rcx + 4], byte bl ; ymmB[u8_index + 4] = img_center[ymmIndex[4]]
		mov [i_ymmB + rcx + 5], byte r15b ; ymmB[u8_index + 5] = img_center[ymmIndex[5]]
		mov [i_ymmB + rcx + 6], byte al ; ymmB[u8_index + 6] = img_center[ymmIndex[6]]
		mov [i_ymmB + rcx + 7], byte dl ; ymmB[u8_index + 7] = img_center[ymmIndex[7]]
	
		add rcx, 8 ; u8_index += 8
		cmp rcx, 32
		jne .EndOfComputeDescription
			; _out[i] |= (a < b) ? (u64_1 << j) : 0;
			vmovdqa ymm0, [i_ymmB]
			vmovdqa ymm1, [i_ymmA]
			vpsubb ymm0, ymm4
			vpsubb ymm1, ymm4
			vpcmpgtb ymm0, ymm1
			vpmovmskb ebx, ymm0
			mov [rdi], ebx

			xor rcx, rcx ; u8_index = 0
			add rdi, COMPV_SIZE_OF_UIN32 ; ++outPtr
		.EndOfComputeDescription

		lea r8, [r8 + 8*COMPV_SIZE_OF_FLOAT] ; Brief256Pattern31AX+=8
		lea r9, [r9 + 8*COMPV_SIZE_OF_FLOAT] ; Brief256Pattern31AY+=8
		lea r10, [r10 + 8*COMPV_SIZE_OF_FLOAT] ; Brief256Pattern31BX+=8
		lea r11, [r11 + 8*COMPV_SIZE_OF_FLOAT] ; Brief256Pattern31BY+=8
		lea rsi, [rsi + 8]
		cmp rsi, 256
		jl .LoopStart

	; unalign stack and free memory
	add rsp, 4*8 + 32*1 + 32*1
	COMPV_YASM_UNALIGN_STACK

	%undef COMPV_SIZE_OF_FLOAT
	%undef COMPV_SIZE_OF_UIN32
	%undef i_ymmIndex
	%undef i_ymmA
	%undef i_ymmB

	;; begin epilog ;;
	pop r15
	pop r14
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
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(Brief256_31_Asm_X64_AVX2):
	Brief256_31_Macro_X64_AVX2 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(Brief256_31_Asm_X64_FMA3_AVX2):
	Brief256_31_Macro_X64_AVX2 1
	
%endif ; COMPV_YASM_ABI_IS_64BIT
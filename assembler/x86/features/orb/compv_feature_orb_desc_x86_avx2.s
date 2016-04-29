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

global sym(Brief256_31_Asm_X86_AVX2)

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
; void Brief256_31_Asm_X86_AVX2(const uint8_t* img_center, compv_scalar_t img_stride, float cosT, float sinT, COMPV_ALIGNED(SSE) void* out)
sym(Brief256_31_Asm_X86_AVX2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	COMPV_YASM_SAVE_YMM 7 ;YMM[6-7]
	push rsi
	push rdi
	push rbx
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

	vzeroupper

	; ymm7 = ymmStride = _mm256_set1_epi32((int)img_stride)
	mov rax, arg(1)
	vmovd xmm7, eax
	vpbroadcastd ymm7, xmm7

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
		lea rax, [sym(kBrief256Pattern31AX)]
		lea rbx, [sym(kBrief256Pattern31AY)]
		vmulps ymm0, ymm6, [rax + rsi*COMPV_SIZE_OF_FLOAT]
		vmulps ymm1, ymm5, [rbx + rsi*COMPV_SIZE_OF_FLOAT]
		vmulps ymm2, ymm5, [rax + rsi*COMPV_SIZE_OF_FLOAT]
		vmulps ymm3, ymm6, [rbx + rsi*COMPV_SIZE_OF_FLOAT]
		vsubps ymm0, ymm1
		vaddps ymm2, ymm3
		vcvtps2dq ymm0, ymm0
		vcvtps2dq ymm2, ymm2
		vpmulld ymm2, ymm7
		vpaddd ymm2, ymm0
		vmovdqa [i_ymmIndex], ymm2
		mov rax, arg(0) ; rax = img_center
		movsxd rdx, dword [i_ymmIndex + 0*4] ; rdx = ymmIndex[0]
		movzx rbx, byte [rax + rdx] ; rbx = img_center[ymmIndex[0]]
		mov [i_ymmA + rcx + 0], byte bl ; ymmA[u8_index + 0] = img_center[ymmIndex[0]]
		movsxd rdx, dword [i_ymmIndex + 1*4] ; rdx = ymmIndex[1]
		movzx rbx, byte [rax + rdx] ; rbx = img_center[ymmIndex[1]]
		mov [i_ymmA + rcx + 1], byte bl ; ymmA[u8_index + 1] = img_center[ymmIndex[1]]
		movsxd rdx, dword [i_ymmIndex + 2*4] ; rdx = ymmIndex[2]
		movzx rbx, byte [rax + rdx] ; rbx = img_center[ymmIndex[2]]
		mov [i_ymmA + rcx + 2], byte bl ; ymmA[u8_index + 2] = img_center[ymmIndex[2]]
		movsxd rdx, dword [i_ymmIndex + 3*4] ; rdx = ymmIndex[3]
		movzx rbx, byte [rax + rdx] ; rbx = img_center[ymmIndex[3]]
		mov [i_ymmA + rcx + 3], byte bl ; ymmA[u8_index + 3] = img_center[ymmIndex[3]]
		movsxd rdx, dword [i_ymmIndex + 4*4] ; rdx = ymmIndex[4]
		movzx rbx, byte [rax + rdx] ; rbx = img_center[ymmIndex[4]]
		mov [i_ymmA + rcx + 4], byte bl ; ymmA[u8_index + 4] = img_center[ymmIndex[4]]
		movsxd rdx, dword [i_ymmIndex + 5*4] ; rdx = ymmIndex[5]
		movzx rbx, byte [rax + rdx] ; rbx = img_center[ymmIndex[5]]
		mov [i_ymmA + rcx + 5], byte bl ; ymmA[u8_index + 5] = img_center[ymmIndex[5]]
		movsxd rdx, dword [i_ymmIndex + 6*4] ; rdx = ymmIndex[6]
		movzx rbx, byte [rax + rdx] ; rbx = img_center[ymmIndex[6]]
		mov [i_ymmA + rcx + 6], byte bl ; ymmA[u8_index + 6] = img_center[ymmIndex[6]]
		movsxd rdx, dword [i_ymmIndex + 7*4] ; rdx = ymmIndex[7]
		movzx rbx, byte [rax + rdx] ; rbx = img_center[ymmIndex[7]]
		mov [i_ymmA + rcx + 7], byte bl ; ymmA[u8_index + 7] = img_center[ymmIndex[7]]

		;; ymmB ;;
		lea rax, [sym(kBrief256Pattern31BX)]
		lea rbx, [sym(kBrief256Pattern31BY)]
		vmulps ymm0, ymm6, [rax + rsi*COMPV_SIZE_OF_FLOAT]
		vmulps ymm1, ymm5, [rbx + rsi*COMPV_SIZE_OF_FLOAT]
		vmulps ymm2, ymm5, [rax + rsi*COMPV_SIZE_OF_FLOAT]
		vmulps ymm3, ymm6, [rbx + rsi*COMPV_SIZE_OF_FLOAT]
		vsubps ymm0, ymm1
		vaddps ymm2, ymm3
		vcvtps2dq ymm0, ymm0
		vcvtps2dq ymm2, ymm2
		vpmulld ymm2, ymm7
		vpaddd ymm2, ymm0
		vmovdqa [i_ymmIndex], ymm2
		mov rax, arg(0) ; rax = img_center
		movsxd rdx, dword [i_ymmIndex + 0*4] ; rdx = ymmIndex[0]
		movzx rbx, byte [rax + rdx] ; rbx = img_center[ymmIndex[0]]
		mov [i_ymmB + rcx + 0], byte bl ; ymmB[u8_index + 0] = img_center[ymmIndex[0]]
		movsxd rdx, dword [i_ymmIndex + 1*4] ; rdx = ymmIndex[1]
		movzx rbx, byte [rax + rdx] ; rbx = img_center[ymmIndex[1]]
		mov [i_ymmB + rcx + 1], byte bl ; ymmB[u8_index + 1] = img_center[ymmIndex[1]]
		movsxd rdx, dword [i_ymmIndex + 2*4] ; rdx = ymmIndex[2]
		movzx rbx, byte [rax + rdx] ; rbx = img_center[ymmIndex[2]]
		mov [i_ymmB + rcx + 2], byte bl ; ymmB[u8_index + 2] = img_center[ymmIndex[2]]
		movsxd rdx, dword [i_ymmIndex + 3*4] ; rdx = ymmIndex[3]
		movzx rbx, byte [rax + rdx] ; rbx = img_center[ymmIndex[3]]
		mov [i_ymmB + rcx + 3], byte bl ; ymmB[u8_index + 3] = img_center[ymmIndex[3]]
		movsxd rdx, dword [i_ymmIndex + 4*4] ; rdx = ymmIndex[4]
		movzx rbx, byte [rax + rdx] ; rbx = img_center[ymmIndex[4]]
		mov [i_ymmB + rcx + 4], byte bl ; ymmB[u8_index + 4] = img_center[ymmIndex[4]]
		movsxd rdx, dword [i_ymmIndex + 5*4] ; rdx = ymmIndex[5]
		movzx rbx, byte [rax + rdx] ; rbx = img_center[ymmIndex[5]]
		mov [i_ymmB + rcx + 5], byte bl ; ymmB[u8_index + 5] = img_center[ymmIndex[5]]
		movsxd rdx, dword [i_ymmIndex + 6*4] ; rdx = ymmIndex[6]
		movzx rbx, byte [rax + rdx] ; rbx = img_center[ymmIndex[6]]
		mov [i_ymmB + rcx + 6], byte bl ; ymmB[u8_index + 6] = img_center[ymmIndex[6]]
		movsxd rdx, dword [i_ymmIndex + 7*4] ; rdx = ymmIndex[7]
		movzx rbx, byte [rax + rdx] ; rbx = img_center[ymmIndex[7]]
		mov [i_ymmB + rcx + 7], byte bl ; ymmB[u8_index + 7] = img_center[ymmIndex[7]]
	
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
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_YMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret
	
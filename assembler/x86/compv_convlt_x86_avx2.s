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

global sym(Convlt1_hz_float32_minpack16_Asm_X86_AVX2)

section .data
	extern sym(kAVXMaskstore_0_1_u64)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; This function requires sizeof(float) = 4byte = 32bits
; arg(0) -> const uint8_t* in_ptr
; arg(1) -> uint8_t* out_ptr
; arg(2) -> compv_scalar_t width
; arg(3) -> compv_scalar_t height
; arg(4) -> compv_scalar_t pad
; arg(5) -> const float* hkern_ptr
; arg(6) -> compv_scalar_t kern_size
; void Convlt1_hz_float32_minpack16_Asm_X86_AVX2(const uint8_t* in_ptr, uint8_t* out_ptr, compv_scalar_t width, compv_scalar_t height, compv_scalar_t pad, const float* hkern_ptr, compv_scalar_t kern_size)
sym(Convlt1_hz_float32_minpack16_Asm_X86_AVX2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_YMM 7 ;XMM[6-7]
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	vzeroupper

	%define COMPV_SIZE_OF_FLOAT 4 ; up to the caller to make sure sizeof(float)=4
	%define i_ymmSF3	rsp + 0

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 32, rax
	sub rsp, 32*1
	; [rsp + 0] = ymmSF3

	; i = rdi
	; xor rdi, rdi

	; rcx = col

	; rbx = out_ptr
	mov rbx, arg(1)

	; j = rsi = height
	mov rsi, arg(3)

	; ymm7 = ymmZero
	vpxor ymm7, ymm7

	; arg(4) = pad += (width & 15)
	mov rdx, arg(2) ; width
	mov rax, arg(4) ; pad
	and rdx, 15
	add rax, rdx
	mov arg(4), rax

	; rax = in_ptr
	mov rax, arg(0)

	; rdx = hkern_ptr
	mov rdx, arg(5)
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopRows
		mov rdi, arg(2) ; i = width
		; jmp .EndOfLoopColumns32 ; Uncomment this line to skip Loop-32
		cmp rdi, 32
		jl .EndOfLoopColumns32
		
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; while (i > 31)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopColumns32
			vxorps ymm5, ymm5 ; ymm5 = ymmSF0
			vxorps ymm6, ymm6 ; ymm6 = ymmSF1
			vxorps ymm4, ymm4 ; ymm4 = ymmSF2
			vmovaps [i_ymmSF3], ymm7

			xor rcx, rcx ; col = 0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (col = 0; col < kern_size; ++col)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			.LoopColumns32Kern32
				vmovss xmm1, [rdx + rcx*COMPV_SIZE_OF_FLOAT]
				vmovdqu ymm0, [rax + rcx] ; ymm0 = ymmI0
				vbroadcastss ymm1, xmm1 ; ymm1 = ymmCoeff
				
				vpunpcklbw ymm2, ymm0, ymm7
				vpunpcklbw ymm3, ymm0, ymm7
				vpunpcklwd ymm2, ymm7
				vpunpckhwd ymm3, ymm7
				vcvtdq2ps ymm2, ymm2
				vcvtdq2ps ymm3, ymm3
				vmulps ymm2, ymm1
				vmulps ymm3, ymm1
				vaddps ymm5, ymm2
				vaddps ymm6, ymm3
				
				vpunpckhbw ymm3, ymm0, ymm7
				vpunpckhbw ymm0, ymm0, ymm7
				vpunpcklwd ymm3, ymm7
				vpunpckhwd ymm0, ymm7
				vcvtdq2ps ymm3, ymm3
				vcvtdq2ps ymm0, ymm0
				vmulps ymm3, ymm1
				vmulps ymm0, ymm1
				vaddps ymm4, ymm3
				vaddps ymm0, [i_ymmSF3]
				vmovaps [i_ymmSF3], ymm0

				inc rcx
				cmp rcx, arg(6)
				jl .LoopColumns32Kern32		

			vcvtps2dq ymm5, ymm5
			vcvtps2dq ymm6, ymm6
			vcvtps2dq ymm4, ymm4
			vcvtps2dq ymm3, [i_ymmSF3]
			vpackssdw ymm5, ymm6
			vpackssdw ymm4, ymm3
			vpackuswb ymm5, ymm4
			lea rax, [rax + 32] ; in_ptr += 32
			vmovdqu [rbx], ymm5
			lea rbx, [rbx + 32] ; out_ptr += 32

			sub rdi, 32 ; i -= 32
			cmp rdi, 32
			jge .LoopColumns32
			.EndOfLoopColumns32

		cmp rdi, 16
		jl .EndOfLoopColumns16
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; while (i > 15)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopColumns16
			vxorps ymm4, ymm4 ; ymm4 = ymmSF0
			vxorps ymm5, ymm5 ; ymm5 = ymmSF1
			vmovdqa ymm6, [sym(kAVXMaskstore_0_1_u64)] ; ymm6 = ymmMaskToExtractFirst128Bits

			xor rcx, rcx ; col = 0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (col = 0; col < kern_size; ++col)
			; When width is mof 32 this code isn't executed, make sure to disable previous "while" if you change something
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			.LoopColumns16Kern16
				vpmaskmovq ymm0, ymm6, [rax + rcx] ; ymm0 = ymmI0
				vmovss xmm1, [rdx + rcx*COMPV_SIZE_OF_FLOAT]
				vpermq ymm0, ymm0, 0xD8
				vbroadcastss ymm1, xmm1 ; ymm1 = ymmCoeff				
				vpunpcklbw ymm0, ymm0, ymm7 ; ymm0 = ymmI1

				vpunpcklwd ymm3, ymm0, ymm7
				vpunpckhwd ymm0, ymm0, ymm7
				vcvtdq2ps ymm3, ymm3
				vcvtdq2ps ymm0, ymm0
				vmulps ymm3, ymm1
				vmulps ymm0, ymm1
				vaddps ymm4, ymm3				
				vaddps ymm5, ymm0

				inc rcx
				cmp rcx, arg(6)
				jl .LoopColumns16Kern16

			
			vcvtps2dq ymm4, ymm4
			vcvtps2dq ymm5, ymm5
			vpackssdw ymm4, ymm5
			vpackuswb ymm4, ymm4
			vpermq ymm4, ymm4, 0xD8
			vpmaskmovq [rbx], ymm6, ymm4

			lea rbx, [rbx + 16] ; out_ptr += 16
			lea rax, [rax + 16] ; in_ptr += 16

			sub rdi, 16 ; i -= 16
			cmp rdi, 16
			jge .LoopColumns16
			.EndOfLoopColumns16
		
		add rbx, arg(4) ; out_ptr += pad
		add rax, arg(4) ; in_ptr += pad

		dec rsi ; --j
		test rsi, rsi
		jnz .LoopRows

	; unalign stack and free memory
	add rsp, 32*1
	COMPV_YASM_UNALIGN_STACK

	%undef COMPV_SIZE_OF_FLOAT
	%undef i_ymmSF3

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
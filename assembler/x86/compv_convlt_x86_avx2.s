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

global sym(Convlt1_verthz_float32_minpack16_Asm_X86_AVX2)
global sym(Convlt1_verthz_float32_minpack16_Asm_X86_FMA3_AVX2)
global sym(Convlt1_verthz_fxpq16_minpack16_Asm_X86_AVX2)

section .data
	extern sym(kAVXMaskstore_0_1_u64)

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
; %1 -> 1: FMA3 enabled, 0: FMA3 disabled
; void Convlt1_verthz_float32_minpack16_Asm_X86_AVX2(const uint8_t* in_ptr, uint8_t* out_ptr, compv_scalar_t width, compv_scalar_t height, compv_scalar_t pad, const float* hkern_ptr, compv_scalar_t kern_size)
%macro Convlt1_verthz_float32_minpack16_Macro_X86_AVX2 1
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_YMM 7 ;YMM[6-7]
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	vzeroupper

	%define COMPV_SIZE_OF_FLOAT 4 ; up to the caller to make sure sizeof(float)=4
	%define i_tmp		rsp + 0
	%define i_ymmZero	rsp + 8
	%define i_ymmCoeff	rsp + 40

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 32, rax
	sub rsp, 32*1 + 32*1 + 8*1
	; [rsp + 0] = compv_scalar_t tmp
	; [rsp + 8] = ymmZero
	; [rsp + 40] = ymmCoeff

	; i = rdi
	; xor rdi, rdi

	; rcx = col

	; rbx = out_ptr
	mov rbx, arg(1)

	; j = rsi = height
	mov rsi, arg(3)

	; ymm7 = ymmZero
	vpxor ymm7, ymm7
	vmovdqa [i_ymmZero], ymm7

	; arg(5) = pad += (width & 15)
	mov rdx, arg(2) ; width
	mov rax, arg(5) ; pad
	and rdx, 15
	add rax, rdx
	mov arg(5), rax

	; rax = in_ptr
	mov rax, arg(0)

	; rdx = hkern_ptr
	mov rdx, arg(6)
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopRows
		mov rdi, arg(2) ; i = width
		; jmp .EndOfLoopColumns32 ; uncomment this line to skip Loop-32
		cmp rdi, 32
		jl .EndOfLoopColumns32
		
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; while (i > 31)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopColumns32
			vxorps ymm5, ymm5 ; ymm5 = ymmSF0
			vxorps ymm6, ymm6 ; ymm6 = ymmSF1
			vxorps ymm4, ymm4 ; ymm4 = ymmSF2
			vxorps ymm7, ymm7 ; ymm7 = ymmSF3

			mov [i_tmp], rax ; save rax = in_ptr
			xor rcx, rcx ; col = 0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (col = 0; col < kern_size; ++col)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			.LoopColumns32Kern32
				vmovss xmm1, [rdx + rcx*COMPV_SIZE_OF_FLOAT]
				vmovdqu ymm0, [rax] ; ymm0 = ymmI0
				vbroadcastss ymm1, xmm1 ; ymm1 = ymmCoeff
				vmovdqa [i_ymmCoeff], ymm1
				
				vpunpcklbw ymm2, ymm0, [i_ymmZero]
				vpunpcklbw ymm3, ymm0, [i_ymmZero]
				vpunpckhbw ymm1, ymm0, [i_ymmZero]
				vpunpckhbw ymm0, ymm0, [i_ymmZero]
				vpunpcklwd ymm2, [i_ymmZero]
				vpunpckhwd ymm3, [i_ymmZero]
				vpunpcklwd ymm1, [i_ymmZero]
				vpunpckhwd ymm0, [i_ymmZero]
				vcvtdq2ps ymm2, ymm2
				vcvtdq2ps ymm3, ymm3
				vcvtdq2ps ymm1, ymm1
				vcvtdq2ps ymm0, ymm0
				%if %1 == 1 ; FMA3
					vfmadd231ps ymm5, ymm2, [i_ymmCoeff]
					vfmadd231ps ymm6, ymm3, [i_ymmCoeff]
					vfmadd231ps ymm4, ymm1, [i_ymmCoeff]
					vfmadd231ps ymm7, ymm0, [i_ymmCoeff]
				%else
					vmulps ymm2, [i_ymmCoeff]
					vmulps ymm3, [i_ymmCoeff]
					vmulps ymm1, [i_ymmCoeff]
					vmulps ymm0, [i_ymmCoeff]
					vaddps ymm5, ymm2
					vaddps ymm6, ymm3
					vaddps ymm4, ymm1
					vaddps ymm7, ymm0
				%endif

				inc rcx
				add rax, arg(4) ; += stride
				cmp rcx, arg(7) ; ==? kern_size
				jl .LoopColumns32Kern32		

			mov rax, [i_tmp] ; restore rax
			vcvtps2dq ymm5, ymm5
			vcvtps2dq ymm6, ymm6
			vcvtps2dq ymm4, ymm4
			vcvtps2dq ymm7, ymm7
			vpackssdw ymm5, ymm6
			vpackssdw ymm4, ymm7
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

			mov [i_tmp], rax ; save rax = in_ptr
			xor rcx, rcx ; col = 0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (col = 0; col < kern_size; ++col)
			; When width is mof 32 this code isn't executed, make sure to disable previous "while" if you change something
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			.LoopColumns16Kern16
				vpmaskmovq ymm0, ymm6, [rax] ; ymm0 = ymmI0
				vmovss xmm1, [rdx + rcx*COMPV_SIZE_OF_FLOAT]
				vpermq ymm0, ymm0, 0xD8
				vbroadcastss ymm1, xmm1 ; ymm1 = ymmCoeff				
				vpunpcklbw ymm0, ymm0, [i_ymmZero] ; ymm0 = ymmI1

				vpunpcklwd ymm3, ymm0, [i_ymmZero]
				vpunpckhwd ymm0, ymm0, [i_ymmZero]
				vcvtdq2ps ymm3, ymm3
				vcvtdq2ps ymm0, ymm0
				%if %1 == 1 ; FMA3
					vfmadd231ps ymm4, ymm3, ymm1
					vfmadd231ps ymm5, ymm0, ymm1
				%else
					vmulps ymm3, ymm1
					vmulps ymm0, ymm1
					vaddps ymm4, ymm3				
					vaddps ymm5, ymm0
				%endif

				inc rcx
				add rax, arg(4) ; += stride
				cmp rcx, arg(7) ; ==? kern_size
				jl .LoopColumns16Kern16

			mov rax, [i_tmp] ; restore rax
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
		
		add rbx, arg(5) ; out_ptr += pad
		add rax, arg(5) ; in_ptr += pad

		dec rsi ; --j
		test rsi, rsi
		jnz .LoopRows

	; unalign stack and free memory
	add rsp, 32*1 + 32*1 + 8*1
	COMPV_YASM_UNALIGN_STACK

	%undef COMPV_SIZE_OF_FLOAT
	%undef i_tmp
	%undef i_ymmZero
	%undef i_ymmCoeff

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
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(Convlt1_verthz_float32_minpack16_Asm_X86_AVX2):
	Convlt1_verthz_float32_minpack16_Macro_X86_AVX2 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(Convlt1_verthz_float32_minpack16_Asm_X86_FMA3_AVX2):
	Convlt1_verthz_float32_minpack16_Macro_X86_AVX2 1


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
; void Convlt1_verthz_fxpq16_minpack16_Asm_X86_AVX2(const uint8_t* in_ptr, uint8_t* out_ptr, compv_scalar_t width, compv_scalar_t height, compv_scalar_t pad, const uint16_t* hkern_ptr, compv_scalar_t kern_size)
sym(Convlt1_verthz_fxpq16_minpack16_Asm_X86_AVX2)
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	vzeroupper

	%define COMPV_SIZE_OF_INT16 2
	%define i_tmp		rsp + 0

	; alloc memory
	sub rsp, 8*1
	; [rsp + 0] = compv_scalar_t tmp

	; i = rdi
	; xor rdi, rdi

	; rcx = col

	; rbx = out_ptr
	mov rbx, arg(1)

	; j = rsi = height
	mov rsi, arg(3)

	; ymm6 = ymmZero
	vpxor ymm6, ymm6

	; arg(5) = pad += (width & 15)
	mov rdx, arg(2) ; width
	mov rax, arg(5) ; pad
	and rdx, 15
	add rax, rdx
	mov arg(5), rax

	; rax = in_ptr
	mov rax, arg(0)

	; rdx = hkern_ptr
	mov rdx, arg(6)
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopRows
		mov rdi, arg(2) ; i = width
		; jmp .EndOfLoopColumns32 ; uncomment this line to skip Loop-32
		cmp rdi, 32
		jl .EndOfLoopColumns32
		
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; while (i > 31)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopColumns32
			vpxor ymm4, ymm4 ; ymm4 = ymmS0
			vpxor ymm5, ymm5 ; ymm5 = ymmS1

			mov [i_tmp], rax ; save rax = in_ptr
			xor rcx, rcx ; col = 0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (col = 0; col < kern_size; ++col)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			.LoopColumns32Kern32
				vmovd xmm1, [rdx + rcx*COMPV_SIZE_OF_INT16]
				vmovdqu ymm0, [rax] ; ymm0 = ymmI0		
				vpbroadcastw ymm1, xmm1 ; ymm1 = ymmCoeff
				vpunpckhbw ymm2, ymm0, ymm6
				vpunpcklbw ymm0, ymm0, ymm6
				vpmulhuw ymm2, ymm1
				vpmulhuw ymm0, ymm1
				vpaddw ymm5, ymm2
				vpaddw ymm4, ymm0

				inc rcx
				add rax, arg(4) ; += stride
				cmp rcx, arg(7) ; ==? kern_size
				jl .LoopColumns32Kern32		

			mov rax, [i_tmp] ; restore rax
			
			vpackuswb ymm4, ymm5
			lea rax, [rax + 32] ; in_ptr += 32
			vmovdqu [rbx], ymm4
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
			vpxor ymm4, ymm4 ; ymm4 = ymmS0
			vmovdqa ymm5, [sym(kAVXMaskstore_0_1_u64)] ; ymm5 = ymmMaskToExtractFirst128Bits

			mov [i_tmp], rax ; save rax = in_ptr
			xor rcx, rcx ; col = 0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (col = 0; col < kern_size; ++col)
			; When width is mof 32 this code isn't executed, make sure to disable previous "while" if you change something
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			.LoopColumns16Kern16
				vpmaskmovq ymm0, ymm5, [rax] ; ymm0 = ymmI0
				vmovd xmm1, [rdx + rcx*COMPV_SIZE_OF_INT16]
				vpbroadcastw ymm1, xmm1 ; ymm1 = ymmCoeff

				vpermq ymm0, ymm0, 0xD8
				vpunpcklbw ymm0, ymm0, ymm6 
				vpmulhuw ymm0, ymm0, ymm1
				vpaddw ymm4, ymm4, ymm0

				inc rcx
				add rax, arg(4) ; += stride
				cmp rcx, arg(7) ; ==? kern_size
				jl .LoopColumns16Kern16

			mov rax, [i_tmp] ; restore rax
			vpackuswb ymm4, ymm4, ymm4
			vpermq ymm4, ymm4, 0xD8
			vpmaskmovq [rbx], ymm5, ymm4

			lea rbx, [rbx + 16] ; out_ptr += 16
			lea rax, [rax + 16] ; in_ptr += 16

			sub rdi, 16 ; i -= 16
			cmp rdi, 16
			jge .LoopColumns16
			.EndOfLoopColumns16
		
		add rbx, arg(5) ; out_ptr += pad
		add rax, arg(5) ; in_ptr += pad

		dec rsi ; --j
		test rsi, rsi
		jnz .LoopRows

	; free memory
	add rsp, 8*1

	%undef COMPV_SIZE_OF_INT16
	%undef i_tmp

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret

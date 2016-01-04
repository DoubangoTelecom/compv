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
%include "compv_asm_x86_common.asm"

COMPV_YASM_DEFAULT_REL


global sym(rgbaToI420Kernel11_CompY_Asm_Aligned_AVX2)
global sym(rgbaToI420Kernel11_CompUV_Asm_Aligned_AVX2)

section .data
	extern sym(k16_i16)
	extern sym(k128_i16)
	extern sym(kRGBAToYUV_YCoeffs8)
	extern sym(kRGBAToYUV_U4V4Coeffs8)
	extern sym(k_0_2_4_6_0_2_4_6_i32)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbaToI420Kernel11_CompY_Asm_Aligned_AVX2(COMV_ALIGNED(16) const uint8_t* rgbaPtr, uint8_t* outYPtr, size_t height, size_t width, size_t stride)
sym(rgbaToI420Kernel11_CompY_Asm_Aligned_AVX2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	push rsi
	push rdi
	push rbx
	; end prolog

	mov rcx, arg(4) ; rcx = stride
	mov rax, arg(3) ; rax = width
	sub rcx, rax ; rcx = (stride - width)
	mov rdx, rcx ; rdx = padY = (stride - width);
	shl rcx, 2 ; rcx = padRGBA = (stride - width) << 2
	mov rax, arg(0) ; rgbaPtr
	mov rsi, arg(2) ; height
	mov rbx, arg(1) ; outYPtr

	LoopHeight:
		mov rdi, arg(3) ; width
		LoopWidth:
			vzeroupper
			vmovdqa ymm0, [sym(kRGBAToYUV_YCoeffs8)]
			vmovdqa ymm1, [sym(k16_i16)]
			vmovdqa ymm2, [rax] ; 8 RGBA samples
			vpmaddubsw ymm2, ymm0
			vphaddw ymm2, ymm2 ; aaaabbbbaaaabbbb
			vpermq ymm2, ymm2, 0xD8 ; aaaaaaaabbbbbbbb
			vpsraw ymm2, 7
			vpaddw ymm2, ymm1
			vpackuswb ymm2, ymm2
			vextracti128 xmm2, ymm2, 0
			vzeroupper
			movq [rbx], xmm2
			add rbx, 8
			add rax, 32

			; end-of-LoopWidth
			sub rdi, 8
			cmp rdi, 0
			jg LoopWidth	
	add rbx, rdx
	add rax, rcx
	; end-of-LoopHeight
	sub rsi, 1
	cmp rsi, 0
	jg LoopHeight

	; begin epilog
	pop rbx
	pop rdi
	pop rsi
    COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbaToI420Kernel11_CompUV_Asm_Aligned_AVX2(COMV_ALIGNED(16) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, size_t height, size_t width, size_t stride)
sym(rgbaToI420Kernel11_CompUV_Asm_Aligned_AVX2)
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	push rsi
	push rdi
	push rbx
	sub rsp, 8
	; end prolog

	mov rcx, arg(5) ; rcx = stride
	mov rax, arg(4) ; rax = width
	sub rcx, rax ; rcx = (stride - width)
	mov rdx, rcx ; rdx = (stride - width)
	shr rdx, 1 ; rdx = padUV = (stride - width) >> 1
	shl rcx, 2 ; rcx = (stride - width) << 2
	mov rax, arg(5) ; rax = stride
	shl rax, 2 ; rax = (stride << 2)
	add rcx, rax; rcx = padRGBA = ((stride - width) << 2) + (stride << 2)

	mov rbx, arg(0) ; rgbaPtr
	mov rsi, arg(3) ; height
	mov rax, arg(4)
	mov [rsp], rax ; [rsp+0] = width

	LoopHeight1:
		mov rax, [rsp]
		mov arg(4), rax ; restore arg(4)=width (decremented in th inner loop)
		LoopWidth1:
			vzeroupper
			vmovdqa ymm0, [sym(k_0_2_4_6_0_2_4_6_i32)] ; mask02460246
			vmovdqa ymm1, [sym(k128_i16)] ; 128's
			vmovdqa ymm3, [sym(kRGBAToYUV_U4V4Coeffs8)] ; ymmUV4Coeffs
			vmovdqa ymm2, [rbx] ; 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): axbxcxdx
			;vpermd ymm2, ymm2, ymm0 ; abcdabcd
			vpmaddubsw ymm2, ymm3 ; Ua Ub Uc Ud Va Vb Vc Vd
			vphaddw ymm2, ymm2
			vpermq ymm2, ymm2, 0xD8
			vpsraw ymm2, 8 ; >> 8
			vpaddw ymm2, ymm1 ; + 128 -> UUVV----
			vpackuswb ymm2, ymm2; Saturate(I16 -> U8)
			vzeroupper
			movd rax, xmm2
			mov rdi, arg(1)
			mov [rdi], eax
			add rdi, 4
			mov arg(1), rdi
			psrldq xmm2, 4 ; V0
			movd rax, xmm2
			mov rdi, arg(2)
			mov [rdi], eax
			add rdi, 4
			mov arg(2), rdi
						
			add rbx, 32 ; rgbaPtr += 32

			; end-of-LoopWidth
			mov rax, arg(4) ; width
			sub rax, 8
			mov arg(4), rax
			cmp rax, 0
			jg LoopWidth1
	add rbx, rcx ; rgbaPtr += padRGBA
	mov rdi, arg(1)
	add rdi, rdx 
	mov arg(1), rdi ; outUPtr += padUV
	mov rdi, arg(2)
	add rdi, rdx 
	mov arg(2), rdi ; outVPtr += padUV
	
	; end-of-LoopHeight1
	sub rsi, 2
	cmp rsi, 0
	jg LoopHeight1

	; begin epilog
	add rsp, 8
	pop rbx
	pop rdi
	pop rsi
    COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
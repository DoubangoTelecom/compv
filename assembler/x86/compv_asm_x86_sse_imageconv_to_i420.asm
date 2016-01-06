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

global sym(rgbaToI420Kernel11_CompY_Asm_X86_Aligned_SSSE3)
global sym(rgbaToI420Kernel41_CompY_ASM_Aligned_SSSE3)
;global sym(rgbaToI420Kernel11_CompUV_Asm_X86_Aligned_SSSE3)

section .data
	extern sym(k16_i16)
	extern sym(k128_i16)
	extern sym(kRGBAToYUV_YCoeffs8)
	extern sym(kRGBAToYUV_U2V2Coeffs8)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbaToI420Kernel11_CompY_Asm_X86_Aligned_SSSE3(COMV_ALIGNED(16) const uint8_t* rgbaPtr, uint8_t* outYPtr, size_t height, size_t width, size_t stride)
sym(rgbaToI420Kernel11_CompY_Asm_X86_Aligned_SSSE3):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	push rsi
	push rdi
	push rbx
	; end prolog

	mov rax, arg(3)
	add rax, 3
	and rax, -4
	mov rcx, arg(4)
	sub rcx, rax
	mov rdx, rcx ; rdx = padY
	shl rcx, 2 ; rcx =  padRGBA
	
	mov rax, arg(0) ; rgbaPtr
	mov rsi, arg(2) ; height
	mov rbx, arg(1) ; outYPtr

	movdqa xmm0, [sym(kRGBAToYUV_YCoeffs8)]
	movdqa xmm1, [sym(k16_i16)]

	LoopHeight1:
		xor rdi, rdi
		LoopWidth1:
			movdqa xmm2, [rax] ; 4 RGBA samples
			pmaddubsw xmm2, xmm0
			phaddw xmm2, xmm2
			psraw xmm2, 7
			paddw xmm2, xmm1
			packuswb xmm2, xmm2
			movd [rbx], xmm2

			add rbx, 4
			add rax, 16

			; end-of-LoopWidth1
			add rdi, 4
			cmp rdi, arg(3)
			jl LoopWidth1
	add rbx, rdx
	add rax, rcx
	; end-of-LoopHeight1
	sub rsi, 1
	cmp rsi, 0
	jg LoopHeight1

	; begin epilog
	pop rbx
	pop rdi
	pop rsi
    COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbaToI420Kernel41_CompY_ASM_Aligned_SSSE3(COMV_ALIGNED(16) const uint8_t* rgbaPtr, uint8_t* outYPtr, size_t height, size_t width, size_t stride)
sym(rgbaToI420Kernel41_CompY_ASM_Aligned_SSSE3):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	push rsi
	push rdi
	push rbx
	; end prolog

	mov rdx, arg(3)
	add rdx, 15
	and rdx, -16
	mov rcx, arg(4)
	sub rcx, rdx ; rcx = padY
	mov rdx, rcx 
	shl rdx, 2 ; rdx = padRGBA
		
	mov rax, arg(0) ; rgbaPtr
	mov rsi, arg(2) ; height
	mov rbx, arg(1) ; outYPtr

	movdqa xmm0, [sym(kRGBAToYUV_YCoeffs8)]
	movdqa xmm1, [sym(k16_i16)]

	LoopHeight0:
		xor rdi, rdi
		LoopWidth0:
			movdqa xmm2, [rax] ; 4 RGBA samples
			movdqa xmm3, [rax + 16] ; 4 RGBA samples
			movdqa xmm4, [rax + 32] ; 4 RGBA samples
			movdqa xmm5, [rax + 48] ; 4 RGBA samples

			pmaddubsw xmm2, xmm0
			pmaddubsw xmm3, xmm0
			pmaddubsw xmm4, xmm0
			pmaddubsw xmm5, xmm0

			phaddw xmm2, xmm3
			phaddw xmm4, xmm5
			
			psraw xmm2, 7
			psraw xmm4, 7
			
			paddw xmm2, xmm1
			paddw xmm4, xmm1
						
			packuswb xmm2, xmm4
			movdqu [rbx], xmm2

			add rbx, 16
			add rax, 64

			; end-of-LoopWidth0
			add rdi, 16
			cmp rdi, arg(3)
			jl LoopWidth0
	add rbx, rcx
	add rax, rdx
	; end-of-LoopHeight0
	sub rsi, 1
	cmp rsi, 0
	jg LoopHeight0

	; begin epilog
	pop rbx
	pop rdi
	pop rsi
    COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbaToI420Kernel11_CompUV_Asm_X86_Aligned_SSSE3(COMV_ALIGNED(16) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, size_t height, size_t width, size_t stride)
;;; TODO(not correct, lack of registers, only rax is free, not really optimized, not used)
;sym(rgbaToI420Kernel11_CompUV_Asm_X86_Aligned_SSSE3):
;	push rbp
;	mov rbp, rsp
;	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
;	push rsi
;	push rdi
;	push rbx
;	sub rsp, 8+8+8; (&outUPtr[0], &outVPtr[8], &TempInt64[16])
;	; end prolog
;	
;	mov rcx, arg(5) ; rcx = stride
;	mov rax, arg(4) ; rax = width
;	sub rcx, rax ; rcx = (stride - width)
;	mov rdx, rcx ; rdx = (stride - width)
;	shr rdx, 1 ; rdx = padUV = (stride - width) >> 1
;	shl rcx, 2 ; rcx = (stride - width) << 2
;	mov rax, arg(5) ; rax = stride
;	shl rax, 2 ; rax = (stride << 2)
;	add rcx, rax; rcx = padRGBA = ((stride - width) << 2) + (stride << 2)
;
;	mov rbx, arg(0) ; rgbaPtr
;	mov rsi, arg(3) ; height
;
;	mov rax, arg(1)
;	mov [rsp], rax ; [rsp] = &outUPtr
;	mov rax, arg(2)
;	mov [rsp + 8], rax ; [rsp+8] = &outVPtr
;
;	movdqa xmm0, [sym(kRGBAToYUV_U2V2Coeffs8)] ; UV coeffs interleaved: each appear #2 times
;	movdqa xmm1, [sym(k128_i16)]
;
;	LoopHeight1:
;		LoopWidth1:
;			movdqa xmm2, [rbx] ; 4 RGBA samples = 16bytes (2 are useless, we want 1 out of 2): axbx
;			punpckldq xmm2, xmm2 ; aaxx
;			punpckhdq xmm2, xmm2 ; bbxx
;			punpckldq xmm2, xmm2 ; abab
;			pmaddubsw xmm2, xmm0 ; Ua Ub Va Vb
;			phaddw xmm2, xmm2
;			psraw xmm2, 8 ; >> 8
;			paddw xmm2, xmm1 ; + 128 -> UUVV----
;			packuswb xmm2, xmm2 ; Saturate(I16 -> U8)
;			movd rdi, xmm2
;			mov rax, arg(1) ;outUPtr
;			mov [rax], dword rdi
;			add rax, 2
;			mov arg(1), rax		
;			
;			add rbx, 16 ; rgbaPtr += 16
;
;			; end-of-LoopWidth
;			mov rax, arg(4) ; width
;			sub rax, 4
;			mov arg(4), rax
;			cmp rax, 0
;			jg LoopWidth1
;	add rbx, rcx ; rgbaPtr += padRGBA
;	mov rax, [rsp]
;	add rax, rdx
;	mov [rsp], rax
;	mov rax, [rsp + 8]
;	add rax, rdx
;	mov [rsp + 8], rax
;	; end-of-LoopHeight1
;	sub rsi, 2
;	cmp rsi, 0
;	;jg LoopHeight1
;
;	; begin epilog
;	add rsp, 8+8+8
;	pop rbx
;	pop rdi
;	pop rsi
;    COMPV_YASM_UNSHADOW_ARGS
;	mov rsp, rbp
;	pop rbp
;	ret

	
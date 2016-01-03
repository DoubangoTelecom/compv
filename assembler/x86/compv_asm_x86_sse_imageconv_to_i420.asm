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

section .data
	extern sym(k16_i16)
	extern sym(kRGBAToYUV_YCoeffs8)

section .text

;;;
;;; void rgbaToI420Kernel11_CompY_Asm_X86_Aligned_SSSE3(COMV_ALIGNED(16) const uint8_t* rgbaPtr, uint8_t* outYPtr, size_t height, size_t width, size_t stride)
sym(rgbaToI420Kernel11_CompY_Asm_X86_Aligned_SSSE3):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	;COMPV_YASM_SAVE_XMM 6
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

	movdqa xmm0, [sym(kRGBAToYUV_YCoeffs8)]
	movdqa xmm1, [sym(k16_i16)]

	LoopHeight:
		mov rdi, arg(3) ; width
		LoopWidth:
			movdqa xmm2, [rax] ; 4 RGBA samples
			pmaddubsw xmm2, xmm0
			phaddw xmm2, xmm2
			psraw xmm2, 7
			paddw xmm2, xmm1
			packuswb xmm2, xmm2
			movd [rbx], xmm2
			add rbx, 4
			add rax, 16

			; end-of-LoopWidth
			sub rdi, 4
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
    ;COMPV_YASM_RESTORE_XMM
    COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret



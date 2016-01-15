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
%include "compv_common_x86.S"

COMPV_YASM_DEFAULT_REL

global sym(rgbaToI420Kernel11_CompY_Asm_X86_Aligned0x_SSSE3)
global sym(rgbaToI420Kernel11_CompY_Asm_X86_Aligned1x_SSSE3)
global sym(rgbaToI420Kernel41_CompY_Asm_X86_Aligned00_SSSE3)
global sym(rgbaToI420Kernel41_CompY_Asm_X86_Aligned01_SSSE3)
global sym(rgbaToI420Kernel41_CompY_Asm_X86_Aligned10_SSSE3)
global sym(rgbaToI420Kernel41_CompY_Asm_X86_Aligned11_SSSE3)
global sym(rgbaToI420Kernel11_CompUV_Asm_X86_Aligned0xx_SSSE3)
global sym(rgbaToI420Kernel11_CompUV_Asm_X86_Aligned1xx_SSSE3)
global sym(rgbaToI420Kernel41_CompUV_Asm_X86_Aligned0xx_SSSE3)
global sym(rgbaToI420Kernel41_CompUV_Asm_X86_Aligned1xx_SSSE3)

global sym(i420ToRGBAKernel11_Asm_X86_Aligned00_SSSE3)
global sym(i420ToRGBAKernel11_Asm_X86_Aligned01_SSSE3)
global sym(i420ToRGBAKernel11_Asm_X86_Aligned10_SSSE3)
global sym(i420ToRGBAKernel11_Asm_X86_Aligned11_SSSE3)

section .data
	extern sym(k5_i8)
	extern sym(k16_i16)
	extern sym(k16_i8)
	extern sym(k128_i16)
	extern sym(k255_i16)
	extern sym(k7120_i16)
	extern sym(k8912_i16)
	extern sym(k4400_i16)
	extern sym(kRGBAToYUV_YCoeffs8)
	extern sym(kRGBAToYUV_UCoeffs8)
	extern sym(kRGBAToYUV_VCoeffs8)
	extern sym(kRGBAToYUV_U2V2Coeffs8)
	extern sym(kYUVToRGBA_RCoeffs8)
	extern sym(kYUVToRGBA_GCoeffs8)
	extern sym(kYUVToRGBA_BCoeffs8)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* rgbaPtr
; arg(1) -> uint8_t* outYPtr
; arg(2) -> vcomp_scalar_t height
; arg(3) -> vcomp_scalar_t width
; arg(4) -> vcomp_scalar_t stride
; arg(5) -> COMV_ALIGNED(SSE) const int8_t* kXXXXToYUV_YCoeffs8
; %1 -> 1: rgbaPtr is aligned, 0: rgbaPtr not aligned
%macro rgbaToI420Kernel11_CompY_Asm_X86_SSSE3 1
push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
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

	mov rax, arg(5)
	movdqa xmm0, [rax]
	movdqa xmm1, [sym(k16_i16)]
	
	mov rax, arg(0) ; rgbaPtr
	mov rsi, arg(2) ; height
	mov rbx, arg(1) ; outYPtr

	.LoopHeight:
		xor rdi, rdi
		.LoopWidth:
			%if %1 == 1
			movdqa xmm2, [rax] ; 4 RGBA samples
			%else
			movdqu xmm2, [rax] ; 4 RGBA samples
			%endif
			pmaddubsw xmm2, xmm0
			phaddw xmm2, xmm2
			psraw xmm2, 7
			paddw xmm2, xmm1
			packuswb xmm2, xmm2
			movd [rbx], xmm2

			add rbx, 4
			add rax, 16

			; end-of-LoopWidth
			add rdi, 4
			cmp rdi, arg(3)
			jl .LoopWidth
	add rbx, rdx
	add rax, rcx
	; end-of-LoopHeight
	sub rsi, 1
	cmp rsi, 0
	jg .LoopHeight

	; begin epilog
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbaToI420Kernel11_CompY_Asm_X86_Aligned0x_SSSE3(const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(SSE) const int8_t* kXXXXToYUV_YCoeffs8)
sym(rgbaToI420Kernel11_CompY_Asm_X86_Aligned0x_SSSE3):
	rgbaToI420Kernel11_CompY_Asm_X86_SSSE3 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbaToI420Kernel11_CompY_Asm_X86_Aligned1x_SSSE3(COMV_ALIGNED(SSE) const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(SSE) const int8_t* kXXXXToYUV_YCoeffs8)
sym(rgbaToI420Kernel11_CompY_Asm_X86_Aligned1x_SSSE3):
	rgbaToI420Kernel11_CompY_Asm_X86_SSSE3 1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* rgbaPtr
; arg(1) -> uint8_t* outYPtr
; arg(2) -> vcomp_scalar_t height
; arg(3) -> vcomp_scalar_t width
; arg(4) -> vcomp_scalar_t stride
; arg(5) -> COMV_ALIGNED(SSE) const int8_t* kXXXXToYUV_YCoeffs8
; %1 -> 1: rgbaPtr aligned, 0: rgbaPtr not aligned
; %2 -> 1: outYPtr aligned, 0: outYPtr not aligned
%macro rgbaToI420Kernel41_CompY_Asm_SSSE3 2
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
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

	mov rax, arg(5)
	movdqa xmm0, [rax]
	movdqa xmm1, [sym(k16_i16)]
		
	mov rax, arg(0) ; rgbaPtr
	mov rsi, arg(2) ; height
	mov rbx, arg(1) ; outYPtr

	.LoopHeight:
		xor rdi, rdi
		.LoopWidth:
			%if %1 == 1
			movdqa xmm2, [rax] ; 4 RGBA samples
			movdqa xmm3, [rax + 16] ; 4 RGBA samples
			movdqa xmm4, [rax + 32] ; 4 RGBA samples
			movdqa xmm5, [rax + 48] ; 4 RGBA samples
			%else
			movdqu xmm2, [rax] ; 4 RGBA samples
			movdqu xmm3, [rax + 16] ; 4 RGBA samples
			movdqu xmm4, [rax + 32] ; 4 RGBA samples
			movdqu xmm5, [rax + 48] ; 4 RGBA samples
			%endif

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
			%if %2 == 1
			movdqa [rbx], xmm2
			%else
			movdqu [rbx], xmm2
			%endif

			add rbx, 16
			add rax, 64

			; end-of-LoopWidth
			add rdi, 16
			cmp rdi, arg(3)
			jl .LoopWidth
	add rbx, rcx
	add rax, rdx
	; end-of-LoopHeight
	sub rsi, 1
	cmp rsi, 0
	jg .LoopHeight

	; begin epilog
	pop rbx
	pop rdi
	pop rsi
    COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbaToI420Kernel41_CompY_Asm_X86_Aligned00_SSSE3(const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(SSE) const int8_t* kXXXXToYUV_YCoeffs8)
sym(rgbaToI420Kernel41_CompY_Asm_X86_Aligned00_SSSE3):
	rgbaToI420Kernel41_CompY_Asm_SSSE3 0, 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbaToI420Kernel41_CompY_Asm_X86_Aligned01_SSSE3(const uint8_t* rgbaPtr, COMV_ALIGNED(SSE) uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(SSE) const int8_t* kXXXXToYUV_YCoeffs8)
sym(rgbaToI420Kernel41_CompY_Asm_X86_Aligned01_SSSE3):
	rgbaToI420Kernel41_CompY_Asm_SSSE3 0, 1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbaToI420Kernel41_CompY_Asm_X86_Aligned10_SSSE3(COMV_ALIGNED(SSE) const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(SSE) const int8_t* kXXXXToYUV_YCoeffs8)
sym(rgbaToI420Kernel41_CompY_Asm_X86_Aligned10_SSSE3):
	rgbaToI420Kernel41_CompY_Asm_SSSE3 1, 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbaToI420Kernel41_CompY_Asm_X86_Aligned11_SSSE3(COMV_ALIGNED(SSE) const uint8_t* rgbaPtr, COMV_ALIGNED(SSE) uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
sym(rgbaToI420Kernel41_CompY_Asm_X86_Aligned11_SSSE3):
	rgbaToI420Kernel41_CompY_Asm_SSSE3 1, 1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* rgbaPtr
; arg(1) -> uint8_t* outUPtr
; arg(2) -> uint8_t* outVPtr
; arg(3) -> vcomp_scalar_t height
; arg(4) -> vcomp_scalar_t width
; arg(5) -> vcomp_scalar_t stride
; arg(6) -> COMV_ALIGNED(SSE) const int8_t* kXXXXToYUV_UCoeffs8
; arg(7) -> COMV_ALIGNED(SSE) const int8_t* kXXXXToYUV_VCoeffs8
; %1 -> 1: rgbaPtr is aligned, 0: rgbaPtr not aligned
%macro rgbaToI420Kernel11_CompUV_Asm_X86_SSSE3 1
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	push rsi
	push rdi
	push rbx
	; end prolog

	; alloc memory
	sub rsp, 8+8

	mov rax, arg(4)
	add rax, 3
	and rax, -4
	mov rcx, arg(5)
	sub rcx, rax
	mov rdx, rcx
	shr rdx, 1
	mov [rsp + 0], rdx ; [rsp + 0] = padUV
	add rcx, arg(5)
	shl rcx, 2
	mov [rsp + 8], rcx ; [rsp + 8] = padRGBA

	; UV coeffs interleaved: each appear #2 times (kRGBAToYUV_U2V2Coeffs8)
	mov rax, arg(6)
	mov rdx, arg(7)
	movdqa xmm0, [rax]
	punpcklqdq xmm0, [rdx] ; UV coeffs interleaved: each appear #2 times
	movdqa xmm1, [sym(k128_i16)]

	mov rax, arg(0) ; rgbaPtr
	mov rbx, arg(1) ; outUPtr
	mov rcx, arg(2) ; outVPtr
	mov rsi, arg(3) ; height

	.LoopHeight:
		xor rdi, rdi
		.LoopWidth:
			%if %1 == 1
			movdqa xmm2, [rax] ; 4 RGBA samples = 16bytes (2 are useless, we want 1 out of 2): axbx
			%else
			movdqu xmm2, [rax] ; 4 RGBA samples = 16bytes (2 are useless, we want 1 out of 2): axbx
			%endif
			punpckldq xmm2, xmm2 ; aaxx
			punpckhdq xmm2, xmm2 ; bbxx
			punpckldq xmm2, xmm2 ; abab
			pmaddubsw xmm2, xmm0 ; Ua Ub Va Vb
			phaddw xmm2, xmm2
			psraw xmm2, 8 ; >> 8
			paddw xmm2, xmm1 ; + 128 -> UUVV----
			packuswb xmm2, xmm2 ; Saturate(I16 -> U8)

			movd rdx, xmm2
			mov [rbx], dx
			shr rdx, 16
			mov [rcx], dx
			
			add rax, 16 ; rgbaPtr += 16
			add rbx, 2 ; outUPtr += 2
			add rcx, 2 ; outVPtr += 2

			; end-of-LoopWidth
			add rdi, 4
			cmp rdi, arg(4)
			jl .LoopWidth
	add rax, [rsp + 8] ; rgbaPtr += padRGBA
	add rbx, [rsp + 0] ; outUPtr += padUV
	add rcx, [rsp + 0] ; outVPtr += padUV
	; end-of-LoopHeight
	sub rsi, 2
	cmp rsi, 0
	jg .LoopHeight

	; free memory
	add rsp, 8+8

	; begin epilog
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbaToI420Kernel11_CompUV_Asm_X86_Aligned0xx_SSSE3(const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(SSE) const int8_t* kXXXXToYUV_UCoeffs8, COMV_ALIGNED(SSE) const int8_t* kXXXXToYUV_VCoeffs8)
sym(rgbaToI420Kernel11_CompUV_Asm_X86_Aligned0xx_SSSE3):
	rgbaToI420Kernel11_CompUV_Asm_X86_SSSE3 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbaToI420Kernel11_CompUV_Asm_X86_Aligned1xx_SSSE3(COMV_ALIGNED(SSE) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(SSE) const int8_t* kXXXXToYUV_UCoeffs8, COMV_ALIGNED(SSE) const int8_t* kXXXXToYUV_VCoeffs8)
sym(rgbaToI420Kernel11_CompUV_Asm_X86_Aligned1xx_SSSE3):
	rgbaToI420Kernel11_CompUV_Asm_X86_SSSE3 1


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* rgbaPtr
; arg(1) -> uint8_t* outUPtr
; arg(2) -> uint8_t* outVPtr
; arg(3) -> vcomp_scalar_t height
; arg(4) -> vcomp_scalar_t width
; arg(5) -> vcomp_scalar_t stride
; arg(6) -> COMV_ALIGNED(SSE) const int8_t* kXXXXToYUV_UCoeffs8
; arg(7) -> COMV_ALIGNED(SSE) const int8_t* kXXXXToYUV_VCoeffs8
; %1 -> 1: rgbaPtr is aligned, 0: rgbaPtr not aligned
%macro rgbaToI420Kernel41_CompUV_Asm_X86_SSSE3 1
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_XMM 7
	push rsi
	push rdi
	push rbx
	; end prolog

	sub rsp, 8+8

	mov rax, arg(4)
	add rax, 15
	and rax, -16
	mov rcx, arg(5)
	sub rcx, rax
	mov rdx, rcx
	shr rdx, 1
	mov [rsp + 0], rdx ; [rsp + 0] = padUV
	add rcx, arg(5)
	shl rcx, 2
	mov [rsp + 8], rcx ; [rsp + 8] = padRGBA

	mov rax, arg(6)
	mov rdx, arg(7)
	movdqa xmm0, [rax] ; xmmUCoeffs
	movdqa xmm1, [rdx] ; xmmVCoeffs
	movdqa xmm7, [sym(k128_i16)] ; xmm128

	mov rax, arg(0) ; rgbaPtr
	mov rbx, arg(1) ; outUPtr
	mov rcx, arg(2) ; outVPtr
	mov rsi, arg(3) ; height
	mov rdx, arg(4) ; width

	.LoopHeight:
		xor rdi, rdi
		.LoopWidth:
%if %1 == 1
			movdqa xmm2, [rax] ; 4 RGBA samples = 16bytes (2 are useless, we want 1 out of 2): 0x1x
			movdqa xmm3, [rax + 16] ; 4 RGBA samples = 16bytes :2x3x
			movdqa xmm4, [rax + 32] ; 4 RGBA samples = 16bytes : 4x5x
			movdqa xmm5, [rax + 48] ; 4 RGBA samples = 16bytes : 6x7x
%else
			movdqu xmm2, [rax] ; 4 RGBA samples = 16bytes (2 are useless, we want 1 out of 2): 0x1x
			movdqu xmm3, [rax + 16] ; 4 RGBA samples = 16bytes :2x3x
			movdqu xmm4, [rax + 32] ; 4 RGBA samples = 16bytes : 4x5x
			movdqu xmm5, [rax + 48] ; 4 RGBA samples = 16bytes : 6x7x
%endif

			movdqa xmm6, xmm2
			punpckldq xmm6, xmm3 ; 02xx
			punpckhdq xmm2, xmm3 ; 13xx
			punpckldq xmm2, xmm6 ; 0123
			movdqa xmm3, xmm2
			
			movdqa xmm6, xmm4
			punpckldq xmm6, xmm5 ; 46xx
			punpckhdq xmm4, xmm5 ; 57xx
			punpckldq xmm4, xmm6 ; 4567
			movdqa xmm5, xmm4

			; U = (xmm2, xmm4)
			; V = (xmm3, xmm5)

			pmaddubsw xmm2, xmm0
			pmaddubsw xmm4, xmm0
			pmaddubsw xmm3, xmm1
			pmaddubsw xmm5, xmm1

			; U = xmm2
			; V = xmm3

			phaddw xmm2, xmm4
			phaddw xmm3, xmm5

			psraw xmm2, 8 ; >> 8
			psraw xmm3, 8 ; >> 8

			paddw xmm2, xmm7 ; + 128 -> UUVV----
			paddw xmm3, xmm7 ; + 128 -> UUVV----

			; UV = xmm2

			packuswb xmm2, xmm3 ; Packs + Saturate(I16 -> U8)

			movq [rbx], xmm2
			psrldq xmm2, 8 ; >> 8
			movq [rcx], xmm2
			
			add rbx, 8 ; outUPtr += 8
			add rcx, 8 ; outVPtr += 8
			add rax, 64 ; rgbaPtr += 64

			; end-of-LoopWidth
			add rdi, 16
			cmp rdi, rdx
			jl .LoopWidth
	add rax, [rsp + 8] ; rgbaPtr += padRGBA
	add rbx, [rsp + 0] ; outUPtr += padUV
	add rcx, [rsp + 0] ; outVPtr += padUV
	; end-of-LoopHeight
	sub rsi, 2
	cmp rsi, 0
	jg .LoopHeight

	add rsp, 8+8

	; begin epilog
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbaToI420Kernel41_CompUV_Asm_X86_Aligned0xx_SSSE3(COMV_ALIGNED(SSE) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(SSE) const int8_t* kXXXXToYUV_UCoeffs8, COMV_ALIGNED(SSE) const int8_t* kXXXXToYUV_VCoeffs8)
sym(rgbaToI420Kernel41_CompUV_Asm_X86_Aligned0xx_SSSE3):
	rgbaToI420Kernel41_CompUV_Asm_X86_SSSE3 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbaToI420Kernel41_CompUV_Asm_X86_Aligned1xx_SSSE3(COMV_ALIGNED(SSE) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(SSE) const int8_t* kXXXXToYUV_UCoeffs8, COMV_ALIGNED(SSE) const int8_t* kXXXXToYUV_VCoeffs8)
sym(rgbaToI420Kernel41_CompUV_Asm_X86_Aligned1xx_SSSE3):
	rgbaToI420Kernel41_CompUV_Asm_X86_SSSE3 1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* yPtr
; arg(1) -> const uint8_t* uPtr
; arg(2) -> const uint8_t* vPtr
; arg(3) -> uint8_t* outRgbaPtr
; arg(4) -> vcomp_scalar_t height
; arg(5) -> vcomp_scalar_t width
; arg(6) -> vcomp_scalar_t stride
; %1 -> 1: yPtr aligned, 0: yPtr not aligned
; %2 -> 1: outRgbaPtr aligned, 0: outRgbaPtr not aligned
%macro i420ToRGBAKernel11_Asm_X86_SSSE3 2
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_XMM 7 ;XMM[6-7]
	push rsi
	push rdi
	push rbx
	; end prolog

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 16, rax
	sub rsp, 16+16+16+8+8+8+8

	; xmmY = [rsp + 32] ; xmmU = [rsp + 48] ; xmmV = [rsp + 64]
	mov rax, arg(5)
	add rax, 15
	and rax, -16
	mov rdx, rax
	add rdx, 1
	shr rdx, 1
	neg rdx
	mov [rsp + 0], rdx ; [rsp + 0] = rollbackUV
	mov rdx, arg(6)
	sub rdx, rax
	mov [rsp + 8], rdx ; [rsp + 8] = padY
	mov rcx, rdx
	add rdx, 1
	shr rdx, 1
	mov [rsp + 16], rdx ; [rsp + 16] = padUV
	shl rcx, 2
	mov [rsp + 24], rcx ; [rsp + 24] = padRGBA

	mov rcx, arg(0) ; yPtr
	mov rdx, arg(1) ; uPtr
	mov rbx, arg(2) ; vPtr
	mov rax, arg(3) ; outRgbaPtr
	mov rsi, arg(4) ; height

	.LoopHeight:
		xor rdi, rdi
		.LoopWidth:
			%if %1 == 1
			movdqa xmm0, [rcx] ; 16 Y samples = 16bytes
			%else
			movdqu xmm0, [rcx] ; 16 Y samples = 16bytes
			%endif
			movq xmm1, [rdx] ; 8 U samples, low mem
			movq xmm2, [rbx] ; 8 V samples, low mem
			punpcklbw xmm1, xmm1 ; duplicate -> 16 U samples
			punpcklbw xmm2, xmm2 ; duplicate -> 16 U samples

			;;;;;;;;;;;;;;;;;;;;;
			;;;;;; 8Y - LOW ;;;;;
			;;;;;;;;;;;;;;;;;;;;;
			
			; YUV0 = (xmm6 || xmm3)
			pxor xmm5, xmm5
			movdqa xmm3, xmm0
			movdqa xmm4, xmm1
			punpcklbw xmm3, xmm2 ; YVYVYVYVYVYVYV....
			punpcklbw xmm4, xmm5 ; U0U0U0U0U0U0U0U0....
			movdqa xmm6, xmm3
			punpcklbw xmm6, xmm4 ; YUV0YUV0YUV0YUV0YUV0YUV0
			punpckhbw xmm3, xmm4

			; save xmm0, xmm1 and xmm2
			movdqa [rsp + 32], xmm0
			movdqa [rsp + 48], xmm1
			movdqa [rsp + 64], xmm2

			; xmm0 = R
			movdqa xmm7, [sym(kYUVToRGBA_RCoeffs8)]
			movdqa xmm0, xmm6
			movdqa xmm1, xmm3
			pmaddubsw xmm0, xmm7
			pmaddubsw xmm1, xmm7
			phaddw xmm0, xmm1
			psubw xmm0, [sym(k7120_i16)]
			psraw xmm0, 5
			; xmm1 = B
			movdqa xmm7, [sym(kYUVToRGBA_BCoeffs8)]
			movdqa xmm1, xmm6
			movdqa xmm2, xmm3
			pmaddubsw xmm1, xmm7
			pmaddubsw xmm2, xmm7
			phaddw xmm1, xmm2
			psubw xmm1, [sym(k8912_i16)]
			psraw xmm1, 5
			; xmm4 = RBRBRBRBRBRB
			movdqa xmm4, xmm0
			movdqa xmm5, xmm0
			punpcklwd xmm4, xmm1
			punpckhwd xmm5, xmm1
			packuswb xmm4, xmm5

			; xmm6 = G
			movdqa xmm7, [sym(kYUVToRGBA_GCoeffs8)]
			movdqa xmm2, [sym(k255_i16)] ; alpha
			pmaddubsw xmm6, xmm7
			pmaddubsw xmm3, xmm7
			phaddw xmm6, xmm3
			paddw xmm6, [sym(k4400_i16)]
			psraw xmm6, 5
			; xmm3 = GAGAGAGAGAGAGA
			movdqa xmm3, xmm6
			punpcklwd xmm3, xmm2
			punpckhwd xmm6, xmm2
			packuswb xmm3, xmm6

			; outRgbaPtr[0-32] = RGBARGBARGBARGBA
			movdqa xmm0, xmm4
			punpckhbw xmm4, xmm3
			punpcklbw xmm0, xmm3
			%if %2 == 1
			movdqa [rax + 16], xmm4 ; high8(RGBARGBARGBARGBA)
			movdqa [rax + 0], xmm0 ; low8(RGBARGBARGBARGBA)
			%else
			movdqu [rax + 16], xmm4 ; high8(RGBARGBARGBARGBA)
			movdqu [rax + 0], xmm0 ; low8(RGBARGBARGBARGBA)
			%endif
			

			;;;;;;;;;;;;;;;;;;;;;
			;;;;;; 8Y-HIGH  ;;;;;
			;;;;;;;;;;;;;;;;;;;;;

			; restore xmm0, xmm1 and xmm2
			movdqa xmm0, [rsp + 32]
			movdqa xmm1, [rsp + 48]
			movdqa xmm2, [rsp + 64]

			; YUV0 = (xmm6 || xmm3)
			movdqa xmm3, xmm0
			punpckhbw xmm3, xmm2 ; YVYVYVYVYVYVYV....
			movdqa xmm4, xmm1
			pxor xmm5, xmm5
			punpckhbw xmm4, xmm5 ; U0U0U0U0U0U0U0U0....
			movdqa xmm6, xmm3
			punpcklbw xmm6, xmm4 ; YUV0YUV0YUV0YUV0YUV0YUV0
			punpckhbw xmm3, xmm4

			; xmm0 = R
			movdqa xmm7, [sym(kYUVToRGBA_RCoeffs8)]
			movdqa xmm0, xmm6
			movdqa xmm1, xmm3
			pmaddubsw xmm0, xmm7
			pmaddubsw xmm1, xmm7
			phaddw xmm0, xmm1
			psubw xmm0, [sym(k7120_i16)]
			psraw xmm0, 5
			; xmm1 = B
			movdqa xmm7, [sym(kYUVToRGBA_BCoeffs8)]
			movdqa xmm1, xmm6
			movdqa xmm2, xmm3
			pmaddubsw xmm1, xmm7
			pmaddubsw xmm2, xmm7
			phaddw xmm1, xmm2
			psubw xmm1, [sym(k8912_i16)]
			psraw xmm1, 5
			; xmm4 = RBRBRBRBRBRB
			movdqa xmm4, xmm0
			movdqa xmm5, xmm0
			punpcklwd xmm4, xmm1
			punpckhwd xmm5, xmm1
			packuswb xmm4, xmm5

			; xmm6 = G
			movdqa xmm7, [sym(kYUVToRGBA_GCoeffs8)] 
			movdqa xmm2, [sym(k255_i16)] ; alpha
			pmaddubsw xmm6, xmm7
			pmaddubsw xmm3, xmm7
			phaddw xmm6, xmm3
			paddw xmm6, [sym(k4400_i16)]
			psraw xmm6, 5
			; xmm3 = GAGAGAGAGAGAGA
			movdqa xmm3, xmm6
			punpcklwd xmm3, xmm2
			punpckhwd xmm6, xmm2
			packuswb xmm3, xmm6

			; outRgbaPtr[32-64] = RGBARGBARGBARGBA
			movdqa xmm0, xmm4
			punpckhbw xmm4, xmm3
			punpcklbw xmm0, xmm3
			%if %2 == 1
			movdqa [rax + 48], xmm4 ; high8(RGBARGBARGBARGBA)
			movdqa [rax + 32], xmm0 ; low8(RGBARGBARGBARGBA)
			%else
			movdqu [rax + 48], xmm4 ; high8(RGBARGBARGBARGBA)
			movdqu [rax + 32], xmm0 ; low8(RGBARGBARGBARGBA)
			%endif

			; Move pointers
			add rcx, 16 ; yPtr += 16
			add rdx, 8 ; uPtr += 8
			add rbx, 8 ; vPtr += 8
			add rax, 64 ; outRgbaPtr += 64

			; end-of-LoopWidth
			add rdi, 16
			cmp rdi, arg(5)
			jl .LoopWidth
	add rcx, [rsp + 8] ; yPtr += padY
	add rax, [rsp + 24] ; outRgbaPtr += padRGBA
	mov rdi, rsi
	and rdi, 1
	cmp rdi, 1
	je .rdi_odd
	.rdi_even:
		add rdx, [rsp + 0] ; uPtr += rollbackUV
		add rbx, [rsp + 0] ; vPtr += rollbackUV
		jmp .rdi_done
	.rdi_odd:
		add rdx, [rsp + 16] ; uPtr += padUV
		add rbx, [rsp + 16] ; vPtr += padUV
	.rdi_done:
	; end-of-LoopHeight
	sub rsi, 1
	cmp rsi, 0
	jg .LoopHeight

	; unalign stack and alloc memory
	add rsp, 16+16+16+8+8+8+8
	COMPV_YASM_UNALIGN_STACK

	; begin epilog
	pop rbx
	pop rdi
	pop rsi
    COMPV_YASM_RESTORE_XMM
    COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void i420ToRGBAKernel11_Asm_X86_Aligned00_SSSE3(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* outRgbaPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
sym(i420ToRGBAKernel11_Asm_X86_Aligned00_SSSE3):
	i420ToRGBAKernel11_Asm_X86_SSSE3 0, 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; void i420ToRGBAKernel11_Asm_X86_Aligned10_SSSE3(COMV_ALIGNED(SSE) const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* outRgbaPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
sym(i420ToRGBAKernel11_Asm_X86_Aligned10_SSSE3):
	i420ToRGBAKernel11_Asm_X86_SSSE3 1, 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; void i420ToRGBAKernel11_Asm_X86_Aligned01_SSSE3(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, COMV_ALIGNED(SSE) uint8_t* outRgbaPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
sym(i420ToRGBAKernel11_Asm_X86_Aligned01_SSSE3):
	i420ToRGBAKernel11_Asm_X86_SSSE3 0, 1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; void i420ToRGBAKernel11_Asm_X86_Aligned11_SSSE3(COMV_ALIGNED(SSE) const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, COMV_ALIGNED(SSE) uint8_t* outRgbaPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
sym(i420ToRGBAKernel11_Asm_X86_Aligned11_SSSE3):
	i420ToRGBAKernel11_Asm_X86_SSSE3 1, 1
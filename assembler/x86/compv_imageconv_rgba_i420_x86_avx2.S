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

global sym(rgbaToI420Kernel11_CompY_Asm_Aligned0_AVX2)
global sym(rgbaToI420Kernel11_CompY_Asm_Aligned1_AVX2)
global sym(rgbaToI420Kernel41_CompY_Asm_Aligned00_AVX2)
global sym(rgbaToI420Kernel41_CompY_Asm_Aligned01_AVX2)
global sym(rgbaToI420Kernel41_CompY_Asm_Aligned10_AVX2)
global sym(rgbaToI420Kernel41_CompY_Asm_Aligned11_AVX2)
global sym(rgbaToI420Kernel11_CompUV_Asm_Aligned0xx_AVX2)
global sym(rgbaToI420Kernel11_CompUV_Asm_Aligned1xx_AVX2)
global sym(rgbaToI420Kernel41_CompUV_Asm_Aligned000_AVX2)
global sym(rgbaToI420Kernel41_CompUV_Asm_Aligned100_AVX2)
global sym(rgbaToI420Kernel41_CompUV_Asm_Aligned110_AVX2)
global sym(rgbaToI420Kernel41_CompUV_Asm_Aligned111_AVX2)

section .data
	extern sym(k16_i16)
	extern sym(k128_i16)
	extern sym(kRGBAToYUV_YCoeffs8)
	extern sym(kRGBAToYUV_U4V4Coeffs8)
	extern sym(kRGBAToYUV_UCoeffs8)
	extern sym(kRGBAToYUV_VCoeffs8)
	extern sym(k_0_2_4_6_0_2_4_6_i32)
	extern sym(kMaskstore_0_i64)
	extern sym(kMaskstore_0_1_i64)
	extern sym(kMaskstore_0_i32)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* rgbaPtr
; arg(1) -> uint8_t* outYPtr
; arg(2) -> vcomp_scalar_t height
; arg(3) -> vcomp_scalar_t width
; arg(4) -> vcomp_scalar_t stride
; %1 -> 1: rgbaPtr is aligned, 0: rgbaPtr isn't aligned
%macro rgbaToI420Kernel11_CompY_Asm_AVX2 1
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	push rsi
	push rdi
	push rbx
	; end prolog

	mov rax, arg(3)
	add rax, 7
	and rax, -8
	mov rcx, arg(4)
	sub rcx, rax ; rcx = padY
	mov rdx, rcx
	shl rdx, 2 ; rdx = padRGBA

	mov rax, arg(0) ; rgbaPtr
	mov rsi, arg(2) ; height
	mov rbx, arg(1) ; outYPtr

	vzeroupper

	vmovdqa ymm0, [sym(kRGBAToYUV_YCoeffs8)] ; ymmYCoeffs
	vmovdqa ymm1, [sym(k16_i16)] ; ymm16
	vmovdqa ymm3, [sym(kMaskstore_0_i64)] ; ymmMaskToExtractFirst64Bits

	.LoopHeight:
		xor rdi, rdi
		.LoopWidth:
			%if %1 == 1
			vmovdqa ymm2, [rax] ; 8 RGBA samples
			%else
			vmovdqu ymm2, [rax] ; 8 RGBA samples
			%endif
			vpmaddubsw ymm2, ymm0
			vphaddw ymm2, ymm2 ; aaaabbbbaaaabbbb
			vpermq ymm2, ymm2, 0xD8 ; aaaaaaaabbbbbbbb
			vpsraw ymm2, 7
			vpaddw ymm2, ymm1
			vpackuswb ymm2, ymm2
			vpmaskmovq [rbx], ymm3, ymm2
			
			add rbx, 8
			add rax, 32

			; end-of-LoopWidth
			add rdi, 8
			cmp rdi, arg(3)
			jl .LoopWidth
	add rbx, rcx
	add rax, rdx
	; end-of-LoopHeight
	sub rsi, 1
	cmp rsi, 0
	jg .LoopHeight

	vzeroupper

	; begin epilog
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* rgbaPtr
; arg(1) -> uint8_t* outYPtr
; arg(2) -> vcomp_scalar_t height
; arg(3) -> vcomp_scalar_t width
; arg(4) -> vcomp_scalar_t stride
;;; void rgbaToI420Kernel11_CompY_Asm_Aligned0_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
sym(rgbaToI420Kernel11_CompY_Asm_Aligned0_AVX2):
	rgbaToI420Kernel11_CompY_Asm_AVX2 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr
; arg(1) -> uint8_t* outYPtr
; arg(2) -> vcomp_scalar_t height
; arg(3) -> vcomp_scalar_t width
; arg(4) -> vcomp_scalar_t stride
;;; void rgbaToI420Kernel11_CompY_Asm_Aligned_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
sym(rgbaToI420Kernel11_CompY_Asm_Aligned1_AVX2):
	rgbaToI420Kernel11_CompY_Asm_AVX2 1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* rgbaPtr
; arg(1) -> uint8_t* outYPtr
; arg(2) -> vcomp_scalar_t height
; arg(3) -> vcomp_scalar_t width
; arg(4) -> vcomp_scalar_t stride
; %1 -> 1: rgbaPtr aligned, 0: rgbaPtr isn't aligned
; %2 -> 1: outYPtr aligned, 0: outYPtr isn't aligned
%macro rgbaToI420Kernel41_CompY_Asm_AVX2 2
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	push rsi
	push rdi
	push rbx
	; end prolog

	mov rax, arg(3)
	add rax, 31
	and rax, -32
	mov rcx, arg(4)
	sub rcx, rax ; rcx = padY
	mov rdx, rcx
	shl rdx, 2 ; rdx = padRGBA

	mov rax, arg(0) ; rgbaPtr
	mov rsi, arg(2) ; height
	mov rbx, arg(1) ; outYPtr

	vzeroupper

	vmovdqa ymm0, [sym(kRGBAToYUV_YCoeffs8)] ; ymmYCoeffs
	vmovdqa ymm1, [sym(k16_i16)] ; ymm16

	.LoopHeight:
		xor rdi, rdi
		.LoopWidth:
			%if %1 == 1
			vmovdqa ymm2, [rax] ; 8 RGBA samples
			vmovdqa ymm3, [rax + 32] ; 8 RGBA samples	
			vmovdqa ymm4, [rax + 64] ; 8 RGBA samples	
			vmovdqa ymm5, [rax + 96] ; 8 RGBA samples
			%else
			vmovdqu ymm2, [rax] ; 8 RGBA samples
			vmovdqu ymm3, [rax + 32] ; 8 RGBA samples	
			vmovdqu ymm4, [rax + 64] ; 8 RGBA samples	
			vmovdqu ymm5, [rax + 96] ; 8 RGBA samples
			%endif

			vpmaddubsw ymm2, ymm0
			vpmaddubsw ymm3, ymm0
			vpmaddubsw ymm4, ymm0
			vpmaddubsw ymm5, ymm0

			vphaddw ymm2, ymm3 ; 0000111100001111
			vphaddw ymm4, ymm5 ; 2222333322223333

			vpermq ymm2, ymm2, 0xD8 ; 0000000011111111
			vpermq ymm4, ymm4, 0xD8 ; 2222222233333333

			vpsraw ymm2, 7 ; >> 7
			vpsraw ymm4, 7 ; >> 7

			vpaddw ymm2, ymm1 ; + 16
			vpaddw ymm4, ymm1 ; + 16

			vpackuswb ymm2, ymm4 ; Saturate(I16 -> U8): 002200220022...

			vpermq ymm2, ymm2, 0xD8 ; 000000022222.....

			%if %2 == 1
			vmovdqa [rbx], ymm2
			%else
			vmovdqu [rbx], ymm2
			%endif
			
			add rbx, 32
			add rax, 128

			; end-of-LoopWidth
			add rdi, 32
			cmp rdi, arg(3)
			jl .LoopWidth
	add rbx, rcx
	add rax, rdx
	; end-of-LoopHeight
	sub rsi, 1
	cmp rsi, 0
	jg .LoopHeight

	vzeroupper
	
	; begin epilog
	pop rbx
	pop rdi
	pop rsi
    COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* rgbaPtr
; arg(1) -> uint8_t* outYPtr
; arg(2) -> vcomp_scalar_t height
; arg(3) -> vcomp_scalar_t width
; arg(4) -> vcomp_scalar_t stride
;;; void rgbaToI420Kernel41_CompY_Asm_Aligned_AVX2(const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
sym(rgbaToI420Kernel41_CompY_Asm_Aligned00_AVX2):
	rgbaToI420Kernel41_CompY_Asm_AVX2 0, 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr
; arg(1) -> uint8_t* outYPtr
; arg(2) -> vcomp_scalar_t height
; arg(3) -> vcomp_scalar_t width
; arg(4) -> vcomp_scalar_t stride
;;; void rgbaToI420Kernel41_CompY_Asm_Aligned_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
sym(rgbaToI420Kernel41_CompY_Asm_Aligned10_AVX2):
	rgbaToI420Kernel41_CompY_Asm_AVX2 1, 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* rgbaPtr
; arg(1) -> COMV_ALIGNED(AVX2) uint8_t* outYPtr
; arg(2) -> vcomp_scalar_t height
; arg(3) -> vcomp_scalar_t width
; arg(4) -> vcomp_scalar_t stride
;;; void rgbaToI420Kernel41_CompY_Asm_Aligned_AVX2(const uint8_t* rgbaPtr, COMV_ALIGNED(AVX2) uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
sym(rgbaToI420Kernel41_CompY_Asm_Aligned01_AVX2)
	rgbaToI420Kernel41_CompY_Asm_AVX2 0, 1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr
; arg(1) -> COMV_ALIGNED(AVX2) uint8_t* outYPtr
; arg(2) -> vcomp_scalar_t height
; arg(3) -> vcomp_scalar_t width
; arg(4) -> vcomp_scalar_t stride
;;; void rgbaToI420Kernel41_CompY_Asm_Aligned_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, COMV_ALIGNED(AVX2) uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
sym(rgbaToI420Kernel41_CompY_Asm_Aligned11_AVX2):
	rgbaToI420Kernel41_CompY_Asm_AVX2 1, 1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* rgbaPtr
; arg(1) -> uint8_t* outUPtr
; arg(2) -> uint8_t* outVPtr
; arg(3) -> vcomp_scalar_t height
; arg(4) -> vcomp_scalar_t width
; arg(5) -> vcomp_scalar_t stride
; %1 -> 1: rgbaPtr is aligned, 0: rgbaPtr isn't aligned
%macro rgbaToI420Kernel11_CompUV_Asm_AVX2 1
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	push rsi
	push rdi
	push rbx
	sub rsp, 16
	; end prolog

	mov rax, arg(4)
	add rax, 7
	and rax, -8
	mov rcx, arg(5)
	sub rcx, rax
	shr rcx, 1
	mov [rsp + 0], rcx ; [rsp + 0] = padUV
	mov rcx, arg(5)
	sub rcx, rax
	add rcx, arg(5)
	shl rcx, 2
	mov [rsp + 8], rcx ; [rsp + 8] = padRGBA
	
	mov rbx, arg(0) ; rgbaPtr
	mov rcx, arg(1); outUPtr
	mov rdx, arg(2); outVPtr
	mov rsi, arg(3) ; height

	vzeroupper

	vmovdqa ymm0, [sym(kMaskstore_0_i32)] ; ymmMaskToExtractFirst32Bits
	vmovdqa ymm1, [sym(k128_i16)] ; ymm128
	vmovdqa ymm3, [sym(kRGBAToYUV_U4V4Coeffs8)] ; ymmUV4Coeffs	

	.LoopHeight:
		xor rdi, rdi
		.LoopWidth:
			%if %1 == 1
			vmovdqa ymm2, [rbx] ; 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): axbxcxdx
			%else
			vmovdqu ymm2, [rbx] ; 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): axbxcxdx
			%endif
			vpmaddubsw ymm2, ymm3 ; Ua Ub Uc Ud Va Vb Vc Vd
			vphaddw ymm2, ymm2
			vpermq ymm2, ymm2, 0xD8
			vpsraw ymm2, 8 ; >> 8
			vpaddw ymm2, ymm1 ; + 128 -> UUVV----
			vpackuswb ymm2, ymm2; Saturate(I16 -> U8)
			vpmaskmovd [rcx], ymm0, ymm2
			vpsrldq ymm2, ymm2, 4 ; >> 4
			vpmaskmovd [rdx], ymm0, ymm2
						
			add rbx, 32 ; rgbaPtr += 32
			add rcx, 4 ; outUPtr += 4
			add rdx, 4 ; outVPtr += 4

			; end-of-LoopWidth
			add rdi, 8
			cmp rdi, arg(4)
			jl .LoopWidth
	add rbx, [rsp + 8]
	add rcx, [rsp + 0]
	add rdx, [rsp + 0]
	
	; end-of-LoopHeight
	sub rsi, 2
	cmp rsi, 0
	jg .LoopHeight

	vzeroupper

	; begin epilog
	add rsp, 16
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* rgbaPtr
; arg(1) -> uint8_t* outUPtr
; arg(2) -> uint8_t* outVPtr
; arg(3) -> vcomp_scalar_t height
; arg(4) -> vcomp_scalar_t width
; arg(5) -> vcomp_scalar_t stride
;;; void rgbaToI420Kernel11_CompUV_Asm_Aligned0xx_AVX2(const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
sym(rgbaToI420Kernel11_CompUV_Asm_Aligned0xx_AVX2):
	rgbaToI420Kernel11_CompUV_Asm_AVX2 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr
; arg(1) -> uint8_t* outUPtr
; arg(2) -> uint8_t* outVPtr
; arg(3) -> vcomp_scalar_t height
; arg(4) -> vcomp_scalar_t width
; arg(5) -> vcomp_scalar_t stride
;;; void rgbaToI420Kernel11_CompUV_Asm_Aligned1xx_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
sym(rgbaToI420Kernel11_CompUV_Asm_Aligned1xx_AVX2):
	rgbaToI420Kernel11_CompUV_Asm_AVX2 1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* rgbaPtr
; arg(1) -> uint8_t* outUPtr
; arg(2) -> uint8_t* outVPtr
; arg(3) -> vcomp_scalar_t height
; arg(4) -> vcomp_scalar_t width
; arg(5) -> vcomp_scalar_t stride
; %1 -> 1: rgbaPtr is aligned, 0: rgbaPtr isn't aligned
; %2 -> 1: outUPtr is aligned, 0: outUPtr isn't aligned
; %3 -> 1: outVPtr is aligned, 0: outVPtr isn't aligned
%macro rgbaToI420Kernel41_CompUV_Asm_AVX2 3
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	push rsi
	push rdi
	push rbx
	sub rsp, 16
	; end prolog

	mov rax, arg(4)
	add rax, 31
	and rax, -32
	mov rcx, arg(5)
	sub rcx, rax
	shr rcx, 1
	mov [rsp + 0], rcx ; [rsp + 0] = padUV
	mov rcx, arg(5)
	sub rcx, rax
	add rcx, arg(5)
	shl rcx, 2
	mov [rsp + 8], rcx ; [rsp + 8] = padRGBA
	
	mov rbx, arg(0) ; rgbaPtr
	mov rcx, arg(1); outUPtr
	mov rdx, arg(2); outVPtr
	mov rsi, arg(3) ; height

	vzeroupper

	vmovdqa ymm0, [sym(kMaskstore_0_1_i64)] ; ymmMaskToExtract128bits
	vmovdqa ymm1, [sym(k128_i16)] ; ymm128

	.LoopHeight:
		xor rdi, rdi
		.LoopWidth:
			%if %1 == 1
			vmovdqa ymm4, [rbx] ; 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): axbxcxdx
			vmovdqa ymm5, [rbx + 32] ; 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): exfxgxhx
			vmovdqa ymm6, [rbx + 64] ; 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): ixjxkxlx
			vmovdqa ymm7, [rbx + 96] ; 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): mxnxoxpx
			%else
			vmovdqa ymm4, [rbx] ; 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): axbxcxdx
			vmovdqa ymm5, [rbx + 32] ; 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): exfxgxhx
			vmovdqa ymm6, [rbx + 64] ; 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): ixjxkxlx
			vmovdqa ymm7, [rbx + 96] ; 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): mxnxoxpx
			%endif

			vpunpckldq ymm2, ymm4, ymm5 ; aexxcgxx
			vpunpckhdq ymm3, ymm4, ymm5 ; bfxxdhxx
			vpunpckldq ymm4, ymm2, ymm3 ; abefcdgh
			vpermq ymm5, ymm4, 0xD8 ; abcdefgh
			vmovdqa ymm4, ymm5

			vpunpckldq ymm2, ymm6, ymm7 ; imxxkoxx
			vpunpckhdq ymm3, ymm6, ymm7 ; jnxxlpxx
			vpunpckldq ymm6, ymm2, ymm3 ; ijmnklop
			vpermq ymm7, ymm6, 0xD8 ; ijklmnop
			vmovdqa ymm6, ymm7

			; U = (ymm4, ymm6)
			; V = (ymm5, ymm7)

			vpmaddubsw ymm4, [sym(kRGBAToYUV_UCoeffs8)]
			vpmaddubsw ymm6, [sym(kRGBAToYUV_UCoeffs8)]
			vpmaddubsw ymm5, [sym(kRGBAToYUV_VCoeffs8)]
			vpmaddubsw ymm7, [sym(kRGBAToYUV_VCoeffs8)]

			; U = ymm4
			; V = ymm5

			vphaddw ymm4, ymm6
			vphaddw ymm5, ymm7
			vpermq ymm4, ymm4, 0xD8
			vpermq ymm5, ymm5, 0xD8

			vpsraw ymm4, 8 ; >> 8
			vpsraw ymm5, 8 ; >> 8

			vpaddw ymm4, ymm1 ; +128
			vpaddw ymm5, ymm1 ; +128

			; UV = ymm4

			vpackuswb ymm4, ymm5 ; Packs + Saturate(I16 -> U8)
			vpermq ymm4, ymm4, 0xD8

			%if %2 == 1
			vextractf128 [rcx], ymm4, 0
			%else
			vpmaskmovq [rcx], ymm0, ymm4
			%endif
			%if %3 == 1
			vextractf128 [rdx], ymm4, 1
			%else
			vpermq ymm4, ymm4, 0xE
			vpmaskmovq [rdx], ymm0, ymm4
			%endif
			
			add rbx, 128 ; rgbaPtr += 128
			add rcx, 16 ; outUPtr += 16
			add rdx, 16 ; outVPtr += 16

			; end-of-LoopWidth
			add rdi, 32
			cmp rdi, arg(4)
			jl .LoopWidth
	add rbx, [rsp + 8]
	add rcx, [rsp + 0]
	add rdx, [rsp + 0]
	
	; end-of-LoopHeight
	sub rsi, 2
	cmp rsi, 0
	jg .LoopHeight

	vzeroupper

	; begin epilog
	add rsp, 16
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* rgbaPtr
; arg(1) -> uint8_t* outUPtr
; arg(2) -> uint8_t* outVPtr
; arg(3) -> vcomp_scalar_t height
; arg(4) -> vcomp_scalar_t width
; arg(5) -> vcomp_scalar_t stride
;;; void rgbaToI420Kernel41_CompUV_Asm_Aligned000_AVX2(const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
sym(rgbaToI420Kernel41_CompUV_Asm_Aligned000_AVX2):
	rgbaToI420Kernel41_CompUV_Asm_AVX2 0, 0, 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr
; arg(1) -> uint8_t* outUPtr
; arg(2) -> uint8_t* outVPtr
; arg(3) -> vcomp_scalar_t height
; arg(4) -> vcomp_scalar_t width
; arg(5) -> vcomp_scalar_t stride
;;; void rgbaToI420Kernel41_CompUV_Asm_Aligned100_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
sym(rgbaToI420Kernel41_CompUV_Asm_Aligned100_AVX2):
	rgbaToI420Kernel41_CompUV_Asm_AVX2 1, 0, 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr
; arg(1) -> COMV_ALIGNED(AVX2) uint8_t* outUPtr
; arg(2) -> uint8_t* outVPtr
; arg(3) -> vcomp_scalar_t height
; arg(4) -> vcomp_scalar_t width
; arg(5) -> vcomp_scalar_t stride
;;; void rgbaToI420Kernel41_CompUV_Asm_Aligned110_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, COMV_ALIGNED(AVX2) uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
sym(rgbaToI420Kernel41_CompUV_Asm_Aligned110_AVX2):
	rgbaToI420Kernel41_CompUV_Asm_AVX2 1, 1, 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr
; arg(1) -> COMV_ALIGNED(AVX2) uint8_t* outUPtr
; arg(2) -> COMV_ALIGNED(AVX2) uint8_t* outVPtr
; arg(3) -> vcomp_scalar_t height
; arg(4) -> vcomp_scalar_t width
; arg(5) -> vcomp_scalar_t stride
;;; void rgbaToI420Kernel41_CompUV_Asm_Aligned000_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, COMV_ALIGNED(AVX2) uint8_t* outUPtr, COMV_ALIGNED(AVX2) uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
sym(rgbaToI420Kernel41_CompUV_Asm_Aligned111_AVX2):
	rgbaToI420Kernel41_CompUV_Asm_AVX2 1, 1, 1

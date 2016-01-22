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
%include "../../../x86/compv_common_x86.S"

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(rgbaToI420Kernel11_CompY_Asm_X64_Aligned0_AVX2)
global sym(rgbaToI420Kernel11_CompY_Asm_X64_Aligned1_AVX2)
global sym(rgbaToI420Kernel41_CompY_Asm_X64_Aligned00_AVX2)
global sym(rgbaToI420Kernel41_CompY_Asm_X64_Aligned01_AVX2)
global sym(rgbaToI420Kernel41_CompY_Asm_X64_Aligned10_AVX2)
global sym(rgbaToI420Kernel41_CompY_Asm_X64_Aligned11_AVX2)
global sym(rgbaToI420Kernel11_CompUV_Asm_X64_Aligned0xx_AVX2)
global sym(rgbaToI420Kernel11_CompUV_Asm_X64_Aligned1xx_AVX2)
global sym(rgbaToI420Kernel41_CompUV_Asm_X64_Aligned000_AVX2)
global sym(rgbaToI420Kernel41_CompUV_Asm_X64_Aligned100_AVX2)
global sym(rgbaToI420Kernel41_CompUV_Asm_X64_Aligned110_AVX2)
global sym(rgbaToI420Kernel41_CompUV_Asm_X64_Aligned111_AVX2)

global sym(i420ToRGBAKernel11_Asm_X64_Aligned00_AVX2)
global sym(i420ToRGBAKernel11_Asm_X64_Aligned01_AVX2)
global sym(i420ToRGBAKernel11_Asm_X64_Aligned10_AVX2)
global sym(i420ToRGBAKernel11_Asm_X64_Aligned11_AVX2)

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
	extern sym(kRGBAToYUV_U4V4Coeffs8)
	extern sym(kYUVToRGBA_RCoeffs8)
	extern sym(kYUVToRGBA_GCoeffs8)
	extern sym(kYUVToRGBA_BCoeffs8)
	extern sym(kAVXMaskstore_0_i64)
	extern sym(kAVXMaskstore_0_1_i64)
	extern sym(kAVXMaskstore_0_i32)
	extern sym(kAVXPermutevar8x32_AEBFCGDH_i32)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* rgbaPtr
; arg(1) -> uint8_t* outYPtr
; arg(2) -> vcomp_scalar_t height
; arg(3) -> vcomp_scalar_t width
; arg(4) -> vcomp_scalar_t stride
; arg(5) -> COMV_ALIGNED(AVX2)const int8_t* kXXXXToYUV_YCoeffs8
; %1 -> 1: rgbaPtr is aligned, 0: rgbaPtr isn't aligned
%macro rgbaToI420Kernel11_CompY_Asm_AVX2 1
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	; end prolog

	mov rax, arg(3)
	add rax, 7
	and rax, -8
	mov rcx, arg(4)
	sub rcx, rax ; rcx = padY
	mov rdx, rcx
	shl rdx, 2 ; rdx = padRGBA

	vzeroupper

	mov rax, arg(5)
	vmovdqa ymm0, [rax] ; ymmYCoeffs
	vmovdqa ymm1, [sym(k16_i16)] ; ymm16
	vmovdqa ymm3, [sym(kAVXMaskstore_0_i64)] ; ymmMaskToExtractFirst64Bits

	mov rax, arg(0) ; rgbaPtr
	mov r8, arg(2) ; height
	mov r10, arg(1) ; outYPtr

	.LoopHeight:
		xor r9, r9
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
			vpmaskmovq [r10], ymm3, ymm2
			
			add r10, 8
			add rax, 32

			; end-of-LoopWidth
			add r9, 8
			cmp r9, arg(3)
			jl .LoopWidth
	add r10, rcx
	add rax, rdx
	; end-of-LoopHeight
	sub r8, 1
	cmp r8, 0
	jg .LoopHeight

	vzeroupper

	; begin epilog
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbaToI420Kernel11_CompY_Asm_X64_Aligned0_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(AVX2)const int8_t* kXXXXToYUV_YCoeffs8)
sym(rgbaToI420Kernel11_CompY_Asm_X64_Aligned0_AVX2):
	rgbaToI420Kernel11_CompY_Asm_AVX2 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbaToI420Kernel11_CompY_Asm_X64_Aligned_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(AVX2)const int8_t* kXXXXToYUV_YCoeffs8)
sym(rgbaToI420Kernel11_CompY_Asm_X64_Aligned1_AVX2):
	rgbaToI420Kernel11_CompY_Asm_AVX2 1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* rgbaPtr
; arg(1) -> uint8_t* outYPtr
; arg(2) -> vcomp_scalar_t height
; arg(3) -> vcomp_scalar_t width
; arg(4) -> vcomp_scalar_t stride
; arg(5) -> COMV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_YCoeffs
; %1 -> 1: rgbaPtr aligned, 0: rgbaPtr isn't aligned
; %2 -> 1: outYPtr aligned, 0: outYPtr isn't aligned
%macro rgbaToI420Kernel41_CompY_Asm_AVX2 2
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	; end prolog

	mov rax, arg(3)
	add rax, 31
	and rax, -32
	mov rcx, arg(4)
	sub rcx, rax ; rcx = padY
	mov rdx, rcx
	shl rdx, 2 ; rdx = padRGBA

	vzeroupper

	mov rax, arg(5)
	vmovdqa ymm0, [rax] ; ymmYCoeffs
	vmovdqa ymm1, [sym(k16_i16)] ; ymm16
	vmovdqa ymm6, [sym(kAVXPermutevar8x32_AEBFCGDH_i32)] ; ymmAEBFCGDH

	mov rax, arg(0) ; rgbaPtr
	mov r8, arg(2) ; height
	mov r10, arg(1) ; outYPtr

	.LoopHeight:
		xor r9, r9
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

			vphaddw ymm2, ymm3 ; hadd(ABCD) -> ACBD
			vphaddw ymm4, ymm5 ; hadd(EFGH) -> EGFH

			vpsraw ymm2, 7 ; >> 7
			vpsraw ymm4, 7 ; >> 7

			vpaddw ymm2, ymm1 ; + 16
			vpaddw ymm4, ymm1 ; + 16

			vpackuswb ymm2, ymm4 ; Saturate(I16 -> U8): packus(ACBD, EGFH) -> AEBFCGDH

			; Final permute
			vpermd ymm2, ymm6, ymm2

			%if %2 == 1
			vmovdqa [r10], ymm2
			%else
			vmovdqu [r10], ymm2
			%endif
			
			add r10, 32
			add rax, 128

			; end-of-LoopWidth
			add r9, 32
			cmp r9, arg(3)
			jl .LoopWidth
	add r10, rcx
	add rax, rdx
	; end-of-LoopHeight
	sub r8, 1
	cmp r8, 0
	jg .LoopHeight

	vzeroupper
	
	; begin epilog
    COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbaToI420Kernel41_CompY_Asm_X64_Aligned_AVX2(const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_YCoeffs)
sym(rgbaToI420Kernel41_CompY_Asm_X64_Aligned00_AVX2):
	rgbaToI420Kernel41_CompY_Asm_AVX2 0, 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbaToI420Kernel41_CompY_Asm_X64_Aligned_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_YCoeffs)
sym(rgbaToI420Kernel41_CompY_Asm_X64_Aligned10_AVX2):
	rgbaToI420Kernel41_CompY_Asm_AVX2 1, 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbaToI420Kernel41_CompY_Asm_X64_Aligned_AVX2(const uint8_t* rgbaPtr, COMV_ALIGNED(AVX2) uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_YCoeffs)
sym(rgbaToI420Kernel41_CompY_Asm_X64_Aligned01_AVX2)
	rgbaToI420Kernel41_CompY_Asm_AVX2 0, 1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbaToI420Kernel41_CompY_Asm_X64_Aligned_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, COMV_ALIGNED(AVX2) uint8_t* outYPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_YCoeffs)
sym(rgbaToI420Kernel41_CompY_Asm_X64_Aligned11_AVX2):
	rgbaToI420Kernel41_CompY_Asm_AVX2 1, 1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* rgbaPtr
; arg(1) -> uint8_t* outUPtr
; arg(2) -> uint8_t* outVPtr
; arg(3) -> vcomp_scalar_t height
; arg(4) -> vcomp_scalar_t width
; arg(5) -> vcomp_scalar_t stride
; arg(6) -> COMV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_UCoeffs8
; arg(7) -> COMV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_VCoeffs8
; %1 -> 1: rgbaPtr is aligned, 0: rgbaPtr isn't aligned
%macro rgbaToI420Kernel11_CompUV_Asm_AVX2 1
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	; end prolog

	mov rax, arg(4)
	add rax, 7
	and rax, -8
	mov rcx, arg(5)
	sub rcx, rax
	shr rcx, 1
	mov r11, rcx ; r11 = padUV
	mov rcx, arg(5)
	sub rcx, rax
	add rcx, arg(5)
	shl rcx, 2
	mov r12, rcx ; r12 = padRGBA

	vzeroupper

	; load UV coeffs interleaved: each appear #4 times (kRGBAToYUV_U4V4Coeffs8) - #4times U(or V) = #4 times 32bits = 128bits
	mov rax, arg(6)
	mov rdx, arg(7)
	vmovdqa ymm0, [rdx]
	vmovdqa ymm1, [rax]
	vinsertf128 ymm3, ymm1, xmm0, 0x1 ; ymmUV4Coeffs
	vmovdqa ymm0, [sym(kAVXMaskstore_0_i32)] ; ymmMaskToExtractFirst32Bits
	vmovdqa ymm1, [sym(k128_i16)] ; ymm128
	
	mov r10, arg(0) ; rgbaPtr
	mov rcx, arg(1); outUPtr
	mov rdx, arg(2); outVPtr
	mov r8, arg(3) ; height

	.LoopHeight:
		xor r9, r9
		.LoopWidth:
			%if %1 == 1
			vmovdqa ymm2, [r10] ; 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): axbxcxdx
			%else
			vmovdqu ymm2, [r10] ; 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): axbxcxdx
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
						
			add r10, 32 ; rgbaPtr += 32
			add rcx, 4 ; outUPtr += 4
			add rdx, 4 ; outVPtr += 4

			; end-of-LoopWidth
			add r9, 8
			cmp r9, arg(4)
			jl .LoopWidth
	add r10, r12
	add rcx, r11
	add rdx, r11
	
	; end-of-LoopHeight
	sub r8, 2
	cmp r8, 0
	jg .LoopHeight

	vzeroupper

	; begin epilog
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbaToI420Kernel11_CompUV_Asm_X64_Aligned0xx_AVX2(const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_UCoeffs8, COMV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_VCoeffs8)
sym(rgbaToI420Kernel11_CompUV_Asm_X64_Aligned0xx_AVX2):
	rgbaToI420Kernel11_CompUV_Asm_AVX2 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbaToI420Kernel11_CompUV_Asm_X64_Aligned1xx_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_UCoeffs8, COMV_ALIGNED(AVX2) const int8_t* kXXXXToYUV_VCoeffs8)
sym(rgbaToI420Kernel11_CompUV_Asm_X64_Aligned1xx_AVX2):
	rgbaToI420Kernel11_CompUV_Asm_AVX2 1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* rgbaPtr
; arg(1) -> uint8_t* outUPtr
; arg(2) -> uint8_t* outVPtr
; arg(3) -> vcomp_scalar_t height
; arg(4) -> vcomp_scalar_t width
; arg(5) -> vcomp_scalar_t stride
; arg(6) -> COMV_ALIGNED(AVX2)const int8_t* kXXXXToYUV_UCoeffs8
; arg(7) -> COMV_ALIGNED(AVX2)const int8_t* kXXXXToYUV_VCoeffs8
; %1 -> 1: rgbaPtr is aligned, 0: rgbaPtr isn't aligned
; %2 -> 1: outUPtr is aligned, 0: outUPtr isn't aligned
; %3 -> 1: outVPtr is aligned, 0: outVPtr isn't aligned
%macro rgbaToI420Kernel41_CompUV_Asm_AVX2 3
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	; end prolog

	mov rax, arg(4)
	add rax, 31
	and rax, -32
	mov rcx, arg(5)
	sub rcx, rax
	shr rcx, 1
	mov r11, rcx ; r11 = padUV
	mov rcx, arg(5)
	sub rcx, rax
	add rcx, arg(5)
	shl rcx, 2
	mov r12, rcx ; r12 = padRGBA

	vzeroupper

	mov rax, arg(6) 
	mov rdx, arg(7)
	vmovdqa ymm0, [sym(kAVXMaskstore_0_1_i64)] ; ymmMaskToExtract128bits
	vmovdqa ymm1, [sym(k128_i16)] ; ymm128
	vmovdqa ymm8, [rax] ; ymmUCoeffs
	vmovdqa ymm9, [rdx] ; ymmVCoeffs
	vmovdqa ymm10, [sym(kAVXPermutevar8x32_AEBFCGDH_i32)] ; ymmAEBFCGDH
	
	mov r10, arg(0) ; rgbaPtr
	mov rcx, arg(1); outUPtr
	mov rdx, arg(2); outVPtr
	mov r8, arg(3) ; height

	.LoopHeight:
		xor r9, r9
		.LoopWidth:
			%if %1 == 1
			vmovdqa ymm4, [r10] ; 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): axbxcxdx
			vmovdqa ymm5, [r10 + 32] ; 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): exfxgxhx
			vmovdqa ymm6, [r10 + 64] ; 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): ixjxkxlx
			vmovdqa ymm7, [r10 + 96] ; 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): mxnxoxpx
			%else
			vmovdqu ymm4, [r10] ; 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): axbxcxdx
			vmovdqu ymm5, [r10 + 32] ; 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): exfxgxhx
			vmovdqu ymm6, [r10 + 64] ; 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): ixjxkxlx
			vmovdqu ymm7, [r10 + 96] ; 8 RGBA samples = 32bytes (4 are useless, we want 1 out of 2): mxnxoxpx
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

			vpmaddubsw ymm4, ymm8
			vpmaddubsw ymm6, ymm8
			vpmaddubsw ymm5, ymm9
			vpmaddubsw ymm7, ymm9

			; U = ymm4
			; V = ymm5

			vphaddw ymm4, ymm6
			vphaddw ymm5, ymm7

			vpsraw ymm4, 8 ; >> 8
			vpsraw ymm5, 8 ; >> 8

			vpaddw ymm4, ymm1 ; +128
			vpaddw ymm5, ymm1 ; +128

			; UV = ymm4
			
			vpackuswb ymm4, ymm5 ; Packs + Saturate(I16 -> U8)

			; Final Permute
			vpermd ymm4, ymm10, ymm4

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
			
			add r10, 128 ; rgbaPtr += 128
			add rcx, 16 ; outUPtr += 16
			add rdx, 16 ; outVPtr += 16

			; end-of-LoopWidth
			add r9, 32
			cmp r9, arg(4)
			jl .LoopWidth
	add r10, r12
	add rcx, r11
	add rdx, r11
	
	; end-of-LoopHeight
	sub r8, 2
	cmp r8, 0
	jg .LoopHeight

	vzeroupper

	; begin epilog
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbaToI420Kernel41_CompUV_Asm_X64_Aligned000_AVX2(const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(AVX2)const int8_t* kXXXXToYUV_UCoeffs8, COMV_ALIGNED(AVX2)const int8_t* kXXXXToYUV_VCoeffs8)
sym(rgbaToI420Kernel41_CompUV_Asm_X64_Aligned000_AVX2):
	rgbaToI420Kernel41_CompUV_Asm_AVX2 0, 0, 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbaToI420Kernel41_CompUV_Asm_X64_Aligned100_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(AVX2)const int8_t* kXXXXToYUV_UCoeffs8, COMV_ALIGNED(AVX2)const int8_t* kXXXXToYUV_VCoeffs8)
sym(rgbaToI420Kernel41_CompUV_Asm_X64_Aligned100_AVX2):
	rgbaToI420Kernel41_CompUV_Asm_AVX2 1, 0, 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbaToI420Kernel41_CompUV_Asm_X64_Aligned110_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, COMV_ALIGNED(AVX2) uint8_t* outUPtr, uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(AVX2)const int8_t* kXXXXToYUV_UCoeffs8, COMV_ALIGNED(AVX2)const int8_t* kXXXXToYUV_VCoeffs8)
sym(rgbaToI420Kernel41_CompUV_Asm_X64_Aligned110_AVX2):
	rgbaToI420Kernel41_CompUV_Asm_AVX2 1, 1, 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbaToI420Kernel41_CompUV_Asm_X64_Aligned000_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgbaPtr, COMV_ALIGNED(AVX2) uint8_t* outUPtr, COMV_ALIGNED(AVX2) uint8_t* outVPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride, COMV_ALIGNED(AVX2)const int8_t* kXXXXToYUV_UCoeffs8, COMV_ALIGNED(AVX2)const int8_t* kXXXXToYUV_VCoeffs8)
sym(rgbaToI420Kernel41_CompUV_Asm_X64_Aligned111_AVX2):
	rgbaToI420Kernel41_CompUV_Asm_AVX2 1, 1, 1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* yPtr
; arg(1) -> const uint8_t* uPtr
; arg(2) -> const uint8_t* vPtr
; arg(3) -> uint8_t* outRgbaPtr
; arg(4) -> vcomp_scalar_t height
; arg(5) -> vcomp_scalar_t width
; arg(6) -> vcomp_scalar_t stride
; %1 -> 1: yPtr aligned, 0: yPtr not aligned
; %2 -> 1: outRgbaPtr aligned, 0: outRgbaPtr not aligned
%macro i420ToRGBAKernel11_Asm_AVX2 2
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	; end prolog

	mov rax, arg(5)
	add rax, 31
	and rax, -32
	mov rdx, rax
	add rdx, 1
	shr rdx, 1
	neg rdx
	mov r11, rdx ; r11 = rollbackUV
	mov rdx, arg(6)
	sub rdx, rax
	mov r12, rdx ; r12 = padY
	mov rcx, rdx
	add rdx, 1
	shr rdx, 1
	mov r13, rdx ; r13 = padUV
	shl rcx, 2
	mov r14, rcx ; r14 = padRGBA

	mov rcx, arg(0) ; yPtr
	mov rdx, arg(1) ; uPtr
	mov r10, arg(2) ; vPtr
	mov rax, arg(3) ; outRgbaPtr
	mov r8, arg(4) ; height

	vzeroupper

	vmovdqa ymm11, [sym(kAVXMaskstore_0_1_i64)]
	vmovdqa ymm12, [sym(kYUVToRGBA_RCoeffs8)]
	vmovdqa ymm13, [sym(kYUVToRGBA_BCoeffs8)]
	vmovdqa ymm14, [sym(kYUVToRGBA_GCoeffs8)]
	vmovdqa ymm15, [sym(k255_i16)] ; alpha

	.LoopHeight:
		xor r9, r9
		.LoopWidth:
			%if %1 == 1
			vmovdqa ymm8, [rcx] ; 32 Y samples = 32bytes
			%else
			vmovdqu ymm8, [rcx] ; 32 Y samples = 32bytes
			%endif
			vpmaskmovq ymm9, ymm11, [rdx] ; 16 U samples, low mem
			vpmaskmovq ymm10, ymm11, [r10] ; 16 V samples, low mem
			; Duplicate and interleave
			vpermq ymm9, ymm9, 0xD8
			vpunpcklbw ymm9, ymm9
			vpermq ymm10, ymm10, 0xD8
			vpunpcklbw ymm10, ymm10

			;;;;;;;;;;;;;;;;;;;;;
			;;;;;; 16Y-LOW ;;;;;;
			;;;;;;;;;;;;;;;;;;;;;
			
			; YUV0 = (ymm6 || ymm3)
			vpxor ymm5, ymm5
			vpunpcklbw ymm3, ymm8, ymm10 ; YVYVYVYVYVYVYV....
			vpunpcklbw ymm4, ymm9, ymm5 ; U0U0U0U0U0U0U0U0....
			vpunpcklbw ymm6, ymm3, ymm4 ; YUV0YUV0YUV0YUV0YUV0YUV0
			vpunpckhbw ymm3, ymm4

			; ymm0 = R
			vpmaddubsw ymm0, ymm6, ymm12
			vpmaddubsw ymm1, ymm3, ymm12
			vphaddw ymm0, ymm1
			vpsubw ymm0, [sym(k7120_i16)]
			vpsraw ymm0, 5
			; ymm1 = B
			vpmaddubsw ymm1, ymm6, ymm13
			vpmaddubsw ymm2, ymm3, ymm13
			vphaddw ymm1, ymm2
			vpsubw ymm1, [sym(k8912_i16)]
			vpsraw ymm1, 5
			; ymm4 = RBRBRBRBRBRB
			vpunpcklwd ymm4, ymm0, ymm1
			vpunpckhwd ymm5, ymm0, ymm1
			vpackuswb ymm4, ymm5

			; ymm6 = G
			vpmaddubsw ymm6, ymm14
			vpmaddubsw ymm3, ymm14
			vphaddw ymm6, ymm3
			vpaddw ymm6, [sym(k4400_i16)]
			vpsraw ymm6, 5
			; ymm3 = GAGAGAGAGAGAGA
			vpunpcklwd ymm3, ymm6, ymm15
			vpunpckhwd ymm6, ymm15
			vpackuswb ymm3, ymm6

			; outRgbaPtr[x-y] = RGBARGBARGBARGBA
			; re-order the samples for the final unpacklo, unpackhi
			vpermq ymm4, ymm4, 0xD8
			vpermq ymm3, ymm3, 0xD8
			; because of AVX cross-lane issue final data = (0, 2, 1, 3)*32 = (0, 64, 32, 96)
			vpunpcklbw ymm5, ymm4, ymm3
			vpunpckhbw ymm4, ymm3

			%if %2==1
			vmovdqa [rax + 0], ymm5 ; high8(RGBARGBARGBARGBA)
			vmovdqa [rax + 64], ymm4 ; low8(RGBARGBARGBARGBA)
			%else
			vmovdqu [rax + 0], ymm5 ; high8(RGBARGBARGBARGBA)
			vmovdqu [rax + 64], ymm4 ; low8(RGBARGBARGBARGBA)
			%endif

			;;;;;;;;;;;;;;;;;;;;;
			;;;;;; 8Y-HIGH  ;;;;;
			;;;;;;;;;;;;;;;;;;;;;

			; YUV0 = (ymm6 || ymm3)
			vpxor ymm5, ymm5
			vpunpckhbw ymm3, ymm8, ymm10 ; YVYVYVYVYVYVYV....
			vpunpckhbw ymm4, ymm9, ymm5 ; U0U0U0U0U0U0U0U0....
			vpunpcklbw ymm6, ymm3, ymm4 ; YUV0YUV0YUV0YUV0YUV0YUV0
			vpunpckhbw ymm3, ymm4

			; ymm0 = R
			vpmaddubsw ymm0, ymm6, ymm12
			vpmaddubsw ymm1, ymm3, ymm12
			vphaddw ymm0, ymm1
			vpsubw ymm0, [sym(k7120_i16)]
			vpsraw ymm0, 5
			; ymm1 = B
			vpmaddubsw ymm1, ymm6, ymm13
			vpmaddubsw ymm2, ymm3, ymm13
			vphaddw ymm1, ymm2
			vpsubw ymm1, [sym(k8912_i16)]
			vpsraw ymm1, 5
			; ymm4 = RBRBRBRBRBRB
			vpunpcklwd ymm4, ymm0, ymm1
			vpunpckhwd ymm5, ymm0, ymm1
			vpackuswb ymm4, ymm5

			; ymm6 = G
			vpmaddubsw ymm6, ymm14
			vpmaddubsw ymm3, ymm14
			vphaddw ymm6, ymm3
			vpaddw ymm6, [sym(k4400_i16)]
			vpsraw ymm6, 5
			; ymm3 = GAGAGAGAGAGAGA
			vpunpcklwd ymm3, ymm6, ymm15
			vpunpckhwd ymm6, ymm15
			vpackuswb ymm3, ymm6

			; outRgbaPtr[x-y] = RGBARGBARGBARGBA
			; re-order the samples for the final unpacklo, unpackhi
			vpermq ymm4, ymm4, 0xD8
			vpermq ymm3, ymm3, 0xD8
			; because of AVX cross-lane issue final data = (0, 2, 1, 3)*32 = (0, 64, 32, 96)
			vpunpcklbw ymm5, ymm4, ymm3
			vpunpckhbw ymm4, ymm3
			%if %2==1
			vmovdqa [rax + 32], ymm5 ; high8(RGBARGBARGBARGBA)
			vmovdqa [rax + 96], ymm4 ; low8(RGBARGBARGBARGBA)
			%else
			vmovdqu [rax + 32], ymm5 ; high8(RGBARGBARGBARGBA)
			vmovdqu [rax + 96], ymm4 ; low8(RGBARGBARGBARGBA)
			%endif

			; Move pointers
			add rcx, 32 ; yPtr += 32
			add rdx, 16 ; uPtr += 16
			add r10, 16 ; vPtr += 16
			add rax, 128 ; outRgbaPtr += 128

			; end-of-LoopWidth
			add r9, 32
			cmp r9, arg(5)
			jl .LoopWidth
	add rcx, r12 ; yPtr += padY
	add rax, r14 ; outRgbaPtr += padRGBA
	mov r9, r8
	and r9, 1
	cmp r9, 1
	je .r9_odd
	.r9_even:
		add rdx, r11 ; uPtr += rollbackUV
		add r10, r11 ; vPtr += rollbackUV
		jmp .r9_done
	.r9_odd:
		add rdx, r13 ; uPtr += padUV
		add r10, r13 ; vPtr += padUV
	.r9_done:
	; end-of-LoopHeight
	sub r8, 1
	cmp r8, 0
	jg .LoopHeight

	vzeroupper

	; begin epilog
    COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void i420ToRGBAKernel11_Asm_X64_Aligned00_AVX2(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* outRgbaPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
sym(i420ToRGBAKernel11_Asm_X64_Aligned00_AVX2):
	i420ToRGBAKernel11_Asm_AVX2 0, 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void i420ToRGBAKernel11_Asm_X64_Aligned10_AVX2(COMV_ALIGNED(AVX2) const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, uint8_t* outRgbaPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
sym(i420ToRGBAKernel11_Asm_X64_Aligned10_AVX2):
	i420ToRGBAKernel11_Asm_AVX2 1, 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void i420ToRGBAKernel11_Asm_X64_Aligned01_AVX2(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, COMV_ALIGNED(AVX2) uint8_t* outRgbaPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
sym(i420ToRGBAKernel11_Asm_X64_Aligned01_AVX2):
	i420ToRGBAKernel11_Asm_AVX2 0, 1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void i420ToRGBAKernel11_Asm_X64_Aligned11_AVX2(COMV_ALIGNED(AVX2) const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, COMV_ALIGNED(AVX2) uint8_t* outRgbaPtr, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
sym(i420ToRGBAKernel11_Asm_X64_Aligned11_AVX2):
	i420ToRGBAKernel11_Asm_AVX2 1, 1



%endif ; COMPV_YASM_ABI_IS_64BIT
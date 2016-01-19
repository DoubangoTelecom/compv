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
%include "../compv_common_x86.S"

COMPV_YASM_DEFAULT_REL

global sym(rgbToRgbaKernel31_Asm_X86_Aligned00_AVX2)
global sym(rgbToRgbaKernel31_Asm_X86_Aligned01_AVX2)
global sym(rgbToRgbaKernel31_Asm_X86_Aligned10_AVX2)
global sym(rgbToRgbaKernel31_Asm_X86_Aligned11_AVX2)

section .data
	extern sym(k_0_0_0_255_u8)
	extern sym(kAVXPermutevar8x32_ABCDDEFG_i32)
	extern sym(kAVXPermutevar8x32_CDEFFGHX_i32)
	extern sym(kAVXPermutevar8x32_XXABBCDE_i32)
	extern sym(kShuffleEpi8_RgbToRgba_i32)

section .text


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* rgb, 
; arg(1) -> uint8_t* rgba
; arg(2) -> vcomp_scalar_t height
; arg(3) -> vcomp_scalar_t width
; arg(4) -> vcomp_scalar_t stride
; %1 -> 1: rgb aligned, 0: rgb not aligned
; %2 -> 1: rgba aligned, 0: rgba not aligned
%macro rgbToRgbaKernel31_Asm_X86_AVX2 2
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	push rsi
	push rdi
	push rbx
	; end prolog

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 32, rax
	sub rsp, 32

	mov rax, arg(3)
	add rax, 31
	and rax, -32
	neg rax
	add rax, arg(4)
	mov rdx, rax
	imul rdx, 3
	mov rsi, rdx ; padRGB
	shl rax, 2
	mov rbx, rax ; padRAGB

	mov rax, arg(0) ; rgb
	mov rdx, arg(1) ; rgba
	mov rcx, arg(2) ; height

	vzeroupper

	vmovdqa ymm4, [sym(kAVXPermutevar8x32_CDEFFGHX_i32)]
	vmovdqa ymm5, [sym(kAVXPermutevar8x32_XXABBCDE_i32)] ; ymmXXABBCDE
	vmovdqa ymm6, [sym(kShuffleEpi8_RgbToRgba_i32)] ; ymmMaskRgbToRgba
	vmovdqa ymm7, [sym(kAVXPermutevar8x32_ABCDDEFG_i32)] ; ymmABCDDEFG

	.LoopHeight:
		xor rdi, rdi
		.LoopWidth:
			;;;;;;;;;; Line-0 ;;;;;;;;;;
			%if %1==1
			vmovdqa ymm0, [rax + 0] ; load first 32 samples
			%else
			vmovdqu ymm0, [rax + 0] ; load first 32 samples
			%endif
			vpermd ymm1, ymm7, ymm0
			vpshufb ymm1, ymm1, ymm6
			vpaddb ymm1, ymm1, [sym(k_0_0_0_255_u8)]
			%if %2==1
			vmovdqa [rdx + 0], ymm1
			%else
			vmovdqu [rdx + 0], ymm1
			%endif

			;;;;;;;;;; Line-1 ;;;;;;;;;;
			%if %1==1
			vmovdqa ymm1, [rax + 32] ; load next 32 samples
			%else
			vmovdqu ymm1, [rax + 32] ; load next 32 samples
			%endif
			vpermq ymm0, ymm0, 0xff ; duplicate lost0
			vextractf128 [rsp + 0], ymm1, 0x1
			vbroadcasti128 ymm3, [rsp + 0] ; ymmLost = ymm3 = high-128 = low-lost = lost0 || lost1
			vpermd ymm1, ymm5, ymm1
			vpblendd ymm1, ymm1, ymm0, 0x03 ; ymm0(64bits)||ymm1(192bits)
			vpshufb ymm1, ymm1, ymm6
			vpaddb ymm1, ymm1, [sym(k_0_0_0_255_u8)]
			%if %2==1
			vmovdqa [rdx + 32], ymm1
			%else
			vmovdqu [rdx + 32], ymm1
			%endif

			;;;;;;;;;; Line-2 ;;;;;;;;;;
			%if %1==1
			vmovdqa ymm0, [rax + 64] ; load next 32 samples
			%else
			vmovdqu ymm0, [rax + 64] ; load next 32 samples
			%endif
			vpermd ymm1, ymm4, ymm0 ; lost0 || lost1 || lost2 || garbage
			vextractf128 [rsp + 0], ymm0, 0x0
			vinserti128 ymm3, ymm3, [rsp + 0], 0x1
			vpermd ymm0, ymm7, ymm3
			vpshufb ymm0, ymm0, ymm6
			vpaddb ymm0, ymm0, [sym(k_0_0_0_255_u8)]
			%if %2==1
			vmovdqa [rdx + 64], ymm0
			%else
			vmovdqu [rdx + 64], ymm0
			%endif

			;;;;;;;;;; Line-3 ;;;;;;;;;;
			vpshufb ymm1, ymm1, ymm6
			vpaddb ymm1, ymm1, [sym(k_0_0_0_255_u8)]
			%if %2==1
			vmovdqa [rdx + 96], ymm1
			%else
			vmovdqu [rdx + 96], ymm1
			%endif
			
			add rax, 96 ; rgb += 96
			add rdx, 128 ; rgba += 128

			; end-of-LoopWidth
			add rdi, 32
			cmp rdi, arg(3)
			jl .LoopWidth
	add rax, rsi ; rgb += padRGB
	add rdx, rbx ; rgba += padRGBA
	; end-of-LoopHeight
	dec rcx
	jnz .LoopHeight

	vzeroupper

	; unalign stack and alloc memory
	add rsp, 32
	COMPV_YASM_UNALIGN_STACK

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
;;; void rgbToRgbaKernel31_Asm_X86_Aligned00_AVX2(const uint8_t* rgb, uint8_t* rgba, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
sym(rgbToRgbaKernel31_Asm_X86_Aligned00_AVX2):
	rgbToRgbaKernel31_Asm_X86_AVX2 0, 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbToRgbaKernel31_Asm_X86_Aligned01_AVX2(const uint8_t* rgb, COMV_ALIGNED(AVX2) uint8_t* rgba, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
sym(rgbToRgbaKernel31_Asm_X86_Aligned01_AVX2):
	rgbToRgbaKernel31_Asm_X86_AVX2 0, 1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbToRgbaKernel31_Asm_X86_Aligned10_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgb, uint8_t* rgba, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
sym(rgbToRgbaKernel31_Asm_X86_Aligned10_AVX2):
	rgbToRgbaKernel31_Asm_X86_AVX2 1, 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbToRgbaKernel31_Asm_X86_Aligned11_AVX2(COMV_ALIGNED(AVX2) const uint8_t* rgb, COMV_ALIGNED(AVX2) uint8_t* rgba, vcomp_scalar_t height, vcomp_scalar_t width, vcomp_scalar_t stride)
sym(rgbToRgbaKernel31_Asm_X86_Aligned11_AVX2):
	rgbToRgbaKernel31_Asm_X86_AVX2 1, 1

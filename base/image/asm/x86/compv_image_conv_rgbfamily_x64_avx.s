;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "../../../asm/x86/compv_common_x86.s"
%if COMPV_YASM_ABI_IS_64BIT

%include "compv_image_conv_macros.s"

COMPV_YASM_DEFAULT_REL

%define rgb24Family		0
%define rgb32Family		1

global sym(CompVImageConvRgb24family_to_y_Asm_X64_AVX2)
global sym(CompVImageConvRgb32family_to_y_Asm_X64_AVX2)
global sym(CompVImageConvRgb24family_to_uv_planar_11_Asm_X64_AVX2)
global sym(CompVImageConvRgb32family_to_uv_planar_11_Asm_X64_AVX2)

section .data
	extern sym(k16_i16)
	extern sym(k128_i16)
	extern sym(kAVXPermutevar8x32_ABCDDEFG_i32)
	extern sym(kAVXPermutevar8x32_AEBFCGDH_i32)
	extern sym(kShuffleEpi8_RgbToRgba_i32)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(AVX) const uint8_t* rgbPtr
; arg(1) -> COMPV_ALIGNED(AVX) uint8_t* outYPtr
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> COMPV_ALIGNED(AVX) compv_uscalar_t stride,
; arg(5) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_YCoeffs8
; %1 -> family: rgb24Family or rgb32Family
%macro CompVImageConvRgbfamily_to_y_Macro_X64_AVX2 1
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	COMPV_YASM_SAVE_YMM 10 ;YMM[6-n]
	; end prolog

	mov rax, arg(2)
	add rax, 31
	and rax, -32
	mov rcx, arg(4)
	sub rcx, rax ; rcx = padY
	mov rdx, rcx
	%if %1 == rgb32Family
		shl rdx, 2 ; rdx = padRGBA
	%elif %1 == rgb24Family
		imul rdx, 3 ; rdx = padRGB
	%else
		%error 'Not implemented'
	%endif

	mov rax, arg(5)
	vmovdqa ymm0, [rax] ; ymmYCoeffs
	vmovdqa ymm1, [sym(k16_i16)] ; ymm16
	vmovdqa ymm6, [sym(kAVXPermutevar8x32_AEBFCGDH_i32)] ; ymmAEBFCGDH
	vmovdqa ymm7, [sym(kShuffleEpi8_RgbToRgba_i32)] ; ymmMaskRgbToRgba
	vmovdqa ymm8, [sym(kAVXPermutevar8x32_ABCDDEFG_i32)] ; ymmABCDDEFG
	;vmovdqa ymm9, [sym(kAVXPermutevar8x32_CDEFFGHX_i32)] ; ymmCDEFFGHX
	;vmovdqa ymm10, [sym(kAVXPermutevar8x32_XXABBCDE_i32)] ; ymmXXABBCDE

	mov rax, arg(0) ; rgbPtr
	mov r8, arg(3) ; height
	mov r10, arg(1) ; outYPtr
	mov r11, arg(2) ; width

	.LoopHeight:
		xor r9, r9
		.LoopWidth:
			%if %1 == rgb32Family
				vmovdqa ymm2, [rax + 0]
				vmovdqa ymm3, [rax + 32]
				vmovdqa ymm4, [rax + 64]
				vmovdqa ymm5, [rax + 96]
				lea rax, [rax + 128] ; rgb32Ptr += 128
			%elif %1 == rgb24Family
				; Convert RGB24 -> RGBA32, alpha channel contains zeros	
				COMPV_32xRGB_TO_32xRGBA_AVX2 rax, ymm2, ymm3, ymm4, ymm5, ymm8, ymm7
				lea rax, [rax + 96] ; rgb24Ptr += 128
			%else
				%error 'Not implemented'
			%endif
			vpmaddubsw ymm2, ymm2, ymm0
			vpmaddubsw ymm3, ymm3, ymm0
			vpmaddubsw ymm4, ymm4, ymm0
			vpmaddubsw ymm5, ymm5, ymm0
			vphaddw ymm2, ymm2, ymm3
			vphaddw ymm4, ymm4, ymm5
			vpsraw ymm2, ymm2, 7 ; >> 7
			vpsraw ymm4, ymm4, 7 ; >> 7
			vpaddw ymm2, ymm2, ymm1 ; + 16
			vpaddw ymm4, ymm4, ymm1 ; + 16
			lea r9, [r9 + 32] ; i += 32
			cmp r9, r11 ; (i < width)?
			vpackuswb ymm2, ymm2, ymm4 ; Saturate(I16 -> U8): packus(ACBD, EGFH) -> AEBFCGDH
			vpermd ymm2, ymm6, ymm2
			vmovdqa [r10], ymm2
			lea r10, [r10 + 32]
			; end-of-LoopWidth
			jl .LoopWidth
	lea r10, [r10 + rcx]
	lea rax, [rax + rdx]
	; end-of-LoopHeight
	dec r8
	jnz .LoopHeight
	
	; begin epilog
	COMPV_YASM_RESTORE_YMM
    COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb24family_to_y_Asm_X64_AVX2)
	CompVImageConvRgbfamily_to_y_Macro_X64_AVX2 rgb24Family

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb32family_to_y_Asm_X64_AVX2)
	CompVImageConvRgbfamily_to_y_Macro_X64_AVX2 rgb32Family

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(AVX) const uint8_t* rgbPtr
; arg(1) -> COMPV_ALIGNED(AVX) uint8_t* outUPtr
; arg(2) -> COMPV_ALIGNED(AVX) uint8_t* outVPtr
; arg(3) -> compv_uscalar_t width
; arg(4) -> compv_uscalar_t height
; arg(5) -> COMPV_ALIGNED(AVX) compv_uscalar_t stride
; arg(6) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_UCoeffs8
; arg(7) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_VCoeffs8
; %1 -> family: rgb24Family or rgb32Family
%macro CompVImageConvRgbfamily_to_uv_planar_11_Macro_X64_AVX2 1
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_YMM 13
	; end prolog
	push r12

	mov rdx, arg(3)
	lea rdx, [rdx + 31]
	and rdx, -32
	mov rcx, arg(5)
	sub rcx, rdx
	mov r11, rcx ; r11 = padUV
	%if %1 == rgb32Family
		shl rcx, 2 ; rcx = padRGBA
	%elif %1 == rgb24Family
		imul rcx, 3 ; rcx = padRGB
	%else
		%error 'Not implemented'
	%endif

	mov rax, arg(6)
	vmovdqa ymm8, [rax] ; ymm8 = ymmUCoeffs
	mov rax, arg(7)
	vmovdqa ymm9, [rax] ; ymm9 = ymmVCoeffs

	vmovdqa ymm10, [sym(kAVXPermutevar8x32_AEBFCGDH_i32)] ; ymmAEBFCGDH
	vmovdqa ymm11, [sym(kShuffleEpi8_RgbToRgba_i32)] ; ymmMaskRgbToRgba
	vmovdqa ymm12, [sym(kAVXPermutevar8x32_ABCDDEFG_i32)] ; ymmABCDDEFG
	vmovdqa ymm13, [sym(k128_i16)] ; ymm128
		
	mov rax, arg(0) ; rax = rgbPtr
	mov r8, arg(4) ; r8 = height
	mov r10, arg(1) ; r10 = outUPtr
	mov rdx, arg(2) ; rdx = outVPtr
	mov r12, arg(3) ; r12 = width
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		xor r9, r9
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width; i += 32)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopWidth:
			%if %1 == rgb32Family
				vmovdqa ymm0, [rax + 0]
				vmovdqa ymm1, [rax + 32]
				vmovdqa ymm2, [rax + 64]
				vmovdqa ymm3, [rax + 96]
				lea rax, [rax + 128] ; rgb32Ptr += 128
			%elif %1 == rgb24Family
				; Convert RGB -> RGBA, alpha channel contains zeros	
				COMPV_32xRGB_TO_32xRGBA_AVX2 rax, ymm0, ymm1, ymm2, ymm3, ymm12, ymm11
				lea rax, [rax + 96] ; rgb24Ptr += 96
			%else
				%error 'Not implemented'
			%endif
			lea r9, [r9 + 32] ; i += 32
			vpmaddubsw ymm4, ymm0, ymm9
			vpmaddubsw ymm5, ymm1, ymm9
			vpmaddubsw ymm6, ymm2, ymm9
			vpmaddubsw ymm7, ymm3, ymm9
			vpmaddubsw ymm0, ymm0, ymm8
			vpmaddubsw ymm1, ymm1, ymm8
			vpmaddubsw ymm2, ymm2, ymm8
			vpmaddubsw ymm3, ymm3, ymm8
			cmp r9, r12 ; (i < width)?
			vphaddw ymm4, ymm4, ymm5
			vphaddw ymm6, ymm6, ymm7
			vphaddw ymm0, ymm0, ymm1
			vphaddw ymm2, ymm2, ymm3
			vpsraw ymm4, ymm4, 8
			vpsraw ymm6, ymm6, 8
			vpsraw ymm0, ymm0, 8
			vpsraw ymm2, ymm2, 8
			vpaddw ymm4, ymm4, ymm13
			vpaddw ymm6, ymm6, ymm13
			vpaddw ymm0, ymm0, ymm13
			vpaddw ymm2, ymm2, ymm13
			vpackuswb ymm4, ymm4, ymm6
			vpackuswb ymm0, ymm0, ymm2
			vpermd ymm4, ymm10, ymm4
			vpermd ymm0, ymm10, ymm0
			vmovdqa [rdx], ymm4
			lea rdx, [rdx + 32] ; outVPtr += 32
			vmovdqa [r10], ymm0
			lea r10, [r10 + 32] ; outUPtr += 32
			; end-of-LoopWidth
			jl .LoopWidth

		lea r10, [r10 + r11] ; outUPtr += padUV
		lea rdx, [rdx + r11] ; outUPtr += padUV
		lea rax, [rax + rcx] ; rgbPtr += padRGB
		; end-of-LoopHeight
		dec r8
		jnz .LoopHeight

	; begin epilog
	pop r12
	COMPV_YASM_RESTORE_YMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb24family_to_uv_planar_11_Asm_X64_AVX2)
	CompVImageConvRgbfamily_to_uv_planar_11_Macro_X64_AVX2 rgb24Family

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb32family_to_uv_planar_11_Asm_X64_AVX2)
	CompVImageConvRgbfamily_to_uv_planar_11_Macro_X64_AVX2 rgb32Family



%undef rgb24Family
%undef rgb32Family

%endif ; COMPV_YASM_ABI_IS_64BIT
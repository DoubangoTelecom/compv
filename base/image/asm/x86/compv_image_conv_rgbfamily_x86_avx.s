;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "../../../asm/x86/compv_common_x86.s"
%include "compv_image_conv_macros.s"

COMPV_YASM_DEFAULT_REL

%define rgb24Family		0
%define rgb32Family		1

global sym(CompVImageConvRgb24family_to_y_Asm_X86_AVX2)
global sym(CompVImageConvRgb32family_to_y_Asm_X86_AVX2)
global sym(CompVImageConvRgb24family_to_uv_planar_11_Asm_X86_AVX2)
global sym(CompVImageConvRgb32family_to_uv_planar_11_Asm_X86_AVX2)

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
%macro CompVImageConvRgbfamily_to_y_Macro_X86_AVX2 1
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	COMPV_YASM_SAVE_YMM 7 ;YMM[6-n]
	push rsi
	push rdi
	push rbx
	; end prolog

	mov rax, arg(2)
	lea rax, [rax + 31]
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
	vmovdqa ymm7, [sym(kAVXPermutevar8x32_ABCDDEFG_i32)] ; ymmABCDDEFG

	mov rax, arg(0) ; rgbPtr
	mov rsi, arg(3) ; height
	mov rbx, arg(1) ; outYPtr

	.LoopHeight:
		xor rdi, rdi
		.LoopWidth:
			%if %1 == rgb32Family
				vmovdqa ymm2, [rax + 0]
				vmovdqa ymm3, [rax + 32]
				vmovdqa ymm4, [rax + 64]
				vmovdqa ymm5, [rax + 96]
				lea rax, [rax + 128] ; rgb32Ptr += 128
			%elif %1 == rgb24Family
				; Convert RGB -> RGBA, alpha channel contains zeros
				COMPV_32xRGB_TO_32xRGBA_AVX2 rax, ymm2, ymm3, ymm4, ymm5, ymm7, [sym(kShuffleEpi8_RgbToRgba_i32)]
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
			lea rdi, [rdi + 32] ; i += 32
			cmp rdi, arg(2) ; (i < width)?
			vpackuswb ymm2, ymm2, ymm4 ; Saturate(I16 -> U8): packus(ACBD, EGFH) -> AEBFCGDH
			vpermd ymm2, ymm6, ymm2
			vmovdqa [rbx], ymm2
			lea rbx, [rbx + 32]
			; end-of-LoopWidth
			jl .LoopWidth
	lea rbx, [rbx + rcx]
	lea rax, [rax + rdx]
	; end-of-LoopHeight
	dec rsi
	jnz .LoopHeight
	
	; begin epilog
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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb24family_to_y_Asm_X86_AVX2)
	CompVImageConvRgbfamily_to_y_Macro_X86_AVX2 rgb24Family

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb32family_to_y_Asm_X86_AVX2)
	CompVImageConvRgbfamily_to_y_Macro_X86_AVX2 rgb32Family
	

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
%macro CompVImageConvRgbfamily_to_uv_planar_11_Macro_X86_AVX2 1
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_YMM 7
	push rsi
	push rdi
	push rbx
	; end prolog

	; alloc memory
	lea rsp, [rsp - COMPV_YASM_REG_SZ_BYTES]
	%define padUV		[rsp + 0]

	mov rdx, arg(3)
	lea rdx, [rdx + 31]
	and rdx, -32
	mov rcx, arg(5)
	sub rcx, rdx
	mov padUV, rcx
	%if %1 == rgb32Family
		shl rcx, 2 ; rcx = padRGBA
	%elif %1 == rgb24Family
		imul rcx, 3 ; rcx = padRGB
	%else
		%error 'Not implemented'
	%endif

	mov rax, arg(6)
	vmovdqa ymm7, [rax] ; ymm7 = ymmUCoeffs
	mov rax, arg(7)
	vmovdqa ymm6, [rax] ; ymm6 = ymmVCoeffs
		
	mov rax, arg(0) ; rax = rgbPtr
	mov rsi, arg(4) ; rsi = height
	mov rbx, arg(1) ; rbx = outUPtr
	mov rdx, arg(2) ; rdx = outVPtr
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		xor rdi, rdi
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
				; Convert RGB24 -> RGBA32, alpha channel contains zeros
				vmovdqa ymm4, [sym(kAVXPermutevar8x32_ABCDDEFG_i32)] ; ymmABCDDEFG
				COMPV_32xRGB_TO_32xRGBA_AVX2 rax, ymm0, ymm1, ymm2, ymm3, ymm4, [sym(kShuffleEpi8_RgbToRgba_i32)]
				lea rax, [rax + 96] ; rgb24Ptr += 96
			%else
				%error 'Not implemented'
			%endif
			lea rdi, [rdi + 32] ; i += 32
			vpmaddubsw ymm4, ymm0, ymm6
			vpmaddubsw ymm5, ymm1, ymm6
			vpmaddubsw ymm0, ymm0, ymm7
			vpmaddubsw ymm1, ymm1, ymm7
			vphaddw ymm4, ymm4, ymm5
			vphaddw ymm0, ymm0, ymm1
			vpsraw ymm4, ymm4, 8
			vpsraw ymm0, ymm0, 8
			vpaddw ymm4, ymm4, [sym(k128_i16)]
			vpaddw ymm0, ymm0, [sym(k128_i16)]
			cmp rdi, arg(3) ; (i < width)?
			vpmaddubsw ymm1, ymm2, ymm6
			vpmaddubsw ymm5, ymm3, ymm6
			vpmaddubsw ymm2, ymm2, ymm7
			vpmaddubsw ymm3, ymm3, ymm7
			vphaddw ymm1, ymm1, ymm5
			vphaddw ymm2, ymm2, ymm3
			vmovdqa ymm5, [sym(kAVXPermutevar8x32_AEBFCGDH_i32)]
			vpsraw ymm1, ymm1, 8
			vpsraw ymm2, ymm2, 8
			vpaddw ymm1, ymm1, [sym(k128_i16)]
			vpaddw ymm2, ymm2, [sym(k128_i16)]
			vpackuswb ymm4, ymm4, ymm1
			vpackuswb ymm0, ymm0, ymm2
			vpermd ymm0, ymm5, ymm0
			vpermd ymm4, ymm5, ymm4
			vmovdqa [rbx], ymm0
			vmovdqa [rdx], ymm4
			lea rbx, [rbx + 32] ; outUPtr += 32
			lea rdx, [rdx + 32] ; outVPtr += 32
			; end-of-LoopWidth
			jl .LoopWidth

		add rbx, padUV	; outUPtr += padUV
		add rdx, padUV	; outUPtr += padUV
		lea rax, [rax + rcx] ; rgbPtr += padRGB
		; end-of-LoopHeight
		dec rsi
		jnz .LoopHeight

	; free memory
	lea rsp, [rsp + COMPV_YASM_REG_SZ_BYTES]
	%undef padUV

	; begin epilog
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
sym(CompVImageConvRgb24family_to_uv_planar_11_Asm_X86_AVX2)
	CompVImageConvRgbfamily_to_uv_planar_11_Macro_X86_AVX2 rgb24Family

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb32family_to_uv_planar_11_Asm_X86_AVX2)
	CompVImageConvRgbfamily_to_uv_planar_11_Macro_X86_AVX2 rgb32Family


%undef rgb24Family
%undef rgb32Family
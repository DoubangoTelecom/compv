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

global sym(CompVImageConvRgb24family_to_y_Asm_X86_AVX2)

section .data
	extern sym(k16_i16)
	extern sym(k128_i16)
	extern sym(kAVXPermutevar8x32_ABCDDEFG_i32)
	extern sym(kAVXPermutevar8x32_XXABBCDE_i32)
	extern sym(kAVXPermutevar8x32_CDEFFGHX_i32)
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
sym(CompVImageConvRgb24family_to_y_Asm_X86_AVX2)
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	COMPV_YASM_SAVE_YMM 6 ;XMM[6-n]
	push rsi
	push rdi
	push rbx
	; end prolog

	mov rax, arg(2)
	add rax, 31
	and rax, -32
	mov rcx, arg(4)
	sub rcx, rax ; rcx = padY
	mov rdx, rcx
	imul rdx, 3 ; rdx = padRGB

	mov rax, arg(5)
	vmovdqa ymm0, [rax] ; ymmYCoeffs
	vmovdqa ymm1, [sym(k16_i16)] ; ymm16
	vmovdqa ymm6, [sym(kAVXPermutevar8x32_AEBFCGDH_i32)] ; ymmAEBFCGDH

	mov rax, arg(0) ; rgbPtr
	mov rsi, arg(3) ; height
	mov rbx, arg(1) ; outYPtr

	.LoopHeight:
		xor rdi, rdi
		.LoopWidth:
			; Convert RGB -> RGBA, alpha channel contains garbage (later multiplied with zero coeff)
			COMPV_32xRGB_TO_32xRGBA_AVX2 rax, ymm2, ymm3, ymm4, ymm5 ;  COMPV_32xRGB_TO_32xRGBA_AVX2(rgbPtr, rgbaPtr[0], gbaPtr[1], gbaPtr[2], gbaPtr[3])			
			
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
			lea rax, [rax + 96]

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
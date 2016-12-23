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

global sym(CompVImageConvRgb24family_to_y_Asm_X86_SSSE3)

section .data
	extern sym(k16_i16)
	extern sym(kShuffleEpi8_RgbToRgba_i32)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* rgbPtr
; arg(1) -> COMPV_ALIGNED(SSE) uint8_t* outYPtr
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
; arg(5) -> COMPV_ALIGNED(SSE) const int8_t* kRGBfamilyToYUV_YCoeffs8
; void CompVImageConvRgb24family_to_y_Asm_X86_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgbPtr, COMPV_ALIGNED(SSE) uint8_t* outYPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride, COMPV_ALIGNED(SSE) const int8_t* kRGBfamilyToYUV_YCoeffs8)
sym(CompVImageConvRgb24family_to_y_Asm_X86_SSSE3)
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	COMPV_YASM_SAVE_XMM 7
	push rsi
	push rdi
	push rbx
	; end prolog

	; align stack and malloc memory
	COMPV_YASM_ALIGN_STACK 16, rax
	sub rsp, 16+16+16+16 ; rgba[4] = [rsp + 0], [rsp + 16], ...

	mov rdx, arg(2)
	add rdx, 15
	and rdx, -16
	mov rcx, arg(4)
	sub rcx, rdx ; rcx = padY
	mov rdx, rcx 
	imul rdx, 3 ; rdx = padRGB

	mov rax, arg(5)
	movdqa xmm0, [rax] ; xmm0 = xmmYCoeffs
	movdqa xmm1, [sym(k16_i16)] ; xmm1 = xmm16
		
	mov rax, arg(0) ; rax = rgbPtr
	mov rsi, arg(3) ; rsi = height
	mov rbx, arg(1) ; rbx = outYPtr
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		xor rdi, rdi
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopWidth:
			; Convert RGB -> RGBA
			; This macro modify [xmm4 - xmm7]
			COMPV_3RGB_TO_4RGBA_SSSE3 rax, rsp, 1, 1 ; COMPV_3RGB_TO_4RGBA_SSSE3(rgbPtr, rgbaPtr, rgbPtrIsAligned, rgbaPtrIsAligned)

			movdqa xmm2, [rsp] ; 4 RGBA samples
			movdqa xmm3, [rsp + 16] ; 4 RGBA samples
			movdqa xmm4, [rsp + 32] ; 4 RGBA samples
			movdqa xmm5, [rsp + 48] ; 4 RGBA samples

			pmaddubsw xmm2, xmm0
			pmaddubsw xmm3, xmm0
			pmaddubsw xmm4, xmm0
			pmaddubsw xmm5, xmm0

			phaddw xmm2, xmm3
			phaddw xmm4, xmm5

			lea rdi, [rdi + 16] ; i += 16
			cmp rdi, arg(2) ; (i < width)?
			
			psraw xmm2, 7
			psraw xmm4, 7
			
			paddw xmm2, xmm1
			paddw xmm4, xmm1

			lea rax, [rax + 48] ; rgbPtr += 48
						
			packuswb xmm2, xmm4
			movdqa [rbx], xmm2

			lea rbx, [rbx + 16] ; outYPtr += 16

			; end-of-LoopWidth
			jl .LoopWidth

		add rbx, rcx
		add rax, rdx
		; end-of-LoopHeight
		dec rsi
		jnz .LoopHeight

	; unalign stack and alloc memory
	add rsp, 16+16+16+16
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

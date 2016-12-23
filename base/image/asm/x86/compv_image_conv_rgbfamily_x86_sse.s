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
global sym(CompVImageConvRgb24family_to_uv_planar_11_Asm_X86_SSSE3)

section .data
	extern sym(k16_i16)
	extern sym(k128_i16)
	extern sym(kShuffleEpi8_RgbToRgba_i32)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* rgbPtr
; arg(1) -> COMPV_ALIGNED(SSE) uint8_t* outYPtr
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
; arg(5) -> COMPV_ALIGNED(SSE) const int8_t* kRGBfamilyToYUV_YCoeffs8
sym(CompVImageConvRgb24family_to_y_Asm_X86_SSSE3)
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	push rsi
	push rdi
	push rbx
	; end prolog

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
			COMPV_3RGB_TO_4RGBA_SSSE3 rax, xmm2, xmm3, xmm4, xmm5 ; COMPV_3RGB_TO_4RGBA_SSSE3(rgbPtr, rgbaPtr[0], rgbaPtr[1], rgbaPtr[2], rgbaPtr[3])

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

		lea rbx, [rbx + rcx]
		lea rax, [rax + rdx]
		; end-of-LoopHeight
		dec rsi
		jnz .LoopHeight

	; begin epilog
	pop rbx
	pop rdi
	pop rsi
    COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* rgbPtr
; arg(1) -> COMPV_ALIGNED(SSE) uint8_t* outUPtr
; arg(2) -> COMPV_ALIGNED(SSE) uint8_t* outVPtr
; arg(3) -> compv_uscalar_t width
; arg(4) -> compv_uscalar_t height
; arg(5) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
; arg(6) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_UCoeffs8
; arg(7) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_VCoeffs8
sym(CompVImageConvRgb24family_to_uv_planar_11_Asm_X86_SSSE3)
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_XMM 7
	push rsi
	push rdi
	push rbx
	; end prolog

	; alloc memory
	lea rsp, [rsp - COMPV_YASM_REG_SZ_BYTES]
	%define padUV		[rsp + 0]

	mov rdx, arg(3)
	lea rdx, [rdx + 15]
	and rdx, -16
	mov rcx, arg(5)
	sub rcx, rdx
	mov padUV, rcx
	imul rcx, 3 ; rcx = padRGB

	mov rax, arg(6)
	movdqa xmm7, [rax] ; xmm7 = xmmUCoeffs
	mov rax, arg(7)
	movdqa xmm6, [rax] ; xmm6 = xmmVCoeffs
		
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
		; for (i = 0; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopWidth:
			; Convert RGB -> RGBA
			COMPV_3RGB_TO_4RGBA_SSSE3 rax, xmm0, xmm1, xmm2, xmm3 ; COMPV_3RGB_TO_4RGBA_SSSE3(rgbPtr, rgbaPtr[0], rgbaPtr[1], rgbaPtr[2], rgbaPtr[3])

			lea rdi, [rdi + 16] ; i += 16

			movdqa xmm4, xmm0
			movdqa xmm5, xmm1
			pmaddubsw xmm0, xmm7
			pmaddubsw xmm1, xmm7
			pmaddubsw xmm4, xmm6
			pmaddubsw xmm5, xmm6
			phaddw xmm0, xmm1
			phaddw xmm4, xmm5
			movdqa xmm1, xmm2
			movdqa xmm5, xmm3
			psraw xmm0, 8
			psraw xmm4, 8
			paddw xmm0, [sym(k128_i16)]
			paddw xmm4, [sym(k128_i16)]
			
			cmp rdi, arg(3) ; (i < width)?		

			pmaddubsw xmm2, xmm7
			pmaddubsw xmm3, xmm7
			pmaddubsw xmm1, xmm6
			pmaddubsw xmm5, xmm6
			phaddw xmm2, xmm3
			phaddw xmm1, xmm5
			psraw xmm2, 8
			psraw xmm1, 8
			paddw xmm2, [sym(k128_i16)]
			paddw xmm1, [sym(k128_i16)]

			packuswb xmm0, xmm2
			packuswb xmm4, xmm1
			movdqa [rbx], xmm0
			movdqa [rdx], xmm4
			
			lea rax, [rax + 48] ; rgbPtr += 48
			lea rbx, [rbx + 16] ; outUPtr += 16
			lea rdx, [rdx + 16] ; outVPtr += 16

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
	COMPV_YASM_RESTORE_XMM
    COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
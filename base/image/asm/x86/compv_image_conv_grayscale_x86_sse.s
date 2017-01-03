;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "../../../asm/x86/compv_common_x86.s"
%include "compv_image_conv_macros.s"

COMPV_YASM_DEFAULT_REL

%define yuyv422Family 0
%define uyvy422Family 1

global sym(CompVImageConvYuyv422_to_y_Asm_X86_SSSE3)
global sym(CompVImageConvUyvy422_to_y_Asm_X86_SSSE3)


section .data
	extern sym(kShuffleEpi8_Yuyv422ToYuv_i32)
	extern sym(kShuffleEpi8_Uyvy422ToYuv_i32)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* yuv422Ptr
; arg(1) -> COMPV_ALIGNED(SSE) uint8_t* outYPtr
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
; %1 -> yuv422Family: yuyv422Family or uyvy422Family
%macro CompVImageConvYuv422family_to_y_Macro_X86_SSSE3 1
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	mov rdx, arg(2)
	lea rdx, [rdx + 15]
	and rdx, -16
	mov rcx, arg(4)
	sub rcx, rdx ; rcx = padY
	mov rdx, rcx
	shl rdx, 1 ; rdx = padYUV

	mov rax, arg(0) ; rax = yuv422Ptr
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*0]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*1]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*2]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]

	; rdi = i
	mov rbx, arg(1) ; rbx = outYPtr
	mov rsi, arg(3) ; rsi = height

	%if %1 == yuyv422Family
		movdqa xmm3, [sym(kShuffleEpi8_Yuyv422ToYuv_i32)] ; xmm3 = xmmMask
	%elif %1 == uyvy422Family
		movdqa xmm3, [sym(kShuffleEpi8_Uyvy422ToYuv_i32)] ; xmm3 = xmmMask
	%else
		%error 'Not implemented'
	%endif

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		xor rdi, rdi
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopWidth:
			prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]
			movdqa xmm0, [rax + 0]
			movdqa xmm1, [rax + 16]
			pshufb xmm0, xmm3
			pshufb xmm1, xmm3
			lea rax, [rax + 32] ; yuv422Ptr += 32
			lea rdi, [rdi + 16] ; i += 16
			punpcklqdq xmm0, xmm1
			cmp rdi, arg(2) ; (i < width)?
			movdqa [rbx], xmm0
			lea rbx, [rbx + 16] ; outYPtr += 16
			; end-of-LoopWidth
			jl .LoopWidth

		lea rbx, [rbx + rcx]
		lea rax, [rax + rdx]
		; end-of-LoopHeight
		dec rsi
		jnz .LoopHeight

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvYuyv422_to_y_Asm_X86_SSSE3)
	CompVImageConvYuv422family_to_y_Macro_X86_SSSE3 yuyv422Family

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvUyvy422_to_y_Asm_X86_SSSE3)
	CompVImageConvYuv422family_to_y_Macro_X86_SSSE3 uyvy422Family

%undef yuyv422Family
%undef uyvy422Family
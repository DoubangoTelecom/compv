;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "../../compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(rgbToRgbaKernel31_Asm_X86_Aligned00_SSSE3)
global sym(rgbToRgbaKernel31_Asm_X86_Aligned01_SSSE3)
global sym(rgbToRgbaKernel31_Asm_X86_Aligned10_SSSE3)
global sym(rgbToRgbaKernel31_Asm_X86_Aligned11_SSSE3)

section .data
	extern sym(k_0_0_0_255_u8)
	extern sym(kShuffleEpi8_RgbToRgba_i32)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* rgb, 
; arg(1) -> uint8_t* rgba
; arg(2) -> compv_scalar_t height
; arg(3) -> compv_scalar_t width
; arg(4) -> compv_scalar_t stride
; %1 -> 1: rgb aligned, 0: rgb not aligned
; %2 -> 1: rgba aligned, 0: rgba not aligned
%macro rgbToRgbaKernel31_Asm_X86_SSSE3 2
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	push rsi
	push rdi
	push rbx
	; end prolog

	mov rax, arg(3)
	add rax, 15
	and rax, -16
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

	movdqa xmm4, [sym(kShuffleEpi8_RgbToRgba_i32)] ; xmmMaskRgbToRgba
	movdqa xmm5, [sym(k_0_0_0_255_u8)] ; xmmAlpha

	.LoopHeight:
		xor rdi, rdi
		.LoopWidth:
			%if %1==1
			movdqa xmm0, [rax + 0]
			movdqa xmm1, [rax + 16]
			%else
			movdqu xmm0, [rax + 0]
			movdqu xmm1, [rax + 16]
			%endif
			movdqa xmm2, xmm0
			movdqa xmm3, xmm1

			pshufb xmm0, xmm4
			paddb xmm0, xmm5
			%if %2==1
			movdqa [rdx + 0], xmm0
			%else
			movdqu [rdx + 0], xmm0
			%endif

			palignr xmm3, xmm2, 12
			pshufb xmm3, xmm4
			paddb xmm3, xmm5
			%if %2==1
			movdqa [rdx + 16], xmm3
			%else
			movdqu [rdx + 16], xmm3
			%endif

			movdqa xmm0, [rax + 32]
			movdqa xmm2, xmm0
			palignr xmm0, xmm1, 8
			pshufb xmm0, xmm4
			paddb xmm0, xmm5
			%if %2==1
			movdqa [rdx + 32], xmm0
			%else
			movdqu [rdx + 32], xmm0
			%endif

			palignr xmm2, xmm2, 4
			pshufb xmm2, xmm4
			paddb xmm2, xmm5
			%if %2==1
			movdqa [rdx + 48], xmm2
			%else
			movdqu [rdx + 48], xmm2
			%endif
			
			add rax, 48 ; rgb += 48
			add rdx, 64 ; rgba += 64

			; end-of-LoopWidth
			add rdi, 16
			cmp rdi, arg(3)
			jl .LoopWidth
	add rax, rsi ; rgb += padRGB
	add rdx, rbx ; rgba += padRGBA
	; end-of-LoopHeight
	dec rcx
	jnz .LoopHeight

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
;; void rgbToRgbaKernel31_Asm_X86_Aligned00_SSSE3(const uint8_t* rgb, uint8_t* rgba, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride)
sym(rgbToRgbaKernel31_Asm_X86_Aligned00_SSSE3):
	rgbToRgbaKernel31_Asm_X86_SSSE3 0, 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; void rgbToRgbaKernel31_Asm_X86_Aligned01_SSSE3(const uint8_t* rgb, COMPV_ALIGNED(SSE) uint8_t* rgba, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride)
sym(rgbToRgbaKernel31_Asm_X86_Aligned01_SSSE3):
	rgbToRgbaKernel31_Asm_X86_SSSE3 0, 1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; void rgbToRgbaKernel31_Asm_X86_Aligned10_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgb, uint8_t* rgba, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride)
sym(rgbToRgbaKernel31_Asm_X86_Aligned10_SSSE3):
	rgbToRgbaKernel31_Asm_X86_SSSE3 1, 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; void rgbToRgbaKernel31_Asm_X86_Aligned11_SSSE3(COMPV_ALIGNED(SSE) const uint8_t* rgb, COMPV_ALIGNED(SSE) uint8_t* rgba, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride)
sym(rgbToRgbaKernel31_Asm_X86_Aligned11_SSSE3):
	rgbToRgbaKernel31_Asm_X86_SSSE3 1, 1
	

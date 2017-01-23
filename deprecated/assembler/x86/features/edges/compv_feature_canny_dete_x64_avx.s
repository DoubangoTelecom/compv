;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "../../compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(CannyNMSApply_Asm_X64_AVX2)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(AVX) uint16_t* grad
; arg(1) -> COMPV_ALIGNED(AVX) uint8_t* nms
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> COMPV_ALIGNED(AVX) compv_uscalar_t stride
sym(CannyNMSApply_Asm_X64_AVX2):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	push rdi
	;; end prolog ;;
	
	vpxor ymm0, ymm0

	mov rax, arg(0) ; rax = grad
	mov rdx, arg(1) ; rdx = nms
	mov r10, arg(2) ; r10 = width
	mov rcx, arg(3) ; rcx = height
	dec rcx ; row start at 1
	mov r8, arg(4) ; r8 = stride
	lea r9, [r8*2] ; r9 = strideInBytesTimes2

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (row_ = 1; row_ < height; ++row_)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopRows		
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (col_ = 0; col_ < width; col_ += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor rdi, rdi
		.LoopCols
			vpcmpeqb xmm1, xmm0, [rdx + rdi]
			vpmovmskb r11, xmm1
			xor r11, 0xffff
			lea rdi, [rdi + 16]
			jz .AllZeros
				vpunpckhbw xmm2, xmm1, xmm1
				vpunpcklbw xmm1, xmm1, xmm1
				vinsertf128 ymm1, ymm1, xmm2, 1
				vmovdqa [rdx + rdi - 16], xmm0
				vpand ymm1, [rax + rdi*2 - 32]
				vmovdqa [rax + rdi*2 - 32], ymm1
			.AllZeros
			cmp rdi, r10
			jl .LoopCols

		dec rcx
		lea rdx, [rdx + r8]
		lea rax, [rax + r9]
		jnz .LoopRows

	;; begin epilog ;;
	pop rdi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret

%endif ; COMPV_YASM_ABI_IS_64BIT
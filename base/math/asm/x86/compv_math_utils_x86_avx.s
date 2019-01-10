;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(CompVMathUtilsSumAbs_16s16u_Asm_X86_AVX2)


section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const COMPV_ALIGNED(AVX) int16_t* a
; arg(1) -> const COMPV_ALIGNED(AVX) int16_t* b
; arg(2) -> COMPV_ALIGNED(AVX) uint16_t* r
; arg(3) -> compv_uscalar_t width
; arg(4) -> compv_uscalar_t height
; arg(5) -> COMPV_ALIGNED(AVX) compv_uscalar_t stride
sym(CompVMathUtilsSumAbs_16s16u_Asm_X86_AVX2):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	COMPV_YASM_SAVE_YMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	mov rcx, arg(5)
	mov rax, arg(0) ; rax = a
	mov rdx, arg(1) ; rdx = b
	lea rcx, [rcx * COMPV_YASM_INT16_SZ_BYTES]
	mov rbx, arg(2) ; rbx = r
	mov rdi, arg(3)
	mov rsi, arg(4) ; rsi = height
	lea rdi, [rdi - 63] ; rdi = width - 63
	mov arg(5), rcx ; strideInBytes

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopRows
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width - 63; i += 64)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor rcx, rcx ; rcx = i = 0
		test rdi, rdi
		jle .EndOfLoopCols64
		.LoopCols64
			vpabsw ymm0, [rax + (rcx + 0)*COMPV_YASM_INT16_SZ_BYTES]
			vpabsw ymm1, [rax + (rcx + 16)*COMPV_YASM_INT16_SZ_BYTES]
			vpabsw ymm2, [rax + (rcx + 32)*COMPV_YASM_INT16_SZ_BYTES]
			vpabsw ymm3, [rax + (rcx + 48)*COMPV_YASM_INT16_SZ_BYTES]
			vpabsw ymm4, [rdx + (rcx + 0)*COMPV_YASM_INT16_SZ_BYTES]
			vpabsw ymm5, [rdx + (rcx + 16)*COMPV_YASM_INT16_SZ_BYTES]
			vpabsw ymm6, [rdx + (rcx + 32)*COMPV_YASM_INT16_SZ_BYTES]
			vpabsw ymm7, [rdx + (rcx + 48)*COMPV_YASM_INT16_SZ_BYTES]
			add rcx, 64
			vpaddusw ymm0, ymm4
			vpaddusw ymm1, ymm5
			vpaddusw ymm2, ymm6
			vpaddusw ymm3, ymm7
			cmp rcx, rdi
			vmovdqa [rbx + (rcx - 64)*COMPV_YASM_INT16_SZ_BYTES], ymm0
			vmovdqa [rbx + (rcx - 48)*COMPV_YASM_INT16_SZ_BYTES], ymm1
			vmovdqa [rbx + (rcx - 32)*COMPV_YASM_INT16_SZ_BYTES], ymm2
			vmovdqa [rbx + (rcx - 16)*COMPV_YASM_INT16_SZ_BYTES], ymm3
			jl .LoopCols64
		.EndOfLoopCols64

		add rdi, 63

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp rcx, rdi
		jge .EndOfLoopCols16
		.LoopCols16
			vpabsw ymm0, [rax + (rcx + 0)*COMPV_YASM_INT16_SZ_BYTES]
			vpabsw ymm1, [rdx + (rcx + 0)*COMPV_YASM_INT16_SZ_BYTES]
			add rcx, 16
			vpaddusw ymm0, ymm0, ymm1
			cmp rcx, rdi
			vmovdqa [rbx + (rcx - 16)*COMPV_YASM_INT16_SZ_BYTES], ymm0
			jl .LoopCols16
		.EndOfLoopCols16

		sub rdi, 63

		add rax, arg(5)
		add rdx, arg(5)
		add rbx, arg(5)
		dec rsi
		jnz .LoopRows

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_YMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret


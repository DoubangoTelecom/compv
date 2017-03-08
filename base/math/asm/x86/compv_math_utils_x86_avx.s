;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(CompVMathUtilsSumAbs_16s16u_Asm_X86_AVX2)
global sym(CompVMathUtilsSum_8u32u_Asm_X86_AVX2)

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
		js .EndOfLoopCols64
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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* data
; arg(1) -> compv_uscalar_t count
; arg(2) -> uint8_t *mean1
sym(CompVMathUtilsSum_8u32u_Asm_X86_AVX2):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 3
	push rsi
	push rdi
	push rbx
	;; end prolog ;;
	
	vpxor ymm0, ymm0 ; ymm0 = ymmZer0
	vpxor ymm3, ymm3 ; ymm3 = ymmSum
	mov rsi, arg(0) ; rsi = data
	mov rdx, arg(1)
	; rcx = i
	lea rbx, [rdx - 31] ; rbx = count - 31
	lea rax, [rdx - 15] ; rax = count - 15
	lea rdi, [rdx - 7] ; rdi = count - 7
	lea rdx, [rdx - 3] ; rdx = count - 3

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (i = 0; i < count_ - 31; i += 32)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	xor rcx, rcx
	.LoopRows 
		vmovdqa ymm2, [rsi + rcx]
		lea rcx, [rcx + 32]
		vpunpcklbw ymm1, ymm2, ymm0
		vpunpckhbw ymm2, ymm2, ymm0
		vpaddw ymm2, ymm2, ymm1
		cmp rcx, rbx
		vpunpcklwd ymm1, ymm2, ymm0
		vpunpckhwd ymm2, ymm2, ymm0
		vpaddd ymm1, ymm1, ymm2
		vpaddd ymm3, ymm3, ymm1	
		jl .LoopRows

	;;;;;;;;;;;;;;;;;;;;;;;;;;
	; if (i < count_ - 15)
	;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rcx, rax
	jge .EndOfMoreThanFifteenRemains
		vmovdqa xmm2, [rsi + rcx]
		lea rcx, [rcx + 16]
		vpunpcklbw xmm1, xmm2, xmm0
		vpunpckhbw xmm2, xmm2, xmm0
		vpaddw xmm2, xmm2, xmm1
		cmp rcx, rbx
		vpunpcklwd xmm1, xmm2, xmm0
		vpunpckhwd xmm2, xmm2, xmm0
		vpaddd xmm1, xmm1, xmm2
		vpaddd ymm3, ymm3, ymm1
	.EndOfMoreThanFifteenRemains

	lea rax, [rdx + 2] ; rax = count - 1

	;;;;;;;;;;;;;;;;;;;;;;;;;;
	; if (i < count_ - 7)
	;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rcx, rdi
	jge .EndOfMoreThanSevenRemains
		vmovq xmm1, [rsi + rcx]
		lea rcx, [rcx + 8]
		vpunpcklbw xmm1, xmm1, xmm0
		vpunpckhwd xmm2, xmm1, xmm0
		vpunpcklwd xmm1, xmm1, xmm0
		vpaddd xmm1, xmm1, xmm2
		vpaddd ymm3, ymm3, ymm1
	.EndOfMoreThanSevenRemains

	;;;;;;;;;;;;;;;;;;;;;;;;;;
	; if (i < count_ - 3)
	;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rcx, rdx
	jge .EndOfMoreThanThreeRemains
		vmovd xmm1, [rsi + rcx]
		lea rcx, [rcx + 4]
		vpunpcklbw xmm1, xmm1, xmm0
		vpunpcklwd xmm1, xmm1, xmm0
		vpaddd ymm3, ymm3, ymm1
	.EndOfMoreThanThreeRemains

	mov rdi, arg(1) ; rdi = count

	vextractf128 xmm1, ymm3, 1
	vpaddd xmm3, xmm3, xmm1
	vphaddd xmm3, xmm3, xmm3
	vphaddd xmm3, xmm3, xmm3
	vmovd ebx, xmm3

	;;;;;;;;;;;;;;;;;;;;;;;;
	; if (i < count_ - 1)
	;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rcx, rax
	jge .EndOfMoreThanOneRemains
		movzx rdx, byte [rsi + rcx]
		movzx rax, byte [rsi + rcx + 1]
		lea rcx, [rcx + 2]
		add rax, rdx
		add rbx, rax
	.EndOfMoreThanOneRemains

	;;;;;;;;;;;;;;;;;;;;
	; if (count_ & 1)
	;;;;;;;;;;;;;;;;;;;;
	test rdi, 1
	jz .EndOfMoreThanZeroRemains
		movzx rdx, byte [rsi + rcx]
		add rbx, rdx
	.EndOfMoreThanZeroRemains

	mov rax, arg(2)
	mov [rax], dword ebx

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret
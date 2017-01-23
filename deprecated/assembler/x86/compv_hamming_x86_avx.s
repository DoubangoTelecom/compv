;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(HammingDistance_Asm_POPCNT_X86_AVX2)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(AVX) const uint8_t* dataPtr
; arg(1) -> compv_scalar_t width
; arg(2) -> compv_scalar_t stride
; arg(3) -> compv_scalar_t height
; arg(4) -> COMPV_ALIGNED(AVX) const uint8_t* patch1xnPtr
; arg(5) -> int32_t* distPtr
; void HammingDistance_Asm_POPCNT_X86_AVX2(COMPV_ALIGNED(AVX) const uint8_t* dataPtr, compv_scalar_t width, compv_scalar_t stride, compv_scalar_t height, COMPV_ALIGNED(AVX) const uint8_t* patch1xnPtr, int32_t* distPtr)
sym(HammingDistance_Asm_POPCNT_X86_AVX2):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; alloc memory
	sub rsp, 8 + 8 + 8 + 8 + 8
	%define i_sub1		rsp + 0
	%define i_sub2		rsp + 8
	%define i_sub4		rsp + 16
	%define i_sub16		rsp + 24
	%define i_sub32		rsp + 32

	mov rax, arg(1)
	sub rax, 1
	mov [i_sub1], rax
	sub rax, 1
	mov [i_sub2], rax
	sub rax, 2
	mov [i_sub4], rax
	sub rax, 12
	mov [i_sub16], rax
	sub rax, 16
	mov [i_sub32], rax

	; rdi = j = 0
	xor rdi, rdi

	; rcx = dataPtr
	mov rcx, arg(0)

	; rdx = patch1xnPtr
	mov rdx, arg(4)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopRows
		xor rbx, rbx ; rbx = cnt
		xor rsi, rsi ; rsi = i

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i <= width - 32; i += 32)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp rsi, [i_sub32]
		jg .EndOfLoopCols32
		.LoopCols32
			vmovdqa ymm0, [rcx + rsi]
			vpxor ymm0, ymm0, [rdx + rsi]
			vextractf128 xmm1, ymm0, 1

			vmovd eax, xmm0
			popcnt eax, eax
			add ebx, eax
			vpextrd eax, xmm0, 1
			popcnt eax, eax
			add ebx, eax
			vpextrd eax, xmm0, 2
			popcnt eax, eax
			add ebx, eax
			vpextrd eax, xmm0, 3
			popcnt eax, eax
			add ebx, eax
			vmovd eax, xmm1
			popcnt eax, eax
			add ebx, eax
			vpextrd eax, xmm1, 1
			popcnt eax, eax
			add ebx, eax
			vpextrd eax, xmm1, 2
			popcnt eax, eax
			add ebx, eax
			vpextrd eax, xmm1, 3
			popcnt eax, eax
			add rsi, 32
			add ebx, eax
			cmp rsi, [i_sub32]
			jle .LoopCols32
			.EndOfLoopCols32

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i <= width - 16; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp rsi, [i_sub16]
		jg .EndOfLoopCols16
		.LoopCols16
			vmovdqa xmm0, [rcx + rsi]
			vpxor xmm0, xmm0, [rdx + rsi]
			vmovd eax, xmm0
			popcnt eax, eax
			add ebx, eax
			vpextrd eax, xmm0, 1
			popcnt eax, eax
			add ebx, eax
			vpextrd eax, xmm0, 2
			popcnt eax, eax
			add ebx, eax
			vpextrd eax, xmm0, 3
			popcnt eax, eax
			add rsi, 16
			add ebx, eax
			cmp rsi, [i_sub16]
			jle .LoopCols16
			.EndOfLoopCols16

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i <= width - 4; i += 4)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp rsi, [i_sub4]
		jg .EndOfLoopCols4
		.LoopCols4
			mov eax, dword [rcx + rsi]
			xor eax, dword [rdx + rsi]
			popcnt eax, eax
			add rsi, 4
			add ebx, eax
			cmp rsi, [i_sub4]
			jle .LoopCols4
			.EndOfLoopCols4

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i <= width - 2)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp rsi, [i_sub2]
		jg .EndOfIf2
			mov ax, word [rcx + rsi]
			xor ax, word [rdx + rsi]
			popcnt ax, ax
			add rsi, 2
			add ebx, eax
			.EndOfIf2

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i <= width - 1)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp rsi, [i_sub1]
		jg .EndOfIf1
			mov al, byte [rcx + rsi]
			xor al, byte [rdx + rsi]
			popcnt ax, ax
			add rsi, 1
			add ebx, eax
			.EndOfIf1

		mov rax, arg(5) ; distPtr
		add rcx, arg(2) ; dataPtr += stride
		mov [rax + rdi*4], ebx ; distPtr[j] = (int32_t)(cnt)

		inc rdi
		cmp rdi, arg(3)
		jl .LoopRows

	; free memory
	add rsp, 8 + 8 + 8 + 8 + 8
	%undef i_sub1
	%undef i_sub2
	%undef i_sub4
	%undef i_sub16
	%undef i_sub32

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret
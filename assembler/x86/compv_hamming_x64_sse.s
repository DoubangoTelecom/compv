;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(HammingDistance_Asm_POPCNT_X64_SSE42)
global sym(HammingDistance256_Asm_POPCNT_X64_SSE42)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* dataPtr
; arg(1) -> compv_scalar_t width
; arg(2) -> compv_scalar_t stride
; arg(3) -> compv_scalar_t height
; arg(4) -> COMPV_ALIGNED(SSE) const uint8_t* patch1xnPtr
; arg(5) -> int32_t* distPtr
; void HammingDistance_Asm_POPCNT_X64_SSE42(COMPV_ALIGNED(SSE) const uint8_t* dataPtr, compv_scalar_t width, compv_scalar_t stride, compv_scalar_t height, COMPV_ALIGNED(SSE) const uint8_t* patch1xnPtr, int32_t* distPtr)
sym(HammingDistance_Asm_POPCNT_X64_SSE42):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	push r14
	push r15
	;; end prolog ;;

	mov rax, arg(1)
	sub rax, 1
	mov r11, rax ; r11 = (width - 1)
	sub rax, 1
	mov r12, rax ; r12 = (width - 2)
	sub rax, 2
	mov r13, rax ; r13 = (width - 4)
	sub rax, 12
	mov r10, rax ; r10 = (width - 16)
	sub rax, 16 ; rax = (width - 32)

	; rdi = j = 0
	xor rdi, rdi

	; rcx = dataPtr
	mov rcx, arg(0)

	; rdx = patch1xnPtr
	mov rdx, arg(4)

	; r8 = stride
	mov r8, arg(2)

	; r9 = distPtr
	mov r9, arg(5)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopRows
		xor rbx, rbx ; rbx = cnt
		xor rsi, rsi ; rsi = i

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i <= width - 32; i += 32)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp rsi, rax
		jg .EndOfLoopCols32
		.LoopCols32
			movdqa xmm0, [rcx + rsi]
			movdqa xmm1, [rdx + rsi]
			movdqa xmm2, [rcx + rsi + 16]
			movdqa xmm3, [rdx + rsi + 16]
			pxor xmm0, xmm1
			pxor xmm2, xmm3
			movq r14, xmm0
			pextrq r15, xmm0, 1
			popcnt r14, r14
			popcnt r15, r15
			add rbx, r14
			add rbx, r15
			movq r14, xmm2
			pextrq r15, xmm2, 1
			popcnt r14, r14
			popcnt r15, r15
			add rsi, 32
			add rbx, r14
			add rbx, r15
			cmp rsi, rax
			jle .LoopCols32
			.EndOfLoopCols32

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i <= width - 16; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp rsi, r10
		jg .EndOfLoopCols16
		.LoopCols16
			movdqa xmm0, [rcx + rsi]
			movdqa xmm1, [rdx + rsi]
			pxor xmm0, xmm1
			movd r14, xmm0
			pextrq r15, xmm0, 1
			popcnt r14, r14
			popcnt r15, r15
			add rsi, 16
			add rbx, r14
			add rbx, r15
			cmp rsi, r10
			jle .LoopCols16
			.EndOfLoopCols16

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i <= width - 4; i += 4)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp rsi, r13
		jg .EndOfLoopCols4
		.LoopCols4
			mov r14d, dword [rcx + rsi]
			xor r14d, dword [rdx + rsi]
			popcnt r14d, r14d
			add rsi, 4
			add rbx, r14
			cmp rsi, r13
			jle .LoopCols4
			.EndOfLoopCols4

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i <= width - 2)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp rsi, r12
		jg .EndOfIf2
			mov r14w, word [rcx + rsi]
			xor r14w, word [rdx + rsi]
			popcnt r14w, r14w
			add rsi, 2
			add rbx, r14
			.EndOfIf2

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i <= width - 1)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp rsi, r11
		jg .EndOfIf1
			mov r14b, byte [rcx + rsi]
			xor r14b, byte [rdx + rsi]
			popcnt r14w, r14w
			inc rsi
			add rbx, r14
			.EndOfIf1
		
		add rcx, r8 ; dataPtr += stride
		mov [r9 + rdi*4], dword ebx ; distPtr[j] = (int32_t)(cnt)

		inc rdi
		cmp rdi, arg(3)
		jl .LoopRows

	;; begin epilog ;;
	pop r15
	pop r14
	pop r13
	pop r12
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* dataPtr
; arg(1) -> compv_scalar_t height
; arg(2) -> COMPV_ALIGNED(SSE) const uint8_t* patch1xnPtr
; arg(3) -> int32_t* distPtr
; void HammingDistance256_Asm_POPCNT_X64_SSE42(COMPV_ALIGNED(SSE) const uint8_t* dataPtr, compv_scalar_t height, COMPV_ALIGNED(SSE) const uint8_t* patch1xnPtr, int32_t* distPtr)
sym(HammingDistance256_Asm_POPCNT_X64_SSE42):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 4
	push rsi
	push rdi
	;; end prolog ;;

	; rdi = j = 0
	xor rdi, rdi

	; rcx = dataPtr
	mov rcx, arg(0)

	; r8 = height
	mov r8, arg(1)

	; rdx = patch1xnPtr
	mov rdx, arg(2)

	; r9 = distPtr
	mov r9, arg(3)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopRows
		movdqa xmm0, [rcx]
		movdqa xmm1, [rdx]
		movdqa xmm2, [rcx + 16]
		movdqa xmm3, [rdx + 16]
		pxor xmm0, xmm1
		pxor xmm2, xmm3
		movq r11, xmm0
		pextrq r10, xmm0, 1
		movq rax, xmm2
		pextrq rsi, xmm2, 1
		popcnt r11, r11
		popcnt r10, r10
		popcnt rax, rax
		popcnt rsi, rsi
		add r11, r10
		add r11, rax
		add r11, rsi

		mov [r9 + rdi*4], dword r11d ; distPtr[j] = (int32_t)(cnt)

		add rcx, 32 ; dataPtr += 32
		inc rdi
		cmp rdi, r8
		jl .LoopRows

	;; begin epilog ;;
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

%endif ; COMPV_YASM_ABI_IS_64BIT
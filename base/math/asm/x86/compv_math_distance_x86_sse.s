;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(CompVMathDistanceHamming_Asm_X86_POPCNT_SSE42)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* dataPtr
; arg(1) -> compv_uscalar_t width
; arg(2) -> compv_uscalar_t height
; arg(3) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
; arg(4) -> COMPV_ALIGNED(SSE) const uint8_t* patch1xnPtr
; arg(5) -> int32_t* distPtr
sym(CompVMathDistanceHamming_Asm_X86_POPCNT_SSE42):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; alloc memory
	sub rsp, (4 * COMPV_YASM_REG_SZ_BYTES)

	%define width_minus15	rsp + (0 * COMPV_YASM_REG_SZ_BYTES)
	%define width_minus3	rsp + (1 * COMPV_YASM_REG_SZ_BYTES)
	%define width_minus1	rsp + (2 * COMPV_YASM_REG_SZ_BYTES)
	%define width_minus0	rsp + (3 * COMPV_YASM_REG_SZ_BYTES)

	mov rax, arg(1)
	lea rbx, [rax - 15]
	lea rdx, [rax - 3]
	lea rdi, [rax - 1]
	mov [width_minus0], rax
	mov [width_minus15], rbx
	mov [width_minus3], rdx
	mov [width_minus1], rdi

	%define j			rsi
	%define cnt			edi
	%define i			rcx
	%define dataPtr		rbx
	%define patch1xnPtr	rdi

	mov j, arg(2) ; rsi = height
	mov dataPtr, arg(0)
	mov patch1xnPtr, arg(4)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width - 15; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		xor cnt, cnt
		.LoopWidth16:
			movdqa xmm0, [dataPtr + i]
			movdqa xmm1, [patch1xnPtr + i]
			pxor xmm0, xmm1
			movd eax, xmm0
			popcnt eax, eax
			lea i, [i + 16]
			lea cnt, [cnt + eax]
			pextrd eax, xmm0, 1
			popcnt eax, eax
			cmp i, [width_minus15]
			lea cnt, [cnt + eax]
			pextrd eax, xmm0, 2
			popcnt eax, eax
			lea cnt, [cnt + eax]
			pextrd eax, xmm0, 3
			popcnt eax, eax
			lea cnt, [cnt + eax]
			jl .LoopWidth16
			; EndOf_LoopWidth16 ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width - 3)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.IfMoreThan4:
			%rep 3
				cmp i, [width_minus3]
				jge .EndOf_IfMoreThan4
				mov eax, dword [dataPtr + i]
				xor eax, dword [patch1xnPtr + i]
				popcnt eax, eax
				lea i, [i + 4]
				lea cnt, [cnt + eax]
			%endrep
			.EndOf_IfMoreThan4:
			; EndOf_IfMoreThan4 ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width - 1)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, [width_minus1]
		jge .EndOf_IfMoreThan2
		.IfMoreThan2:
			mov ax, word [dataPtr + i]
			xor ax, word [patch1xnPtr + i]
			popcnt ax, ax
			lea i, [i + 2]
			lea cnt, [cnt + eax]
			.EndOf_IfMoreThan2:
			; EndOf_IfMoreThan2 ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, [width_minus0]
		jge .EndOf_IfMoreThan1
		.IfMoreThan1:
			mov al, byte [dataPtr + i]
			xor al, byte [patch1xnPtr + i]
			popcnt ax, ax
			lea cnt, [cnt + eax]
			.EndOf_IfMoreThan1:
			; EndOf_IfMoreThan1 ;

		mov rax, arg(5) ; distPtr
		add dataPtr, arg(3) ; dataPtr += stride
		mov [rax + j * COMPV_YASM_INT32_SZ_BYTES], cnt
		dec j
		jnz .LoopHeight
		; EndOf_LoopHeight ;

	%undef j
	%undef cnt
	%undef i
	%undef dataPtr
	%undef patch1xnPtr

	%undef width_minus15
	%undef width_minus3
	%undef width_minus1
	%undef width_minus0

	; free memory
	add rsp, (4 * COMPV_YASM_REG_SZ_BYTES)

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(CompVMathDistanceHamming_Asm_X64_POPCNT_SSE42)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* dataPtr
; arg(1) -> compv_uscalar_t width
; arg(2) -> compv_uscalar_t height
; arg(3) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
; arg(4) -> COMPV_ALIGNED(SSE) const uint8_t* patch1xnPtr
; arg(5) -> int32_t* distPtr
sym(CompVMathDistanceHamming_Asm_X64_POPCNT_SSE42):
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

	%define j				rsi
	%define cnt				edi
	%define i				rcx
	%define dataPtr			rbx
	%define patch1xnPtr		rdx
	%define width_minus0	r8
	%define width_minus1	r9
	%define width_minus3	r10
	%define width_minus7	r11
	%define width_minus15	r12

	mov dataPtr, arg(0)
	mov width_minus0, arg(1)
	mov patch1xnPtr, arg(4)
	lea width_minus1, [width_minus0 - 1]
	lea width_minus3, [width_minus0 - 3]
	lea width_minus7, [width_minus0 - 7]
	lea width_minus15, [width_minus0 - 15]	

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	xor j, j
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
			add i, 16
			add cnt, eax
			pextrd eax, xmm0, 1
			popcnt eax, eax
			add cnt, eax
			pextrd eax, xmm0, 2
			popcnt eax, eax
			add cnt, eax
			pextrd eax, xmm0, 3
			popcnt eax, eax
			cmp i, width_minus15
			lea cnt, [cnt + eax]
			jl .LoopWidth16
			; EndOf_LoopWidth16 ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width - 3)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.IfMoreThan4:
			%rep 3
				cmp i, width_minus3
				jge .EndOf_IfMoreThan4
				mov eax, dword [dataPtr + i]
				xor eax, dword [patch1xnPtr + i]
				popcnt eax, eax
				add i, 4
				add cnt, eax
			%endrep
			.EndOf_IfMoreThan4:
			; EndOf_IfMoreThan4 ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width - 1)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width_minus1
		jge .EndOf_IfMoreThan2
		.IfMoreThan2:
			mov ax, word [dataPtr + i]
			xor ax, word [patch1xnPtr + i]
			popcnt ax, ax
			add i, 2
			add cnt, eax
			.EndOf_IfMoreThan2:
			; EndOf_IfMoreThan2 ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width_minus0
		jge .EndOf_IfMoreThan1
		.IfMoreThan1:
			mov al, byte [dataPtr + i]
			xor al, byte [patch1xnPtr + i]
			popcnt ax, ax
			add cnt, eax
			.EndOf_IfMoreThan1:
			; EndOf_IfMoreThan1 ;

		mov rax, arg(5) ; distPtr
		mov [rax + j * COMPV_YASM_INT32_SZ_BYTES], dword cnt
		inc j
		add dataPtr, arg(3) ; dataPtr += stride
		cmp j, arg(2)
		jl .LoopHeight
		; EndOf_LoopHeight ;

	%undef j
	%undef cnt
	%undef i
	%undef dataPtr
	%undef patch1xnPtr
	%undef width_minus0
	%undef width_minus1
	%undef width_minus3
	%undef width_minus7
	%undef width_minus15

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
	ret


%endif ; COMPV_YASM_ABI_IS_64BIT

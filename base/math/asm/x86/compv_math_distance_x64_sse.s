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
global sym(CompVMathDistanceHamming32_Asm_X64_POPCNT_SSE42)

section .data

section .text

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

	%define argi_width		1

	%define j				rsi
	%define cnt				rdi
	%define cntdword		edi
	%define i				rcx
	%define dataPtr			rbx
	%define patch1xnPtr		rdx
	%define width			r8
	%define widthmax		r9
	%define distPtr			r10
	%define stride			r11
	%define width_minus31	r12

	mov dataPtr, arg(0)
	mov j, arg(2)
	mov stride, arg(3)
	mov width, arg(argi_width)
	mov patch1xnPtr, arg(4)
	mov distPtr, arg(5)
	lea width_minus31, [width - 31]

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		xor i, i
		xor cnt, cnt

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width - 31; i += 32)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		test width_minus31, width_minus31 ; width is required to be > 15 but not to be > 31
		js .EndOf_LoopWidth32 ; jump to the end of the loop if (width_minus31 < 0)
		.LoopWidth32:
			movdqa xmm0, [dataPtr + i]
			movdqa xmm2, [patch1xnPtr + i]
			movdqa xmm1, [dataPtr + i + 16]
			movdqa xmm3, [patch1xnPtr + i + 16]
			pxor xmm0, xmm2
			pxor xmm1, xmm3
			movq rax, xmm0
			pextrq r13, xmm0, 1
			movq r14, xmm1
			pextrq r15, xmm1, 1
			popcnt rax, rax
			popcnt r13, r13
			popcnt r14, r14
			popcnt r15, r15
			add i, 32
			add rax, r13
			add r14, r15
			add cnt, rax
			add cnt, r14
			cmp i, width_minus31
			jl .LoopWidth32
			.EndOf_LoopWidth32
			; EndOf_LoopWidth32 ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width - 15)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea widthmax, [width - 15]
		cmp i, widthmax
		jge .EndOf_IfMoreThan16
		.IfMoreThan16:
			movdqa xmm0, [dataPtr + i]
			movdqa xmm1, [patch1xnPtr + i]
			pxor xmm0, xmm1
			movq rax, xmm0
			pextrq r13, xmm0, 1
			popcnt rax, rax
			popcnt r13, r13
			add i, 16
			add cnt, rax
			add cnt, r13
			.EndOf_IfMoreThan16
			; EndOf_IfMoreThan16 ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width - 7)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea widthmax, [width - 7]
		cmp i, widthmax
		jge .EndOf_IfMoreThan8
		.IfMoreThan8:
			movdqa xmm0, [dataPtr + i]
			movdqa xmm1, [patch1xnPtr + i]
			pxor xmm0, xmm1
			movq rax, xmm0
			popcnt rax, rax
			add i, 8
			add cnt, rax
			.EndOf_IfMoreThan8
			; EndOf_IfMoreThan8 ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width - 3)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea widthmax, [width - 3]
		cmp i, widthmax
		jge .EndOf_IfMoreThan4
		.IfMoreThan4:
			mov eax, dword [dataPtr + i]
			xor eax, dword [patch1xnPtr + i]
			popcnt eax, eax
			add i, 4
			add cnt, rax
			.EndOf_IfMoreThan4:
			; EndOf_IfMoreThan4 ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width - 1)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea widthmax, [width - 1]
		cmp i, widthmax
		jge .EndOf_IfMoreThan2
		.IfMoreThan2:
			mov ax, word [dataPtr + i]
			xor ax, word [patch1xnPtr + i]
			popcnt ax, ax
			add i, 2
			add cnt, rax
			.EndOf_IfMoreThan2:
			; EndOf_IfMoreThan2 ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, arg(argi_width)
		jge .EndOf_IfMoreThan1
		.IfMoreThan1:
			mov al, byte [dataPtr + i]
			xor al, byte [patch1xnPtr + i]
			popcnt ax, ax
			add cnt, rax
			.EndOf_IfMoreThan1:
			; EndOf_IfMoreThan1 ;

		dec j
		mov [distPtr], dword cntdword
		lea dataPtr, [dataPtr + stride]
		lea distPtr, [distPtr + COMPV_YASM_INT32_SZ_BYTES]
		jnz .LoopHeight
		; EndOf_LoopHeight ;

	%undef argi_width

	%undef j
	%undef cnt
	%undef cntdword
	%undef i
	%undef dataPtr
	%undef patch1xnPtr
	%undef width
	%undef widthmax
	%undef distPtr
	%undef stride
	%undef width_minus31

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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* dataPtr
; arg(1) -> compv_uscalar_t height
; arg(2) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
; arg(3) -> COMPV_ALIGNED(SSE) const uint8_t* patch1xnPtr
; arg(4) -> int32_t* distPtr
sym(CompVMathDistanceHamming32_Asm_X64_POPCNT_SSE42):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	push r12
	;; end prolog ;;

	%define j					r8
	%define cnt					rax
	%define cntdword			eax
	%define dataPtr				r9
	%define distPtr				rdx
	%define stride				rcx

	%define vecpatch1xnPtr0		xmm2		
	%define vecpatch1xnPtr1		xmm3			

	mov rax, arg(3)
	movdqa vecpatch1xnPtr0, [rax]
	movdqa vecpatch1xnPtr1, [rax + 16]

	mov dataPtr, arg(0)
	mov j, arg(1)
	mov stride, arg(2)
	mov distPtr, arg(4)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		movdqa xmm0, [dataPtr]
		movdqa xmm1, [dataPtr + 16]
		pxor xmm0, vecpatch1xnPtr0
		pxor xmm1, vecpatch1xnPtr1
		movq cnt, xmm0
		pextrq r10, xmm0, 1
		movq r11, xmm1
		pextrq r12, xmm1, 1
		popcnt cnt, cnt
		popcnt r10, r10
		popcnt r11, r11
		popcnt r12, r12
		lea dataPtr, [dataPtr + stride]
		add cnt, r10
		add r11, r12
		add cnt, r11
		dec j
		mov [distPtr], dword cntdword
		lea distPtr, [distPtr + COMPV_YASM_INT32_SZ_BYTES]
		jnz .LoopHeight
		; EndOf_LoopHeight ;

	%undef j
	%undef cnt
	%undef cntdword
	%undef dataPtr
	%undef distPtr
	%undef stride

	%undef vecpatch1xnPtr0				
	%undef vecpatch1xnPtr1			

	;; begin epilog ;;
	pop r12
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret


%endif ; COMPV_YASM_ABI_IS_64BIT

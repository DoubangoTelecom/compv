;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(CompVConnectedComponentLabelingLSL_Step1Algo13SegmentRLE_8u32s_Asm_X64_CMOV)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* Xi
; arg(1) -> int32_t* RLCi
; arg(2) -> int32_t* ERi
; arg(3) -> int32_t* b1
; arg(4) -> int32_t* er1
; arg(5) -> const int32_t width
sym(CompVConnectedComponentLabelingLSL_Step1Algo13SegmentRLE_8u32s_Asm_X64_CMOV):
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

	%define Xi			rsi
	%define RLCi		rcx
	%define ERi			rdx
	%define width		rax
	%define widthd		eax
	%define i			rdi
	%define b			rbx
	%define bd			ebx
	%define er			r8
	%define erd			r8d

	mov Xi, arg(0)
	mov RLCi, arg(1)
	mov ERi, arg(2)
	mov b, arg(3)
	mov er, arg(4)
	movsxd width, dword arg(5)

	mov bd, dword [b]
	mov erd, dword [er]

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (i = 1; i < width; i += 1)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov i, 1
	.LoopWidth1:
		movzx r10, byte [Xi + (i-1)*COMPV_YASM_UINT8_SZ_BYTES]
		movzx r11, byte [Xi + i*COMPV_YASM_UINT8_SZ_BYTES]
		xor r10, r11
		jz .XorIsEqualToZero
			mov r10, i
			sub r10, b
			mov [RLCi + er*COMPV_YASM_INT32_SZ_BYTES], dword r10d
			xor b, 1
			inc er

		.XorIsEqualToZero:
		mov [ERi + i*COMPV_YASM_INT32_SZ_BYTES], dword erd

		inc i
		cmp i, width
		jl .LoopWidth1
	.EndOf_LoopWidth1:

	mov r10, arg(3)
	mov r11, arg(4)
	mov [r10], dword bd
	mov [r11], dword erd

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
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(CompVMathDistanceHamming32_Asm_X64_POPCNT)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* dataPtr
; arg(1) -> compv_uscalar_t height
; arg(2) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
; arg(3) -> COMPV_ALIGNED(SSE) const uint8_t* patch1xnPtr
; arg(4) -> int32_t* distPtr
sym(CompVMathDistanceHamming32_Asm_X64_POPCNT):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	;; end prolog ;;

	%define j					rsi
	%define cnt					rbx
	%define cntdword			ebx
	%define dataPtr				rdi
	%define distPtr				rdx
	%define stride				rcx
	%define patch1xnPtr0		rax
	%define patch1xnPtr1		r8
	%define patch1xnPtr2		r9
	%define patch1xnPtr3		r10

	mov r13, arg(3)
	mov patch1xnPtr0, [r13 + (0*COMPV_YASM_REG_SZ_BYTES)]
	mov patch1xnPtr1, [r13 + (1*COMPV_YASM_REG_SZ_BYTES)]
	mov patch1xnPtr2, [r13 + (2*COMPV_YASM_REG_SZ_BYTES)]
	mov patch1xnPtr3, [r13 + (3*COMPV_YASM_REG_SZ_BYTES)]

	mov dataPtr, arg(0)
	mov j, arg(1)
	mov stride, arg(2)
	mov distPtr, arg(4)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		mov cnt, [dataPtr + (0*COMPV_YASM_REG_SZ_BYTES)]
		mov r11, [dataPtr + (1*COMPV_YASM_REG_SZ_BYTES)]
		mov r12, [dataPtr + (2*COMPV_YASM_REG_SZ_BYTES)]
		mov r13, [dataPtr + (3*COMPV_YASM_REG_SZ_BYTES)]
		xor cnt, patch1xnPtr0
		xor r11, patch1xnPtr1
		popcnt cnt, cnt
		popcnt r11, r11
		xor r12, patch1xnPtr2
		xor r13, patch1xnPtr3
		popcnt r12, r12
		popcnt r13, r13
		add cnt, r11
		add r12, r13
		add dataPtr, stride
		add cnt, r12
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
	%undef patch1xnPtr0
	%undef patch1xnPtr1
	%undef patch1xnPtr2
	%undef patch1xnPtr3

	;; begin epilog ;;
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

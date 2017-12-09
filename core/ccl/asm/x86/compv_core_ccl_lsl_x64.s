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
; Don''t know how to cancat string ('b' or 'd') to the name of the reg 
%macro SET_RLC_1	4
	%define reg		%1
	%define regb	%2
	%define regd	%3
	%define ii		%4
	xor regb, byte [Xi + (ii)*COMPV_YASM_UINT8_SZ_BYTES]
	jz .%%XorIsEqualToZero
		lea reg, [(ii)]
		sub reg, b
		xor b, 1
		mov [RLCi + er*COMPV_YASM_INT32_SZ_BYTES], dword regd
		inc er

	.%%XorIsEqualToZero:
	mov [ERi + (ii)*COMPV_YASM_INT32_SZ_BYTES], dword erd
	%undef reg	
	%undef regb
	%undef regd
	%undef ii
%endmacro

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
	;; end prolog ;;

	%define Xi			rsi
	%define RLCi		rcx
	%define ERi			rdx
	%define width		rax
	%define i			rdi
	%define b			rbx
	%define bd			ebx
	%define er			r8
	%define erd			r8d
	%define width4		r9

	mov Xi, arg(0)
	mov RLCi, arg(1)
	mov ERi, arg(2)
	mov b, arg(3)
	mov er, arg(4)
	movsxd width, dword arg(5)
	mov width4, width
	and width4, -4

	mov bd, dword [b]
	mov erd, dword [er]

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (i = 1; i < width4; i += 4)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov i, 1
	test width4, width4
	jz .EndOf_LoopWidth4
	.LoopWidth4:
		movzx r10, byte [Xi + (i-1)*COMPV_YASM_UINT8_SZ_BYTES]
		movzx r11, byte [Xi + (i)*COMPV_YASM_UINT8_SZ_BYTES]
		movzx r12, byte [Xi + (i+1)*COMPV_YASM_UINT8_SZ_BYTES]
		movzx r13, byte [Xi + (i+2)*COMPV_YASM_UINT8_SZ_BYTES]
		SET_RLC_1 r10, r10b, r10d, (i + 0)
		SET_RLC_1 r11, r11b, r11d, (i + 1)
		SET_RLC_1 r12, r12b, r12d, (i + 2)
		SET_RLC_1 r13, r13b, r13d, (i + 3)
		add i, 4
		cmp i, width
		jl .LoopWidth4
	.EndOf_LoopWidth4:

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (; i < width; i += 1)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp i, width
	jge .EndOf_LoopWidth1
	.LoopWidth1:
		movzx r10, byte [Xi + (i-1)*COMPV_YASM_UINT8_SZ_BYTES]
		SET_RLC_1 r10, r10b, r10d, (i + 0)
		inc i
		cmp i, width
		jl .LoopWidth1
	.EndOf_LoopWidth1:

	mov r10, arg(3)
	mov r11, arg(4)
	mov [r10], dword bd
	mov [r11], dword erd

	%undef Xi			
	%undef RLCi		
	%undef ERi			
	%undef width		
	%undef i			
	%undef b			
	%undef bd			
	%undef er			
	%undef erd
	%undef width4	

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
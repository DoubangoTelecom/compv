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

global sym(CompVMathHistogramProcess_8u32s_Asm_X64_SSE2)

section .data
	
section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; !!! MUST NOT USE !!!
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* dataPtr
; arg(1) -> compv_uscalar_t width
; arg(2) -> compv_uscalar_t height
; arg(3) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
; arg(4) -> uint32_t *histogram0
sym(CompVMathHistogramProcess_8u32s_Asm_X64_SSE2)
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	COMPV_YASM_SAVE_XMM 15
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	push r14
	push r15
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 16, rax
	sub rsp, 3*(256*COMPV_YASM_UINT32_SZ_BYTES)
	%define histogram1	rsp + 0
	%define histogram2	histogram1 + (256*COMPV_YASM_UINT32_SZ_BYTES)
	%define histogram3	histogram2 + (256*COMPV_YASM_UINT32_SZ_BYTES)

	; Set memmory to zeros
	lea rdi, [rsp + 0] ; dest
	xor rax, rax ; zero
	mov rcx, (3*128)
	rep stosq

	%define dataPtr			rdi
	%define width			rsi 
	%define height			rbx
	%define stride			rax
	%define	histogram0		rdx
	%define i				rcx
	%define maxWidthStep1	r8

	mov dataPtr, arg(0)
	mov width, arg(1)
	mov height, arg(2)
	mov stride, arg(3)
	mov histogram0, arg(4)

	mov maxWidthStep1, width
	and maxWidthStep1, -4

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		xor i, i
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < maxWidthStep1; i += 4)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopWidth4:
			movzx r9, byte [dataPtr + i + 0]
			movzx r10, byte [dataPtr + i + 1]
			movzx r11, byte [dataPtr + i + 2]
			movzx r12, byte [dataPtr + i + 3]
			add i, 4
			inc dword ptr [histogram0 + r9*COMPV_YASM_UINT32_SZ_BYTES]
			inc dword ptr [histogram0 + r10*COMPV_YASM_UINT32_SZ_BYTES]
			inc dword ptr [histogram0 + r11*COMPV_YASM_UINT32_SZ_BYTES]
			inc dword ptr [histogram0 + r12*COMPV_YASM_UINT32_SZ_BYTES]
			cmp i, maxWidthStep1
			jl .LoopWidth4
			;; EndOf_LoopWidth4 ;;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width; ++i)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_LoopWidth1
		.LoopWidth1:
			movzx r9, byte [dataPtr + i + 0]
			inc i
			lea r9, [histogram0 + r9*COMPV_YASM_UINT32_SZ_BYTES]
			inc dword ptr [r9]
			cmp i, width
			jl .LoopWidth1
			.EndOf_LoopWidth1:
			;; EndOf_LoopWidth1 ;;

		dec height
		lea dataPtr, [dataPtr + stride]
		jnz .LoopHeight
		;; EndOf_LoopHeight ;;

	; free memory and unalign stack
	add rsp, 3*(256*COMPV_YASM_UINT32_SZ_BYTES)
	COMPV_YASM_UNALIGN_STACK
	%undef histogram1
	%undef histogram2
	%undef histogram3

	%undef dataPtr			
	%undef width			 
	%undef height			
	%undef stride			
	%undef	histogram0		
	%undef i
	%undef maxWidthStep1			

	;; begin epilog ;;
	pop r15
	pop r14
	pop r13
	pop r12
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

%endif ; COMPV_YASM_ABI_IS_64BIT

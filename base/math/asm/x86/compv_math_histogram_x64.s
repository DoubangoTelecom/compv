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

global sym(CompVMathHistogramProcess_8u32s_Asm_X64)

section .data
	
section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* dataPtr
; arg(1) -> compv_uscalar_t width
; arg(2) -> compv_uscalar_t height
; arg(3) -> compv_uscalar_t stride
; arg(4) -> uint32_t *histogramPtr
sym(CompVMathHistogramProcess_8u32s_Asm_X64)
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	push r14
	push r15
	;; end prolog ;;

	%define dataPtr			rdi
	%define width			rsi 
	%define height			rbx
	%define stride			rax
	%define	histogramPtr	rdx
	%define i				rcx
	%define maxWidthStep1	r8

	mov dataPtr, arg(0)
	mov width, arg(1)
	mov height, arg(2)
	mov stride, arg(3)
	mov histogramPtr, arg(4)

	mov maxWidthStep1, width
	and maxWidthStep1, -8

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		xor i, i
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < maxWidthStep1; i += 8)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopWidth4:
			movzx r9, byte [dataPtr + i + 0]
			movzx r10, byte [dataPtr + i + 1]
			movzx r11, byte [dataPtr + i + 2]
			movzx r12, byte [dataPtr + i + 3]
			movzx r13, byte [dataPtr + i + 4]
			movzx r14, byte [dataPtr + i + 5]
			movzx r15, byte [dataPtr + i + 6]
			inc dword ptr [histogramPtr + r9*COMPV_YASM_UINT32_SZ_BYTES]
			inc dword ptr [histogramPtr + r10*COMPV_YASM_UINT32_SZ_BYTES]
			movzx r9, byte [dataPtr + i + 7]
			add i, 8
			inc dword ptr [histogramPtr + r11*COMPV_YASM_UINT32_SZ_BYTES]
			inc dword ptr [histogramPtr + r12*COMPV_YASM_UINT32_SZ_BYTES]
			inc dword ptr [histogramPtr + r13*COMPV_YASM_UINT32_SZ_BYTES]
			inc dword ptr [histogramPtr + r14*COMPV_YASM_UINT32_SZ_BYTES]
			inc dword ptr [histogramPtr + r15*COMPV_YASM_UINT32_SZ_BYTES]
			inc dword ptr [histogramPtr + r9*COMPV_YASM_UINT32_SZ_BYTES]
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
			inc dword ptr [histogramPtr + r9*COMPV_YASM_UINT32_SZ_BYTES]
			cmp i, width
			jl .LoopWidth1
			.EndOf_LoopWidth1:
			;; EndOf_LoopWidth1 ;;

		dec height
		lea dataPtr, [dataPtr + stride]
		jnz .LoopHeight
		;; EndOf_LoopHeight ;;

	%undef dataPtr			
	%undef width			 
	%undef height			
	%undef stride			
	%undef	histogramPtr		
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
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

%endif ; COMPV_YASM_ABI_IS_64BIT

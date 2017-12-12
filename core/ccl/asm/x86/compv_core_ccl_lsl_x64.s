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

global sym(CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_X64_CMOV)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const int16_t* RLCi
; arg(1) -> const compv_uscalar_t RLCi_stride,
; arg(2) -> int32_t* ERAi
; arg(3) -> const compv_uscalar_t ERAi_stride,
; arg(4) -> const int16_t* ERiminus1
; arg(5) -> const compv_uscalar_t ERi_stride,
; arg(6) -> const int16_t* ner,
; arg(7) -> const compv_uscalar_t width
; arg(8) -> const compv_uscalar_t height
sym(CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_X64_CMOV):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 9
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	push r14
	push r15
	;; end prolog ;;

	%define RLCi			rsi
	%define RLCi_stride		rcx
	%define ERAi			rdx
	%define ERAi_stride		rax
	%define ERiminus1		rdi
	%define ERi_stride		rbx
	%define ner				r8
	%define width			r9	
	%define nerj			r10
	%define er				r11
	%define j0				r12
	%define j0d				r12d
	%define j0w				r12w
	%define j1				r13
	%define j1d				r13d
	%define j1w				r13w
	%define f0				r14
	%define f0b				r14b
	%define f0w				r14w
	%define f1				r15
	%define f1b				r15b
	%define f1w				r15w

	mov RLCi, arg(0)
	mov RLCi_stride, arg(1)
	mov ERAi, arg(2)
	mov ERAi_stride, arg(3)
	mov ERiminus1, arg(4)
	mov ERi_stride, arg(5)
	mov ner, arg(6)
	mov width, arg(7)

	shl ERi_stride, 1
	shl RLCi_stride, 1
	shl ERAi_stride, 2
	xor j0, j0 ; clear high 32 bytes
	xor j1, j1 ; clear high 32 bytes

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		movzx nerj, word [ner];
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (er = 1; er < nerj; er += 2)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		mov er, 1
		cmp er, nerj
		jge .EndOf_LoopER
		.LoopER:
			lea f0, [RLCi + er*COMPV_YASM_INT16_SZ_BYTES]
			lea f1, [ERiminus1 + 0*COMPV_YASM_INT16_SZ_BYTES]
			movzx j0, word [RLCi + (er-1)*COMPV_YASM_INT16_SZ_BYTES] ; use movzx once to clear j0d
			movzx j1, word [RLCi + er*COMPV_YASM_INT16_SZ_BYTES] ; use movzx once to clear j1d
			prefetcht0 [f0 + RLCi_stride]
			prefetcht0 [f1 + ERi_stride]
			cmp j0w, 0
			lea j1, [j1 - 1]
			lea f1, [width - 1]
			setg f0b
			cmp j1w, f1w
			setl f1b
			movzx f0w, byte f0b
			movzx f1w, byte f1b
			sub j0w, f0w
			add j1w, f1w
			mov j0w, word [ERiminus1 + j0*COMPV_YASM_INT16_SZ_BYTES] ; no need for movzx -> high bytes already cleared above
			mov j1w, word [ERiminus1 + j1*COMPV_YASM_INT16_SZ_BYTES] ; no need for movzx -> high bytes already cleared above
			test j0w, 1
			sete f0b
			test j1w, 1
			sete f1b
			movzx f0w, byte f0b
			movzx f1w, byte f1b
			add j0w, f0w
			sub j1w, f1w
			xor f0w, f0w
			cmp j1w, j0w
			cmovl j0w, f0w
			cmovl j1w, f0w
			lea er, [er + 2]
			shl j1d, 16
			or j0d, j1d
			cmp er, nerj 
			mov dword [ERAi + (er-2)*COMPV_YASM_INT32_SZ_BYTES], j0d			
			jl .LoopER
		.EndOf_LoopER:

		dec qword arg(8)
		lea ERiminus1, [ERiminus1 + ERi_stride*1]
		lea RLCi, [RLCi + RLCi_stride*1]
		lea ERAi, [ERAi + ERAi_stride*1]
		lea ner, [ner + 1*COMPV_YASM_INT16_SZ_BYTES]
		jnz .LoopHeight
	.EndOf_LoopHeight:

	%undef RLCi			
	%undef RLCi_stride		
	%undef ERAi			
	%undef ERAi_stride		
	%undef ERiminus1		
	%undef ERi_stride		
	%undef ner				
	%undef width				
	%undef nerj			
	%undef er				
	%undef j0				
	%undef j0d				
	%undef j0w				
	%undef j1				
	%undef j1d				
	%undef j1w				
	%undef f0				
	%undef f0b				
	%undef f0w				
	%undef f1				
	%undef f1b				
	%undef f1w				

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
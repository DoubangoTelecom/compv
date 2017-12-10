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

global sym(CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_8u16s32s_Asm_X64_CMOV)
global sym(CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_X64_CMOV)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Don''t know how to cancat string ('b' or 'd') to the name of the reg 
%macro SET_RLC_1	4
	%define reg		%1
	%define regb	%2
	%define regw	%3
	%define ii		%4
	xor regb, byte [Xi + (ii)*COMPV_YASM_UINT8_SZ_BYTES]
	jz .%%XorIsEqualToZero
		lea reg, [(ii)]
		sub reg, b
		xor b, 1
		mov [RLCi + er*COMPV_YASM_INT16_SZ_BYTES], word regw
		inc er

	.%%XorIsEqualToZero:
	mov [ERi + (ii)*COMPV_YASM_INT16_SZ_BYTES], word erw
	%undef reg	
	%undef regb
	%undef regw
	%undef ii
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* Xi, 
; arg(1) -> const compv_uscalar_t Xi_stride,
; arg(2) -> int16_t* RLCi, 
; arg(3) -> const compv_uscalar_t RLCi_stride,
; arg(4) -> int16_t* ERi, 
; arg(5) -> const compv_uscalar_t ERi_stride,
; arg(6) -> int16_t* ner, 
; arg(7) -> int16_t* ner_max1, 
; arg(8) -> compv_ccl_indice_t* ner_sum1,
; arg(9) -> const compv_uscalar_t width, 
; arg(10) ->const compv_uscalar_t height
sym(CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_8u16s32s_Asm_X64_CMOV):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 11
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
	%define widthw		ax
	%define i			rdi
	%define b			rbx
	%define bw			bx
	%define er			r8
	%define erw			r8w
	%define width4		r9
	%define X			r10
	%define Xb			r10b
	%define Xw			r10w
	%define Xd			r10d
	%define ner			r11
	%define ner_sum		r12
	%define ner_sumd	r12d
	%define ner_max		r13
	%define ner_maxw	r13w
	%define Xi_stride	r14
	%define ERi_stride	r15

	mov Xi, arg(0)
	mov Xi_stride, arg(1)
	mov RLCi, arg(2)
	mov ERi, arg(4)
	mov ERi_stride, arg(5)
	mov ner, arg(6)
	mov width, arg(9)

	lea width4, [width - 1]
	xor ner_sum, ner_sum
	and width4, -4
	xor ner_max, ner_max
	shl ERi_stride, 1 ; from bytes to words (int16)
	mov X, arg(3) ; RLCi_stride
	shl X, 1
	mov arg(3), X

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		; For i = 0 ;
		movzx b, byte [Xi + 0*COMPV_YASM_UINT8_SZ_BYTES]
		mov [RLCi + 0*COMPV_YASM_INT16_SZ_BYTES], word 0
		and b, 1
		mov er, b
		mov [ERi + 0*COMPV_YASM_INT16_SZ_BYTES], word bw
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 1; i < width4; i += 4)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		mov i, 1
		test width4, width4
		jz .EndOf_LoopWidth4
		.LoopWidth4:
			movzx X, byte [Xi + (i-1)*COMPV_YASM_UINT8_SZ_BYTES]
			SET_RLC_1 X, Xb, Xw, (i + 0)
			movzx X, byte [Xi + (i)*COMPV_YASM_UINT8_SZ_BYTES]
			SET_RLC_1 X, Xb, Xw, (i + 1)
			movzx X, byte [Xi + (i+1)*COMPV_YASM_UINT8_SZ_BYTES]
			SET_RLC_1 X, Xb, Xw, (i + 2)
			movzx X, byte [Xi + (i+2)*COMPV_YASM_UINT8_SZ_BYTES]
			SET_RLC_1 X, Xb, Xw, (i + 3)
			add i, 4
			cmp i, width4
			jl .LoopWidth4
		.EndOf_LoopWidth4:

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width; i += 1)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_LoopWidth1
		.LoopWidth1:
			movzx X, byte [Xi + (i-1)*COMPV_YASM_UINT8_SZ_BYTES]
			SET_RLC_1 X, Xb, Xw, (i + 0)
			inc i
			cmp i, width
			jl .LoopWidth1
		.EndOf_LoopWidth1:

		; update las RLCi and ner
		mov [RLCi + er*COMPV_YASM_INT16_SZ_BYTES], word widthw
		movzx X, byte [Xi + (width - 1)*COMPV_YASM_UINT8_SZ_BYTES] ; X will contain nerj
		sub word [RLCi + er*COMPV_YASM_INT16_SZ_BYTES], word bw				
		and Xw, 1
		add Xw, erw
		mov [ner], word Xw
		add ner_sumd, dword Xd
		cmp ner_maxw, Xw
		lea ner, [ner + 1*COMPV_YASM_INT16_SZ_BYTES]
		cmovl ner_max, X
		add RLCi, arg(3)
		dec qword arg(10)
		lea Xi, [Xi + Xi_stride*1]
		lea ERi, [ERi + ERi_stride*1]
		jnz .LoopHeight
	.EndOf_LoopHeight:

	mov rax, arg(7)
	mov rdx, arg(8)
	mov [rax], word ner_maxw
	mov [rdx], dword ner_sumd

	%undef Xi			
	%undef RLCi		
	%undef ERi			
	%undef width
	%undef widthw
	%undef i			
	%undef b			
	%undef bw		
	%undef er			
	%undef erw
	%undef width4
	%undef X			
	%undef Xb			
	%undef Xw
	%undef Xd
	%undef ner
	%undef ner_sum
	%undef ner_sumd
	%undef ner_max
	%undef ner_maxw
	%undef Xi_stride	
	%undef ERi_stride	

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
			movzx j0, word [RLCi + (er-1)*COMPV_YASM_INT16_SZ_BYTES] ; use movzx once to clear j0d
			movzx j1, word [RLCi + er*COMPV_YASM_INT16_SZ_BYTES] ; use movzx once to clear j1d
			cmp j0w, 0
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
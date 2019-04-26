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

global sym(CompVBitsLogicalAnd_8u_Asm_X64_SSE2)
global sym(CompVBitsLogicalNotAnd_8u_Asm_X64_SSE2)
global sym(CompVBitsLogicalNot_8u_Asm_X64_SSE2)
global sym(CompVBitsLogicalXorVt_8u_Asm_X64_SSE2)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* Aptr
; arg(1) -> COMPV_ALIGNED(SSE) const uint8_t* Bptr
; arg(2) -> uint8_t* Rptr
; arg(3) -> compv_uscalar_t width
; arg(4) -> COMPV_ALIGNED(SSE) compv_uscalar_t height
; arg(5) -> COMPV_ALIGNED(SSE) compv_uscalar_t Astride
; arg(6) -> COMPV_ALIGNED(SSE) compv_uscalar_t Bstride
; arg(7) -> COMPV_ALIGNED(SSE) compv_uscalar_t Rstride
sym(CompVBitsLogicalAnd_8u_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	%define Aptr		rax
	%define Bptr		rcx
	%define Rptr		rdx
	%define width		rbx
	%define height		rsi
	%define Astride		rdi
	%define Bstride		r8
	%define Rstride		r9
	%define width64		r10
	%define i			r11

	mov Aptr, arg(0)
	mov Bptr, arg(1)
	mov Rptr, arg(2)
	mov width, arg(3)
	mov height, arg(4)
	mov Astride, arg(5)
	mov Bstride, arg(6)
	mov Rstride, arg(7)
	mov width64, width
	and width64, -64

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 0; j < height; ++j) 
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width64; i += 64)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		test width64, width64
		je .LoopWidth16
		.LoopWidth64:
			movdqa xmm0, [Aptr + (i + 0)*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa xmm1, [Aptr + (i + 16)*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa xmm2, [Aptr + (i + 32)*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa xmm3, [Aptr + (i + 48)*COMPV_YASM_UINT8_SZ_BYTES]
			pand xmm0, [Bptr + (i + 0)*COMPV_YASM_UINT8_SZ_BYTES]
			pand xmm1, [Bptr + (i + 16)*COMPV_YASM_UINT8_SZ_BYTES]
			pand xmm2, [Bptr + (i + 32)*COMPV_YASM_UINT8_SZ_BYTES]
			pand xmm3, [Bptr + (i + 48)*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa [Rptr + (i + 0)*COMPV_YASM_UINT8_SZ_BYTES], xmm0
			movdqa [Rptr + (i + 16)*COMPV_YASM_UINT8_SZ_BYTES], xmm1
			movdqa [Rptr + (i + 32)*COMPV_YASM_UINT8_SZ_BYTES], xmm2
			movdqa [Rptr + (i + 48)*COMPV_YASM_UINT8_SZ_BYTES], xmm3
			add i, 64
			cmp i, width64
			jl .LoopWidth64
		.EndOf_LoopWidth64:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_LoopWidth16
		.LoopWidth16:
			movdqa xmm0, [Aptr + (i + 0)*COMPV_YASM_UINT8_SZ_BYTES]
			pand xmm0, [Bptr + (i + 0)*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa [Rptr + (i + 0)*COMPV_YASM_UINT8_SZ_BYTES], xmm0
			add i, 16
			cmp i, width
			jl .LoopWidth16
		.EndOf_LoopWidth16:

		dec height
		lea Rptr, [Rptr + Rstride]
		lea Aptr, [Aptr + Astride]
		lea Bptr, [Bptr + Bstride]
		jnz .LoopHeight
	.EndOf_LoopHeight:

	%undef Aptr		
	%undef Bptr		
	%undef Rptr		
	%undef width		
	%undef height		
	%undef Astride		
	%undef Bstride		
	%undef Rstride		
	%undef width64		
	%undef i			

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* Aptr
; arg(1) -> COMPV_ALIGNED(SSE) const uint8_t* Bptr
; arg(2) -> uint8_t* Rptr
; arg(3) -> compv_uscalar_t width
; arg(4) -> COMPV_ALIGNED(SSE) compv_uscalar_t height
; arg(5) -> COMPV_ALIGNED(SSE) compv_uscalar_t Astride
; arg(6) -> COMPV_ALIGNED(SSE) compv_uscalar_t Bstride
; arg(7) -> COMPV_ALIGNED(SSE) compv_uscalar_t Rstride
sym(CompVBitsLogicalNotAnd_8u_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	%define Aptr		rax
	%define Bptr		rcx
	%define Rptr		rdx
	%define width		rbx
	%define height		rsi
	%define Astride		rdi
	%define Bstride		r8
	%define Rstride		r9
	%define width64		r10
	%define i			r11

	mov Aptr, arg(0)
	mov Bptr, arg(1)
	mov Rptr, arg(2)
	mov width, arg(3)
	mov height, arg(4)
	mov Astride, arg(5)
	mov Bstride, arg(6)
	mov Rstride, arg(7)
	mov width64, width
	and width64, -64

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 0; j < height; ++j) 
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width64; i += 64)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		test width64, width64
		je .LoopWidth16
		.LoopWidth64:
			movdqa xmm0, [Aptr + (i + 0)*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa xmm1, [Aptr + (i + 16)*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa xmm2, [Aptr + (i + 32)*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa xmm3, [Aptr + (i + 48)*COMPV_YASM_UINT8_SZ_BYTES]
			pandn xmm0, [Bptr + (i + 0)*COMPV_YASM_UINT8_SZ_BYTES]
			pandn xmm1, [Bptr + (i + 16)*COMPV_YASM_UINT8_SZ_BYTES]
			pandn xmm2, [Bptr + (i + 32)*COMPV_YASM_UINT8_SZ_BYTES]
			pandn xmm3, [Bptr + (i + 48)*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa [Rptr + (i + 0)*COMPV_YASM_UINT8_SZ_BYTES], xmm0
			movdqa [Rptr + (i + 16)*COMPV_YASM_UINT8_SZ_BYTES], xmm1
			movdqa [Rptr + (i + 32)*COMPV_YASM_UINT8_SZ_BYTES], xmm2
			movdqa [Rptr + (i + 48)*COMPV_YASM_UINT8_SZ_BYTES], xmm3
			add i, 64
			cmp i, width64
			jl .LoopWidth64
		.EndOf_LoopWidth64:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_LoopWidth16
		.LoopWidth16:
			movdqa xmm0, [Aptr + (i + 0)*COMPV_YASM_UINT8_SZ_BYTES]
			pandn xmm0, [Bptr + (i + 0)*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa [Rptr + (i + 0)*COMPV_YASM_UINT8_SZ_BYTES], xmm0
			add i, 16
			cmp i, width
			jl .LoopWidth16
		.EndOf_LoopWidth16:

		dec height
		lea Rptr, [Rptr + Rstride]
		lea Aptr, [Aptr + Astride]
		lea Bptr, [Bptr + Bstride]
		jnz .LoopHeight
	.EndOf_LoopHeight:

	%undef Aptr		
	%undef Bptr		
	%undef Rptr		
	%undef width		
	%undef height		
	%undef Astride		
	%undef Bstride		
	%undef Rstride		
	%undef width64		
	%undef i			

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* Aptr
; arg(1) -> uint8_t* Rptr
; arg(2) -> compv_uscalar_t width
; arg(3) -> COMPV_ALIGNED(SSE) compv_uscalar_t height
; arg(4) -> COMPV_ALIGNED(SSE) compv_uscalar_t Astride
; arg(5) -> COMPV_ALIGNED(SSE) compv_uscalar_t Rstride
sym(CompVBitsLogicalNot_8u_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	push rsi
	;; end prolog ;;

	%define Aptr		rax
	%define Rptr		rcx
	%define width		rdx
	%define height		r8
	%define Astride		r9
	%define Rstride		r10
	%define width64		r11
	%define i			rsi

	mov Aptr, arg(0)
	mov Rptr, arg(1)
	mov width, arg(2)
	mov height, arg(3)
	mov Astride, arg(4)
	mov Rstride, arg(5)
	mov width64, width
	and width64, -64

	pcmpeqb xmm5, xmm5 ; vecFF

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 0; j < height; ++j) 
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width64; i += 64)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		test width64, width64
		je .LoopWidth16
		.LoopWidth64:
			movdqa xmm0, [Aptr + (i + 0)*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa xmm1, [Aptr + (i + 16)*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa xmm2, [Aptr + (i + 32)*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa xmm3, [Aptr + (i + 48)*COMPV_YASM_UINT8_SZ_BYTES]
			pxor xmm0, xmm5
			pxor xmm1, xmm5
			pxor xmm2, xmm5
			pxor xmm3, xmm5
			movdqa [Rptr + (i + 0)*COMPV_YASM_UINT8_SZ_BYTES], xmm0
			movdqa [Rptr + (i + 16)*COMPV_YASM_UINT8_SZ_BYTES], xmm1
			movdqa [Rptr + (i + 32)*COMPV_YASM_UINT8_SZ_BYTES], xmm2
			movdqa [Rptr + (i + 48)*COMPV_YASM_UINT8_SZ_BYTES], xmm3
			add i, 64
			cmp i, width64
			jl .LoopWidth64
		.EndOf_LoopWidth64:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_LoopWidth16
		.LoopWidth16:
			movdqa xmm0, [Aptr + (i + 0)*COMPV_YASM_UINT8_SZ_BYTES]
			pxor xmm0, xmm5
			movdqa [Rptr + (i + 0)*COMPV_YASM_UINT8_SZ_BYTES], xmm0
			add i, 16
			cmp i, width
			jl .LoopWidth16
		.EndOf_LoopWidth16:

		dec height
		lea Rptr, [Rptr + Rstride]
		lea Aptr, [Aptr + Astride]
		jnz .LoopHeight
	.EndOf_LoopHeight:

	%undef Aptr			
	%undef Rptr		
	%undef width		
	%undef height		
	%undef Astride		
	%undef Rstride		
	%undef width64		
	%undef i			

	;; begin epilog ;;
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* Aptr
; arg(1) -> COMPV_ALIGNED(SSE) const uint8_t* A_Minus1_ptr
; arg(2) -> uint8_t* Rptr
; arg(3) -> compv_uscalar_t width
; arg(4) -> COMPV_ALIGNED(SSE) compv_uscalar_t height
; arg(5) -> COMPV_ALIGNED(SSE) compv_uscalar_t Astride
; arg(6) -> COMPV_ALIGNED(SSE) compv_uscalar_t Rstride
sym(CompVBitsLogicalXorVt_8u_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	%define Aptr			rax
	%define A_Minus1_ptr	rcx
	%define Rptr			rdx
	%define width			rbx
	%define height			rsi
	%define Astride			rdi
	%define Rstride			r8
	%define width64			r9
	%define i				r10

	mov Aptr, arg(0)
	mov A_Minus1_ptr, arg(1)
	mov Rptr, arg(2)
	mov width, arg(3)
	mov height, arg(4)
	mov Astride, arg(5)
	mov Rstride, arg(6)
	mov width64, width
	and width64, -64

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 0; j < height; ++j) 
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width64; i += 64)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		test width64, width64
		je .LoopWidth16
		.LoopWidth64:
			movdqa xmm0, [Aptr + (i + 0)*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa xmm1, [Aptr + (i + 16)*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa xmm2, [Aptr + (i + 32)*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa xmm3, [Aptr + (i + 48)*COMPV_YASM_UINT8_SZ_BYTES]
			pxor xmm0, [A_Minus1_ptr + (i + 0)*COMPV_YASM_UINT8_SZ_BYTES]
			pxor xmm1, [A_Minus1_ptr + (i + 16)*COMPV_YASM_UINT8_SZ_BYTES]
			pxor xmm2, [A_Minus1_ptr + (i + 32)*COMPV_YASM_UINT8_SZ_BYTES]
			pxor xmm3, [A_Minus1_ptr + (i + 48)*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa [Rptr + (i + 0)*COMPV_YASM_UINT8_SZ_BYTES], xmm0
			movdqa [Rptr + (i + 16)*COMPV_YASM_UINT8_SZ_BYTES], xmm1
			movdqa [Rptr + (i + 32)*COMPV_YASM_UINT8_SZ_BYTES], xmm2
			movdqa [Rptr + (i + 48)*COMPV_YASM_UINT8_SZ_BYTES], xmm3
			add i, 64
			cmp i, width64
			jl .LoopWidth64
		.EndOf_LoopWidth64:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_LoopWidth16
		.LoopWidth16:
			movdqa xmm0, [Aptr + (i + 0)*COMPV_YASM_UINT8_SZ_BYTES]
			pxor xmm0, [A_Minus1_ptr + (i + 0)*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa [Rptr + (i + 0)*COMPV_YASM_UINT8_SZ_BYTES], xmm0
			add i, 16
			cmp i, width
			jl .LoopWidth16
		.EndOf_LoopWidth16:

		dec height
		lea Rptr, [Rptr + Rstride]
		lea Aptr, [Aptr + Astride]
		lea A_Minus1_ptr, [A_Minus1_ptr + Astride]
		jnz .LoopHeight
	.EndOf_LoopHeight:

	%undef Aptr		
	%undef A_Minus1_ptr		
	%undef Rptr		
	%undef width		
	%undef height		
	%undef Astride		
	%undef Rstride		
	%undef width64		
	%undef i			

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret


%endif ; COMPV_YASM_ABI_IS_64BIT

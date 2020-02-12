;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(CompVMathOpSubSub_32f32f32f_Asm_X64_SSE2)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const compv_float32_t* Aptr
; arg(1) -> COMPV_ALIGNED(SSE) const compv_float32_t* Bptr
; arg(2) -> COMPV_ALIGNED(SSE) compv_float32_t* Rptr
; arg(3) -> const compv_uscalar_t width
; arg(4) -> const compv_uscalar_t height
; arg(5) -> COMPV_ALIGNED(SSE) const compv_uscalar_t Astride
; arg(6) -> COMPV_ALIGNED(SSE) const compv_uscalar_t Bstride
; arg(7) -> COMPV_ALIGNED(SSE) const compv_uscalar_t Rstride
sym(CompVMathOpSubSub_32f32f32f_Asm_X64_SSE2):
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
	%define width		rsi
	%define height		rdi
	%define Astride		rbx
	%define Bstride		r8
	%define Rstride		r9

	%define width16		r10
	%define i			r11

	mov Aptr, arg(0)
	mov Bptr, arg(1)
	mov Rptr, arg(2)
	mov width, arg(3)
	mov height, arg(4)
	mov Astride, arg(5)
	mov Bstride, arg(6)
	mov Rstride, arg(7) 

	mov width16, width
	and width16, -16

	lea Astride, [Astride * COMPV_YASM_FLOAT32_SZ_BYTES]
	lea Bstride, [Bstride * COMPV_YASM_FLOAT32_SZ_BYTES]
	lea Rstride, [Rstride * COMPV_YASM_FLOAT32_SZ_BYTES]

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width16; i += 16) 
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		test width16, width16
		jz .EndOf_LoopWidth16
		.LoopWidth16:
			movaps xmm0, [Aptr + (i + 0)*COMPV_YASM_FLOAT32_SZ_BYTES]
			movaps xmm1, [Aptr + (i + 4)*COMPV_YASM_FLOAT32_SZ_BYTES]
			movaps xmm2, [Aptr + (i + 8)*COMPV_YASM_FLOAT32_SZ_BYTES]
			movaps xmm3, [Aptr + (i + 12)*COMPV_YASM_FLOAT32_SZ_BYTES]
			subps xmm0, [Bptr + (i + 0)*COMPV_YASM_FLOAT32_SZ_BYTES]
			subps xmm1, [Bptr + (i + 4)*COMPV_YASM_FLOAT32_SZ_BYTES]
			subps xmm2, [Bptr + (i + 8)*COMPV_YASM_FLOAT32_SZ_BYTES]
			subps xmm3, [Bptr + (i + 12)*COMPV_YASM_FLOAT32_SZ_BYTES]
			movaps [Rptr + (i + 0)*COMPV_YASM_FLOAT32_SZ_BYTES], xmm0
			movaps [Rptr + (i + 4)*COMPV_YASM_FLOAT32_SZ_BYTES], xmm1
			movaps [Rptr + (i + 8)*COMPV_YASM_FLOAT32_SZ_BYTES], xmm2
			movaps [Rptr + (i + 12)*COMPV_YASM_FLOAT32_SZ_BYTES], xmm3
			add i, 16
			cmp i, width16
			jl .LoopWidth16
		.EndOf_LoopWidth16:

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width; i += 4)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_LoopWidth4
		.LoopWidth4:
			movaps xmm0, [Aptr + (i + 0)*COMPV_YASM_FLOAT32_SZ_BYTES]
			subps xmm0, [Bptr + (i + 0)*COMPV_YASM_FLOAT32_SZ_BYTES]
			movaps [Rptr + (i + 0)*COMPV_YASM_FLOAT32_SZ_BYTES], xmm0
			add i, 4
			cmp i, width
			jl .LoopWidth4
		.EndOf_LoopWidth4:

		dec height
		lea Aptr, [Aptr + Astride]
		lea Bptr, [Bptr + Bstride]
		lea Rptr, [Rptr + Rstride]
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

	%undef width16
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

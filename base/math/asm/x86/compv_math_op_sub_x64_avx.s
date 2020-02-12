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

section .data

section .text

global sym(CompVMathOpSubSub_32f32f32f_Asm_X64_AVX)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(AVX) const compv_float32_t* Aptr
; arg(1) -> COMPV_ALIGNED(AVX) const compv_float32_t* Bptr
; arg(2) -> COMPV_ALIGNED(AVX) compv_float32_t* Rptr
; arg(3) -> const compv_uscalar_t width
; arg(4) -> const compv_uscalar_t height
; arg(5) -> COMPV_ALIGNED(AVX) const compv_uscalar_t Astride
; arg(6) -> COMPV_ALIGNED(AVX) const compv_uscalar_t Bstride
; arg(7) -> COMPV_ALIGNED(AVX) const compv_uscalar_t Rstride
sym(CompVMathOpSubSub_32f32f32f_Asm_X64_AVX):
	vzeroupper
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

	%define width32		r10
	%define i			r11

	mov Aptr, arg(0)
	mov Bptr, arg(1)
	mov Rptr, arg(2)
	mov width, arg(3)
	mov height, arg(4)
	mov Astride, arg(5)
	mov Bstride, arg(6)
	mov Rstride, arg(7) 

	mov width32, width
	and width32, -32

	lea Astride, [Astride * COMPV_YASM_FLOAT32_SZ_BYTES]
	lea Bstride, [Bstride * COMPV_YASM_FLOAT32_SZ_BYTES]
	lea Rstride, [Rstride * COMPV_YASM_FLOAT32_SZ_BYTES]

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width32; i += 32) 
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		test width32, width32
		jz .EndOf_LoopWidth16
		.LoopWidth32:
			vmovaps ymm0, [Aptr + (i + 0)*COMPV_YASM_FLOAT32_SZ_BYTES]
			vmovaps ymm1, [Aptr + (i + 8)*COMPV_YASM_FLOAT32_SZ_BYTES]
			vmovaps ymm2, [Aptr + (i + 16)*COMPV_YASM_FLOAT32_SZ_BYTES]
			vmovaps ymm3, [Aptr + (i + 24)*COMPV_YASM_FLOAT32_SZ_BYTES]
			vsubps ymm0, [Bptr + (i + 0)*COMPV_YASM_FLOAT32_SZ_BYTES]
			vsubps ymm1, [Bptr + (i + 8)*COMPV_YASM_FLOAT32_SZ_BYTES]
			vsubps ymm2, [Bptr + (i + 16)*COMPV_YASM_FLOAT32_SZ_BYTES]
			vsubps ymm3, [Bptr + (i + 24)*COMPV_YASM_FLOAT32_SZ_BYTES]
			vmovaps [Rptr + (i + 0)*COMPV_YASM_FLOAT32_SZ_BYTES], ymm0
			vmovaps [Rptr + (i + 8)*COMPV_YASM_FLOAT32_SZ_BYTES], ymm1
			vmovaps [Rptr + (i + 16)*COMPV_YASM_FLOAT32_SZ_BYTES], ymm2
			vmovaps [Rptr + (i + 24)*COMPV_YASM_FLOAT32_SZ_BYTES], ymm3
			add i, 32
			cmp i, width32
			jl .LoopWidth32
		.EndOf_LoopWidth16:

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width; i += 8)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_LoopWidth8
		.LoopWidth8:
			vmovaps ymm0, [Aptr + (i + 0)*COMPV_YASM_FLOAT32_SZ_BYTES]
			vsubps ymm0, [Bptr + (i + 0)*COMPV_YASM_FLOAT32_SZ_BYTES]
			vmovaps [Rptr + (i + 0)*COMPV_YASM_FLOAT32_SZ_BYTES], ymm0
			add i, 8
			cmp i, width
			jl .LoopWidth8
		.EndOf_LoopWidth8:

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

	%undef width32
	%undef i

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret

%endif ; COMPV_YASM_ABI_IS_64BIT

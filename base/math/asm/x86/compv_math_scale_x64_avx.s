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

global sym(CompVMathScaleScale_64f64f_Asm_X64_AVX)
global sym(CompVMathScaleScale_32f32f_Asm_X64_AVX)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const compv_float64_t* ptrIn
; arg(1) -> compv_float64_t* ptrOut
; arg(2) -> const compv_uscalar_t width
; arg(3) -> const compv_uscalar_t height
; arg(4) -> const compv_uscalar_t stride
; arg(5) -> const compv_float64_t* s1
sym(CompVMathScaleScale_64f64f_Asm_X64_AVX):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	COMPV_YASM_SAVE_YMM 8
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	%define ptrIn		rax
	%define ptrOut		rcx
	%define width		rdx
	%define height		rbx
	%define stride		rsi
	%define s1			rdi

	%define width32		r8
	%define width4		r9
	%define i			r10

	mov ptrIn, arg(0)
	mov ptrOut, arg(1)
	mov width, arg(2)
	mov height, arg(3)
	mov stride, arg(4)
	mov s1, arg(5)

	mov width32, width
	mov width4, width
	and width32, -32
	and width4, -4

	shl stride, COMPV_YASM_FLOAT64_SHIFT_BYTES

	vbroadcastsd ymm8, [s1]

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width32; i += 32)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		test width32, width32
		jz .EndOf_LoopWidth32
		.LoopWidth32:
			vmulpd ymm0, ymm8, [ptrIn + ((i + 0) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			vmulpd ymm1, ymm8, [ptrIn + ((i + 4) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			vmulpd ymm2, ymm8, [ptrIn + ((i + 8) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			vmulpd ymm3, ymm8, [ptrIn + ((i + 12) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			vmulpd ymm4, ymm8, [ptrIn + ((i + 16) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			vmulpd ymm5, ymm8, [ptrIn + ((i + 20) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			vmulpd ymm6, ymm8, [ptrIn + ((i + 24) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			vmulpd ymm7, ymm8, [ptrIn + ((i + 28) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			add i, 32
			cmp i, width32
			vmovupd [ptrOut + ((i + 0 - 32) * COMPV_YASM_FLOAT64_SZ_BYTES)], ymm0
			vmovupd [ptrOut + ((i + 4 - 32) * COMPV_YASM_FLOAT64_SZ_BYTES)], ymm1
			vmovupd [ptrOut + ((i + 8 - 32) * COMPV_YASM_FLOAT64_SZ_BYTES)], ymm2
			vmovupd [ptrOut + ((i + 12 - 32) * COMPV_YASM_FLOAT64_SZ_BYTES)], ymm3
			vmovupd [ptrOut + ((i + 16 - 32) * COMPV_YASM_FLOAT64_SZ_BYTES)], ymm4
			vmovupd [ptrOut + ((i + 20 - 32) * COMPV_YASM_FLOAT64_SZ_BYTES)], ymm5
			vmovupd [ptrOut + ((i + 24 - 32) * COMPV_YASM_FLOAT64_SZ_BYTES)], ymm6
			vmovupd [ptrOut + ((i + 28 - 32) * COMPV_YASM_FLOAT64_SZ_BYTES)], ymm7
			jl .LoopWidth32
		.EndOf_LoopWidth32:

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width4; i += 4)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width4
		jge .EndOf_LoopWidth4
		.LoopWidth4:
			vmulpd ymm0, ymm8, [ptrIn + ((i + 0) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			add i, 4
			cmp i, width4
			vmovupd [ptrOut + ((i + 0 - 4) * COMPV_YASM_FLOAT64_SZ_BYTES)], ymm0
			jl .LoopWidth4
		.EndOf_LoopWidth4:

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width; i += 1)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_LoopWidth1
		.LoopWidth1:
			vmulsd xmm0, xmm8, [ptrIn + ((i + 0) * COMPV_YASM_FLOAT64_SZ_BYTES)]
			inc i
			cmp i, width
			vmovsd [ptrOut + ((i + 0 - 1) * COMPV_YASM_FLOAT64_SZ_BYTES)], xmm0
			jl .LoopWidth1
		.EndOf_LoopWidth1:
		
		dec height
		lea ptrIn, [ptrIn + stride]
		lea ptrOut, [ptrOut + stride]
		jnz .LoopHeight
	.EndOf_LoopHeight:

	%undef ptrIn		
	%undef ptrOut		
	%undef width		
	%undef height		
	%undef stride		
	%undef s1			

	%undef width32		
	%undef width4		
	%undef i			

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_YMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const compv_float32_t* ptrIn
; arg(1) -> COMPV_ALIGNED(SSE) compv_float32_t* ptrOut
; arg(2) -> const compv_uscalar_t width
; arg(3) -> const compv_uscalar_t height
; arg(4) -> COMPV_ALIGNED(SSE) const compv_uscalar_t stride
; arg(5) -> const compv_float64_t* s1
sym(CompVMathScaleScale_32f32f_Asm_X64_AVX):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	COMPV_YASM_SAVE_YMM 8
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	%define ptrIn		rax
	%define ptrOut		rcx
	%define width		rdx
	%define height		rbx
	%define stride		rsi
	%define s1			rdi

	%define width64		r8
	%define width8		r9
	%define i			r10

	mov ptrIn, arg(0)
	mov ptrOut, arg(1)
	mov width, arg(2)
	mov height, arg(3)
	mov stride, arg(4)
	mov s1, arg(5)

	mov width64, width
	mov width8, width
	and width64, -64
	and width8, -8

	shl stride, COMPV_YASM_FLOAT32_SHIFT_BYTES

	vbroadcastss ymm8, [s1]

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width64; i += 64)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		test width64, width64
		jz .EndOf_LoopWidth64
		.LoopWidth64:
			vmulps ymm0, ymm8, [ptrIn + ((i + 0) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			vmulps ymm1, ymm8, [ptrIn + ((i + 8) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			vmulps ymm2, ymm8, [ptrIn + ((i + 16) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			vmulps ymm3, ymm8, [ptrIn + ((i + 24) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			vmulps ymm4, ymm8, [ptrIn + ((i + 32) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			vmulps ymm5, ymm8, [ptrIn + ((i + 40) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			vmulps ymm6, ymm8, [ptrIn + ((i + 48) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			vmulps ymm7, ymm8, [ptrIn + ((i + 56) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			vmovaps [ptrOut + ((i + 0) * COMPV_YASM_FLOAT32_SZ_BYTES)], ymm0
			vmovaps [ptrOut + ((i + 8) * COMPV_YASM_FLOAT32_SZ_BYTES)], ymm1
			vmovaps [ptrOut + ((i + 16) * COMPV_YASM_FLOAT32_SZ_BYTES)], ymm2
			vmovaps [ptrOut + ((i + 24) * COMPV_YASM_FLOAT32_SZ_BYTES)], ymm3
			vmovaps [ptrOut + ((i + 32) * COMPV_YASM_FLOAT32_SZ_BYTES)], ymm4
			vmovaps [ptrOut + ((i + 40) * COMPV_YASM_FLOAT32_SZ_BYTES)], ymm5
			vmovaps [ptrOut + ((i + 48) * COMPV_YASM_FLOAT32_SZ_BYTES)], ymm6
			vmovaps [ptrOut + ((i + 56) * COMPV_YASM_FLOAT32_SZ_BYTES)], ymm7
			add i, 64
			cmp i, width64
			jl .LoopWidth64
		.EndOf_LoopWidth64:

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width8; i += 8)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width8
		jge .EndOf_LoopWidth8
		.LoopWidth8:
			vmulps ymm0, ymm8, [ptrIn + ((i + 0) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			add i, 8
			cmp i, width8
			vmovaps [ptrOut + ((i + 0 - 8) * COMPV_YASM_FLOAT32_SZ_BYTES)], ymm0
			jl .LoopWidth8
		.EndOf_LoopWidth8:

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width; i += 1)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_LoopWidth1
		.LoopWidth1:
			vmulss xmm0, xmm8, [ptrIn + ((i + 0) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			inc i
			cmp i, width
			vmovss [ptrOut + ((i + 0 - 1) * COMPV_YASM_FLOAT32_SZ_BYTES)], xmm0
			jl .LoopWidth1
		.EndOf_LoopWidth1:
		
		dec height
		lea ptrIn, [ptrIn + stride]
		lea ptrOut, [ptrOut + stride]
		jnz .LoopHeight
	.EndOf_LoopHeight:

	%undef ptrIn		
	%undef ptrOut		
	%undef width		
	%undef height		
	%undef stride		
	%undef s1			

	%undef width64
	%undef width8
	%undef i

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_YMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret


%endif ; COMPV_YASM_ABI_IS_64BIT

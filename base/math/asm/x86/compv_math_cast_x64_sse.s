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

global sym(CompVMathCastProcess_static_8u32f_Asm_X64_SSE2)
global sym(CompVMathCastProcess_static_pixel8_32f_Asm_X64_SSE2)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* src,
; arg(1) -> COMPV_ALIGNED(SSE) compv_float32_t* dst,
; arg(2) -> compv_uscalar_t width,
; arg(3) -> compv_uscalar_t height,
; arg(4) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
sym(CompVMathCastProcess_static_8u32f_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	;; end prolog ;;

	%define src					rax
	%define dst					rcx
	%define width				rdx
	%define height				r8
	%define stride				r9
	%define i					r10
	%define stridef				r11

	mov src, arg(0)
	mov dst, arg(1)
	mov width, arg(2)
	mov height, arg(3)
	mov stride, arg(4)
	lea stridef, [stride * COMPV_YASM_FLOAT32_SZ_BYTES]

	pxor xmm4, xmm4

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (compv_uscalar_t i = 0; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		.LoopWidth:
			movdqa xmm1, [src + i*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa xmm3, xmm1
			punpcklbw xmm1, xmm4
			punpckhbw xmm3, xmm4
			movdqa xmm0, xmm1
			movdqa xmm2, xmm3
			punpcklwd xmm0, xmm4
			punpckhwd xmm1, xmm4
			punpcklwd xmm2, xmm4
			punpckhwd xmm3, xmm4
			cvtdq2ps xmm0, xmm0
			cvtdq2ps xmm1, xmm1
			cvtdq2ps xmm2, xmm2
			cvtdq2ps xmm3, xmm3
			movaps [dst + (i + 0)*COMPV_YASM_FLOAT32_SZ_BYTES], xmm0
			movaps [dst + (i + 4)*COMPV_YASM_FLOAT32_SZ_BYTES], xmm1
			movaps [dst + (i + 8)*COMPV_YASM_FLOAT32_SZ_BYTES], xmm2
			movaps [dst + (i + 12)*COMPV_YASM_FLOAT32_SZ_BYTES], xmm3
			add i, 16
			cmp i, width
			jl .LoopWidth
		.EndOf_LoopWidth:
				
		dec height
		lea src, [src + stride]
		lea dst, [dst + stridef]
		jnz .LoopHeight
	.EndOf_LoopHeight

	%undef src					
	%undef dst					
	%undef width				
	%undef height				
	%undef stride				
	%undef i
	%undef stridef		

	;; begin epilog ;;
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const compv_float32_t* src,
; arg(1) -> COMPV_ALIGNED(SSE) uint8_t* dst,
; arg(2) -> compv_uscalar_t width,
; arg(3) -> compv_uscalar_t height,
; arg(4) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
sym(CompVMathCastProcess_static_pixel8_32f_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	;; end prolog ;;

	%define src					rax
	%define dst					rcx
	%define width				rdx
	%define height				r8
	%define stride				r9
	%define i					r10
	%define stridef				r11

	mov src, arg(0)
	mov dst, arg(1)
	mov width, arg(2)
	mov height, arg(3)
	mov stride, arg(4)
	lea stridef, [stride * COMPV_YASM_FLOAT32_SZ_BYTES]

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (compv_uscalar_t i = 0; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		.LoopWidth:
			cvttps2dq xmm0, [src + (i + 0) * COMPV_YASM_FLOAT32_SZ_BYTES]
			cvttps2dq xmm1, [src + (i + 4) * COMPV_YASM_FLOAT32_SZ_BYTES]
			cvttps2dq xmm2, [src + (i + 8) * COMPV_YASM_FLOAT32_SZ_BYTES]
			cvttps2dq xmm3, [src + (i + 12) * COMPV_YASM_FLOAT32_SZ_BYTES]
			packssdw xmm0, xmm1
			packssdw xmm2, xmm3
			packuswb xmm0, xmm2
			movdqa [dst + i*COMPV_YASM_UINT8_SZ_BYTES], xmm0
			add i, 16
			cmp i, width
			jl .LoopWidth
		.EndOf_LoopWidth:
				
		dec height
		lea src, [src + stridef]
		lea dst, [dst + stride]
		jnz .LoopHeight
	.EndOf_LoopHeight

	%undef src					
	%undef dst					
	%undef width				
	%undef height				
	%undef stride				
	%undef i
	%undef stridef		

	;; begin epilog ;;
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
	
%endif ; COMPV_YASM_ABI_IS_64BIT

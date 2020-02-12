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

global sym(CompVGradientFastGradX_8u16s_Asm_X64_SSE2)
global sym(CompVGradientFastGradX_8u32f_Asm_X64_SSE2)
global sym(CompVGradientFastGradY_8u16s_Asm_X64_SSE2)
global sym(CompVGradientFastGradY_8u32f_Asm_X64_SSE2)

section .data

section .text

%define ARG_OUT			1
%define ARG_OUT_INT16	0
%define ARG_OUT_FLOAT32	1

%define ARG_DIR			2
%define ARG_DIR_X		0
%define ARG_DIR_Y		1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* input
; arg(1) -> COMPV_ALIGNED(SSE) compv_float32_t* grad
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
; %1 -> ARG_OUT
; %2 -> ARG_DIR
%macro CompVGradientFastGrad_8uXX_Macro_X64_SSE2 2
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	%if %2 == ARG_DIR_Y
		push r12
	%endif
	; end prolog

	%define input		rax
	%define grad		rcx
	%define width		rdx
	%define height		r8
	%define stride		r9
	%define i			r10
	%define stridef		r11	
	%if %2 == ARG_DIR_Y
		%define input2	r12
	%endif

	mov input, arg(0)
	mov grad, arg(1)
	mov width, arg(2)
	mov height, arg(3)
	mov stride, arg(4)
	mov i, arg(5)

	%if %1 == ARG_OUT_FLOAT32
		lea stridef, [stride * COMPV_YASM_FLOAT32_SZ_BYTES]
	%else
		lea stridef, [stride * COMPV_YASM_INT16_SZ_BYTES]
	%endif

	%if %2 == ARG_DIR_Y
		mov input2, input
		sub input, stride
		add input2, stride
	%endif

	pxor xmm4, xmm4 ; vecZero

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (compv_uscalar_t i = 0; i < width; i += 16) 
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		.LoopWidth:
			%if %2 == ARG_DIR_Y
				movdqu xmm2, [input + i*COMPV_YASM_UINT8_SZ_BYTES]
				movdqu xmm0, [input2 + i*COMPV_YASM_UINT8_SZ_BYTES]
			%else
				movdqu xmm2, [input + (i - 1)*COMPV_YASM_UINT8_SZ_BYTES]
				movdqu xmm0, [input + (i + 1)*COMPV_YASM_UINT8_SZ_BYTES]
			%endif
			movdqa xmm3, xmm2
			punpcklbw xmm2, xmm4
			punpckhbw xmm3, xmm4
			movdqa xmm1, xmm0
			punpcklbw xmm0, xmm4
			punpckhbw xmm1, xmm4
			psubw xmm0, xmm2
			psubw xmm1, xmm3
			%if %1 == ARG_OUT_FLOAT32
				movdqa xmm2, xmm0
				movdqa xmm3, xmm1
				punpcklwd xmm0, xmm0
				punpckhwd xmm2, xmm2
				punpcklwd xmm1, xmm1
				punpckhwd xmm3, xmm3
				psrad xmm0, 16
				cvtdq2ps xmm0, xmm0
				psrad xmm2, 16
				cvtdq2ps xmm2, xmm2
				psrad xmm1, 16
				cvtdq2ps xmm1, xmm1
				psrad xmm3, 16
				cvtdq2ps xmm3, xmm3
				movaps [grad + (i + 0)*COMPV_YASM_FLOAT32_SZ_BYTES], xmm0
				movaps [grad + (i + 4)*COMPV_YASM_FLOAT32_SZ_BYTES], xmm2
				movaps [grad + (i + 8)*COMPV_YASM_FLOAT32_SZ_BYTES], xmm1
				movaps [grad + (i + 12)*COMPV_YASM_FLOAT32_SZ_BYTES], xmm3
			%else
				movdqa [grad + (i + 0)*COMPV_YASM_INT16_SZ_BYTES], xmm0
				movdqa [grad + (i + 8)*COMPV_YASM_INT16_SZ_BYTES], xmm1
			%endif
			add i, 16
			cmp i, width
			jl .LoopWidth
		.EndOf_LoopWidth:

		dec height
		lea input, [input + stride]
		%if %2 == ARG_DIR_Y
			lea input2, [input2 + stride]
		%endif
		lea grad, [grad + stridef]
		jnz .LoopHeight
	.EndOf_LoopHeight:

	%undef input	
	%undef grad		
	%undef width	
	%undef height	
	%undef stride	
	%undef i
	%undef stridef
	%if %2 == ARG_DIR_Y
		%undef input2
	%endif

	; begin epilog
	%if %2 == ARG_DIR_Y
		pop r12
	%endif
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVGradientFastGradX_8u16s_Asm_X64_SSE2):
	CompVGradientFastGrad_8uXX_Macro_X64_SSE2 ARG_OUT_INT16, ARG_DIR_X

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVGradientFastGradX_8u32f_Asm_X64_SSE2):
	CompVGradientFastGrad_8uXX_Macro_X64_SSE2 ARG_OUT_FLOAT32, ARG_DIR_X

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVGradientFastGradY_8u16s_Asm_X64_SSE2):
	CompVGradientFastGrad_8uXX_Macro_X64_SSE2 ARG_OUT_INT16, ARG_DIR_Y

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVGradientFastGradY_8u32f_Asm_X64_SSE2):
	CompVGradientFastGrad_8uXX_Macro_X64_SSE2 ARG_OUT_FLOAT32, ARG_DIR_Y

%endif ; COMPV_YASM_ABI_IS_64BIT

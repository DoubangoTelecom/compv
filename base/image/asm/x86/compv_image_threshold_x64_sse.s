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

global sym(CompVImageThresholdGlobal_8u8u_Asm_X64_SSE2)

section .data

section .text

; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* inPtr,
; arg(1) -> COMPV_ALIGNED(SSE) uint8_t* outPtr,
; arg(2) -> compv_uscalar_t width, 
; arg(3) -> compv_uscalar_t height,
; arg(4) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride,
; arg(5) -> compv_uscalar_t threshold
sym(CompVImageThresholdGlobal_8u8u_Asm_X64_SSE2)
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	;; end prolog ;;

	%define inPtr			rax
	%define outPtr			rcx
	%define width			rdx
	%define height			r8
	%define stride			r9
	%define i				r10
	%define width1			r11
	%define vec0			xmm0
	%define vec1			xmm1
	%define vec2			xmm2
	%define vec3			xmm3
	%define vecThreshold	xmm4
	%define vecMask			xmm5

	; vecThreshold ;
	movsx eax, byte ptr arg(5)
	xor eax, 0x80
	movd vecThreshold, eax  
	punpcklbw vecThreshold, vecThreshold  
	punpcklwd vecThreshold, vecThreshold  
	pshufd vecThreshold, vecThreshold, 0x00

	; vecMask ;
	mov eax, 0x80
	movd vecMask, eax  
	punpcklbw vecMask, vecMask  
	punpcklwd vecMask, vecMask  
	pshufd vecMask, vecMask, 0x00

	mov inPtr, arg(0)
	mov outPtr, arg(1)
	mov width, arg(2)
	mov height, arg(3)
	mov stride, arg(4)
	mov width1, width
	and width1, -64

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width1; i += 64)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		cmp i, width1
		jge .EndOf_LoopWidth64
		.LoopWidth64
			movdqa vec0, [inPtr + (i+0)*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa vec1, [inPtr + (i+16)*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa vec2, [inPtr + (i+32)*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa vec3, [inPtr + (i+48)*COMPV_YASM_UINT8_SZ_BYTES]
			pxor vec0, vecMask
			pxor vec1, vecMask
			pxor vec2, vecMask
			pxor vec3, vecMask
			lea i, [i + 64]
			pcmpgtb vec0, vecThreshold
			pcmpgtb vec1, vecThreshold
			pcmpgtb vec2, vecThreshold
			pcmpgtb vec3, vecThreshold
			cmp i, width1
			movdqa [outPtr + (i+0-64)*COMPV_YASM_UINT8_SZ_BYTES], vec0
			movdqa [outPtr + (i+16-64)*COMPV_YASM_UINT8_SZ_BYTES], vec1
			movdqa [outPtr + (i+32-64)*COMPV_YASM_UINT8_SZ_BYTES], vec2
			movdqa [outPtr + (i+48-64)*COMPV_YASM_UINT8_SZ_BYTES], vec3
			jl .LoopWidth64
		.EndOf_LoopWidth64


		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_LoopWidth16
		.LoopWidth16
			movdqa vec0, [inPtr + (i+0)*COMPV_YASM_UINT8_SZ_BYTES]
			lea i, [i + 16]
			pxor vec0, vecMask
			cmp i, width
			pcmpgtb vec0, vecThreshold
			movdqa [outPtr + (i+0-16)*COMPV_YASM_UINT8_SZ_BYTES], vec0
			jl .LoopWidth16
		.EndOf_LoopWidth16
		
		dec height
		lea inPtr, [inPtr + stride*COMPV_YASM_UINT8_SZ_BYTES]
		lea outPtr, [outPtr + stride*COMPV_YASM_UINT8_SZ_BYTES]
		jnz .LoopHeight
	.EndOf_LoopHeight

	%undef inPtr			
	%undef outPtr			
	%undef width			
	%undef height			
	%undef stride			
	%undef i
	%undef width1
	%undef vec0			
	%undef vec1			
	%undef vec2			
	%undef vec3			
	%undef vecThreshold	
	%undef vecMask			

	;; begin epilog ;;
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

%endif


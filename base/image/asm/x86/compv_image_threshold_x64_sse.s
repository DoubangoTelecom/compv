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

global sym(CompVImageThresholdGlobal_8u8u_Asm_X64_SSE2)
global sym(CompVImageThresholdOtsuSum_32s32s_Asm_X64_SSE41)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* inPtr,
; arg(1) -> COMPV_ALIGNED(SSE) uint8_t* outPtr,
; arg(2) -> compv_uscalar_t width, 
; arg(3) -> compv_uscalar_t height,
; arg(4) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride,
; arg(5) -> compv_uscalar_t threshold
sym(CompVImageThresholdGlobal_8u8u_Asm_X64_SSE2):
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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint32_t* ptr32uHistogram
; arg(1) -> COMPV_ALIGNED(SSE) uint32_t* sumA256
; arg(2) -> uint32_t* sumB1
sym(CompVImageThresholdOtsuSum_32s32s_Asm_X64_SSE41):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 3
	COMPV_YASM_SAVE_XMM 12
	;; end prolog ;;

	%define ptr32uHistogram	rax
	%define sumA256			rdx
	%define	sumB1			r8
	%define i				r9
	%define vecIndicesInc	xmm0
	%define vecIndices0		xmm1
	%define vecIndices1		xmm2
	%define vecIndices2		xmm3
	%define vecIndices3		xmm4
	%define vec0			xmm5
	%define vec1			xmm6
	%define vec2			xmm7
	%define vec3			xmm8
	%define vecSumB0		xmm9
	%define vecSumB1		xmm10
	%define vecSumB2		xmm11
	%define vecSumB3		xmm12

	mov rax, 0
	mov rdx, 1
	mov rcx, 2
	mov r8, 3
	mov r9, 4
	movd vecIndices0, rax
	movd xmm10, rdx
	movd xmm11, rcx
	movd xmm12, r8
	movd xmm8, r9
	punpckldq vecIndices0, xmm10
	punpckldq xmm11, xmm12
	pshufd xmm8, xmm8, 0x0
	punpcklqdq vecIndices0, xmm11
	movdqa vecIndices1, vecIndices0
	paddd vecIndices1, xmm8
	movdqa vecIndices2, vecIndices1
	paddd vecIndices2, xmm8
	movdqa vecIndices3, vecIndices2
	paddd vecIndices3, xmm8
	
	mov rax, 16
	movd vecIndicesInc, rax
	pshufd vecIndicesInc, vecIndicesInc, 0x0

	pxor vecSumB0, vecSumB0
	pxor vecSumB1, vecSumB1
	pxor vecSumB2, vecSumB2
	pxor vecSumB3, vecSumB3

	mov ptr32uHistogram, arg(0)
	mov sumA256, arg(1)
	mov sumB1, arg(2)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (size_t i = 0; i < 256; i += 16)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	xor i, i
	.LoopWidth:
		movdqa vec0, [ptr32uHistogram + (i)*COMPV_YASM_UINT32_SZ_BYTES]
		movdqa vec1, [ptr32uHistogram + (i+4)*COMPV_YASM_UINT32_SZ_BYTES]
		movdqa vec2, [ptr32uHistogram + (i+8)*COMPV_YASM_UINT32_SZ_BYTES]
		movdqa vec3, [ptr32uHistogram + (i+12)*COMPV_YASM_UINT32_SZ_BYTES]
		pmulld vec0, vecIndices0
		pmulld vec1, vecIndices1
		pmulld vec2, vecIndices2
		pmulld vec3, vecIndices3
		add i, 16
		paddd vecIndices0, vecIndicesInc
		paddd vecIndices1, vecIndicesInc
		paddd vecIndices2, vecIndicesInc
		paddd vecIndices3, vecIndicesInc
		cmp i, 256
		paddd vecSumB0, vec0
		paddd vecSumB1, vec1
		paddd vecSumB2, vec2
		paddd vecSumB3, vec3
		movdqa [sumA256 + (i+0-16)*COMPV_YASM_UINT32_SZ_BYTES], vec0
		movdqa [sumA256 + (i+4-16)*COMPV_YASM_UINT32_SZ_BYTES], vec1
		movdqa [sumA256 + (i+8-16)*COMPV_YASM_UINT32_SZ_BYTES], vec2
		movdqa [sumA256 + (i+12-16)*COMPV_YASM_UINT32_SZ_BYTES], vec3
		jl .LoopWidth
	.EndOf_LoopWidth

	paddd vecSumB0, vecSumB1
	paddd vecSumB2, vecSumB3
	paddd vecSumB0, vecSumB2
	pshufd vecSumB1, vecSumB0, 0x0e
	paddd vecSumB0, vecSumB1
	pshufd vecSumB1, vecSumB0, 0x01
	paddd vecSumB0, vecSumB1
	movd [sumB1], vecSumB0

	%undef ptr32uHistogram	
	%undef sumA256			
	%undef	sumB1			
	%undef i
	%undef vecIndicesInc
	%undef vecIndices0
	%undef vecIndices1
	%undef vecIndices2
	%undef vecIndices3
	%undef vec0
	%undef vec1
	%undef vec2
	%undef vec3
	%undef vecSumB0
	%undef vecSumB1
	%undef vecSumB2
	%undef vecSumB3

	;; begin epilog ;;
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

%endif


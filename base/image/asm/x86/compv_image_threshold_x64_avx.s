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

global sym(CompVImageThresholdGlobal_8u8u_Asm_X64_AVX2)

section .data

section .text

; arg(0) -> COMPV_ALIGNED(AVX) const uint8_t* inPtr,
; arg(1) -> COMPV_ALIGNED(AVX) uint8_t* outPtr,
; arg(2) -> compv_uscalar_t width, 
; arg(3) -> compv_uscalar_t height,
; arg(4) -> COMPV_ALIGNED(AVX) compv_uscalar_t stride,
; arg(5) -> compv_uscalar_t threshold
sym(CompVImageThresholdGlobal_8u8u_Asm_X64_AVX2)
	vzeroupper
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
	%define vec0			ymm0
	%define vec1			ymm1
	%define vec2			ymm2
	%define vec3			ymm3
	%define vecThreshold	ymm4
	%define vecThresholdn	xmm4
	%define vecMask			ymm5
	%define vecMaskn		xmm5

	; vecThreshold ;
	movsx eax, byte ptr arg(5)
	xor eax, 0x80
	vmovd vecThresholdn, eax  
	vpbroadcastb vecThreshold, vecThresholdn

	; vecMask ;
	mov eax, 0x80
	vmovd vecMaskn, eax  
	vpbroadcastb vecMask, vecMaskn

	mov inPtr, arg(0)
	mov outPtr, arg(1)
	mov width, arg(2)
	mov height, arg(3)
	mov stride, arg(4)
	mov width1, width
	and width1, -128

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width1; i += 128)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		cmp i, width1
		jge .EndOf_LoopWidth128
		.LoopWidth128
			vpxor vec0, vecMask, [inPtr + (i+0)*COMPV_YASM_UINT8_SZ_BYTES]
			vpxor vec1, vecMask, [inPtr + (i+32)*COMPV_YASM_UINT8_SZ_BYTES]
			vpxor vec2, vecMask, [inPtr + (i+64)*COMPV_YASM_UINT8_SZ_BYTES]
			vpxor vec3, vecMask, [inPtr + (i+96)*COMPV_YASM_UINT8_SZ_BYTES]
			lea i, [i + 128]
			vpcmpgtb vec0, vec0, vecThreshold
			vpcmpgtb vec1, vec1, vecThreshold
			vpcmpgtb vec2, vec2, vecThreshold
			vpcmpgtb vec3, vec3, vecThreshold
			cmp i, width1
			vmovdqa [outPtr + (i+0-128)*COMPV_YASM_UINT8_SZ_BYTES], vec0
			vmovdqa [outPtr + (i+32-128)*COMPV_YASM_UINT8_SZ_BYTES], vec1
			vmovdqa [outPtr + (i+64-128)*COMPV_YASM_UINT8_SZ_BYTES], vec2
			vmovdqa [outPtr + (i+96-128)*COMPV_YASM_UINT8_SZ_BYTES], vec3
			jl .LoopWidth128
		.EndOf_LoopWidth128


		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width; i += 32)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_LoopWidth32
		.LoopWidth32
			vpxor vec0, vecMask, [inPtr + (i+0)*COMPV_YASM_UINT8_SZ_BYTES]
			lea i, [i + 32]
			vpcmpgtb vec0, vecThreshold
			cmp i, width
			vmovdqa [outPtr + (i+0-32)*COMPV_YASM_UINT8_SZ_BYTES], vec0
			jl .LoopWidth32
		.EndOf_LoopWidth32
		
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
	%undef vecThresholdn
	%undef vecMask
	%undef vecMaskn

	;; begin epilog ;;
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret

%endif


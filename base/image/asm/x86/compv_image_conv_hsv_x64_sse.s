;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"
%if COMPV_YASM_ABI_IS_64BIT

%include "compv_image_conv_macros.s"

COMPV_YASM_DEFAULT_REL

global sym(CompVImageConvRgb24ToHsv_Asm_X64_SSSE3)

section .data
	extern sym(k85_i8)
	extern sym(k171_u8)
	extern sym(k43_f32)
	extern sym(k255_f32)
	extern sym(kShuffleEpi8_DeinterleaveRGB24_i32)
	extern sym(kShuffleEpi8_InterleaveRGB24_Step0_i32)
	extern sym(kShuffleEpi8_InterleaveRGB24_Step1_i32)
	extern sym(kShuffleEpi8_InterleaveRGB24_Step2_i32)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* rgb24Ptr
; arg(1) -> COMPV_ALIGNED(SSE) uint8_t* hsvPtr
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
; arg(5) -> const compv_float32_t(*scales43)[256] -> Useless (For C++ code only)
; arg(6) -> const compv_float32_t(*scales255)[256] -> Useless (For C++ code only)
sym(CompVImageConvRgb24ToHsv_Asm_X64_SSSE3):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_XMM 15
	;; end prolog ;;

	%define rgb24Ptr	rax
	%define hsvPtr		rdx
	%define width		r10
	%define height		r8
	%define stride		r9
	%define i			rcx

	%define vecZero		xmm0
	%define vec0		xmm1
	%define vec1		xmm2
	%define vec2		xmm3
	%define vec3		xmm4
	%define vec4		xmm5
	%define vec5		xmm6
	%define vec6		xmm7
	%define vec7		xmm8
	%define vec8		xmm9
	%define vec9		xmm10
	%define vec0f		xmm11
	%define vec1f		xmm12
	%define vec2f		xmm13
	%define vec3f		xmm14
	%define vec255f		xmm15

	pxor vecZero, vecZero
	movdqa vec255f, [sym(k255_f32)]


	mov rgb24Ptr, arg(0)
	mov hsvPtr, arg(1)
	mov width, arg(2)
	mov height, arg(3)
	mov stride, arg(4)

	lea width, [width+width*2] ; (width*3)
	lea stride, [stride+stride*2] ; (stride*3)


	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width; i += 48)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		.LoopWidth:
			; Load samples ;
			COMPV_VLD3_U8_SSSE3 rgb24Ptr + i, vec0, vec1, vec2, vec3, vec4, vec5

			movdqa vec4, vec0
			movdqa vec6, vec0
			pmaxub vec4, vec1
			pminub vec6, vec1
			pmaxub vec4, vec2 ; vec4 = maxVal = hsv[2].u8
			pminub vec6, vec2
			movdqa vec3, vec4
			movdqa vec5, vec4
			movdqa vec8, vec4
			psubusb vec3, vec6 ; vec3 = minus
			pcmpeqb vec5, vec0 ; m0 = (maxVal == r)
			movdqa vec6, vec5
			movdqa vec7, vec5
			pcmpeqb vec8, vec1
			pcmpeqb vec9, vec9 ; vec9 = vecFF
			pandn vec6, vec8 ; m1 = (maxVal == g) & ~m0
			movdqa vec8, vec2
			por vec7, vec6
			pandn vec7, vec9 ; m2 = ~(m0 | m1)
			movdqa vec9, vec0
			psubb vec9, vec1
			psubb vec1, vec2
			psubb vec8, vec0
			pand vec9, vec7
			pand vec8, vec6
			pand vec5, vec1
			movdqa vec1, vec3
			por vec5, vec8
			punpcklbw vec1, vecZero
			por vec5, vec9 ; vec5 = diff
			punpckhbw vec3, vecZero
			movdqa vec0, vec1
			movdqa vec2, vec3
			punpcklwd vec0, vecZero
			punpckhwd vec1, vecZero
			punpcklwd vec2, vecZero
			punpckhwd vec3, vecZero
			movdqa vec1f, vec4
			movdqa vec3f, vec4
			punpcklbw vec1f, vecZero
			punpckhbw vec3f, vecZero
			movdqa vec0f, vec1f
			movdqa vec2f, vec3f
			punpcklwd vec0f, vecZero
			punpckhwd vec1f, vecZero
			punpcklwd vec2f, vecZero
			punpckhwd vec3f, vecZero
			cvtdq2ps vec0f, vec0f
			cvtdq2ps vec1f, vec1f
			cvtdq2ps vec2f, vec2f
			cvtdq2ps vec3f, vec3f
			rcpps vec8, vec0f
			rcpps vec9, vec1f
			cvtdq2ps vec0, vec0
			cvtdq2ps vec1, vec1
			pcmpeqd vec0f, vecZero
			pcmpeqd vec1f, vecZero
			pandn vec0f, vec8
			pandn vec1f, vec9
			mulps vec0f, vec255f
			mulps vec1f, vec255f
			rcpps vec8, vec2f
			rcpps vec9, vec3f
			cvtdq2ps vec2, vec2
			cvtdq2ps vec3, vec3
			pcmpeqd vec2f, vecZero
			pcmpeqd vec3f, vecZero
			pandn vec2f, vec8
			mulps vec2f, vec255f
			pandn vec3f, vec9
			mulps vec3f, vec255f
			mulps vec0f, vec0
			mulps vec1f, vec1
			mulps vec2f, vec2
			mulps vec3f, vec3
			cvtps2dq vec0f, vec0f
			cvtps2dq vec1f, vec1f
			cvtps2dq vec2f, vec2f
			cvtps2dq vec3f, vec3f
			packssdw vec0f, vec1f
			packssdw vec2f, vec3f
			packuswb vec0f, vec2f
			movdqa vec8, vec0f ; vec8 = hsv[1].u8 - FIXME(dmi): replace next vec0f with vec8 and keep hsv[1].u8 in vec0f
			rcpps vec0f, vec0
			rcpps vec1f, vec1
			rcpps vec2f, vec2
			rcpps vec3f, vec3
			pcmpeqd vec0, vecZero
			pcmpeqd vec1, vecZero			
			pcmpeqd vec2, vecZero
			pcmpeqd vec3, vecZero
			pandn vec0, vec0f
			pandn vec1, vec1f
			pandn vec2, vec2f
			pandn vec3, vec3f
			movaps vec0f, [sym(k43_f32)]
			mulps vec0, vec0f
			mulps vec1, vec0f
			mulps vec2, vec0f
			mulps vec3, vec0f
			movdqa vec0f, vec5
			punpcklbw vec0f, vec0f
			punpckhbw vec5, vec5
			movdqa vec1f, vec0f
			movdqa vec3f, vec5
			punpcklwd vec0f, vec0f
			punpckhwd vec1f, vec1f
			punpcklwd vec5, vec5
			punpckhwd vec3f, vec3f
			psrad vec0f, 24
			psrad vec1f, 24
			psrad vec5, 24
			psrad vec3f, 24
			cvtdq2ps vec0f, vec0f
			cvtdq2ps vec1f, vec1f
			cvtdq2ps vec2f, vec5
			cvtdq2ps vec3f, vec3f
			mulps vec0f, vec0
			mulps vec1f, vec1
			mulps vec2f, vec2
			mulps vec3f, vec3
			cvtps2dq vec0f, vec0f
			cvtps2dq vec1f, vec1f
			cvtps2dq vec2f, vec2f
			cvtps2dq vec3f, vec3f
			pand vec6, [sym(k85_i8)] ; (85 & m1)
			pand vec7, [sym(k171_u8)] ; (171 & m2)
			por vec6, vec7 ; (85 & m1) | (171 & m2)
			packssdw vec0f, vec1f
			packssdw vec2f, vec3f
			packsswb vec0f, vec2f		
			paddsb vec0f, vec6
			
			COMPV_VST3_U8_SSSE3 hsvPtr + i, vec0f, vec8, vec4, vec0, vec1, vec2
			
			add i, 48
			cmp i, width
			
			;; end-of-LoopWidth ;;
			jl .LoopWidth

		dec height
		lea rgb24Ptr, [rgb24Ptr + stride]
		lea hsvPtr, [hsvPtr + stride]
		;; end-of-LoopHeight ;;
		jnz .LoopHeight


	%undef rgb24Ptr
	%undef hsvPtr
	%undef width
	%undef height
	%undef stride
	%undef i

	%undef vecZero		
	%undef vec0		
	%undef vec1		
	%undef vec2		
	%undef vec3		
	%undef vec4		
	%undef vec5		
	%undef vec6		
	%undef vec7		
	%undef vec8		
	%undef vec9		
	%undef vec0f		
	%undef vec1f		
	%undef vec2f		
	%undef vec3f		
	%undef vec255f		

	;; begin epilog ;;
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

%endif ; COMPV_YASM_ABI_IS_64BIT
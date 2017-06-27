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

global sym(CompVImageConvYuv420_to_Rgb24_Asm_X64_SSSE3)

section .data
	extern sym(k16_i16)
	extern sym(k37_i16)
	extern sym(k51_i16)
	extern sym(k65_i16)
	extern sym(k127_i16)
	extern sym(k13_26_i16)
	extern sym(kShuffleEpi8_InterleaveRGB24_Step0_i32)
	extern sym(kShuffleEpi8_InterleaveRGB24_Step1_i32)
	extern sym(kShuffleEpi8_InterleaveRGB24_Step2_i32)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* yPtr
; arg(1) -> COMPV_ALIGNED(SSE) const uint8_t* uPtr
; arg(2) -> COMPV_ALIGNED(SSE) const uint8_t* vPtr
; arg(3) -> COMPV_ALIGNED(SSE) uint8_t* rgbPtr
; arg(4) -> compv_uscalar_t width
; arg(5) -> compv_uscalar_t height
; arg(6) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
sym(CompVImageConvYuv420_to_Rgb24_Asm_X64_SSSE3):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_XMM 15
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	push r14
	;; end prolog ;;

	%define yPtr		rax
	%define uPtr		rbx
	%define vPtr		rdx	
	%define rgbPtr		rcx
	%define width		rsi
	%define height		rdi
	%define stride		r8
	%define strideRGB	r9
	%define strideUV	r10
	%define i			r11
	%define k			r12
	%define l			r13
	%define j			r14

	%define vecYlow		xmm0
	%define vecYhigh	xmm1
	%define vecU		xmm2
	%define vecV		xmm3
	%define vecZero		xmm4
	%define vec16		xmm5
	%define vec37		xmm6
	%define vec51		xmm7
	%define vec65		xmm8
	%define vec127		xmm9
	%define vec13_26	xmm10
	%define vec0		xmm11
	%define vec1		xmm12
	%define vec2		xmm13
	%define vecR		xmm14
	%define vecB		xmm15

	mov yPtr, arg(0)
	mov uPtr, arg(1)
	mov vPtr, arg(2)
	mov rgbPtr, arg(3)
	mov width, arg(4)
	mov height, arg(5)
	mov stride, arg(6)

	lea strideUV, [stride + 1]
	lea strideRGB, [stride + (stride * 2)]
	shr strideUV, 1

	pxor vecZero, vecZero
	movdqa vec16, [sym(k16_i16)]
	movdqa vec37, [sym(k37_i16)]
	movdqa vec51, [sym(k51_i16)]
	movdqa vec65, [sym(k65_i16)]
	movdqa vec127, [sym(k127_i16)]
	movdqa vec13_26, [sym(k13_26_i16)]

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	xor j, j
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0, k = 0, l = 0; i < width; i += 16, k += 48, l += 8)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		xor l, l
		.LoopWidth:
			; Load samples ;
			lea k, [i*3] ; ARM NEON add k, i, i SHL #1
			movdqa vecYlow, [yPtr + i*COMPV_YASM_UINT8_SZ_BYTES]
			movq vecU, [uPtr + l*COMPV_YASM_UINT8_SZ_BYTES]
			movq vecV, [vPtr + l*COMPV_YASM_UINT8_SZ_BYTES]
			add i, 16
			add l, 8
			cmp i, width

			; Convert to I16 ;
			movdqa vecYhigh, vecYlow
			punpcklbw vecYlow, vecZero
			punpckhbw vecYhigh, vecZero
			punpcklbw vecU, vecZero
			punpcklbw vecV, vecZero

			; Compute Yp, Up, Vp ;
			psubw vecYlow, vec16
			psubw vecYhigh, vec16
			psubw vecU, vec127
			psubw vecV, vec127

			; Compute (37Yp), (51Vp) and (65Up) ;
			pmullw vecYlow, vec37
			movdqa vec0, vecV
			pmullw vec0, vec51
			pmullw vecYhigh, vec37
			movdqa vec1, vecU
			pmullw vec1, vec65

			; Compute R = (37Yp + 0Up + 51Vp) >> 5 ;
			movdqa vecR, vec0
			punpcklwd vecR, vecR
			punpckhwd vec0, vec0
			paddw vecR, vecYlow
			paddw vec0, vecYhigh
			psraw vecR, 5
			psraw vec0, 5
			packuswb vecR, vec0

			; B = (37Yp + 65Up + 0Vp) >> 5 ;
			movdqa vecB, vec1
			punpcklwd vecB, vecB
			punpckhwd vec1, vec1
			paddw vecB, vecYlow
			paddw vec1, vecYhigh
			psraw vecB, 5
			psraw vec1, 5
			packuswb vecB, vec1

			; Compute G = (37Y' - 13U' - 26V') >> 5 = (37Y' - (13U' + 26V')) >> 5 ;
			movdqa vec0, vecU
			punpcklwd vec0, vecV
			punpckhwd vecU, vecV
			pmaddwd vec0, vec13_26
			pmaddwd vecU, vec13_26
			packssdw vec0, vecU
			movdqa vec1, vec0
			punpcklwd vec0, vec0
			punpckhwd vec1, vec1
			psubw vecYlow, vec0
			psubw vecYhigh, vec1
			psraw vecYlow, 5
			psraw vecYhigh, 5
			packuswb vecYlow, vecYhigh ; vecYlow contains vecG

			; Store result ;
			COMPV_VST3_I8_SSSE3 rgbPtr + k, vecR, vecYlow, vecB, vec0, vec1, vec2
			
			;; end-of-LoopWidth ;;
			jl .LoopWidth

		mov i, j
		inc j
		and i, 1
		add yPtr, stride
		neg i
		add rgbPtr, strideRGB
		and i, strideUV
		cmp j, height
		lea uPtr, [uPtr + i]
		lea vPtr, [vPtr + i]
				
		;; end-of-LoopHeight ;;
		jl .LoopHeight


	%undef yPtr
	%undef uPtr
	%undef vPtr
	%undef rgbPtr
	%undef width
	%undef height
	%undef stride
	%undef strideRGB
	%undef strideUV
	%undef i
	%undef k			
	%undef l			

	%undef vecYlow
	%undef vecYhigh
	%undef vecU
	%undef vecV
	%undef vecZero
	%undef vec37
	%undef vec51
	%undef vec65
	%undef vec127
	%undef vec13_26
	%undef vec0
	%undef vec1
	%undef vec2
	%undef vecR
	%undef vecB		

	;; begin epilog ;;
	pop r14
	pop r13
	pop r12
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret


%endif ; COMPV_YASM_ABI_IS_64BIT
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"
%if COMPV_YASM_ABI_IS_64BIT

%include "compv_image_conv_macros.s"
%include "compv_vldx_vstx_macros_x86.s"

COMPV_YASM_DEFAULT_REL

%define rgb24Family		0
%define rgba32Family	1

%define yuv420pFamily	2
%define yuv422pFamily	3
%define yuv444pFamily	4 ; there is intrin implementation but no asm (useless for now)
%define nv12Family		5
%define nv21Family		6
%define yuyv422Family	7
%define uyvy422Family	8

global sym(CompVImageConvYuv420p_to_Rgb24_Asm_X64_SSSE3)
global sym(CompVImageConvYuv420p_to_Rgba32_Asm_X64_SSE2)
global sym(CompVImageConvYuv422p_to_Rgb24_Asm_X64_SSSE3)
global sym(CompVImageConvYuv422p_to_Rgba32_Asm_X64_SSE2)
global sym(CompVImageConvNv12_to_Rgb24_Asm_X64_SSSE3)
global sym(CompVImageConvNv12_to_Rgba32_Asm_X64_SSSE3)
global sym(CompVImageConvNv21_to_Rgb24_Asm_X64_SSSE3)
global sym(CompVImageConvNv21_to_Rgba32_Asm_X64_SSSE3)
global sym(CompVImageConvYuyv422_to_Rgb24_Asm_X64_SSSE3)
global sym(CompVImageConvYuyv422_to_Rgba32_Asm_X64_SSSE3)
global sym(CompVImageConvUyvy422_to_Rgb24_Asm_X64_SSSE3)
global sym(CompVImageConvUyvy422_to_Rgba32_Asm_X64_SSSE3)

section .data
	extern sym(k16_16s)
	extern sym(k37_16s)
	extern sym(k51_16s)
	extern sym(k65_16s)
	extern sym(k127_16s)
	extern sym(k13_26_16s)
	extern sym(kShuffleEpi8_Interleave8uL3_Step0_s32)
	extern sym(kShuffleEpi8_Interleave8uL3_Step1_s32)
	extern sym(kShuffleEpi8_Interleave8uL3_Step2_s32)
	extern sym(kShuffleEpi8_Deinterleave8uL2_32s)
	extern sym(kShuffleEpi8_Yuyv422ToYuv_i32)
	extern sym(kShuffleEpi8_Uyvy422ToYuv_i32)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* yPtr
; arg(1) -> COMPV_ALIGNED(SSE) const uint8_t* uPtr
; arg(2) -> COMPV_ALIGNED(SSE) const uint8_t* vPtr
; arg(3) -> COMPV_ALIGNED(SSE) uint8_t* rgbxPtr
; arg(4) -> compv_uscalar_t width
; arg(5) -> compv_uscalar_t height
; arg(6) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
; %1 -> rgbxFamily
; %2 -> yuvFamily
%macro CompVImageConvYuv_to_Rgbx_Macro_X64 2
	%define rgbxFamily	%1
	%define yuvFamily	%2
	push rbp
	mov rbp, rsp
	%if yuvFamily == yuv420pFamily || yuvFamily == yuv422pFamily
		COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	%elif yuvFamily == nv12Family || yuvFamily == nv21Family
		COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	%elif yuvFamily == yuyv422Family || yuvFamily == uyvy422Family
		COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	%else
		%error 'Not implemented'
	%endif
	COMPV_YASM_SAVE_XMM 15
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	push r14
	;; end prolog ;;

	%if rgbxFamily == rgb24Family
		%define rgbxn		3
	%elif rgbxFamily == rgba32Family
		%define rgbxn		4
	%else
		%error 'Not implemented'
	%endif

	%if yuvFamily == yuv420pFamily
		%define y_step			16
		%define uv_step			8
		%define uv_inc_check	1
	%elif yuvFamily == yuv422pFamily
		%define y_step			16
		%define uv_step			8
		%define uv_inc_check	0
	%elif yuvFamily == nv12Family || yuvFamily == nv21Family
		%define y_step			16
		%define uv_step			16
		%define uv_inc_check	1
	%elif yuvFamily == yuyv422Family || yuvFamily == uyvy422Family
		%define y_step			32
		%define uv_step			16
		%define uv_inc_check	0
	%else
		%error 'Not implemented'
	%endif

	%define yPtr		rax
	%if yuvFamily == yuv420pFamily || yuvFamily == yuv422pFamily
		%define uPtr		rbx
		%define vPtr		rdx
	%elif yuvFamily == nv12Family || yuvFamily == nv21Family
		%define uvPtr		rbx
	%elif yuvFamily == yuyv422Family || yuvFamily == uyvy422Family
		; yuvPtr = yPtr
	%else
		%error 'Not implemented'
	%endif
	%define rgbxPtr		rcx
	%define width		rsi
	%define height		rdi
	%define stride		r8
	%define strideRGBx	r9
	%define strideUV	r10
	%define i			r11
	%define k			r12
	%define l			r13
	%define j			r14

	%define vecYlo		xmm0
	%define vecYhi		xmm1
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
	%if yuvFamily == yuv420pFamily || yuvFamily == yuv422pFamily
		mov uPtr, arg(1)
		mov vPtr, arg(2)
		mov rgbxPtr, arg(3)
		mov width, arg(4)
		mov height, arg(5)
		mov stride, arg(6)
	%elif yuvFamily == nv12Family || yuvFamily == nv21Family
		mov uvPtr, arg(1)
		mov rgbxPtr, arg(2)
		mov width, arg(3)
		mov height, arg(4)
		mov stride, arg(5)
	%elif yuvFamily == yuyv422Family || yuvFamily == uyvy422Family
		mov rgbxPtr, arg(1)
		mov width, arg(2)
		mov height, arg(3)
		mov stride, arg(4)
	%else
		%error 'Not implemented'
	%endif
	
	prefetcht0 [yPtr + (COMPV_YASM_CACHE_LINE_SIZE*0)]
	prefetcht0 [yPtr + (COMPV_YASM_CACHE_LINE_SIZE*1)]
	prefetcht0 [yPtr + (COMPV_YASM_CACHE_LINE_SIZE*2)]
	%if yuvFamily == yuv420pFamily || yuvFamily == yuv422pFamily
		prefetcht0 [uPtr + (COMPV_YASM_CACHE_LINE_SIZE*0)]
		prefetcht0 [uPtr + (COMPV_YASM_CACHE_LINE_SIZE*1)]
		prefetcht0 [uPtr + (COMPV_YASM_CACHE_LINE_SIZE*2)]
		prefetcht0 [vPtr + (COMPV_YASM_CACHE_LINE_SIZE*0)]
		prefetcht0 [vPtr + (COMPV_YASM_CACHE_LINE_SIZE*1)]
		prefetcht0 [vPtr + (COMPV_YASM_CACHE_LINE_SIZE*2)]
	%elif yuvFamily == nv12Family || yuvFamily == nv21Family
		prefetcht0 [uvPtr + (COMPV_YASM_CACHE_LINE_SIZE*0)]
		prefetcht0 [uvPtr + (COMPV_YASM_CACHE_LINE_SIZE*1)]
		prefetcht0 [uvPtr + (COMPV_YASM_CACHE_LINE_SIZE*2)]
	%elif yuvFamily == yuyv422Family || yuvFamily == uyvy422Family
		; yuvPtr = yPtr
	%else
		%error 'Not implemented'
	%endif

	lea strideRGBx, [stride * rgbxn]
	mov strideUV, stride
	%if yuvFamily == yuv420pFamily || yuvFamily == yuv422pFamily
		shr strideUV, 1
	%elif yuvFamily == nv12Family || yuvFamily == nv21Family
		; strideUV =  stride
	%elif yuvFamily == yuyv422Family || yuvFamily == uyvy422Family
		; strideUV not used
		shl stride, 1
		shl width, 1
	%else
		%error 'Not implemented'
	%endif

	pxor vecZero, vecZero
	movdqa vec16, [sym(k16_16s)]
	movdqa vec37, [sym(k37_16s)]
	movdqa vec51, [sym(k51_16s)]
	movdqa vec65, [sym(k65_16s)]
	movdqa vec127, [sym(k127_16s)]
	movdqa vec13_26, [sym(k13_26_16s)]

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	xor j, j
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0, k = 0, l = 0; i < width; i += y_step, k += rgbx_step, l += uv_step)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		xor l, l
		xor k, k
		.LoopWidth:
			; Load samples ;
			prefetcht0 [yPtr + (COMPV_YASM_CACHE_LINE_SIZE*3)]
			%if yuvFamily == yuv420pFamily || yuvFamily == yuv422pFamily
				prefetcht0 [uPtr + (COMPV_YASM_CACHE_LINE_SIZE*3)]
				prefetcht0 [vPtr + (COMPV_YASM_CACHE_LINE_SIZE*3)]
			%elif yuvFamily == nv12Family || yuvFamily == nv21Family
				prefetcht0 [uvPtr + (COMPV_YASM_CACHE_LINE_SIZE*3)]
			%elif yuvFamily == yuyv422Family || yuvFamily == uyvy422Family
				; yuvPtr = yPtr
			%else
				%error 'Not implemented'
			%endif
			movdqa vecYlo, [yPtr + i*COMPV_YASM_UINT8_SZ_BYTES]
			%if yuvFamily == yuv420pFamily || yuvFamily == yuv422pFamily
				movq vecU, [uPtr + l*COMPV_YASM_UINT8_SZ_BYTES]
				movq vecV, [vPtr + l*COMPV_YASM_UINT8_SZ_BYTES]
			%elif yuvFamily == nv12Family
				movdqa vecU, [uvPtr + l*COMPV_YASM_UINT8_SZ_BYTES]
				pshufb vecU, [sym(kShuffleEpi8_Deinterleave8uL2_32s)]
			%elif yuvFamily == nv21Family
				movdqa vecV, [uvPtr + l*COMPV_YASM_UINT8_SZ_BYTES]
				pshufb vecV, [sym(kShuffleEpi8_Deinterleave8uL2_32s)]
			%elif yuvFamily == yuyv422Family || yuvFamily == uyvy422Family
				movdqa vecV, [yPtr + (i*COMPV_YASM_UINT8_SZ_BYTES) + 0]
				movdqa vec1, [yPtr + (i*COMPV_YASM_UINT8_SZ_BYTES) + COMPV_YASM_XMM_SZ_BYTES]
				%if yuvFamily == yuyv422Family
					pshufb vecV, [sym(kShuffleEpi8_Yuyv422ToYuv_i32)]
					pshufb vec1, [sym(kShuffleEpi8_Yuyv422ToYuv_i32)]
				%else
					pshufb vecV, [sym(kShuffleEpi8_Uyvy422ToYuv_i32)]
					pshufb vec1, [sym(kShuffleEpi8_Uyvy422ToYuv_i32)]
				%endif
				movdqa vecYlo, vecV
				movdqa vecU, vecV
				punpcklqdq vecYlo, vec1
				punpckhdq vecU, vec1
				psrlq vecV, 32
				psrlq vec1, 32
				punpckhdq vecV, vec1
			%else
				%error 'Not implemented'
			%endif
			
			add i, y_step
			add l, uv_step
			cmp i, width

			%if yuvFamily == nv12Family
				movdqa vecV, vecU
				punpckhqdq vecV, vecV
			%elif yuvFamily == nv21Family
				movdqa vecU, vecV
				punpckhqdq vecU, vecU
			%endif

			movdqa vecYhi, vecYlo
			punpcklbw vecYlo, vecZero
			punpckhbw vecYhi, vecZero
			punpcklbw vecU, vecZero
			punpcklbw vecV, vecZero
			psubw vecYlo, vec16
			psubw vecYhi, vec16
			psubw vecU, vec127
			psubw vecV, vec127
			pmullw vecYlo, vec37
			movdqa vec0, vecV
			pmullw vec0, vec51
			movdqa vec1, vecU
			pmullw vecYhi, vec37
			movdqa vecR, vec0
			pmullw vec1, vec65
			punpcklwd vecR, vecR
			punpckhwd vec0, vec0
			paddw vecR, vecYlo
			paddw vec0, vecYhi
			psraw vecR, 5
			psraw vec0, 5
			movdqa vecB, vec1
			packuswb vecR, vec0
			movdqa vec0, vecU
			punpcklwd vec0, vecV
			punpckhwd vecU, vecV
			pmaddwd vec0, vec13_26
			pmaddwd vecU, vec13_26
			punpcklwd vecB, vecB
			punpckhwd vec1, vec1
			paddw vecB, vecYlo
			paddw vec1, vecYhi
			psraw vecB, 5
			psraw vec1, 5
			packuswb vecB, vec1
			packssdw vec0, vecU
			movdqa vec1, vec0
			punpcklwd vec0, vec0
			punpckhwd vec1, vec1
			psubw vecYlo, vec0
			psubw vecYhi, vec1
			psraw vecYlo, 5
			psraw vecYhi, 5
			packuswb vecYlo, vecYhi ; vecYlo contains vecG

			; Store result ;
			%if rgbxFamily == rgb24Family
				COMPV_VST3_U8_SSSE3 rgbxPtr + k, vecR, vecYlo, vecB, vec0, vec1, vec2
			%elif rgbxFamily == rgba32Family
				pcmpeqb vec0, vec0 ; vecA
				COMPV_VST4_U8_SSE2 rgbxPtr + k, vecR, vecYlo, vecB, vec0, vec1, vec2
			%else
				%error 'Not implemented'
			%endif
			lea k, [k + (rgbxn*COMPV_YASM_XMM_SZ_BYTES)]
			
			;; end-of-LoopWidth ;;
			jl .LoopWidth

		%if uv_inc_check
			mov i, j
		%endif
		inc j
		%if uv_inc_check
			and i, 1
		%endif
		add yPtr, stride
		%if uv_inc_check
			neg i
		%endif
		add rgbxPtr, strideRGBx
		%if uv_inc_check
			and i, strideUV
		%endif
		cmp j, height
		%if uv_inc_check
			%if yuvFamily == nv12Family || yuvFamily == nv21Family
				lea uvPtr, [uvPtr + i]
			%else
				lea uPtr, [uPtr + i]
				lea vPtr, [vPtr + i]
			%endif
		%elif yuvFamily != yuyv422Family && yuvFamily != uyvy422Family
			lea uPtr, [uPtr + strideUV]
			lea vPtr, [vPtr + strideUV]
		%endif
				
		;; end-of-LoopHeight ;;
		jl .LoopHeight


	%undef rgbxn
	%undef uv_step
	%undef y_step
	%undef uv_inc_check

	%undef yPtr
	%undef uPtr
	%undef vPtr
	%undef uvPtr
	%undef rgbxPtr
	%undef width
	%undef height
	%undef stride
	%undef strideRGBx
	%undef strideUV
	%undef i
	%undef k			
	%undef l			

	%undef vecYlo
	%undef vecYhi
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
	%undef rgbxFamily
	%undef yuvFamily
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvYuv420p_to_Rgb24_Asm_X64_SSSE3):
	CompVImageConvYuv_to_Rgbx_Macro_X64 rgb24Family, yuv420pFamily

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvYuv420p_to_Rgba32_Asm_X64_SSE2):
	CompVImageConvYuv_to_Rgbx_Macro_X64 rgba32Family, yuv420pFamily

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvYuv422p_to_Rgb24_Asm_X64_SSSE3):
	CompVImageConvYuv_to_Rgbx_Macro_X64 rgb24Family, yuv422pFamily

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvYuv422p_to_Rgba32_Asm_X64_SSE2):
	CompVImageConvYuv_to_Rgbx_Macro_X64 rgba32Family, yuv422pFamily

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvNv12_to_Rgb24_Asm_X64_SSSE3):
	CompVImageConvYuv_to_Rgbx_Macro_X64 rgb24Family, nv12Family

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvNv12_to_Rgba32_Asm_X64_SSSE3):
	CompVImageConvYuv_to_Rgbx_Macro_X64 rgba32Family, nv12Family

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvNv21_to_Rgb24_Asm_X64_SSSE3):
	CompVImageConvYuv_to_Rgbx_Macro_X64 rgb24Family, nv21Family

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvNv21_to_Rgba32_Asm_X64_SSSE3):
	CompVImageConvYuv_to_Rgbx_Macro_X64 rgba32Family, nv21Family

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvYuyv422_to_Rgb24_Asm_X64_SSSE3):
	CompVImageConvYuv_to_Rgbx_Macro_X64 rgb24Family, yuyv422Family

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvYuyv422_to_Rgba32_Asm_X64_SSSE3):
	CompVImageConvYuv_to_Rgbx_Macro_X64 rgba32Family, yuyv422Family

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvUyvy422_to_Rgb24_Asm_X64_SSSE3):
	CompVImageConvYuv_to_Rgbx_Macro_X64 rgb24Family, uyvy422Family

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvUyvy422_to_Rgba32_Asm_X64_SSSE3):
	CompVImageConvYuv_to_Rgbx_Macro_X64 rgba32Family, uyvy422Family


%endif ; COMPV_YASM_ABI_IS_64BIT

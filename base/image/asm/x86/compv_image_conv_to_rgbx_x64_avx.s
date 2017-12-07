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

global sym(CompVImageConvYuv420p_to_Rgb24_Asm_X64_AVX2)
global sym(CompVImageConvYuv420p_to_Rgba32_Asm_X64_AVX2)
global sym(CompVImageConvYuv422p_to_Rgb24_Asm_X64_AVX2)
global sym(CompVImageConvYuv422p_to_Rgba32_Asm_X64_AVX2)
global sym(CompVImageConvNv12_to_Rgb24_Asm_X64_AVX2)
global sym(CompVImageConvNv12_to_Rgba32_Asm_X64_AVX2)
global sym(CompVImageConvNv21_to_Rgb24_Asm_X64_AVX2)
global sym(CompVImageConvNv21_to_Rgba32_Asm_X64_AVX2)
global sym(CompVImageConvYuyv422_to_Rgb24_Asm_X64_AVX2)
global sym(CompVImageConvYuyv422_to_Rgba32_Asm_X64_AVX2)
global sym(CompVImageConvUyvy422_to_Rgb24_Asm_X64_AVX2)
global sym(CompVImageConvUyvy422_to_Rgba32_Asm_X64_AVX2)

section .data
	extern sym(k16_i16)
	extern sym(k37_i16)
	extern sym(k51_i16)
	extern sym(k65_i16)
	extern sym(k127_i16)
	extern sym(k13_26_i16)
	extern sym(kShuffleEpi8_Interleave8uL3_Step0_i32)
	extern sym(kShuffleEpi8_Interleave8uL3_Step1_i32)
	extern sym(kShuffleEpi8_Interleave8uL3_Step2_i32)
	extern sym(kShuffleEpi8_Deinterleave8uL2_i32)
	extern sym(kShuffleEpi8_Yuyv422ToYuv_i32)
	extern sym(kShuffleEpi8_Uyvy422ToYuv_i32)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(AVX) const uint8_t* yPtr
; arg(1) -> COMPV_ALIGNED(AVX) const uint8_t* uPtr
; arg(2) -> COMPV_ALIGNED(AVX) const uint8_t* vPtr
; arg(3) -> COMPV_ALIGNED(AVX) uint8_t* rgbxPtr
; arg(4) -> compv_uscalar_t width
; arg(5) -> compv_uscalar_t height
; arg(6) -> COMPV_ALIGNED(AVX) compv_uscalar_t stride
; %1 -> rgbxFamily
; %2 -> yuvFamily
%macro CompVImageConvYuv_to_Rgbx_Macro_X64 2
	%define rgbxFamily	%1
	%define yuvFamily	%2
	vzeroupper
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
	COMPV_YASM_SAVE_YMM 15
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
		%define y_step			32
		%define uv_step			16
		%define uv_inc_check	1
	%elif yuvFamily == yuv422pFamily
		%define y_step			32
		%define uv_step			16
		%define uv_inc_check	0
	%elif yuvFamily == nv12Family || yuvFamily == nv21Family
		%define y_step			32
		%define uv_step			32
		%define uv_inc_check	1
	%elif yuvFamily == yuyv422Family || yuvFamily == uyvy422Family
		%define y_step			64
		%define uv_step			32
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

	%define vecYlo		ymm0
	%define vecYlon		xmm0
	%define vecYhi		ymm1
	%define vecYhin		xmm1
	%define vecU		ymm2
	%define vecUn		xmm2
	%define vecV		ymm3
	%define vecVn		xmm3
	%define vecZero		ymm4
	%define vec16		ymm5
	%define vec37		ymm6
	%define vec51		ymm7
	%define vec65		ymm8
	%define vec127		ymm9
	%define vec13_26	ymm10
	%define vec0		ymm11
	%define vec0n		xmm11
	%define vec1		ymm12
	%define vec1n		xmm12
	%define vec2		ymm13
	%define vec2n		xmm13
	%define vecR		ymm14
	%define vecRn		xmm14
	%define vecB		ymm15
	%define vecBn		xmm15

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

	vpxor vecZero, vecZero
	vmovdqa vec16, [sym(k16_i16)]
	vmovdqa vec37, [sym(k37_i16)]
	vmovdqa vec51, [sym(k51_i16)]
	vmovdqa vec65, [sym(k65_i16)]
	vmovdqa vec127, [sym(k127_i16)]
	vmovdqa vec13_26, [sym(k13_26_i16)]

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
			vmovdqa vecYlo, [yPtr + i*COMPV_YASM_UINT8_SZ_BYTES]
			%if yuvFamily == yuv420pFamily || yuvFamily == yuv422pFamily
				vmovdqa vecUn, [uPtr + l*COMPV_YASM_UINT8_SZ_BYTES]
				vmovdqa vecVn, [vPtr + l*COMPV_YASM_UINT8_SZ_BYTES]
			%elif yuvFamily == nv12Family
				vmovdqa vecU, [uvPtr + l*COMPV_YASM_UINT8_SZ_BYTES]
				vpshufb vecU, [sym(kShuffleEpi8_Deinterleave8uL2_i32)]
			%elif yuvFamily == nv21Family
				vmovdqa vecV, [uvPtr + l*COMPV_YASM_UINT8_SZ_BYTES]
				vpshufb vecV, [sym(kShuffleEpi8_Deinterleave8uL2_i32)]
			%elif yuvFamily == yuyv422Family || yuvFamily == uyvy422Family
				vmovdqa vecV, [yPtr + (i*COMPV_YASM_UINT8_SZ_BYTES) + 0]
				vmovdqa vec1, [yPtr + (i*COMPV_YASM_UINT8_SZ_BYTES) + COMPV_YASM_YMM_SZ_BYTES]
				%if yuvFamily == yuyv422Family
					vpshufb vecV, [sym(kShuffleEpi8_Yuyv422ToYuv_i32)]
					vpshufb vec1, [sym(kShuffleEpi8_Yuyv422ToYuv_i32)]
				%else
					vpshufb vecV, [sym(kShuffleEpi8_Uyvy422ToYuv_i32)]
					vpshufb vec1, [sym(kShuffleEpi8_Uyvy422ToYuv_i32)]
				%endif
				vpunpcklqdq vecYlo, vecV, vec1
				vpunpckhdq vecU, vecV, vec1
				vpsrlq vecV, vecV, 32
				vpsrlq vec1, vec1, 32
				vpunpckhdq vecV, vecV, vec1
			%else
				%error 'Not implemented'
			%endif
			
			add i, y_step
			add l, uv_step
			cmp i, width

			%if yuvFamily == nv12Family
				vpunpckhqdq vecV, vecU, vecU
			%elif yuvFamily == nv21Family
				vpunpckhqdq vecU, vecV, vecV
			%endif

			vpunpckhbw vecYhi, vecYlo, vecZero
			vpunpcklbw vecYlo, vecYlo, vecZero
			%if yuvFamily == nv12Family || yuvFamily == nv21Family || yuvFamily == yuyv422Family || yuvFamily == uyvy422Family
				vpunpcklbw vecU, vecU, vecZero
				vpunpcklbw vecV, vecV, vecZero
			%else
				vpmovzxbw vecU, vecUn
				vpmovzxbw vecV, vecVn
			%endif
			vpsubw vecYlo, vecYlo, vec16
			vpsubw vecYhi, vecYhi, vec16
			vpsubw vecU, vecU, vec127
			vpsubw vecV, vecV, vec127
			vpmullw vecYlo, vecYlo, vec37
			vpmullw vec0, vecV, vec51
			vpmullw vecYhi, vecYhi, vec37
			vpmullw vec1, vecU, vec65
			vpunpcklwd vecR, vec0, vec0
			vpunpckhwd vec0, vec0, vec0
			vpaddw vecR, vecR, vecYlo
			vpaddw vec0, vec0, vecYhi
			vpsraw vecR, vecR, 5
			vpsraw vec0, vec0, 5
			vpackuswb vecR, vecR, vec0
			vpunpcklwd vec0, vecU, vecV
			vpunpckhwd vecU, vecU, vecV
			vpmaddwd vec0, vec0, vec13_26
			vpmaddwd vecU, vecU, vec13_26
			vpunpcklwd vecB, vec1, vec1
			vpunpckhwd vec1, vec1, vec1
			vpaddw vecB, vecB, vecYlo
			vpaddw vec1, vec1, vecYhi
			vpsraw vecB, vecB, 5
			vpsraw vec1, vec1, 5
			vpackuswb vecB, vecB, vec1
			vpackssdw vec0, vec0, vecU
			vpunpckhwd vec1, vec0, vec0
			vpunpcklwd vec0, vec0, vec0
			vpsubw vecYlo, vecYlo, vec0
			vpsubw vecYhi, vecYhi, vec1
			vpsraw vecYlo, vecYlo, 5
			vpsraw vecYhi, vecYhi, 5
			vpackuswb vecYlo, vecYlo, vecYhi ; vecYlo contains vecG

			; Store result ;
			%if yuvFamily == yuyv422Family || yuvFamily == uyvy422Family
				vpermq vecR, vecR, 0xD8
				vpermq vecYlo, vecYlo, 0xD8
				vpermq vecB, vecB, 0xD8
			%endif
			%if rgbxFamily == rgb24Family
				vextractf128 vecYhin, vecR, 0x1
				vextractf128 vecUn, vecYlo, 0x1
				vextractf128 vecVn, vecB, 0x1
				COMPV_VST3_U8_SSSE3_VEX rgbxPtr + k, vecRn, vecYlon, vecBn, vec0n, vec1n, vec2n
				COMPV_VST3_U8_SSSE3_VEX rgbxPtr + k + (3*COMPV_YASM_XMM_SZ_BYTES), vecYhin, vecUn, vecVn, vec0n, vec1n, vec2n
			%elif rgbxFamily == rgba32Family
				vpcmpeqb vec0n, vec0n ; vecA
				vextractf128 vecYhin, vecR, 0x1
				vextractf128 vecUn, vecYlo, 0x1
				vextractf128 vecVn, vecB, 0x1
				COMPV_VST4_U8_SSE2_VEX rgbxPtr + k, vecRn, vecYlon, vecBn, vec0n, vec1n, vec2n
				vpcmpeqb vec0n, vec0n ; vecA (will be modified by previous vst4)
				COMPV_VST4_U8_SSE2_VEX rgbxPtr + k + (4*COMPV_YASM_XMM_SZ_BYTES), vecYhin, vecUn, vecVn, vec0n, vec1n, vec2n
			%else
				%error 'Not implemented'
			%endif
			lea k, [k + (rgbxn*COMPV_YASM_YMM_SZ_BYTES)]
			
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
	%undef vecYlon
	%undef vecYhi
	%undef vecYhin
	%undef vecU
	%undef vecUn
	%undef vecV
	%undef vecVn
	%undef vecZero
	%undef vec37
	%undef vec51
	%undef vec65
	%undef vec127
	%undef vec13_26
	%undef vec0
	%undef vec0n
	%undef vec1
	%undef vec1n
	%undef vec2
	%undef vec2n
	%undef vecR
	%undef vecRn
	%undef vecB
	%undef vecBn	

	;; begin epilog ;;
	pop r14
	pop r13
	pop r12
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_YMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret
	%undef rgbxFamily
	%undef yuvFamily
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvYuv420p_to_Rgb24_Asm_X64_AVX2):
	CompVImageConvYuv_to_Rgbx_Macro_X64 rgb24Family, yuv420pFamily

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvYuv420p_to_Rgba32_Asm_X64_AVX2):
	CompVImageConvYuv_to_Rgbx_Macro_X64 rgba32Family, yuv420pFamily

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvYuv422p_to_Rgb24_Asm_X64_AVX2):
	CompVImageConvYuv_to_Rgbx_Macro_X64 rgb24Family, yuv422pFamily

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvYuv422p_to_Rgba32_Asm_X64_AVX2):
	CompVImageConvYuv_to_Rgbx_Macro_X64 rgba32Family, yuv422pFamily

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvNv12_to_Rgb24_Asm_X64_AVX2):
	CompVImageConvYuv_to_Rgbx_Macro_X64 rgb24Family, nv12Family

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvNv12_to_Rgba32_Asm_X64_AVX2):
	CompVImageConvYuv_to_Rgbx_Macro_X64 rgba32Family, nv12Family

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvNv21_to_Rgb24_Asm_X64_AVX2):
	CompVImageConvYuv_to_Rgbx_Macro_X64 rgb24Family, nv21Family

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvNv21_to_Rgba32_Asm_X64_AVX2):
	CompVImageConvYuv_to_Rgbx_Macro_X64 rgba32Family, nv21Family

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvYuyv422_to_Rgb24_Asm_X64_AVX2):
	CompVImageConvYuv_to_Rgbx_Macro_X64 rgb24Family, yuyv422Family

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvYuyv422_to_Rgba32_Asm_X64_AVX2):
	CompVImageConvYuv_to_Rgbx_Macro_X64 rgba32Family, yuyv422Family

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvUyvy422_to_Rgb24_Asm_X64_AVX2):
	CompVImageConvYuv_to_Rgbx_Macro_X64 rgb24Family, uyvy422Family

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvUyvy422_to_Rgba32_Asm_X64_AVX2):
	CompVImageConvYuv_to_Rgbx_Macro_X64 rgba32Family, uyvy422Family


%endif ; COMPV_YASM_ABI_IS_64BIT

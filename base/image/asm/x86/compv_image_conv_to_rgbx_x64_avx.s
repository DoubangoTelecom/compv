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
%include "compv_vldx_vstx_macros_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(CompVImageConvYuv420p_to_Rgb24_Asm_X64_AVX2)

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
; arg(0) -> COMPV_ALIGNED(AVX) const uint8_t* yPtr
; arg(1) -> COMPV_ALIGNED(AVX) const uint8_t* uPtr
; arg(2) -> COMPV_ALIGNED(AVX) const uint8_t* vPtr
; arg(3) -> COMPV_ALIGNED(AVX) uint8_t* rgbPtr
; arg(4) -> compv_uscalar_t width
; arg(5) -> compv_uscalar_t height
; arg(6) -> COMPV_ALIGNED(AVX) compv_uscalar_t stride
sym(CompVImageConvYuv420p_to_Rgb24_Asm_X64_AVX2):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_YMM 15
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

	%define vecYlow		ymm0
	%define vecYlown	xmm0
	%define vecYhigh	ymm1
	%define vecYhighn	xmm1
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
	mov uPtr, arg(1)
	mov vPtr, arg(2)
	mov rgbPtr, arg(3)
	mov width, arg(4)
	mov height, arg(5)
	mov stride, arg(6)

	prefetcht0 [yPtr + (COMPV_YASM_CACHE_LINE_SIZE*0)]
	prefetcht0 [yPtr + (COMPV_YASM_CACHE_LINE_SIZE*1)]
	prefetcht0 [yPtr + (COMPV_YASM_CACHE_LINE_SIZE*2)]
	prefetcht0 [uPtr + (COMPV_YASM_CACHE_LINE_SIZE*0)]
	prefetcht0 [uPtr + (COMPV_YASM_CACHE_LINE_SIZE*1)]
	prefetcht0 [uPtr + (COMPV_YASM_CACHE_LINE_SIZE*2)]
	prefetcht0 [vPtr + (COMPV_YASM_CACHE_LINE_SIZE*0)]
	prefetcht0 [vPtr + (COMPV_YASM_CACHE_LINE_SIZE*1)]
	prefetcht0 [vPtr + (COMPV_YASM_CACHE_LINE_SIZE*2)]

	lea strideUV, [stride + 1]
	lea strideRGB, [stride + (stride * 2)]
	shr strideUV, 1

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
		; for (i = 0, k = 0, l = 0; i < width; i += 32, k += 96, l += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		xor l, l
		.LoopWidth:
			; Load samples ;
			prefetcht0 [yPtr + (COMPV_YASM_CACHE_LINE_SIZE*3)]
			prefetcht0 [uPtr + (COMPV_YASM_CACHE_LINE_SIZE*3)]
			prefetcht0 [vPtr + (COMPV_YASM_CACHE_LINE_SIZE*3)]
			lea k, [i*3] ; ARM NEON add k, i, i SHL #1
			vmovdqa vecYlow, [yPtr + i*COMPV_YASM_UINT8_SZ_BYTES]
			vmovdqa vecUn, [uPtr + l*COMPV_YASM_UINT8_SZ_BYTES]
			vmovdqa vecVn, [vPtr + l*COMPV_YASM_UINT8_SZ_BYTES]
			add i, 32
			add l, 16
			cmp i, width

			; Convert to I16 ;
			vpunpckhbw vecYhigh, vecYlow, vecZero
			vpunpcklbw vecYlow, vecYlow, vecZero
			vpmovzxbw vecU, vecUn
			vpmovzxbw vecV, vecVn

			; Compute Yp, Up, Vp ;
			vpsubw vecYlow, vecYlow, vec16
			vpsubw vecYhigh, vecYhigh, vec16
			vpsubw vecU, vecU, vec127
			vpsubw vecV, vecV, vec127

			; Compute (37Yp), (51Vp) and (65Up) ;
			vpmullw vec0, vecV, vec51
			vpmullw vecYlow, vec37
			vpmullw vecYhigh, vecYhigh, vec37
			vpmullw vec1, vecU, vec65

			; Compute R = (37Yp + 0Up + 51Vp) >> 5 ;
			vpunpcklwd vecR, vec0, vec0
			vpunpckhwd vec0, vec0, vec0
			vpaddw vecR, vecR, vecYlow
			vpaddw vec0, vec0, vecYhigh
			vpsraw vecR, vecR, 5
			vpsraw vec0, vec0, 5
			vpackuswb vecR, vecR, vec0

			; B = (37Yp + 65Up + 0Vp) >> 5 ;
			vpunpcklwd vecB, vec1, vec1
			vpunpckhwd vec1, vec1, vec1
			vpaddw vecB, vecB, vecYlow
			vpaddw vec1, vec1, vecYhigh
			vpsraw vecB, vecB, 5
			vpsraw vec1, vec1, 5
			vpackuswb vecB, vecB, vec1

			; Compute G = (37Y' - 13U' - 26V') >> 5 = (37Y' - (13U' + 26V')) >> 5 ;
			vpunpcklwd vec0, vecU, vecV
			vpunpckhwd vecU, vecU, vecV
			vpmaddwd vec0, vec0, vec13_26
			vpmaddwd vecU, vecU, vec13_26
			vpackssdw vec0, vec0, vecU
			vpunpckhwd vec1, vec0, vec0
			vpunpcklwd vec0, vec0, vec0
			vpsubw vecYhigh, vecYhigh, vec1
			vpsubw vecYlow, vecYlow, vec0
			vpsraw vecYhigh, vecYhigh, 5
			vpsraw vecYlow, vecYlow, 5
			vpackuswb vecYlow, vecYhigh ; vecYlow contains vecG

			; Store result ;
			vextractf128 vecYhighn, vecR, 0x1
			vextractf128 vecUn, vecYlow, 0x1
			vextractf128 vecVn, vecB, 0x1
			COMPV_VST3_U8_SSSE3_VEX rgbPtr + k, vecRn, vecYlown, vecBn, vec0n, vec1n, vec2n
			COMPV_VST3_U8_SSSE3_VEX rgbPtr + k + 48, vecYhighn, vecUn, vecVn, vec0n, vec1n, vec2n
			
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
	%undef vecYlown
	%undef vecYhigh
	%undef vecYhighn
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


%endif ; COMPV_YASM_ABI_IS_64BIT
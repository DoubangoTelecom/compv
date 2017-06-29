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

global sym(CompVImageConvRgb24ToHsv_Asm_X64_AVX2)

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
; arg(0) -> COMPV_ALIGNED(AVX) const uint8_t* rgb24Ptr
; arg(1) -> COMPV_ALIGNED(AVX) uint8_t* hsvPtr
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> COMPV_ALIGNED(AVX) compv_uscalar_t stride
sym(CompVImageConvRgb24ToHsv_Asm_X64_AVX2):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	COMPV_YASM_SAVE_YMM 15
	;; end prolog ;;

	%define rgb24Ptr	rax
	%define hsvPtr		rdx
	%define width		r10
	%define height		r8
	%define stride		r9
	%define i			rcx

	%define vecZero		ymm0
	%define vec0		ymm1
	%define vec0n		xmm1
	%define vec1		ymm2
	%define vec1n		xmm2
	%define vec2		ymm3
	%define vec2n		xmm3
	%define vec3		ymm4
	%define vec3n		xmm4
	%define vec4		ymm5
	%define vec4n		xmm5
	%define vec5		ymm6
	%define vec5n		xmm6
	%define vec6		ymm7
	%define vec6n		xmm7
	%define vec7		ymm8
	%define vec7n		xmm8
	%define vec8		ymm9
	%define vec8n		xmm9
	%define vec9		ymm10
	%define vec9n		xmm10
	%define vec0f		ymm11
	%define vec0fn		xmm11
	%define vec1f		ymm12
	%define vec1fn		xmm12
	%define vec2f		ymm13
	%define vec2fn		xmm13
	%define vec3f		ymm14
	%define vec3fn		xmm14
	%define vec255f		ymm15

	vpxor vecZero, vecZero
	vmovdqa vec255f, [sym(k255_f32)]

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
		; for (i = 0; i < width; i += 96)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		.LoopWidth:
			; Load samples ;
			COMPV_VLD3_U8_SSSE3_VEX rgb24Ptr + i + 0 , vec0n, vec1n, vec2n, vec3n, vec4n, vec5n
			COMPV_VLD3_U8_SSSE3_VEX rgb24Ptr + i + 48, vec3n, vec4n, vec5n, vec6n, vec7n, vec8n
			vinsertf128 vec0, vec0, vec3n, 0x1
			vinsertf128 vec1, vec1, vec4n, 0x1
			vinsertf128 vec2, vec2, vec5n, 0x1

			vpmaxub vec4, vec0, vec1
			vpminub vec6, vec0, vec1
			vpmaxub vec4, vec4, vec2
			vpminub vec6, vec6, vec2
			vpsubusb vec3, vec4, vec6
			vpcmpeqb vec5, vec4, vec0
			vpcmpeqb vec8, vec4, vec1
			vpcmpeqb vec9, vec9, vec9
			vpandn vec6, vec5, vec8
			vpor vec7, vec5, vec6
			vpandn vec7, vec7, vec9
			vpsubb vec9, vec0, vec1
			vpsubb vec1, vec1, vec2
			vpsubb vec8, vec2, vec0
			vpand vec9, vec9, vec7
			vpand vec8, vec8, vec6
			vpand vec5, vec5, vec1
			vpor vec5, vec5, vec8
			vpunpcklbw vec1, vec3, vecZero
			vpor vec5, vec5, vec9
			vpunpckhbw vec3, vec3, vecZero
			vpunpcklwd vec0, vec1, vecZero
			vpunpckhwd vec1, vec1, vecZero
			vpunpcklwd vec2, vec3, vecZero
			vpunpckhwd vec3, vec3, vecZero
			vpunpcklbw vec1f, vec4, vecZero
			vpunpckhbw vec3f, vec4, vecZero
			vpunpcklwd vec0f, vec1f, vecZero
			vpunpckhwd vec1f, vec1f, vecZero
			vpunpcklwd vec2f, vec3f, vecZero
			vpunpckhwd vec3f, vec3f, vecZero
			vcvtdq2ps vec0f, vec0f
			vcvtdq2ps vec1f, vec1f
			vcvtdq2ps vec2f, vec2f
			vcvtdq2ps vec3f, vec3f
			vrcpps vec8, vec0f
			vrcpps vec9, vec1f
			vcvtdq2ps vec0, vec0
			vcvtdq2ps vec1, vec1
			vpcmpeqd vec0f, vec0f, vecZero
			vpcmpeqd vec1f, vec1f, vecZero
			vpandn vec0f, vec0f, vec8
			vpandn vec1f, vec1f, vec9
			vmulps vec0f, vec0f, vec255f
			vmulps vec1f, vec1f, vec255f
			vrcpps vec8, vec2f
			vrcpps vec9, vec3f
			vcvtdq2ps vec2, vec2
			vcvtdq2ps vec3, vec3
			vpcmpeqd vec2f, vec2f, vecZero
			vpcmpeqd vec3f, vec3f, vecZero
			vpandn vec2f, vec2f, vec8
			vmulps vec2f, vec2f, vec255f
			vpandn vec3f, vec3f, vec9
			vmulps vec3f, vec3f, vec255f
			vmulps vec0f, vec0f, vec0
			vmulps vec1f, vec1f, vec1
			vmulps vec2f, vec2f, vec2
			vmulps vec3f, vec3f, vec3
			vcvtps2dq vec0f, vec0f
			vcvtps2dq vec1f, vec1f
			vcvtps2dq vec2f, vec2f
			vcvtps2dq vec3f, vec3f
			vrcpps vec8, vec0
			vpackssdw vec0f, vec0f, vec1f
			vrcpps vec1f, vec1
			vpackssdw vec2f, vec2f, vec3f
			vrcpps vec3f, vec3
			vpackuswb vec0f, vec0f, vec2f			
			vrcpps vec2f, vec2
			vpcmpeqd vec0, vec0, vecZero
			vpcmpeqd vec1, vec1, vecZero			
			vpcmpeqd vec2, vec2, vecZero
			vpcmpeqd vec3, vec3, vecZero
			vpandn vec0, vec0, vec8
			vmovaps vec8, [sym(k43_f32)]
			vpandn vec1, vec1, vec1f
			vpandn vec2, vec2, vec2f
			vpandn vec3, vec3, vec3f
			vmulps vec0, vec0, vec8
			vmulps vec1, vec1, vec8
			vmulps vec2, vec2, vec8
			vmulps vec3, vec3, vec8
			vpunpcklbw vec8, vec5, vec5
			vpunpckhbw vec5, vec5, vec5
			vpunpckhwd vec1f, vec8, vec8
			vpunpcklwd vec8, vec8, vec8
			vpunpckhwd vec3f, vec5, vec5
			vpunpcklwd vec5, vec5, vec5
			vpsrad vec8, vec8, 24
			vpsrad vec1f, vec1f, 24
			vpsrad vec5, vec5, 24
			vpsrad vec3f, vec3f, 24
			vcvtdq2ps vec8, vec8
			vcvtdq2ps vec1f, vec1f
			vcvtdq2ps vec2f, vec5
			vcvtdq2ps vec3f, vec3f
			vmulps vec8, vec8, vec0
			vmulps vec1f, vec1f, vec1
			vmulps vec2f, vec2f, vec2
			vmulps vec3f, vec3f, vec3
			vcvtps2dq vec8, vec8
			vcvtps2dq vec1f, vec1f
			vcvtps2dq vec2f, vec2f
			vcvtps2dq vec3f, vec3f
			vpand vec6, vec6, [sym(k85_i8)]
			vpand vec7, vec7, [sym(k171_u8)]
			vpor vec6, vec6, vec7
			vpackssdw vec8, vec8, vec1f
			vpackssdw vec2f, vec2f, vec3f
			vpacksswb vec8, vec8, vec2f
			vpaddsb vec8, vec8, vec6
			
			vextractf128 vec5n, vec8, 0x1
			vextractf128 vec6n, vec0f, 0x1
			vextractf128 vec7n, vec4, 0x1
			COMPV_VST3_U8_SSSE3_VEX hsvPtr + i + 0, vec8n, vec0fn, vec4n, vec0n, vec1n, vec2n
			COMPV_VST3_U8_SSSE3_VEX hsvPtr + i + 48, vec5n, vec6n, vec7n, vec0n, vec1n, vec2n
			
			add i, 96
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
	%undef vec0n
	%undef vec1
	%undef vec1n	
	%undef vec2	
	%undef vec2n	
	%undef vec3
	%undef vec3n		
	%undef vec4
	%undef vec4n		
	%undef vec5
	%undef vec5n		
	%undef vec6
	%undef vec6n	
	%undef vec7
	%undef vec7n		
	%undef vec8
	%undef vec8n
	%undef vec9
	%undef vec9n	
	%undef vec0f
	%undef vec0fn		
	%undef vec1f
	%undef vec1fn		
	%undef vec2f
	%undef vec2fn		
	%undef vec3f
	%undef vec3fn		
	%undef vec255f		

	;; begin epilog ;;
	COMPV_YASM_RESTORE_YMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret

%endif ; COMPV_YASM_ABI_IS_64BIT
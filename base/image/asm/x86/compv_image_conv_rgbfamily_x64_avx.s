;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"
%if COMPV_YASM_ABI_IS_64BIT

%include "compv_image_conv_macros.s"

COMPV_YASM_DEFAULT_REL

%define rgb24Family		0
%define rgb32Family		1
%define bigEndian		2
%define littleEndian	3

global sym(CompVImageConvRgb24family_to_y_Asm_X64_AVX2)
global sym(CompVImageConvRgb32family_to_y_Asm_X64_AVX2)
global sym(CompVImageConvRgb565lefamily_to_y_Asm_X64_AVX2)
global sym(CompVImageConvRgb565befamily_to_y_Asm_X64_AVX2)
global sym(CompVImageConvRgb24family_to_uv_planar_11_Asm_X64_AVX2)
global sym(CompVImageConvRgb32family_to_uv_planar_11_Asm_X64_AVX2)
global sym(CompVImageConvRgb565lefamily_to_uv_Asm_X64_AVX2)
global sym(CompVImageConvRgb565befamily_to_uv_Asm_X64_AVX2)

section .data
	extern sym(k16_16s)
	extern sym(k128_16s)
	extern sym(kAVXPermutevar8x32_ABCDDEFG_32s)
	extern sym(kAVXPermutevar8x32_AEBFCGDH_32s)
	extern sym(kShuffleEpi8_RgbToRgba_i32)
	extern sym(kRGB565ToYUV_RMask_u16)
	extern sym(kRGB565ToYUV_GMask_u16)
	extern sym(kRGB565ToYUV_BMask_u16)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(AVX) const uint8_t* rgbPtr
; arg(1) -> COMPV_ALIGNED(AVX) uint8_t* outYPtr
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> COMPV_ALIGNED(AVX) compv_uscalar_t stride,
; arg(5) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_YCoeffs8
; %1 -> family: rgb24Family or rgb32Family
%macro CompVImageConvRgbfamily_to_y_Macro_X64_AVX2 1
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	COMPV_YASM_SAVE_YMM 10 ;YMM[6-n]
	; end prolog

	mov rax, arg(2)
	add rax, 31
	and rax, -32
	mov rcx, arg(4)
	sub rcx, rax ; rcx = padY
	mov rdx, rcx
	%if %1 == rgb32Family
		shl rdx, 2 ; rdx = padRGBA
	%elif %1 == rgb24Family
		imul rdx, 3 ; rdx = padRGB
	%else
		%error 'Not implemented'
	%endif

	mov rax, arg(0) ; rgbPtr
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*0]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*1]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*2]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]

	mov r8, arg(5)
	vmovdqa ymm0, [r8] ; ymmYCoeffs
	vmovdqa ymm1, [sym(k16_16s)] ; ymm16
	vmovdqa ymm6, [sym(kAVXPermutevar8x32_AEBFCGDH_32s)] ; ymmAEBFCGDH
	vmovdqa ymm7, [sym(kShuffleEpi8_RgbToRgba_i32)] ; ymmMaskRgbToRgba
	vmovdqa ymm8, [sym(kAVXPermutevar8x32_ABCDDEFG_32s)] ; ymmABCDDEFG
	
	mov r8, arg(3) ; height
	mov r10, arg(1) ; outYPtr
	mov r11, arg(2) ; width

	.LoopHeight:
		xor r9, r9
		.LoopWidth:
			prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]
			%if %1 == rgb32Family
				vmovdqa ymm2, [rax + 0]
				vmovdqa ymm3, [rax + 32]
				vmovdqa ymm4, [rax + 64]
				vmovdqa ymm5, [rax + 96]
				lea rax, [rax + 128] ; rgb32Ptr += 128
			%elif %1 == rgb24Family
				; Convert RGB24 -> RGBA32, alpha channel contains zeros	
				COMPV_32xRGB_TO_32xRGBA_AVX2 rax, ymm2, ymm3, ymm4, ymm5, ymm8, ymm7
				lea rax, [rax + 96] ; rgb24Ptr += 128
			%else
				%error 'Not implemented'
			%endif
			vpmaddubsw ymm2, ymm2, ymm0
			vpmaddubsw ymm3, ymm3, ymm0
			vpmaddubsw ymm4, ymm4, ymm0
			vpmaddubsw ymm5, ymm5, ymm0
			vphaddw ymm2, ymm2, ymm3
			vphaddw ymm4, ymm4, ymm5
			vpsraw ymm2, ymm2, 7 ; >> 7
			vpsraw ymm4, ymm4, 7 ; >> 7
			vpaddw ymm2, ymm2, ymm1 ; + 16
			vpaddw ymm4, ymm4, ymm1 ; + 16
			lea r9, [r9 + 32] ; i += 32
			cmp r9, r11 ; (i < width)?
			vpackuswb ymm2, ymm2, ymm4 ; Saturate(I16 -> U8): packus(ACBD, EGFH) -> AEBFCGDH
			vpermd ymm2, ymm6, ymm2
			vmovdqa [r10], ymm2
			lea r10, [r10 + 32]
			; end-of-LoopWidth
			jl .LoopWidth
	lea r10, [r10 + rcx]
	lea rax, [rax + rdx]
	; end-of-LoopHeight
	dec r8
	jnz .LoopHeight
	
	; begin epilog
	COMPV_YASM_RESTORE_YMM
    COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb24family_to_y_Asm_X64_AVX2)
	CompVImageConvRgbfamily_to_y_Macro_X64_AVX2 rgb24Family

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb32family_to_y_Asm_X64_AVX2)
	CompVImageConvRgbfamily_to_y_Macro_X64_AVX2 rgb32Family


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(AVX) const uint8_t* rgb565Ptr
; arg(1) -> COMPV_ALIGNED(AVX) uint8_t* outYPtr
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> COMPV_ALIGNED(AVX) compv_uscalar_t stride
; arg(5) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_YCoeffs8
; %1 -> endianness: bigEndian or littleEndian
%macro CompVImageConvRgb565family_to_y_Macro_X64_AVX2 1
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	COMPV_YASM_SAVE_YMM 15
	;; end prolog ;;

	mov rdx, arg(2)
	lea rdx, [rdx + 31]
	and rdx, -32
	mov rcx, arg(4)
	sub rcx, rdx ; rcx = padUV
	mov rdx, rcx
	shl rdx, 1 ; rdx = padRGB565

	mov rax, arg(0) ; rax = rgb565Ptr
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*0]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*1]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*2]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]

	mov r8, arg(5)
	movzx r10, byte [r8 + 0]
	vmovd xmm12, r10d
	vpbroadcastw ymm12, xmm12
	movzx r10, byte [r8 + 1]
	vmovd xmm13, r10d
	vpbroadcastw ymm13, xmm13
	movzx r10, byte [r8 + 2]
	vmovd xmm14, r10d
	vpbroadcastw ymm14, xmm14
	
	mov r8, arg(3) ; r8 = height
	mov r10, arg(1) ; r10 = outYPtr
	mov r11, arg(2) ; r11 = width

	vmovdqa ymm15, [sym(k16_16s)]
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		xor r9, r9
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width; i += 32)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopWidth:
			prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]
			vmovdqa ymm4, [rax + 0]
			vmovdqa ymm5, [rax + 32]
			lea rax, [rax + 64] ; rgb565Ptr += 64
			lea r9, [r9 + 32] ; i += 32
			cmp r9, r11 ; (i < width)?
			%if %1 == bigEndian
				vpsrlw ymm0, ymm4, 8
				vpsrlw ymm1, ymm5, 8
				vpsllw ymm4, ymm4, 8
				vpsllw ymm5, ymm5, 8
				vpor ymm4, ymm4, ymm0
				vpor ymm5, ymm5, ymm1
			%endif
			vpand ymm0, ymm4, [sym(kRGB565ToYUV_RMask_u16)]
			vpand ymm1, ymm5, [sym(kRGB565ToYUV_RMask_u16)]
			vpand ymm2, ymm4, [sym(kRGB565ToYUV_GMask_u16)]
			vpand ymm3, ymm5, [sym(kRGB565ToYUV_GMask_u16)]
			vpand ymm4, ymm4, [sym(kRGB565ToYUV_BMask_u16)]
			vpand ymm5, ymm5, [sym(kRGB565ToYUV_BMask_u16)]
			vpsrlw ymm0, ymm0, 8
			vpsrlw ymm1, ymm1, 8
			vpsrlw ymm2, ymm2, 3
			vpsrlw ymm3, ymm3, 3
			vpsllw ymm4, ymm4, 3
			vpsllw ymm5, ymm5, 3
			vpsrlw ymm6, ymm0, 5
			vpsrlw ymm7, ymm1, 5
			vpsrlw ymm8, ymm2, 6
			vpsrlw ymm9, ymm3, 6
			vpsrlw ymm10, ymm4, 5
			vpsrlw ymm11, ymm5, 5
			vpor ymm0, ymm0, ymm6
			vpor ymm1, ymm1, ymm7	
			vpor ymm2, ymm2, ymm8
			vpor ymm3, ymm3, ymm9			
			vpor ymm4, ymm4, ymm10
			vpor ymm5, ymm5, ymm11
			vpmullw ymm0, ymm0, ymm12
			vpmullw ymm1, ymm1, ymm12
			vpmullw ymm2, ymm2, ymm13
			vpmullw ymm3, ymm3, ymm13
			vpmullw ymm4, ymm4, ymm14
			vpmullw ymm5, ymm5, ymm14
			vpaddw ymm0, ymm0, ymm2
			vpaddw ymm1, ymm1, ymm3
			vpaddw ymm0, ymm0, ymm4
			vpaddw ymm1, ymm1, ymm5
			vpsrlw ymm0, ymm0, 7
			vpsrlw ymm1, ymm1, 7
			vpaddw ymm0, ymm0, ymm15		
			vpaddw ymm1, ymm1, ymm15
			vpackuswb ymm0, ymm0, ymm1
			vpermq ymm0, ymm0, 0xD8
			vmovdqa [r10], ymm0
			lea r10, [r10 + 32] ; outYPtr += 32
			; end-of-LoopWidth
			jl .LoopWidth

		lea r10, [r10 + rcx]
		lea rax, [rax + rdx]
		; end-of-LoopHeight
		dec r8
		jnz .LoopHeight

	;; begin epilog ;;
	COMPV_YASM_RESTORE_YMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb565lefamily_to_y_Asm_X64_AVX2):
	CompVImageConvRgb565family_to_y_Macro_X64_AVX2 littleEndian

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb565befamily_to_y_Asm_X64_AVX2):
	CompVImageConvRgb565family_to_y_Macro_X64_AVX2 bigEndian

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(AVX) const uint8_t* rgbPtr
; arg(1) -> COMPV_ALIGNED(AVX) uint8_t* outUPtr
; arg(2) -> COMPV_ALIGNED(AVX) uint8_t* outVPtr
; arg(3) -> compv_uscalar_t width
; arg(4) -> compv_uscalar_t height
; arg(5) -> COMPV_ALIGNED(AVX) compv_uscalar_t stride
; arg(6) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_UCoeffs8
; arg(7) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_VCoeffs8
; %1 -> family: rgb24Family or rgb32Family
%macro CompVImageConvRgbfamily_to_uv_planar_11_Macro_X64_AVX2 1
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_YMM 13
	; end prolog
	push r12

	mov rdx, arg(3)
	lea rdx, [rdx + 31]
	and rdx, -32
	mov rcx, arg(5)
	sub rcx, rdx
	mov r11, rcx ; r11 = padUV
	%if %1 == rgb32Family
		shl rcx, 2 ; rcx = padRGBA
	%elif %1 == rgb24Family
		imul rcx, 3 ; rcx = padRGB
	%else
		%error 'Not implemented'
	%endif

	mov rax, arg(0) ; rax = rgbPtr
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*0]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*1]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*2]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]

	mov r8, arg(6)
	vmovdqa ymm8, [r8] ; ymm8 = ymmUCoeffs
	mov r8, arg(7)
	vmovdqa ymm9, [r8] ; ymm9 = ymmVCoeffs

	vmovdqa ymm10, [sym(kAVXPermutevar8x32_AEBFCGDH_32s)] ; ymmAEBFCGDH
	vmovdqa ymm11, [sym(kShuffleEpi8_RgbToRgba_i32)] ; ymmMaskRgbToRgba
	vmovdqa ymm12, [sym(kAVXPermutevar8x32_ABCDDEFG_32s)] ; ymmABCDDEFG
	vmovdqa ymm13, [sym(k128_16s)] ; ymm128
	
	mov r8, arg(4) ; r8 = height
	mov r10, arg(1) ; r10 = outUPtr
	mov rdx, arg(2) ; rdx = outVPtr
	mov r12, arg(3) ; r12 = width
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		xor r9, r9
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width; i += 32)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopWidth:
			prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]
			%if %1 == rgb32Family
				vmovdqa ymm0, [rax + 0]
				vmovdqa ymm1, [rax + 32]
				vmovdqa ymm2, [rax + 64]
				vmovdqa ymm3, [rax + 96]
				lea rax, [rax + 128] ; rgb32Ptr += 128
			%elif %1 == rgb24Family
				; Convert RGB -> RGBA, alpha channel contains zeros	
				COMPV_32xRGB_TO_32xRGBA_AVX2 rax, ymm0, ymm1, ymm2, ymm3, ymm12, ymm11
				lea rax, [rax + 96] ; rgb24Ptr += 96
			%else
				%error 'Not implemented'
			%endif
			lea r9, [r9 + 32] ; i += 32
			vpmaddubsw ymm4, ymm0, ymm9
			vpmaddubsw ymm5, ymm1, ymm9
			vpmaddubsw ymm6, ymm2, ymm9
			vpmaddubsw ymm7, ymm3, ymm9
			vpmaddubsw ymm0, ymm0, ymm8
			vpmaddubsw ymm1, ymm1, ymm8
			vpmaddubsw ymm2, ymm2, ymm8
			vpmaddubsw ymm3, ymm3, ymm8
			cmp r9, r12 ; (i < width)?
			vphaddw ymm4, ymm4, ymm5
			vphaddw ymm6, ymm6, ymm7
			vphaddw ymm0, ymm0, ymm1
			vphaddw ymm2, ymm2, ymm3
			vpsraw ymm4, ymm4, 8
			vpsraw ymm6, ymm6, 8
			vpsraw ymm0, ymm0, 8
			vpsraw ymm2, ymm2, 8
			vpaddw ymm4, ymm4, ymm13
			vpaddw ymm6, ymm6, ymm13
			vpaddw ymm0, ymm0, ymm13
			vpaddw ymm2, ymm2, ymm13
			vpackuswb ymm4, ymm4, ymm6
			vpackuswb ymm0, ymm0, ymm2
			vpermd ymm4, ymm10, ymm4
			vpermd ymm0, ymm10, ymm0
			vmovdqa [rdx], ymm4
			lea rdx, [rdx + 32] ; outVPtr += 32
			vmovdqa [r10], ymm0
			lea r10, [r10 + 32] ; outUPtr += 32
			; end-of-LoopWidth
			jl .LoopWidth

		lea r10, [r10 + r11] ; outUPtr += padUV
		lea rdx, [rdx + r11] ; outUPtr += padUV
		lea rax, [rax + rcx] ; rgbPtr += padRGB
		; end-of-LoopHeight
		dec r8
		jnz .LoopHeight

	; begin epilog
	pop r12
	COMPV_YASM_RESTORE_YMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb24family_to_uv_planar_11_Asm_X64_AVX2)
	CompVImageConvRgbfamily_to_uv_planar_11_Macro_X64_AVX2 rgb24Family

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb32family_to_uv_planar_11_Asm_X64_AVX2)
	CompVImageConvRgbfamily_to_uv_planar_11_Macro_X64_AVX2 rgb32Family

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(AVX) const uint8_t* rgbPtr
; arg(1) -> COMPV_ALIGNED(AVX) uint8_t* outUPtr
; arg(2) -> COMPV_ALIGNED(AVX) uint8_t* outVPtr
; arg(3) -> compv_uscalar_t width
; arg(4) -> compv_uscalar_t height
; arg(5) -> COMPV_ALIGNED(AVX) compv_uscalar_t stride
; arg(6) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_UCoeffs8
; arg(7) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_VCoeffs8
; %1 -> endianness: bigEndian or littleEndian
%macro CompVImageConvRgb565family_to_uv_Macro_X64_AVX2 1
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_YMM 15
	push r12
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 32, rax
	sub rsp, 32 + 32
	%define ymmCoeffRU [rsp + 0]
	%define ymmCoeffGU [rsp + 32]
	%define ymmCoeffBU ymm15
	%define ymmCoeffRV ymm14
	%define ymmCoeffGV ymm13
	%define ymmCoeffBV ymm12

	mov rdx, arg(3)
	lea rdx, [rdx + 31]
	and rdx, -32
	mov rcx, arg(5)
	sub rcx, rdx 
	mov r11, rcx
	%define padUV r11
	shl rcx, 1
	%define padRGB565 rcx

	mov rax, arg(0) ; rax = rgb565Ptr
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*0]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*1]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*2]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]

	mov r8, arg(6)
	movsx r10, byte [r8 + 0]
	vmovd xmm0, r10d
	vpbroadcastw ymm0, xmm0
	vmovdqa ymmCoeffRU, ymm0
	movsx r10, byte [r8 + 1]
	vmovd xmm0, r10d
	vpbroadcastw ymm0, xmm0
	vmovdqa ymmCoeffGU, ymm0
	movsx r10, byte [r8 + 2]
	vmovd xmm0, r10d
	vpbroadcastw ymmCoeffBU, xmm0
	mov r8, arg(7)
	movsx r10, byte [r8 + 0]
	vmovd xmm0, r10d
	vpbroadcastw ymmCoeffRV, xmm0
	movsx r10, byte [r8 + 1]
	vmovd xmm0, r10d
	vpbroadcastw ymmCoeffGV, xmm0
	movsx r10, byte [r8 + 2]
	vmovd xmm0, r10d
	vpbroadcastw ymmCoeffBV, xmm0
	
	mov r8, arg(4) ; r8 = height
	mov r10, arg(1) ; r10 = outUPtr
	mov rdx, arg(2) ; rdx = outVPtr
	mov r12, arg(3) ; r12 = width
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		xor r9, r9
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width; i += 32)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopWidth:
			prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]
			vmovdqa ymm4, [rax + 0]
			vmovdqa ymm5, [rax + 32]
			lea rax, [rax + 64] ; rgb565Ptr += 64
			lea r9, [r9 + 32] ; i += 32
			cmp r9, r12 ; (i < width)?
			%if %1 == bigEndian
				vpsrlw ymm0, ymm4, 8
				vpsrlw ymm1, ymm5, 8
				vpsllw ymm4, ymm4, 8
				vpsllw ymm5, ymm5, 8
				vpor ymm4, ymm4, ymm0
				vpor ymm5, ymm5, ymm1
			%endif
			vpand ymm0, ymm4, [sym(kRGB565ToYUV_RMask_u16)]
			vpand ymm1, ymm5, [sym(kRGB565ToYUV_RMask_u16)]
			vpand ymm2, ymm4, [sym(kRGB565ToYUV_GMask_u16)]
			vpand ymm3, ymm5, [sym(kRGB565ToYUV_GMask_u16)]
			vpand ymm4, ymm4, [sym(kRGB565ToYUV_BMask_u16)]
			vpand ymm5, ymm5, [sym(kRGB565ToYUV_BMask_u16)]
			vpsrlw ymm0, ymm0, 8
			vpsrlw ymm1, ymm1, 8
			vpsrlw ymm2, ymm2, 3
			vpsrlw ymm3, ymm3, 3
			vpsllw ymm4, ymm4, 3
			vpsllw ymm5, ymm5, 3
			vpsrlw ymm6, ymm0, 5
			vpsrlw ymm7, ymm1, 5
			vpsrlw ymm8, ymm2, 6
			vpsrlw ymm9, ymm3, 6
			vpsrlw ymm10, ymm4, 5
			vpsrlw ymm11, ymm5, 5
			vpor ymm6, ymm0, ymm6
			vpor ymm7, ymm1, ymm7
			vpor ymm8, ymm2, ymm8
			vpor ymm9, ymm3, ymm9
			vpor ymm10, ymm4, ymm10
			vpor ymm11, ymm5, ymm11
			vpmullw ymm0, ymm6, ymmCoeffRU
			vpmullw ymm1, ymm7, ymmCoeffRU
			vpmullw ymm2, ymm8, ymmCoeffGU
			vpmullw ymm3, ymm9, ymmCoeffGU
			vpmullw ymm4, ymm10, ymmCoeffBU
			vpmullw ymm5, ymm11, ymmCoeffBU
			vpmullw ymm6, ymm6, ymmCoeffRV
			vpmullw ymm7, ymm7, ymmCoeffRV
			vpmullw ymm8, ymm8, ymmCoeffGV
			vpmullw ymm9, ymm9, ymmCoeffGV
			vpmullw ymm10, ymm10, ymmCoeffBV
			vpmullw ymm11, ymm11, ymmCoeffBV
			vpaddw ymm0, ymm0, ymm2
			vpaddw ymm1, ymm1, ymm3
			vpaddw ymm6, ymm6, ymm8
			vpaddw ymm7, ymm7, ymm9
			vpaddw ymm0, ymm0, ymm4
			vpaddw ymm1, ymm1, ymm5
			vpaddw ymm6, ymm6, ymm10
			vpaddw ymm7, ymm7, ymm11
			vpsraw ymm0, ymm0, 8
			vpsraw ymm1, ymm1, 8
			vpsraw ymm6, ymm6, 8
			vpsraw ymm7, ymm7, 8
			vpaddw ymm0, ymm0, [sym(k128_16s)]			
			vpaddw ymm1, ymm1, [sym(k128_16s)]
			vpaddw ymm6, ymm6, [sym(k128_16s)]			
			vpaddw ymm7, ymm7, [sym(k128_16s)]
			vpackuswb ymm0, ymm0, ymm1
			vpackuswb ymm6, ymm6, ymm7
			vpermq ymm0, ymm0, 0xD8
			vpermq ymm6, ymm6, 0xD8
			vmovdqa [r10], ymm0
			lea r10, [r10 + 32] ; outUPtr += 32
			vmovdqa [rdx], ymm6
			lea rdx, [rdx + 32] ; outVPtr += 32
			; end-of-LoopWidth
			jl .LoopWidth

		lea r10, [r10 + padUV]
		lea rdx, [rdx + padUV]
		lea rax, [rax + padRGB565]
		; end-of-LoopHeight
		dec r8
		jnz .LoopHeight

	; free memory and unalign stack 
	add rsp, 32 + 32
	COMPV_YASM_UNALIGN_STACK

	%undef padUV
	%undef ymmCoeffRU
	%undef ymmCoeffGU
	%undef ymmCoeffBU
	%undef ymmCoeffRV
	%undef ymmCoeffGV
	%undef padRGB565

	;; begin epilog ;;
	pop r12
	COMPV_YASM_RESTORE_YMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb565lefamily_to_uv_Asm_X64_AVX2):
	CompVImageConvRgb565family_to_uv_Macro_X64_AVX2 littleEndian

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb565befamily_to_uv_Asm_X64_AVX2):
	CompVImageConvRgb565family_to_uv_Macro_X64_AVX2 bigEndian

%undef rgb24Family
%undef rgb32Family
%undef bigEndian
%undef littleEndian

%endif ; COMPV_YASM_ABI_IS_64BIT
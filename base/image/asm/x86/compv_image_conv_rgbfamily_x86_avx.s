;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"
%include "compv_image_conv_macros.s"

COMPV_YASM_DEFAULT_REL

%define rgb24Family		0
%define rgb32Family		1
%define bigEndian		2
%define littleEndian	3

global sym(CompVImageConvRgb24family_to_y_Asm_X86_AVX2)
global sym(CompVImageConvRgb32family_to_y_Asm_X86_AVX2)
global sym(CompVImageConvRgb565lefamily_to_y_Asm_X86_AVX2)
global sym(CompVImageConvRgb565befamily_to_y_Asm_X86_AVX2)
global sym(CompVImageConvRgb24family_to_uv_planar_11_Asm_X86_AVX2)
global sym(CompVImageConvRgb32family_to_uv_planar_11_Asm_X86_AVX2)
global sym(CompVImageConvRgb565lefamily_to_uv_Asm_X86_AVX2)
global sym(CompVImageConvRgb565befamily_to_uv_Asm_X86_AVX2)

section .data
	extern sym(k16_i16)
	extern sym(k128_i16)
	extern sym(kAVXPermutevar8x32_ABCDDEFG_i32)
	extern sym(kAVXPermutevar8x32_AEBFCGDH_i32)
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
%macro CompVImageConvRgbfamily_to_y_Macro_X86_AVX2 1
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	COMPV_YASM_SAVE_YMM 7 ;YMM[6-n]
	push rsi
	push rdi
	push rbx
	; end prolog

	mov rax, arg(2)
	lea rax, [rax + 31]
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

	mov rsi, arg(5)
	vmovdqa ymm0, [rsi] ; ymmYCoeffs
	vmovdqa ymm1, [sym(k16_i16)] ; ymm16
	vmovdqa ymm6, [sym(kAVXPermutevar8x32_AEBFCGDH_i32)] ; ymmAEBFCGDH
	vmovdqa ymm7, [sym(kAVXPermutevar8x32_ABCDDEFG_i32)] ; ymmABCDDEFG
	
	mov rsi, arg(3) ; height
	mov rbx, arg(1) ; outYPtr

	.LoopHeight:
		xor rdi, rdi
		.LoopWidth:
			prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]
			%if %1 == rgb32Family
				vmovdqa ymm2, [rax + 0]
				vmovdqa ymm3, [rax + 32]
				vmovdqa ymm4, [rax + 64]
				vmovdqa ymm5, [rax + 96]
				lea rax, [rax + 128] ; rgb32Ptr += 128
			%elif %1 == rgb24Family
				; Convert RGB -> RGBA, alpha channel contains zeros
				COMPV_32xRGB_TO_32xRGBA_AVX2 rax, ymm2, ymm3, ymm4, ymm5, ymm7, [sym(kShuffleEpi8_RgbToRgba_i32)]
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
			lea rdi, [rdi + 32] ; i += 32
			cmp rdi, arg(2) ; (i < width)?
			vpackuswb ymm2, ymm2, ymm4 ; Saturate(I16 -> U8): packus(ACBD, EGFH) -> AEBFCGDH
			vpermd ymm2, ymm6, ymm2
			vmovdqa [rbx], ymm2
			lea rbx, [rbx + 32]
			; end-of-LoopWidth
			jl .LoopWidth
	lea rbx, [rbx + rcx]
	lea rax, [rax + rdx]
	; end-of-LoopHeight
	dec rsi
	jnz .LoopHeight
	
	; begin epilog
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_YMM
    COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb24family_to_y_Asm_X86_AVX2)
	CompVImageConvRgbfamily_to_y_Macro_X86_AVX2 rgb24Family

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb32family_to_y_Asm_X86_AVX2)
	CompVImageConvRgbfamily_to_y_Macro_X86_AVX2 rgb32Family
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(AVX) const uint8_t* rgb565lePtr
; arg(1) -> COMPV_ALIGNED(AVX) uint8_t* outYPtr
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> COMPV_ALIGNED(AVX) compv_uscalar_t stride
; arg(5) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_YCoeffs8
; %1 -> endianness: bigEndian or littleEndian
%macro CompVImageConvRgb565family_to_y_Macro_X86_AVX2 1
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	COMPV_YASM_SAVE_YMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 32, rax
	sub rsp, 32+32+32
	; [rsp + 0] = ymmCoeffR
	; [rsp + 32] = ymmCoeffG
	; [rsp + 64] = ymmCoeffB

	mov rdx, arg(2)
	lea rdx, [rdx + 31]
	and rdx, -32
	mov rcx, arg(4)
	sub rcx, rdx ; rcx = padUV
	mov rdx, rcx
	shl rdx, 1 ; rdx = padRGB565

	mov rax, arg(0) ; rax = rgb565lePtr
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*0]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*1]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*2]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]

	mov rsi, arg(5)
	movzx rbx, byte [rsi + 0]
	vmovd xmm0, ebx
	vpbroadcastw ymm0, xmm0
	vmovdqa [rsp + 0], ymm0
	movzx rbx, byte [rsi + 1]
	vmovd xmm0, ebx
	vpbroadcastw ymm0, xmm0
	vmovdqa [rsp + 32], ymm0
	movzx rbx, byte [rsi + 2]
	vmovd xmm0, ebx
	vpbroadcastw ymm0, xmm0
	vmovdqa [rsp + 64], ymm0
	
	mov rsi, arg(3) ; rsi = height
	mov rbx, arg(1) ; rbx = outYPtr
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		xor rdi, rdi
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width; i += 32)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopWidth:
			prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]
			vmovdqa ymm6, [rax + 0]
			vmovdqa ymm7, [rax + 32]
			lea rax, [rax + 64] ; rgb565lePtr += 64
			lea rdi, [rdi + 32] ; i += 32
			cmp rdi, arg(2) ; (i < width)?
			%if %1 == bigEndian
				vpsrlw ymm0, ymm6, 8
				vpsrlw ymm1, ymm7, 8
				vpsllw ymm6, ymm6, 8
				vpsllw ymm7, ymm7, 8
				vpor ymm6, ymm6, ymm0
				vpor ymm7, ymm7, ymm1
			%endif
			vpand ymm0, ymm6, [sym(kRGB565ToYUV_RMask_u16)]
			vpand ymm1, ymm7, [sym(kRGB565ToYUV_RMask_u16)]
			vpand ymm2, ymm6, [sym(kRGB565ToYUV_GMask_u16)]
			vpand ymm3, ymm7, [sym(kRGB565ToYUV_GMask_u16)]
			vpand ymm4, ymm6, [sym(kRGB565ToYUV_BMask_u16)]
			vpand ymm5, ymm7, [sym(kRGB565ToYUV_BMask_u16)]
			vpsrlw ymm0, ymm0, 8
			vpsrlw ymm1, ymm1, 8
			vpsrlw ymm2, ymm2, 3
			vpsrlw ymm3, ymm3, 3
			vpsllw ymm4, ymm4, 3
			vpsllw ymm5, ymm5, 3
			vpsrlw ymm6, ymm0, 5
			vpsrlw ymm7, ymm1, 5
			vpor ymm0, ymm0, ymm6
			vpor ymm1, ymm1, ymm7
			vpsrlw ymm6, ymm2, 6
			vpsrlw ymm7, ymm3, 6
			vpor ymm2, ymm2, ymm6
			vpor ymm3, ymm3, ymm7
			vpsrlw ymm6, ymm4, 5
			vpsrlw ymm7, ymm5, 5
			vpor ymm4, ymm4, ymm6
			vpor ymm5, ymm5, ymm7
			vpmullw ymm0, ymm0, [rsp + 0]
			vpmullw ymm1, ymm1, [rsp + 0]
			vpmullw ymm2, ymm2, [rsp + 32]
			vpmullw ymm3, ymm3, [rsp + 32]
			vpmullw ymm4, ymm4, [rsp + 64]
			vpmullw ymm5, ymm5, [rsp + 64]
			vpaddw ymm0, ymm0, ymm2
			vpaddw ymm1, ymm1, ymm3
			vpaddw ymm0, ymm0, ymm4
			vpaddw ymm1, ymm1, ymm5
			vpsrlw ymm0, ymm0, 7
			vpsrlw ymm1, ymm1, 7
			vpaddw ymm0, ymm0, [sym(k16_i16)]		
			vpaddw ymm1, ymm1, [sym(k16_i16)]
			vpackuswb ymm0, ymm1
			vpermq ymm0, ymm0, 0xD8
			vmovdqa [rbx], ymm0
			lea rbx, [rbx + 32] ; outYPtr += 32
			; end-of-LoopWidth
			jl .LoopWidth

		lea rbx, [rbx + rcx]
		lea rax, [rax + rdx]
		; end-of-LoopHeight
		dec rsi
		jnz .LoopHeight

	; free memory and unalign stack 
	add rsp, 32+32+32
	COMPV_YASM_UNALIGN_STACK

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_YMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb565lefamily_to_y_Asm_X86_AVX2):
	CompVImageConvRgb565family_to_y_Macro_X86_AVX2 littleEndian

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb565befamily_to_y_Asm_X86_AVX2):
	CompVImageConvRgb565family_to_y_Macro_X86_AVX2 bigEndian
	

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
%macro CompVImageConvRgbfamily_to_uv_planar_11_Macro_X86_AVX2 1
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_YMM 7
	push rsi
	push rdi
	push rbx
	; end prolog

	; alloc memory
	lea rsp, [rsp - COMPV_YASM_REG_SZ_BYTES]
	%define padUV		[rsp + 0]

	mov rdx, arg(3)
	lea rdx, [rdx + 31]
	and rdx, -32
	mov rcx, arg(5)
	sub rcx, rdx
	mov padUV, rcx
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

	mov rsi, arg(6)
	vmovdqa ymm7, [rsi] ; ymm7 = ymmUCoeffs
	mov rsi, arg(7)
	vmovdqa ymm6, [rsi] ; ymm6 = ymmVCoeffs
	
	mov rsi, arg(4) ; rsi = height
	mov rbx, arg(1) ; rbx = outUPtr
	mov rdx, arg(2) ; rdx = outVPtr
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		xor rdi, rdi
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
				; Convert RGB24 -> RGBA32, alpha channel contains zeros
				vmovdqa ymm4, [sym(kAVXPermutevar8x32_ABCDDEFG_i32)] ; ymmABCDDEFG
				COMPV_32xRGB_TO_32xRGBA_AVX2 rax, ymm0, ymm1, ymm2, ymm3, ymm4, [sym(kShuffleEpi8_RgbToRgba_i32)]
				lea rax, [rax + 96] ; rgb24Ptr += 96
			%else
				%error 'Not implemented'
			%endif
			lea rdi, [rdi + 32] ; i += 32
			vpmaddubsw ymm4, ymm0, ymm6
			vpmaddubsw ymm5, ymm1, ymm6
			vpmaddubsw ymm0, ymm0, ymm7
			vpmaddubsw ymm1, ymm1, ymm7
			vphaddw ymm4, ymm4, ymm5
			vphaddw ymm0, ymm0, ymm1
			vpsraw ymm4, ymm4, 8
			vpsraw ymm0, ymm0, 8
			vpaddw ymm4, ymm4, [sym(k128_i16)]
			vpaddw ymm0, ymm0, [sym(k128_i16)]
			cmp rdi, arg(3) ; (i < width)?
			vpmaddubsw ymm1, ymm2, ymm6
			vpmaddubsw ymm5, ymm3, ymm6
			vpmaddubsw ymm2, ymm2, ymm7
			vpmaddubsw ymm3, ymm3, ymm7
			vphaddw ymm1, ymm1, ymm5
			vphaddw ymm2, ymm2, ymm3
			vmovdqa ymm5, [sym(kAVXPermutevar8x32_AEBFCGDH_i32)]
			vpsraw ymm1, ymm1, 8
			vpsraw ymm2, ymm2, 8
			vpaddw ymm1, ymm1, [sym(k128_i16)]
			vpaddw ymm2, ymm2, [sym(k128_i16)]
			vpackuswb ymm4, ymm4, ymm1
			vpackuswb ymm0, ymm0, ymm2
			vpermd ymm0, ymm5, ymm0
			vpermd ymm4, ymm5, ymm4
			vmovdqa [rbx], ymm0
			vmovdqa [rdx], ymm4
			lea rbx, [rbx + 32] ; outUPtr += 32
			lea rdx, [rdx + 32] ; outVPtr += 32
			; end-of-LoopWidth
			jl .LoopWidth

		add rbx, padUV	; outUPtr += padUV
		add rdx, padUV	; outUPtr += padUV
		lea rax, [rax + rcx] ; rgbPtr += padRGB
		; end-of-LoopHeight
		dec rsi
		jnz .LoopHeight

	; free memory
	lea rsp, [rsp + COMPV_YASM_REG_SZ_BYTES]
	%undef padUV

	; begin epilog
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_YMM
    COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb24family_to_uv_planar_11_Asm_X86_AVX2)
	CompVImageConvRgbfamily_to_uv_planar_11_Macro_X86_AVX2 rgb24Family

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb32family_to_uv_planar_11_Asm_X86_AVX2)
	CompVImageConvRgbfamily_to_uv_planar_11_Macro_X86_AVX2 rgb32Family



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(AVX) const uint8_t* rgbPtr
; arg(1) -> COMPV_ALIGNED(AVX) uint8_t* outUPtr
; arg(2) -> COMPV_ALIGNED(AVX) uint8_t* outVPtr
; arg(3) -> compv_uscalar_t width
; arg(4) -> compv_uscalar_t height
; arg(5) -> COMPV_ALIGNED(AVX) compv_uscalar_t stride
; arg(6) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_UCoeffs8
; arg(7) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_VCoeffs8
%macro CompVImageConvRgb565family_to_uv_Macro_X86_AVX2 1
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_YMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 32, rax
	sub rsp, COMPV_YASM_REG_SZ_BYTES + (32+32+32) + (32+32+32) + (32+32+32+32)
	%define padUV [rsp + 0]
	%define ymmCoeffRU [rsp + COMPV_YASM_REG_SZ_BYTES + 0]
	%define ymmCoeffGU [rsp + COMPV_YASM_REG_SZ_BYTES + 32]
	%define ymmCoeffBU [rsp + COMPV_YASM_REG_SZ_BYTES + 64]
	%define ymmCoeffRV [rsp + COMPV_YASM_REG_SZ_BYTES + 96]
	%define ymmCoeffGV [rsp + COMPV_YASM_REG_SZ_BYTES + 128]
	%define ymmCoeffBV [rsp + COMPV_YASM_REG_SZ_BYTES + 160]
	%define ymm0Saved [rsp + COMPV_YASM_REG_SZ_BYTES + 192]
	%define ymm1Saved [rsp + COMPV_YASM_REG_SZ_BYTES + 224]
	%define ymm2Saved [rsp + COMPV_YASM_REG_SZ_BYTES + 256]
	%define ymm3Saved [rsp + COMPV_YASM_REG_SZ_BYTES + 288]

	mov rdx, arg(3)
	lea rdx, [rdx + 31]
	and rdx, -32
	mov rcx, arg(5)
	sub rcx, rdx 
	mov padUV, rcx
	shl rcx, 1
	%define padRGB565 rcx

	mov rax, arg(0) ; rax = rgb565lePtr
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*0]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*1]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*2]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]

	mov rsi, arg(6)
	movsx rbx, byte [rsi + 0]
	vmovd xmm0, ebx
	vpbroadcastw ymm0, xmm0
	vmovdqa ymmCoeffRU, ymm0
	movsx rbx, byte [rsi + 1]
	vmovd xmm0, ebx
	vpbroadcastw ymm0, xmm0
	vmovdqa ymmCoeffGU, ymm0
	movsx rbx, byte [rsi + 2]
	vmovd xmm0, ebx
	vpbroadcastw ymm0, xmm0
	vmovdqa ymmCoeffBU, ymm0
	mov rsi, arg(7)
	movsx rbx, byte [rsi + 0]
	vmovd xmm0, ebx
	vpbroadcastw ymm0, xmm0
	vmovdqa ymmCoeffRV, ymm0
	movsx rbx, byte [rsi + 1]
	vmovd xmm0, ebx
	vpbroadcastw ymm0, xmm0
	vmovdqa ymmCoeffGV, ymm0
	movsx rbx, byte [rsi + 2]
	vmovd xmm0, ebx
	vpbroadcastw ymm0, xmm0
	vmovdqa ymmCoeffBV, ymm0
	
	mov rsi, arg(4) ; rsi = height
	mov rbx, arg(1) ; rbx = outUPtr
	mov rdx, arg(2) ; rdx = outVPtr
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		xor rdi, rdi
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width; i += 32)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopWidth:
			prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]
			vmovdqa ymm6, [rax + 0]
			vmovdqa ymm7, [rax + 32]
			lea rax, [rax + 64] ; rgb565lePtr += 64
			lea rdi, [rdi + 32] ; i += 32
			cmp rdi, arg(3) ; (i < width)?
			%if %1 == bigEndian
				vpsrlw ymm0, ymm6, 8
				vpsrlw ymm1, ymm7, 8
				vpsllw ymm6, ymm6, 8
				vpsllw ymm7, ymm7, 8
				vpor ymm6, ymm6, ymm0
				vpor ymm7, ymm7, ymm1
			%endif
			vpand ymm0, ymm6, [sym(kRGB565ToYUV_RMask_u16)]
			vpand ymm1, ymm7, [sym(kRGB565ToYUV_RMask_u16)]
			vpand ymm2, ymm6, [sym(kRGB565ToYUV_GMask_u16)]
			vpand ymm3, ymm7, [sym(kRGB565ToYUV_GMask_u16)]
			vpand ymm4, ymm6, [sym(kRGB565ToYUV_BMask_u16)]
			vpand ymm5, ymm7, [sym(kRGB565ToYUV_BMask_u16)]
			vpsrlw ymm0, ymm0, 8
			vpsrlw ymm1, ymm1, 8
			vpsrlw ymm2, ymm2, 3
			vpsrlw ymm3, ymm3, 3
			vpsllw ymm4, ymm4, 3
			vpsllw ymm5, ymm5, 3
			vpsrlw ymm6, ymm0, 5
			vpsrlw ymm7, ymm1, 5
			vpor ymm0, ymm0, ymm6
			vpor ymm1, ymm1, ymm7
			vpsrlw ymm6, ymm2, 6
			vpsrlw ymm7, ymm3, 6
			vpor ymm2, ymm2, ymm6
			vpor ymm3, ymm3, ymm7
			vpsrlw ymm6, ymm4, 5
			vpsrlw ymm7, ymm5, 5
			vpor ymm4, ymm4, ymm6
			vpor ymm5, ymm5, ymm7
			vmovdqa ymm0Saved, ymm0
			vmovdqa ymm1Saved, ymm1
			vmovdqa ymm2Saved, ymm2
			vmovdqa ymm3Saved, ymm3
			vmovdqa ymm6, ymm4
			vmovdqa ymm7, ymm5
			vpmullw ymm0, ymm0, ymmCoeffRU
			vpmullw ymm1, ymm1, ymmCoeffRU
			vpmullw ymm2, ymm2, ymmCoeffGU
			vpmullw ymm3, ymm3, ymmCoeffGU
			vpmullw ymm4, ymm4, ymmCoeffBU
			vpmullw ymm5, ymm5, ymmCoeffBU
			vpaddw ymm0, ymm0, ymm2
			vpaddw ymm1, ymm1, ymm3
			vpaddw ymm0, ymm0, ymm4
			vpaddw ymm1, ymm1, ymm5
			vpsraw ymm0, ymm0, 8
			vpsraw ymm1, ymm1, 8
			vpaddw ymm0, ymm0, [sym(k128_i16)]			
			vpaddw ymm1, ymm1, [sym(k128_i16)]
			vpackuswb ymm0, ymm0, ymm1
			vpermq ymm5, ymm0, 0xD8
			vmovdqa ymm0, ymm0Saved
			vmovdqa ymm1, ymm1Saved
			vmovdqa ymm2, ymm2Saved
			vmovdqa ymm3, ymm3Saved
			vpmullw ymm0, ymm0, ymmCoeffRV
			vpmullw ymm1, ymm1, ymmCoeffRV
			vpmullw ymm2, ymm2, ymmCoeffGV
			vpmullw ymm3, ymm3, ymmCoeffGV
			vpmullw ymm6, ymm6, ymmCoeffBV
			vpmullw ymm7, ymm7, ymmCoeffBV
			vpaddw ymm0, ymm0, ymm2
			vpaddw ymm1, ymm1, ymm3
			vpaddw ymm0, ymm0, ymm6
			vpaddw ymm1, ymm1, ymm7
			vpsraw ymm0, ymm0, 8
			vpsraw ymm1, ymm1, 8
			vpaddw ymm0, ymm0, [sym(k128_i16)]			
			vpaddw ymm1, ymm1, [sym(k128_i16)]
			vpackuswb ymm0, ymm1
			vpermq ymm0, ymm0, 0xD8
			vmovdqa [rbx], ymm5
			lea rbx, [rbx + 32] ; outUPtr += 32
			vmovdqa [rdx], ymm0
			lea rdx, [rdx + 32] ; outVPtr += 32
			; end-of-LoopWidth
			jl .LoopWidth

		add rbx, padUV
		add rdx, padUV
		lea rax, [rax + padRGB565]
		; end-of-LoopHeight
		dec rsi
		jnz .LoopHeight

	; free memory and unalign stack 
	add rsp, COMPV_YASM_REG_SZ_BYTES + (32+32+32) + (32+32+32) + (32+32+32+32)
	COMPV_YASM_UNALIGN_STACK

	%undef padUV
	%undef ymmCoeffRU
	%undef ymmCoeffGU
	%undef ymmCoeffBU
	%undef ymmCoeffRV
	%undef ymmCoeffGV
	%undef ymmCoeffBV
	%undef ymm0Saved
	%undef ymm1Saved
	%undef ymm2Saved
	%undef ymm3Saved
	%undef padRGB565

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_YMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb565lefamily_to_uv_Asm_X86_AVX2):
	CompVImageConvRgb565family_to_uv_Macro_X86_AVX2 littleEndian

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb565befamily_to_uv_Asm_X86_AVX2):
	CompVImageConvRgb565family_to_uv_Macro_X86_AVX2 bigEndian


%undef rgb24Family
%undef rgb32Family
%undef bigEndian
%undef littleEndian
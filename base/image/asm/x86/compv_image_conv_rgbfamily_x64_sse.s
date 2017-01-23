;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"
%if COMPV_YASM_ABI_IS_64BIT
%include "compv_image_conv_macros.s"

COMPV_YASM_DEFAULT_REL

%define rgb24Family		0
%define rgb32Family		1
%define bigEndian		2
%define littleEndian	3

global sym(CompVImageConvRgb565lefamily_to_y_Asm_X64_SSE2)
global sym(CompVImageConvRgb565befamily_to_y_Asm_X64_SSE2)
global sym(CompVImageConvRgb24family_to_uv_planar_11_Asm_X64_SSSE3)
global sym(CompVImageConvRgb32family_to_uv_planar_11_Asm_X64_SSSE3)
global sym(CompVImageConvRgb565lefamily_to_uv_Asm_X64_SSE2)
global sym(CompVImageConvRgb565befamily_to_uv_Asm_X64_SSE2)

section .data
	extern sym(k16_i16)
	extern sym(k128_i16)
	extern sym(kShuffleEpi8_RgbToRgba_i32)
	extern sym(kRGB565ToYUV_RMask_u16)
	extern sym(kRGB565ToYUV_GMask_u16)
	extern sym(kRGB565ToYUV_BMask_u16)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* rgb565Ptr
; arg(1) -> COMPV_ALIGNED(SSE) uint8_t* outYPtr
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
; arg(5) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_YCoeffs8
; %1 -> endianness: bigEndian or littleEndian
%macro CompVImageConvRgb565family_to_y_Macro_X64_SSE2 1
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	COMPV_YASM_SAVE_XMM 15
	;; end prolog ;;

	mov rdx, arg(2)
	lea rdx, [rdx + 15]
	and rdx, -16
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
	movd xmm12, r10d
	punpcklwd xmm12, xmm12
	pshufd xmm12, xmm12, 0
	movzx r10, byte [r8 + 1]
	movd xmm13, r10d
	punpcklwd xmm13, xmm13
	pshufd xmm13, xmm13, 0
	movzx r10, byte [r8 + 2]
	movd xmm14, r10d
	punpcklwd xmm14, xmm14
	pshufd xmm14, xmm14, 0
	
	mov r8, arg(3) ; r8 = height
	mov r10, arg(1) ; r10 = outYPtr
	mov r11, arg(2) ; r11 = width

	movdqa xmm15, [sym(k16_i16)]
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		xor r9, r9
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopWidth:
			prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]
			movdqa xmm0, [rax + 0]
			movdqa xmm1, [rax + 16]
			lea rax, [rax + 32] ; rgb565Ptr += 32
			lea r9, [r9 + 16] ; i += 16
			%if %1 == bigEndian
				movdqa xmm2, xmm0
				movdqa xmm3, xmm1
				psrlw xmm0, 8
				psrlw xmm1, 8
				psllw xmm2, 8
				psllw xmm3, 8
				por xmm0, xmm2
				por xmm1, xmm3
			%endif
			movdqa xmm2, xmm0
			movdqa xmm3, xmm1
			movdqa xmm4, xmm0
			movdqa xmm5, xmm1
			cmp r9, r11 ; (i < width)?
			pand xmm0, [sym(kRGB565ToYUV_RMask_u16)]
			pand xmm1, [sym(kRGB565ToYUV_RMask_u16)]
			pand xmm2, [sym(kRGB565ToYUV_GMask_u16)]
			pand xmm3, [sym(kRGB565ToYUV_GMask_u16)]
			pand xmm4, [sym(kRGB565ToYUV_BMask_u16)]
			pand xmm5, [sym(kRGB565ToYUV_BMask_u16)]
			psrlw xmm0, 8
			psrlw xmm1, 8
			psrlw xmm2, 3
			psrlw xmm3, 3
			psllw xmm4, 3
			psllw xmm5, 3
			movdqa xmm6, xmm0
			movdqa xmm7, xmm1
			movdqa xmm8, xmm2
			movdqa xmm9, xmm3
			movdqa xmm10, xmm4
			movdqa xmm11, xmm5
			psrlw xmm6, 5
			psrlw xmm7, 5
			psrlw xmm8, 6
			psrlw xmm9, 6
			psrlw xmm10, 5
			psrlw xmm11, 5
			por xmm0, xmm6
			por xmm1, xmm7	
			por xmm2, xmm8
			por xmm3, xmm9			
			por xmm4, xmm10
			por xmm5, xmm11
			pmullw xmm0, xmm12
			pmullw xmm1, xmm12
			pmullw xmm2, xmm13
			pmullw xmm3, xmm13
			pmullw xmm4, xmm14
			pmullw xmm5, xmm14
			paddw xmm0, xmm2
			paddw xmm1, xmm3
			paddw xmm0, xmm4
			paddw xmm1, xmm5
			psrlw xmm0, 7
			psrlw xmm1, 7
			paddw xmm0, xmm15		
			paddw xmm1, xmm15
			packuswb xmm0, xmm1
			movdqa [r10], xmm0
			lea r10, [r10 + 16] ; outYPtr += 16
			; end-of-LoopWidth
			jl .LoopWidth

		lea r10, [r10 + rcx]
		lea rax, [rax + rdx]
		; end-of-LoopHeight
		dec r8
		jnz .LoopHeight

	;; begin epilog ;;
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb565lefamily_to_y_Asm_X64_SSE2):
	CompVImageConvRgb565family_to_y_Macro_X64_SSE2 littleEndian

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb565befamily_to_y_Asm_X64_SSE2):
	CompVImageConvRgb565family_to_y_Macro_X64_SSE2 bigEndian

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* rgbPtr
; arg(1) -> COMPV_ALIGNED(SSE) uint8_t* outUPtr
; arg(2) -> COMPV_ALIGNED(SSE) uint8_t* outVPtr
; arg(3) -> compv_uscalar_t width
; arg(4) -> compv_uscalar_t height
; arg(5) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
; arg(6) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_UCoeffs8
; arg(7) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_VCoeffs8
; %1 -> family: rgb24Family or rgb32Family
%macro CompVImageConvRgbfamily_to_uv_planar_11_Macro_X64_SSSE3 1
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_XMM 11
	push r12
	; end prolog

	mov rdx, arg(3)
	lea rdx, [rdx + 15]
	and rdx, -16
	mov rcx, arg(5)
	sub rcx, rdx
	mov r11, rcx ; r11 = padUV
	%if %1 == rgb32Family
		shl rcx, 2 ; rdx = padRGBA
	%elif %1 == rgb24Family
		imul rcx, 3 ; rdx = padRGB
	%else
		%error 'Not implemented'
	%endif

	mov rax, arg(0) ; rax = rgbPtr
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*0]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*1]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*2]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]

	mov r8, arg(6)
	movdqa xmm11, [r8] ; xmm11 = xmmUCoeffs
	mov r8, arg(7)
	movdqa xmm10, [r8] ; xmm10 = xmmVCoeffs
	movdqa xmm8, [sym(kShuffleEpi8_RgbToRgba_i32)] ; xmm8 = xmmRgbToRgbaMask
	movdqa xmm9, [sym(k128_i16)] ; xmm9 = xmm128
	
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
		; for (i = 0; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopWidth:
			prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]
			%if %1 == rgb32Family
				movdqa xmm0, [rax + 0]
				movdqa xmm1, [rax + 16]
				movdqa xmm2, [rax + 32]
				movdqa xmm3, [rax + 48]
				lea rax, [rax + 64] ; rgb32Ptr += 64
			%elif %1 == rgb24Family
				; Convert RGB -> RGBA, alpha channel contains zeros
				COMPV_16xRGB_TO_16xRGBA_SSSE3 rax, xmm0, xmm1, xmm2, xmm3, xmm8
				lea rax, [rax + 48] ; rgb24Ptr += 48
			%else
				%error 'Not implemented'
			%endif
			lea r9, [r9 + 16] ; i += 16
			movdqa xmm4, xmm0
			movdqa xmm5, xmm1
			movdqa xmm6, xmm2
			movdqa xmm7, xmm3
			pmaddubsw xmm0, xmm11
			pmaddubsw xmm1, xmm11
			pmaddubsw xmm2, xmm11
			pmaddubsw xmm3, xmm11
			pmaddubsw xmm4, xmm10
			pmaddubsw xmm5, xmm10
			pmaddubsw xmm6, xmm10
			pmaddubsw xmm7, xmm10
			phaddw xmm0, xmm1
			phaddw xmm2, xmm3
			phaddw xmm4, xmm5
			phaddw xmm6, xmm7
			cmp r9, r12 ; (i < width)?	
			psraw xmm0, 8
			psraw xmm2, 8
			psraw xmm4, 8
			psraw xmm6, 8
			paddw xmm0, xmm9
			paddw xmm2, xmm9
			paddw xmm4, xmm9
			paddw xmm6, xmm9
			packuswb xmm0, xmm2
			packuswb xmm4, xmm6
			movdqa [r10], xmm0
			movdqa [rdx], xmm4
			lea r10, [r10 + 16] ; outUPtr += 16
			lea rdx, [rdx + 16] ; outVPtr += 16
			; end-of-LoopWidth
			jl .LoopWidth

		lea r10, [r10 + r11]	; outUPtr += padUV
		lea rdx, [rdx + r11]	; outUPtr += padUV
		lea rax, [rax + rcx]	; rgbXPtr += padRGBX
		; end-of-LoopHeight
		dec r8
		jnz .LoopHeight

	; begin epilog
	pop r12
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb24family_to_uv_planar_11_Asm_X64_SSSE3)
	CompVImageConvRgbfamily_to_uv_planar_11_Macro_X64_SSSE3 rgb24Family

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb32family_to_uv_planar_11_Asm_X64_SSSE3)
	CompVImageConvRgbfamily_to_uv_planar_11_Macro_X64_SSSE3 rgb32Family


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* rgbPtr
; arg(1) -> COMPV_ALIGNED(SSE) uint8_t* outUPtr
; arg(2) -> COMPV_ALIGNED(SSE) uint8_t* outVPtr
; arg(3) -> compv_uscalar_t width
; arg(4) -> compv_uscalar_t height
; arg(5) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
; arg(6) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_UCoeffs8
; arg(7) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_VCoeffs8
; %1 -> endianness: bigEndian or littleEndian
%macro CompVImageConvRgb565family_to_uv_Macro_X64_SSE2 1
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_XMM 15
	push r12
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 16, rax
	sub rsp, 16 + 16
	%define xmmCoeffRU [rsp + 0]
	%define xmmCoeffGU [rsp + 16]
	%define xmmCoeffBU xmm15
	%define xmmCoeffRV xmm14
	%define xmmCoeffGV xmm13
	%define xmmCoeffBV xmm12

	mov rdx, arg(3)
	lea rdx, [rdx + 15]
	and rdx, -16
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
	movd xmm0, r10
	punpcklwd xmm0, xmm0
	pshufd xmm0, xmm0, 0
	movdqa xmmCoeffRU, xmm0
	movsx r10, byte [r8 + 1]
	movd xmm0, r10
	punpcklwd xmm0, xmm0
	pshufd xmm0, xmm0, 0
	movdqa xmmCoeffGU, xmm0
	movsx r10, byte [r8 + 2]
	movd xmmCoeffBU, r10
	punpcklwd xmmCoeffBU, xmmCoeffBU
	pshufd xmmCoeffBU, xmmCoeffBU, 0
	mov r8, arg(7)
	movsx r10, byte [r8 + 0]
	movd xmmCoeffRV, r10
	punpcklwd xmmCoeffRV, xmmCoeffRV
	pshufd xmmCoeffRV, xmmCoeffRV, 0
	movsx r10, byte [r8 + 1]
	movd xmmCoeffGV, r10
	punpcklwd xmmCoeffGV, xmmCoeffGV
	pshufd xmmCoeffGV, xmmCoeffGV, 0
	movsx r10, byte [r8 + 2]
	movd xmmCoeffBV, r10
	punpcklwd xmmCoeffBV, xmmCoeffBV
	pshufd xmmCoeffBV, xmmCoeffBV, 0
	
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
		; for (i = 0; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopWidth:
			prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]
			movdqa xmm0, [rax + 0]
			movdqa xmm1, [rax + 16]
			lea rax, [rax + 32] ; rgb565Ptr += 32
			lea r9, [r9 + 16] ; i += 16
			%if %1 == bigEndian
				movdqa xmm2, xmm0
				movdqa xmm3, xmm1
				psrlw xmm0, 8
				psrlw xmm1, 8
				psllw xmm2, 8
				psllw xmm3, 8
				por xmm0, xmm2
				por xmm1, xmm3
			%endif
			movdqa xmm2, xmm0
			movdqa xmm3, xmm1
			movdqa xmm4, xmm0
			movdqa xmm5, xmm1
			cmp r9, r12 ; (i < width)?
			pand xmm0, [sym(kRGB565ToYUV_RMask_u16)]
			pand xmm1, [sym(kRGB565ToYUV_RMask_u16)]
			pand xmm2, [sym(kRGB565ToYUV_GMask_u16)]
			pand xmm3, [sym(kRGB565ToYUV_GMask_u16)]
			pand xmm4, [sym(kRGB565ToYUV_BMask_u16)]
			pand xmm5, [sym(kRGB565ToYUV_BMask_u16)]
			psrlw xmm0, 8
			psrlw xmm1, 8
			psrlw xmm2, 3
			psrlw xmm3, 3
			psllw xmm4, 3
			psllw xmm5, 3
			movdqa xmm6, xmm0
			movdqa xmm7, xmm1
			movdqa xmm8, xmm2
			movdqa xmm9, xmm3
			movdqa xmm10, xmm4
			movdqa xmm11, xmm5
			psrlw xmm6, 5
			psrlw xmm7, 5
			psrlw xmm8, 6
			psrlw xmm9, 6
			psrlw xmm10, 5
			psrlw xmm11, 5
			por xmm0, xmm6
			por xmm1, xmm7
			por xmm2, xmm8
			por xmm3, xmm9
			por xmm4, xmm10
			por xmm5, xmm11
			movdqa xmm6, xmm0
			movdqa xmm7, xmm1
			movdqa xmm8, xmm2
			movdqa xmm9, xmm3
			movdqa xmm10, xmm4
			movdqa xmm11, xmm5
			pmullw xmm0, xmmCoeffRU
			pmullw xmm1, xmmCoeffRU
			pmullw xmm2, xmmCoeffGU
			pmullw xmm3, xmmCoeffGU
			pmullw xmm4, xmmCoeffBU
			pmullw xmm5, xmmCoeffBU
			pmullw xmm6, xmmCoeffRV
			pmullw xmm7, xmmCoeffRV
			pmullw xmm8, xmmCoeffGV
			pmullw xmm9, xmmCoeffGV
			pmullw xmm10, xmmCoeffBV
			pmullw xmm11, xmmCoeffBV
			paddw xmm0, xmm2
			paddw xmm1, xmm3
			paddw xmm6, xmm8
			paddw xmm7, xmm9
			paddw xmm0, xmm4
			paddw xmm1, xmm5
			paddw xmm6, xmm10
			paddw xmm7, xmm11
			psraw xmm0, 8
			psraw xmm1, 8
			psraw xmm6, 8
			psraw xmm7, 8
			paddw xmm0, [sym(k128_i16)]			
			paddw xmm1, [sym(k128_i16)]
			paddw xmm6, [sym(k128_i16)]			
			paddw xmm7, [sym(k128_i16)]
			packuswb xmm0, xmm1
			packuswb xmm6, xmm7
			movdqa [r10], xmm0
			lea r10, [r10 + 16] ; outUPtr += 16
			movdqa [rdx], xmm6
			lea rdx, [rdx + 16] ; outVPtr += 16
			; end-of-LoopWidth
			jl .LoopWidth

		lea r10, [r10 + padUV]
		lea rdx, [rdx + padUV]
		lea rax, [rax + padRGB565]
		; end-of-LoopHeight
		dec r8
		jnz .LoopHeight

	; free memory and unalign stack 
	add rsp, 16 + 16
	COMPV_YASM_UNALIGN_STACK

	%undef padUV
	%undef xmmCoeffRU
	%undef xmmCoeffGU
	%undef xmmCoeffBU
	%undef xmmCoeffRV
	%undef xmmCoeffGV
	%undef padRGB565

	;; begin epilog ;;
	pop r12
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb565lefamily_to_uv_Asm_X64_SSE2):
	CompVImageConvRgb565family_to_uv_Macro_X64_SSE2 littleEndian

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb565befamily_to_uv_Asm_X64_SSE2):
	CompVImageConvRgb565family_to_uv_Macro_X64_SSE2 bigEndian


%undef rgb24Family
%undef rgb32Family
%undef bigEndian
%undef littleEndian

%endif ; COMPV_YASM_ABI_IS_64BIT
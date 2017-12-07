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

global sym(CompVImageConvRgb24family_to_y_Asm_X86_SSSE3)
global sym(CompVImageConvRgb32family_to_y_Asm_X86_SSSE3)
global sym(CompVImageConvRgb565lefamily_to_y_Asm_X86_SSE2)
global sym(CompVImageConvRgb565befamily_to_y_Asm_X86_SSE2)
global sym(CompVImageConvRgb24family_to_uv_planar_11_Asm_X86_SSSE3)
global sym(CompVImageConvRgb32family_to_uv_planar_11_Asm_X86_SSSE3)
global sym(CompVImageConvRgb565lefamily_to_uv_Asm_X86_SSE2)
global sym(CompVImageConvRgb565befamily_to_uv_Asm_X86_SSE2)

section .data
	extern sym(k16_i16)
	extern sym(k128_i16)
	extern sym(kShuffleEpi8_RgbToRgba_i32)
	extern sym(kRGB565ToYUV_RMask_u16)
	extern sym(kRGB565ToYUV_GMask_u16)
	extern sym(kRGB565ToYUV_BMask_u16)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* rgbPtr
; arg(1) -> COMPV_ALIGNED(SSE) uint8_t* outYPtr
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
; arg(5) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_YCoeffs8
; %1 -> family: rgb24Family or rgb32Family
%macro CompVImageConvRgbfamily_to_y_Macro_X86_SSSE3 1
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	%if %1 == rgb24Family
		COMPV_YASM_SAVE_XMM 6
	%endif
	push rsi
	push rdi
	push rbx
	; end prolog

	mov rdx, arg(2)
	lea rdx, [rdx + 15]
	and rdx, -16
	mov rcx, arg(4)
	sub rcx, rdx ; rcx = padY
	mov rdx, rcx
	%if %1 == rgb32Family
		shl rdx, 2 ; rdx = padRGBA
	%elif %1 == rgb24Family
		imul rdx, 3 ; rdx = padRGB
	%else
		%error 'Not implemented'
	%endif

	mov rax, arg(0) ; rax = rgb24Ptr or rgb32Ptr
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*0]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*1]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*2]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]

	mov rsi, arg(5)
	movdqa xmm0, [rsi] ; xmm0 = xmmYCoeffs
	movdqa xmm1, [sym(k16_i16)] ; xmm1 = xmm16
	%if %1 == rgb24Family
		movdqa xmm6, [sym(kShuffleEpi8_RgbToRgba_i32)] ; xmm6 = xmmRgbToRgbaMask
	%endif
	
	mov rsi, arg(3) ; rsi = height
	mov rbx, arg(1) ; rbx = outYPtr
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		xor rdi, rdi
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopWidth:
			prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]
			%if %1 == rgb32Family
				movdqa xmm2, [rax + 0]
				movdqa xmm3, [rax + 16]
				movdqa xmm4, [rax + 32]
				movdqa xmm5, [rax + 48]
				lea rax, [rax + 64] ; rgb32Ptr += 64
			%elif %1 == rgb24Family
				; Convert RGB24 -> RGB32, alpha channel contains zeros
				COMPV_16xRGB_TO_16xRGBA_SSSE3 rax, xmm2, xmm3, xmm4, xmm5, xmm6
				lea rax, [rax + 48] ; rgb24Ptr += 48
			%else
				%error 'Not implemented'
			%endif
			lea rdi, [rdi + 16] ; i += 16
			pmaddubsw xmm2, xmm0
			pmaddubsw xmm3, xmm0
			pmaddubsw xmm4, xmm0
			pmaddubsw xmm5, xmm0
			phaddw xmm2, xmm3
			phaddw xmm4, xmm5
			cmp rdi, arg(2) ; (i < width)?
			psraw xmm2, 7
			psraw xmm4, 7
			paddw xmm2, xmm1
			paddw xmm4, xmm1
			packuswb xmm2, xmm4
			movdqa [rbx], xmm2
			lea rbx, [rbx + 16] ; outYPtr += 16
			jl .LoopWidth
			; end-of-LoopWidth

		lea rbx, [rbx + rcx]
		lea rax, [rax + rdx]
		dec rsi
		jnz .LoopHeight
		; end-of-LoopHeight

	; begin epilog
	pop rbx
	pop rdi
	pop rsi
	%if %1 == rgb24Family
		COMPV_YASM_RESTORE_XMM
	%endif
    COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb24family_to_y_Asm_X86_SSSE3)
	CompVImageConvRgbfamily_to_y_Macro_X86_SSSE3 rgb24Family

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb32family_to_y_Asm_X86_SSSE3)
	CompVImageConvRgbfamily_to_y_Macro_X86_SSSE3 rgb32Family


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* rgb565lePtr
; arg(1) -> COMPV_ALIGNED(SSE) uint8_t* outYPtr
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
; arg(5) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_YCoeffs8
; %1 -> endianness: bigEndian or littleEndian
%macro CompVImageConvRgb565family_to_y_Macro_X86_SSE2 1
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	COMPV_YASM_SAVE_XMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 16, rax
	sub rsp, 16+16+16
	; [rsp + 0] = xmmCoeffR
	; [rsp + 16] = xmmCoeffG
	; [rsp + 32] = xmmCoeffB

	mov rdx, arg(2)
	lea rdx, [rdx + 15]
	and rdx, -16
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
	movd xmm0, ebx
	punpcklwd xmm0, xmm0
	pshufd xmm0, xmm0, 0
	movdqa [rsp + 0], xmm0
	movzx rbx, byte [rsi + 1]
	movd xmm0, ebx
	punpcklwd xmm0, xmm0
	pshufd xmm0, xmm0, 0
	movdqa [rsp + 16], xmm0
	movzx rbx, byte [rsi + 2]
	movd xmm0, ebx
	punpcklwd xmm0, xmm0
	pshufd xmm0, xmm0, 0
	movdqa [rsp + 32], xmm0
	
	mov rsi, arg(3) ; rsi = height
	mov rbx, arg(1) ; rbx = outYPtr
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		xor rdi, rdi
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopWidth:
			prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]
			movdqa xmm0, [rax + 0]
			movdqa xmm1, [rax + 16]
			lea rax, [rax + 32] ; rgb565lePtr += 32
			lea rdi, [rdi + 16] ; i += 16
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
			cmp rdi, arg(2) ; (i < width)?
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
			psrlw xmm6, 5
			psrlw xmm7, 5
			por xmm0, xmm6
			por xmm1, xmm7
			movdqa xmm6, xmm2
			movdqa xmm7, xmm3
			psrlw xmm6, 6
			psrlw xmm7, 6
			por xmm2, xmm6
			por xmm3, xmm7
			movdqa xmm6, xmm4
			movdqa xmm7, xmm5
			psrlw xmm6, 5
			psrlw xmm7, 5
			por xmm4, xmm6
			por xmm5, xmm7
			pmullw xmm0, [rsp + 0]
			pmullw xmm1, [rsp + 0]
			pmullw xmm2, [rsp + 16]
			pmullw xmm3, [rsp + 16]
			pmullw xmm4, [rsp + 32]
			pmullw xmm5, [rsp + 32]
			paddw xmm0, xmm2
			paddw xmm1, xmm3
			paddw xmm0, xmm4
			paddw xmm1, xmm5
			psrlw xmm0, 7
			psrlw xmm1, 7
			paddw xmm0, [sym(k16_i16)]			
			paddw xmm1, [sym(k16_i16)]
			packuswb xmm0, xmm1
			movdqa [rbx], xmm0
			lea rbx, [rbx + 16] ; outYPtr += 16
			; end-of-LoopWidth
			jl .LoopWidth

		lea rbx, [rbx + rcx]
		lea rax, [rax + rdx]
		; end-of-LoopHeight
		dec rsi
		jnz .LoopHeight

	; free memory and unalign stack 
	add rsp, 16+16+16
	COMPV_YASM_UNALIGN_STACK

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb565lefamily_to_y_Asm_X86_SSE2):
	CompVImageConvRgb565family_to_y_Macro_X86_SSE2 littleEndian

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb565befamily_to_y_Asm_X86_SSE2):
	CompVImageConvRgb565family_to_y_Macro_X86_SSE2 bigEndian

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
%macro CompVImageConvRgbfamily_to_uv_planar_11_Macro_X86_SSSE3 1
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_XMM 7
	push rsi
	push rdi
	push rbx
	; end prolog

	; alloc memory
	lea rsp, [rsp - COMPV_YASM_REG_SZ_BYTES]
	%define padUV		[rsp + 0]

	mov rdx, arg(3)
	lea rdx, [rdx + 15]
	and rdx, -16
	mov rcx, arg(5)
	sub rcx, rdx
	mov padUV, rcx
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

	mov rsi, arg(6)
	movdqa xmm7, [rsi] ; xmm7 = xmmUCoeffs
	mov rsi, arg(7)
	movdqa xmm6, [rsi] ; xmm6 = xmmVCoeffs
	
	mov rsi, arg(4) ; rsi = height
	mov rbx, arg(1) ; rbx = outUPtr
	mov rdx, arg(2) ; rdx = outVPtr
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		xor rdi, rdi
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
				movdqa xmm4, [sym(kShuffleEpi8_RgbToRgba_i32)]
				COMPV_16xRGB_TO_16xRGBA_SSSE3 rax, xmm0, xmm1, xmm2, xmm3, xmm4
				lea rax, [rax + 48] ; rgb24Ptr += 48
			%else
				%error 'Not implemented'
			%endif
			lea rdi, [rdi + 16] ; i += 16
			movdqa xmm4, xmm0
			movdqa xmm5, xmm1
			pmaddubsw xmm0, xmm7
			pmaddubsw xmm1, xmm7
			pmaddubsw xmm4, xmm6
			pmaddubsw xmm5, xmm6
			phaddw xmm0, xmm1
			phaddw xmm4, xmm5
			movdqa xmm1, xmm2
			movdqa xmm5, xmm3
			psraw xmm0, 8
			psraw xmm4, 8
			paddw xmm0, [sym(k128_i16)]
			paddw xmm4, [sym(k128_i16)]
			cmp rdi, arg(3) ; (i < width)?		
			pmaddubsw xmm2, xmm7
			pmaddubsw xmm3, xmm7
			pmaddubsw xmm1, xmm6
			pmaddubsw xmm5, xmm6
			phaddw xmm2, xmm3
			phaddw xmm1, xmm5
			psraw xmm2, 8
			psraw xmm1, 8
			paddw xmm2, [sym(k128_i16)]
			paddw xmm1, [sym(k128_i16)]
			packuswb xmm0, xmm2
			packuswb xmm4, xmm1
			movdqa [rbx], xmm0
			movdqa [rdx], xmm4
			lea rbx, [rbx + 16] ; outUPtr += 16
			lea rdx, [rdx + 16] ; outVPtr += 16
			; end-of-LoopWidth
			jl .LoopWidth

		add rbx, padUV	; outUPtr += padUV
		add rdx, padUV	; outUPtr += padUV
		lea rax, [rax + rcx] ; rgbXPtr += padRGBX
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
	COMPV_YASM_RESTORE_XMM
    COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb24family_to_uv_planar_11_Asm_X86_SSSE3)
	CompVImageConvRgbfamily_to_uv_planar_11_Macro_X86_SSSE3 rgb24Family

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb32family_to_uv_planar_11_Asm_X86_SSSE3)
	CompVImageConvRgbfamily_to_uv_planar_11_Macro_X86_SSSE3 rgb32Family


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
%macro CompVImageConvRgb565family_to_uv_Macro_X86_SSE2 1
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_XMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 16, rax
	sub rsp, COMPV_YASM_REG_SZ_BYTES + (16+16+16) + (16+16+16) + (16+16+16+16)
	%define padUV [rsp + 0]
	%define xmmCoeffRU [rsp + COMPV_YASM_REG_SZ_BYTES + 0]
	%define xmmCoeffGU [rsp + COMPV_YASM_REG_SZ_BYTES + 16]
	%define xmmCoeffBU [rsp + COMPV_YASM_REG_SZ_BYTES + 32]
	%define xmmCoeffRV [rsp + COMPV_YASM_REG_SZ_BYTES + 48]
	%define xmmCoeffGV [rsp + COMPV_YASM_REG_SZ_BYTES + 64]
	%define xmmCoeffBV [rsp + COMPV_YASM_REG_SZ_BYTES + 80]
	%define xmm0Saved [rsp + COMPV_YASM_REG_SZ_BYTES + 96]
	%define xmm1Saved [rsp + COMPV_YASM_REG_SZ_BYTES + 112]
	%define xmm2Saved [rsp + COMPV_YASM_REG_SZ_BYTES + 128]
	%define xmm3Saved [rsp + COMPV_YASM_REG_SZ_BYTES + 144]

	mov rdx, arg(3)
	lea rdx, [rdx + 15]
	and rdx, -16
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
	movd xmm0, ebx
	punpcklwd xmm0, xmm0
	pshufd xmm0, xmm0, 0
	movdqa xmmCoeffRU, xmm0
	movsx rbx, byte [rsi + 1]
	movd xmm0, ebx
	punpcklwd xmm0, xmm0
	pshufd xmm0, xmm0, 0
	movdqa xmmCoeffGU, xmm0
	movsx rbx, byte [rsi + 2]
	movd xmm0, ebx
	punpcklwd xmm0, xmm0
	pshufd xmm0, xmm0, 0
	movdqa xmmCoeffBU, xmm0
	mov rsi, arg(7)
	movsx rbx, byte [rsi + 0]
	movd xmm0, ebx
	punpcklwd xmm0, xmm0
	pshufd xmm0, xmm0, 0
	movdqa xmmCoeffRV, xmm0
	movsx rbx, byte [rsi + 1]
	movd xmm0, ebx
	punpcklwd xmm0, xmm0
	pshufd xmm0, xmm0, 0
	movdqa xmmCoeffGV, xmm0
	movsx rbx, byte [rsi + 2]
	movd xmm0, ebx
	punpcklwd xmm0, xmm0
	pshufd xmm0, xmm0, 0
	movdqa xmmCoeffBV, xmm0
	
	mov rsi, arg(4) ; rsi = height
	mov rbx, arg(1) ; rbx = outUPtr
	mov rdx, arg(2) ; rdx = outVPtr
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		xor rdi, rdi
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopWidth:
			prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]
			movdqa xmm0, [rax + 0]
			movdqa xmm1, [rax + 16]
			lea rax, [rax + 32] ; rgb565lePtr += 32
			lea rdi, [rdi + 16] ; i += 16
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
			cmp rdi, arg(3) ; (i < width)?
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
			psrlw xmm6, 5
			psrlw xmm7, 5
			por xmm0, xmm6
			por xmm1, xmm7
			movdqa xmm6, xmm2
			movdqa xmm7, xmm3
			psrlw xmm6, 6
			psrlw xmm7, 6
			por xmm2, xmm6
			por xmm3, xmm7
			movdqa xmm6, xmm4
			movdqa xmm7, xmm5
			psrlw xmm6, 5
			psrlw xmm7, 5
			por xmm4, xmm6
			por xmm5, xmm7
			movdqa xmm0Saved, xmm0
			movdqa xmm1Saved, xmm1
			movdqa xmm2Saved, xmm2
			movdqa xmm3Saved, xmm3
			movdqa xmm6, xmm4
			movdqa xmm7, xmm5
			pmullw xmm0, xmmCoeffRU
			pmullw xmm1, xmmCoeffRU
			pmullw xmm2, xmmCoeffGU
			pmullw xmm3, xmmCoeffGU
			pmullw xmm4, xmmCoeffBU
			pmullw xmm5, xmmCoeffBU
			paddw xmm0, xmm2
			paddw xmm1, xmm3
			paddw xmm0, xmm4
			paddw xmm1, xmm5
			psraw xmm0, 8
			psraw xmm1, 8
			paddw xmm0, [sym(k128_i16)]			
			paddw xmm1, [sym(k128_i16)]
			packuswb xmm0, xmm1
			movdqa [rbx], xmm0
			lea rbx, [rbx + 16] ; outUPtr += 16
			movdqa xmm0, xmm0Saved
			movdqa xmm1, xmm1Saved
			movdqa xmm2, xmm2Saved
			movdqa xmm3, xmm3Saved
			pmullw xmm0, xmmCoeffRV
			pmullw xmm1, xmmCoeffRV
			pmullw xmm2, xmmCoeffGV
			pmullw xmm3, xmmCoeffGV
			pmullw xmm6, xmmCoeffBV
			pmullw xmm7, xmmCoeffBV
			paddw xmm0, xmm2
			paddw xmm1, xmm3
			paddw xmm0, xmm6
			paddw xmm1, xmm7
			psraw xmm0, 8
			psraw xmm1, 8
			paddw xmm0, [sym(k128_i16)]			
			paddw xmm1, [sym(k128_i16)]
			packuswb xmm0, xmm1
			movdqa [rdx], xmm0
			lea rdx, [rdx + 16] ; outVPtr += 16
			; end-of-LoopWidth
			jl .LoopWidth

		add rbx, padUV
		add rdx, padUV
		lea rax, [rax + padRGB565]
		; end-of-LoopHeight
		dec rsi
		jnz .LoopHeight

	; free memory and unalign stack 
	add rsp, COMPV_YASM_REG_SZ_BYTES + (16+16+16) + (16+16+16) + (16+16+16+16)
	COMPV_YASM_UNALIGN_STACK

	%undef padUV
	%undef xmmCoeffRU
	%undef xmmCoeffGU
	%undef xmmCoeffBU
	%undef xmmCoeffRV
	%undef xmmCoeffGV
	%undef xmmCoeffBV
	%undef xmm0Saved
	%undef xmm1Saved
	%undef xmm2Saved
	%undef xmm3Saved
	%undef padRGB565

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endmacro


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb565lefamily_to_uv_Asm_X86_SSE2):
	CompVImageConvRgb565family_to_uv_Macro_X86_SSE2 littleEndian

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVImageConvRgb565befamily_to_uv_Asm_X86_SSE2):
	CompVImageConvRgb565family_to_uv_Macro_X86_SSE2 bigEndian

%undef rgb24Family
%undef rgb32Family
%undef bigEndian
%undef littleEndian
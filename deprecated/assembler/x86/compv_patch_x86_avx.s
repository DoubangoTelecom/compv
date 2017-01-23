;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(Moments0110_Asm_X86_AVX2)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(AVX2) const uint8_t* top
; arg(1) -> COMPV_ALIGNED(AVX2)const uint8_t* bottom
; arg(2) -> COMPV_ALIGNED(AVX2)const int16_t* x
; arg(3) -> COMPV_ALIGNED(AVX2) const int16_t* y
; arg(4) -> compv_scalar_t count
; arg(5) -> compv_scalar_t* s01
; arg(6) -> compv_scalar_t* s10
; void Moments0110_Asm_X86_AVX2(COMPV_ALIGNED(AVX2) const uint8_t* top, COMPV_ALIGNED(AVX2)const uint8_t* bottom, COMPV_ALIGNED(AVX2)const int16_t* x, COMPV_ALIGNED(AVX2) const int16_t* y, compv_scalar_t count, compv_scalar_t* s01, compv_scalar_t* s10)
sym(Moments0110_Asm_X86_AVX2):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_YMM 7 ;YMM[6-7]
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	%define COMPV_SIZE_OF_INT16 2

	; ymm7 = ymmZero
	vpxor ymm7, ymm7

	; rsi = i
	xor rsi, rsi

	mov rax, arg(5)
	mov rbx, arg(6)

	; rcx = [s01]
	mov rcx, [rax]

	; rdx = [s10]
	mov rdx, [rbx]

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_scalar_t i = 0; i < count; i += 32)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopRows
		mov rdi, arg(0) ; top
		mov rbx, arg(1) ; bottom
		vmovdqa ymm0, [rdi + rsi] ; ymm0 = ymmTop
		vmovdqa ymm1, [rbx + rsi] ; ymm1 = ymmBottom
		vpermq ymm0, ymm0, 0xD8
		vpermq ymm1, ymm1, 0xD8
		vmovdqa ymm2, ymm0 ; ymm2 = ymmTop
		vmovdqa ymm3, ymm1 ; ymm3 = ymmBottom

		mov rdi, arg(2) ; x
		mov rbx, arg(3) ; y

		vpunpcklbw ymm4, ymm0, ymm7
		vpunpcklbw ymm5, ymm1, ymm7
		vpaddw ymm0, ymm4, ymm5
		vpmullw ymm0, [rdi + rsi*COMPV_SIZE_OF_INT16]
		vpunpcklwd ymm1, ymm7, ymm0
		vpunpckhwd ymm6, ymm7, ymm0
		vpsrad ymm1, ymm1, 16
		vpsrad ymm6, ymm6, 16
		vpsubw ymm4, ymm4, ymm5
		vphaddd ymm1, ymm1, ymm6
		vpmullw ymm4, ymm4, [rbx + rsi*COMPV_SIZE_OF_INT16]
		vpunpcklwd ymm0, ymm7, ymm4
		vpunpckhwd ymm5, ymm7, ymm4
		vpsrad ymm0, ymm0, 16
		vpsrad ymm5, ymm5, 16
		vpunpckhbw ymm4, ymm2, ymm7
		vphaddd ymm0, ymm0, ymm5
		vpunpckhbw ymm5, ymm3, ymm7
		vpaddw ymm2, ymm4, ymm5
		vphaddd ymm1, ymm1, ymm0
		vpmullw ymm2, ymm2, [rdi + rsi*COMPV_SIZE_OF_INT16 + 16*COMPV_SIZE_OF_INT16]
		vpunpcklwd ymm0, ymm7, ymm2
		vpunpckhwd ymm6, ymm7, ymm2
		vpsubw ymm4, ymm4, ymm5
		vpsrad ymm0, ymm0, 16
		vpsrad ymm6, ymm6, 16
		vpmullw ymm4, ymm4, [rbx + rsi*COMPV_SIZE_OF_INT16 + 16*COMPV_SIZE_OF_INT16]
		vpunpcklwd ymm2, ymm7, ymm4
		vpunpckhwd ymm5, ymm7, ymm4
		vpsrad ymm2, ymm2, 16
		vpsrad ymm5, ymm5, 16
		vphaddd ymm0, ymm0, ymm6
		vphaddd ymm2, ymm2, ymm5
		vphaddd ymm0, ymm0, ymm2
		vphaddd ymm1, ymm1, ymm0

		vextracti128 xmm0, ymm1, 0
		vextracti128 xmm1, ymm1, 1

		vpextrd dword edi, xmm0, 1
		vpextrd dword ebx, xmm0, 3
		add dword ecx, edi
		add dword ecx, ebx
		vpextrd dword edi, xmm1, 1
		vpextrd dword ebx, xmm1, 3
		add dword ecx, edi
		add dword ecx, ebx

		vpextrd dword edi, xmm0, 0
		vpextrd dword ebx, xmm0, 2
		add dword edx, edi
		add dword edx, ebx
		vpextrd dword edi, xmm1, 0
		vpextrd dword ebx, xmm1, 2
		add dword edx, edi
		add dword edx, ebx

		add rsi, 32
		cmp rsi, arg(4)
		jl .LoopRows

	mov rax, arg(5)
	mov rbx, arg(6)
	mov [rax], rcx
	mov [rbx], rdx

	%undef COMPV_SIZE_OF_INT16

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
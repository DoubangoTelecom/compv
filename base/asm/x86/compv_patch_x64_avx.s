;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(CompVPatchRadiusLte64Moments0110_Asm_X64_AVX2)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> vCOMPV_ALIGNED(SSE) const uint8_t* top
; arg(1) -> vCOMPV_ALIGNED(SSE) const uint8_t* bottom
; arg(2) -> vCOMPV_ALIGNED(SSE) const int16_t* x
; arg(3) -> vCOMPV_ALIGNED(SSE) const int16_t* y
; arg(4) -> vcompv_uscalar_t count
; arg(5) -> vcompv_scalar_t* s01
; arg(6) -> vcompv_scalar_t* s10
sym(CompVPatchRadiusLte64Moments0110_Asm_X64_AVX2):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_YMM 15
	;; end prolog ;;

	%define i		r8
	%define top		r9
	%define bottom  r10
	%define x		rcx
	%define y		rdx
	%define count	rax

	mov top, arg(0)
	mov bottom, arg(1)
	mov x, arg(2)
	mov y, arg(3)
	mov count, arg(4)

	vpxor ymm8, ymm8
	vpxor ymm9, ymm9
	vpxor ymm10, ymm10
	vpxor ymm11, ymm11
	vpxor ymm12, ymm12
	vpxor ymm13, ymm13
	vpxor ymm14, ymm14
	vpxor ymm15, ymm15

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t i = 0; i < count; i += 16)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	xor i, i
	.LoopCount:
		vmovdqa ymm0, [top + i*COMPV_YASM_UINT8_SZ_BYTES]
		vmovdqa ymm1, [bottom + i*COMPV_YASM_UINT8_SZ_BYTES]

		vpmovzxbw ymm2, xmm0
		vpmovzxbw ymm3, xmm1
		vpaddw ymm4, ymm2, ymm3
		vpsubw ymm2, ymm2, ymm3
		vpmullw ymm4, [x + (i + 0)*COMPV_YASM_INT16_SZ_BYTES]
		vpmullw ymm2, [y + (i + 0)*COMPV_YASM_INT16_SZ_BYTES]

		vextracti128 xmm0, ymm0, 0x1
		vextracti128 xmm1, ymm1, 0x1
		vpmovzxbw ymm0, xmm0
		vpmovzxbw ymm1, xmm1
		vpaddw ymm3, ymm0, ymm1
		vpsubw ymm0, ymm0, ymm1
		vpmullw ymm3, ymm3, [x + (i + 16)*COMPV_YASM_INT16_SZ_BYTES]
		vpmullw ymm0, ymm0, [y + (i + 16)*COMPV_YASM_INT16_SZ_BYTES]
		add i, 32
		vpmovsxwd ymm1, xmm4
		vpmovsxwd ymm5, xmm2
		vextracti128 xmm4, ymm4, 0x1
		vextracti128 xmm2, ymm2, 0x1
		vpmovsxwd ymm4, xmm4		
		vpmovsxwd ymm2, xmm2
		vpmovsxwd ymm6, xmm3
		vpmovsxwd ymm7, xmm0
		vextracti128 xmm3, ymm3, 0x1
		vextracti128 xmm0, ymm0, 0x1
		vpmovsxwd ymm3, xmm3		
		vpmovsxwd ymm0, xmm0

		vpaddd ymm8, ymm8, ymm1
		vpaddd ymm9, ymm9, ymm4
		vpaddd ymm10, ymm10, ymm6
		vpaddd ymm11, ymm11, ymm3
		cmp i, count
		vpaddd ymm12, ymm12, ymm5
		vpaddd ymm13, ymm13, ymm2
		vpaddd ymm14, ymm14, ymm7
		vpaddd ymm15, ymm15, ymm0	
		jl .LoopCount
		;; EndOf_LoopCount ;;

	vpaddd ymm8, ymm8, ymm9
	vpaddd ymm10, ymm10, ymm11
	vpaddd ymm12, ymm12, ymm13
	vpaddd ymm14, ymm14, ymm15

	vpaddd ymm8, ymm8, ymm10
	vpaddd ymm12, ymm12, ymm14

	vphaddd ymm0, ymm8, ymm12
	vphaddd ymm0, ymm0, ymm0

	mov rdi, arg(6) ; s10
	mov rsi, arg(5) ; s01

	vextracti128 xmm1, ymm0, 0x1
	vpaddd xmm0, xmm0, xmm1
	vmovd eax, xmm0
	vpextrd edx, xmm0, 0x1

	add [rdi], dword eax
	add [rsi], dword edx

	%undef i		
	%undef top		
	%undef bottom  
	%undef x		
	%undef y
	%undef count

	;; begin epilog ;;
	COMPV_YASM_RESTORE_YMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret

%endif ; COMPV_YASM_ABI_IS_64BIT
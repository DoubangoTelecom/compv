;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(CompVPatchRadiusLte64Moments0110_Asm_X86_AVX2)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> vCOMPV_ALIGNED(AVX) const uint8_t* top
; arg(1) -> vCOMPV_ALIGNED(AVX) const uint8_t* bottom
; arg(2) -> vCOMPV_ALIGNED(AVX) const int16_t* x
; arg(3) -> vCOMPV_ALIGNED(AVX) const int16_t* y
; arg(4) -> vcompv_uscalar_t count
; arg(5) -> vcompv_scalar_t* s01
; arg(6) -> vcompv_scalar_t* s10
sym(CompVPatchRadiusLte64Moments0110_Asm_X86_AVX2):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_YMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 32, rax
	sub rsp, (4*COMPV_YASM_YMM_SZ_BYTES) + (4*COMPV_YASM_YMM_SZ_BYTES)
	    
	%define vecS10          rsp + 0
	%define vecS01			vecS10 + (4*COMPV_YASM_YMM_SZ_BYTES)

	%define i		rsi
	%define top		rdi
	%define bottom  rbx
	%define x		rcx
	%define y		rdx
	%define count	rax

	mov top, arg(0)
	mov bottom, arg(1)
	mov x, arg(2)
	mov y, arg(3)
	mov count, arg(4)

	vpxor ymm0, ymm0
	%assign index 0
	%rep 4
		vmovdqa [vecS10 + index*COMPV_YASM_YMM_SZ_BYTES], ymm0
		vmovdqa [vecS01 + index*COMPV_YASM_YMM_SZ_BYTES], ymm0
		%assign index index+1
	%endrep

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t i = 0; i < count; i += 32)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	xor i, i
	.LoopCount:
		vmovdqa ymm0, [top + i*COMPV_YASM_UINT8_SZ_BYTES]
		vmovdqa ymm1, [bottom + i*COMPV_YASM_UINT8_SZ_BYTES]

		vpmovzxbw ymm2, xmm0
		vpmovzxbw ymm3, xmm1
		vpaddw ymm4, ymm2, ymm3
		vpsubw ymm2, ymm2, ymm3
		vpmullw ymm4, ymm4, [x + (i + 0)*COMPV_YASM_INT16_SZ_BYTES]
		vpmullw ymm2, ymm2, [y + (i + 0)*COMPV_YASM_INT16_SZ_BYTES]

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
		vpmovsxwd ymm6, xmm3
		vpmovsxwd ymm7, xmm0
		vextracti128 xmm4, ymm4, 0x1
		vextracti128 xmm2, ymm2, 0x1
		vextracti128 xmm3, ymm3, 0x1
		vextracti128 xmm0, ymm0, 0x1
		vpmovsxwd ymm4, xmm4		
		vpmovsxwd ymm2, xmm2		
		vpmovsxwd ymm3, xmm3		
		vpmovsxwd ymm0, xmm0
		; Do not change: cache-friendly
		vpaddd ymm1, ymm1, [vecS10 + 0*COMPV_YASM_YMM_SZ_BYTES]
		vpaddd ymm4, ymm4, [vecS10 + 1*COMPV_YASM_YMM_SZ_BYTES]
		vpaddd ymm6, ymm6, [vecS10 + 2*COMPV_YASM_YMM_SZ_BYTES]
		vpaddd ymm3, ymm3, [vecS10 + 3*COMPV_YASM_YMM_SZ_BYTES]
		vpaddd ymm5, ymm5, [vecS01 + 0*COMPV_YASM_YMM_SZ_BYTES]
		vpaddd ymm2, ymm2, [vecS01 + 1*COMPV_YASM_YMM_SZ_BYTES]
		vpaddd ymm7, ymm7, [vecS01 + 2*COMPV_YASM_YMM_SZ_BYTES]
		vpaddd ymm0, ymm0, [vecS01 + 3*COMPV_YASM_YMM_SZ_BYTES]
		cmp i, count
		vmovdqa [vecS10 + 0*COMPV_YASM_YMM_SZ_BYTES], ymm1
		vmovdqa [vecS10 + 1*COMPV_YASM_YMM_SZ_BYTES], ymm4
		vmovdqa [vecS10 + 2*COMPV_YASM_YMM_SZ_BYTES], ymm6
		vmovdqa [vecS10 + 3*COMPV_YASM_YMM_SZ_BYTES], ymm3
		vmovdqa [vecS01 + 0*COMPV_YASM_YMM_SZ_BYTES], ymm5
		vmovdqa [vecS01 + 1*COMPV_YASM_YMM_SZ_BYTES], ymm2
		vmovdqa [vecS01 + 2*COMPV_YASM_YMM_SZ_BYTES], ymm7
		vmovdqa [vecS01 + 3*COMPV_YASM_YMM_SZ_BYTES], ymm0		
		jl .LoopCount
		;; EndOf_LoopCount ;;

	vmovdqa ymm0, [vecS10 + 0*COMPV_YASM_YMM_SZ_BYTES]
	vmovdqa ymm1, [vecS10 + 2*COMPV_YASM_YMM_SZ_BYTES]
	vmovdqa ymm2, [vecS01 + 0*COMPV_YASM_YMM_SZ_BYTES]
	vmovdqa ymm3, [vecS01 + 2*COMPV_YASM_YMM_SZ_BYTES]

	vpaddd ymm0, ymm0, [vecS10 + 1*COMPV_YASM_YMM_SZ_BYTES]
	vpaddd ymm1, ymm1, [vecS10 + 3*COMPV_YASM_YMM_SZ_BYTES]
	vpaddd ymm2, ymm2, [vecS01 + 1*COMPV_YASM_YMM_SZ_BYTES]
	vpaddd ymm3, ymm3, [vecS01 + 3*COMPV_YASM_YMM_SZ_BYTES]

	vpaddd ymm0, ymm0, ymm1
	vpaddd ymm2, ymm2, ymm3

	vphaddd ymm0, ymm0, ymm2
	vphaddd ymm0, ymm0, ymm0

	mov rdi, arg(6) ; s10
	mov rsi, arg(5) ; s01

	vextracti128 xmm1, ymm0, 0x1
	vpaddd xmm0, xmm0, xmm1
	vmovd eax, xmm0
	vpextrd edx, xmm0, 0x1

	add [rdi], dword eax
	add [rsi], dword edx
	
	%undef vecS10
	%undef vecS01

	%undef i		
	%undef top		
	%undef bottom  
	%undef x		
	%undef y
	%undef count

	; free memory and unalign stack
	add rsp, (4*COMPV_YASM_YMM_SZ_BYTES) + (4*COMPV_YASM_YMM_SZ_BYTES)
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


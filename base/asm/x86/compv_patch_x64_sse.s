;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(CompVPatchRadiusLte64Moments0110_Asm_X64_SSE2)

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
sym(CompVPatchRadiusLte64Moments0110_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_XMM 15
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 16, rax
	sub rsp, (1*COMPV_YASM_XMM_SZ_BYTES)

	%define vecZero         rsp + 0

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

	pxor xmm8, xmm8
	pxor xmm9, xmm9
	pxor xmm10, xmm10
	pxor xmm11, xmm11
	pxor xmm12, xmm12
	pxor xmm13, xmm13
	pxor xmm14, xmm14
	pxor xmm15, xmm15
	movdqa [vecZero], xmm8
	

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t i = 0; i < count; i += 16)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	xor i, i
	.LoopCount:
		movdqa xmm0, [top + i*COMPV_YASM_UINT8_SZ_BYTES]
		movdqa xmm1, [bottom + i*COMPV_YASM_UINT8_SZ_BYTES]
		movdqa xmm2, xmm0
		movdqa xmm3, xmm1

		punpcklbw xmm2, [vecZero]
		punpcklbw xmm3, [vecZero]
		movdqa xmm4, xmm2
		paddw xmm4, xmm3
		psubw xmm2, xmm3
		pmullw xmm4, [x + (i + 0)*COMPV_YASM_INT16_SZ_BYTES]
		pmullw xmm2, [y + (i + 0)*COMPV_YASM_INT16_SZ_BYTES]

		punpckhbw xmm0, [vecZero]
		punpckhbw xmm1, [vecZero]
		movdqa xmm3, xmm0
		paddw xmm3, xmm1
		psubw xmm0, xmm1
		pmullw xmm3, [x + (i + 8)*COMPV_YASM_INT16_SZ_BYTES]
		pmullw xmm0, [y + (i + 8)*COMPV_YASM_INT16_SZ_BYTES]
		add i, 16
		movdqa xmm1, xmm4
		movdqa xmm5, xmm2
		punpcklwd xmm1, xmm1
		punpckhwd xmm4, xmm4
		punpcklwd xmm5, xmm5
		punpckhwd xmm2, xmm2
		cmp i, count
		movdqa xmm6, xmm3
		movdqa xmm7, xmm0
		punpcklwd xmm6, xmm6
		punpckhwd xmm3, xmm3
		punpcklwd xmm7, xmm7
		punpckhwd xmm0, xmm0
		psrad xmm1, 16
		psrad xmm4, 16
		psrad xmm5, 16
		psrad xmm2, 16
		psrad xmm6, 16
		psrad xmm3, 16
		psrad xmm7, 16
		psrad xmm0, 16
		paddd xmm8, xmm1
		paddd xmm9, xmm4
		paddd xmm10, xmm6
		paddd xmm11, xmm3
		paddd xmm12, xmm5
		paddd xmm13, xmm2
		paddd xmm14, xmm7
		paddd xmm15, xmm0	
		jl .LoopCount
		;; EndOf_LoopCount ;;

	paddd xmm8, xmm9
	paddd xmm10, xmm11
	paddd xmm12, xmm13
	paddd xmm14, xmm15

	paddd xmm8, xmm10
	paddd xmm12, xmm14

	movdqa xmm4, xmm8
	movdqa xmm5, xmm12
	psrldq xmm4, 8
	psrldq xmm5, 8
	paddd xmm8, xmm4
	paddd xmm12, xmm5

	movdqa xmm4, xmm8
	movdqa xmm5, xmm12
	psrldq xmm4, 4
	psrldq xmm5, 4
	paddd xmm8, xmm4
	paddd xmm12, xmm5

	movd eax, xmm8
	movd edx, xmm12

	mov r9, arg(6) ; s10
	mov r8, arg(5) ; s01

	add [r9], dword eax
	add [r8], dword edx

	%undef vecZero

	%undef i		
	%undef top		
	%undef bottom  
	%undef x		
	%undef y
	%undef count

	; free memory and unalign stack
	add rsp, (1*COMPV_YASM_XMM_SZ_BYTES)
	COMPV_YASM_UNALIGN_STACK

	;; begin epilog ;;
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

%endif ; COMPV_YASM_ABI_IS_64BIT
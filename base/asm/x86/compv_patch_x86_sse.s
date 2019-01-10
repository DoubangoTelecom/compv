;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(CompVPatchRadiusLte64Moments0110_Asm_X86_SSE2)

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
sym(CompVPatchRadiusLte64Moments0110_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_XMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 16, rax
	sub rsp, (1*COMPV_YASM_XMM_SZ_BYTES) + (4*COMPV_YASM_XMM_SZ_BYTES) + (4*COMPV_YASM_XMM_SZ_BYTES)

	%define vecZero         rsp + 0
	%define vecS10          vecZero + (1*COMPV_YASM_XMM_SZ_BYTES)
	%define vecS01			vecS10 + (4*COMPV_YASM_XMM_SZ_BYTES)

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

	pxor xmm0, xmm0
	movdqa [vecZero], xmm0
	%assign index 0
	%rep 4
		movdqa [vecS10 + index*COMPV_YASM_XMM_SZ_BYTES], xmm0
		movdqa [vecS01 + index*COMPV_YASM_XMM_SZ_BYTES], xmm0
		%assign index index+1
	%endrep

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
		movdqa xmm6, xmm3
		movdqa xmm7, xmm0
		punpcklwd xmm1, xmm1
		punpckhwd xmm4, xmm4
		punpcklwd xmm5, xmm5
		punpckhwd xmm2, xmm2
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
		; Do not change: cache-friendly
		paddd xmm1, [vecS10 + 0*COMPV_YASM_XMM_SZ_BYTES]
		paddd xmm4, [vecS10 + 1*COMPV_YASM_XMM_SZ_BYTES]
		paddd xmm6, [vecS10 + 2*COMPV_YASM_XMM_SZ_BYTES]
		paddd xmm3, [vecS10 + 3*COMPV_YASM_XMM_SZ_BYTES]
		paddd xmm5, [vecS01 + 0*COMPV_YASM_XMM_SZ_BYTES]
		paddd xmm2, [vecS01 + 1*COMPV_YASM_XMM_SZ_BYTES]
		paddd xmm7, [vecS01 + 2*COMPV_YASM_XMM_SZ_BYTES]
		paddd xmm0, [vecS01 + 3*COMPV_YASM_XMM_SZ_BYTES]
		cmp i, count
		movdqa [vecS10 + 0*COMPV_YASM_XMM_SZ_BYTES], xmm1
		movdqa [vecS10 + 1*COMPV_YASM_XMM_SZ_BYTES], xmm4
		movdqa [vecS10 + 2*COMPV_YASM_XMM_SZ_BYTES], xmm6
		movdqa [vecS10 + 3*COMPV_YASM_XMM_SZ_BYTES], xmm3
		movdqa [vecS01 + 0*COMPV_YASM_XMM_SZ_BYTES], xmm5
		movdqa [vecS01 + 1*COMPV_YASM_XMM_SZ_BYTES], xmm2
		movdqa [vecS01 + 2*COMPV_YASM_XMM_SZ_BYTES], xmm7
		movdqa [vecS01 + 3*COMPV_YASM_XMM_SZ_BYTES], xmm0		
		jl .LoopCount
		;; EndOf_LoopCount ;;

	movdqa xmm0, [vecS10 + 0*COMPV_YASM_XMM_SZ_BYTES]
	movdqa xmm1, [vecS10 + 2*COMPV_YASM_XMM_SZ_BYTES]
	movdqa xmm2, [vecS01 + 0*COMPV_YASM_XMM_SZ_BYTES]
	movdqa xmm3, [vecS01 + 2*COMPV_YASM_XMM_SZ_BYTES]

	paddd xmm0, [vecS10 + 1*COMPV_YASM_XMM_SZ_BYTES]
	paddd xmm1, [vecS10 + 3*COMPV_YASM_XMM_SZ_BYTES]
	paddd xmm2, [vecS01 + 1*COMPV_YASM_XMM_SZ_BYTES]
	paddd xmm3, [vecS01 + 3*COMPV_YASM_XMM_SZ_BYTES]

	paddd xmm0, xmm1
	paddd xmm2, xmm3

	movdqa xmm4, xmm0
	movdqa xmm5, xmm2
	psrldq xmm4, 8
	psrldq xmm5, 8
	paddd xmm0, xmm4
	paddd xmm2, xmm5

	movdqa xmm4, xmm0
	movdqa xmm5, xmm2
	psrldq xmm4, 4
	psrldq xmm5, 4
	paddd xmm0, xmm4
	paddd xmm2, xmm5

	movd eax, xmm0
	movd edx, xmm2

	mov rdi, arg(6) ; s10
	mov rsi, arg(5) ; s01

	add [rdi], dword eax
	add [rsi], dword edx

	%undef vecZero
	%undef vecS10
	%undef vecS01

	%undef i		
	%undef top		
	%undef bottom  
	%undef x		
	%undef y
	%undef count

	; free memory and unalign stack
	add rsp, (1*COMPV_YASM_XMM_SZ_BYTES) + (4*COMPV_YASM_XMM_SZ_BYTES) + (4*COMPV_YASM_XMM_SZ_BYTES)
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


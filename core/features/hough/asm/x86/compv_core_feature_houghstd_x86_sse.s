;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(CompVHoughStdAccGatherRow_4mpd_Asm_X86_SSE41)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const int32_t* pCosRho
; arg(1) -> COMPV_ALIGNED(SSE) const int32_t* pRowTimesSinRho
; arg(2) -> compv_uscalar_t col
; arg(3) -> int32_t* pACC
; arg(4) -> compv_uscalar_t accStride
; arg(5) -> compv_uscalar_t maxTheta
sym(CompVHoughStdAccGatherRow_4mpd_Asm_X86_SSE41):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	COMPV_YASM_SAVE_XMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; alloc memory
	sub rsp, 1*COMPV_YASM_REG_SZ_BYTES

	%define maxThetaMinus15	rsp + 0

	%define theta			rcx
	%define pCosRho			rax
	%define pRowTimesSinRho	rdx
	%define vec4			xmm4
	%define vecTheta		xmm5		
	%define vecStride		xmm6
	%define vecColInt32		xmm7

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (theta = 0; theta < maxTheta - 15; theta += 16)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	xor theta, theta
	.LoopTheta16:
		movdqa xmm0, [pCosRho + (theta + 0)*COMPV_YASM_INT32_SZ_BYTES]
		movdqa xmm1, [pCosRho + (theta + 4)*COMPV_YASM_INT32_SZ_BYTES]
		movdqa xmm2, [pCosRho + (theta + 8)*COMPV_YASM_INT32_SZ_BYTES]
		movdqa xmm3, [pCosRho + (theta + 12)*COMPV_YASM_INT32_SZ_BYTES]
		pmulld xmm0, vecColInt32
		pmulld xmm1, vecColInt32
		pmulld xmm2, vecColInt32
		pmulld xmm3, vecColInt32
		paddd xmm0, [pRowTimesSinRho + (theta + 0)*COMPV_YASM_INT32_SZ_BYTES]
		paddd xmm1, [pRowTimesSinRho + (theta + 4)*COMPV_YASM_INT32_SZ_BYTES]
		paddd xmm2, [pRowTimesSinRho + (theta + 8)*COMPV_YASM_INT32_SZ_BYTES]
		paddd xmm3, [pRowTimesSinRho + (theta + 12)*COMPV_YASM_INT32_SZ_BYTES]
		psrad xmm0, 16
		psrad xmm1, 16
		psrad xmm2, 16
		psrad xmm3, 16
		pmulld xmm0, vecStride
		pmulld xmm1, vecStride
		pmulld xmm2, vecStride
		pmulld xmm3, vecStride

		;psubd xmm, xmm

		;; EndOf_LoopTheta16 ;;


	%undef maxThetaMinus15
	%undef pCosRho
	%undef pRowTimesSinRho
	%undef theta
	%undef vec4
	%undef vecTheta	
	%undef vecStride
	%undef vecColInt32

	; free memory
	add rsp, 1*COMPV_YASM_REG_SZ_BYTES

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
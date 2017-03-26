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
global sym(CompVHoughStdNmsGatherRow_4mpd_Asm_X86_SSE2)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const int32_t* pCosRho
; arg(1) -> COMPV_ALIGNED(SSE) const int32_t* pRowTimesSinRho
; arg(2) -> compv_uscalar_t col
; arg(3) -> int32_t* pACC
; arg(4) -> compv_uscalar_t accStride
; arg(5) -> compv_uscalar_t maxTheta
sym(CompVHoughStdAccGatherRow_4mpd_Asm_X86_SSE41): ; !!Not used!!
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


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const int32_t * pAcc
; arg(1) -> compv_uscalar_t nAccStride
; arg(2) -> uint8_t* pNms
; arg(3) -> compv_uscalar_t nThreshold
; arg(4) -> compv_uscalar_t colStart
; arg(5) -> compv_uscalar_t maxCols
sym(CompVHoughStdNmsGatherRow_4mpd_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	COMPV_YASM_SAVE_XMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	%define colStart		rcx
	%define pAcc			rbx
	%define strideInBytes	rdx
	%define vecThreshold	xmm7
	
	mov rax, arg(3) ; nThreshold
	movd xmm7, eax
	pshufd xmm7, xmm7, 0x0

	mov rax, arg(5) ; arg(5)
	and rax, -4 ; backward align
	mov arg(5), rax

	mov colStart, arg(4)
	mov pAcc, arg(0)
	mov strideInBytes, arg(1)
	shl strideInBytes, 2

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (; colStart < maxCols; colStart += 4)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopCols:
		movdqu xmm0, [pAcc + colStart*COMPV_YASM_INT32_SZ_BYTES]
		pcmpgtd xmm0, xmm7
		pmovmskb eax, xmm0
		test rax, rax
		jz .AllZeros
			lea rax, [pAcc + colStart*COMPV_YASM_INT32_SZ_BYTES] ; curr
			movdqu xmm0, [rax] ; xmm0 = vecAcc
			mov rsi, rax
			sub rsi, strideInBytes ; top
			lea rdi, [rax + strideInBytes] ; bottom
			movdqu xmm1, [rax - 1*COMPV_YASM_INT32_SZ_BYTES]
			movdqu xmm2, [rax + 1*COMPV_YASM_INT32_SZ_BYTES]
			movdqu xmm3, [rsi - 1*COMPV_YASM_INT32_SZ_BYTES]
			movdqu xmm4, [rsi + 0*COMPV_YASM_INT32_SZ_BYTES]
			movdqu xmm5, [rsi + 1*COMPV_YASM_INT32_SZ_BYTES]
			movdqu xmm6, [rdi - 1*COMPV_YASM_INT32_SZ_BYTES]
			pcmpgtd xmm1, xmm0
			pcmpgtd xmm2, xmm0
			pcmpgtd xmm3, xmm0
			pcmpgtd xmm4, xmm0
			pcmpgtd xmm5, xmm0
			pcmpgtd xmm6, xmm0
			por xmm1, xmm2
			movdqu xmm2, [rdi + 0*COMPV_YASM_INT32_SZ_BYTES]
			por xmm3, xmm4
			movdqu xmm4, [rdi + 1*COMPV_YASM_INT32_SZ_BYTES]
			por xmm5, xmm6
			por xmm1, xmm3
			pcmpgtd xmm2, xmm0
			pcmpgtd xmm4, xmm0
			por xmm1, xmm5
			por xmm2, xmm4
			pcmpgtd xmm0, xmm7
			por xmm1, xmm2
			mov rax, arg(2) ; pNms
			pand xmm0, xmm1
			packssdw xmm0, xmm0
			packsswb xmm0, xmm0
			movd [rax + colStart*COMPV_YASM_UINT8_SZ_BYTES], xmm0
			.AllZeros:

		add colStart, 4
		cmp colStart, arg(5)
		jl .LoopCols
		;; EndOf_LoopCols ;;

	%undef colStart
	%undef pAcc
	%undef strideInBytes
	%undef vecThreshold

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
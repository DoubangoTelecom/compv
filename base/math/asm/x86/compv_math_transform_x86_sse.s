;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(CompVMathTransformHomogeneousToCartesian2D_4_64f_Asm_X86_SSE2)

section .data
	extern sym(k1_64f)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const compv_float64_t* srcX
; arg(1) -> COMPV_ALIGNED(SSE) const compv_float64_t* srcY
; arg(2) -> COMPV_ALIGNED(SSE) const compv_float64_t* srcZ
; arg(3) -> COMPV_ALIGNED(SSE) compv_float64_t* dstX
; arg(4) -> COMPV_ALIGNED(SSE) compv_float64_t* dstY
; arg(5) -> compv_uscalar_t numPoints
sym(CompVMathTransformHomogeneousToCartesian2D_4_64f_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	push rsi
	push rbx
	;; end prolog ;;

	mov rdx, arg(2) ; srcZ
	movapd xmm0, [sym(k1_64f)]
	movapd xmm1, xmm0
	divpd xmm0, [rdx + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
	divpd xmm1, [rdx + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
	mov rax, arg(0) ; srcX
	mov rcx, arg(1) ; srcY
	mov rbx, arg(3) ; dstX
	mov rsi, arg(4) ; dstY

	movapd xmm2, [rax + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
	movapd xmm3, [rax + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
	mulpd xmm2, xmm0
	mulpd xmm3, xmm1
	mulpd xmm0, [rcx]
	mulpd xmm1, [rcx + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
	movapd [rbx], xmm2
	movapd [rbx + 2*COMPV_YASM_FLOAT64_SZ_BYTES], xmm3
	movapd [rsi], xmm0
	movapd [rsi + 2*COMPV_YASM_FLOAT64_SZ_BYTES], xmm1

	;; begin epilog ;;
	pop rbx
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(CompVMathTransformHomogeneousToCartesian2D_4_64f_Asm_X86_AVX)

section .data
	extern sym(k1_64f)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(AVX) const compv_float64_t* srcX
; arg(1) -> COMPV_ALIGNED(AVX) const compv_float64_t* srcY
; arg(2) -> COMPV_ALIGNED(AVX) const compv_float64_t* srcZ
; arg(3) -> COMPV_ALIGNED(AVX) compv_float64_t* dstX
; arg(4) -> COMPV_ALIGNED(AVX) compv_float64_t* dstY
; arg(5) -> compv_uscalar_t numPoints
sym(CompVMathTransformHomogeneousToCartesian2D_4_64f_Asm_X86_AVX):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	push rsi
	push rbx
	;; end prolog ;;

	mov rdx, arg(2) ; srcZ
	vmovapd ymm0, [sym(k1_64f)]
	vdivpd ymm0, ymm0, [rdx]
	mov rax, arg(0) ; srcX
	mov rcx, arg(1) ; srcY
	mov rbx, arg(3) ; dstX
	mov rsi, arg(4) ; dstY
	vmulpd ymm1, ymm0, [rax]
	vmulpd ymm2, ymm0, [rcx]
	vmovapd [rbx], ymm1
	vmovapd [rsi], ymm2

	;; begin epilog ;;
	pop rbx
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret
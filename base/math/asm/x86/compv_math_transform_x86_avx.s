;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "../../../asm/x86/compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(TransformHomogeneousToCartesian2D_4_64f_Asm_X86_AVX)
section .data
	extern sym(k1_f64)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const COMPV_ALIGNED(AVX) compv_float64_t* srcX
; arg(1) -> const COMPV_ALIGNED(AVX) compv_float64_t* srcY
; arg(2) -> const COMPV_ALIGNED(AVX) compv_float64_t* srcZ
; arg(3) -> COMPV_ALIGNED(AVX) compv_float64_t* dstX
; arg(4) -> COMPV_ALIGNED(AVX) compv_float64_t* dstY
; arg(5) -> compv_uscalar_t numPoints
; void TransformHomogeneousToCartesian2D_4_64f_Asm_X86_AVX(const COMPV_ALIGNED(AVX) compv_float64_t* srcX, const COMPV_ALIGNED(AVX) compv_float64_t* srcY, const COMPV_ALIGNED(AVX) compv_float64_t* srcZ, COMPV_ALIGNED(AVX) compv_float64_t* dstX, COMPV_ALIGNED(AVX) compv_float64_t* dstY, compv_uscalar_t numPoints)
sym(TransformHomogeneousToCartesian2D_4_64f_Asm_X86_AVX):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	push rsi
	push rbx
	;; end prolog ;;

	mov rdx, arg(2) ; srcZ
	vmovapd ymm0, [sym(k1_f64)]
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
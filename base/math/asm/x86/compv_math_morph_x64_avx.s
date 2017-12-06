;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(CompVMathMorphProcessErode_8u_Asm_X64_AVX2)

section .data

section .text


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const compv_uscalar_t* strelInputPtrsPtr
; arg(1) -> const compv_uscalar_t strelInputPtrsCount
; arg(2) -> uint8_t* outPtr
; arg(3) -> const compv_uscalar_t width
; arg(4) -> const compv_uscalar_t height
; arg(5) -> const compv_uscalar_t stride
sym(CompVMathMorphProcessErode_8u_Asm_X64_AVX2):
	ret


%endif ; COMPV_YASM_ABI_IS_64BIT

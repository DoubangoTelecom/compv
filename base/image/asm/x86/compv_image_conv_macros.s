;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Macro used to convert 16 RGB to 16 RGBA samples
; 16 RGB samples requires 48 Bytes(3 XMM registers), will be converted to 16 RGBA samples
; requiring 64 Bytes (4 XMM registers)
; The aplha channel will contain zeros instead of 0xff because this macro is used to fetch samples in place
; %1 -> in @rgb, e.g. rax
; %2 -> @rgba[0], e.g. xmm0
; %3 -> @rgba[1], e.g. xmm1
; %4 -> @rgba[2], e.g. xmm2
; %5 -> @rgba[3], e.g. xmm3
; %6 -> @RgbToRgba mask, e.g. xmm4 or [sym(kShuffleEpi8_RgbToRgba_i32)]
; example: COMPV_16xRGB_TO_16xRGBA_SSSE3 rax, xmm0, xmm1, xmm2, xmm3, xmm4
%macro COMPV_16xRGB_TO_16xRGBA_SSSE3  6
	movdqa %2, [%1 + 0]
	movdqu %3, [%1 + 12]
	movdqu %4, [%1 + 24]
	movdqu %5, [%1 + 36]
	pshufb %2, %6
	pshufb %3, %6
	pshufb %4, %6
	pshufb %5, %6	
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Macro used to convert 32 RGB to 32 RGBA samples
; 32 RGB samples requires 96 Bytes(3 YMM registers), will be converted to 32 RGBA samples
; requiring 128 Bytes (4 YMM registers)
; The aplha channel will contain zeros instead of 0xff because this macro is used to fetch samples in place
; %1 -> in @rgb, e.g. rax
; %2 -> @rgba[0], e.g. ymm0
; %3 -> @rgba[1], e.g. ymm1
; %4 -> @rgba[2], e.g. ymm2
; %5 -> @rgba[3], e.g. ymm3
; %6 -> ymmABCDDEFG, e.g. ymm4
; %7 -> ymmMaskRgbToRgba, e.g. ymm5 or [sym(kShuffleEpi8_RgbToRgba_i32)]
; example: COMPV_16xRGB_TO_16xRGBA_SSSE3 rax, ymm0, ymm1, ymm2, ymm3, ymm4, ymm5
%macro COMPV_32xRGB_TO_32xRGBA_AVX2  7
	vmovdqa %2, [%1 + 0]
	vmovdqu %3, [%1 + 24]
	vmovdqu %4, [%1 + 48]
	vmovdqu %5, [%1 + 72]
	vpermd %2, %6, %2
	vpermd %3, %6, %3
	vpermd %4, %6, %4
	vpermd %5, %6, %5
	vpshufb %2, %2, %7
	vpshufb %3, %3, %7
	vpshufb %4, %4, %7
	vpshufb %5, %5, %7
%endmacro
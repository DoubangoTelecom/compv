;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Macro used to convert 16 RGB to 16 RGBA samples
; 16 RGB samples requires 48 Bytes(3 XMM registers), will be converted to 16 RGBA samples
; requiring 64 Bytes (4 XMM registers)
; The aplha channel will contain garbage instead of 0xff because this macro is used to fetch samples in place
; %1 -> in @rgb, e.g. rax
; %2 -> @rgba[0], e.g. xmm0
; %3 -> @rgba[1], e.g. xmm1
; %4 -> @rgba[2], e.g. xmm2
; %5 -> @rgba[3], e.g. xmm3
; example: COMPV_16xRGB_TO_16xRGBA_SSSE3 rax, xmm0, xmm1, xmm2, xmm3
%macro COMPV_16xRGB_TO_16xRGBA_SSSE3  5
	movdqa %2, [%1 + 0]
	movdqa %3, [%1 + 16]
	movdqa %4, [%1 + 32]
	movdqa %5, [%1 + 32]
	pshufb %2, [sym(kShuffleEpi8_RgbToRgba_i32)]
	palignr %3, [%1 + 0], 12
	pshufb %3, [sym(kShuffleEpi8_RgbToRgba_i32)]
	palignr %4, [%1 + 16], 8
	pshufb %4, [sym(kShuffleEpi8_RgbToRgba_i32)]
	palignr %5, %5, 4
	pshufb %5, [sym(kShuffleEpi8_RgbToRgba_i32)]
%endmacro


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Macro used to convert 32 RGB to 32 RGBA samples
; 32 RGB samples requires 96 Bytes(3 YMM registers), will be converted to 32 RGBA samples
; requiring 128 Bytes (4 YMM registers)
; The aplha channel will contain garbage instead of 0xff because this macro is used to fetch samples in place
; %1 -> in @rgb, e.g. rax
; %2 -> @rgba[0], e.g. ymm0
; %3 -> @rgba[1], e.g. ymm1
; %4 -> @rgba[2], e.g. ymm2
; %5 -> @rgba[3], e.g. ymm3
; example: COMPV_16xRGB_TO_16xRGBA_SSSE3 rax, ymm0, ymm1, ymm2, ymm3
%macro COMPV_32xRGB_TO_32xRGBA_AVX2  5
	vmovdqa %4, [sym(kAVXPermutevar8x32_CDEFFGHX_i32)]
	vmovdqa %5, [%1 + 32]
	vperm2i128 %3, %5, %5, 0x11
	vpermd %5, %4, [%1 + 64]
	vmovdqa %4, [sym(kAVXPermutevar8x32_ABCDDEFG_i32)]
	vperm2i128 %3, %3, [%1 + 64], 0x20
	vpermd %4, %4, %3
	vpshufb %4, %4, [sym(kShuffleEpi8_RgbToRgba_i32)]
	vpshufb %5, %5, [sym(kShuffleEpi8_RgbToRgba_i32)]
	vpermq %2, [%1 + 0], 0xff
	vmovdqa %3, [sym(kAVXPermutevar8x32_XXABBCDE_i32)]
	vpermd %3, %3, [%1 + 32]
	vpblendd %3, %3, %2, 0x03
	vpshufb %3, %3, [sym(kShuffleEpi8_RgbToRgba_i32)]
	vmovdqa %2, [sym(kAVXPermutevar8x32_ABCDDEFG_i32)]
	vpermd %2, %2, [%1 + 0]
	vpshufb %2, %2, [sym(kShuffleEpi8_RgbToRgba_i32)]
%endmacro
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Macro used to convert 3x16RGB to 4x16RGBA samples
; The aplha channel will contain garbage instead of 0xff beacuse this macro is used to fetch the 3samples to 4samples
; %1 -> in @rgb, e.g. rax
; %2 -> @rgba[0], e.g. xmm0
; %3 -> @rgba[1], e.g. xmm1
; %4 -> @rgba[2], e.g. xmm2
; %5 -> @rgba[3], e.g. xmm3
; example: COMPV_3RGB_TO_4RGBA_SSSE3 rax, xmm0, xmm1, xmm2, xmm3
%macro COMPV_3RGB_TO_4RGBA_SSSE3  5
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
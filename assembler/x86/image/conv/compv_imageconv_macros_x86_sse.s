;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Macro used to convert 3x16RGB to 4x16RGBA samples
; This macro will override [xmm4 - xmm7] registers
; The aplha channel will contain garbage instead of 0xff beacuse this macro is used to fetch the 3samples to 4samples
; %1 -> in @rgb, e.g. rax
; %2 -> out @rgba, e.g. rsp
; %3 -> whether %1 is SSE-aligned. 0: not aligned, 1: aligned
; %4 -> whether %2 is SSE-aligned. 0: not aligned, 1: aligned
; example: COMPV_3RGB_TO_4RGBA_SSSE3 rax, rsp, 1, 1
%macro COMPV_3RGB_TO_4RGBA_SSSE3  4
	%if %3==1
	movdqa xmm7, [%1 + 0]
	movdqa xmm6, [%1 + 16]
	%else
	movdqu xmm7, [%1 + 0]
	movdqu xmm6, [%1 + 16]
	%endif
	movdqa xmm5, xmm7
	movdqa xmm4, xmm6

	pshufb xmm7, [sym(kShuffleEpi8_RgbToRgba_i32)]
	%if %4==1
	movdqa [%2 + 0], xmm7
	%else
	movdqu [%2 + 0], xmm7
	%endif

	palignr xmm4, xmm5, 12
	pshufb xmm4, [sym(kShuffleEpi8_RgbToRgba_i32)]
	%if %4==1
	movdqa [%2 + 16], xmm4
	%else
	movdqu [%2 + 16], xmm4
	%endif

	%if %3==1
	movdqa xmm7, [%1 + 32]
	%else
	movdqu xmm7, [%1 + 32]
	%endif
	movdqa xmm5, xmm7
	palignr xmm7, xmm6, 8
	pshufb xmm7, [sym(kShuffleEpi8_RgbToRgba_i32)]
	%if %4==1
	movdqa [%2 + 32], xmm7
	%else
	movdqu [%2 + 32], xmm7
	%endif

	palignr xmm5, xmm5, 4
	pshufb xmm5, [sym(kShuffleEpi8_RgbToRgba_i32)]
	%if %4==1
	movdqa [%2 + 48], xmm5
	%else
	movdqu [%2 + 48], xmm5
	%endif
%endmacro
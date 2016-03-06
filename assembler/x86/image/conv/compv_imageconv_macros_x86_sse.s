; Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
;
; This file is part of Open Source ComputerVision (a.k.a CompV) project.
; Source code hosted at https://github.com/DoubangoTelecom/compv
; Website hosted at http://compv.org
;
; CompV is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; CompV is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with CompV.
;


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
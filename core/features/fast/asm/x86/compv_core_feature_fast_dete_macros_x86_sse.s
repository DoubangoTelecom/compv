;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Macros for FastDataRow
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
%macro _mm_fast_check 2
	mov rax, [circle + (%1*8)]
	mov rsi, [circle + (%2*8)]
	movdqu xmm6, [rax + i] ; xmm4 = vec0
	movdqu xmm7, [rsi + i] ; xmm5 = vec1
	movdqa xmm4, vecDarker1
	movdqa xmm5, vecDarker1
	;  TODO(dmi): for AVX save data to vecCircle16 only after goto Next
	movdqa [vecCircle16 + (%1*16)], xmm6
	movdqa [vecCircle16 + (%2*16)], xmm7
	psubusb xmm4, xmm6
	psubusb xmm5, xmm7
	psubusb xmm6, vecBrighter1
	psubusb xmm7, vecBrighter1
	pcmpeqb xmm4, vecZero
	pcmpeqb xmm5, vecZero
	pcmpeqb xmm6, vecZero
	pcmpeqb xmm7, vecZero
	pandn xmm4, vec0xFF
	pandn xmm5, vec0xFF
	pandn xmm6, vec0xFF
	pandn xmm7, vec0xFF
	por xmm4, xmm5
	por xmm6, xmm7
	por xmm4, xmm6
	pmovmskb rax, xmm4
	test rax, rax
	jz .Next
%endmacro
%macro _mm_fast_compute_Darkers 4
	movdqa xmm4, vecDarker1
	movdqa xmm5, vecDarker1
	movdqa xmm6, vecDarker1
	movdqa xmm7, vecDarker1
	psubusb xmm4, [vecCircle16 + (%1*16)]
	psubusb xmm5, [vecCircle16 + (%2*16)]
	psubusb xmm6, [vecCircle16 + (%3*16)]
	psubusb xmm7, [vecCircle16 + (%4*16)]
	movdqa [vecDiff16 + (%1*16)], xmm4
	movdqa [vecDiff16 + (%2*16)], xmm5
	movdqa [vecDiff16 + (%3*16)], xmm6
	movdqa [vecDiff16 + (%4*16)], xmm7
%endmacro
%macro _mm_fast_compute_Brighters 4
	movdqa xmm4, [vecCircle16 + (%1*16)]
	movdqa xmm5, [vecCircle16 + (%2*16)]
	movdqa xmm6, [vecCircle16 + (%3*16)]
	movdqa xmm7, [vecCircle16 + (%4*16)]
	psubusb xmm4, vecBrighter1
	psubusb xmm5, vecBrighter1
	psubusb xmm6, vecBrighter1
	psubusb xmm7, vecBrighter1
	movdqa [vecDiff16 + (%1*16)], xmm4
	movdqa [vecDiff16 + (%2*16)], xmm5
	movdqa [vecDiff16 + (%3*16)], xmm6
	movdqa [vecDiff16 + (%4*16)], xmm7
%endmacro
%macro _mm_fast_load 4
	;_mm_fast_compute_ %+ %5 %1, %2, %3, %4
	pcmpeqb xmm4, vecZero
	pcmpeqb xmm5, vecZero
	pcmpeqb xmm6, vecZero
	pcmpeqb xmm7, vecZero
	pandn xmm4, vecOne
	pandn xmm5, vecOne
	pandn xmm6, vecOne
	pandn xmm7, vecOne
	movdqa [vecDiffBinary16 + (%1*16)], xmm4
	movdqa [vecDiffBinary16 + (%2*16)], xmm5
	movdqa [vecDiffBinary16 + (%3*16)], xmm6
	movdqa [vecDiffBinary16 + (%4*16)], xmm7
	paddb xmm4, xmm5
	paddb xmm6, xmm7
	paddb vecSum1, xmm4
	paddb vecSum1, xmm6
%endmacro
%macro _mm_fast_load_Darkers 4
	_mm_fast_compute_Darkers %1, %2, %3, %4
	_mm_fast_load %1, %2, %3, %4
%endmacro
%macro _mm_fast_load_Brighters 4
	_mm_fast_compute_Brighters %1, %2, %3, %4
	_mm_fast_load %1, %2, %3, %4
%endmacro
%macro _mm_fast_init_diffbinarysum 1
	movdqa vecSum1, [vecDiffBinary16 + 0*16]
	movdqa xmm4, [vecDiffBinary16 + 1*16]
	movdqa xmm5, [vecDiffBinary16 + 2*16]
	movdqa xmm6, [vecDiffBinary16 + 3*16]
	paddb vecSum1, [vecDiffBinary16 + 4*16]
	paddb xmm4, [vecDiffBinary16 + 5*16]
	paddb xmm5, [vecDiffBinary16 + 6*16]
	paddb xmm6, [vecDiffBinary16 + 7*16]
	paddb vecSum1, xmm4
	paddb xmm5, xmm6
	paddb vecSum1, xmm5
	%if %1 == 12
		movdqa xmm4, [vecDiffBinary16 + 8*16]
		movdqa xmm5, [vecDiffBinary16 + 9*16]
		movdqa xmm6, [vecDiffBinary16 + 10*16]
		paddb xmm4, xmm5
		paddb vecSum1, xmm6
		paddb vecSum1, xmm4
	%endif
%endmacro
%macro _mm_fast_strength 2
	paddb vecSum1, [vecDiffBinary16 + ((%2-1+%1)&15)*16]
	movdqa xmm4, vecSum1
	pcmpgtb xmm4, vecNMinusOne
	pmovmskb rax, xmm4
	test rax, rax
	jz %%LessThanNMinusOne
	movdqa xmm4, [vecDiff16 + ((%1+0)&15)*16]
	movdqa xmm5, [vecDiff16 + ((%1+1)&15)*16]
	movdqa xmm6, [vecDiff16 + ((%1+2)&15)*16]
	movdqa xmm7, [vecDiff16 + ((%1+3)&15)*16]
	pminub xmm4, [vecDiff16 + ((%1+4)&15)*16]
	pminub xmm5, [vecDiff16 + ((%1+5)&15)*16]
	pminub xmm6, [vecDiff16 + ((%1+6)&15)*16]
	pminub xmm7, [vecDiff16 + ((%1+7)&15)*16]
	pminub xmm4, xmm5
	pminub xmm6, xmm7
	pminub xmm4, xmm6
	pminub xmm4, [vecDiff16 + ((%1+8)&15)*16]
	%if %2 == 12
		movdqa xmm5, [vecDiff16 + ((%1+9)&15)*16]
		movdqa xmm6, [vecDiff16 + ((%1+10)&15)*16]
		movdqa xmm7, [vecDiff16 + ((%1+11)&15)*16]
		pminub xmm5, xmm6
		pminub xmm4, xmm7
		pminub xmm4, xmm5
	%endif
	pmaxub vecStrengths, xmm4
	%%LessThanNMinusOne:
	psubb vecSum1, [vecDiffBinary16 + (%1&15)*16]
%endmacro
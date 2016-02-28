; Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
; Copyright (C) 2016 Mamadou DIOP
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
; Compute horizontal minimum. 
; The input flags must be in rax.
; The result is copied in rcx. rcx must contain zero or the result from the previous call.
; This macro overrides rax, rsi, rdi and set the result in rcx.
; %1 -> Name of the components (e.g. Darkers or Brighters)
; %2 -> Whether CMOV is supported. 1: supported, 0: not supported
; %3 -> FAST type. Must be equal to 9 or 12
; %4 -> The XMM register containing the values for which to compute the hz min (e.g. xmm2)
; %5 -> 1st temp XMM register containing zeros (e.g. xmm0)
; %6 -> 2nd temp XMM register (e.g. xmm1)
; %7 -> 3rd temp XMM register (e.g. xmm3)
; %8 -> 4th temp XMM register (e.g. xmm4)
%macro COMPV_FEATURE_FAST_DETE_HORIZ_MIN_SSE41 8
	%ifndef _COMPV_FEATURE_FAST_DETE_HORIZ_MIN_SSE41_
		%define _COMPV_FEATURE_FAST_DETE_HORIZ_MIN_SSE41_
		%assign cffdhms 0 ; counter
	%else
		%assign cffdhms cffdhms+1
	%endif
	%assign m 0
	%rep    16
		test rax, 1<<m
		jz .EndOf %+ %1 %+ Min %+ cffdhms %+ m
		movdqa %6, %4
		%if %3 == 9
			movdqa %7, [sym(kFast9Arcs) + m*16]
		%elif %3 == 12
			movdqa %7, [sym(kFast12Arcs) + m*16]
		%else
			%error "not supported"
		%endif
		pshufb %6, %7
		movdqa %8, %6
		punpcklbw %6, %5
		punpckhbw %8, %5
		phminposuw %6, %6
		phminposuw %8, %8
		movd rdi, %6 ; bits [16:18] contains the index and must be ignored or cleared
		movd rsi, %8 ; bits [16:18] contains the index and must be ignored or cleared
		cmp si, di
		%if %2 == 1
			cmovl di, si
		%else
			jg . %+ %1 %+ NotMin %+ cffdhms %+ m
			mov di, si
			. %+ %1 %+ NotMin %+ cffdhms %+ m
		%endif
		cmp di, cx
		%if %2 == 1
			cmovg cx, di
		%else
			jl . %+ %1 %+ NotMax %+ cffdhms %+ m
			mov cx, di
			. %+ %1 %+ NotMax %+ cffdhms %+ m
		%endif
		.EndOf %+ %1 %+ Min %+ cffdhms %+ m
		%assign m m+1
	%endrep
%endm

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Compute horizontal minimum. 
; The input flags must be in rax.
; The result is copied in rcx. rcx must contain zero or the result from the previous call.
; This macro overrides rax, rsi, rdi and set the result in rcx or rbx.
; %1 -> Name of the components (e.g. Darkers or Brighters)
; %2 -> Whether CMOV is supported. 1: supported, 0: not supported
; %3 -> FAST type. Must be equal to 9 or 12
; %4 -> 0: put the result in rcx, 1: put the result in rbx
; %5 -> The XMM register containing the values for which to compute the hz min (e.g. xmm2)
; %6 -> 1st temp XMM register containing zeros (e.g. xmm0)
; %7 -> 2nd temp XMM register (e.g. xmm1)
; %8 -> 3rd temp XMM register (e.g. xmm3)
%macro COMPV_FEATURE_FAST_DETE_HORIZ_MIN_AVX2 8
	%ifndef _COMPV_FEATURE_FAST_DETE_HORIZ_MIN_AVX2_
		%define _COMPV_FEATURE_FAST_DETE_HORIZ_MIN_AVX2_
		%assign cffdhma 0 ; counter
	%else
		%assign cffdhma cffdhma+1
	%endif
	%assign m 0
	%rep    16
		test rax, 1<<m
		jz .EndOf %+ %1 %+ Min %+ cffdhma %+ m
		%if %3 == 9
			vpshufb %7, %5, [sym(kFast9Arcs) + m*16]
		%elif %3 == 12
			vpshufb %7, %5, [sym(kFast12Arcs) + m*16]
		%else
			%error "not supported"
		%endif
		vpunpckhbw %8, %7, %6
		vpunpcklbw %7, %7, %6
		vphminposuw %7, %7
		vphminposuw %8, %8
		vmovd edi, %7 ; bits [16:18] contains the index and must be ignored or cleared
		vmovd esi, %8 ; bits [16:18] contains the index and must be ignored or cleared
		cmp si, di
		%if %2 == 1
			cmovl di, si
		%else
			jg . %+ %1 %+ NotMin %+ cffdhma %+ m
			mov di, si
			. %+ %1 %+ NotMin %+ cffdhma %+ m
		%endif
		%if %4 == 0
			cmp di, cx
		%else
			cmp di, bx
		%endif
		%if %2 == 1
			%if %4 == 0
				cmovg cx, di
			%else
				cmovg bx, di
			%endif
		%else
			jl . %+ %1 %+ NotMax %+ cffdhma %+ m
			%if %4 == 0
				mov cx, di
			%else
				mov bx, di
			%endif
			. %+ %1 %+ NotMax %+ cffdhma %+ m
		%endif
		.EndOf %+ %1 %+ Min %+ cffdhma %+ m
		%assign m m+1
	%endrep
%endm
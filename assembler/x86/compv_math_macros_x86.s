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
; Transpose a 4x16 matrix containing u8/i8 values.
; From:
; 0 0 0 0 . .
; 1 1 1 1 . .
; 2 2 2 2 . .
; 3 3 3 3 . .
; To:
; 0 1 2 3 . .
; 0 1 2 3 . .
; 0 1 2 3 . .
; %1 -> first register
; %2 -> second register
; %3 -> third register
; %4 -> fourth register
; %5 -> temp register
; example: COMPV_TRANSPOSE_I8_4X16_XMM_SSE2 xmm0, xmm1, xmm2, xmm3, xmm4
%macro COMPV_TRANSPOSE_I8_4X16_XMM_SSE2 5
	COMPV_INTERLEAVE_I8_XMM_SSE2 %1, %3, %5
	COMPV_INTERLEAVE_I8_XMM_SSE2 %2, %4, %5
	COMPV_INTERLEAVE_I8_XMM_SSE2 %1, %2, %5
	COMPV_INTERLEAVE_I8_XMM_SSE2 %3, %4, %5
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Transpose a 4x16 matrix containing u8/i8 values.
; From:
; 0 0 0 0 . .
; 1 1 1 1 . .
; 2 2 2 2 . .
; 3 3 3 3 . .
; To:
; 0 1 2 3 . .
; 0 1 2 3 . .
; 0 1 2 3 . .
; %1 -> first address
; %2 -> second address
; %3 -> third address
; %4 -> fourth address
; %5 -> temp register
; %6 -> temp register
; example: COMPV_TRANSPOSE_I8_4X16_REG_SSE2 rsp+0*16, rsp+1*16, rsp+2*16, rsp+3*16, xmm0, xmm1
%macro COMPV_TRANSPOSE_I8_4X16_REG_SSE2 6
	COMPV_INTERLEAVE_I8_REG_SSE2 %1, %3, %5, %6
	COMPV_INTERLEAVE_I8_REG_SSE2 %2, %4, %5, %6
	COMPV_INTERLEAVE_I8_REG_SSE2 %1, %2, %5, %6
	COMPV_INTERLEAVE_I8_REG_SSE2 %3, %4, %5, %6
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Same as COMPV_TRANSPOSE_I8_4X16_REG_SSE2 but with 5
; temp registers
; This macro uses 4xCOMPV_INTERLEAVE_I8_REG_SSE2 but with
; 10 movdqa instructions instead of 16
%macro COMPV_TRANSPOSE_I8_4X16_REG_T5_SSE2 9
	movdqa %5, [%1]
	movdqa %6, [%1]
	punpcklbw %5, [%3] ; %1
	punpckhbw %6, [%3] ; %3
	
	movdqa %7, [%2]
	movdqa %8, [%2]
	punpcklbw %7, [%4] ; %2
	punpckhbw %8, [%4] ; %4

	movdqa %9, %5 ; %1
	punpcklbw %9, %7 ; %1
	punpckhbw %5, %7 ; %2
	movdqa [%1], %9
	movdqa [%2], %5

	movdqa %9, %6 ; %3
	punpcklbw %9, %8 ; %3
	punpckhbw %6, %8 ; %4
	movdqa [%3], %9
	movdqa [%4], %6
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Same as COMPV_TRANSPOSE_I8_4X16_REG_SSE2 but for AVX2
%macro COMPV_TRANSPOSE_I8_4X16_REG_T5_AVX2 9
	vmovdqa %5, [%1]
	vmovdqa %7, [%2]

	vpunpckhbw %6, %5, [%3]
	vpunpcklbw %5, %5, [%3]
	
	vpunpckhbw %8, %7, [%4]
	vpunpcklbw %7, %7, [%4]
	
	vpunpcklbw %9, %5, %7
	vpunpckhbw %5, %5, %7
	vmovdqa [%1], %9
	vmovdqa [%2], %5

	vpunpcklbw %9, %6, %8
	vpunpckhbw %6, %6, %8
	vmovdqa [%3], %9
	vmovdqa [%4], %6
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Transpose a 16x16 matrix containing u8/i8 values.
; From:
; 0 0 0 0 . .
; 1 1 1 1 . .
; 2 2 2 2 . .
; 3 3 3 3 . .
; To:
; 0 1 2 3 . .
; 0 1 2 3 . .
; 0 1 2 3 . .
; %1 -> 1st register
; ...
; %16 -> 16th register
; %17 -> temp register
%macro COMPV_TRANSPOSE_I8_16X16_XMM_SSE2 17
	; 1 * 5 * 9 * d 
	COMPV_TRANSPOSE_I8_4X16_XMM_SSE2 %2, %6, %10, %14, %17
	; 3 * 7 * b * f
	COMPV_TRANSPOSE_I8_4X16_XMM_SSE2 %4, %8, %12, %16, %17
	; 0 * 4 * 8 * c
	COMPV_TRANSPOSE_I8_4X16_XMM_SSE2 %1, %5, %9, %13, %17
	; 2 * 6 * a * e 
	COMPV_TRANSPOSE_I8_4X16_XMM_SSE2 %3, %7, %11, %15, %17
	; 1 * 3 * 5 * 7 * 9 * b * d * f 
	COMPV_INTERLEAVE_I8_XMM_SSE2 %2, %4, %17
	COMPV_INTERLEAVE_I8_XMM_SSE2 %6, %8, %17
	COMPV_INTERLEAVE_I8_XMM_SSE2 %10, %12, %17
	COMPV_INTERLEAVE_I8_XMM_SSE2 %14, %16, %17
	; 0 * 2 * 4 * 6 * 8 * a * c * e 
	COMPV_INTERLEAVE_I8_XMM_SSE2 %1, %3, %17
	COMPV_INTERLEAVE_I8_XMM_SSE2 %5, %7, %17
	COMPV_INTERLEAVE_I8_XMM_SSE2 %9, %11, %17
	COMPV_INTERLEAVE_I8_XMM_SSE2 %13, %15, %17
	; 0 * 1 * 2 * 3 * 4 * 5 * 6 * 7 * 8 * 9 * a * b * c * d * e * f
	COMPV_INTERLEAVE_I8_XMM_SSE2 %1, %2, %17
	COMPV_INTERLEAVE_I8_XMM_SSE2 %3, %4, %17
	COMPV_INTERLEAVE_I8_XMM_SSE2 %5, %6, %17
	COMPV_INTERLEAVE_I8_XMM_SSE2 %7, %8, %17
	COMPV_INTERLEAVE_I8_XMM_SSE2 %9, %10, %17
	COMPV_INTERLEAVE_I8_XMM_SSE2 %11, %12, %17
	COMPV_INTERLEAVE_I8_XMM_SSE2 %13, %14, %17
	COMPV_INTERLEAVE_I8_XMM_SSE2 %15, %16, %17
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Transpose a 16x16 matrix containing u8/i8 values.
; From:
; 0 0 0 0 . .
; 1 1 1 1 . .
; 2 2 2 2 . .
; 3 3 3 3 . .
; To:
; 0 1 2 3 . .
; 0 1 2 3 . .
; 0 1 2 3 . .
; %1 -> 1st address
; ...
; %16 -> 16th address
; %17 -> temp register
; %18 -> temp register
; example: COMPV_TRANSPOSE_I8_16X16_REG_SSE2 rsp+0*16, rsp+1*16, ....rsp+15*16, xmm0, xmm1
%macro COMPV_TRANSPOSE_I8_16X16_REG_SSE2 18
	; 1 * 5 * 9 * d 
	COMPV_TRANSPOSE_I8_4X16_REG_SSE2 %2, %6, %10, %14, %17, %18
	; 3 * 7 * b * f
	COMPV_TRANSPOSE_I8_4X16_REG_SSE2 %4, %8, %12, %16, %17, %18
	; 0 * 4 * 8 * c
	COMPV_TRANSPOSE_I8_4X16_REG_SSE2 %1, %5, %9, %13, %17, %18
	; 2 * 6 * a * e 
	COMPV_TRANSPOSE_I8_4X16_REG_SSE2 %3, %7, %11, %15, %17, %18
	; 1 * 3 * 5 * 7 * 9 * b * d * f 
	COMPV_INTERLEAVE_I8_REG_SSE2 %2, %4, %17, %18
	COMPV_INTERLEAVE_I8_REG_SSE2 %6, %8, %17, %18
	COMPV_INTERLEAVE_I8_REG_SSE2 %10, %12, %17, %18
	COMPV_INTERLEAVE_I8_REG_SSE2 %14, %16, %17, %18
	; 0 * 2 * 4 * 6 * 8 * a * c * e 
	COMPV_INTERLEAVE_I8_REG_SSE2 %1, %3, %17, %18
	COMPV_INTERLEAVE_I8_REG_SSE2 %5, %7, %17, %18
	COMPV_INTERLEAVE_I8_REG_SSE2 %9, %11, %17, %18
	COMPV_INTERLEAVE_I8_REG_SSE2 %13, %15, %17, %18
	; 0 * 1 * 2 * 3 * 4 * 5 * 6 * 7 * 8 * 9 * a * b * c * d * e * f
	COMPV_INTERLEAVE_I8_REG_SSE2 %1, %2, %17, %18
	COMPV_INTERLEAVE_I8_REG_SSE2 %3, %4, %17, %18
	COMPV_INTERLEAVE_I8_REG_SSE2 %5, %6, %17, %18
	COMPV_INTERLEAVE_I8_REG_SSE2 %7, %8, %17, %18
	COMPV_INTERLEAVE_I8_REG_SSE2 %9, %10, %17, %18
	COMPV_INTERLEAVE_I8_REG_SSE2 %11, %12, %17, %18
	COMPV_INTERLEAVE_I8_REG_SSE2 %13, %14, %17, %18
	COMPV_INTERLEAVE_I8_REG_SSE2 %15, %16, %17, %18
%endmacro


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Same as COMPV_TRANSPOSE_I8_16X16_REG_SSE2 but with 
; 5 temp registers
%macro COMPV_TRANSPOSE_I8_16X16_REG_T5_SSE2 21
	COMPV_TRANSPOSE_I8_4X16_REG_T5_SSE2 %2, %6, %10, %14, %17, %18, %19, %20, %21
	COMPV_TRANSPOSE_I8_4X16_REG_T5_SSE2 %4, %8, %12, %16, %17, %18, %19, %20, %21
	COMPV_TRANSPOSE_I8_4X16_REG_T5_SSE2 %1, %5, %9, %13, %17, %18, %19, %20, %21
	COMPV_TRANSPOSE_I8_4X16_REG_T5_SSE2 %3, %7, %11, %15, %17, %18, %19, %20, %21
	COMPV_TRANSPOSE_I8_4X16_REG_T5_SSE2 %1, %2, %3, %4, %17, %18, %19, %20, %21
	COMPV_TRANSPOSE_I8_4X16_REG_T5_SSE2 %5, %6, %7, %8, %17, %18, %19, %20, %21
	COMPV_TRANSPOSE_I8_4X16_REG_T5_SSE2 %9, %10, %11, %12, %17, %18, %19, %20, %21
	COMPV_TRANSPOSE_I8_4X16_REG_T5_SSE2 %13, %14, %15, %16, %17, %18, %19, %20, %21
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Same as COMPV_TRANSPOSE_I8_16X16_REG_SSE2 but for AVX2 
%macro COMPV_TRANSPOSE_I8_16X16_REG_T5_AVX2 21
	COMPV_TRANSPOSE_I8_4X16_REG_T5_AVX2 %2, %6, %10, %14, %17, %18, %19, %20, %21
	COMPV_TRANSPOSE_I8_4X16_REG_T5_AVX2 %4, %8, %12, %16, %17, %18, %19, %20, %21
	COMPV_TRANSPOSE_I8_4X16_REG_T5_AVX2 %1, %5, %9, %13, %17, %18, %19, %20, %21
	COMPV_TRANSPOSE_I8_4X16_REG_T5_AVX2 %3, %7, %11, %15, %17, %18, %19, %20, %21
	COMPV_TRANSPOSE_I8_4X16_REG_T5_AVX2 %1, %2, %3, %4, %17, %18, %19, %20, %21
	COMPV_TRANSPOSE_I8_4X16_REG_T5_AVX2 %5, %6, %7, %8, %17, %18, %19, %20, %21
	COMPV_TRANSPOSE_I8_4X16_REG_T5_AVX2 %9, %10, %11, %12, %17, %18, %19, %20, %21
	COMPV_TRANSPOSE_I8_4X16_REG_T5_AVX2 %13, %14, %15, %16, %17, %18, %19, %20, %21
%endmacro
 
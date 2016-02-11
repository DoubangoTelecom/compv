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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Same as COMPV_TRANSPOSE_I8_16X16_REG_SSE2 but uses 9
; temp registers.
; %1 to %16 -> memory addresses
; %17 to %25 -> temp registers
%macro COMPV_TRANSPOSE_I8_16X16_REG_T9_X64_SSE2 25
	; LOAD ;
	movdqa %17, [%2]
	movdqa %18, [%6]
	movdqa %19, [%10]
	movdqa %20, [%14]
	movdqa %21, [%4]
	movdqa %22, [%8]
	movdqa %23, [%12]
	movdqa %24, [%16]
	; 1 * 5 * 9 * d 
	COMPV_TRANSPOSE_I8_4X16_XMM_SSE2 %17, %18, %19, %20, %25
	; 3 * 7 * b * f
	COMPV_TRANSPOSE_I8_4X16_XMM_SSE2 %21, %22, %23, %24, %25
	; 1 * 3 * 5 * 7 * 9 * b * d * f 
	COMPV_INTERLEAVE_I8_XMM_SSE2 %17, %21, %25
	COMPV_INTERLEAVE_I8_XMM_SSE2 %18, %22, %25
	COMPV_INTERLEAVE_I8_XMM_SSE2 %19, %23, %25
	COMPV_INTERLEAVE_I8_XMM_SSE2 %20, %24, %25
	; SAVE ;
	movdqa [%2], %17
	movdqa [%6], %18
	movdqa [%10], %19
	movdqa [%14], %20
	movdqa [%4], %21
	movdqa [%8], %22
	movdqa [%12], %23
	movdqa [%16], %24
	; LOAD ;
	movdqa %17, [%1]
	movdqa %18, [%5]
	movdqa %19, [%9]
	movdqa %20, [%13]
	movdqa %21, [%3]
	movdqa %22, [%7]
	movdqa %23, [%11]
	movdqa %24, [%15]
	; 0 * 4 * 8 * c	
	COMPV_TRANSPOSE_I8_4X16_XMM_SSE2 %17, %18, %19, %20, %25
	; 2 * 6 * a * e
	COMPV_TRANSPOSE_I8_4X16_XMM_SSE2 %21, %22, %23, %24, %25
	; 0 * 2 * 4 * 6 * 8 * a * c * e 
	COMPV_INTERLEAVE_I8_XMM_SSE2 %17, %21, %25
	COMPV_INTERLEAVE_I8_XMM_SSE2 %18, %22, %25
	COMPV_INTERLEAVE_I8_XMM_SSE2 %19, %23, %25
	COMPV_INTERLEAVE_I8_XMM_SSE2 %20, %24, %25
	; SAVE ;
	movdqa [%1], %17
	movdqa [%5], %18
	movdqa [%9], %19
	movdqa [%13], %20
	movdqa [%3], %21
	movdqa [%7], %22 
	movdqa [%11], %23
	movdqa [%15], %24
	; 0 * 1 * 2 * 3 * 4 * 5 * 6 * 7 * 8 * 9 * a * b * c * d * e * f
	; LOAD ;
	movdqa %17, [%1]
	movdqa %18, [%2]
	movdqa %19, [%3]
	movdqa %20, [%4]
	movdqa %21, [%5]
	movdqa %22, [%6]
	movdqa %23, [%7]
	movdqa %24, [%8]
	COMPV_INTERLEAVE_I8_XMM_SSE2 %17, %18, %25
	COMPV_INTERLEAVE_I8_XMM_SSE2 %19, %20, %25
	COMPV_INTERLEAVE_I8_XMM_SSE2 %21, %22, %25
	COMPV_INTERLEAVE_I8_XMM_SSE2 %23, %24, %25
	; SAVE ;
	movdqa [%1], %17 
	movdqa [%2], %18 
	movdqa [%3], %19
	movdqa [%4], %20
	movdqa [%5], %21
	movdqa [%6], %22
	movdqa [%7], %23
	movdqa [%8], %24
	; LOAD ;
	movdqa %17, [%9]
	movdqa %18, [%10]
	movdqa %19, [%11]
	movdqa %20, [%12]
	movdqa %21, [%13]
	movdqa %22, [%14]
	movdqa %23, [%15]
	movdqa %24, [%16]
	COMPV_INTERLEAVE_I8_XMM_SSE2 %17, %18, %25
	COMPV_INTERLEAVE_I8_XMM_SSE2 %19, %20, %25
	COMPV_INTERLEAVE_I8_XMM_SSE2 %21, %22, %25
	COMPV_INTERLEAVE_I8_XMM_SSE2 %23, %24, %25
	; SAVE ;
	movdqa [%9], %17 
	movdqa [%10], %18 
	movdqa [%11], %19
	movdqa [%12], %20
	movdqa [%13], %21
	movdqa [%14], %22
	movdqa [%15], %23
	movdqa [%16], %24
%endmacro
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Compute population count (popcnt instruction). Hardward implementation. 
; You must use CPUID instruction to check that your cpu supports this instruction.
; %1 -> the destination register where to copy the result
; %2 -> the register containing the value for which to compute the popcnt
; The first (%1) and second (%2) parameters could be the same register
; example: COMPV_POPCNT_HARD rax, rax
%macro COMPV_POPCNT_HARD 2
popcnt %1, %2
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Compute population count (popcnt instruction). Hardward implementation. 
; You must provide a 16 byte register
; example: COMPV_POPCNT16_HARD ax, ax
%define COMPV_POPCNT16_HARD COMPV_POPCNT_HARD

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Compute population count (popcnt instruction).  Software implementation. 
; %1 -> the destination register where to copy the result
; %2 -> the register containing the value for which to compute the popcnt
; %3 -> temp register
; %4 -> temp register
; The first (%1) and second (%2) parameters could be the same register
; example: COMPV_POPCNT8_SOFT rbx, rbx, rax, rcx
%macro COMPV_POPCNT16_SOFT 4
	mov %3, %2
	and %3, 0xFF
	lea %4, [sym(kPopcnt256) + %3]
	mov %3, %2
	mov %1, [%4] ; %1 and %2 could be represented by the same register -> update %1 after saving %2 in %3
	shr %3, 8
	and %3, 0xFF
	lea %4, [sym(kPopcnt256) + %3]
	add %1, [%4]
%endmacro


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Interleaves two 128bits vectors.
; From:
; 0 0 0 0 0 0 . . . .
; 1 1 1 1 1 1 . . . .
; To:
; 0 1 0 1 0 1 . . . .
; %1 -> first register
; %2 -> second register
; %3 -> tmp xmm register
; example: COMPV_INTERLEAVE_I8_XMM_SSE2 xmm0, xmm1, xmm2
%macro COMPV_INTERLEAVE_I8_XMM_SSE2 3
	movdqa %3, %1
	punpcklbw %1, %2
	punpckhbw %3, %2
	movdqa %2, %3
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Interleaves two 128bits vectors.
; This macro use 4 movdqa while COMPV_INTERLEAVE_I8_XMM_SSE2 uses only 2 movs.
; From:
; 0 0 0 0 0 0 . . . .
; 1 1 1 1 1 1 . . . .
; To:
; 0 1 0 1 0 1 . . . .
; %1 -> first address
; %2 -> second address
; %3 -> tmp xmm register
; %4 -> tmp xmm register
; example: COMPV_INTERLEAVE_I8_REG_SSE2 rsp+0*16, rsp+1*16, xmm0, xmm1
%macro COMPV_INTERLEAVE_I8_REG_SSE2 4
	movdqa %3, [%1]
	movdqa %4, [%1]
	punpcklbw %3, [%2]
	punpckhbw %4, [%2]
	movdqa [%1], %3
	movdqa [%2], %4
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; _mm256_packs_epi16 with 128-lane crossing.
; %1 Destination register (e.g. YMM0). Could be equal to 1st or 2st YMM register
; %2 1st YMM register (e.g. YMM1)
; %3 2nd YMM register (e.g. YMM2)
; example COMPV_PACKS_EPI16_AVX2 ymm0, ymm0, ymm1
%macro COMPV_PACKS_EPI16_AVX2 3
	vpacksswb %1, %2, %3
	vpermq %1, %1, 0xD8 ; 0xD8 = COMPV_MM_SHUFFLE(3, 1, 2, 0)
%endm
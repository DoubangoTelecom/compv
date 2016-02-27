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
%include "../../compv_common_x86.S"
%include "../../compv_bits_macros_x86.S"
%include "../../compv_math_macros_x86.S"

COMPV_YASM_DEFAULT_REL

global sym(Fast9Strengths_Asm_CMOV_X86_SSE41)
global sym(Fast9Strengths_Asm_X86_SSE41)
global sym(Fast12Strengths_Asm_CMOV_X86_SSE41)
global sym(Fast12Strengths_Asm_X86_SSE41)

global sym(Fast9Data16_Asm_POPCNT_X86_SSE2)
global sym(Fast9Data16_Asm_X86_SSE2)
global sym(Fast12Data16_Asm_POPCNT_X86_SSE2)
global sym(Fast12Data16_Asm_X86_SSE2)

section .data
	extern sym(kFast9Arcs)
	extern sym(kFast12Arcs)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t FastStrengths_Asm_X86_SSE41(COMPV_ALIGNED(SSE) const int16_t(&dbrighters)[16], COMPV_ALIGNED(SSE) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
; arg(0) -> COMPV_ALIGNED(SSE) const int16_t(&dbrighters)[16]
; arg(1) -> COMPV_ALIGNED(SSE) const int16_t(&ddarkers)[16]
; arg(2) -> compv_scalar_t fbrighters
; arg(3) -> compv_scalar_t fdarkers
; arg(4) -> compv_scalar_t N
; arg(5) -> COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16]
; %1 -> 1: CMOV is supported, 0 CMOV not supported
; %2 -> 9: Use FAST9, 12: FAST12 ....
%macro FastStrengths_Asm_X86_SSE41 2
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	COMPV_YASM_SAVE_XMM 7 ;XMM[6-n]
	push rsi
	push rdi
	push rbx
	; end prolog

	xor rcx, rcx ; rcx = maxnbrighter
	xor rdx, rdx ; rdx = maxndarker

	; FAST hard-coded flags
	mov rax, arg(5)
	movdqa xmm7, [rax] ; xmm7 = xmmFastXFlagsLow
	movdqa xmm6, [rax + 8*2]; xmm6 = xmmFastXFlagsHigh

	; ---------
	; Brighters
	; ---------
	mov rax, arg(2)
	movd xmm5, rax
	punpcklwd xmm5, xmm5  
	pshufd xmm5, xmm5, 0 ; xmm5 = _mm_set1_epi16(fbrighters)
	movdqa xmm4, xmm5
	pand xmm5, xmm7
	pand xmm4, xmm6
	pcmpeqw xmm5, xmm7
	pcmpeqw xmm4, xmm6
	; clear the high bit in the epi16, otherwise will be considered as the sign bit when saturated to u8
	psrlw xmm5, 1
	psrlw xmm4, 1
	packuswb xmm5, xmm4
	pmovmskb rax, xmm5
	test rax, rax ; rax = r0
	je .Darkers
	pxor xmm3, xmm3 ; xmm3 = Zeros
	; Load dbrighters and convert it from i16 to u8 and saturate
	mov rbx, arg(0)
	movdqa xmm2, [rbx]
	movdqa xmm1, [rbx + 8*2]
	packuswb xmm2, xmm1
	; Compute minimum hz
	%assign i 0
	%rep    16
		test rax, 1<<i
		je .EndOfBrightersMin %+ i
		movdqa xmm1, xmm2
		%if %2 == 9
			movdqa xmm0, [sym(kFast9Arcs) + i*16]
		%elif %2 == 12
			movdqa xmm0, [sym(kFast12Arcs) + i*16]
		%else
			%error "not supported"
		%endif
		pshufb xmm1, xmm0
		movdqa xmm4, xmm1
		punpcklbw xmm1, xmm3
		punpckhbw xmm4, xmm3
		phminposuw xmm1, xmm1
		phminposuw xmm4, xmm4
		movd rdi, xmm1
		movd rsi, xmm4
		and rdi, 0xFFFF
		and rsi, 0xFFFF
		cmp rsi, rdi
		%if %1 == 1
			cmovl rdi, rsi
		%else
			jg .BrightersNotMin %+ i
			mov rdi, rsi
			.BrightersNotMin %+ i
		%endif
		cmp rdi, rcx
		%if %1 == 1
			cmovg rcx, rdi
		%else
			jl .BrightersNotMax %+ i
			mov rcx, rdi
			.BrightersNotMax %+ i
		%endif
		.EndOfBrightersMin %+ i
		%assign i i+1
	%endrep


	; ---------
	; Darkers
	; ---------
.Darkers
	mov rax, arg(3)
	movd xmm5, rax
	punpcklwd xmm5, xmm5  
	pshufd xmm5, xmm5, 0 ; xmm5 = _mm_set1_epi16(fdarkers)
	movdqa xmm4, xmm5
	pand xmm5, xmm7
	pand xmm4, xmm6
	pcmpeqw xmm5, xmm7
	pcmpeqw xmm4, xmm6
	; clear the high bit in the epi16, otherwise will be considered as the sign bit when saturated to u8
	psrlw xmm5, 1
	psrlw xmm4, 1
	packuswb xmm5, xmm4
	pmovmskb rax, xmm5
	test rax, rax ; rax = r0
	je .EndOfProcess
	pxor xmm3, xmm3 ; xmm3 = Zeros
	; Load dbrighters and convert it from i16 to u8 and saturate
	mov rbx, arg(1)
	movdqa xmm2, [rbx]
	movdqa xmm1, [rbx + 8*2]
	packuswb xmm2, xmm1
	; Compute minimum hz
	%assign i 0
	%rep    16
	test rax, 1<<i
	je .EndOfDarkersMin %+ i
	movdqa xmm1, xmm2
	%if %2 == 9
	movdqa xmm0, [sym(kFast9Arcs) + i*16]
	%elif %2 == 12
	movdqa xmm0, [sym(kFast12Arcs) + i*16]
	%else
	%error "not supported"
	%endif
	pshufb xmm1, xmm0
	movdqa xmm4, xmm1
	punpcklbw xmm1, xmm3
	punpckhbw xmm4, xmm3
	phminposuw xmm1, xmm1
	phminposuw xmm4, xmm4
	movd rdi, xmm1
	movd rsi, xmm4
	and rdi, 0xFFFF
	and rsi, 0xFFFF
	cmp rsi, rdi
	%if %1 == 1
			cmovl rdi, rsi
		%else
			jg .DarkersNotMin %+ i
			mov rdi, rsi
			.DarkersNotMin %+ i
		%endif
		cmp rdi, rcx
		%if %1 == 1
			cmovg rcx, rdi
		%else
			jl .DarkersNotMax %+ i
			mov rcx, rdi
			.DarkersNotMax %+ i
		%endif
	.EndOfDarkersMin %+ i
	%assign i i+1
	%endrep

	.EndOfProcess
	; return std::max(maxnbrighter, maxndarker);
	mov rax, rdx
	cmp rcx, rdx
	%if %1 == 1
		cmovg rax, rcx
	%else
		jl .NotMax
		mov rax, rcx
		.NotMax
	%endif

	; begin epilog
	pop rbx
	pop rdi
	pop rsi
    COMPV_YASM_RESTORE_XMM
    COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t Fast9Strengths_Asm_CMOV_X86_SSE41(COMPV_ALIGNED(SSE) const int16_t(&dbrighters)[16], COMPV_ALIGNED(SSE) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
sym(Fast9Strengths_Asm_CMOV_X86_SSE41):
	FastStrengths_Asm_X86_SSE41 1, 9

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t Fast9Strengths_Asm_X86_SSE41(COMPV_ALIGNED(SSE) const int16_t(&dbrighters)[16], COMPV_ALIGNED(SSE) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
sym(Fast9Strengths_Asm_X86_SSE41):
	FastStrengths_Asm_X86_SSE41 0, 9

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t Fast12Strengths_Asm_CMOV_X86_SSE41(COMPV_ALIGNED(SSE) const int16_t(&dbrighters)[16], COMPV_ALIGNED(SSE) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
sym(Fast12Strengths_Asm_CMOV_X86_SSE41):
	FastStrengths_Asm_X86_SSE41 1, 12

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t Fast12Strengths_Asm_X86_SSE41(COMPV_ALIGNED(SSE) const int16_t(&dbrighters)[16], COMPV_ALIGNED(SSE) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
sym(Fast12Strengths_Asm_X86_SSE41):
	FastStrengths_Asm_X86_SSE41 0, 12


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* dataPtr 
; arg(1) -> COMPV_ALIGNED(SSE) const compv_scalar_t(&pixels16)[16]
; arg(2) -> compv_scalar_t N
; arg(3) -> compv_scalar_t threshold
; arg(4) -> COMPV_ALIGNED(SSE) compv_scalar_t(&pfdarkers16)[16]
; arg(5) -> COMPV_ALIGNED(SSE) compv_scalar_t(&pfbrighters16)[16]
; arg(6) -> COMPV_ALIGNED(SSE) int16_t(&ddarkers16x16)[16][16]
; arg(7) -> COMPV_ALIGNED(SSE) int16_t(&dbrighters16x16)[16][16]
; %1 -> 1: POPCNT supported, 0: POPCNT not supported
; %2 -> 9: FAST9, 12: FAST12 ...
;;; compv_scalar_t FastData16_Asm_X86_SSE2(const uint8_t* dataPtr, COMPV_ALIGNED(SSE) const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, COMPV_ALIGNED(SSE) compv_scalar_t(&pfdarkers16)[16], COMPV_ALIGNED(SSE) compv_scalar_t(&pfbrighters16)[16], COMPV_ALIGNED(SSE) int16_t(&ddarkers16x16)[16][16], COMPV_ALIGNED(SSE) int16_t(&dbrighters16x16)[16][16])
%macro FastData16_Asm_X86_SSE2 2
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_XMM 7 ;XMM[6-n]
	push rsi
	push rdi
	push rbx
	; end prolog

	; align and alloc memory
	COMPV_YASM_ALIGN_STACK 16, rax
	sub rsp, 8*3+16*16 ; sum = [rsp + 0], sumd = [rsp + 8], sumb = [rsp + 16], xmm0/15= [rsp + 24 + (i*16)]

	mov rbx, arg(0) ; rbx = dataPtr
	mov rdx, arg(1) ; rdx = pixels16
	pxor xmm0, xmm0 ; xmm0 = zeros
	
	mov rax, arg(3) ; rax = threshold
	movdqu xmm6, [rbx] ; xmm6 = xmmDataPtr[0]
	movd xmm7, rax
	movdqa xmm5, xmm6
	punpcklbw xmm7, xmm7  
	punpcklwd xmm7, xmm7  
	pshufd xmm7, xmm7, 0  ; xmm7 = _mm_set1_epi8((uint8_t)threshold)) = xmmThreshold
	paddusb xmm6, xmm7 ; xmm6 = xmmBrighter
	psubusb xmm5, xmm7 ; xmm5 = xmmDarker

	; compare I1 and I7 aka 0 and 8
	mov rsi, [rdx + 0*COMPV_YASM_REG_SZ_BYTES]
	mov rdi, [rdx + 8*COMPV_YASM_REG_SZ_BYTES]
	lea rax, [rbx + rsi]
	lea rcx, [rbx + rdi]
	movdqa xmm1, xmm5 ; xmmDarker
	movdqa xmm2, xmm5 ; xmmDarker
	movdqu xmm4, [rax] ; dataPtr[pixels16[0]]
	movdqu xmm3, [rcx] ; dataPtr[pixels16[8]]
	mov rax, arg(6) ; xmmDdarkers16x16
	mov rcx, arg(7) ; xmmDbrighters16x16
	psubusb xmm1, xmm4
	psubusb xmm2, xmm3
	movdqa [rax + 0*32+0*16], xmm1 ; xmmDdarkers16x16[0][0]
	movdqa [rax + 8*32+0*16], xmm2 ; xmmDdarkers16x16[8][0]
	psubusb xmm4, xmm6
	psubusb xmm3, xmm6
	movdqa [rcx + 0*32+0*16], xmm4 ; xmmDbrighters16x16[0][0]
	movdqa [rcx + 8*32+0*16], xmm3 ; xmmDbrighters16x16[8][0]
	pcmpeqb xmm1, xmm0 ; xmmDdarkers16x16[0][0] === xmmZeros
	pcmpeqb xmm2, xmm0 ; xmmDdarkers16x16[8][0] === xmmZeros
	pcmpeqb xmm4, xmm0 ; xmmDbrighters16x16[0][0] === xmmZeros
	pcmpeqb xmm3, xmm0 ; xmmDbrighters16x16[8][0] === xmmZeros

	; Speed-Test-1
	pmovmskb rsi, xmm1
	not rsi ; d0
	and rsi, 0xFFFF
	pmovmskb rdi, xmm4
	not rdi ; b0
	and rdi, 0xFFFF
	mov rax, rdi
	or rax, rsi ; b0 |= d0 -> I1 is too brighter or too darker
	popcnt rax, rax
	mov [rsp + 0], rax ; sum = popcnt(b0 |= d0)
	popcnt rax, rsi
	popcnt rcx, rdi
	mov [rsp + 8], rax ; sumd = popcnt(d0)
	mov [rsp + 16], rcx ; sumb = popcnt(b0)
	pmovmskb rax, xmm3
	not rax ; b1
	and rax, 0xFFFF
	pmovmskb rcx, xmm2
	not rcx ; d1
	and rcx, 0xFFFF
	mov rsi, rax
	or rsi, rcx ; b1 |= d1 -> I7 is too brighter or too darker
	popcnt rsi, rsi ; rsi = popcnt(b1 |= d1)
	add [rsp + 0], rsi ; [rsp + 0] = sum = popcnt(b0 |= d0) + popcnt(b1 |= d1)
	popcnt rsi, rax ; rsi = popcnt(b1)
	add [rsp + 16], rsi ; [rsp + 16] = sumb = popcnt(b0) + popcnt(b1)
	popcnt rsi, rcx ; rsi = popcnt(d1)
	add [rsp + 8], rsi ; [rsp + 8] = sumd = popcnt(d0) + popcnt(d1)

	mov rax, [rsp + 0] ; rax = sum
	%if %2 == 12
		test rax, rax
		je .EndOfTheFunction ; rax == 0
	%endif
	
	; compare I5 and I13 aka 4 and 12
	mov rsi, [rdx + 4*COMPV_YASM_REG_SZ_BYTES]
	mov rdi, [rdx + 12*COMPV_YASM_REG_SZ_BYTES]
	lea rax, [rbx + rsi]
	lea rcx, [rbx + rdi]
	movdqa xmm1, xmm5 ; xmmDarker
	movdqa xmm2, xmm5 ; xmmDarker
	movdqu xmm4, [rax] ; dataPtr[pixels16[4]]
	movdqu xmm3, [rcx] ; dataPtr[pixels16[12]]
	mov rax, arg(6) ; xmmDdarkers16x16
	mov rcx, arg(7) ; xmmDbrighters16x16
	psubusb xmm1, xmm4
	psubusb xmm2, xmm3
	movdqa [rax + 4*32+0*16], xmm1 ; xmmDdarkers16x16[4][0]
	movdqa [rax + 12*32+0*16], xmm2 ; xmmDdarkers16x16[12][0]
	psubusb xmm4, xmm6
	psubusb xmm3, xmm6
	movdqa [rcx + 4*32+0*16], xmm4 ; xmmDbrighters16x16[4][0]
	movdqa [rcx + 12*32+0*16], xmm3 ; xmmDbrighters16x16[12][0]
	pcmpeqb xmm1, xmm0 ; xmmDdarkers16x16[4][0] === xmmZeros
	pcmpeqb xmm2, xmm0 ; xmmDdarkers16x16[12][0] === xmmZeros
	pcmpeqb xmm4, xmm0 ; xmmDbrighters16x16[4][0] === xmmZeros
	pcmpeqb xmm3, xmm0 ; xmmDbrighters16x16[12][0] === xmmZeros

	; re-check sum only if we've chance not to reach 3 (for N=12) or 2 (for N=9)
	;mov rax, [rsp + 0]; rax = sum
	;%if %2 == 12
	;	cmp rax, 3
	;	jge .EndOfSpeedTest2
	;%elif %2 == 9
	;	cmp rax, 2
	;	jge .EndOfSpeedTest2
	;%else
	;	%error "not valid"
	;%endif
	
	pmovmskb rsi, xmm1
	not rsi ; d0
	and rsi, 0xFFFF
	pmovmskb rdi, xmm4
	not rdi ; b0
	and rdi, 0xFFFF
	mov rax, rdi
	or rax, rsi ; b0 |= d0 -> I5 is too brighter or too darker
	popcnt rax, rax
	add [rsp + 0], rax ; sum += popcnt(b0 |= d0)
	popcnt rax, rsi
	popcnt rcx, rdi
	add [rsp + 8], rax ; sumd += popcnt(d0)
	add [rsp + 16], rcx ; sumb += popcnt(b0)
	pmovmskb rax, xmm3
	not rax ; b1
	and rax, 0xFFFF
	pmovmskb rcx, xmm2
	not rcx ; d1
	and rcx, 0xFFFF
	mov rsi, rax
	or rsi, rcx ; b1 |= d1 -> I13 is too brighter or too darker
	popcnt rsi, rsi ; rsi = popcnt(b1 |= d1)
	add [rsp + 0], rsi ; [rsp + 0] = sum = popcnt(b0 |= d0) + popcnt(b1 |= d1)
	popcnt rsi, rax ; rsi = popcnt(b1)
	add [rsp + 16], rsi ; [rsp + 16] = sumb = popcnt(b0) + popcnt(b1)
	popcnt rsi, rcx ; rsi = popcnt(d1)
	add [rsp + 8], rsi ; [rsp + 8] = sumd = popcnt(d0) + popcnt(d1)

	;;; Speed-Test-2 ;;;
	mov rcx, [rsp + 0] ; rcx = sum
	xor rax, rax ; set it to zero in case we've to jump to .EndOfTheFunction and return it
	%if %2 == 12
		cmp rcx, 3
		jl .EndOfTheFunction
	%elif %2 == 9
		cmp rcx, 2
		jl .EndOfTheFunction
	%else
		%error "not valid"
	%endif
	.EndOfSpeedTest2

	; loadB
	xor rcx, rcx
	mov rax, [rsp + 16] ; sumb
	mov [rsp + 16], rcx ; [rsp + 16] now holds loadB and its default value is 0
	%if %2 == 12
		cmp rax, 3
		jl .SumbTooShort
		mov rax, 1
		mov [rsp + 16], rax ; loadB = 1
	%elif %2 == 9
		cmp rax, 2
		jl .SumbTooShort
		mov rax, 1
		mov [rsp + 16], rax ; loadB = 1
	%else
		%error "not valid"
	%endif
	.SumbTooShort

	; loadD
	mov rax, [rsp + 8] ; sumd
	mov [rsp + 8], rcx ; [rsp + 8] now holds loadD and its default value is 0
	%if %2 == 12
		cmp rax, 3
		jl .SumdTooShort
		mov rax, 1
		mov [rsp + 8], rax ; loadD = 1
	%elif %2 == 9
		cmp rax, 2
		jl .SumdTooShort
		mov rax, 1
		mov [rsp + 8], rax ; loadD = 1
	%else
		%error "not valid"
	%endif
	.SumdTooShort

	;;;;;;;;;;;;;;;;;;;;;;
	;; dataPtr			
	;;;;;;;;;;;;;;;;;;;;;;
	; 1 2 3 5 6 7
	mov rsi, [rdx + 1*COMPV_YASM_REG_SZ_BYTES]
	mov rdi, [rdx + 2*COMPV_YASM_REG_SZ_BYTES]
	lea rax, [rbx + rsi]
	lea rcx, [rbx + rdi]
	movdqu xmm0, [rax]
	movdqu xmm1, [rcx]
	mov rsi, [rdx + 3*COMPV_YASM_REG_SZ_BYTES]
	mov rdi, [rdx + 5*COMPV_YASM_REG_SZ_BYTES]
	lea rax, [rbx + rsi]
	lea rcx, [rbx + rdi]
	movdqu xmm2, [rax]
	movdqu xmm3, [rcx]
	mov rsi, [rdx + 6*COMPV_YASM_REG_SZ_BYTES]
	mov rdi, [rdx + 7*COMPV_YASM_REG_SZ_BYTES]
	lea rax, [rbx + rsi]
	lea rcx, [rbx + rdi]
	movdqu xmm4, [rax]
	movdqu xmm7, [rcx]
	movdqa [rsp + 24 + (1*16)], xmm0 ; dataPtr[pixels16[1]]
	movdqa [rsp + 24 + (2*16)], xmm1 ; dataPtr[pixels16[2]]
	movdqa [rsp + 24 + (3*16)], xmm2 ; dataPtr[pixels16[3]]
	movdqa [rsp + 24 + (5*16)], xmm3 ; dataPtr[pixels16[5]]
	movdqa [rsp + 24 + (6*16)], xmm4 ; dataPtr[pixels16[6]]
	movdqa [rsp + 24 + (7*16)], xmm7 ; dataPtr[pixels16[7]]
	; 9 10 11 13 14 15
	mov rsi, [rdx + 9*COMPV_YASM_REG_SZ_BYTES]
	mov rdi, [rdx + 10*COMPV_YASM_REG_SZ_BYTES]
	lea rax, [rbx + rsi]
	lea rcx, [rbx + rdi]
	movdqu xmm0, [rax]
	movdqu xmm1, [rcx]
	mov rsi, [rdx + 11*COMPV_YASM_REG_SZ_BYTES]
	mov rdi, [rdx + 13*COMPV_YASM_REG_SZ_BYTES]
	lea rax, [rbx + rsi]
	lea rcx, [rbx + rdi]
	movdqu xmm2, [rax]
	movdqu xmm3, [rcx]
	mov rsi, [rdx + 14*COMPV_YASM_REG_SZ_BYTES]
	mov rdi, [rdx + 15*COMPV_YASM_REG_SZ_BYTES]
	lea rax, [rbx + rsi]
	lea rcx, [rbx + rdi]
	movdqu xmm4, [rax]
	movdqu xmm7, [rcx]
	movdqa [rsp + 24 + (9*16)], xmm0 ; dataPtr[pixels16[9]]
	movdqa [rsp + 24 + (10*16)], xmm1 ; dataPtr[pixels16[10]]	
	movdqa [rsp + 24 + (11*16)], xmm2 ; dataPtr[pixels16[11]]
	movdqa [rsp + 24 + (13*16)], xmm3 ; dataPtr[pixels16[13]]	
	movdqa [rsp + 24 + (14*16)], xmm4 ; dataPtr[pixels16[14]]
	movdqa [rsp + 24 + (15*16)], xmm7 ; dataPtr[pixels16[15]]

	;;;;;;;;;;;;;;;;;;;;;;
	;; Darkers			
	;;;;;;;;;;;;;;;;;;;;;;
	mov rax, [rsp + 8] ; loadD
	test rax, rax
	je .SetDarkersFlagsToZero
	; xmmDdarkers16x16 1 and 2
	mov rax, arg(6) ; xmmDdarkers16x16
	movdqa xmm0, [rsp + 24 + (1*16)] ; dataPtr[pixels16[1]]
	movdqa xmm1, [rsp + 24 + (2*16)] ; dataPtr[pixels16[2]]
	movdqa xmm4, xmm5 ; xmmDarker
	movdqa xmm7, xmm5 ; xmmDarker
	psubusb xmm4, xmm0
	psubusb xmm7, xmm1
	movdqa [rax + 1*32+0*16], xmm4 ; xmmDdarkers16x16[1][0]
	movdqa [rax + 2*32+0*16], xmm7 ; xmmDdarkers16x16[2][0]
	; xmmDdarkers16x16 3 and 5
	movdqa xmm0, [rsp + 24 + (3*16)]  ; dataPtr[pixels16[3]]
	movdqa xmm1, [rsp + 24 + (5*16)]  ; dataPtr[pixels16[5]]
	movdqa xmm4, xmm5 ; xmmDarker
	movdqa xmm7, xmm5 ; xmmDarker
	psubusb xmm4, xmm0
	psubusb xmm7, xmm1
	movdqa [rax + 3*32+0*16], xmm4 ; xmmDdarkers16x16[3][0]
	movdqa [rax + 5*32+0*16], xmm7 ; xmmDdarkers16x16[5][0]
	; xmmDdarkers16x16 6 and 7
	movdqa xmm0, [rsp + 24 + (6*16)]  ; dataPtr[pixels16[6]]
	movdqa xmm1, [rsp + 24 + (7*16)]  ; dataPtr[pixels16[7]]
	movdqa xmm4, xmm5 ; xmmDarker
	movdqa xmm7, xmm5 ; xmmDarker
	psubusb xmm4, xmm0
	psubusb xmm7, xmm1
	movdqa [rax + 6*32+0*16], xmm4 ; xmmDdarkers16x16[6][0]
	movdqa [rax + 7*32+0*16], xmm7 ; xmmDdarkers16x16[7][0]
	; xmmDdarkers16x16 9 and 10
	movdqa xmm0, [rsp + 24 + (9*16)]  ; dataPtr[pixels16[9]]
	movdqa xmm1, [rsp + 24 + (10*16)]  ; dataPtr[pixels16[10]]
	movdqa xmm4, xmm5 ; xmmDarker
	movdqa xmm7, xmm5 ; xmmDarker
	psubusb xmm4, xmm0
	psubusb xmm7, xmm1
	movdqa [rax + 9*32+0*16], xmm4 ; xmmDdarkers16x16[9][0]
	movdqa [rax + 10*32+0*16], xmm7 ; xmmDdarkers16x16[10][0]
	; xmmDdarkers16x16 11 and 13
	movdqa xmm0, [rsp + 24 + (11*16)]  ; dataPtr[pixels16[11]]
	movdqa xmm1, [rsp + 24 + (13*16)]  ; dataPtr[pixels16[13]]
	movdqa xmm4, xmm5 ; xmmDarker
	movdqa xmm7, xmm5 ; xmmDarker
	psubusb xmm4, xmm0
	psubusb xmm7, xmm1
	movdqa [rax + 11*32+0*16], xmm4 ; xmmDdarkers16x16[11][0]
	movdqa [rax + 13*32+0*16], xmm7 ; xmmDdarkers16x16[13][0]
	; xmmDdarkers16x16 14 and 15
	movdqa xmm0, [rsp + 24 + (14*16)] ; dataPtr[pixels16[14]]
	movdqa xmm1, [rsp + 24 + (15*16)] ; dataPtr[pixels16[15]]
	movdqa xmm4, xmm5 ; xmmDarker
	movdqa xmm7, xmm5 ; xmmDarker
	psubusb xmm4, xmm0
	psubusb xmm7, xmm1
	movdqa [rax + 14*32+0*16], xmm4 ; xmmDdarkers16x16[14][0]
	movdqa [rax + 15*32+0*16], xmm7 ; xmmDdarkers16x16[15][0]
	; Transpose
	COMPV_TRANSPOSE_I8_16X16_REG_SSE2 rax+0*32+0*16, rax+1*32+0*16, rax+2*32+0*16, rax+3*32+0*16, rax+4*32+0*16, rax+5*32+0*16, rax+6*32+0*16, rax+7*32+0*16, rax+8*32+0*16, rax+9*32+0*16, rax+10*32+0*16, rax+11*32+0*16, rax+12*32+0*16, rax+13*32+0*16, rax+14*32+0*16, rax+15*32+0*16, xmm0, xmm1	
	; pfdarkers16 ;
	mov rsi, arg(4) ; compv_scalar_t(&pfdarkers16)[16]
	mov rax, arg(6) ; xmmDdarkers16x16
	; first 8 values for pfdarkers16
	%assign i 0
	%rep    8
		pxor xmm %+ i, xmm %+ i
		%assign i i+1
	%endrep
	%assign i 0
	%rep    8
		pcmpeqb xmm %+ i, [rax+i*32+0*16]
		%assign i i+1
	%endrep
	%assign i 0
	%rep    8
		pmovmskb rcx, xmm %+ i
		not rcx
		and rcx, 0xFFFF
		mov [rsi + i*COMPV_YASM_REG_SZ_BYTES], rcx
		%assign i i+1
	%endrep
	; second 8 values for pfdarkers16
	%assign i 0
	%rep    8
		pxor xmm %+ i, xmm %+ i
		%assign i i+1
	%endrep
	%assign i 0
	%rep    8
		pcmpeqb xmm %+ i, [rax+(i+8)*32+0*16]
		%assign i i+1
	%endrep
	%assign i 0
	%rep    8
		lea rdi, [rsi + (i+8)*COMPV_YASM_REG_SZ_BYTES]
		pmovmskb rcx, xmm %+ i
		not rcx
		and rcx, 0xFFFF
		mov [rdi], rcx
		%assign i i+1
	%endrep
	; Convert ddarkers16x16 from epi8 to epi16
	pxor xmm0, xmm0
	mov rax, arg(6) ; xmmDdarkers16x16
	%assign i 0
	%rep    8
		movdqa xmm1, [rax + (i+0)*32+0*16]
		movdqa xmm2, [rax + (i+1)*32+0*16]
		movdqa xmm3, xmm1
		movdqa xmm4, xmm2
		punpcklbw xmm1, xmm0
		punpckhbw xmm3, xmm0
		punpcklbw xmm2, xmm0
		punpckhbw xmm4, xmm0
		movdqa [rax + (i+0)*32+0*16], xmm1
		movdqa [rax + (i+0)*32+1*16], xmm3
		movdqa [rax + (i+1)*32+0*16], xmm2
		movdqa [rax + (i+1)*32+1*16], xmm4
		%assign i i+2
	%endrep
	jmp .EndOfDarkers
	.SetDarkersFlagsToZero
	xor rcx, rcx
	mov rax, arg(4) ; compv_scalar_t(&pfdarkers16)[16]
	%assign i 0
	%rep    16
		mov [rax + i*COMPV_YASM_REG_SZ_BYTES], rcx
		%assign i i+1
	%endrep
	.EndOfDarkers

	;;;;;;;;;;;;;;;;;;;;;;;;
	;; Brighters
	;;;;;;;;;;;;;;;;;;;;;;;;
	mov rax, [rsp + 16] ; loadB
	test rax, rax
	je .SetBrightersFlagsToZero
	; restore xmm6 (xmmBrighter)
	mov rax, arg(3) ; rax = threshold
	movdqu xmm6, [rbx] ; xmm6 = xmmDataPtr[0]
	movd xmm7, rax
	punpcklbw xmm7, xmm7  
	punpcklwd xmm7, xmm7  
	pshufd xmm7, xmm7, 0  ; xmm7 = _mm_set1_epi8((uint8_t)threshold)) = xmmThreshold
	paddusb xmm6, xmm7 ; xmm6 = xmmBrighter
	; xmmDbrighters16x16 1 and 2
	mov rcx, arg(7) ; xmmDbrighters16x16
	movdqa xmm0, [rsp + 24 + (1*16)] ; dataPtr[pixels16[1]]
	movdqa xmm1, [rsp + 24 + (2*16)] ; dataPtr[pixels16[2]]
	psubusb xmm0, xmm6
	psubusb xmm1, xmm6
	movdqa [rcx + 1*32+0*16], xmm0 ; xmmDbrighters16x16[1][0]
	movdqa [rcx + 2*32+0*16], xmm1 ; xmmDbrighters16x16[2][0]
	; xmmDbrighters16x16 3 and 5
	movdqa xmm0, [rsp + 24 + (3*16)] ; dataPtr[pixels16[3]]
	movdqa xmm1, [rsp + 24 + (5*16)] ; dataPtr[pixels16[5]]
	psubusb xmm0, xmm6
	psubusb xmm1, xmm6
	movdqa [rcx + 3*32+0*16], xmm0 ; xmmDbrighters16x16[3][0]
	movdqa [rcx + 5*32+0*16], xmm1 ; xmmDbrighters16x16[5][0]
	; xmmDbrighters16x16 6 and 7
	movdqa xmm0, [rsp + 24 + (6*16)] ; dataPtr[pixels16[6]]
	movdqa xmm1, [rsp + 24 + (7*16)] ; dataPtr[pixels16[7]]
	psubusb xmm0, xmm6
	psubusb xmm1, xmm6
	movdqa [rcx + 6*32+0*16], xmm0 ; xmmDbrighters16x16[6][0]
	movdqa [rcx + 7*32+0*16], xmm1 ; xmmDbrighters16x16[7][0]
	; xmmDbrighters16x16 9 and 10
	movdqa xmm0, [rsp + 24 + (9*16)] ; dataPtr[pixels16[9]]
	movdqa xmm1, [rsp + 24 + (10*16)] ; dataPtr[pixels16[10]]
	psubusb xmm0, xmm6
	psubusb xmm1, xmm6
	movdqa [rcx + 9*32+0*16], xmm0 ; xmmDbrighters16x16[9][0]
	movdqa [rcx + 10*32+0*16], xmm1 ; xmmDbrighters16x16[10][0]
	; xmmDbrighters16x16 11 and 13
	movdqa xmm0, [rsp + 24 + (11*16)] ; dataPtr[pixels16[11]]
	movdqa xmm1, [rsp + 24 + (13*16)] ; dataPtr[pixels16[13]]
	psubusb xmm0, xmm6
	psubusb xmm1, xmm6
	movdqa [rcx + 11*32+0*16], xmm0 ; xmmDbrighters16x16[11][0]
	movdqa [rcx + 13*32+0*16], xmm1 ; xmmDbrighters16x16[13][0]
	; xmmDbrighters16x16 14 and 15
	movdqa xmm0, [rsp + 24 + (14*16)] ; dataPtr[pixels16[14]]
	movdqa xmm1, [rsp + 24 + (15*16)] ; dataPtr[pixels16[15]]
	psubusb xmm0, xmm6
	psubusb xmm1, xmm6
	movdqa [rcx + 14*32+0*16], xmm0 ; xmmDbrighters16x16[14][0]
	movdqa [rcx + 15*32+0*16], xmm1 ; xmmDbrighters16x16[15][0]
	; Transpose
	COMPV_TRANSPOSE_I8_16X16_REG_SSE2 rcx+0*32+0*16, rcx+1*32+0*16, rcx+2*32+0*16, rcx+3*32+0*16, rcx+4*32+0*16, rcx+5*32+0*16, rcx+6*32+0*16, rcx+7*32+0*16, rcx+8*32+0*16, rcx+9*32+0*16, rcx+10*32+0*16, rcx+11*32+0*16, rcx+12*32+0*16, rcx+13*32+0*16, rcx+14*32+0*16, rcx+15*32+0*16, xmm0, xmm1
	; pfbrighters16 ;
	mov rsi, arg(5) ; compv_scalar_t(&pfbrighters16)[16]
	mov rax, arg(7) ; xmmDbrighters16x16
	; first 8 values for pfbrighters16
	%assign i 0
	%rep    8
		pxor xmm %+ i, xmm %+ i
		%assign i i+1
	%endrep
	%assign i 0
	%rep    8
		pcmpeqb xmm %+ i, [rax+i*32+0*16]
		%assign i i+1
	%endrep
	%assign i 0
	%rep    8
		lea rdi, [rsi + i*COMPV_YASM_REG_SZ_BYTES]
		pmovmskb rcx, xmm %+ i
		not rcx
		and rcx, 0xFFFF
		mov [rdi], rcx
		%assign i i+1
	%endrep
	; second 8 values for pfbrighters16
	%assign i 0
	%rep    8
		pxor xmm %+ i, xmm %+ i
		%assign i i+1
	%endrep
	%assign i 0
	%rep    8
		pcmpeqb xmm %+ i, [rax+(i+8)*32+0*16]
		%assign i i+1
	%endrep
	%assign i 0
	%rep    8
		lea rdi, [rsi + (i+8)*COMPV_YASM_REG_SZ_BYTES]
		pmovmskb rcx, xmm %+ i
		not rcx
		and rcx, 0xFFFF
		mov [rdi], rcx
		%assign i i+1
	%endrep
	; Convert dbrighters16x16 from epi8 to epi16
	pxor xmm0, xmm0
	mov rax, arg(7) ; xmmDbrighters16x16
	%assign i 0
	%rep    8
		movdqa xmm1, [rax + (i+0)*32+0*16]
		movdqa xmm2, [rax + (i+1)*32+0*16]
		movdqa xmm3, xmm1
		movdqa xmm4, xmm2
		punpcklbw xmm1, xmm0
		punpckhbw xmm3, xmm0
		punpcklbw xmm2, xmm0
		punpckhbw xmm4, xmm0
		movdqa [rax + (i+0)*32+0*16], xmm1
		movdqa [rax + (i+0)*32+1*16], xmm3
		movdqa [rax + (i+1)*32+0*16], xmm2
		movdqa [rax + (i+1)*32+1*16], xmm4
		%assign i i+2
	%endrep
	jmp .EndOfBrighters
	.SetBrightersFlagsToZero
	xor rcx, rcx
	mov rax, arg(5) ; compv_scalar_t(&pfbrighters16)[16]
	%assign i 0
	%rep    16
		mov [rax + i*COMPV_YASM_REG_SZ_BYTES], rcx
		%assign i i+1
	%endrep
	.EndOfBrighters

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; Build the return value 
	xor rax, rax
	mov rsi, arg(4) ; compv_scalar_t(&pfdarkers16)[16]
	mov rdi, arg(5) ; compv_scalar_t(&pfbrighters16)[16]
	mov rbx, arg(2) ; compv_scalar_t N
	%assign i 0
	%rep    16
		popcnt rcx, [rsi + i*COMPV_YASM_REG_SZ_BYTES] ; FIXME: popcnt
		popcnt rdx, [rdi + i*COMPV_YASM_REG_SZ_BYTES] ; FIXME: popcnt
		cmp rcx, rbx
		jl .SkipDarkers %+ i
		or rax, 1<<i
		.SkipDarkers %+ i
		cmp rdx, rbx
		jl .SkipBrighters %+ i
		or rax, 1<<i
		.SkipBrighters %+ i
	%assign i i+1
	%endrep

	.EndOfTheFunction

	; unalign and free memory
	add rsp, 8*3+16*16
	COMPV_YASM_UNALIGN_STACK

	; begin epilog
	pop rbx
	pop rdi
	pop rsi
    COMPV_YASM_RESTORE_XMM
    COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t Fast9Data16_Asm_POPCNT_X86_SSE2(const uint8_t* dataPtr, COMPV_ALIGNED(SSE) const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, COMPV_ALIGNED(SSE) compv_scalar_t(&pfdarkers16)[16], COMPV_ALIGNED(SSE) compv_scalar_t(&pfbrighters16)[16], COMPV_ALIGNED(SSE) int16_t(&ddarkers16x16)[16][16], COMPV_ALIGNED(SSE) int16_t(&dbrighters16x16)[16][16])
sym(Fast9Data16_Asm_POPCNT_X86_SSE2):
	FastData16_Asm_X86_SSE2 1, 9

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t Fast9Data16_Asm_X86_SSE2(const uint8_t* dataPtr, COMPV_ALIGNED(SSE) const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, COMPV_ALIGNED(SSE) compv_scalar_t(&pfdarkers16)[16], COMPV_ALIGNED(SSE) compv_scalar_t(&pfbrighters16)[16], COMPV_ALIGNED(SSE) int16_t(&ddarkers16x16)[16][16], COMPV_ALIGNED(SSE) int16_t(&dbrighters16x16)[16][16])
sym(Fast9Data16_Asm_X86_SSE2):
	FastData16_Asm_X86_SSE2 0, 9

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t Fast12Data16_Asm_POPCNT_X86_SSE2(const uint8_t* dataPtr, COMPV_ALIGNED(SSE) const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, COMPV_ALIGNED(SSE) compv_scalar_t(&pfdarkers16)[16], COMPV_ALIGNED(SSE) compv_scalar_t(&pfbrighters16)[16], COMPV_ALIGNED(SSE) int16_t(&ddarkers16x16)[16][16], COMPV_ALIGNED(SSE) int16_t(&dbrighters16x16)[16][16])
sym(Fast12Data16_Asm_POPCNT_X86_SSE2):
	FastData16_Asm_X86_SSE2 1, 12

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t Fast12Data16_Asm_X86_SSE2(const uint8_t* dataPtr, COMPV_ALIGNED(SSE) const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, COMPV_ALIGNED(SSE) compv_scalar_t(&pfdarkers16)[16], COMPV_ALIGNED(SSE) compv_scalar_t(&pfbrighters16)[16], COMPV_ALIGNED(SSE) int16_t(&ddarkers16x16)[16][16], COMPV_ALIGNED(SSE) int16_t(&dbrighters16x16)[16][16])
sym(Fast12Data16_Asm_X86_SSE2):
	FastData16_Asm_X86_SSE2 0, 9


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* dataPtr
; arg(1) -> COMPV_ALIGNED(SSE) const compv_scalar_t(&pixels16)[16]
; arg(2) -> compv_scalar_t N
; arg(3) -> compv_scalar_t threshold
; arg(4) -> compv_scalar_t *pfdarkers
; arg(5) -> compv_scalar_t* pfbrighters
; arg(6) -> COMPV_ALIGNED(SSE) int16_t(&ddarkers16)[16]
; arg(7) -> COMPV_ALIGNED(SSE) int16_t(&dbrighters16)[16]
; compv_scalar_t FastData_Intrin_SSE2(const uint8_t* dataPtr, COMPV_ALIGNED(SSE) const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, compv_scalar_t *pfdarkers, compv_scalar_t* pfbrighters, COMPV_ALIGNED(SSE) int16_t(&ddarkers16)[16], COMPV_ALIGNED(SSE) int16_t(&dbrighters16)[16])
sym(FastData_Asm_x86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	push rsi
	push rdi
	push rbx
	; end prolog

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 16, rax
	sub rsp, 16

	mov rbx, arg(0) ; dataPtr
	mov rdx, arg(1) ; pixels16

	mov rax, arg(3) ; threshold
	mov rsi, [rbx]
	mov rdi, [rbx]
	add rsi, rax ; brighter
	sub rdi, rax ; darker

	; compare I1 and I7
	lea rax, [rdx + 0*COMPV_YASM_REG_SZ_BYTES]
	mov rcx, [rbx + rax] ; dataPtr[pixels16[0]];
	;mov BYTE [rsp + 0], rcx




	mov BYTE [rbx], 2

	; unalign stack and free memory
	add rsp, 16
	COMPV_YASM_UNALIGN_STACK

	; begin epilog
	pop rbx
	pop rdi
	pop rsi
    COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
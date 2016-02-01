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

COMPV_YASM_DEFAULT_REL

global sym(FastStrengths9_Asm_CMOV_X86_SSE41)
global sym(FastStrengths9_Asm_X86_SSE41)
global sym(FastStrengths12_Asm_CMOV_X86_SSE41)
global sym(FastStrengths12_Asm_X86_SSE41)

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
;;; compv_scalar_t FastStrengths9_Asm_CMOV_X86_SSE41(COMPV_ALIGNED(SSE) const int16_t(&dbrighters)[16], COMPV_ALIGNED(SSE) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
sym(FastStrengths9_Asm_CMOV_X86_SSE41):
	FastStrengths_Asm_X86_SSE41 1, 9

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t FastStrengths9_Asm_X86_SSE41(COMPV_ALIGNED(SSE) const int16_t(&dbrighters)[16], COMPV_ALIGNED(SSE) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
sym(FastStrengths9_Asm_X86_SSE41):
	FastStrengths_Asm_X86_SSE41 0, 9

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t FastStrengths12_Asm_CMOV_X86_SSE41(COMPV_ALIGNED(SSE) const int16_t(&dbrighters)[16], COMPV_ALIGNED(SSE) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
sym(FastStrengths12_Asm_CMOV_X86_SSE41):
	FastStrengths_Asm_X86_SSE41 1, 12

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t FastStrengths12_Asm_X86_SSE41(COMPV_ALIGNED(SSE) const int16_t(&dbrighters)[16], COMPV_ALIGNED(SSE) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
sym(FastStrengths12_Asm_X86_SSE41):
	FastStrengths_Asm_X86_SSE41 0, 12


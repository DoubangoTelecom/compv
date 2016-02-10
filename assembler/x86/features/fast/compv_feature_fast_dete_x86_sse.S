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

global sym(FastData_Asm_CMOV_POPCNT_x86_SSE2)
global sym(FastData16Row_Asm_X86_SSE2)

section .data
	extern sym(kFast9Arcs)
	extern sym(kFast12Arcs)
	extern sym(k1_i8)
	extern sym(k254_u8)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* IP
; arg(1) -> const uint8_t* IPprev
; arg(2) -> compv_scalar_t width
; arg(3) -> const compv_scalar_t(&pixels16)[16]
; arg(4) -> compv_scalar_t N
; arg(5) -> compv_scalar_t threshold
; arg(6) -> COMPV_ALIGNED(SSE) compv_scalar_t(*pfdarkers16)[16]
; arg(7) -> COMPV_ALIGNED(SSE) compv_scalar_t(*pfbrighters16)[16]
; arg(8) -> COMPV_ALIGNED(SSE) uint8_t* ddarkers16x16
; arg(9) -> COMPV_ALIGNED(SSE) uint8_t* dbrighters16x16
; arg(10) -> compv_scalar_t* rd
; arg(11) -> compv_scalar_t* rb
; arg(12) -> compv_scalar_t* me
; void FastData16Row_Asm_X86_SSE2(const uint8_t* IP, const uint8_t* IPprev, compv_scalar_t width, const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, COMPV_ALIGNED(SSE) compv_scalar_t(*pfdarkers16)[16], COMPV_ALIGNED(SSE) compv_scalar_t(*pfbrighters16)[16], COMPV_ALIGNED(SSE) uint8_t* ddarkers16x16, COMPV_ALIGNED(SSE) uint8_t* dbrighters16x16, compv_scalar_t* rd, compv_scalar_t* rb, compv_scalar_t* me);
sym(FastData16Row_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 13
	COMPV_YASM_SAVE_XMM 7 ;XMM[6-n]
	push rsi
	push rdi
	push rbx
	; end prolog

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 16, rax
	sub rsp, 8+8+8+8+8+8 + 16+16+16+16 + 16*16 + 16*16 + 16*16
	; [rsp + 0] = sum
	; [rsp + 8] = s ; FIXME: remove, useless
	; [rsp + 16] = colDarkersFlags
	; [rsp + 24] = colBrightersFlags
	; [rsp + 32] = loadB ; FIXME: remove
	; [rsp + 40] = loadD ; FIMXE: remove
	; [rsp + 48] = xmmThreshold; FIXME: remove
	; [rsp + 64] = xmmBrighter
	; [rsp + 80] = xmmDarker
	; [rsp + 96] = xmmNMinusOne
	; [rsp + 112] = xmmDarkersFlags[16]
	; [rsp + 368] = xmmBrightersFlags[16]
	; [rsp + 624] = xmmDataPtr[16]

	mov rsi, arg(2) ; rsi = width
	mov rax, arg(5) ; threshold
	mov rbx, arg(0) ; rbx = IP
	movd xmm7, rax
	punpcklbw xmm7, xmm7  
	punpcklwd xmm7, xmm7  
	pshufd xmm7, xmm7, 0  ; xmm7 = _mm_set1_epi8((uint8_t)threshold)) = xmmThreshold

	; Compute xmmNMinusOne
	mov rax, arg(4) ; N
	sub rax, 1
	movd xmm0, rax
	punpcklbw xmm0, xmm0  
	punpcklwd xmm0, xmm0  
	pshufd xmm0, xmm0, 0
	movdqa [rsp + 96], xmm0
	
	;-------------------
	;StartOfLooopRows
	;
	.LoopRows
	; -------------------
	xor rcx, rcx
	mov rax, arg(11) ; rb
	mov rdx, arg(10) ; rd
	mov [rax], rcx
	mov [rdx], rcx

	movdqu xmm6, [rbx]

	; Motion Estimation
	; TODO(dmi): not supported
	; TODO(dmi): inc IPprev at the end of the function if motion estimation is supported
	xor rax, rax
	mov rcx, arg(12) ; me
	mov [rcx], rax

	movdqa xmm5, xmm6
	paddusb xmm6, xmm7 ; xmm6 = xmmBrighter
	psubusb xmm5, xmm7 ; xmm5 = xmmDarker

	;
	; Speed-Test-1
	;

	; compare I1 and I9 aka 0 and 8
	mov rdx, arg(3) ; pixels16
	mov rax, [rdx + 0*COMPV_YASM_REG_SZ_BYTES] ; pixels16[0]
	mov rdx, [rdx + 8*COMPV_YASM_REG_SZ_BYTES] ; pixels16[8]
	lea rax, [rbx + rax] ; IP[pixels16[0]]
	lea rdx, [rbx + rdx] ; IP[pixels16[8]]
	pxor xmm4, xmm4 ; xmm4 = xmmZeros
	movdqu xmm0, [rax]
	movdqu xmm1, [rdx]
	movdqa xmm2, xmm5 ; xmmDarker
	movdqa xmm3, xmm5 ; xmmDarker
	mov rdx, arg(8) ; ddarkers16x16
	mov rax, arg(9) ; dbrighters16x16
	psubusb xmm2, xmm0 ; ddarkers16x16[0]
	psubusb xmm3, xmm1 ; ddarkers16x16[8]
	psubusb xmm0, xmm6 ; dbrighters16x16[0]
	psubusb xmm1, xmm6 ; dbrighters16x16[8]
	movdqa [rdx + 0*16], xmm2
	movdqa [rdx + 8*16], xmm3
	movdqa [rax + 0*16], xmm0
	movdqa [rax + 8*16], xmm1
	pcmpeqb xmm2, xmm4
	pcmpeqb xmm3, xmm4
	pcmpeqb xmm0, xmm4
	pcmpeqb xmm1, xmm4
	pcmpeqb xmm4, xmm4  ; xmm4 = xmmFF
	pandn xmm2, xmm4
	pandn xmm3, xmm4
	pandn xmm0, xmm4
	pandn xmm1, xmm4
	movdqa [rsp + 112 + 0*16], xmm2 ; xmmDarkersFlags[0]
	movdqa [rsp + 112 + 8*16], xmm3 ; xmmDarkersFlags[8]
	movdqa [rsp + 368 + 0*16], xmm0 ; xmmBrightersFlags[0]
	movdqa [rsp + 368 + 8*16], xmm1 ; xmmBrightersFlags[8]
	por xmm0, xmm2
	por xmm1, xmm3
	xor rcx, rcx
	xor rdi, rdi
	pmovmskb rax, xmm0
	pmovmskb rdx, xmm1
	test rax, rax
	mov rax, 1
	cmovnz rcx, rax ; FIXME: CMOV
	test rdx, rdx
	cmovnz rdi, rax ; FIXME: CMOV
	add rdi, rcx
	test rdi, rdi
	jz .LoopRowsNext
	mov [rsp + 0], rdi ; sum = ?

	; compare I5 and I13 aka 4 and 12
	mov rdx, arg(3) ; pixels16
	mov rax, [rdx + 4*COMPV_YASM_REG_SZ_BYTES] ; pixels16[4]
	mov rdx, [rdx + 12*COMPV_YASM_REG_SZ_BYTES] ; pixels16[12]
	lea rax, [rbx + rax] ; IP[pixels16[4]]
	lea rdx, [rbx + rdx] ; IP[pixels16[12]]
	pxor xmm4, xmm4 ; xmm4 = xmmZeros
	movdqu xmm0, [rax]
	movdqu xmm1, [rdx]
	movdqa xmm2, xmm5 ; xmmDarker
	movdqa xmm3, xmm5 ; xmmDarker
	mov rdx, arg(8) ; ddarkers16x16
	mov rax, arg(9) ; dbrighters16x16
	psubusb xmm2, xmm0 ; ddarkers16x16[4]
	psubusb xmm3, xmm1 ; ddarkers16x16[12]
	psubusb xmm0, xmm6 ; dbrighters16x16[4]
	psubusb xmm1, xmm6 ; dbrighters16x16[12]
	movdqa [rdx + 4*16], xmm2
	movdqa [rdx + 12*16], xmm3
	movdqa [rax + 4*16], xmm0
	movdqa [rax + 12*16], xmm1
	pcmpeqb xmm2, xmm4
	pcmpeqb xmm3, xmm4
	pcmpeqb xmm0, xmm4
	pcmpeqb xmm1, xmm4
	pcmpeqb xmm4, xmm4  ; xmm4 = xmmFF
	pandn xmm2, xmm4
	pandn xmm3, xmm4
	pandn xmm0, xmm4
	pandn xmm1, xmm4
	movdqa [rsp + 112 + 4*16], xmm2 ; xmmDarkersFlags[4]
	movdqa [rsp + 112 + 12*16], xmm3 ; xmmDarkersFlags[12]
	movdqa [rsp + 368 + 4*16], xmm0 ; xmmBrightersFlags[4]
	movdqa [rsp + 368 + 12*16], xmm1 ; xmmBrightersFlags[12]
	por xmm0, xmm2
	por xmm1, xmm3
	xor rcx, rcx
	xor rdi, rdi
	pmovmskb rax, xmm0
	pmovmskb rdx, xmm1
	test rax, rax
	mov rax, 1
	cmovnz rcx, rax ; FIXME: CMOV
	test rdx, rdx
	cmovnz rdi, rax ; FIXME: CMOV
	add rdi, rcx
	test rdi, rdi
	jz .LoopRowsNext
	add [rsp + 0], rdi ; sum = ?

	;
	;  Speed-Test-2
	;
	
	mov rcx, arg(4) ; N
	mov rax, [rsp + 0] ; sum
	cmp rcx, 9
	je .SpeedTest2For9
	; otherwise ...N == 12
	cmp rax, 3
	jl .LoopRowsNext
	jmp .EndOfSpeedTest2

	.SpeedTest2For9
	cmp rax, 2
	jl .LoopRowsNext
	
	.EndOfSpeedTest2

	;
	;	Processing
	;

	; Check whether to load Brighters
	movdqa xmm0, [rsp + 368 + 0*16] ; xmmBrightersFlags[0]
	movdqa xmm1, [rsp + 368 + 4*16] ; xmmBrightersFlags[4]
	movdqa xmm2, [rsp + 368 + 8*16] ; xmmBrightersFlags[8]
	movdqa xmm3, [rsp + 368 + 12*16] ; xmmBrightersFlags[12]
	por xmm0, xmm2
	por xmm1, xmm3
	xor rcx, rcx
	xor rdi, rdi
	pmovmskb rax, xmm0
	pmovmskb rdx, xmm1
	test rax, rax
	mov rax, 1
	cmovnz rcx, rax ; FIXME: CMOV
	test rdx, rdx
	cmovnz rdi, rax ; FIXME: CMOV
	xor rax, rax
	add rdi, rcx
	cmp rdi, 1
	cmovle rdi, rax ; if (rdi <= 1) rdi = 0 ; FIXME: CMOV
	mov [rsp + 32], rdi ; loadB

	; Check whether to load Darkers
	movdqa xmm0, [rsp + 112 + 0*16] ; xmmDarkersFlags[0]
	movdqa xmm1, [rsp + 112 + 4*16] ; xmmDarkersFlags[4]
	movdqa xmm2, [rsp + 112 + 8*16] ; xmmDarkersFlags[8]
	movdqa xmm3, [rsp + 112 + 12*16] ; xmmDarkersFlags[12]
	por xmm0, xmm2
	por xmm1, xmm3
	xor rcx, rcx
	pmovmskb rax, xmm0
	test rax, rax
	mov rax, 1
	cmovnz rcx, rax ; FIXME: CMOV
	xor rdx, rdx
	pmovmskb rax, xmm1
	test rax, rax
	mov rax, 1
	cmovnz rdx, rax ; FIXME: CMOV
	xor rax, rax
	add rdx, rcx
	cmp rdx, 1
	cmovle rdx, rax ; if (rdx <= 1) rdx = 0 ; FIXME: CMOV
	mov [rsp + 40], rdx ; loadD

	; rdi = loadB, rdx = loadD
	; skip process if (!(loadB || loadD))
	mov rax, rdi
	or rax, rdx
	test rax, rax
	jz .LoopRowsNext	

	; Load xmmDataPtr
	mov rcx, arg(3) ; pixels16
	mov rax, [rcx + 1*COMPV_YASM_REG_SZ_BYTES] ; pixels16[1]
	lea rax, [rbx + rax] ; IP[pixels16[1]]
	movdqu xmm0, [rax]
	mov rax, [rcx + 2*COMPV_YASM_REG_SZ_BYTES] ; pixels16[2]
	lea rax, [rbx + rax] ; IP[pixels16[2]]
	movdqu xmm1, [rax]
	mov rax, [rcx + 3*COMPV_YASM_REG_SZ_BYTES] ; pixels16[3]
	lea rax, [rbx + rax] ; IP[pixels16[3]]
	movdqu xmm2, [rax]
	mov rax, [rcx + 5*COMPV_YASM_REG_SZ_BYTES] ; pixels16[5]
	lea rax, [rbx + rax] ; IP[pixels16[5]]
	movdqu xmm3, [rax]
	mov rax, [rcx + 6*COMPV_YASM_REG_SZ_BYTES] ; pixels16[6]
	lea rax, [rbx + rax] ; IP[pixels16[6]]
	movdqu xmm4, [rax]
	movdqa [rsp + 624 + 1*16], xmm0
	movdqa [rsp + 624 + 2*16], xmm1
	movdqa [rsp + 624 + 3*16], xmm2
	movdqa [rsp + 624 + 5*16], xmm3
	movdqa [rsp + 624 + 6*16], xmm4
	mov rax, [rcx + 7*COMPV_YASM_REG_SZ_BYTES] ; pixels16[7]
	lea rax, [rbx + rax] ; IP[pixels16[7]]
	movdqu xmm0, [rax]
	mov rax, [rcx + 9*COMPV_YASM_REG_SZ_BYTES] ; pixels16[9]
	lea rax, [rbx + rax] ; IP[pixels16[9]]
	movdqu xmm1, [rax]
	mov rax, [rcx + 10*COMPV_YASM_REG_SZ_BYTES] ; pixels16[10]
	lea rax, [rbx + rax] ; IP[pixels16[10]]
	movdqu xmm2, [rax]
	mov rax, [rcx + 11*COMPV_YASM_REG_SZ_BYTES] ; pixels16[11]
	lea rax, [rbx + rax] ; IP[pixels16[11]]
	movdqu xmm3, [rax]
	mov rax, [rcx + 13*COMPV_YASM_REG_SZ_BYTES] ; pixels16[13]
	lea rax, [rbx + rax] ; IP[pixels16[13]]
	movdqu xmm4, [rax]
	movdqa [rsp + 624 + 7*16], xmm0
	movdqa [rsp + 624 + 9*16], xmm1
	movdqa [rsp + 624 + 10*16], xmm2
	movdqa [rsp + 624 + 11*16], xmm3
	movdqa [rsp + 624 + 13*16], xmm4
	mov rax, [rcx + 14*COMPV_YASM_REG_SZ_BYTES] ; pixels16[14]
	lea rax, [rbx + rax] ; IP[pixels16[14]]
	movdqu xmm0, [rax]
	mov rax, [rcx + 15*COMPV_YASM_REG_SZ_BYTES] ; pixels16[15]
	lea rax, [rbx + rax] ; IP[pixels16[15]]
	movdqu xmm1, [rax]
	movdqa [rsp + 624 + 14*16], xmm0
	movdqa [rsp + 624 + 15*16], xmm1

	; We could compute pixels at 1 and 9, check if at least one is darker or brighter than the candidate
    ; Then, do the same for 2 and 10 etc etc ... but this is slower than whant we're doing below because
    ; _mm_movemask_epi8 is cyclyvore

	;
	;	LoadDarkers
	;
	test rdx, rdx ; rdx was loadD, now it's free
	jz .EndOfDarkers
	; compute ddarkers16x16
	mov rax, arg(8) ; ddarkers16x16
	movdqa xmm0, xmm5
	movdqa xmm1, xmm5
	movdqa xmm2, xmm5
	movdqa xmm3, xmm5
	movdqa xmm4, xmm5
	psubusb xmm0, [rsp + 624 + 1*16]
	psubusb xmm1, [rsp + 624 + 2*16]
	psubusb xmm2, [rsp + 624 + 3*16]
	psubusb xmm3, [rsp + 624 + 5*16]
	psubusb xmm4, [rsp + 624 + 6*16]
	movdqa [rax + 1*16], xmm0
	movdqa [rax + 2*16], xmm1
	movdqa [rax + 3*16], xmm2
	movdqa [rax + 5*16], xmm3
	movdqa [rax + 6*16], xmm4
	movdqa xmm0, xmm5
	movdqa xmm1, xmm5
	movdqa xmm2, xmm5
	movdqa xmm3, xmm5
	movdqa xmm4, xmm5
	psubusb xmm0, [rsp + 624 + 7*16]
	psubusb xmm1, [rsp + 624 + 9*16]
	psubusb xmm2, [rsp + 624 + 10*16]
	psubusb xmm3, [rsp + 624 + 11*16]
	psubusb xmm4, [rsp + 624 + 13*16]
	movdqa [rax + 7*16], xmm0
	movdqa [rax + 9*16], xmm1
	movdqa [rax + 10*16], xmm2
	movdqa [rax + 11*16], xmm3
	movdqa [rax + 13*16], xmm4
	movdqa xmm0, xmm5
	psubusb xmm0, [rsp + 624 + 14*16]
	psubusb xmm5, [rsp + 624 + 15*16] ; xmm5 can be overridden, no longer used
	movdqa [rax + 14*16], xmm0
	movdqa [rax + 15*16], xmm5
	; Compute flags (not really, we have the inverse: 0xFF when zero, the not will be applied later)
	pxor xmm5, xmm5
	movdqa xmm0, [rax + 1*16]
	movdqa xmm1, [rax + 2*16]
	movdqa xmm2, [rax + 3*16]
	movdqa xmm3, [rax + 5*16]
	movdqa xmm4, [rax + 6*16]
	pcmpeqb xmm0, xmm5
	pcmpeqb xmm1, xmm5
	pcmpeqb xmm2, xmm5
	pcmpeqb xmm3, xmm5
	pcmpeqb xmm4, xmm5
	movdqa xmm5, [sym(k1_i8)]
	pandn xmm0, xmm5
	pandn xmm1, xmm5
	pandn xmm2, xmm5
	pandn xmm3, xmm5
	pandn xmm4, xmm5
	paddusb xmm0, xmm1
	paddusb xmm2, xmm3
	paddusb xmm0, xmm4
	paddusb xmm0, xmm2
	movdqa [rsp + 112 + 1*16], xmm0 ; xmmDarkersFlags[1] = 1 + 2 + 3 + 5 + 6
	pxor xmm5, xmm5
	movdqa xmm0, [rax + 7*16]
	movdqa xmm1, [rax + 9*16]
	movdqa xmm2, [rax + 10*16]
	movdqa xmm3, [rax + 11*16]
	movdqa xmm4, [rax + 13*16]
	pcmpeqb xmm0, xmm5
	pcmpeqb xmm1, xmm5
	pcmpeqb xmm2, xmm5
	pcmpeqb xmm3, xmm5
	pcmpeqb xmm4, xmm5
	movdqa xmm5, [sym(k1_i8)]
	pandn xmm0, xmm5
	pandn xmm1, xmm5
	pandn xmm2, xmm5
	pandn xmm3, xmm5
	pandn xmm4, xmm5
	paddusb xmm0, xmm1
	paddusb xmm2, xmm3
	paddusb xmm0, xmm4
	paddusb xmm0, xmm2
	movdqa [rsp + 112 + 7*16], xmm0 ; xmmDarkersFlags[7] = 7 + 9 + 10 + 11 + 13
	pxor xmm5, xmm5
	movdqa xmm4, [sym(k1_i8)]
	movdqa xmm0, [rax + 14*16]
	movdqa xmm1, [rax + 15*16]
	pcmpeqb xmm0, xmm5
	pcmpeqb xmm1, xmm5
	pandn xmm0, xmm4
	pandn xmm1, xmm4
	paddusb xmm0, xmm1
	movdqa [rsp + 112 + 14*16], xmm0 ; xmmDarkersFlags[14] = 14 + 15
	; Compute flags 0, 4, 8, 12
	movdqa xmm5, [sym(k254_u8)]
	movdqa xmm4, [rsp + 96] ; xmmNMinusOne
	movdqa xmm0, xmm5
	movdqa xmm1, xmm5
	movdqa xmm2, xmm5
	movdqa xmm3, xmm5
	pandn xmm0, [rsp + 112 + 0*16]
	pandn xmm1, [rsp + 112 + 4*16]
	pandn xmm2, [rsp + 112 + 8*16]
	pandn xmm3, [rsp + 112 + 12*16]
	paddusb xmm0, xmm1
	paddusb xmm2, xmm3
	paddusb xmm0, xmm2 ; xmm0 = 0 + 4 + 8 + 12
	paddusb xmm0, [rsp + 112 + 1*16] ; xmm0 += 1 + 2 + 3 + 5 + 6
	paddusb xmm0, [rsp + 112 + 7*16] ; xmm0 += 7 + 9 + 10 + 11 + 13
	paddusb xmm0, [rsp + 112 + 14*16] ; xmm0 += 14 + 15
	; Check the columns with at least N non-zero bits
	pcmpgtb xmm0, xmm4
	pmovmskb rdx, xmm0
	test rdx, rdx
	jz .EndOfDarkers
	; Continue loading darkers
	mov rcx, arg(10) ; rd
	mov [rcx], rdx ; (*rd) = colDarkersFlags
	; Transpose
	COMPV_TRANSPOSE_I8_16X16_REG_SSE2 rax+0*16, rax+1*16, rax+2*16, rax+3*16, rax+4*16, rax+5*16, rax+6*16, rax+7*16, rax+8*16, rax+9*16, rax+10*16, rax+11*16, rax+12*16, rax+13*16, rax+14*16, rax+15*16, xmm0, xmm1
	; Flags
	pcmpeqb xmm5, xmm5 ; xmmFF
	mov rdx, arg(6) ; pfdarkers16
	%assign i 0
	%rep    4
		mov rax, arg(8) ; ddarkers16x16
		pxor xmm0, xmm0
		pxor xmm1, xmm1
		pxor xmm2, xmm2
		pxor xmm3, xmm3
		pcmpeqb xmm0, [rax+(0+i)*16]
		pcmpeqb xmm1, [rax+(1+i)*16]
		pcmpeqb xmm2, [rax+(2+i)*16]
		pcmpeqb xmm3, [rax+(3+i)*16]
		pandn xmm0, xmm5
		pandn xmm1, xmm5
		pandn xmm2, xmm5
		pandn xmm3, xmm5
		pmovmskb rax, xmm0
		pmovmskb rcx, xmm1
		mov [rdx + (0+i)*COMPV_YASM_REG_SZ_BYTES], rax
		mov [rdx + (1+i)*COMPV_YASM_REG_SZ_BYTES], rcx
		pmovmskb rax, xmm2
		pmovmskb rcx, xmm3
		mov [rdx + (2+i)*COMPV_YASM_REG_SZ_BYTES], rax
		mov [rdx + (3+i)*COMPV_YASM_REG_SZ_BYTES], rcx
		%assign i i+4
	%endrep
	
	.EndOfDarkers
	

	;
	;	LoadBrighters
	;
	test rdi, rdi ; rdi was loadB, now it's free
	jz .EndOfBrighters
	; compute Dbrighters
	mov rax, arg(9) ; dbrighters16x16
	movdqa xmm0, [rsp + 624 + 1*16]
	movdqa xmm1, [rsp + 624 + 2*16]
	movdqa xmm2, [rsp + 624 + 3*16]
	movdqa xmm3, [rsp + 624 + 5*16]
	movdqa xmm4, [rsp + 624 + 6*16]
	psubusb xmm0, xmm6
	psubusb xmm1, xmm6
	psubusb xmm2, xmm6
	psubusb xmm3, xmm6
	psubusb xmm4, xmm6
	movdqa [rax + 1*16], xmm0
	movdqa [rax + 2*16], xmm1
	movdqa [rax + 3*16], xmm2
	movdqa [rax + 5*16], xmm3
	movdqa [rax + 6*16], xmm4
	movdqa xmm0, [rsp + 624 + 7*16]
	movdqa xmm1, [rsp + 624 + 9*16]
	movdqa xmm2, [rsp + 624 + 10*16]
	movdqa xmm3, [rsp + 624 + 11*16]
	movdqa xmm4, [rsp + 624 + 13*16]
	psubusb xmm0, xmm6
	psubusb xmm1, xmm6
	psubusb xmm2, xmm6
	psubusb xmm3, xmm6
	psubusb xmm4, xmm6
	movdqa [rax + 7*16], xmm0
	movdqa [rax + 9*16], xmm1
	movdqa [rax + 10*16], xmm2
	movdqa [rax + 11*16], xmm3
	movdqa [rax + 13*16], xmm4
	movdqa xmm0, [rsp + 624 + 14*16]
	movdqa xmm1, [rsp + 624 + 15*16]
	psubusb xmm0, xmm6
	psubusb xmm1, xmm6
	movdqa [rax + 14*16], xmm0
	movdqa [rax + 15*16], xmm1
	; Compute flags (not really, we have the inverse: 0xFF when zero, the not will be applied later)
	pxor xmm6, xmm6
	movdqa xmm0, [rax + 1*16]
	movdqa xmm1, [rax + 2*16]
	movdqa xmm2, [rax + 3*16]
	movdqa xmm3, [rax + 5*16]
	movdqa xmm4, [rax + 6*16]
	pcmpeqb xmm0, xmm6
	pcmpeqb xmm1, xmm6
	pcmpeqb xmm2, xmm6
	pcmpeqb xmm3, xmm6
	pcmpeqb xmm4, xmm6
	movdqa xmm6, [sym(k1_i8)]
	pandn xmm0, xmm6
	pandn xmm1, xmm6
	pandn xmm2, xmm6
	pandn xmm3, xmm6
	pandn xmm4, xmm6
	paddusb xmm0, xmm1
	paddusb xmm2, xmm3
	paddusb xmm0, xmm4
	paddusb xmm0, xmm2
	movdqa [rsp + 368 + 1*16], xmm0 ; xmmBrightersFlags[1] = 1 + 2 + 3 + 5 + 6
	pxor xmm6, xmm6
	movdqa xmm0, [rax + 7*16]
	movdqa xmm1, [rax + 9*16]
	movdqa xmm2, [rax + 10*16]
	movdqa xmm3, [rax + 11*16]
	movdqa xmm4, [rax + 13*16]
	pcmpeqb xmm0, xmm6
	pcmpeqb xmm1, xmm6
	pcmpeqb xmm2, xmm6
	pcmpeqb xmm3, xmm6
	pcmpeqb xmm4, xmm6
	movdqa xmm6, [sym(k1_i8)]
	pandn xmm0, xmm6
	pandn xmm1, xmm6
	pandn xmm2, xmm6
	pandn xmm3, xmm6
	pandn xmm4, xmm6
	paddusb xmm0, xmm1
	paddusb xmm2, xmm3
	paddusb xmm0, xmm4
	paddusb xmm0, xmm2
	movdqa [rsp + 368 + 7*16], xmm0 ; xmmBrightersFlags[7] = 7 + 9 + 10 + 11 + 13
	pxor xmm6, xmm6
	movdqa xmm4, [sym(k1_i8)]
	movdqa xmm0, [rax + 14*16]
	movdqa xmm1, [rax + 15*16]
	pcmpeqb xmm0, xmm6
	pcmpeqb xmm1, xmm6
	pandn xmm0, xmm4
	pandn xmm1, xmm4
	paddusb xmm0, xmm1
	movdqa [rsp + 368 + 14*16], xmm0 ; xmmBrightersFlags[14] = 14 + 15
	; Compute flags 0, 4, 8, 12
	movdqa xmm6, [sym(k254_u8)]
	movdqa xmm4, [rsp + 96] ; xmmNMinusOne
	movdqa xmm0, xmm6
	movdqa xmm1, xmm6
	movdqa xmm2, xmm6
	movdqa xmm3, xmm6
	pandn xmm0, [rsp + 368 + 0*16]
	pandn xmm1, [rsp + 368 + 4*16]
	pandn xmm2, [rsp + 368 + 8*16]
	pandn xmm3, [rsp + 368 + 12*16]
	paddusb xmm0, xmm1
	paddusb xmm2, xmm3
	paddusb xmm0, xmm2 ; xmm0 = 0 + 4 + 8 + 12
	paddusb xmm0, [rsp + 368 + 1*16] ; xmm0 += 1 + 2 + 3 + 5 + 6
	paddusb xmm0, [rsp + 368 + 7*16] ; xmm0 += 7 + 9 + 10 + 11 + 13
	paddusb xmm0, [rsp + 368 + 14*16] ; xmm0 += 14 + 15
	; Check the columns with at least N non-zero bits
	pcmpgtb xmm0, xmm4
	pmovmskb rdx, xmm0
	test rdx, rdx
	jz .EndOfBrighters
	; Continue loading brighters
	mov rcx, arg(11) ; rb
	mov rax, arg(9) ; dbrighters16x16
	mov [rcx], rdx ; (*rb) = colBrightersFlags
	; Transpose
	COMPV_TRANSPOSE_I8_16X16_REG_SSE2 rax+0*16, rax+1*16, rax+2*16, rax+3*16, rax+4*16, rax+5*16, rax+6*16, rax+7*16, rax+8*16, rax+9*16, rax+10*16, rax+11*16, rax+12*16, rax+13*16, rax+14*16, rax+15*16, xmm0, xmm1
	; Flags
	pcmpeqb xmm6, xmm6 ; xmmFF
	mov rdx, arg(7) ; pfbrighters16
	%assign i 0
	%rep    4
		pxor xmm0, xmm0
		pxor xmm1, xmm1
		pxor xmm2, xmm2
		pxor xmm3, xmm3
		pcmpeqb xmm0, [rax+(0+i)*16]
		pcmpeqb xmm1, [rax+(1+i)*16]
		pcmpeqb xmm2, [rax+(2+i)*16]
		pcmpeqb xmm3, [rax+(3+i)*16]
		pandn xmm0, xmm6
		pandn xmm1, xmm6
		pandn xmm2, xmm6
		pandn xmm3, xmm6
		pmovmskb rdi, xmm0
		pmovmskb rcx, xmm1
		mov [rdx + (0+i)*COMPV_YASM_REG_SZ_BYTES], rdi
		mov [rdx + (1+i)*COMPV_YASM_REG_SZ_BYTES], rcx
		pmovmskb rdi, xmm2
		pmovmskb rcx, xmm3
		mov [rdx + (2+i)*COMPV_YASM_REG_SZ_BYTES], rdi
		mov [rdx + (3+i)*COMPV_YASM_REG_SZ_BYTES], rcx
		%assign i i+4
	%endrep

	.EndOfBrighters

	
	.LoopRowsNext
	mov rax, arg(11) ; rb
	mov rcx, arg(10) ; rd
	lea rax, [rax + COMPV_YASM_REG_SZ_BYTES]
	lea rcx, [rcx + COMPV_YASM_REG_SZ_BYTES]
	mov arg(11), rax ; rb += 1
	mov arg(10), rcx ; rd += 1
	lea rbx, [rbx + 16] ; IP += 16
	mov rax, arg(6) ; pfdarkers16
	mov rcx, arg(7) ; pfbrighters16
	lea rax, [rax + 16*COMPV_YASM_REG_SZ_BYTES]
	lea rcx, [rcx + 16*COMPV_YASM_REG_SZ_BYTES]
	mov arg(6), rax ; pfdarkers16 += 1
	mov arg(7), rcx ; pfbrighters16 += 1
	mov rax, arg(8) ;  ddarkers16x16
	mov rcx, arg(9) ;  dbrighters16x16
	lea rax, [rax + 16*16]
	lea rcx, [rcx + 16*16]
	mov arg(8), rax ; xmmDdarkers16x16 += 1
	mov arg(9), rcx ; xmmDbrighters16x16 += 1
	; TODO(dmi): Motion estimation not supported -> do not inc IPprev

	;-------------------
	;EndOfLooopRows
	sub rsi, 16
	test rsi, rsi
	jnz .LoopRows
	;-------------------

	; unalign stack and free memory
	add rsp, 8+8+8+8+8+8 + 16+16+16+16 + 16*16 + 16*16 + 16*16
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
	


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t FastStrengths_Asm_X86_SSE41(COMPV_ALIGNED(SSE) const int16_t(&dbrighters)[16], COMPV_ALIGNED(SSE) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t(&dbrighters)[16]
; arg(1) -> COMPV_ALIGNED(SSE) const uint8_t(&ddarkers)[16]
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

	mov rax, arg(5)
	xor rcx, rcx ; rcx = maxnbrighter
	xor rdx, rdx ; rdx = maxndarker

	; FAST hard-coded flags
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
	; Load dbrighters
	mov rbx, arg(0)
	movdqa xmm2, [rbx]
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
	; Load dbrighters
	mov rbx, arg(1)
	movdqa xmm2, [rbx]
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
; FUNCTION NOT USED
; arg(0) -> const uint8_t* dataPtr
; arg(1) -> COMPV_ALIGNED(SSE) const compv_scalar_t(&pixels16)[16]
; arg(2) -> compv_scalar_t N
; arg(3) -> compv_scalar_t threshold
; arg(4) -> compv_scalar_t *pfdarkers
; arg(5) -> compv_scalar_t* pfbrighters
; arg(6) -> COMPV_ALIGNED(SSE) uint8_t(&ddarkers16)[16]
; arg(7) -> COMPV_ALIGNED(SSE) uint8_t(&dbrighters16)[16]
; compv_scalar_t FastData_Asm_CMOV_POPCNT_x86_SSE2(const uint8_t* dataPtr, COMPV_ALIGNED(SSE) const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, compv_scalar_t *pfdarkers, compv_scalar_t* pfbrighters, COMPV_ALIGNED(SSE) int16_t(&ddarkers16)[16], COMPV_ALIGNED(SSE) int16_t(&dbrighters16)[16])
sym(FastData_Asm_CMOV_POPCNT_x86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	push rsi
	push rdi
	push rbx
	; end prolog

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 16, rax
	sub rsp, 16 ; [rsp + 0] = temp16[16]

	mov rbx, arg(0) ; dataPtr
	mov rdx, arg(1) ; pixels16

	; Initialize temp16
	%assign i 0
	%rep    16
		mov rax, [rdx + i*COMPV_YASM_REG_SZ_BYTES] ; rax = pixels16[i]
		movzx rcx, byte ptr [rbx + rax]
		mov byte ptr [rsp + i], cl  

		%if i == 0
			mov rax, arg(3) ; threshold
			movzx rsi, byte ptr [rbx]
			movzx rdi, byte ptr [rbx]
			pxor xmm0, xmm0 ; xmm0 = xmmZeros
			pcmpeqb xmm4, xmm4 ; xmmFF
			add rsi, rax ; brighter
			sub rdi, rax ; darker
			; TODO(dmi): clip brighter and darker (function not used -> ignore the TODO)
		%endif
		
		%if i == 4
			movd xmm2, rdi
			movd xmm3, rsi
		%endif
		%if i == 7
			punpcklbw xmm2, xmm2
			punpcklbw xmm3, xmm3
		%endif
		%if i == 10
			punpcklwd xmm2, xmm2
			punpcklwd xmm3, xmm3
		%endif
		%if i == 13
			pshufd xmm2, xmm2, 0  ; xmm2 = _mm_set1_epi8((uint8_t)darker)) = xmmDarker
			pshufd xmm3, xmm3, 0  ; xmm3 = _mm_set1_epi8((uint8_t)brighter)) = xmmBrighter
		%endif

		%assign i i+1
	%endrep

	movdqa xmm1, [rsp + 0] ; xmm1 = xmmTemp16

	psubusb xmm2, xmm1 ; xmm2 = xmmDdarkers16
	psubusb xmm1, xmm3 ; xmm1 = xmmDbrighters16

	mov rbx, arg(6) ; ddarkers16
	mov rdx, arg(7) ; dbrighters16
	
	movdqa [rbx], xmm2
	movdqa [rdx], xmm1
	
	pcmpeqb xmm2, xmm0
	pcmpeqb xmm1, xmm0
	pandn xmm2, xmm4
	pandn xmm1, xmm4
	pmovmskb rsi, xmm2
	pmovmskb rdi, xmm1

	mov rbx, arg(4) ; pfdarkers
	mov rdx, arg(5) ; pfbrighters

	xor rax, rax
	mov rcx, arg(2) ; N

	cmp rsi, 511
	jl .SkipDarkers
	mov [rbx], rsi
	mov rbx, 1<<0
	popcnt rsi, rsi
	cmp rsi, rcx
	cmovge rax, rbx
	.SkipDarkers
	
	cmp rdi, 511
	jl .SkipBrighters
	mov [rdx], rdi
	mov rdx, 1<<16
	add rdx, rax
	popcnt rdi, rdi
	cmp rdi, rcx
	cmovge rax, rdx
	.SkipBrighters

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
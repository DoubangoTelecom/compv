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

%if COMPV_YASM_ABI_IS_64BIT

%include "../../compv_bits_macros_x86.S"
%include "../../compv_math_macros_x86.S"

COMPV_YASM_DEFAULT_REL

global sym(FastData16Row_Asm_X64_SSE2)

section .data
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
sym(FastData16Row_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 13
	COMPV_YASM_SAVE_XMM 15 ;XMM[6-n]
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	push r14
	push r15
	; end prolog

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 16, rax
	sub rsp, 8 + 16 + 16*16 + 16*16 + 16*16
	; [rsp + 0] = sum
	; [rsp + 8] = xmmNMinusOne
	; [rsp + 24] = xmmDarkersFlags[16]
	; [rsp + 280] = xmmBrightersFlags[16]
	; [rsp + 536] = xmmDataPtr[16]

	mov rdx, arg(2) ; width
	shr rdx, 4 ; div width with 16 and mov rsi by 1 -> because of argi(x,rsi)
	xor rsi, rsi
	mov arg(2), rdx 
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
	movdqa [rsp + 8], xmm0
	
	;-------------------
	;StartOfLooopRows
	;
	.LoopRows
	; -------------------
	xor rcx, rcx
	mov rax, arg(11) ; rb
	mov rdx, arg(10) ; rd
	mov [rax + rsi*COMPV_YASM_REG_SZ_BYTES], rcx
	mov [rdx + rsi*COMPV_YASM_REG_SZ_BYTES], rcx

	movdqu xmm6, [rbx]

	; Motion Estimation
	; TODO(dmi): not supported
	; TODO(dmi): use argi(x,i) to retrieve the argument

	movdqa xmm5, xmm6
	paddusb xmm6, xmm7 ; xmm6 = xmmBrighter
	psubusb xmm5, xmm7 ; xmm5 = xmmDarker

	;
	; Speed-Test-1
	;

	pxor xmm15, xmm15 ; xmm15 = xmmZeros
	pcmpeqb xmm14, xmm14 ; xmm14 = xmmFF

	mov r10, rsi
	mov r8, arg(8) ; ddarkers16x16
	shl r10, 8 ; rsi * (16*16)
	mov r9, arg(9) ; dbrighters16x16
	lea r8, [r8 + r10] ; ddarkers16x16[rsi]
	lea r9, [r9 + r10] ; dbrighters16x16[rsi]

	; compare I1 and I9 aka 0 and 8
	mov rdx, arg(3) ; pixels16
	mov rax, [rdx + 0*COMPV_YASM_REG_SZ_BYTES] ; pixels16[0]
	mov rdx, [rdx + 8*COMPV_YASM_REG_SZ_BYTES] ; pixels16[8]
	movdqu xmm0, [rbx + rax]
	movdqa xmm2, xmm5 ; xmmDarker
	movdqu xmm1, [rbx + rdx]
	movdqa xmm3, xmm5 ; xmmDarker
	psubusb xmm2, xmm0 ; ddarkers16x16[0]
	psubusb xmm3, xmm1 ; ddarkers16x16[8]
	psubusb xmm0, xmm6 ; dbrighters16x16[0]
	psubusb xmm1, xmm6 ; dbrighters16x16[8]
	movdqa [r8 + 0*16], xmm2
	movdqa [r8 + 8*16], xmm3
	movdqa [r9 + 0*16], xmm0
	movdqa [r9 + 8*16], xmm1
	pcmpeqb xmm2, xmm15
	pcmpeqb xmm3, xmm15
	pcmpeqb xmm0, xmm15
	pcmpeqb xmm1, xmm15
	pandn xmm2, xmm14
	pandn xmm3, xmm14
	pandn xmm0, xmm14
	pandn xmm1, xmm14
	movdqa [rsp + 24 + 0*16], xmm2 ; xmmDarkersFlags[0]
	movdqa [rsp + 24 + 8*16], xmm3 ; xmmDarkersFlags[8]
	movdqa [rsp + 280 + 0*16], xmm0 ; xmmBrightersFlags[0]
	movdqa [rsp + 280 + 8*16], xmm1 ; xmmBrightersFlags[8]
	movdqa xmm8, xmm0 ; save xmmBrightersFlags[0] in xmm8
	movdqa xmm9, xmm1 ; save xmmBrightersFlags[8] in xmm9
	movdqa xmm10, xmm2 ; save xmmDarkersFlags[0] in xmm10
	movdqa xmm11, xmm3 ; save xmmDarkersFlags[8] in xmm11
	por xmm0, xmm2
	por xmm1, xmm3
	pmovmskb rax, xmm0
	pmovmskb rdx, xmm1
	test rax, rax
	setnz al
	test rdx, rdx
	setnz dl
	add rdx, rax
	test rdx, rdx
	jz .LoopRowsNext
	mov [rsp + 0], rdx ; sum = ?

	; compare I5 and I13 aka 4 and 12
	mov rdx, arg(3) ; pixels16
	mov rax, [rdx + 4*COMPV_YASM_REG_SZ_BYTES] ; pixels16[4]
	mov rdx, [rdx + 12*COMPV_YASM_REG_SZ_BYTES] ; pixels16[12]
	movdqu xmm0, [rbx + rax]
	movdqa xmm2, xmm5 ; xmmDarker
	movdqu xmm1, [rbx + rdx]
	movdqa xmm3, xmm5 ; xmmDarker
	psubusb xmm2, xmm0 ; ddarkers16x16[4]
	psubusb xmm3, xmm1 ; ddarkers16x16[12]
	psubusb xmm0, xmm6 ; dbrighters16x16[4]
	psubusb xmm1, xmm6 ; dbrighters16x16[12]
	movdqa [r8 + 4*16], xmm2
	movdqa [r8 + 12*16], xmm3
	movdqa [r9 + 4*16], xmm0
	movdqa [r9 + 12*16], xmm1
	pcmpeqb xmm2, xmm15
	pcmpeqb xmm3, xmm15
	pcmpeqb xmm0, xmm15
	pcmpeqb xmm1, xmm15
	pandn xmm2, xmm14
	pandn xmm3, xmm14
	pandn xmm0, xmm14
	pandn xmm1, xmm14
	movdqa [rsp + 24 + 4*16], xmm2 ; xmmDarkersFlags[4]
	movdqa [rsp + 24 + 12*16], xmm3 ; xmmDarkersFlags[12]
	movdqa [rsp + 280 + 4*16], xmm0 ; xmmBrightersFlags[4]
	movdqa [rsp + 280 + 12*16], xmm1 ; xmmBrightersFlags[12]
	movdqa xmm12, xmm0 ; save xmmBrightersFlags[4] in xmm8
	movdqa xmm13, xmm1 ; save xmmBrightersFlags[12] in xmm9
	por xmm0, xmm2
	por xmm1, xmm3
	pmovmskb rax, xmm0
	pmovmskb rdx, xmm1
	test rax, rax
	setnz al 
	test rdx, rdx
	setnz dl
	add rdx, rax
	test rdx, rdx
	jz .LoopRowsNext
	add [rsp + 0], rdx ; sum = ?

	

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
	
	; Data for Darkers (loadD) and Brighters (loadB)
	por xmm10, xmm11
	por xmm2, xmm3
	por xmm8, xmm9
	por xmm12, xmm13
	pmovmskb r9, xmm10
	pmovmskb r10, xmm2
	pmovmskb r11, xmm8
	pmovmskb r12, xmm12

	; Check whether to load Darkers
	test r9, r9
	setnz cl
	test r10, r10
	setnz dl
	add rcx, rdx
	cmp rcx, 1
	setg dl ; rdx = (rcx > 1) ? 1 : 0

	; Check whether to load Brighters
	test r11, r11
	setnz cl
	test r12, r12
	setnz al
	add rcx, rax
	cmp rcx, 1
	setg dil ; rdi = (rcx > 1) ? 1 : 0
	

	; rdi = loadB, rdx = loadD
	; skip process if (!(loadB || loadD))
	mov rax, rdi
	or rax, rdx
	test rax, rax
	jz .LoopRowsNext	

	; Load xmmDataPtr
	mov rcx, arg(3) ; pixels16
	mov rax, [rcx + 1*COMPV_YASM_REG_SZ_BYTES] ; pixels16[1]
	mov r8, [rcx + 2*COMPV_YASM_REG_SZ_BYTES] ; pixels16[2]
	mov r9, [rcx + 3*COMPV_YASM_REG_SZ_BYTES] ; pixels16[3]
	mov r10, [rcx + 5*COMPV_YASM_REG_SZ_BYTES] ; pixels16[5]
	mov r11, [rcx + 6*COMPV_YASM_REG_SZ_BYTES] ; pixels16[6]
	mov r12, [rcx + 7*COMPV_YASM_REG_SZ_BYTES] ; pixels16[7]
	mov r13, [rcx + 9*COMPV_YASM_REG_SZ_BYTES] ; pixels16[9]
	mov r14, [rcx + 10*COMPV_YASM_REG_SZ_BYTES] ; pixels16[10]
	mov r15, [rcx + 11*COMPV_YASM_REG_SZ_BYTES] ; pixels16[11]
	movdqu xmm0, [rbx + rax]
	movdqu xmm1, [rbx + r8]
	movdqu xmm2, [rbx + r9]
	movdqu xmm3, [rbx + r10]
	movdqu xmm4, [rbx + r11]
	movdqu xmm8, [rbx + r12]
	movdqu xmm9, [rbx + r13]
	movdqu xmm10, [rbx + r14]
	movdqu xmm11, [rbx + r15]
	mov r8, [rcx + 13*COMPV_YASM_REG_SZ_BYTES] ; pixels16[13]
	mov r9, [rcx + 14*COMPV_YASM_REG_SZ_BYTES] ; pixels16[14]
	mov r10, [rcx + 15*COMPV_YASM_REG_SZ_BYTES] ; pixels16[15]
	movdqu xmm12, [rbx + r8]
	movdqu xmm13, [rbx + r9]	
	movdqu xmm14, [rbx + r10]
	movdqa [rsp + 536 + 1*16], xmm0
	movdqa [rsp + 536 + 2*16], xmm1
	movdqa [rsp + 536 + 3*16], xmm2
	movdqa [rsp + 536 + 5*16], xmm3
	movdqa [rsp + 536 + 6*16], xmm4
	movdqa [rsp + 536 + 7*16], xmm8
	movdqa [rsp + 536 + 9*16], xmm9
	movdqa [rsp + 536 + 10*16], xmm10
	movdqa [rsp + 536 + 11*16], xmm11
	movdqa [rsp + 536 + 13*16], xmm12
	movdqa [rsp + 536 + 14*16], xmm13
	movdqa [rsp + 536 + 15*16], xmm14

	; We could compute pixels at 1 and 9, check if at least one is darker or brighter than the candidate
	; Then, do the same for 2 and 10 etc etc ... but this is slower than whant we're doing below because
	; _mm_movemask_epi8 is cyclyvore

	;
	;	LoadDarkers
	;
	test rdx, rdx ; rdx was loadD, now it's free
	jz .EndOfDarkers
	; compute ddarkers16x16 and flags
	mov r9, rsi
	shl r9, 8 ; rsi * (16*16)
	mov rax, arg(8) ; ddarkers16x16
	lea rax, [rax + r9]
	pxor xmm4, xmm4
	movdqa xmm0, xmm5
	movdqa xmm1, xmm5
	movdqa xmm2, xmm5
	movdqa xmm3, xmm5
	movdqa xmm8, xmm5
	movdqa xmm9, xmm5
	movdqa xmm10, xmm5
	movdqa xmm11, xmm5
	movdqa xmm12, xmm5
	movdqa xmm13, xmm5
	movdqa xmm14, xmm5
	movdqa xmm15, xmm5
	psubusb xmm0, [rsp + 536 + 1*16]
	psubusb xmm1, [rsp + 536 + 2*16]
	psubusb xmm2, [rsp + 536 + 3*16]
	psubusb xmm3, [rsp + 536 + 5*16]
	psubusb xmm8, [rsp + 536 + 6*16]
	psubusb xmm9, [rsp + 536 + 7*16]
	psubusb xmm10, [rsp + 536 + 9*16]
	psubusb xmm11, [rsp + 536 + 10*16]
	psubusb xmm12, [rsp + 536 + 11*16]
	psubusb xmm13, [rsp + 536 + 13*16]
	psubusb xmm14, [rsp + 536 + 14*16]
	psubusb xmm15, [rsp + 536 + 15*16]
	movdqa [rax + 1*16], xmm0
	movdqa [rax + 2*16], xmm1
	movdqa [rax + 3*16], xmm2
	movdqa [rax + 5*16], xmm3
	movdqa [rax + 6*16], xmm8
	movdqa [rax + 7*16], xmm9
	movdqa [rax + 9*16], xmm10
	movdqa [rax + 10*16], xmm11
	movdqa [rax + 11*16], xmm12
	movdqa [rax + 13*16], xmm13
	movdqa [rax + 14*16], xmm14
	movdqa [rax + 15*16], xmm15
	pcmpeqb xmm0, xmm4
	pcmpeqb xmm1, xmm4
	pcmpeqb xmm2, xmm4
	pcmpeqb xmm3, xmm4
	pcmpeqb xmm8, xmm4
	pcmpeqb xmm9, xmm4
	pcmpeqb xmm10, xmm4
	pcmpeqb xmm11, xmm4
	pcmpeqb xmm12, xmm4
	pcmpeqb xmm13, xmm4
	pcmpeqb xmm14, xmm4
	pcmpeqb xmm15, xmm4
	movdqa xmm4, [sym(k1_i8)]
	pandn xmm0, xmm4
	pandn xmm1, xmm4
	pandn xmm2, xmm4
	pandn xmm3, xmm4
	pandn xmm8, xmm4
	pandn xmm9, xmm4
	pandn xmm10, xmm4
	pandn xmm11, xmm4
	pandn xmm12, xmm4
	pandn xmm13, xmm4
	pandn xmm14, xmm4
	pandn xmm15, xmm4
	paddusb xmm0, xmm1
	paddusb xmm2, xmm3
	paddusb xmm8, xmm9
	paddusb xmm10, xmm11
	paddusb xmm12, xmm13
	paddusb xmm14, xmm15
	paddusb xmm0, xmm2
	paddusb xmm8, xmm10
	paddusb xmm12, xmm14
	paddusb xmm0, xmm8
	paddusb xmm12, xmm0 ; xmm12 = 1 + 2 + 3 + 5 + 6 + 7 + 9 + 10 + 11 + 13 + 14 + 15	
	; Compute flags 0, 4, 8, 12
	movdqa xmm5, [sym(k254_u8)]
	movdqa xmm4, [rsp + 8] ; xmmNMinusOne
	movdqa xmm0, xmm5
	movdqa xmm1, xmm5
	movdqa xmm2, xmm5
	movdqa xmm3, xmm5
	pandn xmm0, [rsp + 24 + 0*16]
	pandn xmm1, [rsp + 24 + 4*16]
	pandn xmm2, [rsp + 24 + 8*16]
	pandn xmm3, [rsp + 24 + 12*16]
	paddusb xmm0, xmm1
	paddusb xmm2, xmm3
	paddusb xmm0, xmm2 ; xmm0 = 0 + 4 + 8 + 12
	paddusb xmm0, xmm12 ; xmm0 += 1 + 2 + 3 + 5 + 6 + 7 + 9 + 10 + 11 + 13 + 14 + 15
	; Check the columns with at least N non-zero bits
	pcmpgtb xmm0, xmm4
	pmovmskb rdx, xmm0
	test rdx, rdx
	jz .EndOfDarkers
	; Continue loading darkers
	mov rcx, arg(10) ; rd
	mov [rcx + rsi*COMPV_YASM_REG_SZ_BYTES], rdx ; (*rd) = colDarkersFlags
	; Transpose
	COMPV_TRANSPOSE_I8_16X16_REG_T9_X64_SSE2 rax+0*16, rax+1*16, rax+2*16, rax+3*16, rax+4*16, rax+5*16, rax+6*16, rax+7*16, rax+8*16, rax+9*16, rax+10*16, rax+11*16, rax+12*16, rax+13*16, rax+14*16, rax+15*16, xmm0, xmm1, xmm2, xmm3, xmm4, xmm8, xmm9, xmm10, xmm11
	; Flags
	mov r8, rsi
	mov r9, rsi
	shl r8, COMPV_YASM_REG_SZ_BYTES_TIMES16_LOG2 ; rsi * (16*COMPV_YASM_REG_SZ_BYTES)
	shl r9, 8 ; rsi * (16 * 16)
	pcmpeqb xmm5, xmm5 ; xmmFF
	mov rdx, arg(6) ; pfdarkers16
	lea rdx, [rdx + r8]
	mov rax, arg(8) ; ddarkers16x16
	lea rax, [rax + r9]
	pxor xmm0, xmm0
	pxor xmm1, xmm1
	pxor xmm2, xmm2
	pxor xmm3, xmm3
	pxor xmm4, xmm4
	pxor xmm8, xmm8
	pxor xmm9, xmm9
	pxor xmm10, xmm10
	pxor xmm11, xmm11
	pxor xmm12, xmm12
	pxor xmm13, xmm13
	pxor xmm14, xmm14
	pxor xmm15, xmm15
	pcmpeqb xmm0, [rax+0*16]
	pcmpeqb xmm1, [rax+1*16]
	pcmpeqb xmm2, [rax+2*16]
	pcmpeqb xmm3, [rax+3*16]
	pcmpeqb xmm4, [rax+4*16]
	pcmpeqb xmm8, [rax+5*16]
	pcmpeqb xmm9, [rax+6*16]
	pcmpeqb xmm10, [rax+7*16]
	pcmpeqb xmm11, [rax+8*16]
	pcmpeqb xmm12, [rax+9*16]
	pcmpeqb xmm13, [rax+10*16]
	pcmpeqb xmm14, [rax+11*16]
	pcmpeqb xmm15, [rax+12*16]
	pandn xmm0, xmm5
	pandn xmm1, xmm5
	pandn xmm2, xmm5
	pandn xmm3, xmm5
	pandn xmm4, xmm5
	pandn xmm8, xmm5
	pandn xmm9, xmm5
	pandn xmm10, xmm5
	pandn xmm11, xmm5
	pandn xmm12, xmm5
	pandn xmm13, xmm5
	pandn xmm14, xmm5
	pandn xmm15, xmm5
	push rax
	pmovmskb rax, xmm0
	pmovmskb rcx, xmm1
	pmovmskb r8, xmm2
	pmovmskb r9, xmm3
	pmovmskb r10, xmm4
	pmovmskb r11, xmm8
	pmovmskb r12, xmm9
	pmovmskb r13, xmm10
	pmovmskb r14, xmm11
	pmovmskb r15, xmm12
	mov [rdx + 0*COMPV_YASM_REG_SZ_BYTES], rax
	mov [rdx + 1*COMPV_YASM_REG_SZ_BYTES], rcx
	mov [rdx + 2*COMPV_YASM_REG_SZ_BYTES], r8
	mov [rdx + 3*COMPV_YASM_REG_SZ_BYTES], r9
	mov [rdx + 4*COMPV_YASM_REG_SZ_BYTES], r10
	mov [rdx + 5*COMPV_YASM_REG_SZ_BYTES], r11
	mov [rdx + 6*COMPV_YASM_REG_SZ_BYTES], r12
	mov [rdx + 7*COMPV_YASM_REG_SZ_BYTES], r13
	mov [rdx + 8*COMPV_YASM_REG_SZ_BYTES], r14
	mov [rdx + 9*COMPV_YASM_REG_SZ_BYTES], r15
	pmovmskb r8, xmm13
	pmovmskb r9, xmm14
	pmovmskb r10, xmm15
	mov [rdx + 10*COMPV_YASM_REG_SZ_BYTES], r8
	mov [rdx + 11*COMPV_YASM_REG_SZ_BYTES], r9
	mov [rdx + 12*COMPV_YASM_REG_SZ_BYTES], r10
	pop rax ; mov rax, arg(8) -> ddarkers16x16
	pxor xmm0, xmm0
	pxor xmm1, xmm1
	pxor xmm2, xmm2
	pcmpeqb xmm0, [rax+13*16]
	pcmpeqb xmm1, [rax+14*16]
	pcmpeqb xmm2, [rax+15*16]
	pandn xmm0, xmm5
	pandn xmm1, xmm5
	pandn xmm2, xmm5
	pmovmskb r8, xmm0
	pmovmskb r9, xmm1
	pmovmskb r10, xmm2
	mov [rdx + 13*COMPV_YASM_REG_SZ_BYTES], r8
	mov [rdx + 14*COMPV_YASM_REG_SZ_BYTES], r9
	mov [rdx + 15*COMPV_YASM_REG_SZ_BYTES], r10
		
	.EndOfDarkers
	

	;
	;	LoadBrighters
	;
	test rdi, rdi ; rdi was loadB, now it's free
	jz .EndOfBrighters
	; compute Dbrighters
	mov r9, rsi
	pxor xmm5, xmm5
	shl r9, 8 ; rsi*(16*16)
	movdqa xmm4, [sym(k1_i8)]
	mov rax, arg(9) ; dbrighters16x16
	lea rax, [rax + r9]
	movdqa xmm0, [rsp + 536 + 1*16]
	movdqa xmm1, [rsp + 536 + 2*16]
	movdqa xmm2, [rsp + 536 + 3*16]
	movdqa xmm3, [rsp + 536 + 5*16]
	movdqa xmm8, [rsp + 536 + 6*16]
	movdqa xmm9, [rsp + 536 + 7*16]
	movdqa xmm10, [rsp + 536 + 9*16]
	movdqa xmm11, [rsp + 536 + 10*16]
	movdqa xmm12, [rsp + 536 + 11*16]
	movdqa xmm13, [rsp + 536 + 13*16]
	movdqa xmm14, [rsp + 536 + 14*16]
	movdqa xmm15, [rsp + 536 + 15*16]
	psubusb xmm0, xmm6
	psubusb xmm1, xmm6
	psubusb xmm2, xmm6
	psubusb xmm3, xmm6
	psubusb xmm8, xmm6
	psubusb xmm9, xmm6
	psubusb xmm10, xmm6
	psubusb xmm11, xmm6
	psubusb xmm12, xmm6
	psubusb xmm13, xmm6
	psubusb xmm14, xmm6
	psubusb xmm15, xmm6
	movdqa [rax + 1*16], xmm0
	movdqa [rax + 2*16], xmm1
	movdqa [rax + 3*16], xmm2
	movdqa [rax + 5*16], xmm3
	movdqa [rax + 6*16], xmm8
	movdqa [rax + 7*16], xmm9
	movdqa [rax + 9*16], xmm10
	movdqa [rax + 10*16], xmm11
	movdqa [rax + 11*16], xmm12
	movdqa [rax + 13*16], xmm13
	movdqa [rax + 14*16], xmm14
	movdqa [rax + 15*16], xmm15
	pcmpeqb xmm0, xmm5
	pcmpeqb xmm1, xmm5
	pcmpeqb xmm2, xmm5
	pcmpeqb xmm3, xmm5
	pcmpeqb xmm8, xmm5
	pcmpeqb xmm9, xmm5
	pcmpeqb xmm10, xmm5
	pcmpeqb xmm11, xmm5
	pcmpeqb xmm12, xmm5
	pcmpeqb xmm13, xmm5
	pcmpeqb xmm14, xmm5
	pcmpeqb xmm15, xmm5
	pandn xmm0, xmm4
	pandn xmm1, xmm4
	pandn xmm2, xmm4
	pandn xmm3, xmm4
	pandn xmm8, xmm4
	pandn xmm9, xmm4
	pandn xmm10, xmm4
	pandn xmm11, xmm4
	pandn xmm12, xmm4
	pandn xmm13, xmm4
	pandn xmm14, xmm4
	pandn xmm15, xmm4
	paddusb xmm0, xmm1
	paddusb xmm2, xmm3
	paddusb xmm8, xmm9
	paddusb xmm10, xmm11
	paddusb xmm12, xmm13
	paddusb xmm14, xmm15
	paddusb xmm0, xmm2
	paddusb xmm8, xmm10
	paddusb xmm12, xmm14
	paddusb xmm0, xmm8
	paddusb xmm12, xmm0 ; xmm12 = 1 + 2 + 3 + 5 + 6 + 7 + 9 + 10 + 11 + 13 + 14 + 15
	; Compute flags 0, 4, 8, 12
	movdqa xmm6, [sym(k254_u8)]
	movdqa xmm4, [rsp + 8] ; xmmNMinusOne
	movdqa xmm0, xmm6
	movdqa xmm1, xmm6
	movdqa xmm2, xmm6
	movdqa xmm3, xmm6
	pandn xmm0, [rsp + 280 + 0*16]
	pandn xmm1, [rsp + 280 + 4*16]
	pandn xmm2, [rsp + 280 + 8*16]
	pandn xmm3, [rsp + 280 + 12*16]
	paddusb xmm0, xmm1
	paddusb xmm2, xmm3
	paddusb xmm0, xmm2 ; xmm0 = 0 + 4 + 8 + 12
	paddusb xmm0, xmm12 ; xmm0 += 1 + 2 + 3 + 5 + 6 + 7 + 9 + 10 + 11 + 13 + 14 + 15
	; Check the columns with at least N non-zero bits
	pcmpgtb xmm0, xmm4
	pmovmskb rdx, xmm0
	test rdx, rdx
	jz .EndOfBrighters
	; Continue loading brighters
	mov rcx, arg(11) ; rb
	mov [rcx + rsi*COMPV_YASM_REG_SZ_BYTES], rdx ; (*rb) = colBrightersFlags
	; Transpose
	COMPV_TRANSPOSE_I8_16X16_REG_T9_X64_SSE2 rax+0*16, rax+1*16, rax+2*16, rax+3*16, rax+4*16, rax+5*16, rax+6*16, rax+7*16, rax+8*16, rax+9*16, rax+10*16, rax+11*16, rax+12*16, rax+13*16, rax+14*16, rax+15*16, xmm0, xmm1, xmm2, xmm3, xmm4, xmm8, xmm9, xmm10, xmm11
	; Flags
	mov r8, rsi
	pcmpeqb xmm6, xmm6 ; xmmFF
	shl r8, COMPV_YASM_REG_SZ_BYTES_TIMES16_LOG2 ; rsi * (16*COMPV_YASM_REG_SZ_BYTES)
	mov rdx, arg(7) ; pfbrighters16
	lea rdx, [rdx + r8]
	pxor xmm0, xmm0
	pxor xmm1, xmm1
	pxor xmm2, xmm2
	pxor xmm3, xmm3
	pxor xmm4, xmm4
	pxor xmm5, xmm5
	pxor xmm8, xmm8
	pxor xmm9, xmm9
	pxor xmm10, xmm10
	pxor xmm11, xmm11
	pxor xmm12, xmm12
	pxor xmm13, xmm13
	pxor xmm14, xmm14
	pxor xmm15, xmm15
	pcmpeqb xmm0, [rax+0*16]
	pcmpeqb xmm1, [rax+1*16]
	pcmpeqb xmm2, [rax+2*16]
	pcmpeqb xmm3, [rax+3*16]
	pcmpeqb xmm4, [rax+4*16]
	pcmpeqb xmm5, [rax+5*16]
	pcmpeqb xmm8, [rax+6*16]
	pcmpeqb xmm9, [rax+7*16]
	pcmpeqb xmm10, [rax+8*16]
	pcmpeqb xmm11, [rax+9*16]
	pcmpeqb xmm12, [rax+10*16]
	pcmpeqb xmm13, [rax+11*16]
	pcmpeqb xmm14, [rax+12*16]
	pcmpeqb xmm15, [rax+13*16]
	pandn xmm0, xmm6
	pandn xmm1, xmm6
	pandn xmm2, xmm6
	pandn xmm3, xmm6
	pandn xmm4, xmm6
	pandn xmm5, xmm6
	pandn xmm8, xmm6
	pandn xmm9, xmm6
	pandn xmm10, xmm6
	pandn xmm11, xmm6
	pandn xmm12, xmm6
	pandn xmm13, xmm6
	pandn xmm14, xmm6
	pandn xmm15, xmm6
	pmovmskb rdi, xmm0
	pmovmskb rcx, xmm1
	pmovmskb r8, xmm2
	pmovmskb r9, xmm3
	pmovmskb r10, xmm4
	pmovmskb r11, xmm5
	pmovmskb r12, xmm8
	pmovmskb r13, xmm9
	pmovmskb r14, xmm10
	pmovmskb r15, xmm11
	mov [rdx + 0*COMPV_YASM_REG_SZ_BYTES], rdi
	mov [rdx + 1*COMPV_YASM_REG_SZ_BYTES], rcx
	mov [rdx + 2*COMPV_YASM_REG_SZ_BYTES], r8
	mov [rdx + 3*COMPV_YASM_REG_SZ_BYTES], r9
	mov [rdx + 4*COMPV_YASM_REG_SZ_BYTES], r10
	mov [rdx + 5*COMPV_YASM_REG_SZ_BYTES], r11
	mov [rdx + 6*COMPV_YASM_REG_SZ_BYTES], r12
	mov [rdx + 7*COMPV_YASM_REG_SZ_BYTES], r13
	mov [rdx + 8*COMPV_YASM_REG_SZ_BYTES], r14
	mov [rdx + 9*COMPV_YASM_REG_SZ_BYTES], r15
	pmovmskb r8, xmm12
	pmovmskb r9, xmm13
	pmovmskb r10, xmm14
	pmovmskb r11, xmm15
	mov [rdx + 10*COMPV_YASM_REG_SZ_BYTES], r8
	mov [rdx + 11*COMPV_YASM_REG_SZ_BYTES], r9
	mov [rdx + 12*COMPV_YASM_REG_SZ_BYTES], r10
	mov [rdx + 13*COMPV_YASM_REG_SZ_BYTES], r11
	pxor xmm0, xmm0
	pxor xmm1, xmm1
	pcmpeqb xmm0, [rax+14*16]
	pcmpeqb xmm1, [rax+15*16]
	pandn xmm0, xmm6
	pandn xmm1, xmm6
	pmovmskb rdi, xmm0
	pmovmskb rcx, xmm1
	mov [rdx + 14*COMPV_YASM_REG_SZ_BYTES], rdi
	mov [rdx + 15*COMPV_YASM_REG_SZ_BYTES], rcx

	.EndOfBrighters

	
	.LoopRowsNext
	lea rbx, [rbx + 16] ; IP += 16
	
	;-------------------
	;EndOfLooopRows
	lea rsi, [rsi + 1]
	cmp rsi, arg(2)
	jl .LoopRows
	;-------------------

	; unalign stack and free memory
	add rsp, 8 + 16 + 16*16 + 16*16 + 16*16
	COMPV_YASM_UNALIGN_STACK

	; begin epilog
	pop r15
	pop r14
	pop r13
	pop r12
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
	

%endif ; COMPV_YASM_ABI_IS_64BIT

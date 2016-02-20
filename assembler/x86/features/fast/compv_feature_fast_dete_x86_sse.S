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

global sym(Fast9Strengths16_Asm_CMOV_X86_SSE41)
global sym(Fast9Strengths16_Asm_X86_SSE41)
global sym(Fast12Strengths16_Asm_CMOV_X86_SSE41)
global sym(Fast12Strengths16_Asm_X86_SSE41)

global sym(FastData_Asm_CMOV_POPCNT_x86_SSE2)
global sym(FastData16Row_Asm_X86_SSE2)

section .data
	extern sym(kFast9Arcs)
	extern sym(kFast12Arcs)
	extern sym(Fast9Flags)
	extern sym(Fast12Flags)
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
	sub rsp, 8 + 16 + 16*16 + 16*16 + 16*16
	; [rsp + 0] = sum
	; [rsp + 8] = xmmNMinusOne
	; [rsp + 24] = xmmDarkersFlags[16]
	; [rsp + 280] = xmmBrightersFlags[16]
	; [rsp + 536] = xmmDataPtr[16]

	mov rsi, arg(2) ; rsi = width
	mov rax, arg(5) ; threshold
	mov rbx, arg(0) ; rbx = IP
	movd xmm7, eax
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
	mov [rax], rcx
	mov [rdx], rcx

	movdqu xmm6, [rbx]

	; Motion Estimation
	; TODO(dmi): not supported
	; TODO(dmi): inc IPprev here

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
	pxor xmm4, xmm4 ; xmm4 = xmmZeros
	movdqu xmm0, [rbx + rax] ; IP[pixels16[0]]
	movdqu xmm1, [rbx + rdx] ; IP[pixels16[8]]
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
	movdqa [rsp + 24 + 0*16], xmm2 ; xmmDarkersFlags[0]
	movdqa [rsp + 24 + 8*16], xmm3 ; xmmDarkersFlags[8]
	movdqa [rsp + 280 + 0*16], xmm0 ; xmmBrightersFlags[0]
	movdqa [rsp + 280 + 8*16], xmm1 ; xmmBrightersFlags[8]
	por xmm0, xmm2
	por xmm1, xmm3
	pmovmskb eax, xmm0
	pmovmskb edx, xmm1
	test ax, ax
	setnz al
	test dx, dx
	setnz dl
	add dl, al
	test dl, dl
	jz .LoopRowsNext
	mov [rsp + 0], dl ; sum = ?

	; compare I5 and I13 aka 4 and 12
	mov rdx, arg(3) ; pixels16
	mov rax, [rdx + 4*COMPV_YASM_REG_SZ_BYTES] ; pixels16[4]
	mov rdx, [rdx + 12*COMPV_YASM_REG_SZ_BYTES] ; pixels16[12]
	pxor xmm4, xmm4 ; xmm4 = xmmZeros
	movdqu xmm0, [rbx + rax] ; IP[pixels16[4]]
	movdqu xmm1, [rbx + rdx] ; IP[pixels16[12]]
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
	movdqa [rsp + 24 + 4*16], xmm2 ; xmmDarkersFlags[4]
	movdqa [rsp + 24 + 12*16], xmm3 ; xmmDarkersFlags[12]
	movdqa [rsp + 280 + 4*16], xmm0 ; xmmBrightersFlags[4]
	movdqa [rsp + 280 + 12*16], xmm1 ; xmmBrightersFlags[12]
	por xmm0, xmm2
	por xmm1, xmm3
	pmovmskb eax, xmm0
	pmovmskb edx, xmm1
	test ax, ax
	setnz al
	test dx, dx
	setnz dl
	add dl, al
	test dl, dl
	jz .LoopRowsNext
	add [rsp + 0], dl ; sum += ?

	;
	;  Speed-Test-2
	;
	
	mov cl, arg(4) ; N
	mov al, [rsp + 0] ; sum
	cmp cl, 9
	je .SpeedTest2For9
	; otherwise ...N == 12
	cmp al, 3
	jl .LoopRowsNext
	jmp .EndOfSpeedTest2

	.SpeedTest2For9
	cmp al, 2
	jl .LoopRowsNext
	
	.EndOfSpeedTest2

	;
	;	Processing
	;

	; Check whether to load Brighters
	movdqa xmm0, [rsp + 280 + 0*16] ; xmmBrightersFlags[0]
	movdqa xmm1, [rsp + 280 + 4*16] ; xmmBrightersFlags[4]
	por xmm0, [rsp + 280 + 8*16] ; xmmBrightersFlags[0] | xmmBrightersFlags[8]
	por xmm1, [rsp + 280 + 12*16] ; xmmBrightersFlags[4] | xmmBrightersFlags[12]
	pmovmskb eax, xmm0
	pmovmskb edx, xmm1
	test ax, ax
	setnz al
	test dx, dx
	setnz dl
	add dl, al
	cmp dl, 1
	setg dl
	movzx rdi, byte dl ; rdi = (rdx > 1) ? 1 : 0

	; Check whether to load Darkers
	movdqa xmm0, [rsp + 24 + 0*16] ; xmmDarkersFlags[0]
	movdqa xmm1, [rsp + 24 + 4*16] ; xmmDarkersFlags[4]
	por xmm0, [rsp + 24 + 8*16] ; xmmDarkersFlags[0] | xmmDarkersFlags[8]
	por xmm1, [rsp + 24 + 12*16] ; xmmDarkersFlags[4] | xmmDarkersFlags[12]
	pmovmskb eax, xmm0
	pmovmskb edx, xmm1
	test ax, ax
	setnz al
	test dx, dx
	setnz dl
	add dl, al
	cmp dl, 1
	setg dl ; rdx = (rdx > 1) ? 1 : 0

	; rdi = loadB, rdx = loadD
	; skip process if (!(loadB || loadD))
	mov rax, rdi
	or al, dl
	test al, al
	jz .LoopRowsNext

	; Load xmmDataPtr
	mov rcx, arg(3) ; pixels16
	mov rax, [rcx + 1*COMPV_YASM_REG_SZ_BYTES] ; pixels16[1]
	movdqu xmm0, [rbx + rax]
	mov rax, [rcx + 2*COMPV_YASM_REG_SZ_BYTES] ; pixels16[2]
	movdqu xmm1, [rbx + rax]
	mov rax, [rcx + 3*COMPV_YASM_REG_SZ_BYTES] ; pixels16[3]
	movdqu xmm2, [rbx + rax]
	mov rax, [rcx + 5*COMPV_YASM_REG_SZ_BYTES] ; pixels16[5]
	movdqu xmm3, [rbx + rax]
	mov rax, [rcx + 6*COMPV_YASM_REG_SZ_BYTES] ; pixels16[6]
	movdqu xmm4, [rbx + rax]
	movdqa [rsp + 536 + 1*16], xmm0
	movdqa [rsp + 536 + 2*16], xmm1
	movdqa [rsp + 536 + 3*16], xmm2
	movdqa [rsp + 536 + 5*16], xmm3
	movdqa [rsp + 536 + 6*16], xmm4
	mov rax, [rcx + 7*COMPV_YASM_REG_SZ_BYTES] ; pixels16[7]
	movdqu xmm0, [rbx + rax]
	mov rax, [rcx + 9*COMPV_YASM_REG_SZ_BYTES] ; pixels16[9]
	movdqu xmm1, [rbx + rax]
	mov rax, [rcx + 10*COMPV_YASM_REG_SZ_BYTES] ; pixels16[10]
	movdqu xmm2, [rbx + rax]
	mov rax, [rcx + 11*COMPV_YASM_REG_SZ_BYTES] ; pixels16[11]
	movdqu xmm3, [rbx + rax]
	mov rax, [rcx + 13*COMPV_YASM_REG_SZ_BYTES] ; pixels16[13]
	movdqu xmm4, [rbx + rax]
	movdqa [rsp + 536 + 7*16], xmm0
	movdqa [rsp + 536 + 9*16], xmm1
	movdqa [rsp + 536 + 10*16], xmm2
	movdqa [rsp + 536 + 11*16], xmm3
	movdqa [rsp + 536 + 13*16], xmm4
	mov rax, [rcx + 14*COMPV_YASM_REG_SZ_BYTES] ; pixels16[14]
	movdqu xmm0, [rbx + rax]
	mov rax, [rcx + 15*COMPV_YASM_REG_SZ_BYTES] ; pixels16[15]
	movdqu xmm1, [rbx + rax]
	movdqa [rsp + 536 + 14*16], xmm0
	movdqa [rsp + 536 + 15*16], xmm1

	; We could compute pixels at 1 and 9, check if at least one is darker or brighter than the candidate
	; Then, do the same for 2 and 10 etc etc ... but this is slower than whant we're doing below because
	; _mm_movemask_epi8 is cyclyvore

	;
	;	LoadDarkers
	;
	test dl, dl ; rdx was loadD, now it's free
	jz .EndOfDarkers
	; compute ddarkers16x16 and flags
	mov rax, arg(8) ; ddarkers16x16
	pxor xmm4, xmm4
	movdqa xmm0, xmm5
	movdqa xmm1, xmm5
	movdqa xmm2, xmm5
	movdqa xmm3, xmm5
	psubusb xmm0, [rsp + 536 + 1*16]
	psubusb xmm1, [rsp + 536 + 2*16]
	psubusb xmm2, [rsp + 536 + 3*16]
	psubusb xmm3, [rsp + 536 + 5*16]
	movdqa [rax + 1*16], xmm0
	movdqa [rax + 2*16], xmm1
	movdqa [rax + 3*16], xmm2
	movdqa [rax + 5*16], xmm3
	pcmpeqb xmm0, xmm4
	pcmpeqb xmm1, xmm4
	pcmpeqb xmm2, xmm4
	pcmpeqb xmm3, xmm4
	movdqa xmm4, [sym(k1_i8)]
	pandn xmm0, xmm4
	pandn xmm1, xmm4
	pandn xmm2, xmm4
	pandn xmm3, xmm4
	paddusb xmm0, xmm1
	paddusb xmm2, xmm3
	paddusb xmm0, xmm2
	movdqa [rsp + 24 + 1*16], xmm0 ; xmmDarkersFlags[1] = 1 + 2 + 3 + 5
	pxor xmm4, xmm4
	movdqa xmm0, xmm5
	movdqa xmm1, xmm5
	movdqa xmm2, xmm5
	movdqa xmm3, xmm5
	psubusb xmm0, [rsp + 536 + 6*16]
	psubusb xmm1, [rsp + 536 + 7*16]
	psubusb xmm2, [rsp + 536 + 9*16]
	psubusb xmm3, [rsp + 536 + 10*16]
	movdqa [rax + 6*16], xmm0
	movdqa [rax + 7*16], xmm1
	movdqa [rax + 9*16], xmm2
	movdqa [rax + 10*16], xmm3
	pcmpeqb xmm0, xmm4
	pcmpeqb xmm1, xmm4
	pcmpeqb xmm2, xmm4
	pcmpeqb xmm3, xmm4
	movdqa xmm4, [sym(k1_i8)]
	pandn xmm0, xmm4
	pandn xmm1, xmm4
	pandn xmm2, xmm4
	pandn xmm3, xmm4
	paddusb xmm0, xmm1
	paddusb xmm2, xmm3
	paddusb xmm0, xmm2
	movdqa [rsp + 24 + 6*16], xmm0 ; xmmDarkersFlags[6] = 6 + 7 + 9 + 10
	pxor xmm4, xmm4
	movdqa xmm0, xmm5
	movdqa xmm1, xmm5
	movdqa xmm2, xmm5
	movdqa xmm3, xmm5
	psubusb xmm0, [rsp + 536 + 11*16]
	psubusb xmm1, [rsp + 536 + 13*16]
	psubusb xmm2, [rsp + 536 + 14*16]
	psubusb xmm3, [rsp + 536 + 15*16]
	movdqa [rax + 11*16], xmm0
	movdqa [rax + 13*16], xmm1
	movdqa [rax + 14*16], xmm2
	movdqa [rax + 15*16], xmm3
	pcmpeqb xmm0, xmm4
	pcmpeqb xmm1, xmm4
	pcmpeqb xmm2, xmm4
	pcmpeqb xmm3, xmm4
	movdqa xmm4, [sym(k1_i8)]
	pandn xmm0, xmm4
	pandn xmm1, xmm4
	pandn xmm2, xmm4
	pandn xmm3, xmm4
	paddusb xmm0, xmm1
	paddusb xmm2, xmm3
	paddusb xmm0, xmm2
	movdqa [rsp + 24 + 11*16], xmm0 ; xmmDarkersFlags[11] = 11 + 13 + 14 + 15
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
	paddusb xmm0, [rsp + 24 + 1*16] ; xmm0 += 1 + 2 + 3 + 5
	paddusb xmm0, [rsp + 24 + 6*16] ; xmm0 += 6 + 7 + 9 + 10
	paddusb xmm0, [rsp + 24 + 11*16] ; xmm0 += 11 + 13 + 14 + 15
	; Check the columns with at least N non-zero bits
	pcmpgtb xmm0, xmm4
	pmovmskb edx, xmm0
	test dx, dx
	jz .EndOfDarkers
	; Continue loading darkers
	mov rcx, arg(10) ; rd
	mov [rcx], dx ; (*rd) = colDarkersFlags
	; Transpose
	COMPV_TRANSPOSE_I8_16X16_REG_T5_SSE2 rax+0*16, rax+1*16, rax+2*16, rax+3*16, rax+4*16, rax+5*16, rax+6*16, rax+7*16, rax+8*16, rax+9*16, rax+10*16, rax+11*16, rax+12*16, rax+13*16, rax+14*16, rax+15*16, xmm0, xmm1, xmm2, xmm3, xmm4
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
		pmovmskb eax, xmm0
		pmovmskb ecx, xmm1
		mov [rdx + (0+i)*COMPV_YASM_REG_SZ_BYTES], ax
		mov [rdx + (1+i)*COMPV_YASM_REG_SZ_BYTES], cx
		pmovmskb eax, xmm2
		pmovmskb ecx, xmm3
		mov [rdx + (2+i)*COMPV_YASM_REG_SZ_BYTES], ax
		mov [rdx + (3+i)*COMPV_YASM_REG_SZ_BYTES], cx
		%assign i i+4
	%endrep
	
	.EndOfDarkers
	

	;
	;	LoadBrighters
	;
	test rdi, rdi ; rdi was loadB, now it's free
	jz .EndOfBrighters
	; compute Dbrighters
	pxor xmm5, xmm5
	mov rax, arg(9) ; dbrighters16x16
	movdqa xmm0, [rsp + 536 + 1*16]
	movdqa xmm1, [rsp + 536 + 2*16]
	movdqa xmm2, [rsp + 536 + 3*16]
	movdqa xmm3, [rsp + 536 + 5*16]
	movdqa xmm4, [rsp + 536 + 6*16]
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
	movdqa [rsp + 280 + 1*16], xmm0 ; xmmBrightersFlags[1] = 1 + 2 + 3 + 5 + 6
	pxor xmm5, xmm5
	movdqa xmm0, [rsp + 536 + 7*16]
	movdqa xmm1, [rsp + 536 + 9*16]
	movdqa xmm2, [rsp + 536 + 10*16]
	movdqa xmm3, [rsp + 536 + 11*16]
	movdqa xmm4, [rsp + 536 + 13*16]
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
	movdqa [rsp + 280 + 7*16], xmm0 ; xmmBrightersFlags[7] = 7 + 9 + 10 + 11 + 13
	pxor xmm5, xmm5
	movdqa xmm4, [sym(k1_i8)]
	movdqa xmm0, [rsp + 536 + 14*16]
	movdqa xmm1, [rsp + 536 + 15*16]
	psubusb xmm0, xmm6
	psubusb xmm1, xmm6
	movdqa [rax + 14*16], xmm0
	movdqa [rax + 15*16], xmm1
	pcmpeqb xmm0, xmm5
	pcmpeqb xmm1, xmm5
	pandn xmm0, xmm4
	pandn xmm1, xmm4
	paddusb xmm0, xmm1
	movdqa [rsp + 280 + 14*16], xmm0 ; xmmBrightersFlags[14] = 14 + 15	
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
	paddusb xmm0, [rsp + 280 + 1*16] ; xmm0 += 1 + 2 + 3 + 5 + 6
	paddusb xmm0, [rsp + 280 + 7*16] ; xmm0 += 7 + 9 + 10 + 11 + 13
	paddusb xmm0, [rsp + 280 + 14*16] ; xmm0 += 14 + 15
	; Check the columns with at least N non-zero bits
	pcmpgtb xmm0, xmm4
	pmovmskb edx, xmm0
	test dx, dx
	jz .EndOfBrighters
	; Continue loading brighters
	mov rcx, arg(11) ; rb
	mov rax, arg(9) ; dbrighters16x16
	mov [rcx], dx ; (*rb) = colBrightersFlags
	; Transpose
	COMPV_TRANSPOSE_I8_16X16_REG_T5_SSE2 rax+0*16, rax+1*16, rax+2*16, rax+3*16, rax+4*16, rax+5*16, rax+6*16, rax+7*16, rax+8*16, rax+9*16, rax+10*16, rax+11*16, rax+12*16, rax+13*16, rax+14*16, rax+15*16, xmm0, xmm1, xmm2, xmm3, xmm4
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
		pmovmskb edi, xmm0
		pmovmskb ecx, xmm1
		mov [rdx + (0+i)*COMPV_YASM_REG_SZ_BYTES], di
		mov [rdx + (1+i)*COMPV_YASM_REG_SZ_BYTES], cx
		pmovmskb edi, xmm2
		pmovmskb ecx, xmm3
		mov [rdx + (2+i)*COMPV_YASM_REG_SZ_BYTES], di
		mov [rdx + (3+i)*COMPV_YASM_REG_SZ_BYTES], cx
		%assign i i+4
	%endrep

	.EndOfBrighters
	
	.LoopRowsNext

	; TODO(dmi): do the same as x64, increment these values only if needed
	lea rbx, [rbx + 16] ; IP += 16
	mov rdx, COMPV_YASM_REG_SZ_BYTES
	add arg(11), rdx ; rb += 1
	add arg(10), rdx ; rd += 1
	mov rdx, 16*COMPV_YASM_REG_SZ_BYTES
	add arg(6), rdx ; pfdarkers16 += 1
	add arg(7), rdx ; pfbrighters16 += 1
	mov rdx, 16*16
	add arg(8), rdx ; xmmDdarkers16x16 += 1
	add arg(9), rdx ; xmmDbrighters16x16 += 1
	; TODO(dmi): Motion estimation not supported -> do not inc IPprev

	;-------------------
	;EndOfLooopRows
	lea rsi, [rsi - 16]
	test rsi, rsi
	jnz .LoopRows
	;-------------------

	; unalign stack and free memory
	add rsp, 8 + 16 + 16*16 + 16*16 + 16*16
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
; arg(0) -> compv_scalar_t rbrighters
; arg(1) -> compv_scalar_t rdarkers
; arg(2) -> COMPV_ALIGNED(SSE) const uint8_t* dbrightersx1616
; arg(3) -> COMPV_ALIGNED(SSE) const uint8_t* ddarkers16x16
; arg(4) -> const compv_scalar_t(*fbrighters16)[16]
; arg(5) -> const compv_scalar_t(*fdarkers16)[16]
; arg(6) -> uint8_t* strengths16
; arg(7) -> compv_scalar_t N
; %1 -> 1: CMOV is supported, 0 CMOV not supported
; %2 -> 9: Use FAST9, 12: FAST12 ....
%macro FastStrengths16_Asm_X86_SSE41 2
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_XMM 7 ;XMM[6-n]
	push rsi
	push rdi
	push rbx
	; end prolog

	; alloc memory
	sub rsp, 8+8
	; [rsp + 0] = (1 << p)
	; [rsp + 8] = (p)

	pxor xmm0, xmm0

	; FAST hard-coded flags
	%if %2 == 9
		movdqa xmm7, [sym(Fast9Flags) + 0] ; xmmFastXFlagsLow
		movdqa xmm6, [sym(Fast9Flags) + 16]; xmm6 = xmmFastXFlagsHigh
	%elif %2 == 12
		movdqa xmm7, [sym(Fast12Flags) + 0] ; xmmFastXFlagsLow
		movdqa xmm6, [sym(Fast12Flags) + 16]; xmm6 = xmmFastXFlagsHigh
	%else
		%error "not supported"
	%endif

	xor rcx, rcx
	mov rax, 1
	mov [rsp + 0], rax ; (1<<p) = 1
	mov [rsp + 8], rcx ; p = 0

	; Set strengths to zero
	mov rax, arg(6) ; strengths16
	movdqu [rax], xmm0

	;----------------------
	; Loop Start
	;----------------------
	.LoopStart
		xor rcx, rcx ; rcx = maxnbrighter
		xor rdx, rdx ; rdx = maxndarker

		; ---------
		; Brighters
		; ---------
		mov rsi, [rsp + 0] ; (1<<p)
		test arg(0), rsi ; (rbrighters & (1 << p)) ?
		jz .EndOfBrighters
		mov rsi, [rsp + 8] ; p
		mov rax, arg(4) ; &fbrighters16[p]
		mov rdi, [rax + rsi*COMPV_YASM_REG_SZ_BYTES] ; fbrighters16[p]

		movd xmm5, rdi
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
		pmovmskb eax, xmm5
		test ax, ax ; rax = r0
		jz .EndOfBrighters
		pxor xmm3, xmm3 ; xmm3 = Zeros
		; Load dbrighters
		mov rbx, arg(2) ; dbrightersx1616
		shl rsi, 4 ; p*16 
		movdqa xmm2, [rbx + rsi]
		; Compute minimum hz
		%assign i 0
		%rep    16
			test rax, 1<<i
			jz .EndOfBrightersMin %+ i
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
		.EndOfBrighters

		; ---------
		; Darkers
		; ---------
	.Darkers
		mov rsi, [rsp + 0] ; (1<<p)
		test arg(1), rsi ; (rdarkers & (1 << p)) ?
		jz .EndOfDarkers
		mov rsi, [rsp + 8] ; p
		mov rax, arg(5) ; &fdarkers16[p]
		mov rdi, [rax + rsi*COMPV_YASM_REG_SZ_BYTES] ; fdarkers16[p]

		movd xmm5, rdi
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
		pmovmskb eax, xmm5
		test ax, ax ; rax = r0
		jz .EndOfDarkers
		pxor xmm3, xmm3 ; xmm3 = Zeros
		; Load ddarkers16x16
		mov rbx, arg(3) ; ddarkers16x16
		shl rsi, 4 ; p*16 
		movdqa xmm2, [rbx + rsi]
		; Compute minimum hz
		%assign i 0
		%rep    16
			test rax, 1<<i
			jz .EndOfDarkersMin %+ i
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
		.EndOfDarkers
		
		; rax = strength = std::max(maxnbrighter, maxndarker);
		mov rax, rdx
		cmp rcx, rdx
		%if %1 == 1
			cmovg rax, rcx
		%else
			jl .NotMax
			mov rax, rcx
			.NotMax
		%endif

	mov rdx, [rsp + 8] ; p
	mov rcx, arg(6) ; &strengths16
	mov [rcx + rdx], byte al ; strengths16[p] = x
	
	mov rcx, 1
	add [rsp + 8], rcx ; p+= 1

	mov rax, [rsp + 0]
	shl rax, 1
	cmp rdx, 16
	mov [rsp + 0], rax
	jl .LoopStart
	;----------------

	; free memory
	add rsp, 8+8

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
;;; compv_scalar_t Fast9Strengths16_Asm_CMOV_X86_SSE41(COMPV_ALIGNED(SSE) const int16_t(&dbrighters)[16], COMPV_ALIGNED(SSE) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
sym(Fast9Strengths16_Asm_CMOV_X86_SSE41):
	FastStrengths16_Asm_X86_SSE41 1, 9

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t Fast9Strengths16_Asm_X86_SSE41(COMPV_ALIGNED(SSE) const int16_t(&dbrighters)[16], COMPV_ALIGNED(SSE) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
sym(Fast9Strengths16_Asm_X86_SSE41):
	FastStrengths16_Asm_X86_SSE41 0, 9

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t Fast12Strengths16_Asm_CMOV_X86_SSE41(COMPV_ALIGNED(SSE) const int16_t(&dbrighters)[16], COMPV_ALIGNED(SSE) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
sym(Fast12Strengths16_Asm_CMOV_X86_SSE41):
	FastStrengths16_Asm_X86_SSE41 1, 12

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t Fast12Strengths16_Asm_X86_SSE41(COMPV_ALIGNED(SSE) const int16_t(&dbrighters)[16], COMPV_ALIGNED(SSE) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
sym(Fast12Strengths16_Asm_X86_SSE41):
	FastStrengths16_Asm_X86_SSE41 0, 12

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
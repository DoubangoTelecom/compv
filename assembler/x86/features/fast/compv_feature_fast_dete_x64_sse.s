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
%include "../../compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT

%include "../../compv_bits_macros_x86.s"
%include "../../compv_math_macros_x86.s"
%include "compv_feature_fast_dete_macros_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(FastData16Row_Asm_X64_SSE2)

global sym(Fast9Strengths16_Asm_CMOV_X64_SSE41)
global sym(Fast9Strengths16_Asm_X64_SSE41)
global sym(Fast12Strengths16_Asm_CMOV_X64_SSE41)
global sym(Fast12Strengths16_Asm_X64_SSE41)

global sym(Fast9Strengths32_Asm_CMOV_X64_SSE41)
global sym(Fast9Strengths32_Asm_X64_SSE41)
global sym(Fast12Strengths32_Asm_CMOV_X64_SSE41)
global sym(Fast12Strengths32_Asm_X64_SSE41)

section .data
	extern sym(kFast9Arcs)
	extern sym(kFast12Arcs)
	extern sym(Fast9Flags)
	extern sym(Fast12Flags)
	extern sym(k1_i8)
	extern sym(k254_u8)
	extern sym(FastStrengths16) ; Function

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> cconst uint8_t* IP,
; arg(1) -> cconst uint8_t* IPprev,
; arg(2) -> ccompv_scalar_t width,
; arg(3) -> cconst compv_scalar_t(&pixels16)[16],
; arg(4) -> ccompv_scalar_t N,
; arg(5) -> ccompv_scalar_t threshold,
; arg(6) -> cuint8_t* strengths,
; arg(7) -> ccompv_scalar_t* me
; void FastData16Row_Asm_X86_SSE2(const uint8_t* IP, const uint8_t* IPprev, compv_scalar_t width, const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, uint8_t* strengths, compv_scalar_t* me);
sym(FastData16Row_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
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
	sub rsp, 8 + 8 + 8 + 8*16 + 8*16 + 16 + 16*16 + 16*16 + 16*16 + 16*16 + 16*16 + 16
	; [rsp + 0] = compv_scalar_t sum
	; [rsp + 8] = compv_scalar_t colDarkersFlags
	; [rsp + 16] = compv_scalar_t colBrightersFlags
	; [rsp + 24] = compv_scalar_t fdarkers16[16];
	; [rsp + 152] = compv_scalar_t fbrighters16[16];
	; [rsp + 280] = __m128i xmmNMinusOne
	; [rsp + 296] = __m128i xmmDarkersFlags[16]
	; [rsp + 552] = __m128i xmmBrightersFlags[16]
	; [rsp + 808] = __m128i xmmDataPtr[16]
	; [rsp + 1064] = __m128i xmmDdarkers16x16[16];
	; [rsp + 1320] = __m128i xmmDbrighters16x16[16];
	; [rsp + 1576] = __m128i xmmThreshold (saved/restored after function call)

	mov rdx, arg(2) ; width
	shr rdx, 4 ; div width with 16 and mov rsi by 1 -> because of argi(x,rsi)
	xor rsi, rsi
	mov arg(2), rdx 
	movzx rax, byte arg(5) ; threshold
	mov rbx, arg(0) ; rbx = IP
	movd xmm7, rax
	punpcklbw xmm7, xmm7  
	punpcklwd xmm7, xmm7  
	pshufd xmm7, xmm7, 0  ; xmm7 = _mm_set1_epi8((uint8_t)threshold)) = xmmThreshold

	; Compute xmmNMinusOne
	movzx rax, byte arg(4) ; N
	sub al, 1
	movd xmm0, eax
	punpcklbw xmm0, xmm0  
	punpcklwd xmm0, xmm0  
	pshufd xmm0, xmm0, 0
	movdqa [rsp + 280], xmm0
	
	;-------------------
	;StartOfLooopRows
	;
	.LoopRows
	; -------------------
	movdqu xmm6, [rbx]
	pxor xmm15, xmm15 ; xmm15 = xmmZeros
	pcmpeqb xmm14, xmm14 ; xmm14 = xmmFF

	; Motion Estimation
	; TODO(dmi): not supported

	; cleanup strengths
	mov rax, arg(6)
	movdqu [rax], xmm15

	movdqa xmm5, xmm6
	paddusb xmm6, xmm7 ; xmm6 = xmmBrighter
	psubusb xmm5, xmm7 ; xmm5 = xmmDarker

	;
	; Speed-Test-1
	;
	
	mov r11, arg(3) ; pixels16

	; compare I1 and I9 aka 0 and 8
	mov rax, [r11 + 0*COMPV_YASM_REG_SZ_BYTES] ; pixels16[0]
	mov rdx, [r11 + 8*COMPV_YASM_REG_SZ_BYTES] ; pixels16[8]
	movdqu xmm0, [rbx + rax]
	movdqa xmm2, xmm5 ; xmmDarker
	movdqu xmm1, [rbx + rdx]
	movdqa xmm3, xmm5 ; xmmDarker
	psubusb xmm2, xmm0 ; ddarkers16x16[0]
	psubusb xmm3, xmm1 ; ddarkers16x16[8]
	psubusb xmm0, xmm6 ; dbrighters16x16[0]
	psubusb xmm1, xmm6 ; dbrighters16x16[8]
	movdqa [rsp + 1064 + 0*16], xmm2
	movdqa [rsp + 1064 + 8*16], xmm3
	movdqa [rsp + 1320 + 0*16], xmm0
	movdqa [rsp + 1320 + 8*16], xmm1
	pcmpeqb xmm2, xmm15
	pcmpeqb xmm3, xmm15
	pcmpeqb xmm0, xmm15
	pcmpeqb xmm1, xmm15
	pandn xmm2, xmm14
	pandn xmm3, xmm14
	pandn xmm0, xmm14
	pandn xmm1, xmm14
	movdqa xmm8, xmm0 ; save xmmBrightersFlags[0] in xmm8
	movdqa xmm9, xmm1 ; save xmmBrightersFlags[8] in xmm9
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
	movdqa xmm10, xmm2 ; save xmmDarkersFlags[0] in xmm10
	movdqa xmm11, xmm3 ; save xmmDarkersFlags[8] in xmm11
	movdqa [rsp + 296 + 0*16], xmm2 ; xmmDarkersFlags[0]
	movdqa [rsp + 296 + 8*16], xmm3 ; xmmDarkersFlags[8]
	movdqa [rsp + 552 + 0*16], xmm8 ; xmmBrightersFlags[0]
	movdqa [rsp + 552 + 8*16], xmm9 ; xmmBrightersFlags[8]

	; compare I5 and I13 aka 4 and 12
	mov rax, [r11 + 4*COMPV_YASM_REG_SZ_BYTES] ; pixels16[4]
	mov rdx, [r11 + 12*COMPV_YASM_REG_SZ_BYTES] ; pixels16[12]
	movdqu xmm0, [rbx + rax]
	movdqa xmm2, xmm5 ; xmmDarker
	movdqu xmm1, [rbx + rdx]
	movdqa xmm3, xmm5 ; xmmDarker
	psubusb xmm2, xmm0 ; ddarkers16x16[4]
	psubusb xmm3, xmm1 ; ddarkers16x16[12]
	psubusb xmm0, xmm6 ; dbrighters16x16[4]
	psubusb xmm1, xmm6 ; dbrighters16x16[12]
	movdqa [rsp + 1064 + 4*16], xmm2
	movdqa [rsp + 1064 + 12*16], xmm3
	movdqa [rsp + 1320 + 4*16], xmm0
	movdqa [rsp + 1320 + 12*16], xmm1
	pcmpeqb xmm2, xmm15
	pcmpeqb xmm3, xmm15
	pcmpeqb xmm0, xmm15
	pcmpeqb xmm1, xmm15
	pandn xmm2, xmm14
	pandn xmm3, xmm14
	pandn xmm0, xmm14
	pandn xmm1, xmm14
	movdqa xmm12, xmm0 ; save xmmBrightersFlags[4] in xmm12
	movdqa xmm13, xmm1 ; save xmmBrightersFlags[12] in xmm13
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
	add [rsp + 0], dl ; sum = ?
	movdqa [rsp + 296 + 4*16], xmm2 ; xmmDarkersFlags[4]
	movdqa [rsp + 296 + 12*16], xmm3 ; xmmDarkersFlags[12]
	movdqa [rsp + 552 + 4*16], xmm12 ; xmmBrightersFlags[4]
	movdqa [rsp + 552 + 12*16], xmm13 ; xmmBrightersFlags[12]

	;
	;  Speed-Test-2
	;
	
	mov cl, byte arg(4) ; N
	mov al, byte [rsp + 0] ; sum
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
	
	; Data for Darkers (loadD) and Brighters (loadB)
	por xmm10, xmm11
	por xmm2, xmm3
	por xmm8, xmm9
	por xmm12, xmm13
	pmovmskb r9d, xmm10
	pmovmskb r10d, xmm2
	pmovmskb r11d, xmm8
	pmovmskb r12d, xmm12

	; Check whether to load Brighters
	test r11w, r11w
	setnz cl
	test r12w, r12w
	setnz al
	add cl, al
	cmp cl, 1
	setg dil ; rdi = (rcx > 1) ? 1 : 0
	
	; Check whether to load Darkers
	test r9w, r9w
	setnz cl
	test r10w, r10w
	setnz dl
	add cl, dl
	cmp cl, 1
	setg dl ; rdx = (rcx > 1) ? 1 : 0

	; rdi = loadB, rdx = loadD
	; skip process if (!(loadB || loadD))
	mov al, byte dil
	or al, dl
	test al, al
	jz .LoopRowsNext

	; set colDarkersFlags and colBrightersFlags
	xor rax, rax
	mov [rsp + 8], rax ; colDarkersFlags
	mov [rsp + 16], rax ; colBrightersFlags

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
	movdqu xmm8, [rbx + r11]
	movdqu xmm9, [rbx + r12]
	movdqu xmm10, [rbx + r13]
	movdqu xmm11, [rbx + r14]
	movdqu xmm12, [rbx + r15]
	mov r8, [rcx + 13*COMPV_YASM_REG_SZ_BYTES] ; pixels16[13]
	mov r9, [rcx + 14*COMPV_YASM_REG_SZ_BYTES] ; pixels16[14]
	mov r10, [rcx + 15*COMPV_YASM_REG_SZ_BYTES] ; pixels16[15]
	movdqu xmm13, [rbx + r8]
	movdqu xmm14, [rbx + r9]	
	movdqu xmm15, [rbx + r10]
	movdqa [rsp + 808 + 1*16], xmm0
	movdqa [rsp + 808 + 2*16], xmm1
	movdqa [rsp + 808 + 3*16], xmm2
	movdqa [rsp + 808 + 5*16], xmm3
	movdqa [rsp + 808 + 6*16], xmm8
	movdqa [rsp + 808 + 7*16], xmm9
	movdqa [rsp + 808 + 9*16], xmm10
	movdqa [rsp + 808 + 10*16], xmm11
	movdqa [rsp + 808 + 11*16], xmm12
	movdqa [rsp + 808 + 13*16], xmm13
	movdqa [rsp + 808 + 14*16], xmm14
	movdqa [rsp + 808 + 15*16], xmm15

	; We could compute pixels at 1 and 9, check if at least one is darker or brighter than the candidate
	; Then, do the same for 2 and 10 etc etc ... but this is slower than whant we're doing below because
	; _mm_movemask_epi8 is cyclyvore
	

	;
	;	LoadBrighters
	;
	test dil, dil ; rdi was loadB, now it's free
	jz .EndOfBrighters
	; compute Dbrighters
	pxor xmm4, xmm4
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
	movdqa [rsp + 1320 + 1*16], xmm0
	movdqa [rsp + 1320 + 2*16], xmm1
	movdqa [rsp + 1320 + 3*16], xmm2
	movdqa [rsp + 1320 + 5*16], xmm3
	movdqa [rsp + 1320 + 6*16], xmm8
	movdqa [rsp + 1320 + 7*16], xmm9
	movdqa [rsp + 1320 + 9*16], xmm10
	movdqa [rsp + 1320 + 10*16], xmm11
	movdqa [rsp + 1320 + 11*16], xmm12
	movdqa [rsp + 1320 + 13*16], xmm13
	movdqa [rsp + 1320 + 14*16], xmm14
	movdqa [rsp + 1320 + 15*16], xmm15
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
	movdqa xmm6, [sym(k254_u8)]
	movdqa xmm4, [rsp + 280] ; xmmNMinusOne
	movdqa xmm0, xmm6
	movdqa xmm1, xmm6
	movdqa xmm2, xmm6
	movdqa xmm3, xmm6
	pandn xmm0, [rsp + 552 + 0*16]
	pandn xmm1, [rsp + 552 + 4*16]
	pandn xmm2, [rsp + 552 + 8*16]
	pandn xmm3, [rsp + 552 + 12*16]
	paddusb xmm0, xmm1
	paddusb xmm2, xmm3
	paddusb xmm0, xmm2 ; xmm0 = 0 + 4 + 8 + 12
	paddusb xmm0, xmm12 ; xmm0 += 1 + 2 + 3 + 5 + 6 + 7 + 9 + 10 + 11 + 13 + 14 + 15
	; Check the columns with at least N non-zero bits
	pcmpgtb xmm0, xmm4
	pmovmskb rax, xmm0
	test rax, rax
	jz .EndOfBrighters
	; Continue loading brighters
	mov [rsp + 16], rax ; colBrightersFlags
	; Transpose
	COMPV_TRANSPOSE_I8_16X16_REG_T5_SSE2 rsp+1320+0*16, rsp+1320+1*16, rsp+1320+2*16, rsp+1320+3*16, rsp+1320+4*16, rsp+1320+5*16, rsp+1320+6*16, rsp+1320+7*16, rsp+1320+8*16, rsp+1320+9*16, rsp+1320+10*16, rsp+1320+11*16, rsp+1320+12*16, rsp+1320+13*16, rsp+1320+14*16, rsp+1320+15*16, xmm0, xmm1, xmm2, xmm3, xmm4
	; Flags
	pcmpeqb xmm6, xmm6 ; xmmFF
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
	pcmpeqb xmm0, [rsp + 1320 +0*16]
	pcmpeqb xmm1, [rsp + 1320 +1*16]
	pcmpeqb xmm2, [rsp + 1320 +2*16]
	pcmpeqb xmm3, [rsp + 1320 +3*16]
	pcmpeqb xmm4, [rsp + 1320 +4*16]
	pcmpeqb xmm8, [rsp + 1320 +5*16]
	pcmpeqb xmm9, [rsp + 1320 +6*16]
	pcmpeqb xmm10, [rsp + 1320 +7*16]
	pcmpeqb xmm11, [rsp + 1320 +8*16]
	pcmpeqb xmm12, [rsp + 1320 +9*16]
	pcmpeqb xmm13, [rsp + 1320 +10*16]
	pcmpeqb xmm14, [rsp + 1320 +11*16]
	pcmpeqb xmm15, [rsp + 1320 +12*16]
	pandn xmm0, xmm6
	pandn xmm1, xmm6
	pandn xmm2, xmm6
	pandn xmm3, xmm6
	pandn xmm4, xmm6
	pandn xmm8, xmm6
	pandn xmm9, xmm6
	pandn xmm10, xmm6
	pandn xmm11, xmm6
	pandn xmm12, xmm6
	pandn xmm13, xmm6
	pandn xmm14, xmm6
	pandn xmm15, xmm6
	pmovmskb edi, xmm0
	pmovmskb ecx, xmm1
	pmovmskb r8d, xmm2
	pmovmskb r9d, xmm3
	pmovmskb r10d, xmm4
	pmovmskb r11d, xmm8
	pmovmskb r12d, xmm9
	pmovmskb r13d, xmm10
	pmovmskb r14d, xmm11
	pmovmskb r15d, xmm12
	mov [rsp + 152 + 0*COMPV_YASM_REG_SZ_BYTES], di
	mov [rsp + 152 + 1*COMPV_YASM_REG_SZ_BYTES], cx
	mov [rsp + 152 + 2*COMPV_YASM_REG_SZ_BYTES], r8w
	mov [rsp + 152 + 3*COMPV_YASM_REG_SZ_BYTES], r9w
	mov [rsp + 152 + 4*COMPV_YASM_REG_SZ_BYTES], r10w
	mov [rsp + 152 + 5*COMPV_YASM_REG_SZ_BYTES], r11w
	mov [rsp + 152 + 6*COMPV_YASM_REG_SZ_BYTES], r12w
	mov [rsp + 152 + 7*COMPV_YASM_REG_SZ_BYTES], r13w
	mov [rsp + 152 + 8*COMPV_YASM_REG_SZ_BYTES], r14w
	mov [rsp + 152 + 9*COMPV_YASM_REG_SZ_BYTES], r15w
	pmovmskb r8d, xmm13
	pmovmskb r9d, xmm14
	pmovmskb r10d, xmm15
	mov [rsp + 152 + 10*COMPV_YASM_REG_SZ_BYTES], r8w
	mov [rsp + 152 + 11*COMPV_YASM_REG_SZ_BYTES], r9w
	mov [rsp + 152 + 12*COMPV_YASM_REG_SZ_BYTES], r10w
	pxor xmm0, xmm0
	pxor xmm1, xmm1
	pxor xmm2, xmm2
	pcmpeqb xmm0, [rsp + 1320 +13*16]
	pcmpeqb xmm1, [rsp + 1320 +14*16]
	pcmpeqb xmm2, [rsp + 1320 +15*16]
	pandn xmm0, xmm6
	pandn xmm1, xmm6
	pandn xmm2, xmm6
	pmovmskb edi, xmm0
	pmovmskb rax, xmm1
	pmovmskb rcx, xmm2
	mov [rsp + 152 + 13*COMPV_YASM_REG_SZ_BYTES], di
	mov [rsp + 152 + 14*COMPV_YASM_REG_SZ_BYTES], ax
	mov [rsp + 152 + 15*COMPV_YASM_REG_SZ_BYTES], cx

	.EndOfBrighters

	;
	;	LoadDarkers
	;
	test dl, dl ; rdx was loadD, now it's free
	jz .EndOfDarkers
	; compute ddarkers16x16 and flags
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
	psubusb xmm0, [rsp + 808 + 1*16]
	psubusb xmm1, [rsp + 808 + 2*16]
	psubusb xmm2, [rsp + 808 + 3*16]
	psubusb xmm3, [rsp + 808 + 5*16]
	psubusb xmm8, [rsp + 808 + 6*16]
	psubusb xmm9, [rsp + 808 + 7*16]
	psubusb xmm10, [rsp + 808 + 9*16]
	psubusb xmm11, [rsp + 808 + 10*16]
	psubusb xmm12, [rsp + 808 + 11*16]
	psubusb xmm13, [rsp + 808 + 13*16]
	psubusb xmm14, [rsp + 808 + 14*16]
	psubusb xmm15, [rsp + 808 + 15*16]
	movdqa [rsp + 1064 + 1*16], xmm0
	movdqa [rsp + 1064 + 2*16], xmm1
	movdqa [rsp + 1064 + 3*16], xmm2
	movdqa [rsp + 1064 + 5*16], xmm3
	movdqa [rsp + 1064 + 6*16], xmm8
	movdqa [rsp + 1064 + 7*16], xmm9
	movdqa [rsp + 1064 + 9*16], xmm10
	movdqa [rsp + 1064 + 10*16], xmm11
	movdqa [rsp + 1064 + 11*16], xmm12
	movdqa [rsp + 1064 + 13*16], xmm13
	movdqa [rsp + 1064 + 14*16], xmm14
	movdqa [rsp + 1064 + 15*16], xmm15
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
	movdqa xmm4, [rsp + 280] ; xmmNMinusOne
	movdqa xmm0, xmm5
	movdqa xmm1, xmm5
	movdqa xmm2, xmm5
	movdqa xmm3, xmm5
	pandn xmm0, [rsp + 296 + 0*16]
	pandn xmm1, [rsp + 296 + 4*16]
	pandn xmm2, [rsp + 296 + 8*16]
	pandn xmm3, [rsp + 296 + 12*16]
	paddusb xmm0, xmm1
	paddusb xmm2, xmm3
	paddusb xmm0, xmm2 ; xmm0 = 0 + 4 + 8 + 12
	paddusb xmm0, xmm12 ; xmm0 += 1 + 2 + 3 + 5 + 6 + 7 + 9 + 10 + 11 + 13 + 14 + 15
	; Check the columns with at least N non-zero bits
	pcmpgtb xmm0, xmm4
	pmovmskb edx, xmm0
	test dx, dx
	jz .EndOfDarkers
	; Continue loading darkers
	mov [rsp + 8], rdx ; colDarkersFlags
	; Transpose
	COMPV_TRANSPOSE_I8_16X16_REG_T5_SSE2 rsp+1064+0*16, rsp+1064+1*16, rsp+1064+2*16, rsp+1064+3*16, rsp+1064+4*16, rsp+1064+5*16, rsp+1064+6*16, rsp+1064+7*16, rsp+1064+8*16, rsp+1064+9*16, rsp+1064+10*16, rsp+1064+11*16, rsp+1064+12*16, rsp+1064+13*16, rsp+1064+14*16, rsp+1064+15*16, xmm0, xmm1, xmm2, xmm3, xmm4	
	; Flags
	pcmpeqb xmm5, xmm5 ; xmmFF
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
	pcmpeqb xmm0, [rsp + 1064 +0*16]
	pcmpeqb xmm1, [rsp + 1064 +1*16]
	pcmpeqb xmm2, [rsp + 1064 +2*16]
	pcmpeqb xmm3, [rsp + 1064 +3*16]
	pcmpeqb xmm4, [rsp + 1064 +4*16]
	pcmpeqb xmm8, [rsp + 1064 +5*16]
	pcmpeqb xmm9, [rsp + 1064 +6*16]
	pcmpeqb xmm10, [rsp + 1064 +7*16]
	pcmpeqb xmm11, [rsp + 1064 +8*16]
	pcmpeqb xmm12, [rsp + 1064 +9*16]
	pcmpeqb xmm13, [rsp + 1064 +10*16]
	pcmpeqb xmm14, [rsp + 1064 +11*16]
	pcmpeqb xmm15, [rsp + 1064 +12*16]
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
	pmovmskb eax, xmm0
	pmovmskb ecx, xmm1
	pmovmskb r8d, xmm2
	pmovmskb r9d, xmm3
	pmovmskb r10d, xmm4
	pmovmskb r11d, xmm8
	pmovmskb r12d, xmm9
	pmovmskb r13d, xmm10
	pmovmskb r14d, xmm11
	pmovmskb r15d, xmm12
	mov [rsp + 24 + 0*COMPV_YASM_REG_SZ_BYTES], ax
	mov [rsp + 24 + 1*COMPV_YASM_REG_SZ_BYTES], cx
	mov [rsp + 24 + 2*COMPV_YASM_REG_SZ_BYTES], r8w
	mov [rsp + 24 + 3*COMPV_YASM_REG_SZ_BYTES], r9w
	mov [rsp + 24 + 4*COMPV_YASM_REG_SZ_BYTES], r10w
	mov [rsp + 24 + 5*COMPV_YASM_REG_SZ_BYTES], r11w
	mov [rsp + 24 + 6*COMPV_YASM_REG_SZ_BYTES], r12w
	mov [rsp + 24 + 7*COMPV_YASM_REG_SZ_BYTES], r13w
	mov [rsp + 24 + 8*COMPV_YASM_REG_SZ_BYTES], r14w
	mov [rsp + 24 + 9*COMPV_YASM_REG_SZ_BYTES], r15w
	pmovmskb r8d, xmm13
	pmovmskb r9d, xmm14
	pmovmskb r10d, xmm15
	mov [rsp + 24 + 10*COMPV_YASM_REG_SZ_BYTES], r8w
	mov [rsp + 24 + 11*COMPV_YASM_REG_SZ_BYTES], r9w
	mov [rsp + 24 + 12*COMPV_YASM_REG_SZ_BYTES], r10w
	pxor xmm0, xmm0
	pxor xmm1, xmm1
	pxor xmm2, xmm2
	pcmpeqb xmm0, [rsp + 1064 +13*16]
	pcmpeqb xmm1, [rsp + 1064 +14*16]
	pcmpeqb xmm2, [rsp + 1064 +15*16]
	pandn xmm0, xmm5
	pandn xmm1, xmm5
	pandn xmm2, xmm5
	pmovmskb r8d, xmm0
	pmovmskb r9d, xmm1
	pmovmskb r10d, xmm2
	mov [rsp + 24 + 13*COMPV_YASM_REG_SZ_BYTES], r8w
	mov [rsp + 24 + 14*COMPV_YASM_REG_SZ_BYTES], r9w
	mov [rsp + 24 + 15*COMPV_YASM_REG_SZ_BYTES], r10w
	
	.EndOfDarkers


	; Check if we have to compute strengths
	mov rax, [rsp + 8] ; colDarkersFlags
	or rax, [rsp + 16] ; | colBrighters
	test rax, rax
	jz .NeitherDarkersNorBrighters
	; call FastStrengths16(colBrightersFlags, colDarkersFlags, (const uint8_t*)xmmDbrighters16x16, (const uint8_t*)xmmDdarkers16x16, &fbrighters16, &fdarkers16, (uint8_t*)xmmStrengths, N);
	mov rax, rsp ; save rsp before reserving params, must not be one of the registers used to save the params (rcx, rdx, r8, r9, rdi, rsi)
	movdqa [rsp + 1576], xmm7 ; save xmmThreshold
	COMPV_YASM_RESERVE_PARAMS r10, 8
	mov r10, [rax + 16] ; colBrightersFlags
	set_param 0, r10
	mov r10, [rax + 8] ; colDarkersFlags
	set_param 1, r10
	lea r10, [rax + 1320] ; xmmDbrighters16x16
	set_param 2, r10
	lea r10, [rax + 1064] ; xmmDdarkers16x16
	set_param 3, r10
	lea r10, [rax + 152] ; fbrighters16
	set_param 4, r10
	lea r10, [rax + 24] ; fdarkers16
	set_param 5, r10
	mov r10, arg(6) ; strengths
	set_param 6, r10
	mov r10, arg(4) ; N
	set_param 7, r10
	call sym(FastStrengths16)
	COMPV_YASM_UNRESERVE_PARAMS
	movdqa xmm7, [rsp + 1576] ; restore xmmThreshold
	.NeitherDarkersNorBrighters
	
	.LoopRowsNext
	mov rax, 16
	lea rbx, [rbx + 16] ; IP += 16
	add arg(6), rax ; strengths += 16
	
	;-------------------
	;EndOfLooopRows
	lea rsi, [rsi + 1]
	cmp rsi, arg(2)
	jl .LoopRows
	;-------------------

	; unalign stack and free memory
	add rsp, 8 + 8 + 8 + 8*16 + 8*16 + 16 + 16*16 + 16*16 + 16*16 + 16*16 + 16*16 + 16
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
	

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> compv_scalar_t rbrighters
; arg(1) -> compv_scalar_t rdarkers
; arg(2) -> COMPV_ALIGNED(SSE) const uint8_t* dbrighters16x16
; arg(3) -> COMPV_ALIGNED(SSE) const uint8_t* ddarkers16x16
; arg(4) -> const compv_scalar_t(*fbrighters16)[16]
; arg(5) -> const compv_scalar_t(*fdarkers16)[16]
; arg(6) -> uint8_t* strengths16
; arg(7) -> compv_scalar_t N
; %1 -> 1: CMOV is supported, 0 CMOV not supported
; %2 -> 9: Use FAST9, 12: FAST12 ....
%macro FastStrengths16_Asm_X64_SSE41 2
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_XMM 7 ;XMM[6-n]
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	push r14
	push r15
	; end prolog
	
	; r9 = p*16
	; r10 = 1<<p

	mov rax, 1
	xor r8, r8 ; r8 = p = 0
	xor r9, r9
	pxor xmm0, xmm0
	mov rbx, arg(6) ; Strengths16
	mov rdx, arg(0) ; rbrighters
	mov r10, rax ; (1<<p) = (1<<0) = 1
	mov r11, arg(1) ; rdarkers
	mov r12, arg(4) ; fbrighters16
	mov r13, arg(5) ; fdarkers16
	mov r14, arg(2) ; dbrighters16x16
	mov r15, arg(3) ; ddarkers16x16

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

	;----------------------
	; Loop Start
	;----------------------
	.LoopStart
		xor rcx, rcx ; rcx = maxn

		; ---------
		; Brighters
		; ---------
		test rdx, r10 ;  (1 << p) ?
		jz .EndOfBrighters
		mov rdi, [r12 + r8*COMPV_YASM_REG_SZ_BYTES] ; fbrighters16[p]

		movd xmm5, rdi
		punpcklwd xmm5, xmm5  
		pshufd xmm5, xmm5, 0 ; xmm5 = _mm_set1_epi16(fbrighters)
		movdqa xmm4, xmm5
		pand xmm5, xmm7
		pand xmm4, xmm6
		pcmpeqw xmm5, xmm7
		pcmpeqw xmm4, xmm6
		packsswb xmm5, xmm4
		pmovmskb eax, xmm5
		test ax, ax ; rax = r0
		jz .EndOfBrighters
		; Load dbrighters
		movdqa xmm2, [r14 + r9] ; dbrighters16x16[p*16]
		; Compute minimum hz
		COMPV_FEATURE_FAST_DETE_HORIZ_MIN_SSE41 Brighters, %1, %2, xmm2, xmm0, xmm1, xmm3, xmm4 ; This macro overrides rax, rsi, rdi and set the result in rcx
		.EndOfBrighters

		; ---------
		; Darkers
		; ---------
	.Darkers
		test r11, r10 ; (rdarkers & (1 << p)) ?
		jz .EndOfDarkers
		mov rdi, [r13 + r8*COMPV_YASM_REG_SZ_BYTES] ; fdarkers16[p]

		movd xmm5, rdi
		punpcklwd xmm5, xmm5  
		pshufd xmm5, xmm5, 0 ; xmm5 = _mm_set1_epi16(fdarkers)
		movdqa xmm4, xmm5
		pand xmm5, xmm7
		pand xmm4, xmm6
		pcmpeqw xmm5, xmm7
		pcmpeqw xmm4, xmm6
		packsswb xmm5, xmm4
		pmovmskb eax, xmm5
		test ax, ax ; rax = r0
		jz .EndOfDarkers
		; Load ddarkers16x16
		movdqa xmm2, [r15 + r9] ; ddarkers16x16[p*16]
		; Compute minimum hz
		COMPV_FEATURE_FAST_DETE_HORIZ_MIN_SSE41 Brighters, %1, %2, xmm2, xmm0, xmm1, xmm3, xmm4 ; This macro overrides rax, rsi, rdi and set the result in rcx
	.EndOfDarkers
		
	; compute strenghts[p]
	mov [rbx + r8], byte cl ; strengths16[p] = maxn

	add r8, 1 ; r8 = p
	add r9, 16 ; r9 = p*16
	shl r10, 1 ; r10 = 1<<p
	cmp r8, 16
	jl .LoopStart
	;----------------

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
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t Fast9Strengths16_Asm_CMOV_X64_SSE41(COMPV_ALIGNED(SSE) const int16_t(&dbrighters)[16], COMPV_ALIGNED(SSE) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
sym(Fast9Strengths16_Asm_CMOV_X64_SSE41):
	FastStrengths16_Asm_X64_SSE41 1, 9

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t Fast9Strengths16_Asm_X64_SSE41(COMPV_ALIGNED(SSE) const int16_t(&dbrighters)[16], COMPV_ALIGNED(SSE) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
sym(Fast9Strengths16_Asm_X64_SSE41):
	FastStrengths16_Asm_X64_SSE41 0, 9

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t Fast12Strengths16_Asm_CMOV_X64_SSE41(COMPV_ALIGNED(SSE) const int16_t(&dbrighters)[16], COMPV_ALIGNED(SSE) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
sym(Fast12Strengths16_Asm_CMOV_X64_SSE41):
	FastStrengths16_Asm_X64_SSE41 1, 12

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t Fast12Strengths16_Asm_X64_SSE41(COMPV_ALIGNED(SSE) const int16_t(&dbrighters)[16], COMPV_ALIGNED(SSE) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
sym(Fast12Strengths16_Asm_X64_SSE41):
	FastStrengths16_Asm_X64_SSE41 0, 12


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> compv_scalar_t rbrighters
; arg(1) -> compv_scalar_t rdarkers
; arg(2) -> COMPV_ALIGNED(SSE) const uint8_t* dbrighters16x32
; arg(3) -> COMPV_ALIGNED(SSE) const uint8_t* ddarkers16x32
; arg(4) -> const compv_scalar_t(*fbrighters16)[16]
; arg(5) -> const compv_scalar_t(*fdarkers16)[16]
; arg(6) -> uint8_t* Strengths32
; arg(7) -> compv_scalar_t N
; %1 -> 1: CMOV is supported, 0 CMOV not supported
; %2 -> 9: Use FAST9, 12: FAST12 ....
%macro FastStrengths32_Asm_X64_SSE41 2
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_XMM 7 ;XMM[6-n]
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	push r14
	push r15
	; end prolog
	
	; r8 = p
	; r9 = p*32
	; r10 = 1<<p	
	
	mov rbx, arg(6) ; Strengths32
	mov rdx, arg(0) ; rbrighters
	mov r11, arg(1) ; rdarkers
	mov r12, arg(4) ; fbrighters16
	mov r13, arg(5) ; fdarkers16
	mov r14, arg(2) ; dbrighters16x32
	mov r15, arg(3) ; ddarkers16x32
	pxor xmm0, xmm0
	

	; FAST hard-coded flags
	%if %2 == 9
		movdqa xmm3, [sym(Fast9Flags) + 0] ; xmm3 = xmmFastXFlagsLow
		movdqa xmm4, [sym(Fast9Flags) + 16]; xmm4 = xmmFastXFlagsHigh
	%elif %2 == 12
		movdqa xmm3, [sym(Fast12Flags) + 0] ; xmm3 = xmmFastXFlagsLow
		movdqa xmm4, [sym(Fast12Flags) + 16]; xmm4 = xmmFastXFlagsHigh
	%else
		%error "not supported"
	%endif	

	;----------------------
	; process16
	;----------------------
	%assign j 0
	%rep 2

		; Check if the low 16bits are unset
		%if j == 0
			mov rax, rdx
			or rax, r11 ; rdx || r11
			test ax, ax
			jnz .Low16bitsAreSet %+ j
			movdqu [rbx], xmm0 ; set strengths to zeros
			jmp .process16_next %+ j
			.Low16bitsAreSet %+ j
		%endif

		mov rax, 1
		xor r8, r8 ; r8 = p = 0
		xor r9, r9 ; r9 = p*32 = 0
		mov r10, rax ; (1<<p) = (1<<0) = 1

		;----------------------
		; Loop Start
		;----------------------
		.LoopStart %+ j
			xor rcx, rcx ; rcx = maxn

			; ---------
			; Brighters
			; ---------
			test rdx, r10 ;  (1 << p) ?
			jz .EndOfBrighters %+ j
				mov rdi, [r12 + r8*COMPV_YASM_REG_SZ_BYTES] ; fbrighters16[p]
				%if j == 1
					shr rdi, 16
				%endif

				movd xmm5, rdi
				punpcklwd xmm5, xmm5  
				pshufd xmm5, xmm5, 0 ; xmm5 = _mm_set1_epi16(fbrighters)
				movdqa xmm6, xmm5
				pand xmm5, xmm3
				pand xmm6, xmm4
				pcmpeqw xmm5, xmm3
				pcmpeqw xmm6, xmm4
				packsswb xmm5, xmm6
				pmovmskb eax, xmm5
				test ax, ax ; rax = r0
				jz .EndOfBrighters %+ j
					; Load dbrighters
					movdqa xmm2, [r14 + r9 + j*16] ; dbrighters16x32[p*16]
					; Compute minimum hz
					COMPV_FEATURE_FAST_DETE_HORIZ_MIN_SSE41 Brighters, %1, %2, xmm2, xmm0, xmm1, xmm6, xmm7 ; This macro overrides rax, rsi, rdi and set the result in rcx
			.EndOfBrighters %+ j

			; ---------
			; Darkers
			; ---------
		.Darkers %+ j
			test r11, r10 ; (rdarkers & (1 << p)) ?
			jz .EndOfDarkers %+ j
				mov rdi, [r13 + r8*COMPV_YASM_REG_SZ_BYTES] ; fdarkers16[p]
				%if j == 1
					shr rdi, 16
				%endif

				movd xmm5, rdi
				punpcklwd xmm5, xmm5  
				pshufd xmm5, xmm5, 0 ; xmm5 = _mm_set1_epi16(fdarkers)
				movdqa xmm6, xmm5
				pand xmm5, xmm3
				pand xmm6, xmm4
				pcmpeqw xmm5, xmm3
				pcmpeqw xmm6, xmm4
				packsswb xmm5, xmm6
				pmovmskb eax, xmm5
				test ax, ax ; rax = r0
				jz .EndOfDarkers %+ j
					; Load ddarkers16x32
					movdqa xmm2, [r15 + r9 + j*16] ; ddarkers16x32[p*16]
					; Compute minimum hz
					COMPV_FEATURE_FAST_DETE_HORIZ_MIN_SSE41 Darkers, %1, %2, xmm2, xmm0, xmm1, xmm6, xmm7 ; This macro overrides rax, rsi, rdi and set the result in rcx
		.EndOfDarkers %+ j
		
		; compute strenghts[p]
		mov [rbx + r8 + j*16], byte cl ; Strengths32[p] = maxn

		lea r8, [r8 + 1]
		lea r9, [r9 + 32] ; r9 = p*32
		shl r10, 1 ; r10 = 1<<p
		cmp r8, 16
		jl .LoopStart %+ j
		;----------------

	.process16_next %+ j

	%if j == 0
		shr rdx, 16
		shr r11, 16
		mov r8, rdx
		or r8, r11 ; rbrighters || rdarkers
		test r8w, r8w
		jnz .process16_continue
		; set remaining strengths to zeros
		movdqu [rbx + 16], xmm0
		jmp .process16_done
		.process16_continue
	%endif

	%assign j j+1

	; EndOf .process16
	%endrep

	.process16_done

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
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t Fast9Strengths32_Asm_CMOV_X64_SSE41(COMPV_ALIGNED(SSE) const int16_t(&dbrighters)[16], COMPV_ALIGNED(SSE) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
sym(Fast9Strengths32_Asm_CMOV_X64_SSE41):
	FastStrengths32_Asm_X64_SSE41 1, 9

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t Fast9Strengths32_Asm_X64_SSE41(COMPV_ALIGNED(SSE) const int16_t(&dbrighters)[16], COMPV_ALIGNED(SSE) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
sym(Fast9Strengths32_Asm_X64_SSE41):
	FastStrengths32_Asm_X64_SSE41 0, 9

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t Fast12Strengths32_Asm_CMOV_X64_SSE41(COMPV_ALIGNED(SSE) const int16_t(&dbrighters)[16], COMPV_ALIGNED(SSE) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
sym(Fast12Strengths32_Asm_CMOV_X64_SSE41):
	FastStrengths32_Asm_X64_SSE41 1, 12

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t Fast12Strengths32_Asm_X64_SSE41(COMPV_ALIGNED(SSE) const int16_t(&dbrighters)[16], COMPV_ALIGNED(SSE) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
sym(Fast12Strengths32_Asm_X64_SSE41):
	FastStrengths32_Asm_X64_SSE41 0, 12

%endif ; COMPV_YASM_ABI_IS_64BIT

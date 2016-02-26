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

global sym(FastData32Row_Asm_X86_AVX2)

section .data
	extern sym(k1_i8)
	extern sym(k254_u8)

	extern sym(FastStrengths32) ; function

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* IP
; arg(1) -> const uint8_t* IPprev
; arg(2) -> compv_scalar_t width
; arg(3) -> const compv_scalar_t(&pixels16)[16]
; arg(4) -> compv_scalar_t N
; arg(5) -> compv_scalar_t threshold
; arg(6) -> uint8_t* strengths
; arg(7) -> compv_scalar_t* me
; void FastData32Row_Asm_X86_AVX2(const uint8_t* IP, const uint8_t* IPprev, compv_scalar_t width, const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, uint8_t* strengths, compv_scalar_t* me);
sym(FastData32Row_Asm_X86_AVX2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_XMM 7 ;XMM[6-n]
	push rsi
	push rdi
	push rbx
	; end prolog

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 32, rax
	sub rsp, 8 + 8 + 8 + 16*8 + 16*8 + 32 + 16*32 + 16*32 + 16*32 + 16*32 + 16*32 + 32
	; [rsp + 0] = compv_scalar_t sum
	; [rsp + 8] = compv_scalar_t colDarkersFlags
	; [rsp + 16] = compv_scalar_t colBrightersFlags
	; [rsp + 24] = compv_scalar_t fdarkers16[16];
	; [rsp + 152] = compv_scalar_t fbrighters16[16];
	; [rsp + 280] = __m256i ymmNMinusOne
	; [rsp + 312] = __m256i ymmDarkersFlags[16]
	; [rsp + 824] = __m256i ymmBrightersFlags[16]
	; [rsp + 1336] = __m256i ymmDataPtr[16]
	; [rsp + 1848] = __m256i ymmDdarkers16x32[16];
	; [rsp + 2360] = __m256i ymmDbrighters16x32[16];
	; [rsp + 2872] = __m256i ymmThreshold (saved/restored after function call)

	
	; Compute xmmThreshold and xmmNMinusOne here to avoid AVX/SSE mixing
	mov rax, arg(5) ; threshold
	mov rcx, arg(4) ; N
	sub rcx, 1
	vmovd xmm7, eax ; xmm7 = xmmThreshold
	movd xmm0, ecx ; xmm0 = xmmNMinusOne

	mov rsi, arg(2) ; rsi = width
	mov rbx, arg(0) ; rbx = IP

	vzeroupper
	
	; Compute ymmThreshold
	vpbroadcastb ymm7, xmm7 ;  ymm7 = ymmThreshold

	; Compute ymmNMinusOne and save
	vpbroadcastb ymm0, xmm0
	vmovdqa [rsp + 280], ymm0
	
	;-------------------
	;StartOfLooopRows
	;
	.LoopRows
	; -------------------
	vmovdqu ymm6, [rbx]
	vpxor ymm4, ymm4

	; Motion Estimation
	; TODO(dmi): not supported
	; TODO(dmi): inc IPprev here

	; cleanup strengths
	mov rax, arg(6)
	vmovdqu [rax], ymm4

	vpsubusb ymm5, ymm6, ymm7 ; ymm5 = ymmDarker
	vpaddusb ymm6, ymm6, ymm7 ; ymm6 = ymmBrighter


	;
	; Speed-Test-1
	;

	; compare I1 and I9 aka 0 and 8
	mov rdx, arg(3) ; pixels16
	mov rax, [rdx + 0*COMPV_YASM_REG_SZ_BYTES] ; pixels16[0]
	mov rdx, [rdx + 8*COMPV_YASM_REG_SZ_BYTES] ; pixels16[8]
	vmovdqu ymm0, [rbx + rax] ; IP[pixels16[0]]
	vmovdqu ymm1, [rbx + rdx] ; IP[pixels16[8]]
	vpsubusb ymm2, ymm5, ymm0 ; ddarkers16x32[0]
	vpsubusb ymm3, ymm5, ymm1 ; ddarkers16x32[8]
	vpsubusb ymm0, ymm6 ; dbrighters16x32[0]
	vpsubusb ymm1, ymm6 ; dbrighters16x32[8]
	vmovdqa [rsp + 1848 + 0*32], ymm2
	vmovdqa [rsp + 1848 + 8*32], ymm3
	vmovdqa [rsp + 2360 + 0*32], ymm0
	vmovdqa [rsp + 2360 + 8*32], ymm1
	vpcmpeqb ymm2, ymm4
	vpcmpeqb ymm3, ymm4
	vpcmpeqb ymm0, ymm4
	vpcmpeqb ymm1, ymm4
	vpcmpeqb ymm4, ymm4  ; ymm4 = ymmFF
	vpandn ymm2, ymm4
	vpandn ymm3, ymm4
	vpandn ymm0, ymm4
	vpandn ymm1, ymm4
	vmovdqa [rsp + 312 + 0*32], ymm2 ; ymmDarkersFlags[0]
	vmovdqa [rsp + 312 + 8*32], ymm3 ; ymmDarkersFlags[8]
	vmovdqa [rsp + 824 + 0*32], ymm0 ; ymmBrightersFlags[0]
	vmovdqa [rsp + 824 + 8*32], ymm1 ; ymmBrightersFlags[8]
	vpor ymm0, ymm2
	vpor ymm1, ymm3
	vpmovmskb eax, ymm0
	vpmovmskb edx, ymm1
	test eax, eax
	setnz al
	test edx, edx
	setnz dl
	add dl, al
	test dl, dl
	jz .LoopRowsNext
	mov [rsp + 0], dl ; sum = ?

	; compare I5 and I13 aka 4 and 12
	mov rdx, arg(3) ; pixels16
	mov rax, [rdx + 4*COMPV_YASM_REG_SZ_BYTES] ; pixels16[4]
	mov rdx, [rdx + 12*COMPV_YASM_REG_SZ_BYTES] ; pixels16[12]
	vmovdqu ymm0, [rbx + rax] ; IP[pixels16[4]]
	vmovdqu ymm1, [rbx + rdx] ; IP[pixels16[12]]
	vpxor ymm4, ymm4 ; ymm4 = ymmZeros
	vpsubusb ymm2, ymm5, ymm0 ; ddarkers16x32[4]
	vpsubusb ymm3, ymm5, ymm1 ; ddarkers16x32[12]
	vpsubusb ymm0, ymm6 ; dbrighters16x32[4]
	vpsubusb ymm1, ymm6 ; dbrighters16x32[12]
	vmovdqa [rsp + 1848 + 4*32], ymm2
	vmovdqa [rsp + 1848 + 12*32], ymm3
	vmovdqa [rsp + 2360 + 4*32], ymm0
	vmovdqa [rsp + 2360 + 12*32], ymm1
	vpcmpeqb ymm2, ymm4
	vpcmpeqb ymm3, ymm4
	vpcmpeqb ymm0, ymm4
	vpcmpeqb ymm1, ymm4
	vpcmpeqb ymm4, ymm4  ; ymm4 = ymmFF
	vpandn ymm2, ymm4
	vpandn ymm3, ymm4
	vpandn ymm0, ymm4
	vpandn ymm1, ymm4
	vmovdqa [rsp + 312 + 4*32], ymm2 ; ymmDarkersFlags[4]
	vmovdqa [rsp + 312 + 12*32], ymm3 ; ymmDarkersFlags[12]
	vmovdqa [rsp + 824 + 4*32], ymm0 ; ymmBrightersFlags[4]
	vmovdqa [rsp + 824 + 12*32], ymm1 ; ymmBrightersFlags[12]
	vpor ymm0, ymm2
	vpor ymm1, ymm3
	vpmovmskb eax, ymm0
	vpmovmskb edx, ymm1
	test eax, eax
	setnz al
	test edx, edx
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
	vmovdqa ymm0, [rsp + 824 + 0*32] ; ymmBrightersFlags[0]
	vmovdqa ymm1, [rsp + 824 + 4*32] ; ymmBrightersFlags[4]
	vpor ymm0, [rsp + 824 + 8*32] ; ymmBrightersFlags[0] | ymmBrightersFlags[8]
	vpor ymm1, [rsp + 824 + 12*32] ; ymmBrightersFlags[4] | ymmBrightersFlags[12]
	vpmovmskb eax, ymm0
	vpmovmskb edx, ymm1
	test eax, eax
	setnz al
	test edx, edx
	setnz dl
	add dl, al
	cmp dl, 1
	setg dl
	movzx rdi, byte dl ; rdi = (rdx > 1) ? 1 : 0

	; Check whether to load Darkers
	vmovdqa ymm0, [rsp + 312 + 0*32] ; ymmDarkersFlags[0]
	vmovdqa ymm1, [rsp + 312 + 4*32] ; ymmDarkersFlags[4]
	vpor ymm0, [rsp + 312 + 8*32] ; ymmDarkersFlags[0] | ymmDarkersFlags[8]
	vpor ymm1, [rsp + 312 + 12*32] ; ymmDarkersFlags[4] | ymmDarkersFlags[12]
	vpmovmskb eax, ymm0
	vpmovmskb edx, ymm1
	test eax, eax
	setnz al
	test edx, edx
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

	; Set colDarkersFlags and colBrightersFlags to zero
	xor rax, rax
	mov [rsp + 8], rax ; colDarkersFlags
	mov [rsp + 16], rax ; colBrightersFlags


	; Load ymmDataPtr
	mov rcx, arg(3) ; pixels16
	mov rax, [rcx + 1*COMPV_YASM_REG_SZ_BYTES] ; pixels16[1]
	vmovdqu ymm0, [rbx + rax]
	mov rax, [rcx + 2*COMPV_YASM_REG_SZ_BYTES] ; pixels16[2]
	vmovdqu ymm1, [rbx + rax]
	mov rax, [rcx + 3*COMPV_YASM_REG_SZ_BYTES] ; pixels16[3]
	vmovdqu ymm2, [rbx + rax]
	mov rax, [rcx + 5*COMPV_YASM_REG_SZ_BYTES] ; pixels16[5]
	vmovdqu ymm3, [rbx + rax]
	mov rax, [rcx + 6*COMPV_YASM_REG_SZ_BYTES] ; pixels16[6]
	vmovdqu ymm4, [rbx + rax]
	vmovdqa [rsp + 1336 + 1*32], ymm0
	vmovdqa [rsp + 1336 + 2*32], ymm1
	vmovdqa [rsp + 1336 + 3*32], ymm2
	vmovdqa [rsp + 1336 + 5*32], ymm3
	vmovdqa [rsp + 1336 + 6*32], ymm4
	mov rax, [rcx + 7*COMPV_YASM_REG_SZ_BYTES] ; pixels16[7]
	vmovdqu ymm0, [rbx + rax]
	mov rax, [rcx + 9*COMPV_YASM_REG_SZ_BYTES] ; pixels16[9]
	vmovdqu ymm1, [rbx + rax]
	mov rax, [rcx + 10*COMPV_YASM_REG_SZ_BYTES] ; pixels16[10]
	vmovdqu ymm2, [rbx + rax]
	mov rax, [rcx + 11*COMPV_YASM_REG_SZ_BYTES] ; pixels16[11]
	vmovdqu ymm3, [rbx + rax]
	mov rax, [rcx + 13*COMPV_YASM_REG_SZ_BYTES] ; pixels16[13]
	vmovdqu ymm4, [rbx + rax]
	vmovdqa [rsp + 1336 + 7*32], ymm0
	vmovdqa [rsp + 1336 + 9*32], ymm1
	vmovdqa [rsp + 1336 + 10*32], ymm2
	vmovdqa [rsp + 1336 + 11*32], ymm3
	vmovdqa [rsp + 1336 + 13*32], ymm4
	mov rax, [rcx + 14*COMPV_YASM_REG_SZ_BYTES] ; pixels16[14]
	vmovdqu ymm0, [rbx + rax]
	mov rax, [rcx + 15*COMPV_YASM_REG_SZ_BYTES] ; pixels16[15]
	vmovdqu ymm1, [rbx + rax]
	vmovdqa [rsp + 1336 + 14*32], ymm0
	vmovdqa [rsp + 1336 + 15*32], ymm1

	; We could compute pixels at 1 and 9, check if at least one is darker or brighter than the candidate
	; Then, do the same for 2 and 10 etc etc ... but this is slower than whant we're doing below because
	; _mm_movemask_epi8 is cyclyvore

	;
	;	LoadDarkers
	;
	test dl, dl ; rdx was loadD, now it's free
	jz .EndOfDarkers
	; compute ddarkers16x32 and flags
	vpxor ymm4, ymm4
	vpsubusb ymm0, ymm5, [rsp + 1336 + 1*32]
	vpsubusb ymm1, ymm5, [rsp + 1336 + 2*32]
	vpsubusb ymm2, ymm5, [rsp + 1336 + 3*32]
	vpsubusb ymm3, ymm5, [rsp + 1336 + 5*32]
	vmovdqa [rsp + 1848 + 1*32], ymm0
	vmovdqa [rsp + 1848 + 2*32], ymm1
	vmovdqa [rsp + 1848 + 3*32], ymm2
	vmovdqa [rsp + 1848 + 5*32], ymm3
	vpcmpeqb ymm0, ymm4
	vpcmpeqb ymm1, ymm4
	vpcmpeqb ymm2, ymm4
	vpcmpeqb ymm3, ymm4
	vmovdqa ymm4, [sym(k1_i8)]
	vpandn ymm0, ymm4
	vpandn ymm1, ymm4
	vpandn ymm2, ymm4
	vpandn ymm3, ymm4
	vpaddusb ymm0, ymm1
	vpaddusb ymm2, ymm3
	vpaddusb ymm0, ymm2
	vmovdqa [rsp + 312 + 1*32], ymm0 ; ymmDarkersFlags[1] = 1 + 2 + 3 + 5
	vpxor ymm4, ymm4
	vpsubusb ymm0, ymm5, [rsp + 1336 + 6*32]
	vpsubusb ymm1, ymm5, [rsp + 1336 + 7*32]
	vpsubusb ymm2, ymm5, [rsp + 1336 + 9*32]
	vpsubusb ymm3, ymm5, [rsp + 1336 + 10*32]
	vmovdqa [rsp + 1848 + 6*32], ymm0
	vmovdqa [rsp + 1848 + 7*32], ymm1
	vmovdqa [rsp + 1848 + 9*32], ymm2
	vmovdqa [rsp + 1848 + 10*32], ymm3
	vpcmpeqb ymm0, ymm4
	vpcmpeqb ymm1, ymm4
	vpcmpeqb ymm2, ymm4
	vpcmpeqb ymm3, ymm4
	vmovdqa ymm4, [sym(k1_i8)]
	vpandn ymm0, ymm4
	vpandn ymm1, ymm4
	vpandn ymm2, ymm4
	vpandn ymm3, ymm4
	vpaddusb ymm0, ymm1
	vpaddusb ymm2, ymm3
	vpaddusb ymm0, ymm2
	vmovdqa [rsp + 312 + 6*32], ymm0 ; ymmDarkersFlags[6] = 6 + 7 + 9 + 10
	vpxor ymm4, ymm4
	vpsubusb ymm0, ymm5, [rsp + 1336 + 11*32]
	vpsubusb ymm1, ymm5, [rsp + 1336 + 13*32]
	vpsubusb ymm2, ymm5, [rsp + 1336 + 14*32]
	vpsubusb ymm3, ymm5, [rsp + 1336 + 15*32]
	vmovdqa [rsp + 1848 + 11*32], ymm0
	vmovdqa [rsp + 1848 + 13*32], ymm1
	vmovdqa [rsp + 1848 + 14*32], ymm2
	vmovdqa [rsp + 1848 + 15*32], ymm3
	vpcmpeqb ymm0, ymm4
	vpcmpeqb ymm1, ymm4
	vpcmpeqb ymm2, ymm4
	vpcmpeqb ymm3, ymm4
	vmovdqa ymm4, [sym(k1_i8)]
	vpandn ymm0, ymm4
	vpandn ymm1, ymm4
	vpandn ymm2, ymm4
	vpandn ymm3, ymm4
	vpaddusb ymm0, ymm1
	vpaddusb ymm2, ymm3
	vpaddusb ymm0, ymm2
	vmovdqa [rsp + 312 + 11*32], ymm0 ; ymmDarkersFlags[11] = 11 + 13 + 14 + 15
	; Compute flags 0, 4, 8, 12
	vmovdqa ymm5, [sym(k254_u8)]
	vmovdqa ymm4, [rsp + 280] ; ymmNMinusOne
	vpandn ymm0, ymm5, [rsp + 312 + 0*32]
	vpandn ymm1, ymm5, [rsp + 312 + 4*32]
	vpandn ymm2, ymm5, [rsp + 312 + 8*32]
	vpandn ymm3, ymm5, [rsp + 312 + 12*32]
	vpaddusb ymm0, ymm1
	vpaddusb ymm2, ymm3
	vpaddusb ymm0, ymm2 ; ymm0 = 0 + 4 + 8 + 12
	vpaddusb ymm0, [rsp + 312 + 1*32] ; ymm0 += 1 + 2 + 3 + 5
	vpaddusb ymm0, [rsp + 312 + 6*32] ; ymm0 += 6 + 7 + 9 + 10
	vpaddusb ymm0, [rsp + 312 + 11*32] ; ymm0 += 11 + 13 + 14 + 15
	; Check the columns with at least N non-zero bits
	vpcmpgtb ymm0, ymm4
	vpmovmskb edx, ymm0
	test edx, edx
	jz .EndOfDarkers
	; Continue loading darkers
	mov [rsp + 8], edx ; colDarkersFlags
	; Transpose
	COMPV_TRANSPOSE_I8_16X16_REG_T5_AVX2 rsp+1848+0*32, rsp+1848+1*32, rsp+1848+2*32, rsp+1848+3*32, rsp+1848+4*32, rsp+1848+5*32, rsp+1848+6*32, rsp+1848+7*32, rsp+1848+8*32, rsp+1848+9*32, rsp+1848+10*32, rsp+1848+11*32, rsp+1848+12*32, rsp+1848+13*32, rsp+1848+14*32, rsp+1848+15*32, ymm0, ymm1, ymm2, ymm3, ymm4
	; Flags
	vpcmpeqb ymm5, ymm5 ; ymmFF
	vpxor ymm4, ymm4 ; ymmZeros
	%assign i 0
	%rep    4
		vpcmpeqb ymm0, ymm4, [rsp + 1848 +(0+i)*32]
		vpcmpeqb ymm1, ymm4, [rsp + 1848 +(1+i)*32]
		vpcmpeqb ymm2, ymm4, [rsp + 1848 +(2+i)*32]
		vpcmpeqb ymm3, ymm4, [rsp + 1848 +(3+i)*32]
		vpandn ymm0, ymm5
		vpandn ymm1, ymm5
		vpandn ymm2, ymm5
		vpandn ymm3, ymm5
		vpmovmskb eax, ymm0
		vpmovmskb ecx, ymm1
		mov [rsp + 24 + (0+i)*COMPV_YASM_REG_SZ_BYTES], eax
		mov [rsp + 24 + (1+i)*COMPV_YASM_REG_SZ_BYTES], ecx
		vpmovmskb eax, ymm2
		vpmovmskb ecx, ymm3
		mov [rsp + 24 + (2+i)*COMPV_YASM_REG_SZ_BYTES], eax
		mov [rsp + 24 + (3+i)*COMPV_YASM_REG_SZ_BYTES], ecx
		%assign i i+4
	%endrep
	
	.EndOfDarkers

	;
	;	LoadBrighters
	;
	test rdi, rdi ; rdi was loadB, now it's free
	jz .EndOfBrighters
	; compute Dbrighters
	vpxor ymm5, ymm5
	vmovdqa ymm0, [rsp + 1336 + 1*32]
	vmovdqa ymm1, [rsp + 1336 + 2*32]
	vmovdqa ymm2, [rsp + 1336 + 3*32]
	vmovdqa ymm3, [rsp + 1336 + 5*32]
	vmovdqa ymm4, [rsp + 1336 + 6*32]
	vpsubusb ymm0, ymm6
	vpsubusb ymm1, ymm6
	vpsubusb ymm2, ymm6
	vpsubusb ymm3, ymm6
	vpsubusb ymm4, ymm6
	vmovdqa [rsp + 2360 + 1*32], ymm0
	vmovdqa [rsp + 2360 + 2*32], ymm1
	vmovdqa [rsp + 2360 + 3*32], ymm2
	vmovdqa [rsp + 2360 + 5*32], ymm3
	vmovdqa [rsp + 2360 + 6*32], ymm4
	vpcmpeqb ymm0, ymm5
	vpcmpeqb ymm1, ymm5
	vpcmpeqb ymm2, ymm5
	vpcmpeqb ymm3, ymm5
	vpcmpeqb ymm4, ymm5
	vmovdqa ymm5, [sym(k1_i8)]
	vpandn ymm0, ymm5
	vpandn ymm1, ymm5
	vpandn ymm2, ymm5
	vpandn ymm3, ymm5
	vpandn ymm4, ymm5
	vpaddusb ymm0, ymm1
	vpaddusb ymm2, ymm3
	vpaddusb ymm0, ymm4
	vpaddusb ymm0, ymm2
	vmovdqa [rsp + 824 + 1*32], ymm0 ; ymmBrightersFlags[1] = 1 + 2 + 3 + 5 + 6
	vpxor ymm5, ymm5
	vmovdqa ymm0, [rsp + 1336 + 7*32]
	vmovdqa ymm1, [rsp + 1336 + 9*32]
	vmovdqa ymm2, [rsp + 1336 + 10*32]
	vmovdqa ymm3, [rsp + 1336 + 11*32]
	vmovdqa ymm4, [rsp + 1336 + 13*32]
	vpsubusb ymm0, ymm6
	vpsubusb ymm1, ymm6
	vpsubusb ymm2, ymm6
	vpsubusb ymm3, ymm6
	vpsubusb ymm4, ymm6
	vmovdqa [rsp + 2360 + 7*32], ymm0
	vmovdqa [rsp + 2360 + 9*32], ymm1
	vmovdqa [rsp + 2360 + 10*32], ymm2
	vmovdqa [rsp + 2360 + 11*32], ymm3
	vmovdqa [rsp + 2360 + 13*32], ymm4
	vpcmpeqb ymm0, ymm5
	vpcmpeqb ymm1, ymm5
	vpcmpeqb ymm2, ymm5
	vpcmpeqb ymm3, ymm5
	vpcmpeqb ymm4, ymm5
	vmovdqa ymm5, [sym(k1_i8)]
	vpandn ymm0, ymm5
	vpandn ymm1, ymm5
	vpandn ymm2, ymm5
	vpandn ymm3, ymm5
	vpandn ymm4, ymm5
	vpaddusb ymm0, ymm1
	vpaddusb ymm2, ymm3
	vpaddusb ymm0, ymm4
	vpaddusb ymm0, ymm2
	vmovdqa [rsp + 824 + 7*32], ymm0 ; ymmBrightersFlags[7] = 7 + 9 + 10 + 11 + 13
	vpxor ymm5, ymm5
	vmovdqa ymm4, [sym(k1_i8)]
	vmovdqa ymm0, [rsp + 1336 + 14*32]
	vmovdqa ymm1, [rsp + 1336 + 15*32]
	vpsubusb ymm0, ymm6
	vpsubusb ymm1, ymm6
	vmovdqa [rsp + 2360 + 14*32], ymm0
	vmovdqa [rsp + 2360 + 15*32], ymm1
	vpcmpeqb ymm0, ymm5
	vpcmpeqb ymm1, ymm5
	vpandn ymm0, ymm4
	vpandn ymm1, ymm4
	vpaddusb ymm0, ymm1
	vmovdqa [rsp + 824 + 14*32], ymm0 ; ymmBrightersFlags[14] = 14 + 15	
	; Compute flags 0, 4, 8, 12
	vmovdqa ymm6, [sym(k254_u8)]
	vmovdqa ymm4, [rsp + 280] ; ymmNMinusOne
	vpandn ymm0, ymm6, [rsp + 824 + 0*32]
	vpandn ymm1, ymm6, [rsp + 824 + 4*32]
	vpandn ymm2, ymm6, [rsp + 824 + 8*32]
	vpandn ymm3, ymm6, [rsp + 824 + 12*32]
	vpaddusb ymm0, ymm1
	vpaddusb ymm2, ymm3
	vpaddusb ymm0, ymm2 ; ymm0 = 0 + 4 + 8 + 12
	vpaddusb ymm0, [rsp + 824 + 1*32] ; ymm0 += 1 + 2 + 3 + 5 + 6
	vpaddusb ymm0, [rsp + 824 + 7*32] ; ymm0 += 7 + 9 + 10 + 11 + 13
	vpaddusb ymm0, [rsp + 824 + 14*32] ; ymm0 += 14 + 15
	; Check the columns with at least N non-zero bits
	vpcmpgtb ymm0, ymm4
	vpmovmskb edx, ymm0
	test edx, edx
	jz .EndOfBrighters
	; Continue loading brighters
	mov [rsp + 16], edx ; colBrightersFlags
	; Transpose
	COMPV_TRANSPOSE_I8_16X16_REG_T5_AVX2 rsp+2360+0*32, rsp+2360+1*32, rsp+2360+2*32, rsp+2360+3*32, rsp+2360+4*32, rsp+2360+5*32, rsp+2360+6*32, rsp+2360+7*32, rsp+2360+8*32, rsp+2360+9*32, rsp+2360+10*32, rsp+2360+11*32, rsp+2360+12*32, rsp+2360+13*32, rsp+2360+14*32, rsp+2360+15*32, ymm0, ymm1, ymm2, ymm3, ymm4
	; Flags
	vpcmpeqb ymm6, ymm6 ; ymmFF
	vpxor ymm4, ymm4 ; ymmZeros
	%assign i 0
	%rep    4
		vpcmpeqb ymm0, ymm4, [rsp + 2360 +(0+i)*32]
		vpcmpeqb ymm1, ymm4, [rsp + 2360 +(1+i)*32]
		vpcmpeqb ymm2, ymm4, [rsp + 2360 +(2+i)*32]
		vpcmpeqb ymm3, ymm4, [rsp + 2360 +(3+i)*32]
		vpandn ymm0, ymm6
		vpandn ymm1, ymm6
		vpandn ymm2, ymm6
		vpandn ymm3, ymm6
		vpmovmskb edi, ymm0
		vpmovmskb ecx, ymm1
		mov [rsp + 152 + (0+i)*COMPV_YASM_REG_SZ_BYTES], edi
		mov [rsp + 152 + (1+i)*COMPV_YASM_REG_SZ_BYTES], ecx
		vpmovmskb edi, ymm2
		vpmovmskb ecx, ymm3
		mov [rsp + 152 + (2+i)*COMPV_YASM_REG_SZ_BYTES], edi
		mov [rsp + 152 + (3+i)*COMPV_YASM_REG_SZ_BYTES], ecx
		%assign i i+4
	%endrep

	.EndOfBrighters

	; Check if we have to compute strengths
	mov rax, [rsp + 8] ; colDarkersFlags
	or rax, [rsp + 16] ; | colBrighters
	test rax, rax
	jz .NeitherDarkersNorBrighters
	; call FastStrengths32(compv_scalar_t rbrighters, compv_scalar_t rdarkers, COMPV_ALIGNED(SSE) const uint8_t* dbrighters16x32, COMPV_ALIGNED(SSE) const uint8_t* ddarkers16x32, const compv_scalar_t(*fbrighters16)[16], const compv_scalar_t(*fdarkers16)[16], uint8_t* strengths32, compv_scalar_t N);
	mov rax, rsp ; save rsp before reserving params, must not be one of the registers used to save the params (rcx, rdx, r8, r9, rdi, rsi)
	vmovdqa [rsp + 2872], ymm7 ; save ymmThreshold
	push rbx ; because we cannot use [rcx, rdx, r8, r9, rdi, rsi]
	COMPV_YASM_RESERVE_PARAMS rbx, 8
	mov rbx, [rax + 16] ; colBrightersFlags
	set_param 0, rbx
	mov rbx, [rax + 8] ; colDarkersFlags
	set_param 1, rbx
	lea rbx, [rax + 2360] ; ymmDbrighters16x32
	set_param 2, rbx
	lea rbx, [rax + 1848] ; ymmDdarkers16x32
	set_param 3, rbx
	lea rbx, [rax + 152] ; fbrighters16
	set_param 4, rbx
	lea rbx, [rax + 24] ; fdarkers16
	set_param 5, rbx
	mov rbx, arg(6) ; strengths
	set_param 6, rbx
	mov rbx, arg(4) ; N
	set_param 7, rbx
	call sym(FastStrengths32)
	COMPV_YASM_UNRESERVE_PARAMS
	pop rbx
	vmovdqa ymm7, [rsp + 2872] ; restore ymmThreshold
	.NeitherDarkersNorBrighters
	
	.LoopRowsNext

	; TODO(dmi): do the same as x64, increment these values only if needed
	mov rdx, 32
	lea rbx, [rbx + 32] ; IP += 32
	add arg(6), rdx ; strenghts += 32
	; TODO(dmi): Motion estimation not supported -> do not inc IPprev

	;-------------------
	;EndOfLooopRows
	lea rsi, [rsi - 32]
	test rsi, rsi
	jnz .LoopRows
	;-------------------

	.EndOfFunction

	; unalign stack and free memory
	add rsp,  8 + 8 + 8 + 16*8 + 16*8 + 32 + 16*32 + 16*32 + 16*32 + 16*32 + 16*32 + 32
	COMPV_YASM_UNALIGN_STACK

	vzeroupper

	; begin epilog
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
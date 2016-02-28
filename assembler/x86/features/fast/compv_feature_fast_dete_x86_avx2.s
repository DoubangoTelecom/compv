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
%include "../../compv_common_x86.s"
%include "../../compv_bits_macros_x86.s"
%include "../../compv_math_macros_x86.s"
%include "compv_feature_fast_dete_macros_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(FastData32Row_Asm_X86_AVX2)

global sym(Fast9Strengths32_Asm_CMOV_X86_AVX2)
global sym(Fast9Strengths32_Asm_X86_AVX2)
global sym(Fast12Strengths32_Asm_CMOV_X86_AVX2)
global sym(Fast12Strengths32_Asm_X86_AVX2)

section .data
	extern sym(k1_i8)
	extern sym(k254_u8)
	extern sym(kFast9Arcs)
	extern sym(kFast12Arcs)
	extern sym(Fast9Flags)
	extern sym(Fast12Flags)

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
	
	; Compute ymmThreshold and save
	vpbroadcastb ymm7, xmm7 ;  ymm7 = ymmThreshold
	vmovdqa [rsp + 2872], ymm7

	; Compute ymmNMinusOne and save
	vpbroadcastb ymm0, xmm0
	vmovdqa [rsp + 280], ymm0

	; ymm7 = ymmZero
	; ymm7 must be saved/restored before/after calling FastStrengths32
	vpxor ymm7, ymm7
	
	;-------------------
	;StartOfLooopRows
	;
	.LoopRows
	; -------------------
	vmovdqu ymm6, [rbx]

	; Motion Estimation
	; TODO(dmi): not supported
	; TODO(dmi): inc IPprev here

	; cleanup strengths
	mov rax, arg(6)
	vmovdqu [rax], ymm7

	vpsubusb ymm5, ymm6, [rsp + 2872] ; ymm5 = ymmDarker
	vpaddusb ymm6, ymm6, [rsp + 2872] ; ymm6 = ymmBrighter


	;
	; Speed-Test-1
	;

	; compare I1 and I9 aka 0 and 8
	vpcmpeqb ymm4, ymm4  ; ymm4 = ymmFF
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
	vpcmpeqb ymm2, ymm7
	vpcmpeqb ymm3, ymm7
	vpcmpeqb ymm0, ymm7
	vpcmpeqb ymm1, ymm7
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
	vpcmpeqb ymm4, ymm4  ; ymm4 = ymmFF
	mov rdx, arg(3) ; pixels16
	mov rax, [rdx + 4*COMPV_YASM_REG_SZ_BYTES] ; pixels16[4]
	mov rdx, [rdx + 12*COMPV_YASM_REG_SZ_BYTES] ; pixels16[12]
	vmovdqu ymm0, [rbx + rax] ; IP[pixels16[4]]
	vmovdqu ymm1, [rbx + rdx] ; IP[pixels16[12]]
	vpsubusb ymm2, ymm5, ymm0 ; ddarkers16x32[4]
	vpsubusb ymm3, ymm5, ymm1 ; ddarkers16x32[12]
	vpsubusb ymm0, ymm6 ; dbrighters16x32[4]
	vpsubusb ymm1, ymm6 ; dbrighters16x32[12]
	vmovdqa [rsp + 1848 + 4*32], ymm2
	vmovdqa [rsp + 1848 + 12*32], ymm3
	vmovdqa [rsp + 2360 + 4*32], ymm0
	vmovdqa [rsp + 2360 + 12*32], ymm1
	vpcmpeqb ymm2, ymm7
	vpcmpeqb ymm3, ymm7
	vpcmpeqb ymm0, ymm7
	vpcmpeqb ymm1, ymm7
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
	vmovdqa ymm4, [sym(k1_i8)]
	vpsubusb ymm0, ymm5, [rsp + 1336 + 1*32]
	vpsubusb ymm1, ymm5, [rsp + 1336 + 2*32]
	vpsubusb ymm2, ymm5, [rsp + 1336 + 3*32]
	vpsubusb ymm3, ymm5, [rsp + 1336 + 5*32]
	vmovdqa [rsp + 1848 + 1*32], ymm0
	vmovdqa [rsp + 1848 + 2*32], ymm1
	vmovdqa [rsp + 1848 + 3*32], ymm2
	vmovdqa [rsp + 1848 + 5*32], ymm3
	vpcmpeqb ymm0, ymm7
	vpcmpeqb ymm1, ymm7
	vpcmpeqb ymm2, ymm7
	vpcmpeqb ymm3, ymm7
	vpandn ymm0, ymm4
	vpandn ymm1, ymm4
	vpandn ymm2, ymm4
	vpandn ymm3, ymm4
	vpaddusb ymm0, ymm1
	vpaddusb ymm2, ymm3
	vpaddusb ymm0, ymm2
	vmovdqa [rsp + 312 + 1*32], ymm0 ; ymmDarkersFlags[1] = 1 + 2 + 3 + 5
	vpsubusb ymm0, ymm5, [rsp + 1336 + 6*32]
	vpsubusb ymm1, ymm5, [rsp + 1336 + 7*32]
	vpsubusb ymm2, ymm5, [rsp + 1336 + 9*32]
	vpsubusb ymm3, ymm5, [rsp + 1336 + 10*32]
	vmovdqa [rsp + 1848 + 6*32], ymm0
	vmovdqa [rsp + 1848 + 7*32], ymm1
	vmovdqa [rsp + 1848 + 9*32], ymm2
	vmovdqa [rsp + 1848 + 10*32], ymm3
	vpcmpeqb ymm0, ymm7
	vpcmpeqb ymm1, ymm7
	vpcmpeqb ymm2, ymm7
	vpcmpeqb ymm3, ymm7
	vpandn ymm0, ymm4
	vpandn ymm1, ymm4
	vpandn ymm2, ymm4
	vpandn ymm3, ymm4
	vpaddusb ymm0, ymm1
	vpaddusb ymm2, ymm3
	vpaddusb ymm0, ymm2
	vmovdqa [rsp + 312 + 6*32], ymm0 ; ymmDarkersFlags[6] = 6 + 7 + 9 + 10
	vpsubusb ymm0, ymm5, [rsp + 1336 + 11*32]
	vpsubusb ymm1, ymm5, [rsp + 1336 + 13*32]
	vpsubusb ymm2, ymm5, [rsp + 1336 + 14*32]
	vpsubusb ymm3, ymm5, [rsp + 1336 + 15*32]
	vmovdqa [rsp + 1848 + 11*32], ymm0
	vmovdqa [rsp + 1848 + 13*32], ymm1
	vmovdqa [rsp + 1848 + 14*32], ymm2
	vmovdqa [rsp + 1848 + 15*32], ymm3
	vpcmpeqb ymm0, ymm7
	vpcmpeqb ymm1, ymm7
	vpcmpeqb ymm2, ymm7
	vpcmpeqb ymm3, ymm7
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
	vmovdqa ymm5, [sym(k1_i8)]
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
	vpcmpeqb ymm0, ymm7
	vpcmpeqb ymm1, ymm7
	vpcmpeqb ymm2, ymm7
	vpcmpeqb ymm3, ymm7
	vpcmpeqb ymm4, ymm7
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
	vpcmpeqb ymm0, ymm7
	vpcmpeqb ymm1, ymm7
	vpcmpeqb ymm2, ymm7
	vpcmpeqb ymm3, ymm7
	vpcmpeqb ymm4, ymm7
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
	vmovdqa ymm4, [sym(k1_i8)]
	vmovdqa ymm0, [rsp + 1336 + 14*32]
	vmovdqa ymm1, [rsp + 1336 + 15*32]
	vpsubusb ymm0, ymm6
	vpsubusb ymm1, ymm6
	vmovdqa [rsp + 2360 + 14*32], ymm0
	vmovdqa [rsp + 2360 + 15*32], ymm1
	vpcmpeqb ymm0, ymm7
	vpcmpeqb ymm1, ymm7
	vpandn ymm0, ymm5
	vpandn ymm1, ymm5
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
	%assign i 0
	%rep    4
		vpcmpeqb ymm0, ymm7, [rsp + 2360 +(0+i)*32]
		vpcmpeqb ymm1, ymm7, [rsp + 2360 +(1+i)*32]
		vpcmpeqb ymm2, ymm7, [rsp + 2360 +(2+i)*32]
		vpcmpeqb ymm3, ymm7, [rsp + 2360 +(3+i)*32]
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
	test eax, eax
	jz .NeitherDarkersNorBrighters
	; call FastStrengths32(compv_scalar_t rbrighters, compv_scalar_t rdarkers, COMPV_ALIGNED(SSE) const uint8_t* dbrighters16x32, COMPV_ALIGNED(SSE) const uint8_t* ddarkers16x32, const compv_scalar_t(*fbrighters16)[16], const compv_scalar_t(*fdarkers16)[16], uint8_t* strengths32, compv_scalar_t N);
	mov rax, rsp ; save rsp before reserving params, must not be one of the registers used to save the params (rcx, rdx, r8, r9, rdi, rsi)
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
	vpxor ymm7, ymm7 ; restore ymm7
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
%macro FastStrengths32_Asm_X86_AVX2 2
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_XMM 7 ;XMM[6-n]
	push rsi
	push rdi
	push rbx
	; end prolog

	; alloc memory
	sub rsp, 8 + 8 + 8 + 8
	; [rsp + 0] = (1 << p)
	; [rsp + 8] = (1 << g)
	; [rsp + 16] = maxnLow
	; [rsp + 24] = maxnHigh

	vzeroupper

	vpxor ymm0, ymm0

	; FAST hard-coded flags
	%if %2 == 9
		vmovdqa ymm7, [sym(Fast9Flags) + 0] ; xmmFastXFlags
	%elif %2 == 12
		vmovdqa ymm7, [sym(Fast12Flags) + 0] ; xmmFastXFlags
	%else
		%error "not supported"
	%endif

	xor rdx, rdx ; rdx = p = 0
	mov rax, 1<<0
	mov rcx, 1<<16
	mov [rsp + 0], rax ; (1<<p)
	mov [rsp + 8], rcx ; (1<<g)

	;----------------------
	; Loop Start
	;----------------------
	.LoopStart
		xor rcx, rcx
		mov [rsp + 16], rcx ; maxnLow = 0
		mov [rsp + 24], rcx ; maxnHigh = 0

		; ---------
		; Brighters
		; ---------
		mov rsi, [rsp + 0] ; (1<<p)
		or rsi, [rsp + 8] ; (1<<p) | (1<<g)
		test arg(0), rsi ; (rb & (1 << p) || rb & (1 << g)) ?
		jz .EndOfBrighters
		mov rax, arg(4) ; &fbrighters16[p]
		mov rdi, [rax + rdx*COMPV_YASM_REG_SZ_BYTES] ; fbrighters16[p]
		; brighters flags
		vmovd xmm5, edi
		vpbroadcastw xmm5, xmm5
		vinsertf128 ymm5, ymm5, xmm5, 1 ; ymm5 = ymmFLow
		shr rdi, 16
		vmovd xmm6, edi
		vpbroadcastw xmm6, xmm6
		vinsertf128 ymm6, ymm6, xmm6, 1 ; ymm6 = ymmFHigh

		vpand ymm5, ymm5, ymm7
		vpcmpeqw ymm5, ymm5, ymm7
		vpand ymm6, ymm6, ymm7
		vpcmpeqw ymm6, ymm6, ymm7
		COMPV_PACKS_EPI16_AVX2 ymm5, ymm5, ymm6
		vpmovmskb eax, ymm5
		test eax, eax ; rax = r0
		jz .EndOfBrighters
		; Load dbrighters
		mov rbx, arg(2) ; dbrighters16x32
		mov rsi, rdx ; rsi = p
		shl rsi, 5 ; p*32 
		vmovdqa ymm2, [rbx + rsi]
		vextractf128 xmm5, ymm2, 0x1
		; Compute minimum hz (low)
		COMPV_FEATURE_FAST_DETE_HORIZ_MIN_AVX2 Brighters, %1, %2, xmm2, xmm0, xmm1, xmm3, xmm4 ; This macro overrides rax, rsi, rdi and set the result in rcx
		mov [rsp + 16], rcx ; maxnLow
		; Compute minimum hz (high)
		xor rcx, rcx
		shr rax, 16 ; rax = r1 = r0 >> 16
		COMPV_FEATURE_FAST_DETE_HORIZ_MIN_AVX2 Brighters, %1, %2, xmm5, xmm0, xmm1, xmm3, xmm4 ; This macro overrides rax, rsi, rdi and set the result in rcx
		mov [rsp + 24], rcx ; maxnHigh
		.EndOfBrighters

		; ---------
		; Darkers
		; ---------
		.Darkers
		mov rsi, [rsp + 0] ; (1<<p)
		or rsi, [rsp + 8] ; (1<<p) | (1<<g)
		test arg(1), rsi ; (rd & (1 << p) || rd & (1 << g)) ?
		jz .EndOfDarkers
		mov rax, arg(5) ; &fdarkers16[p]
		mov rdi, [rax + rdx*COMPV_YASM_REG_SZ_BYTES] ; fdarkers16[p]
		; darkers flags
		vmovd xmm5, edi
		vpbroadcastw xmm5, xmm5
		vinsertf128 ymm5, ymm5, xmm5, 1 ; ymm5 = ymmFLow
		shr rdi, 16
		vmovd xmm6, edi
		vpbroadcastw xmm6, xmm6
		vinsertf128 ymm6, ymm6, xmm6, 1 ; ymm6 = ymmFHigh
		
		vpand ymm5, ymm5, ymm7
		vpcmpeqw ymm5, ymm5, ymm7
		vpand ymm6, ymm6, ymm7
		vpcmpeqw ymm6, ymm6, ymm7
		COMPV_PACKS_EPI16_AVX2 ymm5, ymm5, ymm6
		vpmovmskb eax, ymm5
		test eax, eax ; rax = r0
		jz .EndOfDarkers
		; Load ddarkers
		mov rbx, arg(3) ; ddarkers16x32
		mov rsi, rdx ; rsi = p
		shl rsi, 5 ; p*32 
		vmovdqa ymm2, [rbx + rsi]
		vextractf128 xmm5, ymm2, 0x1
		; Compute minimum hz (low)
		mov rcx, [rsp + 16] ; load rcx with privious max
		COMPV_FEATURE_FAST_DETE_HORIZ_MIN_AVX2 Darkers, %1, %2, xmm2, xmm0, xmm1, xmm3, xmm4 ; This macro overrides rax, rsi, rdi and set the result in rcx
		mov [rsp + 16], rcx ; maxnLow
		; Compute minimum hz (high)
		mov rcx, [rsp + 24] ; load rcx with privious max
		shr rax, 16 ; rax = r1 = r0 >> 16
		COMPV_FEATURE_FAST_DETE_HORIZ_MIN_AVX2 Brighters, %1, %2, xmm5, xmm0, xmm1, xmm3, xmm4 ; This macro overrides rax, rsi, rdi and set the result in rcx
		.EndOfDarkers
		
		; compute strenghts[p]
		mov rax, arg(6) ; &strengths32
		mov [rax + rdx + 16], byte cl ; strengths16[g] = maxnHigh
		mov rcx, [rsp + 16] ; maxnLow
		mov [rax + rdx + 0], byte cl ; trengths16[p] = maxnLow
	
		inc rdx ; p+= 1

		mov rax, [rsp + 0] ; (1<<p)
		mov rcx, [rsp + 8] ; (1<<g)

		shl rax, 1
		shl rcx, 1
		cmp rdx, 16
		mov [rsp + 0], rax
		mov [rsp + 8], rcx
	jl .LoopStart
	;----------------

	vzeroupper

	; free memory
	add rsp, 8 + 8 + 8 + 8


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
;;; compv_scalar_t Fast9Strengths32_Asm_CMOV_X86_AVX2(COMPV_ALIGNED(AVX) const int16_t(&dbrighters)[16], COMPV_ALIGNED(AVX) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
sym(Fast9Strengths32_Asm_CMOV_X86_AVX2):
	FastStrengths32_Asm_X86_AVX2 1, 9

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t Fast9Strengths32_Asm_X86_AVX2(COMPV_ALIGNED(AVX) const int16_t(&dbrighters)[16], COMPV_ALIGNED(AVX) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
sym(Fast9Strengths32_Asm_X86_AVX2):
	FastStrengths32_Asm_X86_AVX2 0, 9

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t Fast12Strengths32_Asm_CMOV_X86_AVX2(COMPV_ALIGNED(AVX) const int16_t(&dbrighters)[16], COMPV_ALIGNED(AVX) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
sym(Fast12Strengths32_Asm_CMOV_X86_AVX2):
	FastStrengths32_Asm_X86_AVX2 1, 12

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t Fast12Strengths32_Asm_X86_AVX2(COMPV_ALIGNED(AVX) const int16_t(&dbrighters)[16], COMPV_ALIGNED(AVX) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
sym(Fast12Strengths32_Asm_X86_AVX2):
	FastStrengths32_Asm_X86_AVX2 0, 12
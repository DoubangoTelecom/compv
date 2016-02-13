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

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* IP
; arg(1) -> const uint8_t* IPprev
; arg(2) -> compv_scalar_t width
; arg(3) -> const compv_scalar_t(&pixels16)[16]
; arg(4) -> compv_scalar_t N
; arg(5) -> compv_scalar_t threshold
; arg(6) -> COMPV_ALIGNED(AVX2) compv_scalar_t(*pfdarkers16)[16]
; arg(7) -> COMPV_ALIGNED(AVX2) compv_scalar_t(*pfbrighters16)[16]
; arg(8) -> COMPV_ALIGNED(AVX2) uint8_t* ddarkers16x32
; arg(9) -> COMPV_ALIGNED(AVX2) uint8_t* dbrighters16x32
; arg(10) -> compv_scalar_t* rd
; arg(11) -> compv_scalar_t* rb
; arg(12) -> compv_scalar_t* me
; void FastData32Row_Asm_X86_AVX2(const uint8_t* IP, const uint8_t* IPprev, compv_scalar_t width, const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, COMPV_ALIGNED(AVX2) compv_scalar_t(*pfdarkers16)[16], COMPV_ALIGNED(AVX2) compv_scalar_t(*pfbrighters16)[16], COMPV_ALIGNED(AVX2) uint8_t* ddarkers16x32, COMPV_ALIGNED(AVX2) uint8_t* dbrighters16x32, compv_scalar_t* rd, compv_scalar_t* rb, compv_scalar_t* me);
sym(FastData32Row_Asm_X86_AVX2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 13
	push rsi
	push rdi
	push rbx
	; end prolog

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 32, rax
	sub rsp, 8 + 32 + 16*32 + 16*32 + 16*32
	; [rsp + 0] = sum
	; [rsp + 8] = ymmNMinusOne
	; [rsp + 40] = ymmDarkersFlags[16]
	; [rsp + 552] = ymmBrightersFlags[16]
	; [rsp + 1064] = ymmDataPtr[16]

	vzeroupper

	mov rsi, arg(2) ; rsi = width
	mov rax, arg(5) ; threshold
	mov rbx, arg(0) ; rbx = IP
	vmovd xmm7, eax
	vpbroadcastb ymm7, xmm7 ;  ymm7 = ymmThreshold

	; Compute xmmNMinusOne
	mov rax, arg(4) ; N
	sub rax, 1
	movd xmm0, eax
	vpbroadcastb ymm0, xmm0
	vmovdqa [rsp + 8], ymm0
	
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

	vmovdqu ymm6, [rbx]

	; Motion Estimation
	; TODO(dmi): not supported
	; TODO(dmi): inc IPprev here

	vmovdqa ymm5, ymm6
	vpaddusb ymm6, ymm7 ; ymm6 = ymmBrighter
	vpsubusb ymm5, ymm7 ; ymm5 = ymmDarker

	;
	; Speed-Test-1
	;

	; compare I1 and I9 aka 0 and 8
	mov rdx, arg(3) ; pixels16
	mov rax, [rdx + 0*COMPV_YASM_REG_SZ_BYTES] ; pixels16[0]
	mov rdx, [rdx + 8*COMPV_YASM_REG_SZ_BYTES] ; pixels16[8]
	vpxor ymm4, ymm4 ; ymm4 = ymmZeros
	vmovdqu ymm0, [rbx + rax] ; IP[pixels16[0]]
	vmovdqu ymm1, [rbx + rdx] ; IP[pixels16[8]]
	mov rdx, arg(8) ; ddarkers16x32
	mov rax, arg(9) ; dbrighters16x32
	vpsubusb ymm2, ymm5, ymm0 ; ddarkers16x32[0]
	vpsubusb ymm3, ymm5, ymm1 ; ddarkers16x32[8]
	vpsubusb ymm0, ymm6 ; dbrighters16x32[0]
	vpsubusb ymm1, ymm6 ; dbrighters16x32[8]
	vmovdqa [rdx + 0*32], ymm2
	vmovdqa [rdx + 8*32], ymm3
	vmovdqa [rax + 0*32], ymm0
	vmovdqa [rax + 8*32], ymm1
	vpcmpeqb ymm2, ymm4
	vpcmpeqb ymm3, ymm4
	vpcmpeqb ymm0, ymm4
	vpcmpeqb ymm1, ymm4
	vpcmpeqb ymm4, ymm4  ; ymm4 = ymmFF
	vpandn ymm2, ymm4
	vpandn ymm3, ymm4
	vpandn ymm0, ymm4
	vpandn ymm1, ymm4
	vmovdqa [rsp + 40 + 0*32], ymm2 ; ymmDarkersFlags[0]
	vmovdqa [rsp + 40 + 8*32], ymm3 ; ymmDarkersFlags[8]
	vmovdqa [rsp + 552 + 0*32], ymm0 ; ymmBrightersFlags[0]
	vmovdqa [rsp + 552 + 8*32], ymm1 ; ymmBrightersFlags[8]
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
	vpxor ymm4, ymm4 ; ymm4 = ymmZeros
	vmovdqu ymm0, [rbx + rax] ; IP[pixels16[4]]
	vmovdqu ymm1, [rbx + rdx] ; IP[pixels16[12]]
	mov rdx, arg(8) ; ddarkers16x32
	mov rax, arg(9) ; dbrighters16x32
	vpsubusb ymm2, ymm5, ymm0 ; ddarkers16x32[4]
	vpsubusb ymm3, ymm5, ymm1 ; ddarkers16x32[12]
	vpsubusb ymm0, ymm6 ; dbrighters16x32[4]
	vpsubusb ymm1, ymm6 ; dbrighters16x32[12]
	vmovdqa [rdx + 4*32], ymm2
	vmovdqa [rdx + 12*32], ymm3
	vmovdqa [rax + 4*32], ymm0
	vmovdqa [rax + 12*32], ymm1
	vpcmpeqb ymm2, ymm4
	vpcmpeqb ymm3, ymm4
	vpcmpeqb ymm0, ymm4
	vpcmpeqb ymm1, ymm4
	vpcmpeqb ymm4, ymm4  ; ymm4 = ymmFF
	vpandn ymm2, ymm4
	vpandn ymm3, ymm4
	vpandn ymm0, ymm4
	vpandn ymm1, ymm4
	vmovdqa [rsp + 40 + 4*32], ymm2 ; ymmDarkersFlags[4]
	vmovdqa [rsp + 40 + 12*32], ymm3 ; ymmDarkersFlags[12]
	vmovdqa [rsp + 552 + 4*32], ymm0 ; ymmBrightersFlags[4]
	vmovdqa [rsp + 552 + 12*32], ymm1 ; ymmBrightersFlags[12]
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
	vmovdqa ymm0, [rsp + 552 + 0*32] ; ymmBrightersFlags[0]
	vmovdqa ymm1, [rsp + 552 + 4*32] ; ymmBrightersFlags[4]
	vpor ymm0, [rsp + 552 + 8*32] ; ymmBrightersFlags[0] | ymmBrightersFlags[8]
	vpor ymm1, [rsp + 552 + 12*32] ; ymmBrightersFlags[4] | ymmBrightersFlags[12]
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
	vmovdqa ymm0, [rsp + 40 + 0*32] ; ymmDarkersFlags[0]
	vmovdqa ymm1, [rsp + 40 + 4*32] ; ymmDarkersFlags[4]
	vpor ymm0, [rsp + 40 + 8*32] ; ymmDarkersFlags[0] | ymmDarkersFlags[8]
	vpor ymm1, [rsp + 40 + 12*32] ; ymmDarkersFlags[4] | ymmDarkersFlags[12]
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
	vmovdqa [rsp + 1064 + 1*32], ymm0
	vmovdqa [rsp + 1064 + 2*32], ymm1
	vmovdqa [rsp + 1064 + 3*32], ymm2
	vmovdqa [rsp + 1064 + 5*32], ymm3
	vmovdqa [rsp + 1064 + 6*32], ymm4
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
	vmovdqa [rsp + 1064 + 7*32], ymm0
	vmovdqa [rsp + 1064 + 9*32], ymm1
	vmovdqa [rsp + 1064 + 10*32], ymm2
	vmovdqa [rsp + 1064 + 11*32], ymm3
	vmovdqa [rsp + 1064 + 13*32], ymm4
	mov rax, [rcx + 14*COMPV_YASM_REG_SZ_BYTES] ; pixels16[14]
	vmovdqu ymm0, [rbx + rax]
	mov rax, [rcx + 15*COMPV_YASM_REG_SZ_BYTES] ; pixels16[15]
	vmovdqu ymm1, [rbx + rax]
	vmovdqa [rsp + 1064 + 14*32], ymm0
	vmovdqa [rsp + 1064 + 15*32], ymm1

	; We could compute pixels at 1 and 9, check if at least one is darker or brighter than the candidate
	; Then, do the same for 2 and 10 etc etc ... but this is slower than whant we're doing below because
	; _mm_movemask_epi8 is cyclyvore

	;
	;	LoadDarkers
	;
	test dl, dl ; rdx was loadD, now it's free
	jz .EndOfDarkers
	; compute ddarkers16x32 and flags
	mov rax, arg(8) ; ddarkers16x32
	vpxor ymm4, ymm4
	vpsubusb ymm0, ymm5, [rsp + 1064 + 1*32]
	vpsubusb ymm1, ymm5, [rsp + 1064 + 2*32]
	vpsubusb ymm2, ymm5, [rsp + 1064 + 3*32]
	vpsubusb ymm3, ymm5, [rsp + 1064 + 5*32]
	vmovdqa [rax + 1*32], ymm0
	vmovdqa [rax + 2*32], ymm1
	vmovdqa [rax + 3*32], ymm2
	vmovdqa [rax + 5*32], ymm3
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
	vmovdqa [rsp + 40 + 1*32], ymm0 ; ymmDarkersFlags[1] = 1 + 2 + 3 + 5
	vpxor ymm4, ymm4
	vpsubusb ymm0, ymm5, [rsp + 1064 + 6*32]
	vpsubusb ymm1, ymm5, [rsp + 1064 + 7*32]
	vpsubusb ymm2, ymm5, [rsp + 1064 + 9*32]
	vpsubusb ymm3, ymm5, [rsp + 1064 + 10*32]
	vmovdqa [rax + 6*32], ymm0
	vmovdqa [rax + 7*32], ymm1
	vmovdqa [rax + 9*32], ymm2
	vmovdqa [rax + 10*32], ymm3
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
	vmovdqa [rsp + 40 + 6*32], ymm0 ; ymmDarkersFlags[6] = 6 + 7 + 9 + 10
	vpxor ymm4, ymm4
	vpsubusb ymm0, ymm5, [rsp + 1064 + 11*32]
	vpsubusb ymm1, ymm5, [rsp + 1064 + 13*32]
	vpsubusb ymm2, ymm5, [rsp + 1064 + 14*32]
	vpsubusb ymm3, ymm5, [rsp + 1064 + 15*32]
	vmovdqa [rax + 11*32], ymm0
	vmovdqa [rax + 13*32], ymm1
	vmovdqa [rax + 14*32], ymm2
	vmovdqa [rax + 15*32], ymm3
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
	vmovdqa [rsp + 40 + 11*32], ymm0 ; ymmDarkersFlags[11] = 11 + 13 + 14 + 15
	; Compute flags 0, 4, 8, 12
	vmovdqa ymm5, [sym(k254_u8)]
	vmovdqa ymm4, [rsp + 8] ; ymmNMinusOne
	vpandn ymm0, ymm5, [rsp + 40 + 0*32]
	vpandn ymm1, ymm5, [rsp + 40 + 4*32]
	vpandn ymm2, ymm5, [rsp + 40 + 8*32]
	vpandn ymm3, ymm5, [rsp + 40 + 12*32]
	vpaddusb ymm0, ymm1
	vpaddusb ymm2, ymm3
	vpaddusb ymm0, ymm2 ; ymm0 = 0 + 4 + 8 + 12
	vpaddusb ymm0, [rsp + 40 + 1*32] ; ymm0 += 1 + 2 + 3 + 5
	vpaddusb ymm0, [rsp + 40 + 6*32] ; ymm0 += 6 + 7 + 9 + 10
	vpaddusb ymm0, [rsp + 40 + 11*32] ; ymm0 += 11 + 13 + 14 + 15
	; Check the columns with at least N non-zero bits
	vpcmpgtb ymm0, ymm4
	vpmovmskb edx, ymm0
	test edx, edx
	jz .EndOfDarkers
	; Continue loading darkers
	mov rcx, arg(10) ; rd
	mov [rcx], edx ; (*rd) = colDarkersFlags
	; Transpose
	COMPV_TRANSPOSE_I8_16X16_REG_T5_AVX2 rax+0*32, rax+1*32, rax+2*32, rax+3*32, rax+4*32, rax+5*32, rax+6*32, rax+7*32, rax+8*32, rax+9*32, rax+10*32, rax+11*32, rax+12*32, rax+13*32, rax+14*32, rax+15*32, ymm0, ymm1, ymm2, ymm3, ymm4
	; Flags
	vpcmpeqb ymm5, ymm5 ; ymmFF
	vpxor ymm4, ymm4 ; ymmZeros
	mov rdx, arg(6) ; pfdarkers16
	%assign i 0
	%rep    4
		mov rax, arg(8) ; ddarkers16x32
		vpcmpeqb ymm0, ymm4, [rax+(0+i)*32]
		vpcmpeqb ymm1, ymm4, [rax+(1+i)*32]
		vpcmpeqb ymm2, ymm4, [rax+(2+i)*32]
		vpcmpeqb ymm3, ymm4, [rax+(3+i)*32]
		vpandn ymm0, ymm5
		vpandn ymm1, ymm5
		vpandn ymm2, ymm5
		vpandn ymm3, ymm5
		vpmovmskb eax, ymm0
		vpmovmskb ecx, ymm1
		mov [rdx + (0+i)*COMPV_YASM_REG_SZ_BYTES], eax
		mov [rdx + (1+i)*COMPV_YASM_REG_SZ_BYTES], ecx
		vpmovmskb eax, ymm2
		vpmovmskb ecx, ymm3
		mov [rdx + (2+i)*COMPV_YASM_REG_SZ_BYTES], eax
		mov [rdx + (3+i)*COMPV_YASM_REG_SZ_BYTES], ecx
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
	mov rax, arg(9) ; dbrighters16x32
	vmovdqa ymm0, [rsp + 1064 + 1*32]
	vmovdqa ymm1, [rsp + 1064 + 2*32]
	vmovdqa ymm2, [rsp + 1064 + 3*32]
	vmovdqa ymm3, [rsp + 1064 + 5*32]
	vmovdqa ymm4, [rsp + 1064 + 6*32]
	vpsubusb ymm0, ymm6
	vpsubusb ymm1, ymm6
	vpsubusb ymm2, ymm6
	vpsubusb ymm3, ymm6
	vpsubusb ymm4, ymm6
	vmovdqa [rax + 1*32], ymm0
	vmovdqa [rax + 2*32], ymm1
	vmovdqa [rax + 3*32], ymm2
	vmovdqa [rax + 5*32], ymm3
	vmovdqa [rax + 6*32], ymm4
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
	vmovdqa [rsp + 552 + 1*32], ymm0 ; ymmBrightersFlags[1] = 1 + 2 + 3 + 5 + 6
	vpxor ymm5, ymm5
	vmovdqa ymm0, [rsp + 1064 + 7*32]
	vmovdqa ymm1, [rsp + 1064 + 9*32]
	vmovdqa ymm2, [rsp + 1064 + 10*32]
	vmovdqa ymm3, [rsp + 1064 + 11*32]
	vmovdqa ymm4, [rsp + 1064 + 13*32]
	vpsubusb ymm0, ymm6
	vpsubusb ymm1, ymm6
	vpsubusb ymm2, ymm6
	vpsubusb ymm3, ymm6
	vpsubusb ymm4, ymm6
	vmovdqa [rax + 7*32], ymm0
	vmovdqa [rax + 9*32], ymm1
	vmovdqa [rax + 10*32], ymm2
	vmovdqa [rax + 11*32], ymm3
	vmovdqa [rax + 13*32], ymm4
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
	vmovdqa [rsp + 552 + 7*32], ymm0 ; ymmBrightersFlags[7] = 7 + 9 + 10 + 11 + 13
	vpxor ymm5, ymm5
	vmovdqa ymm4, [sym(k1_i8)]
	vmovdqa ymm0, [rsp + 1064 + 14*32]
	vmovdqa ymm1, [rsp + 1064 + 15*32]
	vpsubusb ymm0, ymm6
	vpsubusb ymm1, ymm6
	vmovdqa [rax + 14*32], ymm0
	vmovdqa [rax + 15*32], ymm1
	vpcmpeqb ymm0, ymm5
	vpcmpeqb ymm1, ymm5
	vpandn ymm0, ymm4
	vpandn ymm1, ymm4
	vpaddusb ymm0, ymm1
	vmovdqa [rsp + 552 + 14*32], ymm0 ; ymmBrightersFlags[14] = 14 + 15	
	; Compute flags 0, 4, 8, 12
	vmovdqa ymm6, [sym(k254_u8)]
	vmovdqa ymm4, [rsp + 8] ; ymmNMinusOne
	vpandn ymm0, ymm6, [rsp + 552 + 0*32]
	vpandn ymm1, ymm6, [rsp + 552 + 4*32]
	vpandn ymm2, ymm6, [rsp + 552 + 8*32]
	vpandn ymm3, ymm6, [rsp + 552 + 12*32]
	vpaddusb ymm0, ymm1
	vpaddusb ymm2, ymm3
	vpaddusb ymm0, ymm2 ; ymm0 = 0 + 4 + 8 + 12
	vpaddusb ymm0, [rsp + 552 + 1*32] ; ymm0 += 1 + 2 + 3 + 5 + 6
	vpaddusb ymm0, [rsp + 552 + 7*32] ; ymm0 += 7 + 9 + 10 + 11 + 13
	vpaddusb ymm0, [rsp + 552 + 14*32] ; ymm0 += 14 + 15
	; Check the columns with at least N non-zero bits
	vpcmpgtb ymm0, ymm4
	vpmovmskb edx, ymm0
	test edx, edx
	jz .EndOfBrighters
	; Continue loading brighters
	mov rcx, arg(11) ; rb
	mov rax, arg(9) ; dbrighters16x32
	mov [rcx], edx ; (*rb) = colBrightersFlags
	; Transpose
	COMPV_TRANSPOSE_I8_16X16_REG_T5_AVX2 rax+0*32, rax+1*32, rax+2*32, rax+3*32, rax+4*32, rax+5*32, rax+6*32, rax+7*32, rax+8*32, rax+9*32, rax+10*32, rax+11*32, rax+12*32, rax+13*32, rax+14*32, rax+15*32, ymm0, ymm1, ymm2, ymm3, ymm4
	; Flags
	vpcmpeqb ymm6, ymm6 ; ymmFF
	vpxor ymm4, ymm4 ; ymmZeros
	mov rdx, arg(7) ; pfbrighters16
	%assign i 0
	%rep    4
		vpcmpeqb ymm0, ymm4, [rax+(0+i)*32]
		vpcmpeqb ymm1, ymm4, [rax+(1+i)*32]
		vpcmpeqb ymm2, ymm4, [rax+(2+i)*32]
		vpcmpeqb ymm3, ymm4, [rax+(3+i)*32]
		vpandn ymm0, ymm6
		vpandn ymm1, ymm6
		vpandn ymm2, ymm6
		vpandn ymm3, ymm6
		vpmovmskb edi, ymm0
		vpmovmskb ecx, ymm1
		mov [rdx + (0+i)*COMPV_YASM_REG_SZ_BYTES], edi
		mov [rdx + (1+i)*COMPV_YASM_REG_SZ_BYTES], ecx
		vpmovmskb edi, ymm2
		vpmovmskb ecx, ymm3
		mov [rdx + (2+i)*COMPV_YASM_REG_SZ_BYTES], edi
		mov [rdx + (3+i)*COMPV_YASM_REG_SZ_BYTES], ecx
		%assign i i+4
	%endrep

	.EndOfBrighters
	
	.LoopRowsNext

	; TODO(dmi): do the same as x64, increment these values only if needed
	lea rbx, [rbx + 32] ; IP += 32
	mov rdx, COMPV_YASM_REG_SZ_BYTES
	add arg(11), rdx ; rb += 1
	add arg(10), rdx ; rd += 1
	mov rdx, 16*COMPV_YASM_REG_SZ_BYTES
	add arg(6), rdx ; pfdarkers16 += 1
	add arg(7), rdx ; pfbrighters16 += 1
	mov rdx, 16*32
	add arg(8), rdx ; ymmDdarkers16x32 += 1
	add arg(9), rdx ; ymmDbrighters16x32 += 1
	; TODO(dmi): Motion estimation not supported -> do not inc IPprev

	;-------------------
	;EndOfLooopRows
	lea rsi, [rsi - 32]
	test rsi, rsi
	jnz .LoopRows
	;-------------------

	.EndOfFunction

	; unalign stack and free memory
	add rsp, 8 + 32 + 16*32 + 16*32 + 16*32
	COMPV_YASM_UNALIGN_STACK

	vzeroupper

	; begin epilog
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
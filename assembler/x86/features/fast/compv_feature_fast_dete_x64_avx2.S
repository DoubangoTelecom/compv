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

global sym(FastData32Row_Asm_X64_AVX2)

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
; void FastData32Row_Asm_X64_AVX2(const uint8_t* IP, const uint8_t* IPprev, compv_scalar_t width, const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, COMPV_ALIGNED(AVX2) compv_scalar_t(*pfdarkers16)[16], COMPV_ALIGNED(AVX2) compv_scalar_t(*pfbrighters16)[16], COMPV_ALIGNED(AVX2) uint8_t* ddarkers16x32, COMPV_ALIGNED(AVX2) uint8_t* dbrighters16x32, compv_scalar_t* rd, compv_scalar_t* rb, compv_scalar_t* me);
sym(FastData32Row_Asm_X64_AVX2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 13
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	push r14
	push r15
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

	mov rdx, arg(2) ; width
	shr rdx, 5 ; div width with 32 and mov rsi by 1 -> because of argi(x,rsi)
	xor rsi, rsi
	mov arg(2), rdx 
	movzx rax, byte arg(5) ; threshold
	mov rbx, arg(0) ; rbx = IP
	vmovd xmm7, eax
	vpbroadcastb ymm7, xmm7 ;  ymm7 = ymmThreshold

	; Compute xmmNMinusOne
	movzx rax, byte arg(4) ; N
	sub al, 1
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
	mov [rax + rsi*COMPV_YASM_REG_SZ_BYTES], rcx
	mov [rdx + rsi*COMPV_YASM_REG_SZ_BYTES], rcx

	vmovdqu ymm6, [rbx]

	; Motion Estimation
	; TODO(dmi): not supported
	
	vpxor ymm15, ymm15 ; ymm15 = ymmZeros
	vpcmpeqb ymm14, ymm14 ; ymm14 = ymmFF

	vpsubusb ymm5, ymm6, ymm7 ; ymm5 = ymmDarker
	vpaddusb ymm6, ymm6, ymm7 ; ymm6 = ymmBrighter

	;
	; Speed-Test-1
	;

	mov r11, arg(3) ; pixels16
	mov r10, rsi
	mov r8, arg(8) ; ddarkers16x32
	shl r10, 9 ; rsi * (16*32)
	mov r9, arg(9) ; dbrighters16x32
	lea r8, [r8 + r10] ; ddarkers16x32[rsi]
	lea r9, [r9 + r10] ; dbrighters16x32[rsi]

	; compare I1 and I9 aka 0 and 8
	mov rax, [r11 + 0*COMPV_YASM_REG_SZ_BYTES] ; pixels16[0]
	mov rdx, [r11 + 8*COMPV_YASM_REG_SZ_BYTES] ; pixels16[8]
	vmovdqu ymm0, [rbx + rax]
	vmovdqu ymm1, [rbx + rdx]
	vpsubusb ymm2, ymm5, ymm0 ; ddarkers16x32[0]
	vpsubusb ymm3, ymm5, ymm1 ; ddarkers16x32[8]
	vpsubusb ymm0, ymm6 ; dbrighters16x32[0]
	vpsubusb ymm1, ymm6 ; dbrighters16x32[8]
	vmovdqa [r8 + 0*32], ymm2
	vmovdqa [r8 + 8*32], ymm3
	vmovdqa [r9 + 0*32], ymm0
	vmovdqa [r9 + 8*32], ymm1
	vpcmpeqb ymm2, ymm15
	vpcmpeqb ymm3, ymm15
	vpcmpeqb ymm0, ymm15
	vpcmpeqb ymm1, ymm15
	vpandn ymm10, ymm2, ymm14 ; ymmDarkersFlags[0]
	vpandn ymm11, ymm3, ymm14 ; ymmDarkersFlags[8]
	vpandn ymm8, ymm0, ymm14 ; ymmBrightersFlags[0]
	vpandn ymm9, ymm1, ymm14 ; ymmBrightersFlags[8]
	vpor ymm0, ymm8, ymm10
	vpor ymm1, ymm9, ymm11
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
	mov rax, [r11 + 4*COMPV_YASM_REG_SZ_BYTES] ; pixels16[4]
	mov rdx, [r11 + 12*COMPV_YASM_REG_SZ_BYTES] ; pixels16[12]
	vmovdqu ymm0, [rbx + rax]
	vmovdqu ymm1, [rbx + rdx]
	vpsubusb ymm2, ymm5, ymm0 ; ddarkers16x32[4]
	vpsubusb ymm3, ymm5, ymm1 ; ddarkers16x32[12]
	vpsubusb ymm0, ymm6 ; dbrighters16x32[4]
	vpsubusb ymm1, ymm6 ; dbrighters16x32[12]
	vmovdqa [r8 + 4*32], ymm2
	vmovdqa [r8 + 12*32], ymm3
	vmovdqa [r9 + 4*32], ymm0
	vmovdqa [r9 + 12*32], ymm1
	vpcmpeqb ymm2, ymm15 
	vpcmpeqb ymm3, ymm15 
	vpcmpeqb ymm0, ymm15 
	vpcmpeqb ymm1, ymm15 
	vpandn ymm2, ymm14 ; ymmDarkersFlags[4]
	vpandn ymm3, ymm14 ; ymmDarkersFlags[12]
	vpandn ymm0, ymm14 ; ymmBrightersFlags[4]
	vpandn ymm1, ymm14 ; ymmBrightersFlags[12]
	vpor ymm12, ymm0, ymm2
	vpor ymm13, ymm1, ymm3
	vpmovmskb eax, ymm12
	vpmovmskb edx, ymm13
	test eax, eax
	setnz al 
	test edx, edx
	setnz dl
	add dl, al
	test dl, dl
	jz .LoopRowsNext
	add [rsp + 0], dl ; sum = ?

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
	vpor ymm14, ymm10, ymm11 ; ymmDarkersFlags[0] | ymmDarkersFlags[8]
	vpor ymm15, ymm8, ymm9 ; ymmBrightersFlags[0] | ymmBrightersFlags[8]
	vpor ymm12, ymm0, ymm1 ; ymmBrightersFlags[4] | ymmBrightersFlags[12] 
	vpor ymm13, ymm2, ymm3 ; ymmDarkersFlags[4] | ymmDarkersFlags[12]
	vpmovmskb r9d, ymm14
	vpmovmskb r11d, ymm15
	vpmovmskb r12d, ymm12
	vpmovmskb r10d, ymm13
	
	; Check whether to load Brighters
	test r11d, r11d
	setnz cl
	test r12d, r12d
	setnz al
	add cl, al
	cmp cl, 1
	setg dil ; rdi = (rcx > 1) ? 1 : 0
	
	; Check whether to load Darkers
	test r9d, r9d
	setnz cl
	test r10d, r10d
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

	; Save data
	vmovdqa [rsp + 40 + 0*32], ymm10 ; ymmDarkersFlags[0]
	vmovdqa [rsp + 40 + 8*32], ymm11 ; ymmDarkersFlags[8]
	vmovdqa [rsp + 40 + 4*32], ymm2 ; ymmDarkersFlags[4]
	vmovdqa [rsp + 40 + 12*32], ymm3 ; ymmDarkersFlags[12]
	vmovdqa [rsp + 552 + 0*32], ymm8 ; ymmBrightersFlags[0]
	vmovdqa [rsp + 552 + 8*32], ymm9 ; ymmBrightersFlags[8]
	vmovdqa [rsp + 552 + 4*32], ymm0 ; ymmBrightersFlags[4]
	vmovdqa [rsp + 552 + 12*32], ymm1 ; ymmBrightersFlags[12]

	; Load ymmDataPtr
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
	vmovdqu ymm0, [rbx + rax]
	vmovdqu ymm1, [rbx + r8]
	vmovdqu ymm2, [rbx + r9]
	vmovdqu ymm3, [rbx + r10]
	vmovdqu ymm4, [rbx + r11]
	vmovdqu ymm8, [rbx + r12]
	vmovdqu ymm9, [rbx + r13]
	vmovdqu ymm10, [rbx + r14]
	vmovdqu ymm11, [rbx + r15]
	mov r8, [rcx + 13*COMPV_YASM_REG_SZ_BYTES] ; pixels16[13]
	mov r9, [rcx + 14*COMPV_YASM_REG_SZ_BYTES] ; pixels16[14]
	mov r10, [rcx + 15*COMPV_YASM_REG_SZ_BYTES] ; pixels16[15]
	vmovdqu ymm12, [rbx + r8]
	vmovdqu ymm13, [rbx + r9]	
	vmovdqu ymm14, [rbx + r10]
	vmovdqa [rsp + 1064 + 1*32], ymm0
	vmovdqa [rsp + 1064 + 2*32], ymm1
	vmovdqa [rsp + 1064 + 3*32], ymm2
	vmovdqa [rsp + 1064 + 5*32], ymm3
	vmovdqa [rsp + 1064 + 6*32], ymm4
	vmovdqa [rsp + 1064 + 7*32], ymm8
	vmovdqa [rsp + 1064 + 9*32], ymm9
	vmovdqa [rsp + 1064 + 10*32], ymm10
	vmovdqa [rsp + 1064 + 11*32], ymm11
	vmovdqa [rsp + 1064 + 13*32], ymm12
	vmovdqa [rsp + 1064 + 14*32], ymm13
	vmovdqa [rsp + 1064 + 15*32], ymm14

	; We could compute pixels at 1 and 9, check if at least one is darker or brighter than the candidate
	; Then, do the same for 2 and 10 etc etc ... but this is slower than whant we're doing below because
	; _mm_movemask_epi8 is cyclyvore

	;
	;	LoadDarkers
	;
	test dl, dl ; rdx was loadD, now it's free
	jz .EndOfDarkers
	; compute ddarkers16x32 and flags
	mov r9, rsi
	shl r9, 9 ; rsi * (16*32)
	mov rax, arg(8) ; ddarkers16x32
	lea rax, [rax + r9]
	vpxor ymm4, ymm4
	vpsubusb ymm0, ymm5, [rsp + 1064 + 1*32]
	vpsubusb ymm1, ymm5, [rsp + 1064 + 2*32]
	vpsubusb ymm2, ymm5, [rsp + 1064 + 3*32]
	vpsubusb ymm3, ymm5, [rsp + 1064 + 5*32]
	vpsubusb ymm8, ymm5, [rsp + 1064 + 6*32]
	vpsubusb ymm9, ymm5, [rsp + 1064 + 7*32]
	vpsubusb ymm10, ymm5, [rsp + 1064 + 9*32]
	vpsubusb ymm11, ymm5, [rsp + 1064 + 10*32]
	vpsubusb ymm12, ymm5, [rsp + 1064 + 11*32]
	vpsubusb ymm13, ymm5, [rsp + 1064 + 13*32]
	vpsubusb ymm14, ymm5, [rsp + 1064 + 14*32]
	vpsubusb ymm15, ymm5, [rsp + 1064 + 15*32]
	vmovdqa [rax + 1*32], ymm0
	vmovdqa [rax + 2*32], ymm1
	vmovdqa [rax + 3*32], ymm2
	vmovdqa [rax + 5*32], ymm3
	vmovdqa [rax + 6*32], ymm8
	vmovdqa [rax + 7*32], ymm9
	vmovdqa [rax + 9*32], ymm10
	vmovdqa [rax + 10*32], ymm11
	vmovdqa [rax + 11*32], ymm12
	vmovdqa [rax + 13*32], ymm13
	vmovdqa [rax + 14*32], ymm14
	vmovdqa [rax + 15*32], ymm15
	vpcmpeqb ymm0, ymm4
	vpcmpeqb ymm1, ymm4
	vpcmpeqb ymm2, ymm4
	vpcmpeqb ymm3, ymm4
	vpcmpeqb ymm8, ymm4
	vpcmpeqb ymm9, ymm4
	vpcmpeqb ymm10, ymm4
	vpcmpeqb ymm11, ymm4
	vpcmpeqb ymm12, ymm4
	vpcmpeqb ymm13, ymm4
	vpcmpeqb ymm14, ymm4
	vpcmpeqb ymm15, ymm4
	vmovdqa ymm4, [sym(k1_i8)]
	vpandn ymm0, ymm4
	vpandn ymm1, ymm4
	vpandn ymm2, ymm4
	vpandn ymm3, ymm4
	vpandn ymm8, ymm4
	vpandn ymm9, ymm4
	vpandn ymm10, ymm4
	vpandn ymm11, ymm4
	vpandn ymm12, ymm4
	vpandn ymm13, ymm4
	vpandn ymm14, ymm4
	vpandn ymm15, ymm4
	vpaddusb ymm0, ymm1
	vpaddusb ymm2, ymm3
	vpaddusb ymm8, ymm9
	vpaddusb ymm10, ymm11
	vpaddusb ymm12, ymm13
	vpaddusb ymm14, ymm15
	vpaddusb ymm0, ymm2
	vpaddusb ymm8, ymm10
	vpaddusb ymm12, ymm14
	vpaddusb ymm0, ymm8
	vpaddusb ymm12, ymm0 ; ymm12 = 1 + 2 + 3 + 5 + 6 + 7 + 9 + 10 + 11 + 13 + 14 + 15	
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
	vpaddusb ymm0, ymm12 ; ymm0 += 1 + 2 + 3 + 5 + 6 + 7 + 9 + 10 + 11 + 13 + 14 + 15
	; Check the columns with at least N non-zero bits
	vpcmpgtb ymm0, ymm4
	vpmovmskb edx, ymm0
	test edx, edx
	jz .EndOfDarkers
	; Continue loading darkers
	mov rcx, arg(10) ; rd
	mov [rcx + rsi*COMPV_YASM_REG_SZ_BYTES], edx ; (*rd) = colDarkersFlags
	; Transpose
	COMPV_TRANSPOSE_I8_16X16_REG_T5_AVX2 rax+0*32, rax+1*32, rax+2*32, rax+3*32, rax+4*32, rax+5*32, rax+6*32, rax+7*32, rax+8*32, rax+9*32, rax+10*32, rax+11*32, rax+12*32, rax+13*32, rax+14*32, rax+15*32, ymm0, ymm1, ymm2, ymm3, ymm4	
	; Flags
	mov r8, rsi
	mov r9, rsi
	shl r8, COMPV_YASM_REG_SZ_BYTES_TIMES16_LOG2 ; rsi * (16*COMPV_YASM_REG_SZ_BYTES)
	shl r9, 9 ; rsi * (16 * 32)
	vpxor ymm5, ymm5 ; ymmZero
	mov rdx, arg(6) ; pfdarkers16
	lea rdx, [rdx + r8]
	mov rax, arg(8) ; ddarkers16x32
	lea rax, [rax + r9]
	vpcmpeqb ymm0, ymm5, [rax+0*32]
	vpcmpeqb ymm1, ymm5, [rax+1*32]
	vpcmpeqb ymm2, ymm5, [rax+2*32]
	vpcmpeqb ymm3, ymm5, [rax+3*32]
	vpcmpeqb ymm4, ymm5, [rax+4*32]
	vpcmpeqb ymm8, ymm5, [rax+5*32]
	vpcmpeqb ymm9, ymm5, [rax+6*32]
	vpcmpeqb ymm10, ymm5, [rax+7*32]
	vpcmpeqb ymm11, ymm5, [rax+8*32]
	vpcmpeqb ymm12, ymm5, [rax+9*32]
	vpcmpeqb ymm13, ymm5, [rax+10*32]
	vpcmpeqb ymm14, ymm5, [rax+11*32]
	vpcmpeqb ymm15, ymm5, [rax+12*32]
	vpcmpeqb ymm5, ymm5 ; ymmFF
	vpandn ymm0, ymm5
	vpandn ymm1, ymm5
	vpandn ymm2, ymm5
	vpandn ymm3, ymm5
	vpandn ymm4, ymm5
	vpandn ymm8, ymm5
	vpandn ymm9, ymm5
	vpandn ymm10, ymm5
	vpandn ymm11, ymm5
	vpandn ymm12, ymm5
	vpandn ymm13, ymm5
	vpandn ymm14, ymm5
	vpandn ymm15, ymm5
	push rax ; save rax 
	vpmovmskb eax, ymm0
	vpmovmskb ecx, ymm1
	vpmovmskb r8d, ymm2
	vpmovmskb r9d, ymm3
	vpmovmskb r10d, ymm4
	vpmovmskb r11d, ymm8
	vpmovmskb r12d, ymm9
	vpmovmskb r13d, ymm10
	vpmovmskb r14d, ymm11
	vpmovmskb r15d, ymm12
	mov [rdx + 0*COMPV_YASM_REG_SZ_BYTES], eax
	mov [rdx + 1*COMPV_YASM_REG_SZ_BYTES], ecx
	mov [rdx + 2*COMPV_YASM_REG_SZ_BYTES], r8d
	mov [rdx + 3*COMPV_YASM_REG_SZ_BYTES], r9d
	mov [rdx + 4*COMPV_YASM_REG_SZ_BYTES], r10d
	mov [rdx + 5*COMPV_YASM_REG_SZ_BYTES], r11d
	mov [rdx + 6*COMPV_YASM_REG_SZ_BYTES], r12d
	mov [rdx + 7*COMPV_YASM_REG_SZ_BYTES], r13d
	mov [rdx + 8*COMPV_YASM_REG_SZ_BYTES], r14d
	mov [rdx + 9*COMPV_YASM_REG_SZ_BYTES], r15d
	vpmovmskb r8d, ymm13
	vpmovmskb r9d, ymm14
	vpmovmskb r10d, ymm15
	vpxor ymm15, ymm15 ; ymmZeros
	mov [rdx + 10*COMPV_YASM_REG_SZ_BYTES], r8d
	mov [rdx + 11*COMPV_YASM_REG_SZ_BYTES], r9d
	mov [rdx + 12*COMPV_YASM_REG_SZ_BYTES], r10d
	pop rax ; restore rax, arg(8) -> ddarkers16x32
	vpcmpeqb ymm0, ymm15, [rax+13*32]
	vpcmpeqb ymm1, ymm15, [rax+14*32]
	vpcmpeqb ymm2, ymm15, [rax+15*32]
	vpandn ymm0, ymm5
	vpandn ymm1, ymm5
	vpandn ymm2, ymm5
	vpmovmskb r8d, ymm0
	vpmovmskb r9d, ymm1
	vpmovmskb r10d, ymm2
	mov [rdx + 13*COMPV_YASM_REG_SZ_BYTES], r8d
	mov [rdx + 14*COMPV_YASM_REG_SZ_BYTES], r9d
	mov [rdx + 15*COMPV_YASM_REG_SZ_BYTES], r10d
		
	.EndOfDarkers
	

	;
	;	LoadBrighters
	;
	test dil, dil ; rdi was loadB, now it's free
	jz .EndOfBrighters
	; compute Dbrighters
	mov r9, rsi
	vpxor ymm5, ymm5
	shl r9, 9 ; rsi*(16*32)
	vmovdqa ymm4, [sym(k1_i8)]
	mov rax, arg(9) ; dbrighters16x32
	lea rax, [rax + r9]
	vmovdqa ymm0, [rsp + 1064 + 1*32]
	vmovdqa ymm1, [rsp + 1064 + 2*32]
	vmovdqa ymm2, [rsp + 1064 + 3*32]
	vmovdqa ymm3, [rsp + 1064 + 5*32]
	vmovdqa ymm8, [rsp + 1064 + 6*32]
	vmovdqa ymm9, [rsp + 1064 + 7*32]
	vmovdqa ymm10, [rsp + 1064 + 9*32]
	vmovdqa ymm11, [rsp + 1064 + 10*32]
	vmovdqa ymm12, [rsp + 1064 + 11*32]
	vmovdqa ymm13, [rsp + 1064 + 13*32]
	vmovdqa ymm14, [rsp + 1064 + 14*32]
	vmovdqa ymm15, [rsp + 1064 + 15*32]
	vpsubusb ymm0, ymm6
	vpsubusb ymm1, ymm6
	vpsubusb ymm2, ymm6
	vpsubusb ymm3, ymm6
	vpsubusb ymm8, ymm6
	vpsubusb ymm9, ymm6
	vpsubusb ymm10, ymm6
	vpsubusb ymm11, ymm6
	vpsubusb ymm12, ymm6
	vpsubusb ymm13, ymm6
	vpsubusb ymm14, ymm6
	vpsubusb ymm15, ymm6
	vmovdqa [rax + 1*32], ymm0
	vmovdqa [rax + 2*32], ymm1
	vmovdqa [rax + 3*32], ymm2
	vmovdqa [rax + 5*32], ymm3
	vmovdqa [rax + 6*32], ymm8
	vmovdqa [rax + 7*32], ymm9
	vmovdqa [rax + 9*32], ymm10
	vmovdqa [rax + 10*32], ymm11
	vmovdqa [rax + 11*32], ymm12
	vmovdqa [rax + 13*32], ymm13
	vmovdqa [rax + 14*32], ymm14
	vmovdqa [rax + 15*32], ymm15
	vpcmpeqb ymm0, ymm5
	vpcmpeqb ymm1, ymm5
	vpcmpeqb ymm2, ymm5
	vpcmpeqb ymm3, ymm5
	vpcmpeqb ymm8, ymm5
	vpcmpeqb ymm9, ymm5
	vpcmpeqb ymm10, ymm5
	vpcmpeqb ymm11, ymm5
	vpcmpeqb ymm12, ymm5
	vpcmpeqb ymm13, ymm5
	vpcmpeqb ymm14, ymm5
	vpcmpeqb ymm15, ymm5
	vpandn ymm0, ymm4
	vpandn ymm1, ymm4
	vpandn ymm2, ymm4
	vpandn ymm3, ymm4
	vpandn ymm8, ymm4
	vpandn ymm9, ymm4
	vpandn ymm10, ymm4
	vpandn ymm11, ymm4
	vpandn ymm12, ymm4
	vpandn ymm13, ymm4
	vpandn ymm14, ymm4
	vpandn ymm15, ymm4
	vpaddusb ymm0, ymm1
	vpaddusb ymm2, ymm3
	vpaddusb ymm8, ymm9
	vpaddusb ymm10, ymm11
	vpaddusb ymm12, ymm13
	vpaddusb ymm14, ymm15
	vpaddusb ymm0, ymm2
	vpaddusb ymm8, ymm10
	vpaddusb ymm12, ymm14
	vpaddusb ymm0, ymm8
	vpaddusb ymm12, ymm0 ; ymm12 = 1 + 2 + 3 + 5 + 6 + 7 + 9 + 10 + 11 + 13 + 14 + 15
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
	vpaddusb ymm0, ymm12 ; ymm0 += 1 + 2 + 3 + 5 + 6 + 7 + 9 + 10 + 11 + 13 + 14 + 15
	; Check the columns with at least N non-zero bits
	vpcmpgtb ymm0, ymm4
	vpmovmskb edx, ymm0
	test edx, edx
	jz .EndOfBrighters
	; Continue loading brighters
	mov rcx, arg(11) ; rb
	mov [rcx + rsi*COMPV_YASM_REG_SZ_BYTES], edx ; (*rb) = colBrightersFlags
	; Transpose
	COMPV_TRANSPOSE_I8_16X16_REG_T5_AVX2 rax+0*32, rax+1*32, rax+2*32, rax+3*32, rax+4*32, rax+5*32, rax+6*32, rax+7*32, rax+8*32, rax+9*32, rax+10*32, rax+11*32, rax+12*32, rax+13*32, rax+14*32, rax+15*32, ymm0, ymm1, ymm2, ymm3, ymm4
	; Flags
	mov r8, rsi
	vpxor ymm6, ymm6 ; ymmZero
	shl r8, COMPV_YASM_REG_SZ_BYTES_TIMES16_LOG2 ; rsi * (16*COMPV_YASM_REG_SZ_BYTES)
	mov rdx, arg(7) ; pfbrighters16
	lea rdx, [rdx + r8]
	vpcmpeqb ymm0, ymm6, [rax+0*32]
	vpcmpeqb ymm1, ymm6, [rax+1*32]
	vpcmpeqb ymm2, ymm6, [rax+2*32]
	vpcmpeqb ymm3, ymm6, [rax+3*32]
	vpcmpeqb ymm4, ymm6, [rax+4*32]
	vpcmpeqb ymm5, ymm6, [rax+5*32]
	vpcmpeqb ymm8, ymm6, [rax+6*32]
	vpcmpeqb ymm9, ymm6, [rax+7*32]
	vpcmpeqb ymm10, ymm6, [rax+8*32]
	vpcmpeqb ymm11, ymm6, [rax+9*32]
	vpcmpeqb ymm12, ymm6, [rax+10*32]
	vpcmpeqb ymm13, ymm6, [rax+11*32]
	vpcmpeqb ymm14, ymm6, [rax+12*32]
	vpcmpeqb ymm15, ymm6, [rax+13*32]
	vpcmpeqb ymm6, ymm6 ; ymmFF
	vpandn ymm0, ymm6
	vpandn ymm1, ymm6
	vpandn ymm2, ymm6
	vpandn ymm3, ymm6
	vpandn ymm4, ymm6
	vpandn ymm5, ymm6
	vpandn ymm8, ymm6
	vpandn ymm9, ymm6
	vpandn ymm10, ymm6
	vpandn ymm11, ymm6
	vpandn ymm12, ymm6
	vpandn ymm13, ymm6
	vpandn ymm14, ymm6
	vpandn ymm15, ymm6
	vpmovmskb edi, ymm0
	vpmovmskb ecx, ymm1
	vpmovmskb r8d, ymm2
	vpmovmskb r9d, ymm3
	vpmovmskb r10d, ymm4
	vpmovmskb r11d, ymm5
	vpmovmskb r12d, ymm8
	vpmovmskb r13d, ymm9
	vpmovmskb r14d, ymm10
	vpmovmskb r15d, ymm11
	vpxor ymm2, ymm2 ; ymmZero
	mov [rdx + 0*COMPV_YASM_REG_SZ_BYTES], edi
	mov [rdx + 1*COMPV_YASM_REG_SZ_BYTES], ecx
	mov [rdx + 2*COMPV_YASM_REG_SZ_BYTES], r8d
	mov [rdx + 3*COMPV_YASM_REG_SZ_BYTES], r9d
	mov [rdx + 4*COMPV_YASM_REG_SZ_BYTES], r10d
	mov [rdx + 5*COMPV_YASM_REG_SZ_BYTES], r11d
	mov [rdx + 6*COMPV_YASM_REG_SZ_BYTES], r12d
	mov [rdx + 7*COMPV_YASM_REG_SZ_BYTES], r13d
	mov [rdx + 8*COMPV_YASM_REG_SZ_BYTES], r14d
	mov [rdx + 9*COMPV_YASM_REG_SZ_BYTES], r15d
	vpmovmskb r8d, ymm12
	vpmovmskb r9d, ymm13
	vpmovmskb r10d, ymm14
	vpmovmskb r11d, ymm15
	mov [rdx + 10*COMPV_YASM_REG_SZ_BYTES], r8d
	mov [rdx + 11*COMPV_YASM_REG_SZ_BYTES], r9d
	mov [rdx + 12*COMPV_YASM_REG_SZ_BYTES], r10d
	mov [rdx + 13*COMPV_YASM_REG_SZ_BYTES], r11d
	vpcmpeqb ymm0, ymm2, [rax+14*32]
	vpcmpeqb ymm1, ymm2, [rax+15*32]
	vpandn ymm0, ymm6
	vpandn ymm1, ymm6
	vpmovmskb edi, ymm0
	vpmovmskb ecx, ymm1
	mov [rdx + 14*COMPV_YASM_REG_SZ_BYTES], edi
	mov [rdx + 15*COMPV_YASM_REG_SZ_BYTES], ecx

	.EndOfBrighters

	
	.LoopRowsNext
	lea rbx, [rbx + 32] ; IP += 32
	
	;-------------------
	;EndOfLooopRows
	lea rsi, [rsi + 1]
	cmp rsi, arg(2)
	jl .LoopRows
	;-------------------

	; unalign stack and free memory
	add rsp, 8 + 32 + 16*32 + 16*32 + 16*32
	COMPV_YASM_UNALIGN_STACK

	vzeroupper

	; begin epilog
	pop r15
	pop r14
	pop r13
	pop r12
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
	

%endif ; COMPV_YASM_ABI_IS_64BIT
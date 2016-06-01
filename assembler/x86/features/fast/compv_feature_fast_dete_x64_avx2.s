;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "../../compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT

%include "../../compv_bits_macros_x86.s"
%include "../../math/compv_math_macros_x86.s"
%include "compv_feature_fast_dete_macros_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(FastData32Row_Asm_X64_AVX2)

global sym(Fast9Strengths32_Asm_CMOV_X64_AVX2)
global sym(Fast9Strengths32_Asm_X64_AVX2)
global sym(Fast12Strengths32_Asm_CMOV_X64_AVX2)
global sym(Fast12Strengths32_Asm_X64_AVX2)

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
; void FastData32Row_Asm_X64_AVX2(const uint8_t* IP, const uint8_t* IPprev, compv_scalar_t width, const compv_scalar_t(&pixels16)[16], compv_scalar_t N, compv_scalar_t threshold, uint8_t* strengths, compv_scalar_t* me);
sym(FastData32Row_Asm_X64_AVX2):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_YMM 15 ;YMM[6-n]
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

	; Compute xmmThreshold and xmmNMinusOne here before zeroupper 
	movzx rax, byte arg(4) ; N
	movzx rcx, byte arg(5) ; threshold
	sub al, 1
	movd xmm0, eax	; xmm0 = xmmThreshold
	vmovd xmm7, ecx ; xmm7 = xmmNMinusOne

	mov rbx, arg(0) ; rbx = IP
	mov rdi, arg(6) ; rdi = strengths
	mov rdx, arg(2) ; width
	shr rdx, 5 ; div width with 32 and mov rsi by 1 -> because of argi(x,rsi)
	xor rsi, rsi
	mov arg(2), rdx

	; Compute ymmNMinusOne and save
	vpbroadcastb ymm0, xmm0
	vmovdqa [rsp + 280], ymm0

	; Compute ymmThreshold and save
	vpbroadcastb ymm7, xmm7
	vmovdqa [rsp + 2872], ymm7

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

	; cleanup strengths
	vmovdqu [rdi], ymm7

	;
	; Speed-Test-1
	;

	mov r11, arg(3) ; pixels16

	; Load pixels 0 and 8 (ymm0 and ymm1)
	mov rax, [r11 + 0*COMPV_YASM_REG_SZ_BYTES] ; pixels16[0]
	mov rdx, [r11 + 8*COMPV_YASM_REG_SZ_BYTES] ; pixels16[8]
	vmovdqu ymm0, [rbx + rax]
	vmovdqu ymm1, [rbx + rdx]

	; Load Pixels 4 and 12 (ymm12 and ymm13) here regardless the speed-test result for
	; pixels 0 and 8 to hide latency
	mov rcx, [r11 + 4*COMPV_YASM_REG_SZ_BYTES] ; pixels16[4]
	mov r12, [r11 + 12*COMPV_YASM_REG_SZ_BYTES] ; pixels16[12]
	vmovdqu ymm12, [rbx + rcx]
	vmovdqu ymm13, [rbx + r12]
	
	; compute ymmDarker and ymmBrighter
	; done here to hide "vmovdqu ymm6, x" latency
	vpsubusb ymm5, ymm6, [rsp + 2872] ; ymm5 = ymmDarker
	vpaddusb ymm6, ymm6, [rsp + 2872] ; ymm6 = ymmBrighter
	vpcmpeqb ymm14, ymm14 ; ymm14 = ymmFF

	; compare I1 and I9 aka 0 and 8
	vpsubusb ymm2, ymm5, ymm0 ; ddarkers16x32[0]
	vpsubusb ymm0, ymm6 ; dbrighters16x32[0]
	vpsubusb ymm3, ymm5, ymm1 ; ddarkers16x32[8]
	vpsubusb ymm1, ymm6 ; dbrighters16x32[8]
	vmovdqa [rsp + 1848 + 0*32], ymm2
	vmovdqa [rsp + 2360 + 0*32], ymm0
	vmovdqa [rsp + 1848 + 8*32], ymm3
	vmovdqa [rsp + 2360 + 8*32], ymm1
	vpcmpeqb ymm2, ymm7
	vpcmpeqb ymm3, ymm7
	vpcmpeqb ymm0, ymm7
	vpcmpeqb ymm1, ymm7
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
	vpsubusb ymm2, ymm5, ymm12 ; ddarkers16x32[4]
	vpsubusb ymm0, ymm12, ymm6 ; dbrighters16x32[4]
	vpsubusb ymm3, ymm5, ymm13 ; ddarkers16x32[12]
	vpsubusb ymm1, ymm13, ymm6 ; dbrighters16x32[12]
	vmovdqa [rsp + 1848 + 4*32], ymm2
	vmovdqa [rsp + 2360 + 4*32], ymm0
	vmovdqa [rsp + 1848 + 12*32], ymm3
	vmovdqa [rsp + 2360 + 12*32], ymm1
	vpcmpeqb ymm2, ymm7 
	vpcmpeqb ymm3, ymm7 
	vpcmpeqb ymm0, ymm7 
	vpcmpeqb ymm1, ymm7 
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
	setg r15b ; r15 = (rcx > 1) ? 1 : 0
	
	; Check whether to load Darkers
	test r9d, r9d
	setnz cl
	test r10d, r10d
	setnz dl
	add cl, dl
	cmp cl, 1
	setg dl ; rdx = (rcx > 1) ? 1 : 0

	; r15 = loadB, rdx = loadD
	; skip process if (!(loadB || loadD))
	mov al, byte r15b
	or al, dl
	test al, al
	jz .LoopRowsNext

	; Set colDarkersFlags and colBrightersFlags to zero
	xor rax, rax
	mov [rsp + 8], rax ; colDarkersFlags
	mov [rsp + 16], rax ; colBrightersFlags

	; Save data
	vmovdqa [rsp + 312 + 0*32], ymm10 ; ymmDarkersFlags[0]
	vmovdqa [rsp + 312 + 8*32], ymm11 ; ymmDarkersFlags[8]
	vmovdqa [rsp + 312 + 4*32], ymm2 ; ymmDarkersFlags[4]
	vmovdqa [rsp + 312 + 12*32], ymm3 ; ymmDarkersFlags[12]
	vmovdqa [rsp + 824 + 0*32], ymm8 ; ymmBrightersFlags[0]
	vmovdqa [rsp + 824 + 8*32], ymm9 ; ymmBrightersFlags[8]
	vmovdqa [rsp + 824 + 4*32], ymm0 ; ymmBrightersFlags[4]
	vmovdqa [rsp + 824 + 12*32], ymm1 ; ymmBrightersFlags[12]

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
	vmovdqu ymm0, [rbx + rax]
	vmovdqu ymm1, [rbx + r8]
	vmovdqu ymm2, [rbx + r9]
	vmovdqu ymm3, [rbx + r10]
	vmovdqu ymm8, [rbx + r11]
	vmovdqu ymm9, [rbx + r12]
	vmovdqu ymm10, [rbx + r13]
	vmovdqu ymm11, [rbx + r14]
	mov r8, [rcx + 11*COMPV_YASM_REG_SZ_BYTES] ; pixels16[11]
	mov r9, [rcx + 13*COMPV_YASM_REG_SZ_BYTES] ; pixels16[13]
	mov r10, [rcx + 14*COMPV_YASM_REG_SZ_BYTES] ; pixels16[14]
	mov r11, [rcx + 15*COMPV_YASM_REG_SZ_BYTES] ; pixels16[15]
	vmovdqu ymm12, [rbx + r8]
	vmovdqu ymm13, [rbx + r9]
	vmovdqu ymm14, [rbx + r10]
	vmovdqu ymm15, [rbx + r11]
	vmovdqa [rsp + 1336 + 1*32], ymm0
	vmovdqa [rsp + 1336 + 2*32], ymm1
	vmovdqa [rsp + 1336 + 3*32], ymm2
	vmovdqa [rsp + 1336 + 5*32], ymm3
	vmovdqa [rsp + 1336 + 6*32], ymm8
	vmovdqa [rsp + 1336 + 7*32], ymm9
	vmovdqa [rsp + 1336 + 9*32], ymm10
	vmovdqa [rsp + 1336 + 10*32], ymm11
	vmovdqa [rsp + 1336 + 11*32], ymm12
	vmovdqa [rsp + 1336 + 13*32], ymm13
	vmovdqa [rsp + 1336 + 14*32], ymm14
	vmovdqa [rsp + 1336 + 15*32], ymm15

	; We could compute pixels at 1 and 9, check if at least one is darker or brighter than the candidate
	; Then, do the same for 2 and 10 etc etc ... but this is slower than whant we're doing below because
	; _mm_movemask_epi8 is cyclyvore


	;
	;	LoadBrighters
	;
	test r15b, r15b ; r15 was loadB, now it's free
	jz .EndOfBrighters
	; compute Dbrighters
	vmovdqa ymm4, [sym(k1_i8)]
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
	vmovdqa [rsp + 2360 + 1*32], ymm0
	vmovdqa [rsp + 2360 + 2*32], ymm1
	vmovdqa [rsp + 2360 + 3*32], ymm2
	vmovdqa [rsp + 2360 + 5*32], ymm3
	vmovdqa [rsp + 2360 + 6*32], ymm8
	vmovdqa [rsp + 2360 + 7*32], ymm9
	vmovdqa [rsp + 2360 + 9*32], ymm10
	vmovdqa [rsp + 2360 + 10*32], ymm11
	vmovdqa [rsp + 2360 + 11*32], ymm12
	vmovdqa [rsp + 2360 + 13*32], ymm13
	vmovdqa [rsp + 2360 + 14*32], ymm14
	vmovdqa [rsp + 2360 + 15*32], ymm15
	vpcmpeqb ymm0, ymm7
	vpcmpeqb ymm1, ymm7
	vpcmpeqb ymm2, ymm7
	vpcmpeqb ymm3, ymm7
	vpcmpeqb ymm8, ymm7
	vpcmpeqb ymm9, ymm7
	vpcmpeqb ymm10, ymm7
	vpcmpeqb ymm11, ymm7
	vpcmpeqb ymm12, ymm7
	vpcmpeqb ymm13, ymm7
	vpcmpeqb ymm14, ymm7
	vpcmpeqb ymm15, ymm7
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
	vmovdqa ymm4, [rsp + 280] ; ymmNMinusOne
	vpandn ymm0, ymm6, [rsp + 824 + 0*32]
	vpandn ymm1, ymm6, [rsp + 824 + 4*32]
	vpandn ymm2, ymm6, [rsp + 824 + 8*32]
	vpandn ymm3, ymm6, [rsp + 824 + 12*32]
	vpaddusb ymm0, ymm1
	vpaddusb ymm2, ymm3
	vpaddusb ymm0, ymm2 ; ymm0 = 0 + 4 + 8 + 12
	vpaddusb ymm0, ymm12 ; ymm0 += 1 + 2 + 3 + 5 + 6 + 7 + 9 + 10 + 11 + 13 + 14 + 15
	; Check the columns with at least N non-zero bits
	vpcmpgtb ymm0, ymm4
	vpmovmskb eax, ymm0
	test eax, eax
	jz .EndOfBrighters
	; Continue loading brighters
	mov [rsp + 16], eax ; colBrightersFlags
	; Transpose
	COMPV_TRANSPOSE_I8_16X16_REG_T5_AVX2 rsp+2360+0*32, rsp+2360+1*32, rsp+2360+2*32, rsp+2360+3*32, rsp+2360+4*32, rsp+2360+5*32, rsp+2360+6*32, rsp+2360+7*32, rsp+2360+8*32, rsp+2360+9*32, rsp+2360+10*32, rsp+2360+11*32, rsp+2360+12*32, rsp+2360+13*32, rsp+2360+14*32, rsp+2360+15*32, ymm0, ymm1, ymm2, ymm3, ymm4
	; Flags
	vpcmpeqb ymm6, ymm6 ; ymmFF
	vpcmpeqb ymm0, ymm7, [rsp + 2360 +0*32]
	vpcmpeqb ymm1, ymm7, [rsp + 2360 +1*32]
	vpcmpeqb ymm2, ymm7, [rsp + 2360 +2*32]
	vpcmpeqb ymm3, ymm7, [rsp + 2360 +3*32]
	vpcmpeqb ymm4, ymm7, [rsp + 2360 +4*32]
	vpcmpeqb ymm8, ymm7, [rsp + 2360 +5*32]
	vpcmpeqb ymm9, ymm7, [rsp + 2360 +6*32]
	vpcmpeqb ymm10, ymm7, [rsp + 2360 +7*32]
	vpcmpeqb ymm11, ymm7, [rsp + 2360 +8*32]
	vpcmpeqb ymm12, ymm7, [rsp + 2360 +9*32]
	vpcmpeqb ymm13, ymm7, [rsp + 2360 +10*32]
	vpcmpeqb ymm14, ymm7, [rsp + 2360 +11*32]
	vpcmpeqb ymm15, ymm7, [rsp + 2360 +12*32]
	vpandn ymm0, ymm6
	vpandn ymm1, ymm6
	vpandn ymm2, ymm6
	vpandn ymm3, ymm6
	vpandn ymm4, ymm6
	vpandn ymm8, ymm6
	vpandn ymm9, ymm6
	vpandn ymm10, ymm6
	vpandn ymm11, ymm6
	vpandn ymm12, ymm6
	vpandn ymm13, ymm6
	vpandn ymm14, ymm6
	vpandn ymm15, ymm6
	vpmovmskb eax, ymm0
	vpmovmskb ecx, ymm1
	vpmovmskb r8d, ymm2
	vpmovmskb r9d, ymm3
	vpmovmskb r10d, ymm4
	vpmovmskb r11d, ymm8
	vpmovmskb r12d, ymm9
	vpmovmskb r13d, ymm10
	vpmovmskb r14d, ymm11
	mov [rsp + 152 + 0*COMPV_YASM_REG_SZ_BYTES], eax
	mov [rsp + 152 + 1*COMPV_YASM_REG_SZ_BYTES], ecx
	mov [rsp + 152 + 2*COMPV_YASM_REG_SZ_BYTES], r8d
	mov [rsp + 152 + 3*COMPV_YASM_REG_SZ_BYTES], r9d
	mov [rsp + 152 + 4*COMPV_YASM_REG_SZ_BYTES], r10d
	mov [rsp + 152 + 5*COMPV_YASM_REG_SZ_BYTES], r11d
	mov [rsp + 152 + 6*COMPV_YASM_REG_SZ_BYTES], r12d
	mov [rsp + 152 + 7*COMPV_YASM_REG_SZ_BYTES], r13d
	mov [rsp + 152 + 8*COMPV_YASM_REG_SZ_BYTES], r14d
	vpmovmskb r8d, ymm12
	vpmovmskb r9d, ymm13
	vpmovmskb r10d, ymm14
	vpmovmskb r11d, ymm15
	mov [rsp + 152 + 9*COMPV_YASM_REG_SZ_BYTES], r8d
	mov [rsp + 152 + 10*COMPV_YASM_REG_SZ_BYTES], r9d
	mov [rsp + 152 + 11*COMPV_YASM_REG_SZ_BYTES], r10d
	mov [rsp + 152 + 12*COMPV_YASM_REG_SZ_BYTES], r11d
	vpcmpeqb ymm0, ymm7, [rsp + 2360 +13*32]
	vpcmpeqb ymm1, ymm7, [rsp + 2360 +14*32]
	vpcmpeqb ymm2, ymm7, [rsp + 2360 +15*32]
	vpandn ymm0, ymm6
	vpandn ymm1, ymm6
	vpandn ymm2, ymm6
	vpmovmskb eax, ymm0
	vpmovmskb ecx, ymm1
	vpmovmskb r8d, ymm2
	mov [rsp + 152 + 13*COMPV_YASM_REG_SZ_BYTES], eax
	mov [rsp + 152 + 14*COMPV_YASM_REG_SZ_BYTES], ecx
	mov [rsp + 152 + 15*COMPV_YASM_REG_SZ_BYTES], r8d

	.EndOfBrighters


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
	vpsubusb ymm8, ymm5, [rsp + 1336 + 6*32]
	vpsubusb ymm9, ymm5, [rsp + 1336 + 7*32]
	vpsubusb ymm10, ymm5, [rsp + 1336 + 9*32]
	vpsubusb ymm11, ymm5, [rsp + 1336 + 10*32]
	vpsubusb ymm12, ymm5, [rsp + 1336 + 11*32]
	vpsubusb ymm13, ymm5, [rsp + 1336 + 13*32]
	vpsubusb ymm14, ymm5, [rsp + 1336 + 14*32]
	vpsubusb ymm15, ymm5, [rsp + 1336 + 15*32]
	vmovdqa [rsp + 1848 + 1*32], ymm0
	vmovdqa [rsp + 1848 + 2*32], ymm1
	vmovdqa [rsp + 1848 + 3*32], ymm2
	vmovdqa [rsp + 1848 + 5*32], ymm3
	vmovdqa [rsp + 1848 + 6*32], ymm8
	vmovdqa [rsp + 1848 + 7*32], ymm9
	vmovdqa [rsp + 1848 + 9*32], ymm10
	vmovdqa [rsp + 1848 + 10*32], ymm11
	vmovdqa [rsp + 1848 + 11*32], ymm12
	vmovdqa [rsp + 1848 + 13*32], ymm13
	vmovdqa [rsp + 1848 + 14*32], ymm14
	vmovdqa [rsp + 1848 + 15*32], ymm15
	vpcmpeqb ymm0, ymm7
	vpcmpeqb ymm1, ymm7
	vpcmpeqb ymm2, ymm7
	vpcmpeqb ymm3, ymm7
	vpcmpeqb ymm8, ymm7
	vpcmpeqb ymm9, ymm7
	vpcmpeqb ymm10, ymm7
	vpcmpeqb ymm11, ymm7
	vpcmpeqb ymm12, ymm7
	vpcmpeqb ymm13, ymm7
	vpcmpeqb ymm14, ymm7
	vpcmpeqb ymm15, ymm7
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
	vmovdqa ymm4, [rsp + 280] ; ymmNMinusOne
	vpandn ymm0, ymm5, [rsp + 312 + 0*32]
	vpandn ymm1, ymm5, [rsp + 312 + 4*32]
	vpandn ymm2, ymm5, [rsp + 312 + 8*32]
	vpandn ymm3, ymm5, [rsp + 312 + 12*32]
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
	mov [rsp + 8], edx ;colDarkersFlags
	; Transpose
	COMPV_TRANSPOSE_I8_16X16_REG_T5_AVX2 rsp+1848+0*32, rsp+1848+1*32, rsp+1848+2*32, rsp+1848+3*32, rsp+1848+4*32, rsp+1848+5*32, rsp+1848+6*32, rsp+1848+7*32, rsp+1848+8*32, rsp+1848+9*32, rsp+1848+10*32, rsp+1848+11*32, rsp+1848+12*32, rsp+1848+13*32, rsp+1848+14*32, rsp+1848+15*32, ymm0, ymm1, ymm2, ymm3, ymm4	
	; Flags
	vpcmpeqb ymm5, ymm5 ; ymmFF
	vpcmpeqb ymm0, ymm7, [rsp + 1848 +0*32]
	vpcmpeqb ymm1, ymm7, [rsp + 1848 +1*32]
	vpcmpeqb ymm2, ymm7, [rsp + 1848 +2*32]
	vpcmpeqb ymm3, ymm7, [rsp + 1848 +3*32]
	vpcmpeqb ymm4, ymm7, [rsp + 1848 +4*32]
	vpcmpeqb ymm8, ymm7, [rsp + 1848 +5*32]
	vpcmpeqb ymm9, ymm7, [rsp + 1848 +6*32]
	vpcmpeqb ymm10, ymm7, [rsp + 1848 +7*32]
	vpcmpeqb ymm11, ymm7, [rsp + 1848 +8*32]
	vpcmpeqb ymm12, ymm7, [rsp + 1848 +9*32]
	vpcmpeqb ymm13, ymm7, [rsp + 1848 +10*32]
	vpcmpeqb ymm14, ymm7, [rsp + 1848 +11*32]
	vpcmpeqb ymm15, ymm7, [rsp + 1848 +12*32]
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
	mov [rsp + 24 + 0*COMPV_YASM_REG_SZ_BYTES], eax
	mov [rsp + 24 + 1*COMPV_YASM_REG_SZ_BYTES], ecx
	mov [rsp + 24 + 2*COMPV_YASM_REG_SZ_BYTES], r8d
	mov [rsp + 24 + 3*COMPV_YASM_REG_SZ_BYTES], r9d
	mov [rsp + 24 + 4*COMPV_YASM_REG_SZ_BYTES], r10d
	mov [rsp + 24 + 5*COMPV_YASM_REG_SZ_BYTES], r11d
	mov [rsp + 24 + 6*COMPV_YASM_REG_SZ_BYTES], r12d
	mov [rsp + 24 + 7*COMPV_YASM_REG_SZ_BYTES], r13d
	mov [rsp + 24 + 8*COMPV_YASM_REG_SZ_BYTES], r14d
	mov [rsp + 24 + 9*COMPV_YASM_REG_SZ_BYTES], r15d
	vpmovmskb r8d, ymm13
	vpmovmskb r9d, ymm14
	vpmovmskb r10d, ymm15
	mov [rsp + 24 + 10*COMPV_YASM_REG_SZ_BYTES], r8d
	mov [rsp + 24 + 11*COMPV_YASM_REG_SZ_BYTES], r9d
	mov [rsp + 24 + 12*COMPV_YASM_REG_SZ_BYTES], r10d
	vpcmpeqb ymm0, ymm7, [rsp + 1848 +13*32]
	vpcmpeqb ymm1, ymm7, [rsp + 1848 +14*32]
	vpcmpeqb ymm2, ymm7, [rsp + 1848 +15*32]
	vpandn ymm0, ymm5
	vpandn ymm1, ymm5
	vpandn ymm2, ymm5
	vpmovmskb r8d, ymm0
	vpmovmskb r9d, ymm1
	vpmovmskb r10d, ymm2
	mov [rsp + 24 + 13*COMPV_YASM_REG_SZ_BYTES], r8d
	mov [rsp + 24 + 14*COMPV_YASM_REG_SZ_BYTES], r9d
	mov [rsp + 24 + 15*COMPV_YASM_REG_SZ_BYTES], r10d
		
	.EndOfDarkers

	; Check if we have to compute strengths
	mov rax, [rsp + 8] ; colDarkersFlags
	or rax, [rsp + 16] ; | colBrighters
	test eax, eax
	jz .NeitherDarkersNorBrighters
	; call FastStrengths32(compv_scalar_t rbrighters, compv_scalar_t rdarkers, COMPV_ALIGNED(SSE) const uint8_t* dbrighters16x32, COMPV_ALIGNED(SSE) const uint8_t* ddarkers16x32, const compv_scalar_t(*fbrighters16)[16], const compv_scalar_t(*fdarkers16)[16], uint8_t* strengths32, compv_scalar_t N);
	mov rax, rsp ; save rsp before reserving params, must not be one of the registers used to save the params (rcx, rdx, r8, r9, rdi, rsi)
	mov r11, rdi ; strengths
	; starting this line [rcx, rdx, r8, r9, rdi, rsi] must not be used
	COMPV_YASM_RESERVE_PARAMS r10, 8
	mov r10, [rax + 16] ; colBrightersFlags
	set_param 0, r10
	mov r10, [rax + 8] ; colDarkersFlags
	set_param 1, r10
	lea r10, [rax + 2360] ; ymmDbrighters16x32
	set_param 2, r10
	lea r10, [rax + 1848] ; ymmDdarkers16x32
	set_param 3, r10
	lea r10, [rax + 152] ; fbrighters16
	set_param 4, r10
	lea r10, [rax + 24] ; fdarkers16
	set_param 5, r10
	mov r10, r11 ; strengths
	set_param 6, r10
	mov r10, arg(4) ; N
	set_param 7, r10
	call sym(FastStrengths32)
	COMPV_YASM_UNRESERVE_PARAMS
	
	vpxor ymm7, ymm7 ; restore ymm7
	.NeitherDarkersNorBrighters
	
	.LoopRowsNext
	lea rbx, [rbx + 32] ; IP += 32
	lea rdi, [rdi + 32] ; strengths += 32
	
	;-------------------
	;EndOfLooopRows
	lea rsi, [rsi + 1]
	cmp rsi, arg(2)
	jl .LoopRows
	;-------------------

	; unalign stack and free memory
	add rsp,  8 + 8 + 8 + 16*8 + 16*8 + 32 + 16*32 + 16*32 + 16*32 + 16*32 + 16*32 + 32
	COMPV_YASM_UNALIGN_STACK

	; begin epilog
	pop r15
	pop r14
	pop r13
	pop r12
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_YMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
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
%macro FastStrengths32_Asm_X64_AVX2 2
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	; Do not save [XMMx] we only use XMM0-XMM5
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	push r14
	push r15
	; end prolog

	%define ret_in_rcx 0
	%define ret_in_rbx 1

	vzeroall

	; vzeroall already set ymm0 to zeros
	; vpxor ymm0, ymm0

	; FAST hard-coded flags
	%if %2 == 9
		%define ymmFastXFlags [sym(Fast9Flags) + 0] ; xmmFastXFlags
	%elif %2 == 12
		%define ymmFastXFlags [sym(Fast12Flags) + 0] ; xmmFastXFlags
	%else
		%error "not supported"
	%endif

	xor rdx, rdx ; rdx = p = 0
	mov r8, arg(4) ; r8 = &fbrighters16[p]
	mov r9, arg(5) ; r9 = &fdarkers16[p]
	mov r10, arg(0) ; r10 = rbrighters
	mov r11, arg(1) ; r11 = rdarkers
	mov r12, arg(2) ; r12 = dbrighters16x32
	mov r13, arg(3) ; r13 = ddarkers16x32
	mov r14, arg(6) ; r14 = &strengths32
	mov r15, (1<<0 | 1<<16)  ; r15 = (1<<p) | (1<<g)

	;----------------------
	; Loop Start
	;----------------------
	.LoopStart
		xor rcx, rcx ; maxnLow = 0
		xor rbx, rbx ; maxnHigh = 0

		; ---------
		; Brighters
		; ---------
		test r10, r15 ; (rb & (1 << p) || rb & (1 << g)) ?
		jz .EndOfBrighters
			mov rdi, [r8 + rdx*COMPV_YASM_REG_SZ_BYTES] ; fbrighters16[p]
			; brighters flags
			vmovd xmm5, edi
			vpbroadcastw xmm5, xmm5
			vinsertf128 ymm5, ymm5, xmm5, 1 ; ymm5 = ymmFLow
			shr rdi, 16
			vmovd xmm4, edi
			vpbroadcastw xmm4, xmm4
			vinsertf128 ymm4, ymm4, xmm4, 1 ; ymm4 = ymmFHigh

			vpand ymm5, ymm5, ymmFastXFlags
			vpcmpeqw ymm5, ymm5, ymmFastXFlags
			vpand ymm4, ymm4, ymmFastXFlags
			vpcmpeqw ymm4, ymm4, ymmFastXFlags
			COMPV_PACKS_EPI16_AVX2 ymm5, ymm5, ymm4
			vpmovmskb eax, ymm5
			test eax, eax ; rax = r0
			jz .EndOfBrighters
				; Load dbrighters
				vmovdqa xmm2, [r12 + 0]
				vmovdqa xmm5, [r12 + 16]
				test rax, 0xFFFF
				jz .EndOfBrightersHzMinLow
					; Compute minimum hz (low)
					COMPV_FEATURE_FAST_DETE_HORIZ_MIN_AVX2 Brighters, %1, %2, ret_in_rcx, xmm2, xmm0, xmm1, xmm3 ; This macro overrides rax, rsi, rdi and set the result in rcx or rbx
				.EndOfBrightersHzMinLow
				test rax, 0xFFFF0000
				jz .EndOfBrightersHzMinHigh
					; Compute minimum hz (high)
					shr rax, 16 ; rax = r1 = r0 >> 16
					COMPV_FEATURE_FAST_DETE_HORIZ_MIN_AVX2 Brighters, %1, %2, ret_in_rbx, xmm5, xmm0, xmm1, xmm3 ; This macro overrides rax, rsi, rdi and set the result in rcx or rbx
				.EndOfBrightersHzMinHigh
		.EndOfBrighters

		; ---------
		; Darkers
		; ---------
		.Darkers
		test r11, r15 ; (rd & (1 << p) || rd & (1 << g)) ?
		jz .EndOfDarkers
			mov rdi, [r9 + rdx*COMPV_YASM_REG_SZ_BYTES] ; fdarkers16[p]
			; darkers flags
			vmovd xmm5, edi
			vpbroadcastw xmm5, xmm5
			vinsertf128 ymm5, ymm5, xmm5, 1 ; ymm5 = ymmFLow
			shr rdi, 16
			vmovd xmm4, edi
			vpbroadcastw xmm4, xmm4
			vinsertf128 ymm4, ymm4, xmm4, 1 ; ymm4 = ymmFHigh
		
			vpand ymm5, ymm5, ymmFastXFlags
			vpcmpeqw ymm5, ymm5, ymmFastXFlags
			vpand ymm4, ymm4, ymmFastXFlags
			vpcmpeqw ymm4, ymm4, ymmFastXFlags
			COMPV_PACKS_EPI16_AVX2 ymm5, ymm5, ymm4
			vpmovmskb eax, ymm5
			test eax, eax ; rax = r0
			jz .EndOfDarkers
				; Load ddarkers
				vmovdqa xmm2, [r13 + 0]
				vmovdqa xmm5, [r13 + 16]
				test rax, 0xFFFF
				jz .EndOfDarkersHzMinLow
					; Compute minimum hz (low)
					COMPV_FEATURE_FAST_DETE_HORIZ_MIN_AVX2 Darkers, %1, %2, ret_in_rcx, xmm2, xmm0, xmm1, xmm3 ; This macro overrides rax, rsi, rdi and set the result in rcx or rbx
				.EndOfDarkersHzMinLow
				test rax, 0xFFFF0000
				jz .EndOfDarkersHzMinHigh
					; Compute minimum hz (high)
					shr rax, 16 ; rax = r1 = r0 >> 16
					COMPV_FEATURE_FAST_DETE_HORIZ_MIN_AVX2 Darkers, %1, %2, ret_in_rbx, xmm5, xmm0, xmm1, xmm3 ; This macro overrides rax, rsi, rdi and set the result in rcx or rbx
				.EndOfDarkersHzMinHigh
		.EndOfDarkers
		
		; compute strenghts[p]
		mov [r14 + rdx + 0], byte cl ; strengths16[g] = maxnLow
		mov [r14 + rdx + 16], byte bl ; trengths16[p] = maxnHigh
	
		inc rdx ; p+= 1
		shl r15, 1 ; ((1<<p) | (1<<g)) << 1
		lea r12, [r12 + 32] ; dbrighters16x32 += 32
		lea r13, [r13 + 32] ; ddarkers16x32 += 32
		cmp rdx, 16
	jl .LoopStart
	;----------------

	vzeroall

	%undef ret_in_rcx
	%undef ret_in_rbx
	%undef ymmFastXFlags

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
%endmacro


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t Fast9Strengths32_Asm_CMOV_X64_AVX2(COMPV_ALIGNED(AVX) const int16_t(&dbrighters)[16], COMPV_ALIGNED(AVX) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
sym(Fast9Strengths32_Asm_CMOV_X64_AVX2):
	FastStrengths32_Asm_X64_AVX2 1, 9

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t Fast9Strengths32_Asm_X64_AVX2(COMPV_ALIGNED(AVX) const int16_t(&dbrighters)[16], COMPV_ALIGNED(AVX) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
sym(Fast9Strengths32_Asm_X64_AVX2):
	FastStrengths32_Asm_X64_AVX2 0, 9

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t Fast12Strengths32_Asm_CMOV_X64_AVX2(COMPV_ALIGNED(AVX) const int16_t(&dbrighters)[16], COMPV_ALIGNED(AVX) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
sym(Fast12Strengths32_Asm_CMOV_X64_AVX2):
	FastStrengths32_Asm_X64_AVX2 1, 12

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; compv_scalar_t Fast12Strengths32_Asm_X64_AVX2(COMPV_ALIGNED(AVX) const int16_t(&dbrighters)[16], COMPV_ALIGNED(AVX) const int16_t(&ddarkers)[16], compv_scalar_t fbrighters, compv_scalar_t fdarkers, compv_scalar_t N, COMPV_ALIGNED(SSE) const uint16_t(&FastXFlags)[16])
sym(Fast12Strengths32_Asm_X64_AVX2):
	FastStrengths32_Asm_X64_AVX2 0, 12

%endif ; COMPV_YASM_ABI_IS_64BIT
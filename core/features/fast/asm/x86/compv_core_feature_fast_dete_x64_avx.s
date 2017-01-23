;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"
%if COMPV_YASM_ABI_IS_64BIT
%include "compv_core_feature_fast_dete_macros_x86_avx.s"

COMPV_YASM_DEFAULT_REL

global sym(CompVFast9DataRow_Asm_X64_AVX2)
global sym(CompVFast12DataRow_Asm_X64_AVX2)
global sym(CompVFastNmsGather_Asm_X64_AVX2)

section .data
	extern COMPV_YASM_DLLIMPORT_DECL(k1_i8)
	
section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* IP
; arg(1) -> COMPV_ALIGNED(AVX) compv_uscalar_t width
; arg(2) -> const compv_scalar_t *pixels16
; arg(3) -> compv_uscalar_t N
; arg(4) -> compv_uscalar_t threshold
; arg(5) -> uint8_t* strengths
; %1 -> fastType 9 or 12
%macro CompVFastDataRow_Macro_X64_AVX2 1
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	COMPV_YASM_SAVE_YMM 15 ; ymm14 and ymm15 used in '_mm_fast_check' macro
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 32, rax
	sub rsp, (8*16) + (32*16) + (32*16) + (32*16)
	%define vecThreshold			ymm8
	%define vecNMinSumMinusOne		ymm9
	%define vecNMinusOne			ymm10
	%define vecOne					ymm11
	%define vec0xFF					ymm12
	%define vecZero					ymm13
	%define circle					rsp + 0   ; +128
	%define vecDiffBinary16			rsp + 128 ; +512
	%define vecDiff16				rsp + 640 ; +512
	%define vecCircle16				rsp + 1152 ; +512

	%define IP			arg(0)
	%define width		arg(1)
	%define pixels16	arg(2)
	%define N			arg(3)
	%define threshold	arg(4)
	%define strengths	arg(5)
	%define i			rcx

	mov rax, threshold
	mov rcx, N
	mov rdx, N
	shr rcx, 2 ; minsum = (N == 12 ? 3 : 2)
	dec rdx
	dec rcx ; minsum - 1

	vmovd xmm8, eax
	vmovd xmm9, ecx
	vmovd xmm10, edx
	vpbroadcastb ymm8, xmm8
	vpbroadcastb ymm9, xmm9
	vpbroadcastb ymm10, xmm10
	COMPV_YASM_DLLIMPORT_LOAD vmovdqa, vecOne, k1_i8, rax
	vpcmpeqb vec0xFF, vec0xFF
	vpxor vecZero, vecZero

	; Load circle
	; TODO(dmi): load circle indexes only when neeeded
	mov rax, pixels16
	mov rdx, IP ; rdx = IP
	%assign circleIdx 0
	%rep 16
		mov rsi, [rax + circleIdx*COMPV_YASM_REG_SZ_BYTES]
		lea rdi, [rdx + rsi]
		mov [circle + circleIdx*8], rdi
		%assign circleIdx circleIdx+1
	%endrep

	mov rbx, strengths ; rbx = strengths

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (i = 0; i < width; i += 32)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	xor rcx, rcx ; rcx = i
	.LoopWidth
		%define vecDarker1 ymm0
		%define vecBrighter1 ymm1
		%define vecStrengths ymm2
		%define vecSum1 ymm3
		vmovdqu vecDarker1, [rdx + i] ; Do not prefetcht0 unaligned memory (tests show it''s very sloow)
		vpxor vecStrengths, vecStrengths
		vpaddusb vecBrighter1, vecDarker1, vecThreshold
		vpsubusb vecDarker1, vecDarker1, vecThreshold
		
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; Check
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		_mm_fast_check 0, 8
		_mm_fast_check 4, 12
		_mm_fast_check 1, 9
		_mm_fast_check 5, 13
		_mm_fast_check 2, 10
		_mm_fast_check 6, 14
		_mm_fast_check 3, 11
		_mm_fast_check 7, 15

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; Darkers
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		vpxor vecSum1, vecSum1
		_mm_fast_load_Darkers 0, 8, 4, 12
		vpcmpgtb ymm4, vecSum1, vecNMinSumMinusOne
		vpmovmskb rax, ymm4
		test rax, rax
		jz .EndOfDarkers
		_mm_fast_load_Darkers 1, 9, 5, 13
		_mm_fast_load_Darkers 2, 10, 6, 14
		_mm_fast_load_Darkers 3, 11, 7, 15
		vpcmpgtb ymm4, vecSum1, vecNMinusOne
		vpmovmskb rax, ymm4
		test rax, rax
		jz .EndOfDarkers
		_mm_fast_init_diffbinarysum %1
		%assign strengthIdx 0
		%rep 16
			_mm_fast_strength strengthIdx, %1
			%assign strengthIdx strengthIdx+1
		%endrep
		.EndOfDarkers
		; end of darkers

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; Brighters
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		vpxor vecSum1, vecSum1
		_mm_fast_load_Brighters 0, 8, 4, 12
		vpcmpgtb ymm4, vecSum1, vecNMinSumMinusOne
		vpmovmskb rax, ymm4
		test rax, rax
		jz .EndOfBrighters
		_mm_fast_load_Brighters 1, 9, 5, 13
		_mm_fast_load_Brighters 2, 10, 6, 14
		_mm_fast_load_Brighters 3, 11, 7, 15
		vpcmpgtb ymm4, vecSum1, vecNMinusOne
		vpmovmskb rax, ymm4
		test rax, rax
		jz .EndOfBrighters
		_mm_fast_init_diffbinarysum %1
		%assign strengthIdx 0
		%rep 16
			_mm_fast_strength strengthIdx, %1
			%assign strengthIdx strengthIdx+1
		%endrep
		.EndOfBrighters
		; end of brighters

		.Next
		lea i, [i + 32]
		cmp i, width
		vmovdqu [rbx + i - 32], vecStrengths
		
		jl .LoopWidth
		; end-of-LoopWidth 

	; free memory and unalign stack
	add rsp, (8*16) + (32*16) + (32*16) + (32*16)
	COMPV_YASM_UNALIGN_STACK

	%undef vecThreshold
	%undef vecNMinSumMinusOne
	%undef vecNMinusOne
	%undef vecSum1
	%undef vecStrengths
	%undef vecBrighter1
	%undef vecDarker1
	%undef vecOne
	%undef vec0xFF
	%undef vecZero
	%undef circle
	%undef vecDiffBinary16
	%undef vecDiff16
	%undef vecCircle16
	%undef IP
	%undef width
	%undef pixels16
	%undef N
	%undef threshold
	%undef strengths
	%undef i

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_YMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVFast9DataRow_Asm_X64_AVX2):
	CompVFastDataRow_Macro_X64_AVX2 9

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVFast12DataRow_Asm_X64_AVX2):
	CompVFastDataRow_Macro_X64_AVX2 12

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* pcStrengthsMap
; arg(1) -> uint8_t* pNMS
; arg(2) -> const compv_uscalar_t width
; arg(3) -> compv_uscalar_t heigth
; arg(4) -> COMPV_ALIGNED(AVX) compv_uscalar_t stride
sym(CompVFastNmsGather_Asm_X64_AVX2):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	COMPV_YASM_SAVE_YMM 11
	push r12
	push r13
	push r14
	push r15
	;; end prolog ;;

	mov rcx, arg(4)
	imul rcx, 3
	mov rax, arg(0) ; rax = pcStrengthsMap
	mov rdx, arg(1) ; rdx = pNMS
	lea rax, [rax + rcx]
	lea rdx, [rdx + rcx]

	mov r15, arg(2) ; r15 = width
	mov r8, arg(3)
	lea r8, [r8 - 3] ; r8 = j
	mov r11, arg(4) ; r11 = stride
	mov r13, 3
	mov r14, 3
	sub r13, r11 ; r13 = (i - stride)
	add r14, r11 ; r14 = (i + stride)
	vpxor ymm0, ymm0
	vpcmpeqb ymm1, ymm1
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 3; j < heigth - 3; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		mov r9, 3 ; r9 = i
		mov r10, r13 ; r10 = (i - stride)
		mov r12, r14 ; r12 = (i + stride)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 3; i < width - 3; i += 32) ; stride aligned on (width + 3) which means we can ignore the '-3' guard
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopWidth
			vmovdqu ymm2, [rax + r9]
			vpcmpeqb ymm3, ymm2, ymm0
			vpandn ymm3, ymm1
			vpmovmskb rcx, ymm3
			test rcx, rcx
			jz .Next
			vmovdqu ymm4, [rax + r9 - 1] ; left
			vmovdqu ymm5, [rax + r9 + 1] ; right
			vmovdqu ymm6, [rax + r10 - 1] ; left-top
			vmovdqu ymm7, [rax + r10] ; top
			vmovdqu ymm8, [rax + r10 + 1] ; right-top
			vmovdqu ymm9, [rax + r12 - 1] ; left-bottom
			vmovdqu ymm10, [rax + r12] ; bottom
			vmovdqu ymm11, [rax + r12 + 1] ; right-bottom
			vpminub ymm4, ymm2
			vpminub ymm5, ymm2
			vpminub ymm6, ymm2
			vpminub ymm7, ymm2
			vpminub ymm8, ymm2
			vpminub ymm9, ymm2
			vpminub ymm10, ymm2
			vpminub ymm11, ymm2
			vpcmpeqb ymm4, ymm2
			vpcmpeqb ymm5, ymm2
			vpcmpeqb ymm6, ymm2
			vpcmpeqb ymm7, ymm2
			vpcmpeqb ymm8, ymm2
			vpcmpeqb ymm9, ymm2
			vpcmpeqb ymm10, ymm2
			vpcmpeqb ymm11, ymm2
			vpor ymm4, ymm5
			vpor ymm4, ymm6
			vpor ymm4, ymm7
			vpor ymm4, ymm8
			vpor ymm4, ymm9
			vpor ymm4, ymm10
			vpor ymm4, ymm11
			vpand ymm4, ymm2
			vmovdqu [rdx + r9], ymm4
			.Next
			
			lea r9, [r9 + 32]
			cmp r9, r15
			lea r10, [r10 + 32]
			lea r12, [r12 + 32]
			jl .LoopWidth
			; end-of-LoopWidth

		dec r8
		lea rax, [rax + r11]
		lea rdx, [rdx + r11]
		jnz .LoopHeight
		; end-of-LoopHeight

	;; begin epilog ;;
	pop r15
	pop r14
	pop r13
	pop r12
	COMPV_YASM_RESTORE_YMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret

%endif ; COMPV_YASM_ABI_IS_64BIT
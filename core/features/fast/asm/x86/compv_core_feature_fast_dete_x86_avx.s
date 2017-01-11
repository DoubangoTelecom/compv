;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"
%include "compv_core_feature_fast_dete_macros_x86_avx.s"

COMPV_YASM_DEFAULT_REL

global sym(CompVFast9DataRow_Asm_X86_AVX2)
global sym(CompVFast12DataRow_Asm_X86_AVX2)
global sym(CompVFastNmsGather_Asm_X86_AVX2)
global sym(CompVFastNmsApply_Asm_X86_AVX2)

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
%macro CompVFastDataRow_Macro_X86_AVX2 1
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	COMPV_YASM_SAVE_YMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 32, rax
	sub rsp, 32 + 32 +  32 + 32 + 32 + 32 + (8*16) + (32*16) + (32*16) + (32*16)
	%define vecThreshold			[rsp + 0]   ; +32
	%define vecNMinSumMinusOne		[rsp + 32]  ; +32
	%define vecNMinusOne			[rsp + 64]  ; +32
	%define vecOne					[rsp + 96]  ; +32
	%define vec0xFF					[rsp + 128]  ; +32
	%define vecZero					[rsp + 160]  ; +32
	%define circle					rsp + 192    ; +128
	%define vecDiffBinary16			rsp + 320   ; +512
	%define vecDiff16				rsp + 832   ; +512
	%define vecCircle16				rsp + 1344   ; +512

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

	vmovd xmm0, eax
	vmovd xmm1, ecx
	vmovd xmm2, edx
	vpbroadcastb ymm0, xmm0
	vpbroadcastb ymm1, xmm1
	vpbroadcastb ymm2, xmm2
	vmovdqa vecThreshold, ymm0
	vmovdqa vecNMinSumMinusOne, ymm1
	vmovdqa vecNMinusOne, ymm2
	COMPV_YASM_DLLIMPORT_LOAD vmovdqa, ymm3, k1_i8, rax
	vpcmpeqb ymm4, ymm4
	vpxor ymm5, ymm5
	vmovdqa vecOne, ymm3
	vmovdqa vec0xFF, ymm4
	vmovdqa vecZero, ymm5

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
		vmovdqu [rbx + i], vecStrengths

		lea i, [i + 32]
		cmp i, width
		jl .LoopWidth
		; end-of-LoopWidth 

	; free memory and unalign stack
	add rsp, 32 + 32 +  32 + 32 + 32 + 32 + (8*16) + (32*16) + (32*16) + (32*16)
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
sym(CompVFast9DataRow_Asm_X86_AVX2):
	CompVFastDataRow_Macro_X86_AVX2 9

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVFast12DataRow_Asm_X86_AVX2):
	CompVFastDataRow_Macro_X86_AVX2 12

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* pcStrengthsMap
; arg(1) -> uint8_t* pNMS
; arg(2) -> const compv_uscalar_t width
; arg(3) -> compv_uscalar_t heigth
; arg(4) -> COMPV_ALIGNED(AVX) compv_uscalar_t stride
sym(CompVFastNmsGather_Asm_X86_AVX2):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	COMPV_YASM_SAVE_YMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	mov rcx, arg(4)
	imul rcx, 3
	mov rax, arg(0) ; rax = pcStrengthsMap
	mov rdx, arg(1) ; rdx = pNMS
	lea rax, [rax + rcx]
	lea rdx, [rdx + rcx]

	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*0]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*1]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*2]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]

	mov rsi, arg(3)
	lea rsi, [rsi - 3] ; rsi = j
	vpxor ymm0, ymm0
	vpcmpeqb ymm1, ymm1
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 3; j < heigth - 3; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		mov rdi, 3 ; rdi = i
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 3; i < width - 3; i += 32) ; stride aligned on (width + 3) which means we can ignore the '-3' guard
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopWidth
			prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]
			vmovdqu ymm2, [rax + rdi]
			vpcmpeqb ymm3, ymm2, ymm0
			vpandn ymm3, ymm3, ymm1
			vpmovmskb rcx, ymm3
			test rcx, rcx
			jz .Next
			mov rbx, rdi
			mov rcx, rdi
			sub rbx, arg(4)
			add rcx, arg(4)
			vmovdqu ymm4, [rax + rdi - 1] ; left
			vmovdqu ymm5, [rax + rdi + 1] ; right
			vmovdqu ymm6, [rax + rbx - 1] ; left-top
			vmovdqu ymm7, [rax + rbx] ; top
			vpminub ymm4, ymm2
			vpminub ymm5, ymm2
			vpminub ymm6, ymm2
			vpminub ymm7, ymm2
			vpcmpeqb ymm4, ymm2
			vpcmpeqb ymm5, ymm2
			vpcmpeqb ymm6, ymm2
			vpcmpeqb ymm7, ymm2
			vpor ymm4, ymm5
			vmovdqu ymm5, [rax + rbx + 1] ; right-top
			vpor ymm4, ymm6
			vmovdqu ymm6, [rax + rcx - 1] ; left-bottom
			vpor ymm4, ymm7			
			vmovdqu ymm7, [rax + rcx] ; bottom
			vpminub ymm5, ymm2
			vpminub ymm6, ymm2
			vpminub ymm7, ymm2
			vpcmpeqb ymm5, ymm2
			vpcmpeqb ymm6, ymm2
			vpcmpeqb ymm7, ymm2
			vpor ymm4, ymm5
			vmovdqu ymm5, [rax + rcx + 1] ; right-bottom
			vpor ymm4, ymm6
			vpminub ymm5, ymm2
			vpor ymm4, ymm7			
			vpcmpeqb ymm5, ymm2
			vpor ymm4, ymm5
			vpand ymm4, ymm2
			vmovdqu [rdx + rdi], ymm4
			.Next
			
			lea rdi, [rdi + 32]
			cmp rdi, arg(2)
			jl .LoopWidth
			; end-of-LoopWidth

		add rax, arg(4)
		add rdx, arg(4)
		dec rsi
		jnz .LoopHeight
		; end-of-LoopHeight

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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(AVX) uint8_t* pcStrengthsMap
; arg(1) -> COMPV_ALIGNED(AVX) uint8_t* pNMS
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t heigth
; arg(4) -> COMPV_ALIGNED(AVX) compv_uscalar_t stride
sym(CompVFastNmsApply_Asm_X86_AVX2):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	mov rcx, arg(4)
	imul rcx, 3
	mov rax, arg(0) ; rax = pcStrengthsMap
	mov rdx, arg(1) ; rdx = pNMS
	lea rax, [rax + rcx]
	lea rdx, [rdx + rcx]

	mov rsi, arg(3)
	lea rsi, [rsi - 3] ; rsi = j
	mov rbx, arg(2) ; rbx = width
	vpxor ymm0, ymm0
	vpcmpeqb ymm1, ymm1
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 3; j < heigth - 3; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		xor rdi, rdi ; rdi = i
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopWidth
			vpcmpeqb ymm2, ymm0, [rdx + rdi]
			vpandn ymm2, ymm1
			vpmovmskb rcx, ymm2
			test rcx, rcx
			jz .Next
			vpandn ymm2, [rax + rdi]
			vmovdqa [rdx + rdi], ymm0
			vmovdqa [rax + rdi], ymm2
			.Next
			
			; end-of-LoopWidth
			lea rdi, [rdi + 32]
			cmp rdi, rbx
			jl .LoopWidth

		add rax, arg(4)
		add rdx, arg(4)
		; end-of-LoopHeight
		dec rsi
		jnz .LoopHeight

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret

	

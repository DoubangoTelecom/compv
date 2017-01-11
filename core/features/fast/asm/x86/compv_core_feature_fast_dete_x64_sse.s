;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"
%if COMPV_YASM_ABI_IS_64BIT
%include "compv_core_feature_fast_dete_macros_x86_sse.s"

COMPV_YASM_DEFAULT_REL

global sym(CompVFast9DataRow_Asm_X64_SSE2)
global sym(CompVFast12DataRow_Asm_X64_SSE2)
global sym(CompVFastNmsGather_Asm_X64_SSE2)

section .data
	extern COMPV_YASM_DLLIMPORT_DECL(k1_i8)
	
section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* IP
; arg(1) -> COMPV_ALIGNED(SSE) compv_uscalar_t width
; arg(2) -> const compv_scalar_t *pixels16
; arg(3) -> compv_uscalar_t N
; arg(4) -> compv_uscalar_t threshold
; arg(5) -> uint8_t* strengths
; %1 -> fastType 9 or 12
%macro CompVFastDataRow_Macro_X64_SSE2 1
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	COMPV_YASM_SAVE_XMM 13
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 16, rax
	sub rsp, (8*16) + (16*16) + (16*16) + (16*16)
	%define vecThreshold			xmm8
	%define vecNMinSumMinusOne		xmm9
	%define vecNMinusOne			xmm10
	%define vecOne					xmm11
	%define vec0xFF					xmm12
	%define vecZero					xmm13
	%define circle					rsp + 0   ; +128
	%define vecDiffBinary16			rsp + 128 ; +256
	%define vecDiff16				rsp + 384 ; +256
	%define vecCircle16				rsp + 640 ; +256

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

	movd vecThreshold, rax
	movd vecNMinSumMinusOne, rcx
	movd vecNMinusOne, rdx
	punpcklbw vecThreshold, vecThreshold
	punpcklbw vecNMinSumMinusOne, vecNMinSumMinusOne
	punpcklbw vecNMinusOne, vecNMinusOne
	punpcklwd vecThreshold, vecThreshold
	punpcklwd vecNMinSumMinusOne, vecNMinSumMinusOne
	punpcklwd vecNMinusOne, vecNMinusOne
	pshufd vecThreshold, vecThreshold, 0
	pshufd vecNMinSumMinusOne, vecNMinSumMinusOne, 0
	pshufd vecNMinusOne, vecNMinusOne, 0
	COMPV_YASM_DLLIMPORT_LOAD movdqa, vecOne, k1_i8, rax
	pcmpeqb vec0xFF, vec0xFF
	pxor vecZero, vecZero

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
	; for (i = 0; i < width; i += 16)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	xor rcx, rcx ; rcx = i
	.LoopWidth
		%define vecDarker1 xmm0
		%define vecBrighter1 xmm1
		%define vecStrengths xmm2
		%define vecSum1 xmm3
		movdqu vecDarker1, [rdx + i] ; Do not prefetcht0 unaligned memory (tests show it''s very sloow)
		pxor vecStrengths, vecStrengths
		movdqa vecBrighter1, vecDarker1
		psubusb vecDarker1, vecThreshold
		paddusb vecBrighter1, vecThreshold
		
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
		pxor vecSum1, vecSum1
		_mm_fast_load_Darkers 0, 8, 4, 12
		movdqa xmm4, vecSum1
		pcmpgtb xmm4, vecNMinSumMinusOne
		pmovmskb rax, xmm4
		test rax, rax
		jz .EndOfDarkers
		_mm_fast_load_Darkers 1, 9, 5, 13
		_mm_fast_load_Darkers 2, 10, 6, 14
		_mm_fast_load_Darkers 3, 11, 7, 15
		movdqa xmm4, vecSum1
		pcmpgtb xmm4, vecNMinusOne
		pmovmskb rax, xmm4
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
		pxor vecSum1, vecSum1
		_mm_fast_load_Brighters 0, 8, 4, 12
		movdqa xmm4, vecSum1
		pcmpgtb xmm4, vecNMinSumMinusOne
		pmovmskb rax, xmm4
		test rax, rax
		jz .EndOfBrighters
		_mm_fast_load_Brighters 1, 9, 5, 13
		_mm_fast_load_Brighters 2, 10, 6, 14
		_mm_fast_load_Brighters 3, 11, 7, 15
		movdqa xmm4, vecSum1
		pcmpgtb xmm4, vecNMinusOne
		pmovmskb rax, xmm4
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
		movdqu [rbx + i], vecStrengths

		lea i, [i + 16]
		cmp i, width
		jl .LoopWidth
		; end-of-LoopWidth 

	; free memory and unalign stack
	add rsp, (8*16) + (16*16) + (16*16) + (16*16)
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
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVFast9DataRow_Asm_X64_SSE2):
	CompVFastDataRow_Macro_X64_SSE2 9

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVFast12DataRow_Asm_X64_SSE2):
	CompVFastDataRow_Macro_X64_SSE2 12

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* pcStrengthsMap
; arg(1) -> uint8_t* pNMS
; arg(2) -> const compv_uscalar_t width
; arg(3) -> compv_uscalar_t heigth
; arg(4) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
sym(CompVFastNmsGather_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	COMPV_YASM_SAVE_XMM 11
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

	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*0]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*1]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*2]
	prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]

	mov r15, arg(2) ; r15 = width
	mov r8, arg(3)
	lea r8, [r8 - 3] ; r8 = j
	mov r11, arg(4) ; r11 = stride
	mov r13, 3
	mov r14, 3
	sub r13, r11 ; r13 = (i - stride)
	add r14, r11 ; r14 = (i + stride)
	pxor xmm0, xmm0
	pcmpeqb xmm1, xmm1
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 3; j < heigth - 3; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		mov r9, 3 ; r9 = i
		mov r10, r13 ; r10 = (i - stride)
		mov r12, r14 ; r12 = (i + stride)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 3; i < width - 3; i += 16) ; stride aligned on (width + 3) which means we can ignore the '-3' guard
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopWidth
			prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]
			movdqu xmm2, [rax + r9]
			movdqa xmm3, xmm2
			pcmpeqb xmm3, xmm0
			pandn xmm3, xmm1
			pmovmskb rcx, xmm3
			test rcx, rcx
			jz .Next
			movdqu xmm4, [rax + r9 - 1] ; left
			movdqu xmm5, [rax + r9 + 1] ; right
			movdqu xmm6, [rax + r10 - 1] ; left-top
			movdqu xmm7, [rax + r10] ; top
			movdqu xmm8, [rax + r10 + 1] ; right-top
			movdqu xmm9, [rax + r12 - 1] ; left-bottom
			movdqu xmm10, [rax + r12] ; bottom
			movdqu xmm11, [rax + r12 + 1] ; right-bottom
			pminub xmm4, xmm2
			pminub xmm5, xmm2
			pminub xmm6, xmm2
			pminub xmm7, xmm2
			pminub xmm8, xmm2
			pminub xmm9, xmm2
			pminub xmm10, xmm2
			pminub xmm11, xmm2
			pcmpeqb xmm4, xmm2
			pcmpeqb xmm5, xmm2
			pcmpeqb xmm6, xmm2
			pcmpeqb xmm7, xmm2
			pcmpeqb xmm8, xmm2
			pcmpeqb xmm9, xmm2
			pcmpeqb xmm10, xmm2
			pcmpeqb xmm11, xmm2
			por xmm4, xmm5
			por xmm4, xmm6
			por xmm4, xmm7
			por xmm4, xmm8
			por xmm4, xmm9
			por xmm4, xmm10
			por xmm4, xmm11
			pand xmm4, xmm2
			movdqu [rdx + r9], xmm4
			.Next
			
			lea r9, [r9 + 16]
			cmp r9, r15
			lea r10, [r10 + 16]
			lea r12, [r12 + 16]
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
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

%endif ; COMPV_YASM_ABI_IS_64BIT
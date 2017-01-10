;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(CompVFast9DataRow_Asm_X86_SSE2)
global sym(CompVFast12DataRow_Asm_X86_SSE2)
global sym(CompVFastNmsGather_Asm_X86_SSE2)
global sym(CompVFastNmsApply_Asm_X86_SSE2)

section .data
	extern COMPV_YASM_DLLIMPORT_DECL(k1_i8)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Macros for FastDataRow
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
%macro _mm_fast_check 2
	mov rax, [circle + (%1*8)]
	mov rsi, [circle + (%2*8)]
	movdqu xmm6, [rax + i] ; xmm4 = vec0
	movdqu xmm7, [rsi + i] ; xmm5 = vec1
	movdqa xmm4, xmm0
	movdqa xmm5, xmm0
	;  TODO(dmi): for AVX save data to vecCircle16 only after goto Next
	movdqa [vecCircle16 + (%1*16)], xmm6
	movdqa [vecCircle16 + (%2*16)], xmm7
	psubusb xmm4, xmm6
	psubusb xmm5, xmm7
	psubusb xmm6, xmm1
	psubusb xmm7, xmm1
	pcmpeqb xmm4, [vecZero]
	pcmpeqb xmm5, [vecZero]
	pcmpeqb xmm6, [vecZero]
	pcmpeqb xmm7, [vecZero]
	pandn xmm4, [vec0xFF]	
	pandn xmm5, [vec0xFF]	
	pandn xmm6, [vec0xFF]	
	pandn xmm7, [vec0xFF]
	por xmm4, xmm5
	por xmm6, xmm7
	por xmm4, xmm6
	pmovmskb rax, xmm4
	test rax, rax
	jz .Next
%endmacro
%macro _mm_fast_compute_Darkers 4
	movdqa xmm4, xmm0
	movdqa xmm5, xmm0
	movdqa xmm6, xmm0
	movdqa xmm7, xmm0
	psubusb xmm4, [vecCircle16 + (%1*16)]
	psubusb xmm5, [vecCircle16 + (%2*16)]
	psubusb xmm6, [vecCircle16 + (%3*16)]
	psubusb xmm7, [vecCircle16 + (%4*16)]
	movdqa [vecDiff16 + (%1*16)], xmm4
	movdqa [vecDiff16 + (%2*16)], xmm5
	movdqa [vecDiff16 + (%3*16)], xmm6
	movdqa [vecDiff16 + (%4*16)], xmm7
%endmacro
%macro _mm_fast_compute_Brighters 4
	movdqa xmm4, [vecCircle16 + (%1*16)]
	movdqa xmm5, [vecCircle16 + (%2*16)]
	movdqa xmm6, [vecCircle16 + (%3*16)]
	movdqa xmm7, [vecCircle16 + (%4*16)]
	psubusb xmm4, xmm1
	psubusb xmm5, xmm1
	psubusb xmm6, xmm1
	psubusb xmm7, xmm1
	movdqa [vecDiff16 + (%1*16)], xmm4
	movdqa [vecDiff16 + (%2*16)], xmm5
	movdqa [vecDiff16 + (%3*16)], xmm6
	movdqa [vecDiff16 + (%4*16)], xmm7
%endmacro
%macro _mm_fast_load 4
	;_mm_fast_compute_ %+ %5 %1, %2, %3, %4
	pcmpeqb xmm4, [vecZero]
	pcmpeqb xmm5, [vecZero]
	pcmpeqb xmm6, [vecZero]
	pcmpeqb xmm7, [vecZero]
	pandn xmm4, [vecOne]
	pandn xmm5, [vecOne]
	pandn xmm6, [vecOne]
	pandn xmm7, [vecOne]
	movdqa [vecDiffBinary16 + (%1*16)], xmm4
	movdqa [vecDiffBinary16 + (%2*16)], xmm5
	movdqa [vecDiffBinary16 + (%3*16)], xmm6
	movdqa [vecDiffBinary16 + (%4*16)], xmm7
	paddb xmm4, xmm5
	paddb xmm6, xmm7
	paddb xmm3, xmm4
	paddb xmm3, xmm6
%endmacro
%macro _mm_fast_load_Darkers 4
	_mm_fast_compute_Darkers %1, %2, %3, %4
	_mm_fast_load %1, %2, %3, %4
%endmacro
%macro _mm_fast_load_Brighters 4
	_mm_fast_compute_Brighters %1, %2, %3, %4
	_mm_fast_load %1, %2, %3, %4
%endmacro
%macro _mm_fast_init_diffbinarysum 1
	movdqa xmm3, [vecDiffBinary16 + 0*16]
	movdqa xmm4, [vecDiffBinary16 + 1*16]
	movdqa xmm5, [vecDiffBinary16 + 2*16]
	movdqa xmm6, [vecDiffBinary16 + 3*16]
	paddb xmm3, [vecDiffBinary16 + 4*16]
	paddb xmm4, [vecDiffBinary16 + 5*16]
	paddb xmm5, [vecDiffBinary16 + 6*16]
	paddb xmm6, [vecDiffBinary16 + 7*16]
	paddb xmm3, xmm4
	paddb xmm5, xmm6
	paddb xmm3, xmm5
	%if %1 == 12
		movdqa xmm4, [vecDiffBinary16 + 8*16]
		movdqa xmm5, [vecDiffBinary16 + 9*16]
		movdqa xmm6, [vecDiffBinary16 + 10*16]
		paddb xmm4, xmm5
		paddb xmm3, xmm6
		paddb xmm3, xmm4
	%endif
%endmacro
%macro _mm_fast_strength 2
	paddb xmm3, [vecDiffBinary16 + ((%2-1+%1)&15)*16]
	movdqa xmm4, xmm3
	pcmpgtb xmm4, [vecNMinusOne]
	pmovmskb rax, xmm4
	test rax, rax
	jz %%LessThanNMinusOne
	movdqa xmm4, [vecDiff16 + ((%1+0)&15)*16]
	movdqa xmm5, [vecDiff16 + ((%1+1)&15)*16]
	movdqa xmm6, [vecDiff16 + ((%1+2)&15)*16]
	movdqa xmm7, [vecDiff16 + ((%1+3)&15)*16]
	pminub xmm4, [vecDiff16 + ((%1+4)&15)*16]
	pminub xmm5, [vecDiff16 + ((%1+5)&15)*16]
	pminub xmm6, [vecDiff16 + ((%1+6)&15)*16]
	pminub xmm7, [vecDiff16 + ((%1+7)&15)*16]
	pminub xmm4, xmm5
	pminub xmm6, xmm7
	pminub xmm4, xmm6
	pminub xmm4, [vecDiff16 + ((%1+8)&15)*16]
	%if %2 == 12
		movdqa xmm5, [vecDiff16 + ((%1+9)&15)*16]
		movdqa xmm6, [vecDiff16 + ((%1+10)&15)*16]
		movdqa xmm7, [vecDiff16 + ((%1+11)&15)*16]
		pminub xmm5, xmm6
		pminub xmm4, xmm7
		pminub xmm4, xmm5
	%endif
	pmaxub xmm2, xmm4
	%%LessThanNMinusOne:
	psubb xmm3, [vecDiffBinary16 + (%1&15)*16]
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* IP
; arg(1) -> COMPV_ALIGNED(SSE) compv_uscalar_t width
; arg(2) -> COMPV_ALIGNED(SSE) const compv_scalar_t *pixels16
; arg(3) -> compv_uscalar_t N
; arg(4) -> compv_uscalar_t threshold
; arg(5) -> uint8_t* strengths
; %1 -> fastType 9 or 12
%macro CompVFastDataRow_Macro_X86_SSE2 1
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	COMPV_YASM_SAVE_XMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 16, rax
	sub rsp, 8 + 16 + 16 + 16 + 16 + 16 + 16 + 16 + 16 + 16 + 16 + (8*16) + (16*16) + (16*16) + (16*16)
	%define NminusOne				rsp + 0
	%define vecThreshold			rsp + 8
	%define vecNMinSumMinusOne		rsp + 24
	%define vecNMinusOne			rsp + 40
	%define vecSum1					rsp + 56
	%define vecStrengths			rsp + 72
	%define vecBrighter1			rsp + 88
	%define vecDarker1				rsp + 104
	%define vecOne					rsp + 120
	%define vec0xFF					rsp + 136
	%define vecZero					rsp + 152
	%define circle					rsp + 168
	%define vecDiffBinary16			rsp + 296
	%define vecDiff16				rsp + 552
	%define vecCircle16				rsp + 808

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
	dec rdx ; NminusOne = N - 1
	dec rcx ; minsum - 1
	mov [NminusOne], rdx

	movd xmm0, rax
	movd xmm1, rcx
	movd xmm2, rdx
	punpcklbw xmm0, xmm0
	punpcklbw xmm1, xmm1
	punpcklbw xmm2, xmm2
	punpcklwd xmm0, xmm0
	punpcklwd xmm1, xmm1
	punpcklwd xmm2, xmm2
	pshufd xmm0, xmm0, 0
	pshufd xmm1, xmm1, 0
	pshufd xmm2, xmm2, 0
	movdqa [vecThreshold], xmm0
	movdqa [vecNMinSumMinusOne], xmm1
	movdqa [vecNMinusOne], xmm2
	COMPV_YASM_DLLIMPORT_LOAD movdqa, xmm3, k1_i8, rax
	pcmpeqb xmm4, xmm4
	pxor xmm5, xmm5
	movdqa [vecOne], xmm3
	movdqa [vec0xFF], xmm4
	movdqa [vecZero], xmm5

	; Load circle
	; TODO(dmi): load circle indexes only when neeeded
	mov rax, pixels16
	mov rdx, IP ; rdx = IP
	prefetcht0 [rdx + COMPV_YASM_CACHE_LINE_SIZE*0]
	prefetcht0 [rdx + COMPV_YASM_CACHE_LINE_SIZE*1]
	prefetcht0 [rdx + COMPV_YASM_CACHE_LINE_SIZE*2]
	%assign circleIdx 0
	%rep 16
		prefetcht0 [rdx + COMPV_YASM_CACHE_LINE_SIZE*3]
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
		prefetcht0 [rdx + COMPV_YASM_CACHE_LINE_SIZE*3] 
		movdqu xmm0, [rdx + i]
		pxor xmm2, xmm2 ; xmm2 = vecStrengths
		movdqa xmm1, xmm0
		psubusb xmm0, [vecThreshold] ; xmm0 = vecDarker1
		paddusb xmm1, [vecThreshold] ; xmm1 = vecBrighter1
		
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
		pxor xmm3, xmm3 ; xmm3 = vecSum1
		_mm_fast_load_Darkers 0, 8, 4, 12
		movdqa xmm4, xmm3
		pcmpgtb xmm4, [vecNMinSumMinusOne]
		pmovmskb rax, xmm4
		test rax, rax
		jz .EndOfDarkers
		_mm_fast_load_Darkers 1, 9, 5, 13
		_mm_fast_load_Darkers 2, 10, 6, 14
		_mm_fast_load_Darkers 3, 11, 7, 15
		movdqa xmm4, xmm3
		pcmpgtb xmm4, [vecNMinusOne]
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
		pxor xmm3, xmm3 ; xmm3 = vecSum1
		_mm_fast_load_Brighters 0, 8, 4, 12
		movdqa xmm4, xmm3
		pcmpgtb xmm4, [vecNMinSumMinusOne]
		pmovmskb rax, xmm4
		test rax, rax
		jz .EndOfBrighters
		_mm_fast_load_Brighters 1, 9, 5, 13
		_mm_fast_load_Brighters 2, 10, 6, 14
		_mm_fast_load_Brighters 3, 11, 7, 15
		movdqa xmm4, xmm3
		pcmpgtb xmm4, [vecNMinusOne]
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
		movdqu [rbx + i], xmm2

		lea i, [i + 16]
		cmp i, width
		jl .LoopWidth
		; end-of-LoopWidth 

	; free memory and unalign stack
	add rsp, 8 + 16 + 16 + 16 + 16 + 16 + 16 + 16 + 16 + 16 + 16 + (8*16) + (16*16) + (16*16) + (16*16)
	COMPV_YASM_UNALIGN_STACK

	%undef NminusOne
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
sym(CompVFast9DataRow_Asm_X86_SSE2):
	CompVFastDataRow_Macro_X86_SSE2 9

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVFast12DataRow_Asm_X86_SSE2):
	CompVFastDataRow_Macro_X86_SSE2 12

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* pcStrengthsMap
; arg(1) -> uint8_t* pNMS
; arg(2) -> const compv_uscalar_t width
; arg(3) -> compv_uscalar_t heigth
; arg(4) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
sym(CompVFastNmsGather_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	COMPV_YASM_SAVE_XMM 7
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
	pxor xmm0, xmm0
	pcmpeqb xmm1, xmm1
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 3; j < heigth - 3; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		mov rdi, 3 ; rdi = i
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 3; i < width - 3; i += 16) ; stride aligned on (width + 3) which means we can ignore the '-3' guard
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopWidth
			prefetcht0 [rax + COMPV_YASM_CACHE_LINE_SIZE*3]
			movdqu xmm2, [rax + rdi]
			movdqa xmm3, xmm2
			pcmpeqb xmm3, xmm0
			pandn xmm3, xmm1
			pmovmskb rcx, xmm3
			test rcx, rcx
			jz .Next
			mov rbx, rdi
			mov rcx, rdi
			sub rbx, arg(4)
			add rcx, arg(4)
			movdqu xmm4, [rax + rdi - 1] ; left
			movdqu xmm5, [rax + rdi + 1] ; right
			movdqu xmm6, [rax + rbx - 1] ; left-top
			movdqu xmm7, [rax + rbx] ; top
			pminub xmm4, xmm2
			pminub xmm5, xmm2
			pminub xmm6, xmm2
			pminub xmm7, xmm2
			pcmpeqb xmm4, xmm2
			pcmpeqb xmm5, xmm2
			pcmpeqb xmm6, xmm2
			pcmpeqb xmm7, xmm2
			por xmm4, xmm5
			movdqu xmm5, [rax + rbx + 1] ; right-top
			por xmm4, xmm6
			movdqu xmm6, [rax + rcx - 1] ; left-bottom
			por xmm4, xmm7			
			movdqu xmm7, [rax + rcx] ; bottom
			pminub xmm5, xmm2
			pminub xmm6, xmm2
			pminub xmm7, xmm2
			pcmpeqb xmm5, xmm2
			pcmpeqb xmm6, xmm2
			pcmpeqb xmm7, xmm2
			por xmm4, xmm5
			movdqu xmm5, [rax + rcx + 1] ; right-bottom
			por xmm4, xmm6
			pminub xmm5, xmm2
			por xmm4, xmm7			
			pcmpeqb xmm5, xmm2
			por xmm4, xmm5
			pand xmm4, xmm2
			movdqu [rdx + rdi], xmm4
			.Next
			
			lea rdi, [rdi + 16]
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
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) uint8_t* pcStrengthsMap
; arg(1) -> COMPV_ALIGNED(SSE) uint8_t* pNMS
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t heigth
; arg(4) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
sym(CompVFastNmsApply_Asm_X86_SSE2):
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

	prefetcht0 [rdx + COMPV_YASM_CACHE_LINE_SIZE*0]
	prefetcht0 [rdx + COMPV_YASM_CACHE_LINE_SIZE*1]
	prefetcht0 [rdx + COMPV_YASM_CACHE_LINE_SIZE*2]
	prefetcht0 [rdx + COMPV_YASM_CACHE_LINE_SIZE*3]

	mov rsi, arg(3)
	lea rsi, [rsi - 3] ; rsi = j
	mov rbx, arg(2) ; rbx = width
	pxor xmm0, xmm0
	pcmpeqb xmm1, xmm1
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 3; j < heigth - 3; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		xor rdi, rdi ; rdi = i
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopWidth
			prefetcht0 [rdx + COMPV_YASM_CACHE_LINE_SIZE*4]
			movdqa xmm2, [rdx + rdi]
			pcmpeqb xmm2, xmm0
			pandn xmm2, xmm1
			pmovmskb rcx, xmm2
			test rcx, rcx
			jz .Next
			pandn xmm2, [rax + rdi]
			movdqa [rdx + rdi], xmm0
			movdqa [rax + rdi], xmm2
			.Next
			
			; end-of-LoopWidth
			lea rdi, [rdi + 16]
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
	ret

	

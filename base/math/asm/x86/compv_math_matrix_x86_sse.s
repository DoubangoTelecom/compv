;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(CompVMathMatrixMulABt_64f_Asm_X86_SSE2)
global sym(CompVMathMatrixMulGA_64f_Asm_X86_SSE2)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const COMPV_ALIGNED(SSE) compv_float64_t* A
; arg(1) -> compv_uscalar_t aRows
; arg(2) -> COMPV_ALIGNED(SSE) compv_uscalar_t aStrideInBytes
; arg(3) -> const COMPV_ALIGNED(SSE) compv_float64_t* B
; arg(4) -> compv_uscalar_t bRows
; arg(5) -> compv_uscalar_t bCols
; arg(6) -> COMPV_ALIGNED(SSE) compv_uscalar_t bStrideInBytes
; arg(7) -> COMPV_ALIGNED(SSE) compv_float64_t* R
; arg(8) -> COMPV_ALIGNED(SSE) compv_uscalar_t rStrideInBytes
sym(CompVMathMatrixMulABt_64f_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 9
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; alloc memory
	sub rsp, (3*COMPV_YASM_REG_SZ_BYTES)

	%define bColsSignedMinus7       rsp + 0
	%define bColsSignedMinus3		bColsSignedMinus7 + (1*COMPV_YASM_REG_SZ_BYTES)
	%define bColsSignedMinus1		bColsSignedMinus3 + (1*COMPV_YASM_REG_SZ_BYTES)

	mov rax, arg(5) ; bCols
	lea rcx, [rax - 7]
	lea rdx, [rax - 3]
	lea rbx, [rax - 1]
	mov [bColsSignedMinus7], rcx
	mov [bColsSignedMinus3], rdx
	mov [bColsSignedMinus1], rbx

	%define B0		rbx
	%define i		rsi
	%define j		rdi
	%define k		rcx
	%define A		rdx
	%define R		rax

	mov A, arg(0)
	mov R, arg(7)
	mov i, arg(1) ; aRows

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (i = 0; i < aRows; ++i)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopARows:
		mov B0, arg(3) ; B0 = B
		xor j, j ; j = 0
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (j = 0; j < bRows; ++j)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopBRows:
			pxor xmm4, xmm4 ; xmm4 = vecSum
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (k = 0; k < bColsSigned - 7; k += 8)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			xor k, k
			cmp k, [bColsSignedMinus7]
			jge .EndOf_LoopBCols8
			.LoopBCols8:
				movapd xmm0, [A + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
				movapd xmm1, [A + (k + 2)*COMPV_YASM_FLOAT64_SZ_BYTES]
				movapd xmm2, [A + (k + 4)*COMPV_YASM_FLOAT64_SZ_BYTES]
				movapd xmm3, [A + (k + 6)*COMPV_YASM_FLOAT64_SZ_BYTES]
				mulpd xmm0, [B0 + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
				mulpd xmm1, [B0 + (k + 2)*COMPV_YASM_FLOAT64_SZ_BYTES]
				mulpd xmm2, [B0 + (k + 4)*COMPV_YASM_FLOAT64_SZ_BYTES]
				mulpd xmm3, [B0 + (k + 6)*COMPV_YASM_FLOAT64_SZ_BYTES]
				add k, 8
				addpd xmm0, xmm1
				addpd xmm2, xmm3
				addpd xmm4, xmm0
				cmp k, [bColsSignedMinus7]
				addpd xmm4, xmm2
				jl .LoopBCols8
				.EndOf_LoopBCols8:
				;; EndOf_LoopBCols8 ;;

			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; if (k < bColsSigned - 3)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			cmp k, [bColsSignedMinus3]
			jge .EndOf_LoopBCols4
			.LoopBCols4:
				movapd xmm0, [A + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
				movapd xmm1, [A + (k + 2)*COMPV_YASM_FLOAT64_SZ_BYTES]
				mulpd xmm0, [B0 + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
				mulpd xmm1, [B0 + (k + 2)*COMPV_YASM_FLOAT64_SZ_BYTES]
				add k, 4
				addpd xmm0, xmm1
				addpd xmm4, xmm0
				.EndOf_LoopBCols4:
				;; EndOf_LoopBCols4 ;;

			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; if (k < bColsSigned - 1)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			cmp k, [bColsSignedMinus1]
			jge .EndOf_LoopBCols2
			.LoopBCols2:
				movapd xmm0, [A + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
				mulpd xmm0, [B0 + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
				add k, 2
				addpd xmm4, xmm0
				.EndOf_LoopBCols2:
				;; EndOf_LoopBCols2 ;;

			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; if (bColsSigned & 1)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			cmp k, arg(5)
			jge .EndOf_LoopBCols1
			.LoopBCols1:
				movsd xmm0, [A + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
				mulsd xmm0, [B0 + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
				addsd xmm4, xmm0
				.EndOf_LoopBCols1:
				;; EndOf_LoopBCols1 ;;

			movapd xmm0, xmm4
			shufpd xmm0, xmm0, 0x1
			addpd xmm4, xmm0
			movsd [R + j*COMPV_YASM_FLOAT64_SZ_BYTES], xmm4
			inc j
			add B0, arg(6) ; B0 += bStrideInBytes
			cmp j, arg(4)
			jl .LoopBRows
			;; EndOf_LoopBRows ;;
		
		add A, arg(2) ; A += aStrideInBytes
		add R, arg(8) ; R += rStrideInBytes
		dec i
		jnz .LoopARows
		;; EndOf_LoopARows;;

	; free memory
	add rsp, (3*COMPV_YASM_REG_SZ_BYTES)

	%undef bColsSignedMinus7
	%undef bColsSignedMinus3
	%undef bColsSignedMinus1

	%undef B0		
	%undef i		
	%undef j		
	%undef k
	%undef A
	%undef R

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) compv_float64_t* ri
; arg(1) -> COMPV_ALIGNED(SSE) compv_float64_t* rj
; arg(2) -> const compv_float64_t* c1
; arg(3) -> const compv_float64_t* s1
; arg(4) -> compv_uscalar_t count
sym(CompVMathMatrixMulGA_64f_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	COMPV_YASM_SAVE_XMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 16, rax
	sub rsp, (2*COMPV_YASM_XMM_SZ_BYTES)

	%define vecC		rsp + 0
	%define vecS		vecC + (1*COMPV_YASM_XMM_SZ_BYTES)

	%define i			rsi
	%define ri			rdi
	%define rj			rbx
	%define count		rcx
	%define counMinus7	rdx
	%define countMinus3	rax

	mov rax, arg(2)
	mov rdx, arg(3)
	movsd xmm0, [rax]
	movsd xmm1, [rdx]
	shufpd xmm0, xmm0, 0x0
	shufpd xmm1, xmm1, 0x0
	movapd [vecC], xmm0
	movapd [vecS], xmm1

	mov ri, arg(0)
	mov rj, arg(1)
	mov count, arg(4)
	lea counMinus7, [count - 7]
	lea countMinus3, [count - 3]
	xor i, i

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (i = 0; i < countSigned - 7; i += 8)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp i, counMinus7
	jge .EndOf_LoopCount8
	.LoopCount8:
		movapd xmm0, [ri + (i + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm1, [ri + (i + 2)*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm2, [ri + (i + 4)*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm3, [ri + (i + 6)*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm4, [rj + (i + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm5, [rj + (i + 2)*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm6, [rj + (i + 4)*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm7, [rj + (i + 6)*COMPV_YASM_FLOAT64_SZ_BYTES]
		mulpd xmm0, [vecC]
		mulpd xmm1, [vecC]
		mulpd xmm2, [vecC]
		mulpd xmm3, [vecC]
		mulpd xmm4, [vecS]
		mulpd xmm5, [vecS]
		mulpd xmm6, [vecS]
		mulpd xmm7, [vecS]
		addpd xmm0, xmm4
		movapd xmm4, [ri + (i + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
		addpd xmm1, xmm5
		movapd xmm5, [ri + (i + 2)*COMPV_YASM_FLOAT64_SZ_BYTES]
		addpd xmm2, xmm6
		movapd xmm6, [ri + (i + 4)*COMPV_YASM_FLOAT64_SZ_BYTES]
		addpd xmm3, xmm7
		movapd xmm7, [ri + (i + 6)*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd [ri + (i + 0)*COMPV_YASM_FLOAT64_SZ_BYTES], xmm0
		movapd xmm0, [rj + (i + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd [ri + (i + 2)*COMPV_YASM_FLOAT64_SZ_BYTES], xmm1
		movapd xmm1, [rj + (i + 2)*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd [ri + (i + 4)*COMPV_YASM_FLOAT64_SZ_BYTES], xmm2
		movapd xmm2, [rj + (i + 4)*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd [ri + (i + 6)*COMPV_YASM_FLOAT64_SZ_BYTES], xmm3		
		movapd xmm3, [rj + (i + 6)*COMPV_YASM_FLOAT64_SZ_BYTES]
		mulpd xmm0, [vecC]
		mulpd xmm1, [vecC]
		mulpd xmm2, [vecC]
		mulpd xmm3, [vecC]
		mulpd xmm4, [vecS]
		mulpd xmm5, [vecS]
		mulpd xmm6, [vecS]
		mulpd xmm7, [vecS]
		subpd xmm0, xmm4
		subpd xmm1, xmm5
		subpd xmm2, xmm6
		subpd xmm3, xmm7
		movapd [rj + (i + 0)*COMPV_YASM_FLOAT64_SZ_BYTES], xmm0
		movapd [rj + (i + 2)*COMPV_YASM_FLOAT64_SZ_BYTES], xmm1
		movapd [rj + (i + 4)*COMPV_YASM_FLOAT64_SZ_BYTES], xmm2
		movapd [rj + (i + 6)*COMPV_YASM_FLOAT64_SZ_BYTES], xmm3

		add i, 8
		cmp i, counMinus7
		jl .LoopCount8
		.EndOf_LoopCount8
		;; EndOf_LoopCount8 ;;

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; if (i < countSigned - 3)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp i, countMinus3
	jge .EndOf_LoopCount4
	.LoopCount4:
		movapd xmm0, [ri + (i + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm1, [ri + (i + 2)*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm2, [rj + (i + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm3, [rj + (i + 2)*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm4, xmm2
		movapd xmm5, xmm3
		movapd xmm6, xmm0
		movapd xmm7, xmm1
		mulpd xmm0, [vecC]
		mulpd xmm1, [vecC]
		mulpd xmm2, [vecC]
		mulpd xmm3, [vecC]
		mulpd xmm4, [vecS]
		mulpd xmm5, [vecS]
		mulpd xmm6, [vecS]
		mulpd xmm7, [vecS]
		addpd xmm0, xmm4
		addpd xmm1, xmm5
		subpd xmm2, xmm6
		subpd xmm3, xmm7
		movapd [ri + (i + 0)*COMPV_YASM_FLOAT64_SZ_BYTES], xmm0
		movapd [ri + (i + 2)*COMPV_YASM_FLOAT64_SZ_BYTES], xmm1
		movapd [rj + (i + 0)*COMPV_YASM_FLOAT64_SZ_BYTES], xmm2
		movapd [rj + (i + 2)*COMPV_YASM_FLOAT64_SZ_BYTES], xmm3

		add i, 4
		.EndOf_LoopCount4
		;; EndOf_LoopCount4 ;;

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (; i < countSigned; i += 2)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp i, count
	jge .EndOf_LoopCount2and1
	.LoopCount2and1:
		movapd xmm0, [ri + (i + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm1, [rj + (i + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm2, xmm1
		movapd xmm3, xmm0
		mulpd xmm0, [vecC]
		mulpd xmm1, [vecC]
		mulpd xmm2, [vecS]
		mulpd xmm3, [vecS]
		addpd xmm0, xmm2
		subpd xmm1, xmm3
		movapd [ri + (i + 0)*COMPV_YASM_FLOAT64_SZ_BYTES], xmm0
		movapd [rj + (i + 0)*COMPV_YASM_FLOAT64_SZ_BYTES], xmm1

		add i, 2
		cmp i, count
		jl .LoopCount2and1
		.EndOf_LoopCount2and1
		;; EndOf_LoopCount2and1 ;;

	%undef vecC
	%undef vecS

	%undef i
	%undef ri
	%undef rj
	%undef count
	%undef counMinus7
	%undef countMinus3

	; free memory and unalign stack
	add rsp, (2*COMPV_YASM_XMM_SZ_BYTES)
	COMPV_YASM_UNALIGN_STACK

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
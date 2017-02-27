;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(CompVMathMatrixMulABt_64f_Asm_X64_SSE2)

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
sym(CompVMathMatrixMulABt_64f_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 9
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	push r14
	push r15
	;; end prolog ;;

	%define bCols					r8
	%define bColsSignedMinus7       r9
	%define bColsSignedMinus3		r10
	%define bColsSignedMinus1		r11

	mov bCols, arg(5)
	lea r8, [bCols - 7]
	lea r9, [bCols - 3]
	lea r10, [bCols - 1]

	%define B0				rbx
	%define i				rsi
	%define j				rdi
	%define k				rcx
	%define A				rdx
	%define R				rax
	%define	bStrideInBytes	r12
	%define aStrideInBytes	r13
	%define rStrideInBytes	r14
	%define bRows			r15

	mov A, arg(0)
	mov R, arg(7)
	mov i, arg(1) ; aRows
	mov bStrideInBytes, arg(6)
	mov aStrideInBytes, arg(2)
	mov rStrideInBytes, arg(8)
	mov bRows, arg(4)

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
			cmp k, bColsSignedMinus7
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
				cmp k, bColsSignedMinus7
				addpd xmm4, xmm2
				jl .LoopBCols8
				.EndOf_LoopBCols8:
				;; EndOf_LoopBCols8 ;;

			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; if (k < bColsSigned - 3)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			cmp k, bColsSignedMinus3
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
			cmp k, bColsSignedMinus1
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
				add k, 2
				addsd xmm4, xmm0
				.EndOf_LoopBCols1:
				;; EndOf_LoopBCols1 ;;

			movapd xmm0, xmm4
			shufpd xmm0, xmm0, 0x1
			addpd xmm4, xmm0
			movsd [R + j*COMPV_YASM_FLOAT64_SZ_BYTES], xmm4
			inc j
			cmp j, bRows
			lea B0, [B0 + bStrideInBytes]
			jl .LoopBRows
			;; EndOf_LoopBRows ;;
		
		dec i
		lea A, [A + aStrideInBytes]
		lea R, [R + rStrideInBytes]
		jnz .LoopARows
		;; EndOf_LoopARows;;

	%undef bCols
	%undef bColsSignedMinus7
	%undef bColsSignedMinus3
	%undef bColsSignedMinus1

	%undef B0		
	%undef i		
	%undef j		
	%undef k
	%undef A
	%undef R
	%undef	bStrideInBytes	
	%undef aStrideInBytes	
	%undef rStrideInBytes	
	%undef bRows			

	;; begin epilog ;;
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
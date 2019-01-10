;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(CompVMathMatrixMulABt_64f_Asm_X64_SSE2)
global sym(CompVMathMatrixMulGA_64f_Asm_X64_SSE2)
global sym(CompVMathMatrixBuildHomographyEqMatrix_64f_Asm_X64_SSE2)

section .data
	extern sym(kAVXFloat64MaskAbs)
	extern sym(km1_64f)
	extern sym(km1_0_64f)
	extern sym(kAVXFloat64MaskNegate)
	

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
	lea bColsSignedMinus7, [bCols - 7]
	lea bColsSignedMinus3, [bCols - 3]
	lea bColsSignedMinus1, [bCols - 1]

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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) compv_float64_t* ri
; arg(1) -> COMPV_ALIGNED(SSE) compv_float64_t* rj
; arg(2) -> const compv_float64_t* c1
; arg(3) -> const compv_float64_t* s1
; arg(4) -> compv_uscalar_t count
sym(CompVMathMatrixMulGA_64f_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	COMPV_YASM_SAVE_XMM 15
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
	mov r8, count
	shr r8, 3
	.LoopCount8:
		movapd xmm0, [ri + (i + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm1, [ri + (i + 2)*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm2, [ri + (i + 4)*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm3, [ri + (i + 6)*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm4, [rj + (i + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm5, [rj + (i + 2)*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm6, [rj + (i + 4)*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm7, [rj + (i + 6)*COMPV_YASM_FLOAT64_SZ_BYTES]
		movapd xmm8, xmm4
		movapd xmm9, xmm5
		movapd xmm10, xmm6
		movapd xmm11, xmm7
		movapd xmm12, xmm0
		movapd xmm13, xmm1
		movapd xmm14, xmm2
		movapd xmm15, xmm3
		mulpd xmm0, [vecC]
		mulpd xmm8, [vecS]
		mulpd xmm1, [vecC]
		mulpd xmm9, [vecS]
		mulpd xmm2, [vecC]
		mulpd xmm10, [vecS]
		mulpd xmm3, [vecC]
		mulpd xmm11, [vecS]
		mulpd xmm4, [vecC]
		mulpd xmm12, [vecS]
		mulpd xmm5, [vecC]
		mulpd xmm13, [vecS]
		mulpd xmm6, [vecC]
		mulpd xmm14, [vecS]
		mulpd xmm7, [vecC]
		mulpd xmm15, [vecS]
		dec r8
		addpd xmm0, xmm8
		addpd xmm1, xmm9
		addpd xmm2, xmm10
		addpd xmm3, xmm11
		subpd xmm4, xmm12
		subpd xmm5, xmm13
		subpd xmm6, xmm14
		subpd xmm7, xmm15
		movapd [ri + (i + 0)*COMPV_YASM_FLOAT64_SZ_BYTES], xmm0
		movapd [ri + (i + 2)*COMPV_YASM_FLOAT64_SZ_BYTES], xmm1
		movapd [ri + (i + 4)*COMPV_YASM_FLOAT64_SZ_BYTES], xmm2
		movapd [ri + (i + 6)*COMPV_YASM_FLOAT64_SZ_BYTES], xmm3
		movapd [rj + (i + 0)*COMPV_YASM_FLOAT64_SZ_BYTES], xmm4
		movapd [rj + (i + 2)*COMPV_YASM_FLOAT64_SZ_BYTES], xmm5
		movapd [rj + (i + 4)*COMPV_YASM_FLOAT64_SZ_BYTES], xmm6
		movapd [rj + (i + 6)*COMPV_YASM_FLOAT64_SZ_BYTES], xmm7
		lea i, [i + 8]
		jnz .LoopCount8
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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const compv_float64_t* srcX
; arg(1) -> COMPV_ALIGNED(SSE) const compv_float64_t* srcY
; arg(2) -> COMPV_ALIGNED(SSE) const compv_float64_t* dstX
; arg(3) -> COMPV_ALIGNED(SSE) const compv_float64_t* dstY
; arg(4) -> COMPV_ALIGNED(SSE) compv_float64_t* M
; arg(5) -> COMPV_ALIGNED(SSE) compv_uscalar_t M_strideInBytes
; arg(6) -> compv_uscalar_t numPoints
sym(CompVMathMatrixBuildHomographyEqMatrix_64f_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_XMM 12
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	xor rcx, rcx ; rcx = i = 0
	mov rax, arg(5)
	mov rsi, arg(4) ; rsi = M0_ptr
	lea rdi, [rsi + rax] ; rdi = M1_ptr
	shl rax, 1 ; rax = M_strideInBytesTimes2
	mov rbx, arg(0) ; rbx = srcX
	mov rdx, arg(1) ; rdx = srcY
	mov r8, arg(2) ; r8 = dstX		
	mov r9, arg(3) ; r9 = dstY
	mov r10, arg(6) ; r10 = numPoints

	xorpd xmm9, xmm9 ; xmm7 = vecZero
	movapd xmm10, [sym(km1_0_64f)] ; xmm10 = vecMinusOneZero
	movapd xmm11, [sym(kAVXFloat64MaskNegate)] ; xmm11 = vecMaskNegate
	movapd xmm12, [sym(km1_64f)]

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (size_t i = 0; i < numPoints; ++i)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopPoints:
		movsd xmm0, [rbx + rcx*COMPV_YASM_FLOAT64_SZ_BYTES]
		movsd xmm1, [rdx + rcx*COMPV_YASM_FLOAT64_SZ_BYTES]
		movsd xmm2, [r8 + rcx*COMPV_YASM_FLOAT64_SZ_BYTES]
		movsd xmm3, [r9 + rcx*COMPV_YASM_FLOAT64_SZ_BYTES]
		inc rcx
		unpcklpd xmm0, xmm1
		unpcklpd xmm2, xmm2
		unpcklpd xmm3, xmm3
		movapd xmm5, xmm2
		mulpd xmm5, xmm0
		movapd xmm7, xmm3
		mulpd xmm7, xmm0
		movapd xmm4, xmm0
		xorpd xmm4, xmm11
		xorpd xmm6, xmm6
		movapd xmm8, xmm4
		unpcklpd xmm6, xmm4
		unpckhpd xmm8, xmm12
		cmp rcx, r10
		;/!\ do not revert store instructions: will be slower (because of cache issues)
		movapd [rsi + 0*COMPV_YASM_FLOAT64_SZ_BYTES], xmm4
		movapd [rsi + 2*COMPV_YASM_FLOAT64_SZ_BYTES], xmm10
		movapd [rsi + 4*COMPV_YASM_FLOAT64_SZ_BYTES], xmm9
		movapd [rsi + 6*COMPV_YASM_FLOAT64_SZ_BYTES], xmm5
		movsd [rsi + 8*COMPV_YASM_FLOAT64_SZ_BYTES], xmm2
		lea rsi, [rsi + rax]
		movapd [rdi + 0*COMPV_YASM_FLOAT64_SZ_BYTES], xmm9
		movapd [rdi + 2*COMPV_YASM_FLOAT64_SZ_BYTES], xmm6
		movapd [rdi + 4*COMPV_YASM_FLOAT64_SZ_BYTES], xmm8
		movapd [rdi + 6*COMPV_YASM_FLOAT64_SZ_BYTES], xmm7
		movsd [rdi + 8*COMPV_YASM_FLOAT64_SZ_BYTES], xmm3
		lea rdi, [rdi + rax]
		jl .LoopPoints

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

%endif ; COMPV_YASM_ABI_IS_64BIT
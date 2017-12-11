;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(CompVMathMatrixMulABt_64f_Asm_X86_SSE2)
global sym(CompVMathMatrixMulGA_64f_Asm_X86_SSE2)
global sym(CompVMathMatrixBuildHomographyEqMatrix_64f_Asm_X86_SSE2)
global sym(CompVMathMatrixInvA3x3_64f_Asm_X86_SSE2)

section .data
	extern sym(kAVXFloat64MaskAbs)
	extern sym(k1_64f)
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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const compv_float64_t* srcX
; arg(1) -> COMPV_ALIGNED(SSE) const compv_float64_t* srcY
; arg(2) -> COMPV_ALIGNED(SSE) const compv_float64_t* dstX
; arg(3) -> COMPV_ALIGNED(SSE) const compv_float64_t* dstY
; arg(4) -> COMPV_ALIGNED(SSE) compv_float64_t* M
; arg(5) -> COMPV_ALIGNED(SSE) compv_uscalar_t M_strideInBytes
; arg(6) -> compv_uscalar_t numPoints
sym(CompVMathMatrixBuildHomographyEqMatrix_64f_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_XMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; alloc memory
	sub rsp, COMPV_YASM_REG_SZ_BYTES
	; [rsp + 0] = M_strideInBytesTimes2

	xor rcx, rcx ; rcx = i = 0
	mov rax, arg(5)
	mov rsi, arg(4) ; rsi = M0_ptr
	lea rdi, [rsi + rax] ; rdi = M1_ptr
	shl rax, 1
	mov [rsp + 0], rax ; [rsp + 0] = M_strideInBytesTimes2
	mov rax, arg(0) ; rax = srcX
	mov rbx, arg(2) ; dstX

	xorpd xmm7, xmm7 ; xmm7 = vecZero
	movapd xmm6, [sym(km1_0_64f)] ; xmm6 = vecMinusOneZero
	movapd xmm5, [sym(kAVXFloat64MaskNegate)] ; xmm5 = vecMaskNegate

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (size_t i = 0; i < numPoints; ++i)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopPoints:
		mov rdx, arg(1) ; srcY
		movsd xmm0, [rax + rcx*COMPV_YASM_FLOAT64_SZ_BYTES]
		movsd xmm1, [rdx + rcx*COMPV_YASM_FLOAT64_SZ_BYTES]
		unpcklpd xmm0, xmm1
		mov rdx, arg(3) ; dstY
		movsd xmm2, [rdx + rcx*COMPV_YASM_FLOAT64_SZ_BYTES]
		movsd xmm1, [rbx + rcx*COMPV_YASM_FLOAT64_SZ_BYTES]
		unpcklpd xmm2, xmm2
		unpcklpd xmm1, xmm1
		movapd [rdi + 8*COMPV_YASM_FLOAT64_SZ_BYTES], xmm2
		mulpd xmm2, xmm0
		movapd xmm3, xmm1
		movsd [rsi + 8*COMPV_YASM_FLOAT64_SZ_BYTES], xmm1
		xorpd xmm1, xmm1
		movapd xmm4, xmm0
		mulpd xmm3, xmm0
		xorpd xmm4, xmm5
		movapd [rsi + 0*COMPV_YASM_FLOAT64_SZ_BYTES], xmm4	
		unpcklpd xmm1, xmm4
		unpckhpd xmm4, [sym(km1_64f)]
		inc rcx	
		movapd [rsi + 2*COMPV_YASM_FLOAT64_SZ_BYTES], xmm6
		movapd [rsi + 4*COMPV_YASM_FLOAT64_SZ_BYTES], xmm7
		movapd [rdi + 0*COMPV_YASM_FLOAT64_SZ_BYTES], xmm7
		movapd [rsi + 6*COMPV_YASM_FLOAT64_SZ_BYTES], xmm3
		movapd [rdi + 2*COMPV_YASM_FLOAT64_SZ_BYTES], xmm1
		movapd [rdi + 4*COMPV_YASM_FLOAT64_SZ_BYTES], xmm4
		movapd [rdi + 6*COMPV_YASM_FLOAT64_SZ_BYTES], xmm2	
		add rsi, [rsp + 0]
		add rdi, [rsp + 0]
		cmp rcx, arg(6)
		jl .LoopPoints

	; free memory
	add rsp, COMPV_YASM_REG_SZ_BYTES

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
; arg(0) -> COMPV_ALIGNED(SSE) const compv_float64_t* A
; arg(1) -> COMPV_ALIGNED(SSE) compv_float64_t* R
; arg(2) -> compv_uscalar_t strideInBytes
; arg(3) -> compv_float64_t* det1
sym(CompVMathMatrixInvA3x3_64f_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 4
	COMPV_YASM_SAVE_XMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 16, rax
	sub rsp, COMPV_YASM_XMM_SZ_BYTES

	%define a0				rsi
	%define a1				rdi
	%define a2				rbx
	%define strideInBytes	rdx
	%define det1			rcx
	%define R				rax

	mov a0, arg(0)
	mov strideInBytes, arg(2)
	mov R, arg(1)
	mov det1, arg(3)
	lea a1, [a0 + strideInBytes]
	lea a2, [a1 + strideInBytes]

	movsd xmm0, [a1 + 1*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm5, [a1 + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm1, [a2 + 1*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm4, [a2 + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm2, [a0 + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm6, [a0 + 1*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm7, xmm1
	movsd xmm3, xmm4
	unpcklpd xmm0, xmm6
	unpcklpd xmm1, xmm7
	movsd xmm6, [a0 + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm7, [a1 + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
	unpcklpd xmm4, xmm3
	unpcklpd xmm5, xmm6
	mulpd xmm0, xmm4
	mulpd xmm1, xmm5
	unpcklpd xmm2, xmm7
	movsd xmm4, [a0 + 1*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm7, [a0 + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm5, [a1 + 1*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm6, [a1 + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
	unpcklpd xmm4, xmm5
	unpcklpd xmm6, xmm7
	mulpd xmm4, xmm6
	movapd xmm3, xmm0
	subpd xmm3, xmm1
	mulpd xmm3, xmm2
	movapd xmm0, xmm4
	shufpd xmm4, xmm4, 0x01
	subsd xmm0, xmm4
	mulsd xmm0, [a2 + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
	xorpd xmm7, xmm7
	movapd xmm4, xmm3
	shufpd xmm4, xmm4, 0x01
	subsd xmm3, xmm4
	addsd xmm0, xmm3
	comisd xmm0, xmm7
	movsd [det1], xmm0
	jz EndOfTheFunction

	;; detA not zero ;;

	movsd xmm7, [sym(k1_64f)]
	divsd xmm7, xmm0
	movsd xmm0, [a1 + 1*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm5, [a1 + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm2, [a0 + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm1, [a2 + 1*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm3, [a2 + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm6, xmm1
	movsd xmm4, xmm3
	unpcklpd xmm0, xmm2
	unpcklpd xmm1, xmm3
	movsd xmm3, [a0 + 1*COMPV_YASM_FLOAT64_SZ_BYTES]
	unpcklpd xmm4, xmm6
	unpcklpd xmm5, xmm3
	mulpd xmm0, xmm4
	mulpd xmm1, xmm5
	movsd xmm2, xmm3
	movsd xmm3, [a1 + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm5, [a2 + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm4, xmm3
	unpcklpd xmm2, xmm3
	unpcklpd xmm4, xmm5
	mulpd xmm2, xmm4
	movsd xmm6, [a1 + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm3, [a1 + 1*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm4, [a2 + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm5, [a0 + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
	unpcklpd xmm3, xmm4
	unpcklpd xmm5, xmm6
	mulpd xmm3, xmm5
	shufpd xmm7, xmm7, 0x0
	subpd xmm0, xmm1
	movapd[rsp + 0], xmm7
	mulpd xmm0, xmm7
	subpd xmm2, xmm3
	mulpd xmm2, xmm7
	movapd [R + strideInBytes*0 + 0*COMPV_YASM_FLOAT64_SZ_BYTES], xmm0 ; r0[0]
	movsd [R + strideInBytes*0 + 2*COMPV_YASM_FLOAT64_SZ_BYTES], xmm2 ; r0[2]
	shufpd xmm2, xmm2, 0x1
	movsd [R + strideInBytes*1 + 0*COMPV_YASM_FLOAT64_SZ_BYTES], xmm2 ; r1[0]
	movsd xmm0, [a0 + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm2, [a0 + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm1, [a2 + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm4, [a2 + 2*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm5, [a1 + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm3, [a1 + 2*COMPV_YASM_FLOAT64_SZ_BYTES]	
	movsd xmm6, xmm2
	movsd xmm7, xmm0
	unpcklpd xmm0, xmm2
	unpcklpd xmm1, xmm3
	unpcklpd xmm4, xmm5
	unpcklpd xmm6, xmm7
	mulpd xmm0, xmm4
	mulpd xmm1, xmm6
	movsd xmm2, [a1 + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm6, [a1 + 1*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm3, [a2 + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm4, [a2 + 1*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm5, xmm3
	movsd xmm7, [a0 + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
	unpcklpd xmm4, xmm5
	unpcklpd xmm6, xmm7
	subpd xmm0, xmm1
	movsd xmm5, [a0 + 1*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm7, [a2 + 1*COMPV_YASM_FLOAT64_SZ_BYTES]
	unpcklpd xmm2, xmm5
	mulpd xmm2, xmm4
	mulpd xmm0, [rsp + 0]
	unpcklpd xmm3, xmm7
	mulpd xmm3, xmm6
	movsd xmm1, [a0 + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm6, xmm5
	movsd xmm4, [a1 + 1*COMPV_YASM_FLOAT64_SZ_BYTES]
	movsd xmm5, [a1 + 0*COMPV_YASM_FLOAT64_SZ_BYTES]
	subpd xmm2, xmm3
	unpcklpd xmm1, xmm5
	mulpd xmm2, [rsp + 0]
	unpcklpd xmm4, xmm6
	mulpd xmm1, xmm4
	movupd [R + strideInBytes*1 + 1*COMPV_YASM_FLOAT64_SZ_BYTES], xmm0 ; r1[1]
	movapd [R + strideInBytes*2 + 0*COMPV_YASM_FLOAT64_SZ_BYTES], xmm2 ; r2[0]
	movapd xmm3, xmm1
	shufpd xmm3, xmm3, 0x01
	subsd xmm1, xmm3
	mulsd xmm1, [rsp + 0]
	movsd [R + strideInBytes*2 + 2*COMPV_YASM_FLOAT64_SZ_BYTES], xmm1 ; r2[2]

	;; EndOfTheFunction ;;
	EndOfTheFunction:

	%undef a0
	%undef a1
	%undef a2
	%undef strideInBytes
	%undef det1
	%undef R

	; free memory and unalign stack
	add rsp, COMPV_YASM_XMM_SZ_BYTES
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
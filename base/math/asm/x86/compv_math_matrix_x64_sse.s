;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "../../../asm/x86/compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(MatrixMulABt_64f_minpack1_Asm_X64_SSE2)
global sym(MatrixMaxAbsOffDiagSymm_64f_Asm_X64_SSE2)
global sym(MatrixBuildHomographyEqMatrix_64f_Asm_X64_SSE2)

section .data
	extern sym(kAVXFloat64MaskAbs)
	extern sym(km1_f64)
	extern sym(km1_0_f64)
	extern sym(kAVXFloat64MaskNegate)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) - > const COMPV_ALIGNED(SSE) compv_float64_t* A
; arg(1) - > const COMPV_ALIGNED(SSE) compv_float64_t* B
; arg(2) - > compv_uscalar_t aRows
; arg(3) - > compv_uscalar_t bRows
; arg(4) - > compv_uscalar_t bCols
; arg(5) - > compv_uscalar_t aStrideInBytes
; arg(6) - > compv_uscalar_t bStrideInBytes
; arg(7) - > COMPV_ALIGNED(SSE) compv_float64_t* R
; arg(8) - > compv_uscalar_t rStrideInBytes
; void MatrixMulABt_64f_minpack1_Asm_X64_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* A, const COMPV_ALIGNED(SSE) compv_float64_t* B, compv_uscalar_t aRows, compv_uscalar_t bRows, compv_uscalar_t bCols, compv_uscalar_t aStrideInBytes, compv_uscalar_t bStrideInBytes, COMPV_ALIGNED(SSE) compv_float64_t* R, compv_uscalar_t rStrideInBytes)
sym(MatrixMulABt_64f_minpack1_Asm_X64_SSE2):
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

	mov rax, arg(7) ; rax = R
	mov rsi, arg(2) ; rsi = aRows
	mov rdx, arg(0) ; rdx = a

	mov r8, arg(4) ; r8 = bCols
	lea r9, [r8 - 7] ; r9 = (bCols - 7)
	lea r10, [r8 - 3] ; r10 = (bCols - 3)
	lea r11, [r8 - 1] ; r11 = (bCols - 1)
	mov r12, arg(6) ; r12 = bStrideInBytes
	mov r13, arg(3)	; r13 = bRows
	mov r14, arg(8) ; r14 = rStrideInBytes
	mov r15, arg(5) ; r15 = aStrideInBytes

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (i = 0; i < aRows; ++i) 
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopARows
		mov rbx, arg(1) ; rbx = B
		xor rdi, rdi ; rdi = j = 0

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (j = 0; j < bRows; ++j)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopBRows
			pxor xmm0, xmm0 ; xmm0 = xmmSum
			xor rcx, rcx ; rcx = k = 0

			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (k = 0; k < bCols - 7; k += 8)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			cmp rcx, r9
			jge .EndOfLoop8BCols
			.Loop8BCols
				movapd xmm1, [rdx + rcx*8]
				movapd xmm2, [rdx + rcx*8 + 16]
				movapd xmm3, [rdx + rcx*8 + 16 + 16]
				movapd xmm4, [rdx + rcx*8 + 16 + 16 + 16]
				mulpd xmm1, [rbx + rcx*8]
				mulpd xmm2, [rbx + rcx*8 + 16]
				mulpd xmm3, [rbx + rcx*8 + 16 + 16]
				mulpd xmm4, [rbx + rcx*8 + 16 + 16 + 16]
				lea rcx, [rcx + 8] ; k += 8
				addpd xmm1, xmm2
				addpd xmm3, xmm4
				addpd xmm0, xmm1
				addpd xmm0, xmm3
				cmp rcx, r9 ; k <? (Cols - 7)
				jl .Loop8BCols
			.EndOfLoop8BCols

			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; if (k < bCols - 3)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			cmp rcx, r10
			jge .EndOfMoreThanFourRemain
			.MoreThanFourRemain
				movapd xmm1, [rdx + rcx*8]
				movapd xmm2, [rdx + rcx*8 + 16]
				mulpd xmm1, [rbx + rcx*8]
				mulpd xmm2, [rbx + rcx*8 + 16]
				lea rcx, [rcx + 4] ; k += 4
				addpd xmm0, xmm1
				addpd xmm0, xmm2
			.EndOfMoreThanFourRemain

			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; if (k < bCols - 1)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			cmp rcx, r11
			jge .EndOfMoreThanTwoRemains
				movapd xmm1, [rdx + rcx*8]
				mulpd xmm1, [rbx + rcx*8]
				lea rcx, [rcx + 2] ; k += 2
				addpd xmm0, xmm1
			.EndOfMoreThanTwoRemains

			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; if (k < bCols)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			test r8, 1
			jz .EndOfMoreThanOneRemains
				movsd xmm1, [rdx + rcx*8]
				movsd xmm2, [rbx + rcx*8]
				mulsd xmm1, xmm2
				addsd xmm0, xmm1
			.EndOfMoreThanOneRemains
			
			inc rdi ; ++j

			movapd xmm1, xmm0
			shufpd xmm1, xmm0, 0x1
			addsd xmm0, xmm1
			movsd [rax + rdi*8 - 8], xmm0
			
			lea rbx, [rbx + r12] ; b += bStrideInBytes
			cmp rdi, r13 ; j <? bRows
			jl .LoopBRows
		.EndOfLoopBRows

		dec rsi ; --i
		lea rax, [rax + r14] ; r += rStrideInBytes
		test rsi, rsi
		lea rdx, [rdx + r15] ; a += aStrideInBytes		
		jnz .LoopARows
	.EndOfLoopARows

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
; arg(0) -> COMPV_ALIGNED(SSE) compv_float64_t* S
; arg(1) -> compv_uscalar_t *row
; arg(2) -> compv_uscalar_t *col
; arg(3) -> compv_float64_t* max
; arg(4) -> compv_uscalar_t rowStart
; arg(5) -> compv_uscalar_t rowEnd
; arg(6) -> compv_uscalar_t strideInBytes
; void MatrixMaxAbsOffDiagSymm_64f_Asm_X64_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* S, compv_uscalar_t *row, compv_uscalar_t *col, compv_float64_t* max, compv_uscalar_t rowStart, compv_uscalar_t rowEnd, compv_uscalar_t strideInBytes)
sym(MatrixMaxAbsOffDiagSymm_64f_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_XMM 6
	push rsi
	push rdi
	push rbx
	push r12
	;; end prolog ;;

	; xmm4 = xmmAbsMask
	movapd xmm4, [sym(kAVXFloat64MaskAbs)]

	; xmm3 = xmmAllZerosMask
	cmppd xmm3, xmm3, 0x0 ; 0xfff....

	; xmm5 = xmmMax
	xorpd xmm5, xmm5
	
	mov rcx, arg(6) ; strideInBytes
	mov rax, arg(4) ; rowStart
	imul rax, rcx 
	mov rdx, arg(0)
	add rdx, rax ; rdx = S0_

	xor rcx, rcx ; rcx = row
	xor rbx, rbx ; rbx = col

	mov rsi, arg(4) ; rsi = j = rowStart
	mov r9, arg(6) ; r9 = strideInBytes
	mov r10, arg(5) ; r10 = rowEnd
	lea r8, [rsi - 3] ; r8 = j - 3
	lea r11, [rsi - 1] ; r11 = j - 1
	; r12 used as temp var
	

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = rowStart; j < rowEnd; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopRows
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < j - 3; i += 4)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor rdi, rdi ; rdi = i = 0
		cmp rdi, r8
		jge .EndOfLoopCols0
		.LoopCols0
			movapd xmm0, [rdx + rdi * 8]
			movapd xmm2, [rdx + rdi * 8 + 16]
			movapd xmm1, xmm5
			movapd xmm6, xmm5
			andpd xmm0, xmm4
			andpd xmm2, xmm4
			cmppd xmm1, xmm0, 1 ; testing lT instead of GT
			cmppd xmm6, xmm2, 1 ; testing lT instead of GT
			movmskpd rax, xmm1
			movmskpd r12, xmm6
			lea rdi, [rdi + 4] ; increment i
			
			; First #2 doubles
			test rax, rax
			jz .LoopCols0NotGreater1
				comisd xmm0, xmm5
				mov rcx, rsi ; update row
				pshufd xmm1, xmm0, 0x4E ; swap first and second doubles -> high first then low
				jbe .LoopCols0NotGreater2
					pshufd xmm5, xmm0, 0x44 ; duplicate low 8bytes
					lea rbx, [rdi - 4] ; update col = i + 0
				.LoopCols0NotGreater2
				comisd xmm1, xmm5
				jbe .LoopCols0NotGreater3
					lea rbx, [rdi - 3] ; update col = i + 1
					pshufd xmm5, xmm1, 0x44 ; duplicate low 8bytes
				.LoopCols0NotGreater3
			.LoopCols0NotGreater1

			; Second #2 doubles
			test r12, r12
			jz .LoopCols0NotGreater4
				comisd xmm2, xmm5
				mov rcx, rsi ; update row
				pshufd xmm6, xmm2, 0x4E ; swap first and second doube -> high first then low
				jbe .LoopCols0NotGreater5
					pshufd xmm5, xmm2, 0x44 ; duplicate low 8bytes
					lea rbx, [rdi - 2] ; update col = i + 0
				.LoopCols0NotGreater5
				comisd xmm6, xmm5
				jbe .LoopCols0NotGreater6
					lea rbx, [rdi - 1] ; update col = i + 1
					pshufd xmm5, xmm6, 0x44 ; duplicate low 8bytes
				.LoopCols0NotGreater6
			.LoopCols0NotGreater4

			cmp rdi, r8 ; i <? j - 3
			jl .LoopCols0
		.EndOfLoopCols0
		
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < (j - 1))
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp rdi, r11
		jge .EndOfMoreThanTwoRemain
		.MoreThanTwoRemain
			movapd xmm0, [rdx + rdi * 8]
			movapd xmm1, xmm5
			andpd xmm0, xmm4
			cmppd xmm1, xmm0, 1 ; testing lT instead of GT
			movmskpd rax, xmm1
			lea rdi, [rdi + 2] ; increment i
			test rax, rax
			jz .MoreThanTwoRemainNotGreater1
				comisd xmm0, xmm5
				mov rcx, rsi ; update row
				pshufd xmm1, xmm0, 0x4E ; swap first and second doube -> high first then low
				jbe .MoreThanTwoRemainNotGreater2
					pshufd xmm5, xmm0, 0x44 ; duplicate low 8bytes
					lea rbx, [rdi - 2] ; update col = i + 0
				.MoreThanTwoRemainNotGreater2
				comisd xmm1, xmm5
				jbe .MoreThanTwoRemainNotGreater3
					lea rbx, [rdi - 1] ; update col = i + 1
					pshufd xmm5, xmm1, 0x44 ; duplicate low 8bytes
				.MoreThanTwoRemainNotGreater3
			.MoreThanTwoRemainNotGreater1
		.EndOfMoreThanTwoRemain

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (j & 1)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		test rsi, 1
		jz .EndOfMoreThanOneRemain
		.MoreThanOneRemain
			movsd xmm0, [rdx + rdi * 8] ; 8 = sizeof(#1 double)
			andpd xmm0, xmm4
			comisd xmm0, xmm5
			jbe .MoreThanOneRemainNotGreater1
				pshufd xmm5, xmm0, 0x44 ; duplicate low 8bytes
				mov rbx, rdi ; update col = i
				mov rcx, rsi ; update row
			.MoreThanOneRemainNotGreater1
		.EndOfMoreThanOneRemain
		
		inc rsi
		inc r8
		inc r11
		add rdx, r9 ; S0_ += strideInBytes
		cmp rsi, r10 ; rsi <? rowEnd
		jl .LoopRows
	.EndOfLoopRows

	mov rax, arg(3) ; max
	mov rsi, arg(1) ; row
	mov rdi, arg(2) ; col
	movsd [rax], xmm5
	mov [rsi], rcx
	mov [rdi], rbx

	;; begin epilog ;;
	pop r12
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const COMPV_ALIGNED(SSE) compv_float64_t* srcX
; arg(1) -> const COMPV_ALIGNED(SSE) compv_float64_t* srcY
; arg(2) -> const COMPV_ALIGNED(SSE) compv_float64_t* dstX
; arg(3) -> const COMPV_ALIGNED(SSE) compv_float64_t* dstY
; arg(4) -> COMPV_ALIGNED(SSE) compv_float64_t* M
; arg(5) -> COMPV_ALIGNED(SSE)compv_uscalar_t M_strideInBytes
; arg(6) -> compv_uscalar_t numPoints
; void MatrixBuildHomographyEqMatrix_64f_Asm_X64_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* srcX, const COMPV_ALIGNED(SSE) compv_float64_t* srcY, const COMPV_ALIGNED(SSE) compv_float64_t* dstX, const COMPV_ALIGNED(SSE) compv_float64_t* dstY, COMPV_ALIGNED(SSE) compv_float64_t* M, COMPV_ALIGNED(SSE)compv_uscalar_t M_strideInBytes, compv_uscalar_t numPoints)
sym(MatrixBuildHomographyEqMatrix_64f_Asm_X64_SSE2):
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

	xorpd xmm9, xmm9 ; xmm7 = xmmZero
	movapd xmm10, [sym(km1_0_f64)]; xmm10 = xmmMinusOneZero
	movapd xmm11, [sym(kAVXFloat64MaskNegate)] ; xmm11 = xmmMaskNegate
	movapd xmm12, [sym(km1_f64)]

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (size_t i = 0; i < numPoints; ++i)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopPoints
		movsd xmm0, [rbx + rcx*8]
		movsd xmm1, [rdx + rcx*8]
		movsd xmm2, [r8 + rcx*8]
		movsd xmm3, [r9 + rcx*8]
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
		;/!\ do not revert store instructions: will be slower (because of cache issues)
		movapd [rsi + 0*8], xmm4
		movapd [rsi + 2*8], xmm10
		movapd [rsi + 4*8], xmm9
		movapd [rsi + 6*8], xmm5
		movsd [rsi + 8*8], xmm2
		movapd [rdi + 0*8], xmm9
		movapd [rdi + 2*8], xmm6
		movapd [rdi + 4*8], xmm8
		movapd [rdi + 6*8], xmm7
		movsd [rdi + 8*8], xmm3
		lea rsi, [rsi + rax]
		lea rdi, [rdi + rax]
		cmp rcx, arg(6)
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





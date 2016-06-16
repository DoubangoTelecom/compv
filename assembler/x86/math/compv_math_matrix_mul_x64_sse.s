;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "../compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(MatrixMulABt_float64_minpack1_Asm_X64_SSE2)
global sym(MatrixMaxAbsOffDiagSymm_float64_Asm_X64_SSE41)
global sym(MatrixMaxAbsOffDiagSymm_float64_Asm_X64_SSE2)

section .data
	extern sym(kAVXFloat64MaskAbs)

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
; void MatrixMulABt_float64_minpack1_Asm_X64_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* A, const COMPV_ALIGNED(SSE) compv_float64_t* B, compv_uscalar_t aRows, compv_uscalar_t bRows, compv_uscalar_t bCols, compv_uscalar_t aStrideInBytes, compv_uscalar_t bStrideInBytes, COMPV_ALIGNED(SSE) compv_float64_t* R, compv_uscalar_t rStrideInBytes)
sym(MatrixMulABt_float64_minpack1_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 9
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	;; end prolog ;;
	
	mov rax, arg(7) ; rax = R
	mov rsi, arg(2) ; rsi = aRows
	mov rdx, arg(0) ; rdx = a
	mov r8, arg(8) ; r8 = rStrideInBytes
	mov r9, arg(5) ; r9 = aStrideInBytes
	mov r10, arg(4)
	dec r10 ; r10 = bCols - 1
	mov r11, arg(6) ; r11 = bStrideInBytes
	mov r12, arg(3) ; r12 = bRows
	mov r13, arg(4) ; r13 = bCols

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
			; for (k = 0; k < bCols - 1; k += 2)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			.LoopBCols
				movapd xmm1, [rdx + rcx*8]
				mulpd xmm1, [rbx + rcx*8]
				lea rcx, [rcx + 2] ; k += 2
				addpd xmm0, xmm1	
				cmp rcx, r10 ; k <? (Cols - 1)
				jl .LoopBCols
			.EndOfLoopBCols
			
			test r13, 1 ; IsOdd(bCols)?
			jz .BColsNotOdd
				movsd xmm1, [rdx + rcx*8]
				movsd xmm2, [rbx + rcx*8]
				mulpd xmm1, xmm2
				addpd xmm0, xmm1
			.BColsNotOdd
			
			movapd xmm1, xmm0
			shufpd xmm1, xmm0, 0x1
			addpd xmm0, xmm1
			movsd [rax + rdi*8], xmm0

			inc rdi ; ++j
			lea rbx, [rbx + r11] ; b += bStrideInBytes
			cmp rdi, r12 ; j <? bRows
			jl .LoopBRows
		.EndOfLoopBRows

		dec rsi ; --i
		lea rax, [rax + r8] ; r += rStrideInBytes
		test rsi, rsi
		lea rdx, [rdx + r9] ; a += aStrideInBytes		
		jnz .LoopARows
	.EndOfLoopARows

	;; begin epilog ;;
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
; %1 -> 41 -> use SSE41, 2 -> use SSE2
; arg(0) -> COMPV_ALIGNED(SSE) compv_float64_t* S
; arg(1) -> compv_uscalar_t *row
; arg(2) -> compv_uscalar_t *col
; arg(3) -> compv_float64_t* max
; arg(4) -> compv_uscalar_t rowStart
; arg(5) -> compv_uscalar_t rowEnd
; arg(6) -> compv_uscalar_t strideInBytes
; void MatrixMaxAbsOffDiagSymm_float64_Asm_X64_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* S, compv_uscalar_t *row, compv_uscalar_t *col, compv_float64_t* max, compv_uscalar_t rowStart, compv_uscalar_t rowEnd, compv_uscalar_t strideInBytes)
%macro MatrixMaxAbsOffDiagSymm_float64_Asm_X64 1
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

%if %1 != 41 && %1 != 2
	%error "not supported"
%endif

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

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = rowStart; j < rowEnd; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopRows
		xor rdi, rdi ; rdi = i = 0

		lea r8, [rsi - 1]
		cmp rdi, r8
		jge .EndOfLoopCols1
		
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < j - 1; i += 2)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
		.LoopCols1
			movapd xmm0, [rdx + rdi * 8]
			movapd xmm1, xmm5
			andpd xmm0, xmm4
			cmppd xmm1, xmm0, 1 ; testing lT instead of GT
%if %1 == 41 ; SSE41
			ptest xmm1, xmm3
			lea rdi, [rdi + 2] ; increment i
%else ; SSE2
			movmskpd rax, xmm1
			lea rdi, [rdi + 2] ; increment i
			test rax, rax
%endif
			jz .LoopCols1NotGreater1
				comisd xmm0, xmm5
				mov rcx, rsi ; update row
				pshufd xmm1, xmm0, 0x4E ; swap first and second doube -> high first then low
				jbe .LoopCols1NotGreater2
					pshufd xmm5, xmm0, 0x44 ; duplicate low 8bytes
					lea rbx, [rdi - 2] ; update col = i + 0
				.LoopCols1NotGreater2
				comisd xmm1, xmm5
				jbe .LoopCols1NotGreater3
					lea rbx, [rdi - 1] ; update col = i + 1
					pshufd xmm5, xmm1, 0x44 ; duplicate low 8bytes
				.LoopCols1NotGreater3
			.LoopCols1NotGreater1

			cmp rdi, r8 ; i <? j -1
			jl .LoopCols1
		.EndOfLoopCols1
		
		cmp rdi, rsi ; i <? j
		jge .EndOfLoopCols2

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < j; i += 1)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopCols2
			movsd xmm0, [rdx + rdi * 8] ; 8 = sizeof(#1 double)
			andpd xmm0, xmm4
			comisd xmm0, xmm5
			lea rdi, [rdi + 1] ; increment i
			jbe .LoopCols2NotGreater1
				pshufd xmm5, xmm0, 0x44 ; duplicate low 8bytes
				lea rbx, [rdi - 1] ; update col = i + 0
				mov rcx, rsi ; update row
			.LoopCols2NotGreater1
			cmp rdi, rsi ; i <? j
			jl .LoopCols2
		.EndOfLoopCols2
		
		inc rsi
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
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(MatrixMaxAbsOffDiagSymm_float64_Asm_X64_SSE41):
	MatrixMaxAbsOffDiagSymm_float64_Asm_X64 41

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(MatrixMaxAbsOffDiagSymm_float64_Asm_X64_SSE2):
	MatrixMaxAbsOffDiagSymm_float64_Asm_X64 2

%endif ; COMPV_YASM_ABI_IS_64BIT





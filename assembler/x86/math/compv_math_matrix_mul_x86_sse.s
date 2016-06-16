;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "../compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(MatrixMulGA_float64_Asm_X86_SSE2)
global sym(MatrixMulGA_float32_Asm_X86_SSE2)
global sym(MatrixMulABt_float64_minpack1_Asm_X86_SSE2)
global sym(MatrixMaxAbsOffDiagSymm_float64_Asm_X86_SSE41)
global sym(MatrixMaxAbsOffDiagSymm_float64_Asm_X86_SSE2)

section .data
	extern sym(kAVXFloat64MaskAbs)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) compv_float64_t* ri
; arg(1) -> COMPV_ALIGNED(SSE) compv_float64_t* rj
; arg(2) -> const compv_float64_t* c1
; arg(3) -> const compv_float64_t* s1
; arg(4) -> compv_uscalar_t count
; void MatrixMulGA_float64_Asm_X86_SSE2(COMPV_ALIGNED(SSE) compv_float64_t* ri, COMPV_ALIGNED(SSE) compv_float64_t* rj, const compv_float64_t* c1, const compv_float64_t* s1, compv_uscalar_t count)
sym(MatrixMulGA_float64_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	push rbx
	;; end prolog ;;

	xor rcx, rcx ; rcx = i
	mov rbx, arg(4) ; rbx = count
	imul rbx, 8

	mov rax, arg(2)
	mov rdx, arg(3)
	movsd xmm0, [rax]
	movsd xmm1, [rdx]
	shufpd xmm0, xmm0, 0x0 ; xmm0 = xmmC
	shufpd xmm1, xmm1, 0x0 ; xmm1 = xmmS

	mov rax, arg(0) ; ri
	mov rdx, arg(1) ; rj 

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; while (i < count)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopProcess
		movapd xmm2, [rax + rcx] ; xmmRI
		movapd xmm3, [rdx + rcx] ; XmmRJ
		movapd xmm4, xmm1
		movapd xmm5, xmm0

		mulpd xmm4, xmm2
		mulpd xmm2, xmm0
		mulpd xmm5, xmm3
		mulpd xmm3, xmm1

		add rcx, 16

		subpd xmm5, xmm4
		addpd xmm2, xmm3
		
		movapd [rdx + rcx - 16], xmm5
		movapd [rax + rcx - 16], xmm2
		
		cmp rcx, rbx
		jl .LoopProcess

	;; begin epilog ;;
	pop rbx
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) compv_float32_t* ri
; arg(1) -> COMPV_ALIGNED(SSE) compv_float32_t* rj
; arg(2) -> const compv_float32_t* c1
; arg(3) -> const compv_float32_t* s1
; arg(4) -> compv_uscalar_t count
; void MatrixMulGA_float32_Asm_X86_SSE2(COMPV_ALIGNED(SSE) compv_float32_t* ri, COMPV_ALIGNED(SSE) compv_float32_t* rj, const compv_float32_t* c1, const compv_float32_t* s1, compv_uscalar_t count)
sym(MatrixMulGA_float32_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	push rbx
	;; end prolog ;;

	xor rcx, rcx ; rcx = i
	mov rbx, arg(4) ; rbx = count
	imul rbx, 4

	mov rax, arg(2)
	mov rdx, arg(3)
	movss xmm0, [rax]
	movss xmm1, [rdx]
	shufps xmm0, xmm0, 0x0 ; xmm0 = xmmC
	shufps xmm1, xmm1, 0x0 ; xmm1 = xmmS

	mov rax, arg(0) ; ri
	mov rdx, arg(1) ; rj 

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; while (i < count)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopProcess
		movaps xmm2, [rax + rcx] ; xmmRI
		movaps xmm3, [rdx + rcx] ; XmmRJ
		movaps xmm4, xmm1
		movaps xmm5, xmm0

		mulps xmm4, xmm2
		mulps xmm2, xmm0
		mulps xmm5, xmm3
		mulps xmm3, xmm1

		add rcx, 16

		subps xmm5, xmm4
		addps xmm2, xmm3
		
		movaps [rdx + rcx - 16], xmm5
		movaps [rax + rcx - 16], xmm2
		
		cmp rcx, rbx
		jl .LoopProcess

	;; begin epilog ;;
	pop rbx
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

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
; void MatrixMulABt_float64_minpack1_Asm_X86_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* A, const COMPV_ALIGNED(SSE) compv_float64_t* B, compv_uscalar_t aRows, compv_uscalar_t bRows, compv_uscalar_t bCols, compv_uscalar_t aStrideInBytes, compv_uscalar_t bStrideInBytes, COMPV_ALIGNED(SSE) compv_float64_t* R, compv_uscalar_t rStrideInBytes)
sym(MatrixMulABt_float64_minpack1_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 9
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; alloc memory
	sub rsp, 8
	; [rsp + 0] = bCols - 1

	mov rax, arg(4)
	dec rax
	mov [rsp + 0], rax

	mov rsi, arg(2) ; rsi = aRows
	mov rdx, arg(0) ; rdx = a
	mov rax, arg(7) ; rax = r

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
				cmp rcx, [rsp] ; k <? (Cols - 1)
				jl .LoopBCols
			.EndOfLoopBCols

			cmp rcx, arg(4) ; IsOdd(bCols)?
			jge .BColsNotOdd
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
			add rbx, arg(6) ; b += bStrideInBytes
			cmp rdi, arg(3) ; j <? bRows
			jl .LoopBRows
		.EndOfLoopBRows

		mov rdi, arg(5) ; aStrideInBytes
		mov rcx, arg(8) ; rStrideInBytes
		lea rdx, [rdx + rdi] ; a += aStrideInBytes
		lea rax, [rax + rcx] ; r += rStrideInBytes

		dec rsi ; --i
		test rsi, rsi
		jnz .LoopARows
	.EndOfLoopARows

	; free memory
	add rsp, 8

	;; begin epilog ;;
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
; void MatrixMaxAbsOffDiagSymm_float64_Asm_X86_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* S, compv_uscalar_t *row, compv_uscalar_t *col, compv_float64_t* max, compv_uscalar_t rowStart, compv_uscalar_t rowEnd, compv_uscalar_t strideInBytes)
%macro MatrixMaxAbsOffDiagSymm_float64_Asm_X86 1
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

%if %1 == 2 ; SSE2
	; alloc memory
	sub rsp, 8
	; [rsp + 0] = j - 1
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

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = rowStart; j < rowEnd; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopRows
		xor rdi, rdi ; rdi = i = 0

		lea rax, [rsi - 1]
		cmp rdi, rax
		jge .EndOfLoopCols1
%if %1 == 2 ; SSE2
		mov [rsp + 0], rax
%endif
		
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
%if %1 == 2 ; SSE2
			cmp rdi, [rsp + 0] ; i <? j -1
%else ; SSE41
			cmp rdi, rax ; i <? j -1
%endif
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
		add rdx, arg(6) ; S0_ += strideInBytes
		cmp rsi, arg(5) ; rsi <? rowEng
		jl .LoopRows
	.EndOfLoopRows

	mov rax, arg(3) ; max
	mov rsi, arg(1) ; row
	mov rdi, arg(2) ; col
	movsd [rax], xmm5
	mov [rsi], rcx
	mov [rdi], rbx

%if %1 == 2 ; SSE2
	; free memory
	add rsp, 8
%endif

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
sym(MatrixMaxAbsOffDiagSymm_float64_Asm_X86_SSE41):
	MatrixMaxAbsOffDiagSymm_float64_Asm_X86 41

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(MatrixMaxAbsOffDiagSymm_float64_Asm_X86_SSE2):
	MatrixMaxAbsOffDiagSymm_float64_Asm_X86 2






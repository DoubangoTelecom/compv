;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "../compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(MatrixMulGA_64f_Asm_X86_SSE2)
global sym(MatrixMulGA_32f_Asm_X86_SSE2)
global sym(MatrixMulABt_64f_minpack1_Asm_X86_SSE2)
global sym(MatrixMulABt_64f_3x3_Asm_X86_SSE41)
global sym(MatrixMaxAbsOffDiagSymm_64f_Asm_X86_SSE2)
global sym(MatrixBuildHomographyEqMatrix_64f_Asm_X86_SSE2)

section .data
	extern sym(kAVXFloat64MaskAbs)
	extern sym(km1_f64)
	extern sym(km1_0_f64)
	extern sym(kAVXFloat64MaskNegate)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) compv_float64_t* ri
; arg(1) -> COMPV_ALIGNED(SSE) compv_float64_t* rj
; arg(2) -> const compv_float64_t* c1
; arg(3) -> const compv_float64_t* s1
; arg(4) -> compv_uscalar_t count
; void MatrixMulGA_64f_Asm_X86_SSE2(COMPV_ALIGNED(SSE) compv_float64_t* ri, COMPV_ALIGNED(SSE) compv_float64_t* rj, const compv_float64_t* c1, const compv_float64_t* s1, compv_uscalar_t count)
sym(MatrixMulGA_64f_Asm_X86_SSE2):
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

		lea rcx, [rcx + 16]

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
; void MatrixMulGA_32f_Asm_X86_SSE2(COMPV_ALIGNED(SSE) compv_float32_t* ri, COMPV_ALIGNED(SSE) compv_float32_t* rj, const compv_float32_t* c1, const compv_float32_t* s1, compv_uscalar_t count)
sym(MatrixMulGA_32f_Asm_X86_SSE2):
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
; void MatrixMulABt_64f_minpack1_Asm_X86_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* A, const COMPV_ALIGNED(SSE) compv_float64_t* B, compv_uscalar_t aRows, compv_uscalar_t bRows, compv_uscalar_t bCols, compv_uscalar_t aStrideInBytes, compv_uscalar_t bStrideInBytes, COMPV_ALIGNED(SSE) compv_float64_t* R, compv_uscalar_t rStrideInBytes)
sym(MatrixMulABt_64f_minpack1_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 9
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; alloc memory
	sub rsp, 8 + 8
	; [rsp + 0] -> bCols - 3
	; [rsp + 8] -> bCols - 1

	mov rsi, arg(2) ; rsi = aRows
	mov rdx, arg(0) ; rdx = a

	mov rax, arg(4)
	lea rax, [rax - 3]
	mov [rsp + 0], rax ; [rsp + 0] = (bCols - 3)
	lea rax, [rax + 2]
	mov [rsp + 8], rax ; [rsp + 8] = ((bCols - 3) + 2) = (bCols - 1)
	lea rax, [rax - 6] ; rax = ((bCols - 1) - 6) = (bCols - 7)

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
			cmp rcx, rax
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
				cmp rcx, rax ; k <? (Cols - 7)
				jl .Loop8BCols
			.EndOfLoop8BCols

			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; if (k < bCols - 3)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			cmp rcx, [rsp + 0]
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
			cmp rcx, [rsp + 8]
			jge .EndOfMoreThanTwoRemains
			.MoreThanTwoRemains
				movapd xmm1, [rdx + rcx*8]
				mulpd xmm1, [rbx + rcx*8]
				lea rcx, [rcx + 2] ; k += 2
				addpd xmm0, xmm1
			.EndOfMoreThanTwoRemains

			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; if (k < bCols)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			cmp rcx, arg(4)
			jge .EndOfMoreThanOneRemains
			.MoreThanOneRemains
				movsd xmm1, [rdx + rcx*8]
				movsd xmm2, [rbx + rcx*8]
				mulsd xmm1, xmm2
				addsd xmm0, xmm1
			.EndOfMoreThanOneRemains

			movapd xmm1, xmm0
			mov rcx, arg(7) ; R
			shufpd xmm1, xmm0, 0x1
			addsd xmm0, xmm1
			movsd [rcx + rdi*8], xmm0
			
			inc rdi ; ++j
			add rbx, arg(6) ; b += bStrideInBytes
			cmp rdi, arg(3) ; j <? bRows
			jl .LoopBRows
		.EndOfLoopBRows

		dec rsi ; --i
		mov rcx, arg(8) ; rStrideInBytes
		mov rdi, arg(5) ; aStrideInBytes
		add arg(7), rcx ; r += rStrideInBytes
		lea rdx, [rdx + rdi] ; a += aStrideInBytes
		
		test rsi, rsi
		jnz .LoopARows
	.EndOfLoopARows

	; free memory
	add rsp, 8 + 8

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
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
; void MatrixMulABt_64f_3x3_Asm_X86_SSE41(const COMPV_ALIGNED(SSE) compv_float64_t* A, const COMPV_ALIGNED(SSE) compv_float64_t* B, compv_uscalar_t aRows, compv_uscalar_t bRows, compv_uscalar_t bCols, compv_uscalar_t aStrideInBytes, compv_uscalar_t bStrideInBytes, COMPV_ALIGNED(SSE) compv_float64_t* R, compv_uscalar_t rStrideInBytes);
sym(MatrixMulABt_64f_3x3_Asm_X86_SSE41):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 9
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	mov rsi, arg(0) ; rsi = A
	mov rdi, arg(1) ; rdi = B
	mov rbx, arg(7) ; rbx = R
	mov rcx, arg(6) ; rcx = bStrideInBytes
	mov rdx, arg(8) ; rdx = rStrideInBytes
	mov rax, arg(5) ; rax = aStrideInBytes

	movapd xmm0, [rsi]
	movsd xmm1, [rsi + 16]
	movapd xmm2, xmm0
	movapd xmm3, xmm1
	movapd xmm4, xmm0
	movapd xmm5, xmm1
	%if 0 ; FMA3 (AVX/SSE transition issue)
		dppd xmm0, [rdi], 0xff
		dppd xmm2, [rdi + rcx*1], 0xff
		dppd xmm4, [rdi + rcx*2], 0xff
		vfmadd231pd xmm0, xmm1, [rdi + 16]
		vfmadd231pd xmm2, xmm3, [rdi + rcx*1 + 16]
		vfmadd231pd xmm4, xmm5, [rdi + rcx*2 + 16]
	%else
		dppd xmm0, [rdi], 0xff
		mulpd xmm1, [rdi + 16]
		dppd xmm2, [rdi + rcx*1], 0xff
		mulpd xmm3, [rdi + rcx*1 + 16]
		dppd xmm4, [rdi + rcx*2], 0xff
		mulpd xmm5, [rdi + rcx*2 + 16]
		addpd xmm0, xmm1
		addpd xmm2, xmm3
		addpd xmm4, xmm5
	%endif
	shufpd xmm0, xmm2, 0x0
	movapd [rbx + 16], xmm4
	movapd [rbx], xmm0

	movapd xmm0, [rsi + rax]
	movsd xmm1, [rsi + rax + 16]
	movapd xmm2, xmm0
	movapd xmm3, xmm1
	movapd xmm4, xmm0
	movapd xmm5, xmm1
	%if 0 ; FMA3 (AVX/SSE transition issue)
		dppd xmm0, [rdi], 0xff
		vfmadd231pd xmm0, xmm1, [rdi + 16]
		dppd xmm2, [rdi + rcx*1], 0xff
		vfmadd231pd xmm2, xmm3, [rdi + rcx*1 + 16]
		dppd xmm4, [rdi + rcx*2], 0xff
		vfmadd231pd xmm4, xmm5, [rdi + rcx*2 + 16]		
	%else
		dppd xmm0, [rdi], 0xff
		mulpd xmm1, [rdi + 16]
		dppd xmm2, [rdi + rcx*1], 0xff
		mulpd xmm3, [rdi + rcx*1 + 16]
		dppd xmm4, [rdi + rcx*2], 0xff
		mulpd xmm5, [rdi + rcx*2 + 16]
		addpd xmm0, xmm1
		addpd xmm2, xmm3
		addpd xmm4, xmm5
	%endif
	shufpd xmm0, xmm2, 0x0
	movapd [rbx + rdx + 16], xmm4
	movapd [rbx + rdx], xmm0
	
	movapd xmm0, [rsi + rax*2]
	movsd xmm1, [rsi + rax*2 + 16]
	movapd xmm2, xmm0
	movapd xmm3, xmm1
	movapd xmm4, xmm0
	movapd xmm5, xmm1
	%if 0 ; FMA3 (AVX/SSE transition issue)
		dppd xmm0, [rdi], 0xff
		vfmadd231pd xmm0, xmm1, [rdi + 16]
		dppd xmm2, [rdi + rcx*1], 0xff
		vfmadd231pd xmm2, xmm3, [rdi + rcx*1 + 16]
		dppd xmm4, [rdi + rcx*2], 0xff
		vfmadd231pd xmm4, xmm5, [rdi + rcx*2 + 16]
	%else
		dppd xmm0, [rdi], 0xff
		mulpd xmm1, [rdi + 16]
		dppd xmm2, [rdi + rcx*1], 0xff
		mulpd xmm3, [rdi + rcx*1 + 16]
		dppd xmm4, [rdi + rcx*2], 0xff
		mulpd xmm5, [rdi + rcx*2 + 16]
		addpd xmm0, xmm1
		addpd xmm2, xmm3
		addpd xmm4, xmm5
	%endif
	shufpd xmm0, xmm2, 0x0
	movapd [rbx + rdx*2 + 16], xmm4
	movapd [rbx + rdx*2], xmm0

	;; begin epilog ;;
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
; void MatrixMaxAbsOffDiagSymm_64f_Asm_X86_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* S, compv_uscalar_t *row, compv_uscalar_t *col, compv_float64_t* max, compv_uscalar_t rowStart, compv_uscalar_t rowEnd, compv_uscalar_t strideInBytes)
sym(MatrixMaxAbsOffDiagSymm_64f_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_XMM 6
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; alloc memory
	sub rsp, 8 + 8 + 8
	; [rsp + 0] = j - 1
	; [rsp + 8] = row
	; [rsp + 16] = col

	; xmm4 = xmmAbsMask
	movapd xmm4, [sym(kAVXFloat64MaskAbs)]

	; xmm3 = xmmAllZerosMask
	cmppd xmm3, xmm3, 0x0 ; 0xfff....

	; xmm5 = xmmMax
	xorpd xmm5, xmm5

	xor rax, rax
	mov [rsp + 8], rax ; row = 0
	mov [rsp + 16], rax ; col = 0
	
	mov rcx, arg(6) ; strideInBytes
	mov rax, arg(4) ; rowStart
	imul rax, rcx 
	mov rdx, arg(0)
	add rdx, rax ; rdx = S0_

	xor rbx, rbx ; rbx = col
	mov rsi, arg(4) ; rsi = j = rowStart

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = rowStart; j < rowEnd; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopRows
		xor rdi, rdi ; rdi = i = 0

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < j - 3; i += 4)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea rcx, [rsi - 3] ; rcx = (j - 3)
		cmp rdi, rcx
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
			
			; First #2 doubles
			movmskpd rax, xmm1
			lea rdi, [rdi + 4] ; increment i
			test rax, rax
			movmskpd rax, xmm6 ; prepare mask for second #2 doubles
			jz .LoopCols0NotGreater1
				comisd xmm0, xmm5
				mov [rsp + 8], rsi ; update row
				pshufd xmm1, xmm0, 0x4E ; swap first and second doube -> high first then low
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
			test rax, rax
			jz .LoopCols0NotGreater4
				comisd xmm2, xmm5
				mov [rsp + 8], rsi ; update row
				pshufd xmm1, xmm2, 0x4E ; swap first and second doube -> high first then low
				jbe .LoopCols0NotGreater5
					pshufd xmm5, xmm2, 0x44 ; duplicate low 8bytes
					lea rbx, [rdi - 2] ; update col = i + 2 + 0
				.LoopCols0NotGreater5
				comisd xmm1, xmm5
				jbe .LoopCols0NotGreater6
					lea rbx, [rdi - 1] ; update col = i + 2 + 1
					pshufd xmm5, xmm1, 0x44 ; duplicate low 8bytes
				.LoopCols0NotGreater6
			.LoopCols0NotGreater4

			cmp rdi, rcx ; i <? j - 3
			jl .LoopCols0
		.EndOfLoopCols0
		
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < (j - 1))
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea rax, [rsi - 1]
		cmp rdi, rax
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
				mov [rsp + 8], rsi ; update row
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
				mov [rsp + 8], rsi ; update row
			.MoreThanOneRemainNotGreater1
		.EndOfMoreThanOneRemain			
		
		inc rsi
		add rdx, arg(6) ; S0_ += strideInBytes
		cmp rsi, arg(5) ; rsi <? rowEng
		jl .LoopRows
	.EndOfLoopRows

	mov rcx, [rsp + 8] ; row
	mov rax, arg(3) ; &max
	mov rsi, arg(1) ; &row
	mov rdi, arg(2) ; &col
	movsd [rax], xmm5
	mov [rsi], rcx
	mov [rdi], rbx

	; free memory
	add rsp, 8 + 8 + 8

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
; arg(0) -> const COMPV_ALIGNED(SSE) compv_float64_t* srcX
; arg(1) -> const COMPV_ALIGNED(SSE) compv_float64_t* srcY
; arg(2) -> const COMPV_ALIGNED(SSE) compv_float64_t* dstX
; arg(3) -> const COMPV_ALIGNED(SSE) compv_float64_t* dstY
; arg(4) -> COMPV_ALIGNED(SSE) compv_float64_t* M
; arg(5) -> COMPV_ALIGNED(SSE)compv_uscalar_t M_strideInBytes
; arg(6) -> compv_uscalar_t numPoints
; void MatrixBuildHomographyEqMatrix_64f_Asm_X86_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* srcX, const COMPV_ALIGNED(SSE) compv_float64_t* srcY, const COMPV_ALIGNED(SSE) compv_float64_t* dstX, const COMPV_ALIGNED(SSE) compv_float64_t* dstY, COMPV_ALIGNED(SSE) compv_float64_t* M, COMPV_ALIGNED(SSE)compv_uscalar_t M_strideInBytes, compv_uscalar_t numPoints)
sym(MatrixBuildHomographyEqMatrix_64f_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_XMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; alloc memory
	sub rsp, 8
	; [rsp + 0] = M_strideInBytesTimes2

	xor rcx, rcx ; rcx = i = 0
	mov rax, arg(5)
	mov rsi, arg(4) ; rsi = M0_ptr
	lea rdi, [rsi + rax] ; rdi = M1_ptr
	shl rax, 1
	mov [rsp + 0], rax ; [rsp + 0] = M_strideInBytesTimes2
	mov rax, arg(0) ; rax = srcX
	mov rbx, arg(2) ; dstX

	xorpd xmm7, xmm7 ; xmm7 = xmmZero
	movapd xmm6, [sym(km1_0_f64)]; xmm6 = xmmMinusOneZero
	movapd xmm5, [sym(kAVXFloat64MaskNegate)] ; xmm5 = xmmMaskNegate

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (size_t i = 0; i < numPoints; ++i)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopPoints
		mov rdx, arg(1)
		movsd xmm0, [rax + rcx*8]
		movsd xmm1, [rdx + rcx*8]
		unpcklpd xmm0, xmm1
		mov rdx, arg(3) ; dstY
		movsd xmm1, [rbx + rcx*8]
		movsd xmm2, [rdx + rcx*8]
		unpcklpd xmm1, xmm1
		unpcklpd xmm2, xmm2
		movapd [rdi + 8*8], xmm2
		mulpd xmm2, xmm0
		movapd xmm3, xmm1
		movsd [rsi + 8*8], xmm1
		xorpd xmm1, xmm1
		movapd xmm4, xmm0
		mulpd xmm3, xmm0
		xorpd xmm4, xmm5
		movapd [rsi + 0*8], xmm4	
		unpcklpd xmm1, xmm4
		unpckhpd xmm4, [sym(km1_f64)]
		inc rcx	
		movapd [rsi + 2*8], xmm6
		movapd [rsi + 4*8], xmm7
		movapd [rdi + 0*8], xmm7
		movapd [rsi + 6*8], xmm3
		movapd [rdi + 2*8], xmm1
		movapd [rdi + 4*8], xmm4
		movapd [rdi + 6*8], xmm2	
		add rsi, [rsp + 0]
		add rdi, [rsp + 0]
		cmp rcx, arg(6)
		jl .LoopPoints

	; free memory
	add rsp, 8

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

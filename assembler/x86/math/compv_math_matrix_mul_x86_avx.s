;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "../compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(MatrixMulGA_float64_Asm_X86_AVX)
global sym(MatrixMulGA_float32_Asm_X86_AVX)
global sym(MatrixMulABt_float64_minpack1_Asm_X86_AVX)
global sym(MatrixMulABt_float64_minpack1_Asm_X86_FMA3_AVX)

section .data
	extern sym(kAVXMaskstore_0_u64)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) compv_float64_t* ri
; arg(1) -> COMPV_ALIGNED(SSE) compv_float64_t* rj
; arg(2) -> const compv_float64_t* c1
; arg(3) -> const compv_float64_t* s1
; arg(4) -> compv_uscalar_t count
; void MatrixMulGA_float64_Asm_X86_AVX(COMPV_ALIGNED(AVX) compv_float64_t* ri, COMPV_ALIGNED(AVX) compv_float64_t* rj, const compv_float64_t* c1, const compv_float64_t* s1, compv_uscalar_t count)
sym(MatrixMulGA_float64_Asm_X86_AVX):
	vzeroupper
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
	vbroadcastsd ymm0, [rax]
	vbroadcastsd ymm1, [rdx]

	mov rax, arg(0) ; ri
	mov rdx, arg(1) ; rj 

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; while (i < count)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopProcess
		vmovapd ymm2, [rax + rcx] ; ymmRI
		vmovapd ymm3, [rdx + rcx] ; ymmRJ

		add rcx, 32

%if 0 ; FMA3 disabled
		vmulpd ymm4, ymm1, ymm2
		vmulpd ymm2, ymm2, ymm0
		vfmadd231pd ymm2, ymm3, ymm1
		vfmsub231pd ymm4, ymm0, ymm3
		vmovapd [rax + rcx - 32], ymm2
		vmovapd [rdx + rcx - 32], ymm4		
%else
		vmulpd ymm4, ymm1, ymm2
		vmulpd ymm2, ymm2, ymm0
		vmulpd ymm5, ymm0, ymm3
		vmulpd ymm3, ymm3, ymm1
		vsubpd ymm5, ymm5, ymm4
		vaddpd ymm2, ymm2, ymm3
		vmovapd [rdx + rcx - 32], ymm5
		vmovapd [rax + rcx - 32], ymm2
%endif
		
		cmp rcx, rbx
		jl .LoopProcess

	;; begin epilog ;;
	pop rbx
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) compv_float32_t* ri
; arg(1) -> COMPV_ALIGNED(SSE) compv_float32_t* rj
; arg(2) -> const compv_float32_t* c1
; arg(3) -> const compv_float32_t* s1
; arg(4) -> compv_uscalar_t count
; void MatrixMulGA_float32_Asm_X86_AVX(COMPV_ALIGNED(AVX) compv_float32_t* ri, COMPV_ALIGNED(AVX) compv_float32_t* rj, const compv_float32_t* c1, const compv_float32_t* s1, compv_uscalar_t count)
sym(MatrixMulGA_float32_Asm_X86_AVX):
	vzeroupper
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
	vbroadcastss ymm0, [rax]
	vbroadcastss ymm1, [rdx]

	mov rax, arg(0) ; ri
	mov rdx, arg(1) ; rj 

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; while (i < count)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopProcess
		vmovaps ymm2, [rax + rcx] ; ymmRI
		vmovaps ymm3, [rdx + rcx] ; ymmRJ

		add rcx, 32

%if 0 ; FMA3 disabled
		vmulps ymm4, ymm1, ymm2
		vmulps ymm2, ymm2, ymm0
		vfmadd231ps ymm2, ymm3, ymm1
		vfmsub231ps ymm4, ymm0, ymm3
		vmovaps [rax + rcx - 32], ymm2
		vmovaps [rdx + rcx - 32], ymm4		
%else
		vmulps ymm4, ymm1, ymm2
		vmulps ymm2, ymm2, ymm0
		vmulps ymm5, ymm0, ymm3
		vmulps ymm3, ymm3, ymm1
		vsubps ymm5, ymm5, ymm4
		vaddps ymm2, ymm2, ymm3
		vmovaps [rdx + rcx - 32], ymm5
		vmovaps [rax + rcx - 32], ymm2
%endif
		
		cmp rcx, rbx
		jl .LoopProcess

	;; begin epilog ;;
	pop rbx
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; %1 -> 0: FMA3 not supported, 1: FMA3 supported
; arg(0) - > const COMPV_ALIGNED(AVX) compv_float64_t* A
; arg(1) - > const COMPV_ALIGNED(AVX) compv_float64_t* B
; arg(2) - > compv_uscalar_t aRows
; arg(3) - > compv_uscalar_t bRows
; arg(4) - > compv_uscalar_t bCols
; arg(5) - > compv_uscalar_t aStrideInBytes
; arg(6) - > compv_uscalar_t bStrideInBytes
; arg(7) - > COMPV_ALIGNED(AVX) compv_float64_t* R
; arg(8) - > compv_uscalar_t rStrideInBytes
; void MatrixMulABt_float64_minpack1_Macro_X86_AVX(const COMPV_ALIGNED(AVX) compv_float64_t* A, const COMPV_ALIGNED(AVX) compv_float64_t* B, compv_uscalar_t aRows, compv_uscalar_t bRows, compv_uscalar_t bCols, compv_uscalar_t aStrideInBytes, compv_uscalar_t bStrideInBytes, COMPV_ALIGNED(AVX) compv_float64_t* R, compv_uscalar_t rStrideInBytes)
%macro MatrixMulABt_float64_minpack1_Macro_X86_AVX 1
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 9
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	mov rsi, arg(2) ; rsi = aRows
	mov rdx, arg(0) ; rdx = a

	vmovapd ymm3, [sym(kAVXMaskstore_0_u64)] ; ymm3 = ymmMaskToExtractFirst64Bits

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
			mov rax, arg(4); bCols
			vpxor ymm0, ymm0 ; ymm0 = ymmSum
			xor rcx, rcx ; rcx = k = 0
			lea rax, [rax - 3] ; rax = bCols - 3

			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (k = 0; k < bCols - 1; k += 2)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			.LoopBCols
				vmovapd ymm1, [rdx + rcx*8]
				%if %1 == 1 ; FMA3
					lea rcx, [rcx + 4] ; k += 4
					vfmadd231pd ymm0, ymm1, [rbx + rcx*8 - 32]
				%else
					vmulpd ymm1, ymm1, [rbx + rcx*8]
					lea rcx, [rcx + 4] ; k += 4
					vaddpd ymm0, ymm0, ymm1
				%endif
				cmp rcx, rax ; k <? (Cols - 3)
				jl .LoopBCols
			.EndOfLoopBCols

			lea rax, [rax + 3] ; rax = bCols
			cmp rcx, rax ; k <? bCols
			jge .BColsIsPerfectlyAVXAligned
				;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				; for (; k < bCols; k += 1)
				;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				.LoopBColsRemaining
					vmaskmovpd ymm1, ymm3, [rdx + rcx*8]
					vmaskmovpd ymm2, ymm3, [rbx + rcx*8]
					inc rcx
					%if %1 == 1 ; FMA3
						vfmadd231pd ymm0, ymm1, ymm2
					%else
						vmulpd ymm1, ymm1, ymm2
						vaddpd ymm0, ymm0, ymm1
					%endif
					cmp rcx, rax ; k <? bCols
					jl .LoopBColsRemaining
				.EndOfLoopBColsRemaining
			.BColsIsPerfectlyAVXAligned			
			
			vhaddpd ymm0, ymm0, ymm0
			vperm2f128 ymm1, ymm0, ymm0, 0x11
			mov rax, arg(7) ; R
			vaddpd ymm0, ymm0, ymm1
			vmaskmovpd [rax + rdi*8], ymm3, ymm0

			inc rdi ; ++j
			add rbx, arg(6) ; b += bStrideInBytes
			cmp rdi, arg(3) ; j <? bRows
			jl .LoopBRows
		.EndOfLoopBRows

		dec rsi ; --i
		mov rcx, arg(8) ; rStrideInBytes
		mov rdi, arg(5) ; aStrideInBytes
		lea rax, [rax + rcx] ; r += rStrideInBytes
		lea rdx, [rdx + rdi] ; a += aStrideInBytes
		mov arg(7), rax
		
		test rsi, rsi
		jnz .LoopARows
	.EndOfLoopARows

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(MatrixMulABt_float64_minpack1_Asm_X86_AVX):
	MatrixMulABt_float64_minpack1_Macro_X86_AVX 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(MatrixMulABt_float64_minpack1_Asm_X86_FMA3_AVX):
	MatrixMulABt_float64_minpack1_Macro_X86_AVX 1
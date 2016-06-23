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

global sym(MatrixMulGA_float64_Asm_X64_AVX)
global sym(MatrixMulABt_float64_minpack1_Asm_X64_AVX)

section .data
	extern sym(kAVXMaskstore_0_u64)
	extern sym(kAVXMaskstore_0_1_u64)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) compv_float64_t* ri
; arg(1) -> COMPV_ALIGNED(SSE) compv_float64_t* rj
; arg(2) -> const compv_float64_t* c1
; arg(3) -> const compv_float64_t* s1
; arg(4) -> compv_uscalar_t count
; void MatrixMulGA_float64_Asm_X64_AVX(COMPV_ALIGNED(AVX) compv_float64_t* ri, COMPV_ALIGNED(AVX) compv_float64_t* rj, const compv_float64_t* c1, const compv_float64_t* s1, compv_uscalar_t count)
sym(MatrixMulGA_float64_Asm_X64_AVX):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	COMPV_YASM_SAVE_YMM 13
	;; end prolog ;;

	xor rcx, rcx ; rcx = i
	mov r8, arg(4) ; r8 = count
	shl r8, 3
	lea r9, [r8 - 7*8] ; r9 = count - 7

	mov rax, arg(2)
	mov rdx, arg(3)
	vbroadcastsd ymm0, [rax]
	vbroadcastsd ymm1, [rdx]

	mov rax, arg(0) ; ri
	mov rdx, arg(1) ; rj 

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (i = 0; i < countSigned - 7; i += 8)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.Loop8
		vmovapd ymm10, [rax + rcx]
		vmovapd ymm11, [rdx + rcx]
		vmovapd ymm12, [rax + rcx + 4*8]
		vmovapd ymm13, [rdx + rcx + 4*8]
		lea rcx, [rcx + 8*8]
		vmulpd ymm2, ymm1, ymm10
		vmulpd ymm3, ymm0, ymm10
		vmulpd ymm4, ymm0, ymm11
		vmulpd ymm5, ymm1, ymm11
		vmulpd ymm6, ymm1, ymm12
		vmulpd ymm7, ymm0, ymm12
		vmulpd ymm8, ymm0, ymm13
		vmulpd ymm9, ymm1, ymm13
		vsubpd ymm2, ymm4, ymm2
		vaddpd ymm3, ymm3, ymm5
		vsubpd ymm6, ymm8, ymm6
		vaddpd ymm7, ymm7, ymm9
		vmovapd [rdx + rcx - 8*8], ymm2
		vmovapd [rdx + rcx - 8*8 + 4*8], ymm6
		vmovapd [rax + rcx - 8*8], ymm3		
		vmovapd [rax + rcx - 8*8 + 4*8], ymm7
		cmp rcx, r9
		jl .Loop8

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (; i < countSigned; i += 4)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp rcx, r8
	jge .EndOfLoop4
	.Loop4
		vmovapd ymm6, [rax + rcx]
		vmovapd ymm7, [rdx + rcx]
		lea rcx, [rcx + 4*8]
		vmulpd ymm2, ymm1, ymm6
		vmulpd ymm3, ymm0, ymm6
		vmulpd ymm4, ymm0, ymm7
		vmulpd ymm5, ymm1, ymm7
		vsubpd ymm2, ymm4, ymm2
		vaddpd ymm3, ymm3, ymm5
		vmovapd [rdx + rcx - 4*8], ymm2
		vmovapd [rax + rcx - 4*8], ymm3
		cmp rcx, r8
		jl .Loop4
	.EndOfLoop4

	;; begin epilog ;;
	COMPV_YASM_RESTORE_YMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; TODO(dmi): FMA good candidate but testing shows not faster
; arg(0) - > const COMPV_ALIGNED(AVX) compv_float64_t* A
; arg(1) - > const COMPV_ALIGNED(AVX) compv_float64_t* B
; arg(2) - > compv_uscalar_t aRows
; arg(3) - > compv_uscalar_t bRows
; arg(4) - > compv_uscalar_t bCols
; arg(5) - > compv_uscalar_t aStrideInBytes
; arg(6) - > compv_uscalar_t bStrideInBytes
; arg(7) - > COMPV_ALIGNED(AVX) compv_float64_t* R
; arg(8) - > compv_uscalar_t rStrideInBytes
; void MatrixMulABt_float64_minpack1_Macro_X64_AVX(const COMPV_ALIGNED(AVX) compv_float64_t* A, const COMPV_ALIGNED(AVX) compv_float64_t* B, compv_uscalar_t aRows, compv_uscalar_t bRows, compv_uscalar_t bCols, compv_uscalar_t aStrideInBytes, compv_uscalar_t bStrideInBytes, COMPV_ALIGNED(AVX) compv_float64_t* R, compv_uscalar_t rStrideInBytes)
sym(MatrixMulABt_float64_minpack1_Asm_X64_AVX):
	vzeroupper
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
	mov r12, arg(8) ; r12 = rStrideInBytes
	mov r13, arg(5) ; r13 = aStrideInBytes
	mov r14, arg(6) ; r14 = bStrideInBytes
	mov r15, arg(3) ; r15 = bRows

	vmovapd ymm3, [sym(kAVXMaskstore_0_u64)] ; ymm3 = ymmMaskToExtractFirst64Bits
	vmovapd ymm4, [sym(kAVXMaskstore_0_1_u64)] ; ymm4 = ymmMaskToExtractFirst128Bits

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
			vpxor ymm0, ymm0 ; ymm0 = ymmSum
			xor rcx, rcx ; rcx = k = 0

			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (k = 0; k < bColsSigned - 7; k += 8)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			cmp rcx, r9
			jge .EndOfLoop8Cols
			.Loop8Cols
				vmovapd ymm1, [rdx + rcx*8]
				vmovapd ymm2, [rdx + rcx*8 + 32]
				vmulpd ymm1, ymm1, [rbx + rcx*8]
				vmulpd ymm2, ymm2, [rbx + rcx*8 + 32]
				lea rcx, [rcx + 8] ; k += 8
				vaddpd ymm0, ymm0, ymm1
				vaddpd ymm0, ymm0, ymm2
				cmp rcx, r9
				jl .Loop8Cols
			.EndOfLoop8Cols

			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; if (k < bColsSigned - 3)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			cmp rcx, r10
			jge .EndOfMoreThanFourRemains
			.MoreThanFourRemains
				vmovapd ymm1, [rdx + rcx*8]
				vmulpd ymm1, ymm1, [rbx + rcx*8]
				lea rcx, [rcx + 4] ; k += 4
				vaddpd ymm0, ymm0, ymm1
			.EndOfMoreThanFourRemains

			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; if (k < bColsSigned - 1)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			cmp rcx, r11
			jge .EndOfMoreThanTwoRemains
			.MoreThanTwoRemains
				vmaskmovpd ymm1, ymm4, [rdx + rcx*8]
				vmaskmovpd ymm2, ymm4, [rbx + rcx*8]
				lea rcx, [rcx + 2]  ; k += 2
				vmulpd ymm1, ymm1, ymm2
				vaddpd ymm0, ymm0, ymm1
			.EndOfMoreThanTwoRemains

			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; if (k < bColsSigned)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			cmp rcx, r8
			jge .EndOfMoreThanOneRemains
			.MoreThanOneRemains
				vmaskmovpd ymm1, ymm3, [rdx + rcx*8]
				vmaskmovpd ymm2, ymm3, [rbx + rcx*8]
				vmulpd ymm1, ymm1, ymm2
				vaddpd ymm0, ymm0, ymm1
			.EndOfMoreThanOneRemains		
			
			vhaddpd ymm0, ymm0, ymm0
			vperm2f128 ymm1, ymm0, ymm0, 0x11
			vaddpd ymm0, ymm0, ymm1
			vmaskmovpd [rax + rdi*8], ymm3, ymm0

			inc rdi ; ++j
			lea rbx, [rbx + r14] ; b += bStrideInBytes
			cmp rdi, r15 ; j <? bRows
			jl .LoopBRows
		.EndOfLoopBRows

		dec rsi ; --i
		lea rax, [rax + r12] ; r += rStrideInBytes
		lea rdx, [rdx + r13] ; a += aStrideInBytes
		
		test rsi, rsi
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
	vzeroupper
	ret





%endif ; COMPV_YASM_ABI_IS_64BIT
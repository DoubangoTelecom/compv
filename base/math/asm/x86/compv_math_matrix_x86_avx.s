;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(CompVMathMatrixMulABt_64f_Asm_X86_AVX)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const COMPV_ALIGNED(AVX) compv_float64_t* A
; arg(1) -> compv_uscalar_t aRows
; arg(2) -> COMPV_ALIGNED(AVX) compv_uscalar_t aStrideInBytes
; arg(3) -> const COMPV_ALIGNED(AVX) compv_float64_t* B
; arg(4) -> compv_uscalar_t bRows
; arg(5) -> compv_uscalar_t bCols
; arg(6) -> COMPV_ALIGNED(AVX) compv_uscalar_t bStrideInBytes
; arg(7) -> COMPV_ALIGNED(AVX) compv_float64_t* R
; arg(8) -> COMPV_ALIGNED(AVX) compv_uscalar_t rStrideInBytes
sym(CompVMathMatrixMulABt_64f_Asm_X86_AVX):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 9
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; alloc memory
	sub rsp, (4*COMPV_YASM_REG_SZ_BYTES)

	%define bColsSignedMinus15      rsp + 0
	%define bColsSignedMinus7       bColsSignedMinus15 + (1*COMPV_YASM_REG_SZ_BYTES)
	%define bColsSignedMinus3		bColsSignedMinus7 + (1*COMPV_YASM_REG_SZ_BYTES)
	%define bColsSignedMinus1		bColsSignedMinus3 + (1*COMPV_YASM_REG_SZ_BYTES)

	mov rax, arg(5) ; bCols
	lea rsi, [rax - 15]
	lea rcx, [rax - 7]
	lea rdx, [rax - 3]
	lea rbx, [rax - 1]
	mov [bColsSignedMinus15], rsi
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
			vpxor ymm4, ymm4 ; ymm4 = vecSum
			xor k, k
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (k = 0; k < bColsSigned - 15; k += 16)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			cmp k, [bColsSignedMinus15]
			jge .EndOf_LoopBCols16
			.LoopBCols16:
				vmovapd ymm0, [A + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
				vmovapd ymm1, [A + (k + 4)*COMPV_YASM_FLOAT64_SZ_BYTES]
				vmovapd ymm2, [A + (k + 8)*COMPV_YASM_FLOAT64_SZ_BYTES]
				vmovapd ymm3, [A + (k + 12)*COMPV_YASM_FLOAT64_SZ_BYTES]
				vmulpd ymm0, ymm0, [B0 + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
				vmulpd ymm1, ymm1, [B0 + (k + 4)*COMPV_YASM_FLOAT64_SZ_BYTES]
				vmulpd ymm2, ymm2, [B0 + (k + 8)*COMPV_YASM_FLOAT64_SZ_BYTES]
				vmulpd ymm3, ymm3, [B0 + (k + 12)*COMPV_YASM_FLOAT64_SZ_BYTES]
				add k, 16
				vaddpd ymm0, ymm0, ymm1
				vaddpd ymm2, ymm2, ymm3
				vaddpd ymm4, ymm4, ymm0
				cmp k, [bColsSignedMinus15]
				vaddpd ymm4, ymm4, ymm2
				jl .LoopBCols16
				.EndOf_LoopBCols16:
				;; EndOf_LoopBCols16 ;;

			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; if (k < bColsSigned - 7)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			cmp k, [bColsSignedMinus7]
			jge .EndOf_LoopBCols8
			.LoopBCols8:
				vmovapd ymm0, [A + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
				vmovapd ymm1, [A + (k + 4)*COMPV_YASM_FLOAT64_SZ_BYTES]
				vmulpd ymm0, ymm0, [B0 + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
				vmulpd ymm1, ymm1, [B0 + (k + 4)*COMPV_YASM_FLOAT64_SZ_BYTES]
				add k, 8
				vaddpd ymm0, ymm0, ymm1
				vaddpd ymm4, ymm4, ymm0
				.EndOf_LoopBCols8:
				;; EndOf_LoopBCols8 ;;

			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; if (k < bColsSigned - 3)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			cmp k, [bColsSignedMinus3]
			jge .EndOf_LoopBCols4
			.LoopBCols4:
				vmovapd ymm0, [A + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
				vmulpd ymm0, ymm0, [B0 + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
				add k, 4
				vaddpd ymm4, ymm4, ymm0
				.EndOf_LoopBCols4:
				;; EndOf_LoopBCols4 ;;

			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; if (k < bColsSigned - 1)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			cmp k, [bColsSignedMinus1]
			jge .EndOf_LoopBCols2
			.LoopBCols2:
				vmovapd xmm0, [A + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
				vmulpd xmm0, [B0 + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
				add k, 2
				vaddpd xmm5, xmm4, xmm0
				vinsertf128 ymm4, ymm4, xmm5, 0x0
				.EndOf_LoopBCols2:
				;; EndOf_LoopBCols2 ;;

			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; if (bColsSigned & 1)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			cmp k, arg(5)
			jge .EndOf_LoopBCols1
			.LoopBCols1:
				vmovsd xmm0, [A + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
				vmulsd xmm0, [B0 + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
				add k, 2
				vaddsd xmm5, xmm4, xmm0
				vinsertf128 ymm4, ymm4, xmm5, 0x0
				.EndOf_LoopBCols1:
				;; EndOf_LoopBCols1 ;;

			vperm2f128 ymm5, ymm4, ymm4, 0x11
			vaddpd ymm4, ymm4, ymm5
			vshufpd xmm0, xmm4, xmm4, 0x1
			vaddpd xmm4, xmm4, xmm0
			vmovsd [R + j*COMPV_YASM_FLOAT64_SZ_BYTES], xmm4
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
	add rsp, (4*COMPV_YASM_REG_SZ_BYTES)

	%undef bColsSignedMinus15
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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(CompVMathMatrixMulABt_64f_Asm_X64_AVX)
global sym(CompVMathMatrixMulABt_64f_Asm_X64_FMA3_AVX)
global sym(CompVMathMatrixMulGA_64f_Asm_X64_AVX)
global sym(CompVMathMatrixMulGA_64f_Asm_X64_FMA3_AVX)

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
; %1 : 1: FMA3 enabled, 0: FMA3 disabled
%macro CompVMathMatrixMulABt_64f_Macro_X64_AVX 1
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

	%define bCols					r8
	%define bColsSignedMinus15      r9
	%define bColsSignedMinus7       r10
	%define bColsSignedMinus3		r11
	%define bColsSignedMinus1		r12

	mov bCols, arg(5)
	lea bColsSignedMinus15, [bCols - 15]
	lea bColsSignedMinus7, [bCols - 7]
	lea bColsSignedMinus3, [bCols - 3]
	lea bColsSignedMinus1, [bCols - 1]

	%define B0				rbx
	%define i				rsi
	%define j				rdi
	%define k				rcx
	%define A				rdx
	%define R				rax
	%define	bStrideInBytes	r13
	%define aStrideInBytes	r14
	%define rStrideInBytes	r15

	mov A, arg(0)
	mov R, arg(7)
	mov i, arg(1) ; aRows
	mov bStrideInBytes, arg(6)
	mov aStrideInBytes, arg(2)
	mov rStrideInBytes, arg(8)

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
			vxorpd ymm4, ymm4, ymm4 ; ymm4 = vecSum - "vpxor YMM" is AVX2 despite what is said on Intel website - https://www.felixcloutier.com/x86/pxor
			xor k, k
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (k = 0; k < bColsSigned - 15; k += 16)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			cmp k, bColsSignedMinus15
			jge .EndOf_LoopBCols16
			.LoopBCols16:
				vmovapd ymm0, [A + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
				vmovapd ymm1, [A + (k + 4)*COMPV_YASM_FLOAT64_SZ_BYTES]
				vmovapd ymm2, [A + (k + 8)*COMPV_YASM_FLOAT64_SZ_BYTES]
				vmovapd ymm3, [A + (k + 12)*COMPV_YASM_FLOAT64_SZ_BYTES]
				vmulpd ymm0, ymm0, [B0 + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
				vmulpd ymm1, ymm1, [B0 + (k + 4)*COMPV_YASM_FLOAT64_SZ_BYTES]
				%if %1
					vfmadd231pd ymm0, ymm2, [B0 + (k + 8)*COMPV_YASM_FLOAT64_SZ_BYTES]
					vfmadd231pd ymm1, ymm3, [B0 + (k + 12)*COMPV_YASM_FLOAT64_SZ_BYTES]
					add k, 16
					vaddpd ymm0, ymm0, ymm1
					cmp k, bColsSignedMinus15
					vaddpd ymm4, ymm4, ymm0
				%else
					vmulpd ymm2, ymm2, [B0 + (k + 8)*COMPV_YASM_FLOAT64_SZ_BYTES]
					vmulpd ymm3, ymm3, [B0 + (k + 12)*COMPV_YASM_FLOAT64_SZ_BYTES]
					add k, 16
					vaddpd ymm0, ymm0, ymm1
					vaddpd ymm2, ymm2, ymm3
					vaddpd ymm4, ymm4, ymm0
					cmp k, bColsSignedMinus15
					vaddpd ymm4, ymm4, ymm2
				%endif
				jl .LoopBCols16
				.EndOf_LoopBCols16:
				;; EndOf_LoopBCols16 ;;

			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; if (k < bColsSigned - 7)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			cmp k, bColsSignedMinus7
			jge .EndOf_LoopBCols8
			.LoopBCols8:
				vmovapd ymm0, [A + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
				vmovapd ymm1, [A + (k + 4)*COMPV_YASM_FLOAT64_SZ_BYTES]
				vmulpd ymm0, ymm0, [B0 + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
				%if %1
					vfmadd231pd ymm0, ymm1, [B0 + (k + 4)*COMPV_YASM_FLOAT64_SZ_BYTES]
					add k, 8
				%else
					vmulpd ymm1, ymm1, [B0 + (k + 4)*COMPV_YASM_FLOAT64_SZ_BYTES]
					add k, 8
					vaddpd ymm0, ymm0, ymm1
				%endif
				vaddpd ymm4, ymm4, ymm0
				.EndOf_LoopBCols8:
				;; EndOf_LoopBCols8 ;;

			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; if (k < bColsSigned - 3)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			cmp k, bColsSignedMinus3
			jge .EndOf_LoopBCols4
			.LoopBCols4:
				vmovapd ymm0, [A + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
				%if %1
					vfmadd231pd ymm4, ymm0, [B0 + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
					add k, 4
				%else
					vmulpd ymm0, ymm0, [B0 + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
					add k, 4
					vaddpd ymm4, ymm4, ymm0
				%endif
				.EndOf_LoopBCols4:
				;; EndOf_LoopBCols4 ;;

			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; if (k < bColsSigned - 1)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			cmp k, bColsSignedMinus1
			jge .EndOf_LoopBCols2
			.LoopBCols2:
				vmovapd xmm0, [A + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
				%if %1
					vmovapd xmm5, xmm4
					vfmadd231pd xmm5, xmm0, [B0 + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
					add k, 2
				%else
					vmulpd xmm0, [B0 + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
					add k, 2
					vaddpd xmm5, xmm4, xmm0
				%endif
				vinsertf128 ymm4, ymm4, xmm5, 0x0
				.EndOf_LoopBCols2:
				;; EndOf_LoopBCols2 ;;

			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; if (bColsSigned & 1)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			cmp k, bCols
			jge .EndOf_LoopBCols1
			.LoopBCols1:
				vmovsd xmm0, [A + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
				%if %1
					vmovapd xmm5, xmm4 ; "vmovapd" instead of "vmovsd" to preserve the higher 64bits instead of filling it with zeros
					vfmadd231sd xmm5, xmm0, [B0 + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
				%else
					vmulsd xmm0, [B0 + (k + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
					vaddsd xmm5, xmm4, xmm0
				%endif
				vinsertf128 ymm4, ymm4, xmm5, 0x0
				.EndOf_LoopBCols1:
				;; EndOf_LoopBCols1 ;;

			vperm2f128 ymm5, ymm4, ymm4, 0x11
			inc j
			vaddpd ymm4, ymm4, ymm5
			cmp j, arg(4)
			vshufpd xmm0, xmm4, xmm4, 0x1
			vaddpd xmm4, xmm4, xmm0
			lea B0, [B0 + bStrideInBytes]
			vmovsd [R + (j - 1)*COMPV_YASM_FLOAT64_SZ_BYTES], xmm4			
			jl .LoopBRows
			;; EndOf_LoopBRows ;;
		
		dec i
		lea A, [A + aStrideInBytes]
		lea R, [R + rStrideInBytes]
		jnz .LoopARows
		;; EndOf_LoopARows;;

	%undef bCols
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
	%undef	bStrideInBytes	
	%undef aStrideInBytes	
	%undef rStrideInBytes	

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
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVMathMatrixMulABt_64f_Asm_X64_AVX):
	CompVMathMatrixMulABt_64f_Macro_X64_AVX 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVMathMatrixMulABt_64f_Asm_X64_FMA3_AVX):
	CompVMathMatrixMulABt_64f_Macro_X64_AVX 1


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(AVX) compv_float64_t* ri
; arg(1) -> COMPV_ALIGNED(AVX) compv_float64_t* rj
; arg(2) -> const compv_float64_t* c1
; arg(3) -> const compv_float64_t* s1
; arg(4) -> compv_uscalar_t count
; %1 : 1: FMA3 enabled, 0: FMA3 disabled
%macro CompVMathMatrixMulGA_64f_Macro_X64_AVX 1
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	COMPV_YASM_SAVE_YMM 15
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 32, rax
	sub rsp, (2*COMPV_YASM_YMM_SZ_BYTES)

	%define vecC		rsp + 0
	%define vecS		vecC + (1*COMPV_YASM_YMM_SZ_BYTES)

	%define i			rsi
	%define ri			rdi
	%define rj			rbx
	%define count		rcx
	%define countMinus15	rdx
	%define countMinus7	rax

	mov rax, arg(2)
	mov rdx, arg(3)
	vbroadcastsd ymm0, [rax]
	vbroadcastsd ymm1, [rdx]
	vmovapd [vecC], ymm0
	vmovapd [vecS], ymm1

	mov ri, arg(0)
	mov rj, arg(1)
	mov count, arg(4)
	lea countMinus15, [count - 15]
	lea countMinus7, [count - 7]
	xor i, i

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (i = 0; i < countSigned - 15; i += 16)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp i, countMinus15
	jge .EndOf_LoopCount16
	mov r8, count
	shr r8, 4
	.LoopCount16:
		vmovapd ymm0, [ri + (i + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
		vmovapd ymm1, [ri + (i + 4)*COMPV_YASM_FLOAT64_SZ_BYTES]
		vmovapd ymm8, [ri + (i + 8)*COMPV_YASM_FLOAT64_SZ_BYTES]
		vmovapd ymm9, [ri + (i + 12)*COMPV_YASM_FLOAT64_SZ_BYTES]
		vmovapd ymm2, [rj + (i + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
		vmovapd ymm3, [rj + (i + 4)*COMPV_YASM_FLOAT64_SZ_BYTES]
		vmovapd ymm10, [rj + (i + 8)*COMPV_YASM_FLOAT64_SZ_BYTES]
		vmovapd ymm11, [rj + (i + 12)*COMPV_YASM_FLOAT64_SZ_BYTES]
		%if %1
			vmulpd ymm4, ymm0, [vecC]
			vmulpd ymm5, ymm1, [vecC]
			vmulpd ymm6, ymm0, [vecS]
			vmulpd ymm7, ymm1, [vecS]
			vmulpd ymm12, ymm8, [vecC]
			vmulpd ymm13, ymm9, [vecC]
			vmulpd ymm14, ymm8, [vecS]
			vmulpd ymm15, ymm9, [vecS]
			vfmadd231pd ymm4, ymm2, [vecS]
			vfmadd231pd ymm5, ymm3, [vecS]
			vfmsub231pd ymm6, ymm2, [vecC]
			vfmsub231pd ymm7, ymm3, [vecC]
			vfmadd231pd ymm12, ymm10, [vecS]
			vfmadd231pd ymm13, ymm11, [vecS]
			vfmsub231pd ymm14, ymm10, [vecC]
			vfmsub231pd ymm15, ymm11, [vecC]
		%else
			vmulpd ymm4, ymm0, [vecC]
			vmulpd ymm5, ymm1, [vecC]
			vmulpd ymm6, ymm2, [vecC]
			vmulpd ymm7, ymm3, [vecC]
			vmulpd ymm2, ymm2, [vecS]
			vmulpd ymm3, ymm3, [vecS]
			vmulpd ymm0, ymm0, [vecS]
			vmulpd ymm1, ymm1, [vecS]
			vmulpd ymm12, ymm8, [vecC]
			vmulpd ymm13, ymm9, [vecC]
			vmulpd ymm14, ymm10, [vecC]
			vmulpd ymm15, ymm11, [vecC]
			vmulpd ymm10, ymm10, [vecS]
			vmulpd ymm11, ymm11, [vecS]
			vmulpd ymm8, ymm8, [vecS]
			vmulpd ymm9, ymm9, [vecS]
			vaddpd ymm4, ymm4, ymm2
			vaddpd ymm5, ymm5, ymm3
			vsubpd ymm6, ymm6, ymm0
			vsubpd ymm7, ymm7, ymm1
			vaddpd ymm12, ymm12, ymm10
			vaddpd ymm13, ymm13, ymm11
			vsubpd ymm14, ymm14, ymm8
			vsubpd ymm15, ymm15, ymm9
		%endif
		dec r8
		vmovapd [ri + (i + 0)*COMPV_YASM_FLOAT64_SZ_BYTES], ymm4
		vmovapd [ri + (i + 4)*COMPV_YASM_FLOAT64_SZ_BYTES], ymm5
		vmovapd [ri + (i + 8)*COMPV_YASM_FLOAT64_SZ_BYTES], ymm12
		vmovapd [ri + (i + 12)*COMPV_YASM_FLOAT64_SZ_BYTES], ymm13
		vmovapd [rj + (i + 0)*COMPV_YASM_FLOAT64_SZ_BYTES], ymm6
		vmovapd [rj + (i + 4)*COMPV_YASM_FLOAT64_SZ_BYTES], ymm7
		vmovapd [rj + (i + 8)*COMPV_YASM_FLOAT64_SZ_BYTES], ymm14
		vmovapd [rj + (i + 12)*COMPV_YASM_FLOAT64_SZ_BYTES], ymm15

		lea i, [i + 16]
		jnz .LoopCount16
		.EndOf_LoopCount16
		;; EndOf_LoopCount16 ;;

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; if (i < countSigned - 7)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp i, countMinus7
	jge .EndOf_LoopCount8
	.LoopCount8:
		vmovapd ymm0, [ri + (i + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
		vmovapd ymm1, [ri + (i + 4)*COMPV_YASM_FLOAT64_SZ_BYTES]
		vmovapd ymm2, [rj + (i + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
		vmovapd ymm3, [rj + (i + 4)*COMPV_YASM_FLOAT64_SZ_BYTES]
		%if %1
			vmulpd ymm4, ymm0, [vecC]
			vmulpd ymm5, ymm1, [vecC]
			vmulpd ymm6, ymm0, [vecS]
			vmulpd ymm7, ymm1, [vecS]
			vfmadd231pd ymm4, ymm2, [vecS]
			vfmadd231pd ymm5, ymm3, [vecS]
			vfmsub231pd ymm6, ymm2, [vecC]
			vfmsub231pd ymm7, ymm3, [vecC]
		%else
			vmulpd ymm4, ymm0, [vecC]
			vmulpd ymm5, ymm1, [vecC]
			vmulpd ymm6, ymm2, [vecC]
			vmulpd ymm7, ymm3, [vecC]
			vmulpd ymm2, ymm2, [vecS]
			vmulpd ymm3, ymm3, [vecS]
			vmulpd ymm0, ymm0, [vecS]
			vmulpd ymm1, ymm1, [vecS]
			vaddpd ymm4, ymm4, ymm2
			vaddpd ymm5, ymm5, ymm3
			vsubpd ymm6, ymm6, ymm0
			vsubpd ymm7, ymm7, ymm1
		%endif
		vmovapd [ri + (i + 0)*COMPV_YASM_FLOAT64_SZ_BYTES], ymm4
		vmovapd [ri + (i + 4)*COMPV_YASM_FLOAT64_SZ_BYTES], ymm5
		vmovapd [rj + (i + 0)*COMPV_YASM_FLOAT64_SZ_BYTES], ymm6
		vmovapd [rj + (i + 4)*COMPV_YASM_FLOAT64_SZ_BYTES], ymm7

		add i, 8
		.EndOf_LoopCount8
		;; EndOf_LoopCount8 ;;

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (; i < countSigned; i += 4)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp i, count
	jge .EndOf_LoopCountOthers
	.LoopCountOthers:
		vmovapd ymm0, [ri + (i + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
		vmovapd ymm2, [rj + (i + 0)*COMPV_YASM_FLOAT64_SZ_BYTES]
		%if %1
			vmulpd ymm4, ymm0, [vecC]
			vmulpd ymm6, ymm0, [vecS]
			vfmadd231pd ymm4, ymm2, [vecS]
			vfmsub231pd ymm6, ymm2, [vecC]
		%else
			vmulpd ymm4, ymm0, [vecC]
			vmulpd ymm6, ymm2, [vecC]
			vmulpd ymm2, ymm2, [vecS]
			vmulpd ymm0, ymm0, [vecS]
			vaddpd ymm4, ymm4, ymm2
			vsubpd ymm6, ymm6, ymm0
		%endif
		vmovapd [ri + (i + 0)*COMPV_YASM_FLOAT64_SZ_BYTES], ymm4
		vmovapd [rj + (i + 0)*COMPV_YASM_FLOAT64_SZ_BYTES], ymm6

		add i, 4
		cmp i, count
		jl .LoopCountOthers
		.EndOf_LoopCountOthers
		;; EndOf_LoopCountOthers ;;

	%undef vecC
	%undef vecS

	%undef i
	%undef ri
	%undef rj
	%undef count
	%undef countMinus15
	%undef countMinus7

	; free memory and unalign stack
	add rsp, (2*COMPV_YASM_YMM_SZ_BYTES)
	COMPV_YASM_UNALIGN_STACK

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_YMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVMathMatrixMulGA_64f_Asm_X64_AVX):
	CompVMathMatrixMulGA_64f_Macro_X64_AVX 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVMathMatrixMulGA_64f_Asm_X64_FMA3_AVX):
	CompVMathMatrixMulGA_64f_Macro_X64_AVX 1

%endif ; COMPV_YASM_ABI_IS_64BIT
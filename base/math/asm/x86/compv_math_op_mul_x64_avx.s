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

global sym(CompVMathOpMulMulABt_32f32f32f_Asm_X64_AVX)
global sym(CompVMathOpMulMulABt_32f32f32f_Asm_X64_FMA3_AVX)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(AVX) const compv_float32_t* Aptr,
; arg(1) -> COMPV_ALIGNED(AVX) const compv_float32_t* Bptr,
; arg(2) -> COMPV_ALIGNED(AVX) compv_float32_t* Rptr,
; arg(3) -> const compv_uscalar_t Bcols,
; arg(4) -> const compv_uscalar_t Arows,
; arg(5) -> const compv_uscalar_t Brows,
; arg(6) -> COMPV_ALIGNED(AVX) const compv_uscalar_t Astride,
; arg(7) -> COMPV_ALIGNED(AVX) const compv_uscalar_t Bstride,
; arg(8) -> COMPV_ALIGNED(AVX) const compv_uscalar_t Rstride
; %1 -> 1: FMA3 enabled, 0: FMA3 disabled
%macro CompVMathOpMulMulABt_32f32f32f_macro_X64_AVX 1
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 9
	COMPV_YASM_SAVE_YMM 7
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	push r14
	;; end prolog ;;

	%define Aptr		rax
	%define Bptr		rcx
	%define Rptr		rdx
	%define Bcols		rsi
	%define Arows		rdi
	%define Brows		rbx
	%define Astride		r8
	%define Bstride		r9
	%define Rstride		r10

	%define Bcols16		r11
	%define Bcols4		r12
	%define j			r13
	%define k			r14

	mov Aptr, arg(0)
	mov Bptr, arg(1)
	mov Rptr, arg(2)
	mov Bcols, arg(3)
	mov Arows, arg(4)
	mov Brows, arg(5)
	mov Astride, arg(6)
	mov Bstride, arg(7)
	mov Rstride, arg(8)

	mov Bcols16, Bcols
	mov Bcols4, Bcols
	and Bcols16, -16 ; Not Using Bcols32 to make sure MD5 match. Also, not big opt
	and Bcols4, -4

	lea Astride, [Astride * COMPV_YASM_FLOAT32_SZ_BYTES]
	lea Bstride, [Bstride * COMPV_YASM_FLOAT32_SZ_BYTES]
	lea Rstride, [Rstride * COMPV_YASM_FLOAT32_SZ_BYTES]

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t i = 0; i < Arows; ++i)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopArows:
		mov Bptr, arg(1)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (compv_uscalar_t j = 0; j < Brows; ++j)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor j, j
		.LoopBrows:
			vpxor xmm0, xmm0
			vpxor xmm1, xmm1
			vpxor xmm2, xmm2
			vpxor xmm3, xmm3
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (k = 0; k < Bcols16; k += 16)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			xor k, k
			test Bcols16, Bcols16
			jz .EndOf_LoopBcols16
			.LoopBcols16:
				vmovaps xmm4, [Aptr + (k + 0)*COMPV_YASM_FLOAT32_SZ_BYTES]
				vmovaps xmm5, [Aptr + (k + 4)*COMPV_YASM_FLOAT32_SZ_BYTES]
				vmovaps xmm6, [Aptr + (k + 8)*COMPV_YASM_FLOAT32_SZ_BYTES]
				vmovaps xmm7, [Aptr + (k + 12)*COMPV_YASM_FLOAT32_SZ_BYTES]
				%if %1
					vfmadd231ps xmm0, xmm4, [Bptr + (k + 0)*COMPV_YASM_FLOAT32_SZ_BYTES]
					vfmadd231ps xmm1, xmm5, [Bptr + (k + 4)*COMPV_YASM_FLOAT32_SZ_BYTES]
					vfmadd231ps xmm2, xmm6, [Bptr + (k + 8)*COMPV_YASM_FLOAT32_SZ_BYTES]
					vfmadd231ps xmm3, xmm7, [Bptr + (k + 12)*COMPV_YASM_FLOAT32_SZ_BYTES]
				%else
					vmulps xmm4, xmm4, [Bptr + (k + 0)*COMPV_YASM_FLOAT32_SZ_BYTES]
					vmulps xmm5, xmm5, [Bptr + (k + 4)*COMPV_YASM_FLOAT32_SZ_BYTES]
					vmulps xmm6, xmm6, [Bptr + (k + 8)*COMPV_YASM_FLOAT32_SZ_BYTES]
					vmulps xmm7, xmm7, [Bptr + (k + 12)*COMPV_YASM_FLOAT32_SZ_BYTES]
					vaddps xmm0, xmm0, xmm4
					vaddps xmm1, xmm1, xmm5
					vaddps xmm2, xmm2, xmm6
					vaddps xmm3, xmm3, xmm7
				%endif

				add k, 16
				cmp k, Bcols16
				jl .LoopBcols16
			.EndOf_LoopBcols16:

			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (; k < Bcols4; k += 4)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			cmp k, Bcols4
			jge .EndOf_LoopBcols4
			.LoopBcols4:
				vmovaps xmm4, [Aptr + (k + 0)*COMPV_YASM_FLOAT32_SZ_BYTES]
				%if %1
					vfmadd231ps xmm0, xmm4, [Bptr + (k + 0)*COMPV_YASM_FLOAT32_SZ_BYTES]
				%else
					vmulps xmm4, xmm4, [Bptr + (k + 0)*COMPV_YASM_FLOAT32_SZ_BYTES]
					vaddps xmm0, xmm0, xmm4
				%endif
				add k, 4
				cmp k, Bcols4
				jl .LoopBcols4
			.EndOf_LoopBcols4:

			vaddps xmm0, xmm0, xmm1
			vaddps xmm2, xmm2, xmm3
			vaddps xmm0, xmm0, xmm2
			vshufps xmm1, xmm0, xmm0, 0x0E
			vaddps xmm0, xmm0, xmm1
			vshufps xmm1, xmm0, xmm0, 0x01
			vaddss xmm0, xmm0, xmm1

			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (; k < Bcols; k += 1)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			cmp k, Bcols
			jge .EndOf_LoopBcols1
			.LoopBcols1:
				vmovss xmm4, [Aptr + (k + 0)*COMPV_YASM_FLOAT32_SZ_BYTES]
				%if %1
					vfmadd231ss xmm0, xmm4, [Bptr + (k + 0)*COMPV_YASM_FLOAT32_SZ_BYTES]
				%else
					vmulss xmm4, xmm4, [Bptr + (k + 0)*COMPV_YASM_FLOAT32_SZ_BYTES]
					vaddss xmm0, xmm0, xmm4
				%endif
				inc k
				cmp k, Bcols
				jl .LoopBcols1
			.EndOf_LoopBcols1:

			vmovss [Rptr + j*COMPV_YASM_FLOAT32_SZ_BYTES], xmm0
			lea Bptr, [Bptr + Bstride]
			inc j
			cmp j, Brows
			jl .LoopBrows
		.EndOf_LoopBrows:

		dec Arows
		lea Aptr, [Aptr + Astride]
		lea Rptr, [Rptr + Rstride]
		jnz .LoopArows
	.EndOf_LoopArows:

	%undef Aptr		
	%undef Bptr		
	%undef Rptr		
	%undef Bcols		
	%undef Arows		
	%undef Brows		
	%undef Astride		
	%undef Bstride		
	%undef Rstride		

	%undef Bcols16		
	%undef Bcols4		
	%undef j			
	%undef k			

	;; begin epilog ;;
	pop r14
	pop r13
	pop r12
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
sym(CompVMathOpMulMulABt_32f32f32f_Asm_X64_AVX):
	CompVMathOpMulMulABt_32f32f32f_macro_X64_AVX 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVMathOpMulMulABt_32f32f32f_Asm_X64_FMA3_AVX):
	CompVMathOpMulMulABt_32f32f32f_macro_X64_AVX 1

%endif ; COMPV_YASM_ABI_IS_64BIT

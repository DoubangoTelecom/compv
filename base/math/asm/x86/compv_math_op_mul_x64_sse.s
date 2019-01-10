;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(CompVMathOpMulMulABt_32f32f32f_Asm_X64_SSE2)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const compv_float32_t* Aptr,
; arg(1) -> COMPV_ALIGNED(SSE) const compv_float32_t* Bptr,
; arg(2) -> COMPV_ALIGNED(SSE) compv_float32_t* Rptr,
; arg(3) -> const compv_uscalar_t Bcols,
; arg(4) -> const compv_uscalar_t Arows,
; arg(5) -> const compv_uscalar_t Brows,
; arg(6) -> COMPV_ALIGNED(SSE) const compv_uscalar_t Astride,
; arg(7) -> COMPV_ALIGNED(SSE) const compv_uscalar_t Bstride,
; arg(8) -> COMPV_ALIGNED(SSE) const compv_uscalar_t Rstride
sym(CompVMathOpMulMulABt_32f32f32f_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 9
	COMPV_YASM_SAVE_XMM 7
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
	and Bcols16, -16
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
			pxor xmm0, xmm0
			pxor xmm1, xmm1
			pxor xmm2, xmm2
			pxor xmm3, xmm3
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (k = 0; k < Bcols16; k += 16)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			xor k, k
			test Bcols16, Bcols16
			jz .EndOf_LoopBcols16
			.LoopBcols16:
				movaps xmm4, [Aptr + (k + 0)*COMPV_YASM_FLOAT32_SZ_BYTES]
				movaps xmm5, [Aptr + (k + 4)*COMPV_YASM_FLOAT32_SZ_BYTES]
				movaps xmm6, [Aptr + (k + 8)*COMPV_YASM_FLOAT32_SZ_BYTES]
				movaps xmm7, [Aptr + (k + 12)*COMPV_YASM_FLOAT32_SZ_BYTES]
				mulps xmm4, [Bptr + (k + 0)*COMPV_YASM_FLOAT32_SZ_BYTES]
				mulps xmm5, [Bptr + (k + 4)*COMPV_YASM_FLOAT32_SZ_BYTES]
				mulps xmm6, [Bptr + (k + 8)*COMPV_YASM_FLOAT32_SZ_BYTES]
				mulps xmm7, [Bptr + (k + 12)*COMPV_YASM_FLOAT32_SZ_BYTES]
				add k, 16
				cmp k, Bcols16
				addps xmm0, xmm4
				addps xmm1, xmm5
				addps xmm2, xmm6
				addps xmm3, xmm7
				jl .LoopBcols16
			.EndOf_LoopBcols16:

			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (; k < Bcols4; k += 4)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			cmp k, Bcols4
			jge .EndOf_LoopBcols4
			.LoopBcols4:
				movaps xmm4, [Aptr + (k + 0)*COMPV_YASM_FLOAT32_SZ_BYTES]
				mulps xmm4, [Bptr + (k + 0)*COMPV_YASM_FLOAT32_SZ_BYTES]
				add k, 4
				cmp k, Bcols4
				addps xmm0, xmm4
				jl .LoopBcols4
			.EndOf_LoopBcols4:

			addps xmm0, xmm1
			addps xmm2, xmm3
			addps xmm0, xmm2
			movaps xmm1, xmm0
			shufps xmm1, xmm0, 0x0E
			addps xmm0, xmm1
			movaps xmm1, xmm0
			shufps xmm1, xmm0, 0x01
			addss xmm0, xmm1

			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (; k < Bcols; k += 1)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			cmp k, Bcols
			jge .EndOf_LoopBcols1
			.LoopBcols1:
				movss xmm4, [Aptr + (k + 0)*COMPV_YASM_FLOAT32_SZ_BYTES]
				mulss xmm4, [Bptr + (k + 0)*COMPV_YASM_FLOAT32_SZ_BYTES]
				inc k
				cmp k, Bcols
				addss xmm0, xmm4
				jl .LoopBcols4
			.EndOf_LoopBcols1:

			movss [Rptr + j*COMPV_YASM_FLOAT32_SZ_BYTES], xmm0
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
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret


%endif ; COMPV_YASM_ABI_IS_64BIT

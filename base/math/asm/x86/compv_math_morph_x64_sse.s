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

%define CompVMathMorphProcessOpErode	0
%define CompVMathMorphProcessOpDilate	1

global sym(CompVMathMorphProcessErode_8u_Asm_X64_SSE2)
global sym(CompVMathMorphProcessDilate_8u_Asm_X64_SSE2)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const compv_uscalar_t* strelInputPtrsPtr
; arg(1) -> const compv_uscalar_t strelInputPtrsCount
; arg(2) -> uint8_t* outPtr
; arg(3) -> const compv_uscalar_t width
; arg(4) -> const compv_uscalar_t height
; arg(5) -> const compv_uscalar_t stride
%macro CompVMathMorphProcessOp_8u_Macro_X64_SSE2 1
	%define CompVMathMorphProcessOp %1
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	COMPV_YASM_SAVE_XMM 7
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	push r14
	push r15
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 16, rax
	sub rsp, (16*COMPV_YASM_UINT8_SZ_BYTES)

	%define mem						rsp + 0
	%define strelInputPtrsPtr		rbx
	%define strelInputPtrsCount		rcx
	%define outPtr					rdx
	%define width					rsi
	%define height					rdi
	%define	stride					rax
	%define i						r8
	%define width64					r9
	%define width16					r10
	%define strelInputPtrsPtrv		r11 
	%define v						r12
	%define uint8					r13b
	%define k						r14
	%define strelInputPtrsPad		r15

	mov strelInputPtrsPtr, arg(0)
	mov strelInputPtrsCount, arg(1)
	mov outPtr, arg(2)
	mov width, arg(3)
	mov height, arg(4)
	mov stride, arg(5)
	mov width16, width
	mov width64, width
	lea strelInputPtrsPad, [width + 15]
	and width16, -16
	and width64, -64
	and strelInputPtrsPad, -16
	neg strelInputPtrsPad
	add strelInputPtrsPad, stride

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0, k = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	xor k, k
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width64; i += 64, k += 64)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		test width64, width64
		jz .EndOf_LoopWidth64
		.LoopWidth64:
			mov strelInputPtrsPtrv, [strelInputPtrsPtr + (0*COMPV_YASM_REG_SZ_BYTES)] ; strelInputPtrsPtrv = (strelInputPtrsPtr[v=0])
			movdqu xmm0, [strelInputPtrsPtrv + (k * COMPV_YASM_UINT8_SZ_BYTES) + (0*COMPV_YASM_XMM_SZ_BYTES)]
			movdqu xmm1, [strelInputPtrsPtrv + (k * COMPV_YASM_UINT8_SZ_BYTES) + (1*COMPV_YASM_XMM_SZ_BYTES)]
			movdqu xmm2, [strelInputPtrsPtrv + (k * COMPV_YASM_UINT8_SZ_BYTES) + (2*COMPV_YASM_XMM_SZ_BYTES)]
			movdqu xmm3, [strelInputPtrsPtrv + (k * COMPV_YASM_UINT8_SZ_BYTES) + (3*COMPV_YASM_XMM_SZ_BYTES)]
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (v = 1; v < strelInputPtrsCount; ++v)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			mov v, 1
			.LoopStrel64:
				mov strelInputPtrsPtrv, [strelInputPtrsPtr + (v*COMPV_YASM_REG_SZ_BYTES)] ; strelInputPtrsPtrv = (strelInputPtrsPtr[v])
				inc v
				lea strelInputPtrsPtrv, [strelInputPtrsPtrv + (k * COMPV_YASM_UINT8_SZ_BYTES)] ; strelInputPtrsPtrv += k
				cmp v, strelInputPtrsCount
				movdqu xmm4, [strelInputPtrsPtrv + (0*COMPV_YASM_XMM_SZ_BYTES)]
				movdqu xmm5, [strelInputPtrsPtrv + (1*COMPV_YASM_XMM_SZ_BYTES)]
				movdqu xmm6, [strelInputPtrsPtrv + (2*COMPV_YASM_XMM_SZ_BYTES)]
				movdqu xmm7, [strelInputPtrsPtrv + (3*COMPV_YASM_XMM_SZ_BYTES)]
				%if CompVMathMorphProcessOp == CompVMathMorphProcessOpErode
					pminub xmm0, xmm4
					pminub xmm1, xmm5
					pminub xmm2, xmm6
					pminub xmm3, xmm7
				%else
					pmaxub xmm0, xmm4
					pmaxub xmm1, xmm5
					pmaxub xmm2, xmm6
					pmaxub xmm3, xmm7
				%endif
				jl .LoopStrel64
			.EndOf_LoopStrel64:

			lea v, [outPtr + (i * COMPV_YASM_UINT8_SZ_BYTES)] ; v = &outPtr[i]
			lea i, [i + 64]
			lea k, [k + 64]
			cmp i, width64
			movdqu [v + (0*COMPV_YASM_XMM_SZ_BYTES)], xmm0
			movdqu [v + (1*COMPV_YASM_XMM_SZ_BYTES)], xmm1
			movdqu [v + (2*COMPV_YASM_XMM_SZ_BYTES)], xmm2
			movdqu [v + (3*COMPV_YASM_XMM_SZ_BYTES)], xmm3			
			jl .LoopWidth64
		.EndOf_LoopWidth64:

		cmp i, width
		jge .EndOf_LoopWidth16

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width; i += 16, k += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopWidth16:
			mov strelInputPtrsPtrv, [strelInputPtrsPtr + (0*COMPV_YASM_REG_SZ_BYTES)] ; strelInputPtrsPtrv = (strelInputPtrsPtr[v=0])
			movdqu xmm0, [strelInputPtrsPtrv + (k * COMPV_YASM_UINT8_SZ_BYTES)]
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (v = 1; v < strelInputPtrsCount; ++v)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			mov v, 1
			.LoopStrel16:
				mov strelInputPtrsPtrv, [strelInputPtrsPtr + (v*COMPV_YASM_REG_SZ_BYTES)] ; strelInputPtrsPtrv = (strelInputPtrsPtr[v])
				inc v
				movdqu xmm4, [strelInputPtrsPtrv + (k * COMPV_YASM_UINT8_SZ_BYTES) + (0*COMPV_YASM_XMM_SZ_BYTES)]
				cmp v, strelInputPtrsCount
				%if CompVMathMorphProcessOp == CompVMathMorphProcessOpErode
					pminub xmm0, xmm4
				%else
					pmaxub xmm0, xmm4
				%endif
				jl .LoopStrel16
			.EndOf_LoopStrel16:
			
			cmp i, width16
			jge .MoreThanWidth16
				
			;; if (i < width16) ;;
			.LessThanWidth16:
				movdqu [(outPtr + (i * COMPV_YASM_UINT8_SZ_BYTES))], xmm0
				jmp .EndOf_MoreThanWidth16
			.EndOf_LessThanWidth16:

			;; if (i >= width16) ;;
			.MoreThanWidth16:
				movdqa [mem], xmm0
				;; for (v = 0; i < width; ++i, ++v) ;;
				xor v, v
				.LoopMoreThanWidth16:
					mov uint8, byte [mem + v*COMPV_YASM_UINT8_SZ_BYTES]
					inc v
					mov [outPtr + i*COMPV_YASM_UINT8_SZ_BYTES], byte uint8
					inc i
					cmp i, width
					jl .LoopMoreThanWidth16
				.EndOf_LoopMoreThanWidth16:
			.EndOf_MoreThanWidth16:

			lea i, [i + 16]
			cmp i, width
			lea k, [k + 16]
			jl .LoopWidth16
		.EndOf_LoopWidth16:
		
		dec height
		lea k, [k + strelInputPtrsPad]
		lea outPtr, [outPtr + stride*COMPV_YASM_UINT8_SZ_BYTES]
		jnz .LoopHeight
	.EndOf_LoopHeight:

	%undef mem
	%undef strelInputPtrsPtr		
	%undef strelInputPtrsCount		
	%undef outPtr					
	%undef width					
	%undef height					
	%undef	stride					
	%undef i						
	%undef width64					
	%undef width16					
	%undef strelInputPtrsPtrv		 
	%undef v						
	%undef uint8	
	%undef k
	%undef strelInputPtrsPad				

	; free memory and unalign stack
	add rsp, (16*COMPV_YASM_UINT8_SZ_BYTES)
	COMPV_YASM_UNALIGN_STACK

	;; begin epilog ;;
	pop r15
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
	%undef CompVMathMorphProcessOp
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVMathMorphProcessErode_8u_Asm_X64_SSE2):
	CompVMathMorphProcessOp_8u_Macro_X64_SSE2 CompVMathMorphProcessOpErode

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVMathMorphProcessDilate_8u_Asm_X64_SSE2):
	CompVMathMorphProcessOp_8u_Macro_X64_SSE2 CompVMathMorphProcessOpDilate

%undef CompVMathMorphProcessOpErode
%undef CompVMathMorphProcessOpDilate

%endif ; COMPV_YASM_ABI_IS_64BIT

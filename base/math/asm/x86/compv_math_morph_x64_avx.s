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

global sym(CompVMathMorphProcessErode_8u_Asm_X64_AVX2)
global sym(CompVMathMorphProcessDilate_8u_Asm_X64_AVX2)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const compv_uscalar_t* strelInputPtrsPtr
; arg(1) -> const compv_uscalar_t strelInputPtrsCount
; arg(2) -> uint8_t* outPtr
; arg(3) -> const compv_uscalar_t width
; arg(4) -> const compv_uscalar_t height
; arg(5) -> const compv_uscalar_t stride
%macro CompVMathMorphProcessOp_8u_Macro_X64_AVX2 1
	%define CompVMathMorphProcessOp %1
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	COMPV_YASM_SAVE_YMM 7
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	push r14
	push r15
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 32, rax
	sub rsp, (32*COMPV_YASM_UINT8_SZ_BYTES)

	%define mem						rsp + 0
	%define strelInputPtrsPtr		rbx
	%define strelInputPtrsCount		rcx
	%define outPtr					rdx
	%define width					rsi
	%define height					rdi
	%define	stride					rax
	%define i						r8
	%define width128				r9
	%define width32					r10
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
	mov width32, width
	mov width128, width
	lea strelInputPtrsPad, [width + 31]
	and width32, -32
	and width128, -128
	and strelInputPtrsPad, -32
	neg strelInputPtrsPad
	add strelInputPtrsPad, stride

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0, k = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	xor k, k
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width128; i += 128, k += 128)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		test width128, width128
		jz .EndOf_LoopWidth128
		.LoopWidth128:
			mov strelInputPtrsPtrv, [strelInputPtrsPtr + (0*COMPV_YASM_REG_SZ_BYTES)] ; strelInputPtrsPtrv = (strelInputPtrsPtr[v=0])
			vmovdqu ymm0, [strelInputPtrsPtrv + (k * COMPV_YASM_UINT8_SZ_BYTES) + (0*COMPV_YASM_YMM_SZ_BYTES)]
			vmovdqu ymm1, [strelInputPtrsPtrv + (k * COMPV_YASM_UINT8_SZ_BYTES) + (1*COMPV_YASM_YMM_SZ_BYTES)]
			vmovdqu ymm2, [strelInputPtrsPtrv + (k * COMPV_YASM_UINT8_SZ_BYTES) + (2*COMPV_YASM_YMM_SZ_BYTES)]
			vmovdqu ymm3, [strelInputPtrsPtrv + (k * COMPV_YASM_UINT8_SZ_BYTES) + (3*COMPV_YASM_YMM_SZ_BYTES)]
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (v = 1; v < strelInputPtrsCount; ++v)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			mov v, 1
			.LoopStrel128:
				mov strelInputPtrsPtrv, [strelInputPtrsPtr + (v*COMPV_YASM_REG_SZ_BYTES)] ; strelInputPtrsPtrv = (strelInputPtrsPtr[v])
				inc v
				lea strelInputPtrsPtrv, [strelInputPtrsPtrv + (k * COMPV_YASM_UINT8_SZ_BYTES)] ; strelInputPtrsPtrv += k
				cmp v, strelInputPtrsCount
				%if CompVMathMorphProcessOp == CompVMathMorphProcessOpErode
					vpminub ymm0, ymm0, [strelInputPtrsPtrv + (0*COMPV_YASM_YMM_SZ_BYTES)]
					vpminub ymm1, ymm1, [strelInputPtrsPtrv + (1*COMPV_YASM_YMM_SZ_BYTES)]
					vpminub ymm2, ymm2, [strelInputPtrsPtrv + (2*COMPV_YASM_YMM_SZ_BYTES)]
					vpminub ymm3, ymm3, [strelInputPtrsPtrv + (3*COMPV_YASM_YMM_SZ_BYTES)]
				%else
					vpmaxub ymm0, ymm0, [strelInputPtrsPtrv + (0*COMPV_YASM_YMM_SZ_BYTES)]
					vpmaxub ymm1, ymm1, [strelInputPtrsPtrv + (1*COMPV_YASM_YMM_SZ_BYTES)]
					vpmaxub ymm2, ymm2, [strelInputPtrsPtrv + (2*COMPV_YASM_YMM_SZ_BYTES)]
					vpmaxub ymm3, ymm3, [strelInputPtrsPtrv + (3*COMPV_YASM_YMM_SZ_BYTES)]
				%endif
				jl .LoopStrel128
			.EndOf_LoopStrel128:

			lea v, [outPtr + (i * COMPV_YASM_UINT8_SZ_BYTES)] ; v = &outPtr[i]
			lea i, [i + 128]
			lea k, [k + 128]
			cmp i, width128
			vmovdqu [v + (0*COMPV_YASM_YMM_SZ_BYTES)], ymm0
			vmovdqu [v + (1*COMPV_YASM_YMM_SZ_BYTES)], ymm1
			vmovdqu [v + (2*COMPV_YASM_YMM_SZ_BYTES)], ymm2
			vmovdqu [v + (3*COMPV_YASM_YMM_SZ_BYTES)], ymm3			
			jl .LoopWidth128
		.EndOf_LoopWidth128:

		cmp i, width
		jge .EndOf_LoopWidth32

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width; i += 32, k += 32)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopWidth32:
			mov strelInputPtrsPtrv, [strelInputPtrsPtr + (0*COMPV_YASM_REG_SZ_BYTES)] ; strelInputPtrsPtrv = (strelInputPtrsPtr[v=0])
			vmovdqu ymm0, [strelInputPtrsPtrv + (k * COMPV_YASM_UINT8_SZ_BYTES)]
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (v = 1; v < strelInputPtrsCount; ++v)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			mov v, 1
			.LoopStrel32:
				mov strelInputPtrsPtrv, [strelInputPtrsPtr + (v*COMPV_YASM_REG_SZ_BYTES)] ; strelInputPtrsPtrv = (strelInputPtrsPtr[v])
				inc v
				vmovdqu ymm4, [strelInputPtrsPtrv + (k * COMPV_YASM_UINT8_SZ_BYTES) + (0*COMPV_YASM_YMM_SZ_BYTES)]
				cmp v, strelInputPtrsCount
				%if CompVMathMorphProcessOp == CompVMathMorphProcessOpErode
					vpminub ymm0, ymm0, ymm4
				%else
					vpmaxub ymm0, ymm0, ymm4
				%endif
				jl .LoopStrel32
			.EndOf_LoopStrel32:
			
			cmp i, width32
			jge .MoreThanWidth32
				
			;; if (i < width32) ;;
			.LessThanWidth32:
				vmovdqu [(outPtr + (i * COMPV_YASM_UINT8_SZ_BYTES))], ymm0
				jmp .EndOf_MoreThanWidth32
			.EndOf_LessThanWidth32:

			;; if (i >= width32) ;;
			.MoreThanWidth32:
				vmovdqa [mem], ymm0
				;; for (v = 0; i < width; ++i, ++v) ;;
				xor v, v
				.LoopMoreThanWidth32:
					mov uint8, byte [mem + v*COMPV_YASM_UINT8_SZ_BYTES]
					inc v
					mov [outPtr + i*COMPV_YASM_UINT8_SZ_BYTES], byte uint8
					inc i
					cmp i, width
					jl .LoopMoreThanWidth32
				.EndOf_LoopMoreThanWidth32:
			.EndOf_MoreThanWidth32:

			lea i, [i + 32]
			cmp i, width
			lea k, [k + 32]
			jl .LoopWidth32
		.EndOf_LoopWidth32:
		
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
	%undef width128					
	%undef width32					
	%undef strelInputPtrsPtrv		 
	%undef v						
	%undef uint8	
	%undef k
	%undef strelInputPtrsPad				

	; free memory and unalign stack
	add rsp, (32*COMPV_YASM_UINT8_SZ_BYTES)
	COMPV_YASM_UNALIGN_STACK

	;; begin epilog ;;
	pop r15
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
	%undef CompVMathMorphProcessOp
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVMathMorphProcessErode_8u_Asm_X64_AVX2):
	CompVMathMorphProcessOp_8u_Macro_X64_AVX2 CompVMathMorphProcessOpErode

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVMathMorphProcessDilate_8u_Asm_X64_AVX2):
	CompVMathMorphProcessOp_8u_Macro_X64_AVX2 CompVMathMorphProcessOpDilate

%undef CompVMathMorphProcessOpErode
%undef CompVMathMorphProcessOpDilate

%endif ; COMPV_YASM_ABI_IS_64BIT

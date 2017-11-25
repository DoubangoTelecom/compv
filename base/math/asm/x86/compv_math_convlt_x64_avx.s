;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_X64_AVX2)
global sym(CompVMathConvlt1VtHz_8u32f8u_Asm_X64_AVX2)
global sym(CompVMathConvlt1VtHz_8u32f8u_Asm_X64_FMA3_AVX2)
global sym(CompVMathConvlt1VtHz_8u16s16s_Asm_X64_AVX2)
global sym(CompVMathConvlt1VtHz_16s16s16s_Asm_X64_AVX2)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* inPtr
; arg(1) -> uint8_t* outPtr
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> compv_uscalar_t step
; arg(5) -> compv_uscalar_t pad
; arg(6) -> const uint16_t* vthzKernPtr
; arg(7) -> compv_uscalar_t kernSize
sym(CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_X64_AVX2):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
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

	%define vecZero				ymm0
	%define vecSum0				ymm1
	%define vecSum1				ymm2
	%define vecCoeff			ymm3
	%define vec0				ymm4
	%define vec1				ymm5

	vpxor vecZero, vecZero

	%define argi_inPtr			0
	%define argi_outPtr			1
	%define argi_width			2
	%define argi_height			3
	%define argi_step			4 
	%define argi_pad			5
	%define argi_vthzKernPtr	6
	%define argi_kernSize		7	

	%define mem				rsp + 0
	%define width			rbx
	%define step			rcx
	%define row				rdx
	%define j				rsi
	%define i				rdi
	%define	k				rax
	%define inPtr			r8
	%define outPtr			r9
	%define vthzKernPtr		r10
	%define stride			r11
	%define width32			r12
	%define inPtrPlusI		r13
	%define octet			r14b
	%define kernSize		r15
	mov width, arg(argi_width)
	mov step, arg(argi_step)
	mov j, arg(argi_height)
	mov inPtr, arg(argi_inPtr)
	mov outPtr, arg(argi_outPtr)
	mov vthzKernPtr, arg(argi_vthzKernPtr)
	mov stride, arg(argi_pad)
	lea stride, [width + stride] ; stride = width + pad
	mov width32, width
	and width32, -32
	mov kernSize, arg(argi_kernSize)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width; i += 32)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		.LoopWidth:
			vpxor vecSum0, vecSum0
			vpxor vecSum1, vecSum1
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea inPtrPlusI, [inPtr + i*COMPV_YASM_UINT8_SZ_BYTES]
			xor row, row
			.LoopKernel:
				vmovdqu vec1, [inPtrPlusI]
				vpbroadcastw vecCoeff, word [vthzKernPtr + row*COMPV_YASM_UINT16_SZ_BYTES]
				add inPtrPlusI, step
				inc row
				vpunpcklbw vec0, vec1, vecZero
				vpmulhuw vec0, vec0, vecCoeff
				vpunpckhbw vec1, vec1, vecZero
				vpmulhuw vec1, vec1, vecCoeff
				cmp row, kernSize
				vpaddusw vecSum0, vecSum0, vec0
				vpaddusw vecSum1, vecSum1, vec1	
				jl .LoopKernel
			.EndOf_LoopKernel:
			
			cmp i, width32
			vpackuswb vecSum0, vecSum0, vecSum1
			jge .MoreThanWidth32
				
			;; if (i < width32) ;;
			.LessThanWidth32:
				vmovdqu [outPtr + i*COMPV_YASM_UINT8_SZ_BYTES], vecSum0
				jmp .EndOf_MoreThanWidth32
			.EndOf_LessThanWidth32:

			;; if (i >= width32) ;;
			.MoreThanWidth32:
				vmovdqa [mem], vecSum0
				;; for (k = 0; i < width; ++i, ++k) ;;
				xor k, k
				.LoopMoreThanWidth32:
					mov octet, byte [mem + k*COMPV_YASM_UINT8_SZ_BYTES]
					inc k
					mov [outPtr + i*COMPV_YASM_UINT8_SZ_BYTES], byte octet
					inc i
					cmp i, width
					jl .LoopMoreThanWidth32
				.EndOf_LoopMoreThanWidth32:
			.EndOf_MoreThanWidth32:
			
			add i, 32
			cmp i, width
			jl .LoopWidth
		.EndOf_LoopWidth:
		
		dec j
		lea inPtr, [inPtr + stride]
		lea outPtr, [outPtr + stride]
		jnz .LoopHeight
	.EndOf_LoopHeight:

	%undef vecZero
	%undef vecSum0
	%undef vecSum1
	%undef vecCoeff
	%undef vec0
	%undef vec1

	%undef argi_inPtr
	%undef argi_outPtr
	%undef argi_width
	%undef argi_height
	%undef argi_step
	%undef argi_pad
	%undef argi_vthzKernPtr
	%undef argi_kernSize

	%undef mem
	%undef width
	%undef step
	%undef row
	%undef j
	%undef i
	%undef k
	%undef inPtr
	%undef outPtr
	%undef vthzKernPtr
	%undef width32
	%undef inPtrPlusI
	%undef octet
	%undef kernSize

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
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* inPtr
; arg(1) -> uint8_t* outPtr
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> compv_uscalar_t step
; arg(5) -> compv_uscalar_t pad
; arg(6) -> const compv_float32_t* vthzKernPtr
; arg(7) -> compv_uscalar_t kernSize
; %1 -> 1: FMA3 enabled, 0: FMA3 disabled
%macro CompVMathConvlt1VtHz_8u32f8u_Macro_X64_AVX2 1
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_YMM 9
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

	%define vecZero				ymm0
	%define vecSum0				ymm1
	%define vecSum1				ymm2
	%define vecSum2				ymm3
	%define vecSum3				ymm4
	%define vecCoeff			ymm5
	%define vec0				ymm6
	%define vec1				ymm7
	%define vec2				ymm8
	%define vec3				ymm9

	vpxor vecZero, vecZero

	%define argi_inPtr			0
	%define argi_outPtr			1
	%define argi_width			2
	%define argi_height			3
	%define argi_step			4 
	%define argi_pad			5
	%define argi_vthzKernPtr	6
	%define argi_kernSize		7	

	%define mem				rsp + 0
	%define width			rbx
	%define step			rcx
	%define row				rdx
	%define j				rsi
	%define i				rdi
	%define	k				rax
	%define inPtr			r8
	%define outPtr			r9
	%define vthzKernPtr		r10
	%define stride			r11
	%define width32			r12
	%define inPtrPlusI		r13
	%define octet			r14b
	%define kernSize		r15
	mov width, arg(argi_width)
	mov step, arg(argi_step)
	mov j, arg(argi_height)
	mov inPtr, arg(argi_inPtr)
	mov outPtr, arg(argi_outPtr)
	mov vthzKernPtr, arg(argi_vthzKernPtr)
	mov stride, arg(argi_pad)
	lea stride, [width + stride] ; stride = width + pad
	mov width32, width
	and width32, -32
	mov kernSize, arg(argi_kernSize)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width; i += 32)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		.LoopWidth:
			vxorps vecSum0, vecSum0
			vxorps vecSum1, vecSum1
			vxorps vecSum2, vecSum2
			vxorps vecSum3, vecSum3
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea inPtrPlusI, [inPtr + i*COMPV_YASM_UINT8_SZ_BYTES]
			xor row, row
			.LoopKernel:
				vmovdqu vec3, [inPtrPlusI]
				vbroadcastss vecCoeff, dword ptr [vthzKernPtr + row*COMPV_YASM_FLOAT32_SZ_BYTES]
				vpunpcklbw vec1, vec3, vecZero
				vpunpckhbw vec3, vec3, vecZero
				vpunpcklwd vec0, vec1, vecZero
				vpunpckhwd vec1, vec1, vecZero
				vcvtdq2ps vec0, vec0
				vcvtdq2ps vec1, vec1
				vpunpcklwd vec2, vec3, vecZero
				vpunpckhwd vec3, vec3, vecZero
				vcvtdq2ps vec2, vec2
				vcvtdq2ps vec3, vec3
				vmulps vec0, vecCoeff
				vmulps vec1, vecCoeff
				vmulps vec2, vecCoeff
				vmulps vec3, vecCoeff
				add inPtrPlusI, step
				inc row
				cmp row, kernSize
				vaddps vecSum0, vec0
				vaddps vecSum1, vec1
				vaddps vecSum2, vec2
				vaddps vecSum3, vec3
				jl .LoopKernel
			.EndOf_LoopKernel:

			vcvttps2dq vecSum0, vecSum0
			vcvttps2dq vecSum1, vecSum1
			vcvttps2dq vecSum2, vecSum2
			vcvttps2dq vecSum3, vecSum3
			cmp i, width32
			vpackssdw vecSum0, vecSum1
			vpackssdw vecSum2, vecSum3
			vpackuswb vecSum0, vecSum2
			jge .MoreThanWidth32
				
			;; if (i < width32) ;;
			.LessThanWidth32:
				vmovdqu [outPtr + i*COMPV_YASM_UINT8_SZ_BYTES], vecSum0
				jmp .EndOf_MoreThanWidth32
			.EndOf_LessThanWidth32:

			;; if (i >= width32) ;;
			.MoreThanWidth32:
				vmovdqa [mem], vecSum0
				;; for (k = 0; i < width; ++i, ++k) ;;
				xor k, k
				.LoopMoreThanWidth32:
					mov octet, byte [mem + k*COMPV_YASM_UINT8_SZ_BYTES]
					inc k
					mov [outPtr + i*COMPV_YASM_UINT8_SZ_BYTES], byte octet
					inc i
					cmp i, width
					jl .LoopMoreThanWidth32
				.EndOf_LoopMoreThanWidth32:
			.EndOf_MoreThanWidth32:
			
			add i, 32
			cmp i, width
			jl .LoopWidth
		.EndOf_LoopWidth:
		
		dec j
		lea inPtr, [inPtr + stride]
		lea outPtr, [outPtr + stride]
		jnz .LoopHeight
	.EndOf_LoopHeight:

	%undef vecZero				
	%undef vecSum0				
	%undef vecSum1				
	%undef vecSum2				
	%undef vecSum3				
	%undef vecCoeff
	%undef vec0				
	%undef vec1				
	%undef vec2				
	%undef vec3				

	%undef argi_inPtr
	%undef argi_outPtr
	%undef argi_width
	%undef argi_height
	%undef argi_step
	%undef argi_pad
	%undef argi_vthzKernPtr
	%undef argi_kernSize

	%undef mem
	%undef width
	%undef step
	%undef row
	%undef j
	%undef i
	%undef k
	%undef inPtr
	%undef outPtr
	%undef vthzKernPtr
	%undef width32
	%undef inPtrPlusI
	%undef octet
	%undef kernSize

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
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVMathConvlt1VtHz_8u32f8u_Asm_X64_AVX2):
	CompVMathConvlt1VtHz_8u32f8u_Macro_X64_AVX2 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVMathConvlt1VtHz_8u32f8u_Asm_X64_FMA3_AVX2):
	CompVMathConvlt1VtHz_8u32f8u_Macro_X64_AVX2 1


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* inPtr
; arg(1) -> int16_t* outPtr
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> compv_uscalar_t step
; arg(5) -> compv_uscalar_t pad
; arg(6) -> const int16_t* vthzKernPtr
; arg(7) -> compv_uscalar_t kernSize
sym(CompVMathConvlt1VtHz_8u16s16s_Asm_X64_AVX2):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	push r14
	push r15
	;; end prolog ;;

	%define vecZero				ymm0
	%define vecZeron			xmm0
	%define vecSum0				ymm1
	%define vecSum0n			xmm1
	%define vecSum1				ymm2
	%define vecSum1n			xmm2
	%define vecCoeff			ymm3
	%define vecCoeffn			xmm3
	%define vec0				ymm4
	%define vec0n				xmm4
	%define vec1				ymm5
	%define vec1n				xmm5

	vpxor vecZero, vecZero, vecZero

	%define argi_inPtr			0
	%define argi_outPtr			1
	%define argi_width			2
	%define argi_height			3
	%define argi_step			4 
	%define argi_pad			5
	%define argi_vthzKernPtr	6
	%define argi_kernSize		7	

	%define width			rbx
	%define step			rcx
	%define j				rsi
	%define i				rdi
	%define inPtr			r8
	%define outPtr			r9
	%define stride			r10
	%define widthMinus31	r11
	%define widthMinus7		r12
	%define octet			r13
	%define kernSize		r14
	mov width, arg(argi_width)
	mov step, arg(argi_step)
	mov j, arg(argi_height)
	mov inPtr, arg(argi_inPtr)
	mov outPtr, arg(argi_outPtr)
	mov stride, arg(argi_pad)
	lea stride, [width + stride] ; stride = width + pad
	lea widthMinus31, [width - 31]
	lea widthMinus7, [width - 7]
	mov kernSize, arg(argi_kernSize)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width - 31; i += 32)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		.LoopWidth_Per32Samples:
			vpxor vecSum0, vecSum0
			vpxor vecSum1, vecSum1
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i] ; rax = &inPtr[i]
			mov r15, arg(argi_vthzKernPtr)
			mov rdx, kernSize
			.LoopKernelSize_Per32Samples:
				vmovdqu vec0, [rax]
				vmovd vecCoeffn, [r15]
				vpbroadcastw vecCoeff, vecCoeffn
				vpunpckhbw vec1, vec0, vecZero
				vpmullw vec1, vec1, vecCoeff
				vpunpcklbw vec0, vec0, vecZero
				dec rdx
				vpmullw vec0, vec0, vecCoeff
				lea rax, [rax + step]
				lea r15, [r15 + COMPV_YASM_UINT16_SZ_BYTES]				
				vpaddw vecSum1, vecSum1, vec1
				vpaddw vecSum0, vecSum0, vec0
				jnz .LoopKernelSize_Per32Samples
				; EndOf_LoopKernelSize_Per32Samples ;
			
			vperm2i128 vec0, vecSum0, vecSum1, 0x20
			vperm2i128 vec1, vecSum0, vecSum1, 0x31
			vmovdqu [outPtr + (i + 0)*COMPV_YASM_INT16_SZ_BYTES], vec0
			vmovdqu [outPtr + (i + 16)*COMPV_YASM_INT16_SZ_BYTES], vec1
			lea i, [i + 32]
			cmp i, widthMinus31
			jl .LoopWidth_Per32Samples
			; EndOf_LoopWidth_Per32Samples ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; i < width - 15
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea rax, [width - 15]
		cmp i, rax
		jge .EndOf_If_Per16Samples
		.If_Per16Samples:
			vpxor vecSum0n, vecSum0n, vecSum0n
			vpxor vecSum1n, vecSum1n, vecSum1n
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i] ; rax = &inPtr[i]
			mov r15, arg(argi_vthzKernPtr)
			mov rdx, kernSize
			.LoopKernelSize_Per16Samples:
				vmovdqu vec0n, [rax]
				vmovd vecCoeffn, [r15]
				vpunpcklwd vecCoeffn, vecCoeffn
				vpshufd vecCoeffn, vecCoeffn, 0
				vpunpckhbw vec1n, vec0n, vecZeron
				vpmullw vec1n, vec1n, vecCoeffn
				vpunpcklbw vec0n, vec0n, vecZeron
				dec rdx
				vpmullw vec0n, vec0n, vecCoeffn
				lea rax, [rax + step]
				lea r15, [r15 + COMPV_YASM_UINT16_SZ_BYTES]
				vpaddw vecSum1n, vecSum1n, vec1n
				vpaddw vecSum0n, vecSum0n, vec0n
				jnz .LoopKernelSize_Per16Samples
				; EndOf_LoopKernelSize_Per16Samples ;
			
			vmovdqu [outPtr + (i + 0)*COMPV_YASM_INT16_SZ_BYTES], vecSum0n
			vmovdqu [outPtr + (i + 8)*COMPV_YASM_INT16_SZ_BYTES], vecSum1n
			lea i, [i + 16]
			.EndOf_If_Per16Samples
			; EndOf_If_Per16Samples ;


		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; i < width - 7
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, widthMinus7
		jge .EndOf_If_Per8Samples
		.If_Per8Samples:
			vpxor vecSum0n, vecSum0n, vecSum0n
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i] ; rax = &inPtr[i]
			mov r15, arg(argi_vthzKernPtr)
			mov rdx, kernSize
			.LoopKernelSize_Per8Samples:
				vmovq vec0n, [rax]
				vmovd vecCoeffn, [r15]
				vpunpcklwd vecCoeffn, vecCoeffn
				vpshufd vecCoeffn, vecCoeffn, 0
				vpunpcklbw vec0n, vec0n, vecZeron
				dec rdx
				vpmullw vec0n, vec0n, vecCoeffn
				lea rax, [rax + step]
				lea r15, [r15 + COMPV_YASM_UINT16_SZ_BYTES]
				vpaddw vecSum0n, vecSum0n, vec0n
				jnz .LoopKernelSize_Per8Samples
				; EndOf_LoopKernelSize_Per8Samples ;
			
			vmovdqu [outPtr + i*COMPV_YASM_INT16_SZ_BYTES], vecSum0n
			lea i, [i + 8]
			.EndOf_If_Per8Samples
			; EndOf_If_Per8Samples ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; i < width - 3
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea rax, [width - 3]
		cmp i, rax
		jge .EndOf_If_Per4Samples
		.If_Per4Samples:
			vpxor vecSum0n, vecSum0n, vecSum0n
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i] ; rax = &inPtr[i]
			mov r15, arg(argi_vthzKernPtr)
			mov rdx, kernSize
			.LoopKernelSize_Per4Samples:
				vmovd vec0n, [rax]
				vmovd vecCoeffn, [r15]
				vpunpcklwd vecCoeffn, vecCoeffn
				vpshufd vecCoeffn, vecCoeffn, 0
				vpunpcklbw vec0n, vec0n, vecZeron
				dec rdx
				vpmullw vec0n, vec0n, vecCoeffn
				lea rax, [rax + step]
				lea r15, [r15 + COMPV_YASM_UINT16_SZ_BYTES]
				vpaddw vecSum0n, vecSum0n, vec0n
				jnz .LoopKernelSize_Per4Samples
				; EndOf_LoopKernelSize_Per4Samples ;
			
			vmovq [outPtr + i*COMPV_YASM_INT16_SZ_BYTES], vecSum0n
			lea i, [i + 4]
			.EndOf_If_Per4Samples
			; EndOf_If_Per4Samples ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; i < width
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_If_Per1Samples
		.If_Per1Samples:
			vpxor vecSum0n, vecSum0n, vecSum0n
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i] ; rax = &inPtr[i]
			mov r15, arg(argi_vthzKernPtr)
			mov rdx, kernSize
			.LoopKernelSize_Per1Samples:
				vmovd vec0n, [rax]
				vmovd vecCoeffn, [r15]
				vpunpcklwd vecCoeffn, vecCoeffn
				vpshufd vecCoeffn, vecCoeffn, 0
				vpunpcklbw vec0n, vec0n, vecZeron
				dec rdx
				vpmullw vec0n, vec0n, vecCoeffn
				lea rax, [rax + step]
				lea r15, [r15 + COMPV_YASM_UINT16_SZ_BYTES]
				vpaddw vecSum0n, vecSum0n, vec0n
				jnz .LoopKernelSize_Per1Samples
				; EndOf_LoopKernelSize_Per1Samples ;

			
			vmovq rax, vecSum0n
			%assign index 0
			%rep 4
				mov [outPtr + i*COMPV_YASM_INT16_SZ_BYTES], word ax
				inc i
				cmp i, width
				jge .EndOf_If_Per1Samples
				shr rax, 16
				%assign index index+1
			%endrep
			.EndOf_If_Per1Samples
			; EndOf_If_Per1Samples ;
		
		dec j
		lea inPtr, [inPtr + stride*COMPV_YASM_UINT8_SZ_BYTES]
		lea outPtr, [outPtr + stride*COMPV_YASM_INT16_SZ_BYTES]
		jnz .LoopHeight
		; EndOf_LoopHeight ;

	%undef vecZero				
	%undef vecZeron			
	%undef vecSum0				
	%undef vecSum0n			
	%undef vecSum1				
	%undef vecSum1n			
	%undef vecCoeff			
	%undef vecCoeffn			
	%undef vec0				
	%undef vec0n				
	%undef vec1				
	%undef vec1n				

	%undef argi_inPtr
	%undef argi_outPtr
	%undef argi_width
	%undef argi_height
	%undef argi_step
	%undef argi_pad
	%undef argi_vthzKernPtr
	%undef argi_kernSize

	%undef width
	%undef step
	%undef j
	%undef i
	%undef inPtr
	%undef outPtr
	%undef widthMinus31
	%undef widthMinus7
	%undef octet
	%undef kernSize

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


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const int16_t* inPtr
; arg(1) -> int16_t* outPtr
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> compv_uscalar_t step
; arg(5) -> compv_uscalar_t pad
; arg(6) -> const int16_t* vthzKernPtr
; arg(7) -> compv_uscalar_t kernSize
sym(CompVMathConvlt1VtHz_16s16s16s_Asm_X64_AVX2):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	push r14
	push r15
	;; end prolog ;;

	%define vecSum0				ymm0
	%define vecSum0n			xmm0
	%define vecSum1				ymm1
	%define vecCoeff			ymm2
	%define vecCoeffn			xmm2
	%define vec0				ymm3
	%define vec0n				xmm3
	%define vec1				ymm4

	%define argi_inPtr			0
	%define argi_outPtr			1
	%define argi_width			2
	%define argi_height			3
	%define argi_step			4 
	%define argi_pad			5
	%define argi_vthzKernPtr	6
	%define argi_kernSize		7	

	%define width			rbx
	%define step			rcx
	%define j				rsi
	%define i				rdi
	%define inPtr			r8
	%define outPtr			r9
	%define stride			r10
	%define widthMinus31	r11
	%define widthMinus7		r12
	%define octet			r13
	%define kernSize		r14
	mov width, arg(argi_width)
	mov step, arg(argi_step)
	lea step, [step*COMPV_YASM_INT16_SZ_BYTES] ; convert step from samples to bytes
	mov j, arg(argi_height)
	mov inPtr, arg(argi_inPtr)
	mov outPtr, arg(argi_outPtr)
	mov stride, arg(argi_pad)
	lea stride, [width + stride] ; stride = width + pad
	lea widthMinus31, [width - 31]
	lea widthMinus7, [width - 7]
	mov kernSize, arg(argi_kernSize)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width - 31; i += 32)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		.LoopWidth_Per32Samples:
			vpxor vecSum0, vecSum0
			vpxor vecSum1, vecSum1
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i*COMPV_YASM_INT16_SZ_BYTES] ; rax = &inPtr[i]
			mov r15, arg(argi_vthzKernPtr)
			mov rdx, kernSize
			.LoopKernelSize_Per32Samples:
				vmovdqu vec0, [rax]
				vmovdqu vec1, [rax + 16*COMPV_YASM_INT16_SZ_BYTES]
				vmovd vecCoeffn, [r15]
				vpbroadcastw vecCoeff, vecCoeffn
				vpmullw vec0, vec0, vecCoeff
				dec rdx
				vpmullw vec1, vec1, vecCoeff
				lea rax, [rax + step]
				lea r15, [r15 + COMPV_YASM_INT16_SZ_BYTES]
				vpaddw vecSum0, vecSum0, vec0			
				vpaddw vecSum1, vecSum1, vec1
				jnz .LoopKernelSize_Per32Samples
				; EndOf_LoopKernelSize_Per32Samples ;

			vmovdqu [outPtr + (i + 0)*COMPV_YASM_INT16_SZ_BYTES], vecSum0
			vmovdqu [outPtr + (i + 16)*COMPV_YASM_INT16_SZ_BYTES], vecSum1
			lea i, [i + 32]
			cmp i, widthMinus31
			jl .LoopWidth_Per32Samples
			; EndOf_LoopWidth_Per32Samples ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; i < width - 15
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea rax, [width - 15]
		cmp i, rax
		jge .EndOf_If_Per16Samples
		.If_Per16Samples:
			vpxor vecSum0, vecSum0, vecSum0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i*COMPV_YASM_INT16_SZ_BYTES] ; rax = &inPtr[i]
			mov r15, arg(argi_vthzKernPtr)
			mov rdx, kernSize
			.LoopKernelSize_Per16Samples:
				vmovdqu vec0, [rax]
				vmovd vecCoeffn, [r15]
				vpbroadcastw vecCoeff, vecCoeffn
				dec rdx
				vpmullw vec0, vec0, vecCoeff
				lea rax, [rax + step]
				lea r15, [r15 + COMPV_YASM_INT16_SZ_BYTES]
				vpaddw vecSum0, vecSum0, vec0
				jnz .LoopKernelSize_Per16Samples
				; EndOf_LoopKernelSize_Per16Samples ;
			
			vmovdqu [outPtr + (i + 0)*COMPV_YASM_INT16_SZ_BYTES], vecSum0
			lea i, [i + 16]
			.EndOf_If_Per16Samples
			; EndOf_If_Per16Samples ;


		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; i < width - 7
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, widthMinus7
		jge .EndOf_If_Per8Samples
		.If_Per8Samples:
			vpxor vecSum0n, vecSum0n, vecSum0n
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i*COMPV_YASM_INT16_SZ_BYTES] ; rax = &inPtr[i]
			mov r15, arg(argi_vthzKernPtr)
			mov rdx, kernSize
			.LoopKernelSize_Per8Samples:
				vmovdqu vec0n, [rax]
				vmovd vecCoeffn, [r15]
				vpunpcklwd vecCoeffn, vecCoeffn
				vpshufd vecCoeffn, vecCoeffn, 0
				dec rdx
				vpmullw vec0n, vec0n, vecCoeffn
				lea rax, [rax + step]
				lea r15, [r15 + COMPV_YASM_INT16_SZ_BYTES]
				vpaddw vecSum0n, vecSum0n, vec0n
				jnz .LoopKernelSize_Per8Samples
				; EndOf_LoopKernelSize_Per8Samples ;
			
			vmovdqu [outPtr + i*COMPV_YASM_INT16_SZ_BYTES], vecSum0n
			lea i, [i + 8]
			.EndOf_If_Per8Samples
			; EndOf_If_Per8Samples ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; i < width - 3
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea rax, [width - 3]
		cmp i, rax
		jge .EndOf_If_Per4Samples
		.If_Per4Samples:
			vpxor vecSum0n, vecSum0n, vecSum0n
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i*COMPV_YASM_INT16_SZ_BYTES] ; rax = &inPtr[i]
			mov r15, arg(argi_vthzKernPtr)
			mov rdx, kernSize
			.LoopKernelSize_Per4Samples:
				vmovq vec0n, [rax]
				vmovd vecCoeffn, [r15]
				vpunpcklwd vecCoeffn, vecCoeffn
				vpshufd vecCoeffn, vecCoeffn, 0
				dec rdx
				vpmullw vec0n, vec0n, vecCoeffn
				lea rax, [rax + step]
				lea r15, [r15 + COMPV_YASM_INT16_SZ_BYTES]
				vpaddw vecSum0n, vecSum0n, vec0n
				jnz .LoopKernelSize_Per4Samples
				; EndOf_LoopKernelSize_Per4Samples ;
			
			vmovq [outPtr + i*COMPV_YASM_INT16_SZ_BYTES], vecSum0n
			lea i, [i + 4]
			.EndOf_If_Per4Samples
			; EndOf_If_Per4Samples ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; i < width
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_If_Per1Samples
		.If_Per1Samples:
			vpxor vecSum0n, vecSum0n, vecSum0n
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i*COMPV_YASM_INT16_SZ_BYTES] ; rax = &inPtr[i]
			mov r15, arg(argi_vthzKernPtr)
			mov rdx, kernSize
			.LoopKernelSize_Per1Samples:
				vmovq vec0n, [rax]
				vmovd vecCoeffn, [r15]
				vpunpcklwd vecCoeffn, vecCoeffn
				vpshufd vecCoeffn, vecCoeffn, 0
				dec rdx
				vpmullw vec0n, vec0n, vecCoeffn
				lea rax, [rax + step]
				lea r15, [r15 + COMPV_YASM_INT16_SZ_BYTES]
				vpaddw vecSum0n, vecSum0n, vec0n
				jnz .LoopKernelSize_Per1Samples
				; EndOf_LoopKernelSize_Per1Samples ;

			
			vmovq rax, vecSum0n
			%assign index 0
			%rep 4
				mov [outPtr + i*COMPV_YASM_INT16_SZ_BYTES], word ax
				inc i
				cmp i, width
				jge .EndOf_If_Per1Samples
				shr rax, 16
				%assign index index+1
			%endrep
			.EndOf_If_Per1Samples
			; EndOf_If_Per1Samples ;
		
		dec j
		lea inPtr, [inPtr + stride*COMPV_YASM_INT16_SZ_BYTES]
		lea outPtr, [outPtr + stride*COMPV_YASM_INT16_SZ_BYTES]
		jnz .LoopHeight
		; EndOf_LoopHeight ;

	
	%undef vecSum0				
	%undef vecSum0n			
	%undef vecSum1				
	%undef vecCoeff			
	%undef vecCoeffn			
	%undef vec0				
	%undef vec0n				
	%undef vec1

	%undef argi_inPtr
	%undef argi_outPtr
	%undef argi_width
	%undef argi_height
	%undef argi_step
	%undef argi_pad
	%undef argi_vthzKernPtr
	%undef argi_kernSize

	%undef width
	%undef step
	%undef j
	%undef i
	%undef inPtr
	%undef outPtr
	%undef widthMinus31
	%undef widthMinus7
	%undef octet
	%undef kernSize

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

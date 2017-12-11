;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   ;
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
global sym(CompVMathConvlt1VtHz_8u32f32f_Asm_X64_AVX2)
global sym(CompVMathConvlt1VtHz_8u32f32f_Asm_X64_FMA3_AVX2)
global sym(CompVMathConvlt1VtHz_32f32f32f_Asm_X64_AVX2)
global sym(CompVMathConvlt1VtHz_32f32f32f_Asm_X64_FMA3_AVX2)
global sym(CompVMathConvlt1VtHz_32f32f8u_Asm_X64_AVX2)
global sym(CompVMathConvlt1VtHz_32f32f8u_Asm_X64_FMA3_AVX2)
global sym(CompVMathConvlt1VtHz_8u16s16s_Asm_X64_AVX2)
global sym(CompVMathConvlt1VtHz_16s16s16s_Asm_X64_AVX2)

section .data
	extern sym(kAVXPermutevar8x32_AEBFCGDH_32s)

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

	%define vecSum0				ymm0
	%define vecSum1				ymm1
	%define vecCoeff			ymm2
	%define vec0				ymm3
	%define vec1				ymm4

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
	%define uint8			r14b
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
				vpmovzxbw vec0, [inPtrPlusI + 0*COMPV_YASM_UINT8_SZ_BYTES]
				vpmovzxbw vec1, [inPtrPlusI + 16*COMPV_YASM_UINT8_SZ_BYTES]
				vpbroadcastw vecCoeff, word [vthzKernPtr + row*COMPV_YASM_UINT16_SZ_BYTES]
				inc row
				vpmulhuw vec0, vec0, vecCoeff
				vpmulhuw vec1, vec1, vecCoeff
				cmp row, kernSize
				lea inPtrPlusI, [inPtrPlusI + step*COMPV_YASM_UINT8_SZ_BYTES]
				vpaddusw vecSum0, vecSum0, vec0
				vpaddusw vecSum1, vecSum1, vec1	
				jl .LoopKernel
			.EndOf_LoopKernel:
			
			cmp i, width32
			vpackuswb vecSum0, vecSum0, vecSum1
			vpermq vecSum0, vecSum0, 0xD8
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
					mov uint8, byte [mem + k*COMPV_YASM_UINT8_SZ_BYTES]
					inc k
					mov [outPtr + i*COMPV_YASM_UINT8_SZ_BYTES], byte uint8
					inc i
					cmp i, width
					jl .LoopMoreThanWidth32
				.EndOf_LoopMoreThanWidth32:
				jmp .EndOf_LoopWidth
			.EndOf_MoreThanWidth32:
			
			add i, 32
			cmp i, width
			jl .LoopWidth
		.EndOf_LoopWidth:
		
		dec j
		lea inPtr, [inPtr + stride*COMPV_YASM_UINT8_SZ_BYTES]
		lea outPtr, [outPtr + stride*COMPV_YASM_UINT8_SZ_BYTES]
		jnz .LoopHeight
	.EndOf_LoopHeight:

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
	%undef uint8
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

	%define vecAEBFCGDH			ymm0
	%define vecSum0				ymm1
	%define vecSum1				ymm2
	%define vecSum2				ymm3
	%define vecSum3				ymm4
	%define vecCoeff			ymm5
	%define vec0				ymm6
	%define vec1				ymm7
	%define vec2				ymm8
	%define vec3				ymm9

	vmovdqa vecAEBFCGDH, [sym(kAVXPermutevar8x32_AEBFCGDH_32s)]

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
	%define uint8			r14b
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
				vpmovzxbd vec0, [inPtrPlusI + 0*COMPV_YASM_UINT8_SZ_BYTES]
				vpmovzxbd vec1, [inPtrPlusI + 8*COMPV_YASM_UINT8_SZ_BYTES]
				vpmovzxbd vec2, [inPtrPlusI + 16*COMPV_YASM_UINT8_SZ_BYTES]
				vpmovzxbd vec3, [inPtrPlusI + 24*COMPV_YASM_UINT8_SZ_BYTES]
				vbroadcastss vecCoeff, dword ptr [vthzKernPtr + row*COMPV_YASM_FLOAT32_SZ_BYTES]				
				vcvtdq2ps vec0, vec0
				vcvtdq2ps vec1, vec1
				vcvtdq2ps vec2, vec2
				vcvtdq2ps vec3, vec3
				%if %1 == 1
					lea inPtrPlusI, [inPtrPlusI + step*COMPV_YASM_UINT8_SZ_BYTES]
					inc row
					cmp row, kernSize
					vfmadd231ps vecSum0, vec0, vecCoeff
					vfmadd231ps vecSum1, vec1, vecCoeff
					vfmadd231ps vecSum2, vec2, vecCoeff
					vfmadd231ps vecSum3, vec3, vecCoeff
				%else
					vmulps vec0, vecCoeff
					vmulps vec1, vecCoeff
					vmulps vec2, vecCoeff
					vmulps vec3, vecCoeff
					lea inPtrPlusI, [inPtrPlusI + step*COMPV_YASM_UINT8_SZ_BYTES]
					inc row
					cmp row, kernSize
					vaddps vecSum0, vec0
					vaddps vecSum1, vec1
					vaddps vecSum2, vec2
					vaddps vecSum3, vec3
				%endif
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
			vpermd vecSum0, vecAEBFCGDH, vecSum0
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
					mov uint8, byte [mem + k*COMPV_YASM_UINT8_SZ_BYTES]
					inc k
					mov [outPtr + i*COMPV_YASM_UINT8_SZ_BYTES], byte uint8
					inc i
					cmp i, width
					jl .LoopMoreThanWidth32
				.EndOf_LoopMoreThanWidth32:
				jmp .EndOf_LoopWidth
			.EndOf_MoreThanWidth32:
			
			add i, 32
			cmp i, width
			jl .LoopWidth
		.EndOf_LoopWidth:
		
		dec j
		lea inPtr, [inPtr + stride*COMPV_YASM_UINT8_SZ_BYTES]
		lea outPtr, [outPtr + stride*COMPV_YASM_UINT8_SZ_BYTES]
		jnz .LoopHeight
	.EndOf_LoopHeight:

	%undef vecAEBFCGDH				
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
	%undef uint8
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
; arg(1) -> compv_float32_t* outPtr
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> compv_uscalar_t step
; arg(5) -> compv_uscalar_t pad
; arg(6) -> const compv_float32_t* vthzKernPtr
; arg(7) -> compv_uscalar_t kernSize
; %1 -> 1: FMA3 enabled, 0: FMA3 disabled
%macro CompVMathConvlt1VtHz_8u32f32f_Macro_X64_AVX2 1
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_YMM 8
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
	sub rsp, (32*COMPV_YASM_FLOAT32_SZ_BYTES)

	%define vecSum0				ymm0
	%define vecSum1				ymm1
	%define vecSum2				ymm2
	%define vecSum3				ymm3
	%define vecCoeff			ymm4
	%define vec0				ymm5
	%define vec1				ymm6
	%define vec2				ymm7
	%define vec3				ymm8

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
	%define float32			r14d
	%define kernSize		r15
	mov width, arg(argi_width)
	mov step, arg(argi_step)
	mov j, arg(argi_height)
	mov inPtr, arg(argi_inPtr)
	mov outPtr, arg(argi_outPtr)
	mov vthzKernPtr, arg(argi_vthzKernPtr)
	mov stride, arg(argi_pad)
	lea stride, [width + stride] ; stride = (width + pad)
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
				vpmovzxbd vec0, [inPtrPlusI + 0*COMPV_YASM_UINT8_SZ_BYTES]
				vpmovzxbd vec1, [inPtrPlusI + 8*COMPV_YASM_UINT8_SZ_BYTES]
				vpmovzxbd vec2, [inPtrPlusI + 16*COMPV_YASM_UINT8_SZ_BYTES]
				vpmovzxbd vec3, [inPtrPlusI + 24*COMPV_YASM_UINT8_SZ_BYTES]
				vbroadcastss vecCoeff, dword ptr [vthzKernPtr + row*COMPV_YASM_FLOAT32_SZ_BYTES]
				vcvtdq2ps vec0, vec0
				vcvtdq2ps vec1, vec1
				vcvtdq2ps vec2, vec2
				vcvtdq2ps vec3, vec3
				%if %1 == 1
					lea inPtrPlusI, [inPtrPlusI + step*COMPV_YASM_UINT8_SZ_BYTES]
					inc row
					cmp row, kernSize
					vfmadd231ps vecSum0, vecCoeff, vec0
					vfmadd231ps vecSum1, vecCoeff, vec1
					vfmadd231ps vecSum2, vecCoeff, vec2
					vfmadd231ps vecSum3, vecCoeff, vec3
				%else
					vmulps vec0, vecCoeff, vec0
					vmulps vec1, vecCoeff, vec1
					vmulps vec2, vecCoeff, vec2
					vmulps vec3, vecCoeff, vec3
					lea inPtrPlusI, [inPtrPlusI + step*COMPV_YASM_UINT8_SZ_BYTES]
					inc row
					cmp row, kernSize
					vaddps vecSum0, vecSum0, vec0
					vaddps vecSum1, vecSum1, vec1
					vaddps vecSum2, vecSum2, vec2
					vaddps vecSum3, vecSum3, vec3
				%endif
				jl .LoopKernel
			.EndOf_LoopKernel:

			cmp i, width32
			jge .MoreThanWidth32
				
			;; if (i < width32) ;;
			.LessThanWidth32:
				vmovups xmmword ptr [outPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum0
				vmovups xmmword ptr [outPtr + (i+8)*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum1
				vmovups xmmword ptr [outPtr + (i+16)*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum2
				vmovups xmmword ptr [outPtr + (i+24)*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum3
				jmp .EndOf_MoreThanWidth32
			.EndOf_LessThanWidth32:

			;; if (i >= width32) ;;
			.MoreThanWidth32:
				vmovaps xmmword ptr [mem + (0)*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum0
				vmovaps xmmword ptr [mem + (8)*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum1
				vmovaps xmmword ptr [mem + (16)*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum2
				vmovaps xmmword ptr [mem + (24)*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum3
				;; for (k = 0; i < width; ++i, ++k) ;;
				xor k, k
				.LoopMoreThanWidth32:
					mov float32, dword [mem + k*COMPV_YASM_FLOAT32_SZ_BYTES]
					inc k
					mov [outPtr + i*COMPV_YASM_FLOAT32_SZ_BYTES], dword float32
					inc i
					cmp i, width
					jl .LoopMoreThanWidth32
				.EndOf_LoopMoreThanWidth32:
				jmp .EndOf_LoopWidth
			.EndOf_MoreThanWidth32:
			
			add i, 32
			cmp i, width
			jl .LoopWidth
		.EndOf_LoopWidth:
		
		dec j
		lea inPtr, [inPtr + stride*COMPV_YASM_UINT8_SZ_BYTES]
		lea outPtr, [outPtr + stride*COMPV_YASM_FLOAT32_SZ_BYTES]
		jnz .LoopHeight
	.EndOf_LoopHeight:

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
	%undef float32
	%undef kernSize

	; free memory and unalign stack
	add rsp, (32*COMPV_YASM_FLOAT32_SZ_BYTES)
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
sym(CompVMathConvlt1VtHz_8u32f32f_Asm_X64_AVX2):
	CompVMathConvlt1VtHz_8u32f32f_Macro_X64_AVX2 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVMathConvlt1VtHz_8u32f32f_Asm_X64_FMA3_AVX2):
	CompVMathConvlt1VtHz_8u32f32f_Macro_X64_AVX2 1


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const compv_float32_t* inPtr
; arg(1) -> compv_float32_t* outPtr
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> compv_uscalar_t step
; arg(5) -> compv_uscalar_t pad
; arg(6) -> const compv_float32_t* vthzKernPtr
; arg(7) -> compv_uscalar_t kernSize
; %1 -> 1: FMA3 enabled, 0: FMA3 disabled
%macro CompVMathConvlt1VtHz_32f32f32f_Macro_X64_AVX2 1
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_YMM 8
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
	sub rsp, (32*COMPV_YASM_FLOAT32_SZ_BYTES)

	%define vecSum0				ymm0
	%define vecSum1				ymm1
	%define vecSum2				ymm2
	%define vecSum3				ymm3
	%define vecCoeff			ymm4
	%define vec0				ymm5
	%define vec1				ymm6
	%define vec2				ymm7
	%define vec3				ymm8

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
	%define float32			r14d
	%define kernSize		r15
	mov width, arg(argi_width)
	mov step, arg(argi_step)
	mov j, arg(argi_height)
	mov inPtr, arg(argi_inPtr)
	mov outPtr, arg(argi_outPtr)
	mov vthzKernPtr, arg(argi_vthzKernPtr)
	mov stride, arg(argi_pad)
	lea stride, [width + stride] ; stride = (width + pad)
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
			lea inPtrPlusI, [inPtr + i*COMPV_YASM_FLOAT32_SZ_BYTES]
			xor row, row
			.LoopKernel:
				vbroadcastss vecCoeff, dword ptr [vthzKernPtr + row*COMPV_YASM_FLOAT32_SZ_BYTES]
				%if %1 == 1
					inc row
					cmp row, kernSize
					vfmadd231ps vecSum0, vecCoeff, [inPtrPlusI + 0*COMPV_YASM_FLOAT32_SZ_BYTES]
					vfmadd231ps vecSum1, vecCoeff, [inPtrPlusI + 8*COMPV_YASM_FLOAT32_SZ_BYTES]
					vfmadd231ps vecSum2, vecCoeff, [inPtrPlusI + 16*COMPV_YASM_FLOAT32_SZ_BYTES]
					vfmadd231ps vecSum3, vecCoeff, [inPtrPlusI + 24*COMPV_YASM_FLOAT32_SZ_BYTES]
					lea inPtrPlusI, [inPtrPlusI + step*COMPV_YASM_FLOAT32_SZ_BYTES]
				%else
					vmulps vec0, vecCoeff, [inPtrPlusI + 0*COMPV_YASM_FLOAT32_SZ_BYTES]
					vmulps vec1, vecCoeff, [inPtrPlusI + 8*COMPV_YASM_FLOAT32_SZ_BYTES]
					vmulps vec2, vecCoeff, [inPtrPlusI + 16*COMPV_YASM_FLOAT32_SZ_BYTES]
					vmulps vec3, vecCoeff, [inPtrPlusI + 24*COMPV_YASM_FLOAT32_SZ_BYTES]
					lea inPtrPlusI, [inPtrPlusI + step*COMPV_YASM_FLOAT32_SZ_BYTES]
					inc row
					cmp row, kernSize
					vaddps vecSum0, vecSum0, vec0
					vaddps vecSum1, vecSum1, vec1
					vaddps vecSum2, vecSum2, vec2
					vaddps vecSum3, vecSum3, vec3
				%endif
				jl .LoopKernel
			.EndOf_LoopKernel:

			cmp i, width32
			jge .MoreThanWidth32
				
			;; if (i < width32) ;;
			.LessThanWidth32:
				vmovups xmmword ptr [outPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum0
				vmovups xmmword ptr [outPtr + (i+8)*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum1
				vmovups xmmword ptr [outPtr + (i+16)*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum2
				vmovups xmmword ptr [outPtr + (i+24)*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum3
				jmp .EndOf_MoreThanWidth32
			.EndOf_LessThanWidth32:

			;; if (i >= width32) ;;
			.MoreThanWidth32:
				vmovaps xmmword ptr [mem + (0)*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum0
				vmovaps xmmword ptr [mem + (8)*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum1
				vmovaps xmmword ptr [mem + (16)*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum2
				vmovaps xmmword ptr [mem + (24)*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum3
				;; for (k = 0; i < width; ++i, ++k) ;;
				xor k, k
				.LoopMoreThanWidth32:
					mov float32, dword [mem + k*COMPV_YASM_FLOAT32_SZ_BYTES]
					inc k
					mov [outPtr + i*COMPV_YASM_FLOAT32_SZ_BYTES], dword float32
					inc i
					cmp i, width
					jl .LoopMoreThanWidth32
				.EndOf_LoopMoreThanWidth32:
				jmp .EndOf_LoopWidth
			.EndOf_MoreThanWidth32:
			
			add i, 32
			cmp i, width
			jl .LoopWidth
		.EndOf_LoopWidth:
		
		dec j
		lea inPtr, [inPtr + stride*COMPV_YASM_FLOAT32_SZ_BYTES]
		lea outPtr, [outPtr + stride*COMPV_YASM_FLOAT32_SZ_BYTES]
		jnz .LoopHeight
	.EndOf_LoopHeight:

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
	%undef float32
	%undef kernSize

	; free memory and unalign stack
	add rsp, (32*COMPV_YASM_FLOAT32_SZ_BYTES)
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
sym(CompVMathConvlt1VtHz_32f32f32f_Asm_X64_AVX2):
	CompVMathConvlt1VtHz_32f32f32f_Macro_X64_AVX2 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVMathConvlt1VtHz_32f32f32f_Asm_X64_FMA3_AVX2):
	CompVMathConvlt1VtHz_32f32f32f_Macro_X64_AVX2 1


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const compv_float32_t* inPtr
; arg(1) -> uint8_t* outPtr
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> compv_uscalar_t step
; arg(5) -> compv_uscalar_t pad
; arg(6) -> const compv_float32_t* vthzKernPtr
; arg(7) -> compv_uscalar_t kernSize
; %1 -> 1: FMA3 enabled, 0: FMA3 disabled
%macro CompVMathConvlt1VtHz_32f32f8u_Macro_X64_AVX2 1
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

	%define vecSum0				ymm0
	%define vecSum1				ymm1
	%define vecSum2				ymm2
	%define vecSum3				ymm3
	%define vecCoeff			ymm4
	%define vec0				ymm5
	%define vec1				ymm6
	%define vec2				ymm7
	%define vec3				ymm8
	%define vecAEBFCGDH			ymm9	

	vmovdqa vecAEBFCGDH, [sym(kAVXPermutevar8x32_AEBFCGDH_32s)]

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
	%define uint8			r14b
	%define kernSize		r15
	mov width, arg(argi_width)
	mov step, arg(argi_step)
	mov j, arg(argi_height)
	mov inPtr, arg(argi_inPtr)
	mov outPtr, arg(argi_outPtr)
	mov vthzKernPtr, arg(argi_vthzKernPtr)
	mov stride, arg(argi_pad)
	lea stride, [width + stride] ; stride = (width + pad)
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
			lea inPtrPlusI, [inPtr + i*COMPV_YASM_FLOAT32_SZ_BYTES]
			xor row, row
			.LoopKernel:
				vbroadcastss vecCoeff, dword ptr [vthzKernPtr + row*COMPV_YASM_FLOAT32_SZ_BYTES]
				%if %1 == 1
					inc row
					cmp row, kernSize
					vfmadd231ps vecSum0, vecCoeff, [inPtrPlusI + 0*COMPV_YASM_FLOAT32_SZ_BYTES]
					vfmadd231ps vecSum1, vecCoeff, [inPtrPlusI + 8*COMPV_YASM_FLOAT32_SZ_BYTES]
					vfmadd231ps vecSum2, vecCoeff, [inPtrPlusI + 16*COMPV_YASM_FLOAT32_SZ_BYTES]
					vfmadd231ps vecSum3, vecCoeff, [inPtrPlusI + 24*COMPV_YASM_FLOAT32_SZ_BYTES]
					lea inPtrPlusI, [inPtrPlusI + step*COMPV_YASM_FLOAT32_SZ_BYTES]
				%else
					vmulps vec0, vecCoeff, [inPtrPlusI + 0*COMPV_YASM_FLOAT32_SZ_BYTES]
					vmulps vec1, vecCoeff, [inPtrPlusI + 8*COMPV_YASM_FLOAT32_SZ_BYTES]
					vmulps vec2, vecCoeff, [inPtrPlusI + 16*COMPV_YASM_FLOAT32_SZ_BYTES]
					vmulps vec3, vecCoeff, [inPtrPlusI + 24*COMPV_YASM_FLOAT32_SZ_BYTES]
					lea inPtrPlusI, [inPtrPlusI + step*COMPV_YASM_FLOAT32_SZ_BYTES]
					inc row
					cmp row, kernSize
					vaddps vecSum0, vecSum0, vec0
					vaddps vecSum1, vecSum1, vec1
					vaddps vecSum2, vecSum2, vec2
					vaddps vecSum3, vecSum3, vec3
				%endif
				jl .LoopKernel
			.EndOf_LoopKernel:

			vcvttps2dq vecSum0, vecSum0
			vcvttps2dq vecSum1, vecSum1
			vcvttps2dq vecSum2, vecSum2
			vcvttps2dq vecSum3, vecSum3
			cmp i, width32
			vpackssdw vecSum0, vecSum0, vecSum1
			vpackssdw vecSum2, vecSum2, vecSum3
			vpackuswb vecSum0, vecSum0, vecSum2
			vpermd vecSum0, vecAEBFCGDH, vecSum0
			jge .MoreThanWidth32
				
			;; if (i < width32) ;;
			.LessThanWidth32:
				vmovdqu xmmword ptr [outPtr + (i+0)*COMPV_YASM_UINT8_SZ_BYTES], vecSum0
				jmp .EndOf_MoreThanWidth32
			.EndOf_LessThanWidth32:

			;; if (i >= width32) ;;
			.MoreThanWidth32:
				vmovdqa xmmword ptr [mem + (0)*COMPV_YASM_UINT8_SZ_BYTES], vecSum0
				;; for (k = 0; i < width; ++i, ++k) ;;
				xor k, k
				.LoopMoreThanWidth32:
					mov uint8, byte [mem + k*COMPV_YASM_UINT8_SZ_BYTES]
					inc k
					mov [outPtr + i*COMPV_YASM_UINT8_SZ_BYTES], byte uint8
					inc i
					cmp i, width
					jl .LoopMoreThanWidth32
				.EndOf_LoopMoreThanWidth32:
				jmp .EndOf_LoopWidth
			.EndOf_MoreThanWidth32:
			
			add i, 32
			cmp i, width
			jl .LoopWidth
		.EndOf_LoopWidth:
		
		dec j
		lea inPtr, [inPtr + stride*COMPV_YASM_FLOAT32_SZ_BYTES]
		lea outPtr, [outPtr + stride*COMPV_YASM_UINT8_SZ_BYTES]
		jnz .LoopHeight
	.EndOf_LoopHeight:

	%undef vecSum0				
	%undef vecSum1				
	%undef vecSum2				
	%undef vecSum3				
	%undef vecCoeff
	%undef vec0				
	%undef vec1				
	%undef vec2				
	%undef vec3
	%undef vecAEBFCGDH		

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
	%undef uint8
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
sym(CompVMathConvlt1VtHz_32f32f8u_Asm_X64_AVX2):
	CompVMathConvlt1VtHz_32f32f8u_Macro_X64_AVX2 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVMathConvlt1VtHz_32f32f8u_Asm_X64_FMA3_AVX2):
	CompVMathConvlt1VtHz_32f32f8u_Macro_X64_AVX2 1


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
	COMPV_YASM_SAVE_YMM 8
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
	sub rsp, (32*COMPV_YASM_INT16_SZ_BYTES)

	%define vecSum0				ymm0
	%define vecSum1				ymm1
	%define vecSum2				ymm2
	%define vecSum3				ymm3
	%define vecCoeff			ymm4
	%define vecCoeffn			xmm4
	%define vec0				ymm5
	%define vec1				ymm6
	%define vec2				ymm7
	%define vec3				ymm8

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
	%define int16w			r14w
	%define int16q			r14
	%define int16d			r14d
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
			vpxor vecSum0, vecSum0, vecSum0
			vpxor vecSum1, vecSum1, vecSum1
			vpxor vecSum2, vecSum2, vecSum2
			vpxor vecSum3, vecSum3, vecSum3
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea inPtrPlusI, [inPtr + i*COMPV_YASM_UINT8_SZ_BYTES]
			xor row, row
			.LoopKernel:
				movsx int16d, word [vthzKernPtr + row*COMPV_YASM_INT16_SZ_BYTES]
				vmovd vecCoeffn, int16d
				inc row
				vpbroadcastd vecCoeff, vecCoeffn
				vpmovzxbd vec0, [inPtrPlusI + 0*COMPV_YASM_UINT8_SZ_BYTES]
				vpmovzxbd vec1, [inPtrPlusI + 8*COMPV_YASM_UINT8_SZ_BYTES]
				vpmulld vec0, vec0, vecCoeff
				vpmulld vec1, vec1, vecCoeff
				vpmovzxbd vec2, [inPtrPlusI + 16*COMPV_YASM_UINT8_SZ_BYTES]
				vpmovzxbd vec3, [inPtrPlusI + 24*COMPV_YASM_UINT8_SZ_BYTES]
				vpmulld vec2, vec2, vecCoeff
				vpmulld vec3, vec3, vecCoeff
				lea inPtrPlusI, [inPtrPlusI + step*COMPV_YASM_UINT8_SZ_BYTES]		
				cmp row, kernSize
				vpaddd vecSum0, vecSum0, vec0
				vpaddd vecSum1, vecSum1, vec1
				vpaddd vecSum2, vecSum2, vec2
				vpaddd vecSum3, vecSum3, vec3
				jl .LoopKernel
			.EndOf_LoopKernel:
						
			vpackssdw vecSum0, vecSum1
			vpackssdw vecSum2, vecSum3
			cmp i, width32
			vpermq vecSum0, vecSum0, 0xD8
			vpermq vecSum2, vecSum2, 0xD8
			jge .MoreThanWidth32
				
			;; if (i < width32) ;;
			.LessThanWidth32:
				vmovdqu [outPtr + (i+0)*COMPV_YASM_INT16_SZ_BYTES], vecSum0
				vmovdqu [outPtr + (i+16)*COMPV_YASM_INT16_SZ_BYTES], vecSum2
				jmp .EndOf_MoreThanWidth32
			.EndOf_LessThanWidth32:

			;; if (i >= width32) ;;
			.MoreThanWidth32:
				vmovdqa [mem + (0)*COMPV_YASM_INT16_SZ_BYTES], vecSum0
				vmovdqa [mem + (16)*COMPV_YASM_INT16_SZ_BYTES], vecSum2
				;; for (k = 0; i < width; ++i, ++k) ;;
				xor k, k
				.LoopMoreThanWidth32:
					mov int16w, word [mem + k*COMPV_YASM_INT16_SZ_BYTES]
					inc k
					mov [outPtr + i*COMPV_YASM_INT16_SZ_BYTES], word int16w
					inc i
					cmp i, width
					jl .LoopMoreThanWidth32
				.EndOf_LoopMoreThanWidth32:
				jmp .EndOf_LoopWidth
			.EndOf_MoreThanWidth32:
			
			add i, 32
			cmp i, width
			jl .LoopWidth
		.EndOf_LoopWidth:
		
		dec j
		lea inPtr, [inPtr + stride*COMPV_YASM_UINT8_SZ_BYTES]
		lea outPtr, [outPtr + stride*COMPV_YASM_INT16_SZ_BYTES]
		jnz .LoopHeight
	.EndOf_LoopHeight:

	%undef vecSum0
	%undef vecSum1
	%undef vecSum2
	%undef vecSum3
	%undef vecCoeff
	%undef vecCoeffn
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
	%undef int16w			
	%undef int16q			
	%undef int16d			
	%undef kernSize

	; free memory and unalign stack
	add rsp, (32*COMPV_YASM_INT16_SZ_BYTES)
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
	COMPV_YASM_SAVE_YMM 8
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
	sub rsp, (32*COMPV_YASM_INT16_SZ_BYTES)

	%define vecSum0				ymm0
	%define vecSum1				ymm1
	%define vecSum2				ymm2
	%define vecSum3				ymm3
	%define vecCoeff			ymm4
	%define vecCoeffn			xmm4
	%define vec0				ymm5
	%define vec1				ymm6
	%define vec2				ymm7
	%define vec3				ymm8

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
	%define int16w			r14w
	%define int16q			r14
	%define int16d			r14d
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
			vpxor vecSum0, vecSum0, vecSum0
			vpxor vecSum1, vecSum1, vecSum1
			vpxor vecSum2, vecSum2, vecSum2
			vpxor vecSum3, vecSum3, vecSum3
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea inPtrPlusI, [inPtr + i*COMPV_YASM_INT16_SZ_BYTES]
			xor row, row
			.LoopKernel:
				vpmovsxwd vec0, [inPtrPlusI + 0*COMPV_YASM_INT16_SZ_BYTES]		
				vpmovsxwd vec1, [inPtrPlusI + 8*COMPV_YASM_INT16_SZ_BYTES]
				vpmovsxwd vec2, [inPtrPlusI + 16*COMPV_YASM_INT16_SZ_BYTES]
				vpmovsxwd vec3, [inPtrPlusI + 24*COMPV_YASM_INT16_SZ_BYTES]
				movsx int16d, word [vthzKernPtr + row*COMPV_YASM_INT16_SZ_BYTES]
				lea inPtrPlusI, [inPtrPlusI + step*COMPV_YASM_INT16_SZ_BYTES]
				vmovd vecCoeffn, int16d
				inc row
				vpbroadcastd vecCoeff, vecCoeffn
				vpmulld vec0, vec0, vecCoeff
				vpmulld vec1, vec1, vecCoeff
				vpmulld vec2, vec2, vecCoeff
				vpmulld vec3, vec3, vecCoeff				
				cmp row, kernSize
				vpaddd vecSum0, vecSum0, vec0
				vpaddd vecSum1, vecSum1, vec1
				vpaddd vecSum2, vecSum2, vec2
				vpaddd vecSum3, vecSum3, vec3
				jl .LoopKernel
			.EndOf_LoopKernel:
						
			vpackssdw vecSum0, vecSum1
			vpackssdw vecSum2, vecSum3
			cmp i, width32
			vpermq vecSum0, vecSum0, 0xD8
			vpermq vecSum2, vecSum2, 0xD8
			jge .MoreThanWidth32
				
			;; if (i < width32) ;;
			.LessThanWidth32:
				vmovdqu [outPtr + (i+0)*COMPV_YASM_INT16_SZ_BYTES], vecSum0
				vmovdqu [outPtr + (i+16)*COMPV_YASM_INT16_SZ_BYTES], vecSum2
				jmp .EndOf_MoreThanWidth32
			.EndOf_LessThanWidth32:

			;; if (i >= width32) ;;
			.MoreThanWidth32:
				vmovdqa [mem + (0)*COMPV_YASM_INT16_SZ_BYTES], vecSum0
				vmovdqa [mem + (16)*COMPV_YASM_INT16_SZ_BYTES], vecSum2
				;; for (k = 0; i < width; ++i, ++k) ;;
				xor k, k
				.LoopMoreThanWidth32:
					mov int16w, word [mem + k*COMPV_YASM_INT16_SZ_BYTES]
					inc k
					mov [outPtr + i*COMPV_YASM_INT16_SZ_BYTES], word int16w
					inc i
					cmp i, width
					jl .LoopMoreThanWidth32
				.EndOf_LoopMoreThanWidth32:
				jmp .EndOf_LoopWidth
			.EndOf_MoreThanWidth32:
			
			add i, 32
			cmp i, width
			jl .LoopWidth
		.EndOf_LoopWidth:
		
		dec j
		lea inPtr, [inPtr + stride*COMPV_YASM_INT16_SZ_BYTES]
		lea outPtr, [outPtr + stride*COMPV_YASM_INT16_SZ_BYTES]
		jnz .LoopHeight
	.EndOf_LoopHeight:

	%undef vecSum0
	%undef vecSum1
	%undef vecSum2
	%undef vecSum3
	%undef vecCoeff
	%undef vecCoeffn
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
	%undef int16w			
	%undef int16q			
	%undef int16d			
	%undef kernSize

	; free memory and unalign stack
	add rsp, (32*COMPV_YASM_INT16_SZ_BYTES)
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

%endif ; COMPV_YASM_ABI_IS_64BIT

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

global sym(CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_X64_SSE2)
global sym(CompVMathConvlt1VtHz_8u32f8u_Asm_X64_SSE2)
global sym(CompVMathConvlt1VtHz_8u32f32f_Asm_X64_SSE2)
global sym(CompVMathConvlt1VtHz_32f32f32f_Asm_X64_SSE2)
global sym(CompVMathConvlt1VtHz_32f32f8u_Asm_X64_SSE2)
global sym(CompVMathConvlt1VtHz_8u16s16s_Asm_X64_SSE2)
global sym(CompVMathConvlt1VtHz_16s16s16s_Asm_X64_SSE2)


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
sym(CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_X64_SSE2):
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
	COMPV_YASM_ALIGN_STACK 16, rax
	sub rsp, (16*COMPV_YASM_UINT8_SZ_BYTES)

	%define vecZero				xmm0
	%define vecSum0				xmm1
	%define vecSum1				xmm2
	%define vecCoeff			xmm3
	%define vec0				xmm4
	%define vec1				xmm5

	pxor vecZero, vecZero

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
	%define width16			r12
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
	mov width16, width
	and width16, -16
	mov kernSize, arg(argi_kernSize)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		.LoopWidth:
			pxor vecSum0, vecSum0
			pxor vecSum1, vecSum1
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea inPtrPlusI, [inPtr + i*COMPV_YASM_UINT8_SZ_BYTES]
			xor row, row
			.LoopKernel:
				movdqu vec0, [inPtrPlusI]
				movd vecCoeff, dword [vthzKernPtr + row*COMPV_YASM_UINT16_SZ_BYTES]
				lea inPtrPlusI, [inPtrPlusI + step*COMPV_YASM_UINT8_SZ_BYTES]
				inc row
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0x00
				movdqa vec1, vec0
				punpcklbw vec0, vecZero
				pmulhuw vec0, vecCoeff
				punpckhbw vec1, vecZero
				pmulhuw vec1, vecCoeff
				cmp row, kernSize
				paddusw vecSum0, vec0
				paddusw vecSum1, vec1	
				jl .LoopKernel
			.EndOf_LoopKernel:
			
			cmp i, width16
			packuswb vecSum0, vecSum1
			jge .MoreThanWidth16
				
			;; if (i < width16) ;;
			.LessThanWidth16:
				movdqu [outPtr + i*COMPV_YASM_UINT8_SZ_BYTES], vecSum0
				jmp .EndOf_MoreThanWidth16
			.EndOf_LessThanWidth16:

			;; if (i >= width16) ;;
			.MoreThanWidth16:
				movdqa [mem], vecSum0
				;; for (k = 0; i < width; ++i, ++k) ;;
				xor k, k
				.LoopMoreThanWidth16:
					mov octet, byte [mem + k*COMPV_YASM_UINT8_SZ_BYTES]
					inc k
					mov [outPtr + i*COMPV_YASM_UINT8_SZ_BYTES], byte octet
					inc i
					cmp i, width
					jl .LoopMoreThanWidth16
				.EndOf_LoopMoreThanWidth16:
			.EndOf_MoreThanWidth16:
			
			add i, 16
			cmp i, width
			jl .LoopWidth
		.EndOf_LoopWidth:
		
		dec j
		lea inPtr, [inPtr + stride*COMPV_YASM_UINT8_SZ_BYTES]
		lea outPtr, [outPtr + stride*COMPV_YASM_UINT8_SZ_BYTES]
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
	%undef width16
	%undef inPtrPlusI
	%undef octet
	%undef kernSize

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
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
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
sym(CompVMathConvlt1VtHz_8u32f8u_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_XMM 9
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

	%define vecZero				xmm0
	%define vecSum0				xmm1
	%define vecSum1				xmm2
	%define vecSum2				xmm3
	%define vecSum3				xmm4
	%define vecCoeff			xmm5
	%define vec0				xmm6
	%define vec1				xmm7
	%define vec2				xmm8
	%define vec3				xmm9

	pxor vecZero, vecZero

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
	%define width16			r12
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
	mov width16, width
	and width16, -16
	mov kernSize, arg(argi_kernSize)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		.LoopWidth:
			xorps vecSum0, vecSum0
			xorps vecSum1, vecSum1
			xorps vecSum2, vecSum2
			xorps vecSum3, vecSum3
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea inPtrPlusI, [inPtr + i*COMPV_YASM_UINT8_SZ_BYTES]
			xor row, row
			.LoopKernel:
				movdqu vec1, [inPtrPlusI]
				movss vecCoeff, [vthzKernPtr + row*COMPV_YASM_FLOAT32_SZ_BYTES]
				lea inPtrPlusI, [inPtrPlusI + step*COMPV_YASM_UINT8_SZ_BYTES]
				inc row
				shufps vecCoeff, vecCoeff, 0x00
				movdqa vec3, vec1
				punpcklbw vec1, vecZero
				punpckhbw vec3, vecZero
				movdqa vec0, vec1
				movdqa vec2, vec3
				punpcklwd vec0, vecZero
				punpckhwd vec1, vecZero
				cvtdq2ps vec0, vec0
				cvtdq2ps vec1, vec1
				punpcklwd vec2, vecZero
				punpckhwd vec3, vecZero
				cvtdq2ps vec2, vec2
				cvtdq2ps vec3, vec3
				mulps vec0, vecCoeff
				mulps vec1, vecCoeff
				mulps vec2, vecCoeff
				mulps vec3, vecCoeff
				cmp row, kernSize
				addps vecSum0, vec0
				addps vecSum1, vec1
				addps vecSum2, vec2
				addps vecSum3, vec3
				jl .LoopKernel
			.EndOf_LoopKernel:

			cvttps2dq vecSum0, vecSum0
			cvttps2dq vecSum1, vecSum1
			cvttps2dq vecSum2, vecSum2
			cvttps2dq vecSum3, vecSum3
			cmp i, width16
			packssdw vecSum0, vecSum1
			packssdw vecSum2, vecSum3
			packuswb vecSum0, vecSum2
			jge .MoreThanWidth16
				
			;; if (i < width16) ;;
			.LessThanWidth16:
				movdqu [outPtr + i*COMPV_YASM_UINT8_SZ_BYTES], vecSum0
				jmp .EndOf_MoreThanWidth16
			.EndOf_LessThanWidth16:

			;; if (i >= width16) ;;
			.MoreThanWidth16:
				movdqa [mem], vecSum0
				;; for (k = 0; i < width; ++i, ++k) ;;
				xor k, k
				.LoopMoreThanWidth16:
					mov octet, byte ptr [mem + k*COMPV_YASM_UINT8_SZ_BYTES]
					inc k
					mov [outPtr + i*COMPV_YASM_UINT8_SZ_BYTES], byte octet
					inc i
					cmp i, width
					jl .LoopMoreThanWidth16
				.EndOf_LoopMoreThanWidth16:
			.EndOf_MoreThanWidth16:
			
			add i, 16
			cmp i, width
			jl .LoopWidth
		.EndOf_LoopWidth:
		
		dec j
		lea inPtr, [inPtr + stride*COMPV_YASM_UINT8_SZ_BYTES]
		lea outPtr, [outPtr + stride*COMPV_YASM_UINT8_SZ_BYTES]
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
	%undef width16
	%undef inPtrPlusI
	%undef octet
	%undef kernSize

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



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* inPtr
; arg(1) -> compv_float32_t* outPtr
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> compv_uscalar_t step
; arg(5) -> compv_uscalar_t pad
; arg(6) -> const compv_float32_t* vthzKernPtr
; arg(7) -> compv_uscalar_t kernSize
sym(CompVMathConvlt1VtHz_8u32f32f_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_XMM 9
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
	sub rsp, (16*COMPV_YASM_FLOAT32_SZ_BYTES)

	%define vecZero				xmm0
	%define vecSum0				xmm1
	%define vecSum1				xmm2
	%define vecSum2				xmm3
	%define vecSum3				xmm4
	%define vecCoeff			xmm5
	%define vec0				xmm6
	%define vec1				xmm7
	%define vec2				xmm8
	%define vec3				xmm9

	pxor vecZero, vecZero

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
	%define width16			r12
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
	mov width16, width
	and width16, -16
	mov kernSize, arg(argi_kernSize)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		.LoopWidth:
			xorps vecSum0, vecSum0
			xorps vecSum1, vecSum1
			xorps vecSum2, vecSum2
			xorps vecSum3, vecSum3
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea inPtrPlusI, [inPtr + i*COMPV_YASM_UINT8_SZ_BYTES]
			xor row, row
			.LoopKernel:
				movdqu vec1, [inPtrPlusI]
				movss vecCoeff, [vthzKernPtr + row*COMPV_YASM_FLOAT32_SZ_BYTES]
				lea inPtrPlusI, [inPtrPlusI + step*COMPV_YASM_UINT8_SZ_BYTES]
				inc row
				shufps vecCoeff, vecCoeff, 0x00
				movdqa vec3, vec1
				punpcklbw vec1, vecZero
				punpckhbw vec3, vecZero
				movdqa vec0, vec1
				movdqa vec2, vec3
				punpcklwd vec0, vecZero
				punpckhwd vec1, vecZero
				cvtdq2ps vec0, vec0
				cvtdq2ps vec1, vec1
				punpcklwd vec2, vecZero
				punpckhwd vec3, vecZero
				cvtdq2ps vec2, vec2
				cvtdq2ps vec3, vec3
				mulps vec0, vecCoeff
				mulps vec1, vecCoeff
				mulps vec2, vecCoeff
				mulps vec3, vecCoeff
				cmp row, kernSize
				addps vecSum0, vec0
				addps vecSum1, vec1
				addps vecSum2, vec2
				addps vecSum3, vec3
				jl .LoopKernel
			.EndOf_LoopKernel:
			
			cmp i, width16
			jge .MoreThanWidth16
				
			;; if (i < width16) ;;
			.LessThanWidth16:
				movups xmmword ptr [outPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum0
				movups xmmword ptr [outPtr + (i+4)*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum1
				movups xmmword ptr [outPtr + (i+8)*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum2
				movups xmmword ptr [outPtr + (i+12)*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum3
				jmp .EndOf_MoreThanWidth16
			.EndOf_LessThanWidth16:

			;; if (i >= width16) ;;
			.MoreThanWidth16:
				movaps xmmword ptr [mem + (0)*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum0
				movaps xmmword ptr [mem + (4)*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum1
				movaps xmmword ptr [mem + (8)*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum2
				movaps xmmword ptr [mem + (12)*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum3
				;; for (k = 0; i < width; ++i, ++k) ;;
				xor k, k
				.LoopMoreThanWidth16:
					mov float32, dword [mem + k*COMPV_YASM_FLOAT32_SZ_BYTES]
					inc k
					mov [outPtr + i*COMPV_YASM_FLOAT32_SZ_BYTES], dword float32
					inc i
					cmp i, width
					jl .LoopMoreThanWidth16
				.EndOf_LoopMoreThanWidth16:
			.EndOf_MoreThanWidth16:
			
			add i, 16
			cmp i, width
			jl .LoopWidth
		.EndOf_LoopWidth:
		
		dec j
		lea inPtr, [inPtr + stride*COMPV_YASM_UINT8_SZ_BYTES]
		lea outPtr, [outPtr + stride*COMPV_YASM_FLOAT32_SZ_BYTES]
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
	%undef width16
	%undef inPtrPlusI
	%undef float32
	%undef kernSize

	; free memory and unalign stack
	add rsp, (16*COMPV_YASM_FLOAT32_SZ_BYTES)
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



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const compv_float32_t* inPtr
; arg(1) -> compv_float32_t* outPtr
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> compv_uscalar_t step
; arg(5) -> compv_uscalar_t pad
; arg(6) -> const compv_float32_t* vthzKernPtr
; arg(7) -> compv_uscalar_t kernSize
sym(CompVMathConvlt1VtHz_32f32f32f_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_XMM 9
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
	sub rsp, (16*COMPV_YASM_FLOAT32_SZ_BYTES)

	%define vecZero				xmm0
	%define vecSum0				xmm1
	%define vecSum1				xmm2
	%define vecSum2				xmm3
	%define vecSum3				xmm4
	%define vecCoeff			xmm5
	%define vec0				xmm6
	%define vec1				xmm7
	%define vec2				xmm8
	%define vec3				xmm9

	pxor vecZero, vecZero

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
	%define width16			r12
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
	mov width16, width
	and width16, -16
	mov kernSize, arg(argi_kernSize)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		.LoopWidth:
			xorps vecSum0, vecSum0
			xorps vecSum1, vecSum1
			xorps vecSum2, vecSum2
			xorps vecSum3, vecSum3
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea inPtrPlusI, [inPtr + i*COMPV_YASM_FLOAT32_SZ_BYTES]
			xor row, row
			.LoopKernel:
				movups xmmword ptr vec0, [inPtrPlusI + 0*COMPV_YASM_FLOAT32_SZ_BYTES]
				movups xmmword ptr vec1, [inPtrPlusI + 4*COMPV_YASM_FLOAT32_SZ_BYTES]
				movups xmmword ptr vec2, [inPtrPlusI + 8*COMPV_YASM_FLOAT32_SZ_BYTES]
				movups xmmword ptr vec3, [inPtrPlusI + 12*COMPV_YASM_FLOAT32_SZ_BYTES]
				movss vecCoeff, [vthzKernPtr + row*COMPV_YASM_FLOAT32_SZ_BYTES]
				lea inPtrPlusI, [inPtrPlusI + step*COMPV_YASM_FLOAT32_SZ_BYTES]
				inc row
				shufps vecCoeff, vecCoeff, 0x00
				mulps vec0, vecCoeff
				mulps vec1, vecCoeff
				mulps vec2, vecCoeff
				mulps vec3, vecCoeff
				cmp row, kernSize
				addps vecSum0, vec0
				addps vecSum1, vec1
				addps vecSum2, vec2
				addps vecSum3, vec3
				jl .LoopKernel
			.EndOf_LoopKernel:
			
			cmp i, width16
			jge .MoreThanWidth16
				
			;; if (i < width16) ;;
			.LessThanWidth16:
				movups xmmword ptr [outPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum0
				movups xmmword ptr [outPtr + (i+4)*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum1
				movups xmmword ptr [outPtr + (i+8)*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum2
				movups xmmword ptr [outPtr + (i+12)*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum3
				jmp .EndOf_MoreThanWidth16
			.EndOf_LessThanWidth16:

			;; if (i >= width16) ;;
			.MoreThanWidth16:
				movaps xmmword ptr [mem + 0*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum0
				movaps xmmword ptr [mem + 4*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum1
				movaps xmmword ptr [mem + 8*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum2
				movaps xmmword ptr [mem + 12*COMPV_YASM_FLOAT32_SZ_BYTES], vecSum3
				;; for (k = 0; i < width; ++i, ++k) ;;
				xor k, k
				.LoopMoreThanWidth16:
					mov float32, dword [mem + k*COMPV_YASM_FLOAT32_SZ_BYTES]
					inc k
					mov [outPtr + i*COMPV_YASM_FLOAT32_SZ_BYTES], dword float32
					inc i
					cmp i, width
					jl .LoopMoreThanWidth16
				.EndOf_LoopMoreThanWidth16:
			.EndOf_MoreThanWidth16:
			
			add i, 16
			cmp i, width
			jl .LoopWidth
		.EndOf_LoopWidth:
		
		dec j
		lea inPtr, [inPtr + stride*COMPV_YASM_FLOAT32_SZ_BYTES]
		lea outPtr, [outPtr + stride*COMPV_YASM_FLOAT32_SZ_BYTES]
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
	%undef width16
	%undef inPtrPlusI
	%undef float32
	%undef kernSize

	; free memory and unalign stack
	add rsp, (16*COMPV_YASM_FLOAT32_SZ_BYTES)
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



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const compv_float32_t* inPtr
; arg(1) -> uint8_t* outPtr
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> compv_uscalar_t step
; arg(5) -> compv_uscalar_t pad
; arg(6) -> const compv_float32_t* vthzKernPtr
; arg(7) -> compv_uscalar_t kernSize
sym(CompVMathConvlt1VtHz_32f32f8u_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_XMM 9
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

	%define vecZero				xmm0
	%define vecSum0				xmm1
	%define vecSum1				xmm2
	%define vecSum2				xmm3
	%define vecSum3				xmm4
	%define vecCoeff			xmm5
	%define vec0				xmm6
	%define vec1				xmm7
	%define vec2				xmm8
	%define vec3				xmm9

	pxor vecZero, vecZero

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
	%define width16			r12
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
	lea stride, [width + stride] ; stride = (width + pad)
	mov width16, width
	and width16, -16
	mov kernSize, arg(argi_kernSize)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		.LoopWidth:
			xorps vecSum0, vecSum0
			xorps vecSum1, vecSum1
			xorps vecSum2, vecSum2
			xorps vecSum3, vecSum3
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea inPtrPlusI, [inPtr + i*COMPV_YASM_FLOAT32_SZ_BYTES]
			xor row, row
			.LoopKernel:
				movups xmmword ptr vec0, [inPtrPlusI + 0*COMPV_YASM_FLOAT32_SZ_BYTES]
				movups xmmword ptr vec1, [inPtrPlusI + 4*COMPV_YASM_FLOAT32_SZ_BYTES]
				movups xmmword ptr vec2, [inPtrPlusI + 8*COMPV_YASM_FLOAT32_SZ_BYTES]
				movups xmmword ptr vec3, [inPtrPlusI + 12*COMPV_YASM_FLOAT32_SZ_BYTES]
				movss vecCoeff, [vthzKernPtr + row*COMPV_YASM_FLOAT32_SZ_BYTES]
				lea inPtrPlusI, [inPtrPlusI + step*COMPV_YASM_FLOAT32_SZ_BYTES]
				inc row
				shufps vecCoeff, vecCoeff, 0x00
				mulps vec0, vecCoeff
				mulps vec1, vecCoeff
				mulps vec2, vecCoeff
				mulps vec3, vecCoeff
				cmp row, kernSize
				addps vecSum0, vec0
				addps vecSum1, vec1
				addps vecSum2, vec2
				addps vecSum3, vec3
				jl .LoopKernel
			.EndOf_LoopKernel:
			
			cvttps2dq vecSum0, vecSum0
			cvttps2dq vecSum1, vecSum1
			cvttps2dq vecSum2, vecSum2
			cvttps2dq vecSum3, vecSum3
			cmp i, width16
			packssdw vecSum0, vecSum1
			packssdw vecSum2, vecSum3
			packuswb vecSum0, vecSum2
			jge .MoreThanWidth16
				
			;; if (i < width16) ;;
			.LessThanWidth16:
				movdqu xmmword ptr [outPtr + (i+0)*COMPV_YASM_UINT8_SZ_BYTES], vecSum0
				jmp .EndOf_MoreThanWidth16
			.EndOf_LessThanWidth16:

			;; if (i >= width16) ;;
			.MoreThanWidth16:
				movdqu xmmword ptr [mem + 0*COMPV_YASM_UINT8_SZ_BYTES], vecSum0
				;; for (k = 0; i < width; ++i, ++k) ;;
				xor k, k
				.LoopMoreThanWidth16:
					mov octet, byte [mem + k*COMPV_YASM_UINT8_SZ_BYTES]
					inc k
					mov [outPtr + i*COMPV_YASM_UINT8_SZ_BYTES], byte octet
					inc i
					cmp i, width
					jl .LoopMoreThanWidth16
				.EndOf_LoopMoreThanWidth16:
				jmp .EndOf_LoopWidth
			.EndOf_MoreThanWidth16:
			
			add i, 16
			cmp i, width
			jl .LoopWidth
		.EndOf_LoopWidth:
		
		dec j
		lea inPtr, [inPtr + stride*COMPV_YASM_FLOAT32_SZ_BYTES]
		lea outPtr, [outPtr + stride*COMPV_YASM_UINT8_SZ_BYTES]
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
	%undef width16
	%undef inPtrPlusI
	%undef float32
	%undef kernSize

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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* inPtr
; arg(1) -> int16_t* outPtr
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> compv_uscalar_t step
; arg(5) -> compv_uscalar_t pad
; arg(6) -> const int16_t* vthzKernPtr
; arg(7) -> compv_uscalar_t kernSize
sym(CompVMathConvlt1VtHz_8u16s16s_Asm_X64_SSE2):
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

	%define vecZero				xmm0
	%define vecSum0				xmm1
	%define vecSum1				xmm2
	%define vecCoeff			xmm3
	%define vec0				xmm4
	%define vec1				xmm5

	pxor vecZero, vecZero

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
	%define vthzKernPtr		r10
	%define stride			r11
	%define widthMinus15	r12
	%define widthMinus3		r13
	%define octet			r14
	%define kernSize		r15
	mov width, arg(argi_width)
	mov step, arg(argi_step)
	mov j, arg(argi_height)
	mov inPtr, arg(argi_inPtr)
	mov outPtr, arg(argi_outPtr)
	mov vthzKernPtr, arg(argi_vthzKernPtr)
	mov stride, arg(argi_pad)
	lea stride, [width + stride] ; stride = width + pad
	lea widthMinus15, [width - 15]
	lea widthMinus3, [width - 3]
	mov kernSize, arg(argi_kernSize)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width - 15; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		.LoopWidth_Per16Samples:
			pxor vecSum0, vecSum0
			pxor vecSum1, vecSum1
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i] ; rax = &inPtr[i]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per16Samples:
				movdqu vec0, [rax]
				movd vecCoeff, [vthzKernPtr + rdx*COMPV_YASM_INT16_SZ_BYTES]
				add rax, step
				inc rdx
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				movdqa vec1, vec0
				punpcklbw vec0, vecZero
				pmullw vec0, vecCoeff
				punpckhbw vec1, vecZero
				pmullw vec1, vecCoeff
				cmp rdx, kernSize
				paddw vecSum0, vec0
				paddw vecSum1, vec1	
				jl .LoopKernelSize_Per16Samples
				; EndOf_LoopKernelSize_Per16Samples ;
			
			movdqu [outPtr + (i + 0)*COMPV_YASM_INT16_SZ_BYTES], vecSum0
			movdqu [outPtr + (i + 8)*COMPV_YASM_INT16_SZ_BYTES], vecSum1
			lea i, [i + 16]
			cmp i, widthMinus15
			jl .LoopWidth_Per16Samples
			; EndOf_LoopWidth_Per16Samples ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width - 7)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea rax, [width - 7]
		cmp i, rax
		jge .EndOf_If_Per8Samples
		.If_Per8Samples:
			pxor vecSum0, vecSum0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i] ; rax = &inPtr[i]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per8Samples:
				movq vec0, [rax]
				add rax, step
				movd vecCoeff, [vthzKernPtr + rdx*COMPV_YASM_INT16_SZ_BYTES]
				inc rdx
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				punpcklbw vec0, vecZero
				pmullw vec0, vecCoeff
				cmp rdx, kernSize
				paddw vecSum0, vec0
				jl .LoopKernelSize_Per8Samples
				; EndOf_LoopKernelSize_Per8Samples ;
						
			movdqu [outPtr + i*COMPV_YASM_INT16_SZ_BYTES], vecSum0
			lea i, [i + 8]
			.EndOf_If_Per8Samples
			; EndOf_If_Per8Samples ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width - 3)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, widthMinus3
		jge .EndOf_If_Per4Samples
		.If_Per4Samples:
			pxor vecSum0, vecSum0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i] ; rax = &inPtr[i]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per4Samples:
				movd vec0, [rax]
				add rax, step
				movd vecCoeff, [vthzKernPtr + rdx*COMPV_YASM_INT16_SZ_BYTES]
				inc rdx
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				punpcklbw vec0, vecZero
				pmullw vec0, vecCoeff
				cmp rdx, kernSize
				paddw vecSum0, vec0
				jl .LoopKernelSize_Per4Samples
				; EndOf_LoopKernelSize_Per4Samples ;

			movq [outPtr + i*COMPV_YASM_INT16_SZ_BYTES], vecSum0
			lea i, [i + 4]
			.EndOf_If_Per4Samples
			; EndOf_If_Per4Samples ;

		
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_If_Per1Samples
		.If_Per1Samples:
			pxor vecSum0, vecSum0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i] ; rax = &inPtr[i]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per1Samples:
				movd vec0, [rax]
				add rax, step
				movd vecCoeff, [vthzKernPtr + rdx*COMPV_YASM_INT16_SZ_BYTES]
				inc rdx
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				punpcklbw vec0, vecZero
				pmullw vec0, vecCoeff
				cmp rdx, kernSize
				paddw vecSum0, vec0
				jl .LoopKernelSize_Per1Samples
				; EndOf_LoopKernelSize_Per1Samples ;
						
			movq rax, vecSum0
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
		lea inPtr, [inPtr + stride]
		lea outPtr, [outPtr + stride*COMPV_YASM_INT16_SZ_BYTES]
		jnz .LoopHeight
		; EndOf_LoopHeight ;

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

	%undef width
	%undef step
	%undef j
	%undef i
	%undef inPtr
	%undef outPtr
	%undef vthzKernPtr
	%undef widthMinus15
	%undef widthMinus3
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
sym(CompVMathConvlt1VtHz_16s16s16s_Asm_X64_SSE2):
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

	%define vecSum0				xmm0
	%define vecSum1				xmm1
	%define vecCoeff			xmm2
	%define vec0				xmm3
	%define vec1				xmm4

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
	%define vthzKernPtr		r10
	%define stride			r11
	%define widthMinus15	r12
	%define widthMinus3		r13
	%define octet			r14
	%define kernSize		r15
	mov width, arg(argi_width)
	mov step, arg(argi_step)
	lea step, [step * COMPV_YASM_INT16_SZ_BYTES] ; from samples to bytes
	mov j, arg(argi_height)
	mov inPtr, arg(argi_inPtr)
	mov outPtr, arg(argi_outPtr)
	mov vthzKernPtr, arg(argi_vthzKernPtr)
	mov stride, arg(argi_pad)
	lea stride, [width + stride] ; stride = width + pad
	lea widthMinus15, [width - 15]
	lea widthMinus3, [width - 3]
	mov kernSize, arg(argi_kernSize)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width - 15; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		.LoopWidth_Per16Samples:
			pxor vecSum0, vecSum0
			pxor vecSum1, vecSum1
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i*COMPV_YASM_INT16_SZ_BYTES] ; rax = &inPtr[i]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per16Samples:
				movdqu vec0, [rax]
				movdqu vec1, [rax + 8*COMPV_YASM_INT16_SZ_BYTES]
				movd vecCoeff, [vthzKernPtr + rdx*COMPV_YASM_INT16_SZ_BYTES]
				add rax, step
				inc rdx
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				pmullw vec0, vecCoeff
				pmullw vec1, vecCoeff
				cmp rdx, kernSize
				paddw vecSum0, vec0
				paddw vecSum1, vec1	
				jl .LoopKernelSize_Per16Samples
				; EndOf_LoopKernelSize_Per16Samples ;
			
			movdqu [outPtr + (i + 0)*COMPV_YASM_INT16_SZ_BYTES], vecSum0
			movdqu [outPtr + (i + 8)*COMPV_YASM_INT16_SZ_BYTES], vecSum1
			lea i, [i + 16]
			cmp i, widthMinus15
			jl .LoopWidth_Per16Samples
			; EndOf_LoopWidth_Per16Samples ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width - 7)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea rax, [width - 7]
		cmp i, rax
		jge .EndOf_If_Per8Samples
		.If_Per8Samples:
			pxor vecSum0, vecSum0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i*COMPV_YASM_INT16_SZ_BYTES] ; rax = &inPtr[i]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per8Samples:
				movdqu vec0, [rax]
				add rax, step
				movd vecCoeff, [vthzKernPtr + rdx*COMPV_YASM_INT16_SZ_BYTES]
				inc rdx
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				pmullw vec0, vecCoeff
				cmp rdx, kernSize
				paddw vecSum0, vec0
				jl .LoopKernelSize_Per8Samples
				; EndOf_LoopKernelSize_Per8Samples ;
						
			movdqu [outPtr + i*COMPV_YASM_INT16_SZ_BYTES], vecSum0
			lea i, [i + 8]
			.EndOf_If_Per8Samples
			; EndOf_If_Per8Samples ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width - 3)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, widthMinus3
		jge .EndOf_If_Per4Samples
		.If_Per4Samples:
			pxor vecSum0, vecSum0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i*COMPV_YASM_INT16_SZ_BYTES] ; rax = &inPtr[i]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per4Samples:
				movq vec0, [rax]
				add rax, step
				movd vecCoeff, [vthzKernPtr + rdx*COMPV_YASM_INT16_SZ_BYTES]
				inc rdx
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				pmullw vec0, vecCoeff
				cmp rdx, kernSize
				paddw vecSum0, vec0
				jl .LoopKernelSize_Per4Samples
				; EndOf_LoopKernelSize_Per4Samples ;

			movq [outPtr + i*COMPV_YASM_INT16_SZ_BYTES], vecSum0
			lea i, [i + 4]
			.EndOf_If_Per4Samples
			; EndOf_If_Per4Samples ;

		
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_If_Per1Samples
		.If_Per1Samples:
			pxor vecSum0, vecSum0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i*COMPV_YASM_INT16_SZ_BYTES] ; rax = &inPtr[i]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per1Samples:
				movq vec0, [rax]
				add rax, step
				movd vecCoeff, [vthzKernPtr + rdx*COMPV_YASM_INT16_SZ_BYTES]
				inc rdx
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				pmullw vec0, vecCoeff
				cmp rdx, kernSize
				paddw vecSum0, vec0
				jl .LoopKernelSize_Per1Samples
				; EndOf_LoopKernelSize_Per1Samples ;
						
			movq rax, vecSum0
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

	%undef width
	%undef step
	%undef j
	%undef i
	%undef inPtr
	%undef outPtr
	%undef vthzKernPtr
	%undef widthMinus15
	%undef widthMinus3
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
	ret


%endif ; COMPV_YASM_ABI_IS_64BIT

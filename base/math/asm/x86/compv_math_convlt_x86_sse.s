;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
%if 0 ; No longer used (deprecated), see x64 version
%include "compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(CompVMathConvlt1VtHz_8u32f8u_Asm_X86_SSE2)
global sym(CompVMathConvlt1VtHz_8u16s16s_Asm_X86_SSE2)
global sym(CompVMathConvlt1VtHz_16s16s16s_Asm_X86_SSE2)
global sym(CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_X86_SSE2)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* inPtr
; arg(1) -> uint8_t* outPtr
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> compv_uscalar_t step
; arg(5) -> compv_uscalar_t pad
; arg(6) -> const compv_float32_t* vthzKernPtr
; arg(7) -> compv_uscalar_t kernSize
sym(CompVMathConvlt1VtHz_8u32f8u_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_XMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 16, rax
	sub rsp, (2*16)

	%define vecZero				rsp + 0
	%define vecSum0				vecZero + 16
	pxor xmm0, xmm0
	movdqa [vecZero], xmm0

	%define argi_inPtr			0
	%define argi_outPtr			1
	%define argi_width			2
	%define argi_height			3
	%define argi_step			4 
	%define argi_pad			5
	%define argi_vthzKernPtr	6
	%define argi_kernSize		7

	mov rax, arg(argi_width)
	mov rbx, arg(argi_pad)
	lea rax, [rax + rbx]
	%define argi_stride 5 ; argi_pad
	mov arg(argi_stride), rax ; stride = (pad + width)
	%undef argi_pad

	%define width	rcx
	%define j		rsi
	%define i		rdi
	mov j, arg(argi_height)
	mov width, arg(argi_width)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width - 15; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		.LoopWidth_Per16Samples:
			xorps xmm5, xmm5 ; vecSum1
			xorps xmm6, xmm6 ; vecSum2
			xorps xmm7, xmm7 ; vecSum3
			movaps [vecSum0], xmm5
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per16Samples:
				movdqu xmm0, [rax] ; xmm0 = vecInPtr
				movss xmm4, [rbx + rdx*COMPV_YASM_FLOAT32_SZ_BYTES]
				inc rdx
				shufps xmm4, xmm4, 0x0 ; xmm4 = vecCoeff
				add rax, arg(argi_step)
				movdqa xmm3, xmm0
				punpcklbw xmm0, [vecZero]
				punpckhbw xmm3, [vecZero]
				cmp rdx, arg(argi_kernSize)
				movdqa xmm1, xmm0
				movdqa xmm2, xmm3
				punpcklwd xmm0, [vecZero]
				punpckhwd xmm1, [vecZero]
				punpcklwd xmm2, [vecZero]
				punpckhwd xmm3, [vecZero]
				cvtdq2ps xmm0, xmm0
				cvtdq2ps xmm1, xmm1
				cvtdq2ps xmm2, xmm2
				cvtdq2ps xmm3, xmm3
				mulps xmm0, xmm4
				mulps xmm1, xmm4
				mulps xmm2, xmm4
				mulps xmm3, xmm4
				addps xmm0, [vecSum0]
				addps xmm5, xmm1
				addps xmm6, xmm2
				addps xmm7, xmm3
				movdqa [vecSum0], xmm0		
				jl .LoopKernelSize_Per16Samples
				; EndOf_LoopKernelSize_Per16Samples ;
			
			cvttps2dq xmm0, [vecSum0]
			cvttps2dq xmm5, xmm5
			cvttps2dq xmm6, xmm6
			cvttps2dq xmm7, xmm7
			mov rdx, arg(argi_outPtr)
			lea rax, [width - 15]
			lea i, [i + 16]
			cmp i, rax
			packssdw xmm0, xmm5
			packssdw xmm6, xmm7
			packuswb xmm0, xmm6
			movdqu [rdx + i - 16], xmm0
			jl .LoopWidth_Per16Samples
			; EndOf_LoopWidth_Per16Samples ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width - 3; i += 4)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea rax, [width - 3]
		cmp i, rax
		jge .EndOf_LoopWidth_Per4Samples
		.LoopWidth_Per4Samples:
			xorps xmm0, xmm0 ; vecSum0
			pxor xmm1, xmm1 ; vecZero
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per4Samples:
				movss xmm2, [rax] ; xmm2 = vecInPtr
				movss xmm3, [rbx + rdx*COMPV_YASM_FLOAT32_SZ_BYTES]
				inc rdx
				add rax, arg(argi_step)
				punpcklbw xmm2, xmm1
				punpcklwd xmm2, xmm1
				cvtdq2ps xmm2, xmm2
				shufps xmm3, xmm3, 0x0 ; xmm3 = vecCoeff
				mulps xmm2, xmm3
				cmp rdx, arg(argi_kernSize)
				addps xmm0, xmm2
				jl .LoopKernelSize_Per4Samples
				; EndOf_LoopKernelSize_Per4Samples ;


			mov rdx, arg(argi_outPtr)
			lea rax, [width - 3]
			lea i, [i + 4]
			cmp i, rax
			cvttps2dq xmm0, xmm0
			packssdw xmm0, xmm0
			packuswb xmm0, xmm0
			movd [rdx + i - 4], xmm0
			jl .LoopWidth_Per4Samples
			.EndOf_LoopWidth_Per4Samples
			; EndOf_LoopWidth_Per4Samples ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width; i += 1)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_LoopWidth_Per1Samples
		.LoopWidth_Per1Samples:
			xorps xmm0, xmm0 ; vecSum0
			pxor xmm1, xmm1 ; xmm2 = vecZero
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per1Samples:
				movss xmm2, [rax] ; TODO(dmi): use movzx for x64, here we cannot because we''re out of 32b registers
				movss xmm3, [rbx + rdx*COMPV_YASM_FLOAT32_SZ_BYTES] ; xmm3 = vecCoeff
				inc rdx
				add rax, arg(argi_step)
				cmp rdx, arg(argi_kernSize)
				punpcklbw xmm2, xmm1
				punpcklwd xmm2, xmm1
				cvtdq2ps xmm2, xmm2
				mulss xmm2, xmm3
				addss xmm0, xmm2	
				jl .LoopKernelSize_Per1Samples
				; EndOf_LoopKernelSize_Per1Samples ;


			inc i
			mov rdx, arg(argi_outPtr)
			cmp i, width
			cvttss2si rax, xmm0
			mov [rdx + i - 1], byte al
			jl .LoopWidth_Per1Samples
			.EndOf_LoopWidth_Per1Samples
			; EndOf_LoopWidth_Per1Samples ;
		
		mov rax, arg(argi_inPtr)
		mov rbx, arg(argi_outPtr)
		mov rdx, arg(argi_stride)
		dec j
		lea rax, [rax + rdx]
		lea rbx, [rbx + rdx]
		mov arg(argi_inPtr), rax
		mov arg(argi_outPtr), rbx
		jnz .LoopHeight
		; EndOf_LoopHeight ;

	%undef vecZero
	%undef vecSum0

	%undef argi_inPtr
	%undef argi_outPtr
	%undef argi_width
	%undef argi_height
	%undef argi_step
	%undef argi_pad
	%undef argi_vthzKernPtr
	%undef argi_kernSize
	%undef argi_stride

	%undef width
	%undef j
	%undef i

	; free memory and unalign stack
	add rsp, (2*16)
	COMPV_YASM_UNALIGN_STACK

	;; begin epilog ;;
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
sym(CompVMathConvlt1VtHz_8u16s16s_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	push rsi
	push rdi
	push rbx
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

	mov rax, arg(argi_width)
	mov rbx, arg(argi_pad)
	lea rax, [rax + rbx]
	%define argi_stride 5 ; argi_pad
	mov arg(argi_stride), rax ; stride = (pad + width)
	%undef argi_pad

	%define width	rcx
	%define j		rsi
	%define i		rdi
	mov j, arg(argi_height)
	mov width, arg(argi_width)

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
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per16Samples:
				movdqu vec0, [rax]
				movd vecCoeff, [rbx + rdx*COMPV_YASM_UINT16_SZ_BYTES]  
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				inc rdx
				movdqa vec1, vec0
				punpcklbw vec0, vecZero
				pmullw vec0, vecCoeff
				punpckhbw vec1, vecZero
				pmullw vec1, vecCoeff
				add rax, arg(argi_step)
				cmp rdx, arg(argi_kernSize)
				paddw vecSum0, vec0
				paddw vecSum1, vec1		
				jl .LoopKernelSize_Per16Samples
				; EndOf_LoopKernelSize_Per16Samples ;
			
			mov rdx, arg(argi_outPtr)
			lea rax, [width - 15]
			movdqu [rdx + (i + 0)*COMPV_YASM_INT16_SZ_BYTES], vecSum0
			movdqu [rdx + (i + 8)*COMPV_YASM_INT16_SZ_BYTES], vecSum1
			lea i, [i + 16]
			cmp i, rax
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
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per8Samples:
				movq vec0, [rax]
				movd vecCoeff, [rbx + rdx*COMPV_YASM_UINT16_SZ_BYTES]  
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				inc rdx
				add rax, arg(argi_step)
				punpcklbw vec0, vecZero
				pmullw vec0, vecCoeff
				cmp rdx, arg(argi_kernSize)
				paddw vecSum0, vec0
				jl .LoopKernelSize_Per8Samples
				; EndOf_LoopKernelSize_Per8Samples ;

			mov rdx, arg(argi_outPtr)
			movdqu [rdx + i*COMPV_YASM_INT16_SZ_BYTES], vecSum0
			lea i, [i + 8]
			.EndOf_If_Per8Samples
			; EndOf_If_Per8Samples ;


		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width -3)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea rax, [width - 3]
		cmp i, rax
		jge .EndOf_If_Per4Samples
		.If_Per4Samples:
			pxor vecSum0, vecSum0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per4Samples:
				movd vec0, [rax]
				movd vecCoeff, [rbx + rdx*COMPV_YASM_UINT16_SZ_BYTES]  
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				inc rdx
				add rax, arg(argi_step)
				punpcklbw vec0, vecZero
				pmullw vec0, vecCoeff
				cmp rdx, arg(argi_kernSize)
				paddw vecSum0, vec0
				jl .LoopKernelSize_Per4Samples
				; EndOf_LoopKernelSize_Per4Samples ;

			mov rdx, arg(argi_outPtr)
			movq [rdx + i*COMPV_YASM_INT16_SZ_BYTES], vecSum0
			lea i, [i + 4]
			.EndOf_If_Per4Samples
			; EndOf_If_Per4Samples ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea rax, [width]
		cmp i, rax
		jge .EndOf_If_Per1Samples
		.If_Per1Samples:
			pxor vecSum0, vecSum0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per1Samples:
				movd vec0, [rax]
				movd vecCoeff, [rbx + rdx*COMPV_YASM_UINT16_SZ_BYTES]  
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				inc rdx
				add rax, arg(argi_step)
				punpcklbw vec0, vecZero
				pmullw vec0, vecCoeff
				cmp rdx, arg(argi_kernSize)
				paddw vecSum0, vec0
				jl .LoopKernelSize_Per1Samples
				; EndOf_LoopKernelSize_Per1Samples ;

			mov rdx, arg(argi_outPtr)
			%assign index 0
			%rep 4
				%if index == 0
					movd rax, vecSum0
				%elif index == 2
					psrldq vecSum0, 4
					movd rax, vecSum0
				%endif
				mov [rdx + i*COMPV_YASM_INT16_SZ_BYTES], word ax
				inc i
				cmp i, width
				jge .EndOf_If_Per1Samples
				shr rax, 16
				%assign index index+1
			%endrep			
			.EndOf_If_Per1Samples
			; EndOf_If_Per1Samples ;
		
		mov rax, arg(argi_inPtr)
		mov rbx, arg(argi_outPtr)
		mov rdx, arg(argi_stride)
		dec j
		lea rax, [rax + rdx]
		lea rbx, [rbx + rdx*COMPV_YASM_INT16_SZ_BYTES]
		mov arg(argi_inPtr), rax
		mov arg(argi_outPtr), rbx
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
	%undef argi_stride

	%undef width
	%undef j
	%undef i

	;; begin epilog ;;
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
sym(CompVMathConvlt1VtHz_16s16s16s_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	push rsi
	push rdi
	push rbx
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

	mov rax, arg(argi_width)
	mov rbx, arg(argi_pad)
	lea rax, [rax + rbx]
	%define argi_stride 5 ; argi_pad
	mov arg(argi_stride), rax ; stride = (pad + width)
	%undef argi_pad

	%define width	rcx
	%define j		rsi
	%define i		rdi
	mov j, arg(argi_height)
	mov width, arg(argi_width)

	; convert step from samples to bytes
	mov rax, arg(argi_step)
	lea rax, [rax * COMPV_YASM_INT16_SZ_BYTES]
	mov arg(argi_step), rax

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
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i*COMPV_YASM_INT16_SZ_BYTES] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per16Samples:
				movdqu vec0, [rax]
				movdqu vec1, [rax + 8*COMPV_YASM_INT16_SZ_BYTES]
				movd vecCoeff, [rbx + rdx*COMPV_YASM_UINT16_SZ_BYTES]  
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				inc rdx
				pmullw vec0, vecCoeff
				pmullw vec1, vecCoeff
				add rax, arg(argi_step)
				cmp rdx, arg(argi_kernSize)
				paddw vecSum0, vec0
				paddw vecSum1, vec1		
				jl .LoopKernelSize_Per16Samples
				; EndOf_LoopKernelSize_Per16Samples ;
			
			mov rdx, arg(argi_outPtr)
			lea rax, [width - 15]
			movdqu [rdx + (i + 0)*COMPV_YASM_INT16_SZ_BYTES], vecSum0
			movdqu [rdx + (i + 8)*COMPV_YASM_INT16_SZ_BYTES], vecSum1
			lea i, [i + 16]
			cmp i, rax
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
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i*COMPV_YASM_INT16_SZ_BYTES] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per8Samples:
				movdqu vec0, [rax]
				movd vecCoeff, [rbx + rdx*COMPV_YASM_UINT16_SZ_BYTES]  
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				inc rdx
				pmullw vec0, vecCoeff
				add rax, arg(argi_step)
				cmp rdx, arg(argi_kernSize)
				paddw vecSum0, vec0
				jl .LoopKernelSize_Per8Samples
				; EndOf_LoopKernelSize_Per8Samples ;

			mov rdx, arg(argi_outPtr)
			movdqu [rdx + i*COMPV_YASM_INT16_SZ_BYTES], vecSum0
			lea i, [i + 8]
			.EndOf_If_Per8Samples
			; EndOf_If_Per8Samples ;


		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width -3)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea rax, [width - 3]
		cmp i, rax
		jge .EndOf_If_Per4Samples
		.If_Per4Samples:
			pxor vecSum0, vecSum0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i*COMPV_YASM_INT16_SZ_BYTES] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per4Samples:
				movq vec0, [rax]
				movd vecCoeff, [rbx + rdx*COMPV_YASM_UINT16_SZ_BYTES]  
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				inc rdx
				pmullw vec0, vecCoeff
				add rax, arg(argi_step)
				cmp rdx, arg(argi_kernSize)
				paddw vecSum0, vec0
				jl .LoopKernelSize_Per4Samples
				; EndOf_LoopKernelSize_Per4Samples ;

			mov rdx, arg(argi_outPtr)
			movq [rdx + i*COMPV_YASM_INT16_SZ_BYTES], vecSum0
			lea i, [i + 4]
			.EndOf_If_Per4Samples
			; EndOf_If_Per4Samples ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea rax, [width]
		cmp i, rax
		jge .EndOf_If_Per1Samples
		.If_Per1Samples:
			pxor vecSum0, vecSum0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i*COMPV_YASM_INT16_SZ_BYTES] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per1Samples:
				movq vec0, [rax]
				movd vecCoeff, [rbx + rdx*COMPV_YASM_UINT16_SZ_BYTES]  
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				inc rdx
				pmullw vec0, vecCoeff
				add rax, arg(argi_step)
				cmp rdx, arg(argi_kernSize)
				paddw vecSum0, vec0
				jl .LoopKernelSize_Per1Samples
				; EndOf_LoopKernelSize_Per1Samples ;

			mov rdx, arg(argi_outPtr)
			%assign index 0
			%rep 4
				%if index == 0
					movd rax, vecSum0
				%elif index == 2
					psrldq vecSum0, 4
					movd rax, vecSum0
				%endif
				mov [rdx + i*COMPV_YASM_INT16_SZ_BYTES], word ax
				inc i
				cmp i, width
				jge .EndOf_If_Per1Samples
				shr rax, 16
				%assign index index+1
			%endrep			
			.EndOf_If_Per1Samples
			; EndOf_If_Per1Samples ;
		
		mov rax, arg(argi_inPtr)
		mov rbx, arg(argi_outPtr)
		mov rdx, arg(argi_stride)
		dec j
		lea rax, [rax + rdx*COMPV_YASM_INT16_SZ_BYTES]
		lea rbx, [rbx + rdx*COMPV_YASM_INT16_SZ_BYTES]
		mov arg(argi_inPtr), rax
		mov arg(argi_outPtr), rbx
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
	%undef argi_stride

	%undef width
	%undef j
	%undef i

	;; begin epilog ;;
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
; arg(6) -> const uint16_t* vthzKernPtr
; arg(7) -> compv_uscalar_t kernSize
sym(CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	push rsi
	push rdi
	push rbx
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

	mov rax, arg(argi_width)
	mov rbx, arg(argi_pad)
	lea rax, [rax + rbx]
	%define argi_stride 5 ; argi_pad
	mov arg(argi_stride), rax ; stride = (pad + width)
	%undef argi_pad

	%define width	rcx
	%define j		rsi
	%define i		rdi
	mov j, arg(argi_height)
	mov width, arg(argi_width)

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
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per16Samples:
				movdqu vec0, [rax]
				movd vecCoeff, [rbx + rdx*COMPV_YASM_UINT16_SZ_BYTES]  
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				inc rdx
				movdqa vec1, vec0
				punpcklbw vec0, vecZero
				pmulhuw vec0, vecCoeff
				punpckhbw vec1, vecZero
				pmulhuw vec1, vecCoeff
				add rax, arg(argi_step)
				cmp rdx, arg(argi_kernSize)
				paddusw vecSum0, vec0
				paddusw vecSum1, vec1		
				jl .LoopKernelSize_Per16Samples
				; EndOf_LoopKernelSize_Per16Samples ;
			
			mov rdx, arg(argi_outPtr)
			lea rax, [width - 15]
			lea i, [i + 16]
			cmp i, rax
			packuswb vecSum0, vecSum1
			movdqu [rdx + i - 16], vecSum0
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
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per8Samples:
				movq vec0, [rax]
				movd vecCoeff, [rbx + rdx*COMPV_YASM_UINT16_SZ_BYTES]  
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				inc rdx
				add rax, arg(argi_step)
				punpcklbw vec0, vecZero
				pmulhuw vec0, vecCoeff
				cmp rdx, arg(argi_kernSize)
				paddusw vecSum0, vec0
				jl .LoopKernelSize_Per8Samples
				; EndOf_LoopKernelSize_Per8Samples ;

			mov rdx, arg(argi_outPtr)
			packuswb vecSum0, vecSum0
			movq [rdx + i], vecSum0
			lea i, [i + 8]
			.EndOf_If_Per8Samples
			; EndOf_If_Per8Samples ;


		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width -3)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea rax, [width - 3]
		cmp i, rax
		jge .EndOf_If_Per4Samples
		.If_Per4Samples:
			pxor vecSum0, vecSum0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per4Samples:
				movd vec0, [rax]
				movd vecCoeff, [rbx + rdx*COMPV_YASM_UINT16_SZ_BYTES]  
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				inc rdx
				add rax, arg(argi_step)
				punpcklbw vec0, vecZero
				pmulhuw vec0, vecCoeff
				cmp rdx, arg(argi_kernSize)
				paddusw vecSum0, vec0
				jl .LoopKernelSize_Per4Samples
				; EndOf_LoopKernelSize_Per4Samples ;

			mov rdx, arg(argi_outPtr)
			packuswb vecSum0, vecSum0
			movd [rdx + i], vecSum0
			lea i, [i + 4]
			.EndOf_If_Per4Samples
			; EndOf_If_Per4Samples ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea rax, [width]
		cmp i, rax
		jge .EndOf_If_Per1Samples
		.If_Per1Samples:
			pxor vecSum0, vecSum0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per1Samples:
				movd vec0, [rax]
				movd vecCoeff, [rbx + rdx*COMPV_YASM_UINT16_SZ_BYTES]  
				punpcklwd vecCoeff, vecCoeff
				pshufd vecCoeff, vecCoeff, 0
				inc rdx
				add rax, arg(argi_step)
				punpcklbw vec0, vecZero
				pmulhuw vec0, vecCoeff
				cmp rdx, arg(argi_kernSize)
				paddusw vecSum0, vec0
				jl .LoopKernelSize_Per1Samples
				; EndOf_LoopKernelSize_Per1Samples ;

			packuswb vecSum0, vecSum0
			mov rdx, arg(argi_outPtr)
			movd rax, vecSum0
			%assign index 0
			%rep 4
				mov [rdx + i], byte al
				inc i
				cmp i, width
				jge .EndOf_If_Per1Samples
				shr rax, 8
				%assign index index+1
			%endrep			
			.EndOf_If_Per1Samples
			; EndOf_If_Per1Samples ;
		
		mov rax, arg(argi_inPtr)
		mov rbx, arg(argi_outPtr)
		mov rdx, arg(argi_stride)
		dec j
		lea rax, [rax + rdx]
		lea rbx, [rbx + rdx]
		mov arg(argi_inPtr), rax
		mov arg(argi_outPtr), rbx
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
	%undef argi_stride

	%undef width
	%undef j
	%undef i

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

%endif
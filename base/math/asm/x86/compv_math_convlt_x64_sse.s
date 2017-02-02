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

global sym(CompVMathConvlt1VtHz_8u32f8u_Asm_X64_SSE2)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* inPtr
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

	%define vecZero				xmm8
	%define vecSum0				xmm9
	%define vecSum1				xmm5
	%define vecSum2				xmm6
	%define vecSum3				xmm7
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
		.LoopWidth_Per16Bytes:
			xorps vecSum0, vecSum0
			xorps vecSum1, vecSum1
			xorps vecSum2, vecSum2 
			xorps vecSum3, vecSum3
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i] ; rax = &inPtr[i]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per16Bytes:
				movdqu xmm0, [rax] ; xmm0 = vecInPtr
				add rax, step
				movss xmm4, [vthzKernPtr + rdx*COMPV_YASM_FLOAT32_SZ_BYTES]
				inc rdx
				shufps xmm4, xmm4, 0x0 ; xmm4 = vecCoeff
				movdqa xmm3, xmm0
				punpcklbw xmm0, vecZero
				punpckhbw xmm3, vecZero
				movdqa xmm1, xmm0				
				punpcklwd xmm0, vecZero
				punpckhwd xmm1, vecZero
				movdqa xmm2, xmm3
				punpcklwd xmm2, vecZero
				punpckhwd xmm3, vecZero
				cvtdq2ps xmm0, xmm0
				cvtdq2ps xmm1, xmm1
				cvtdq2ps xmm2, xmm2
				cvtdq2ps xmm3, xmm3
				mulps xmm0, xmm4
				mulps xmm1, xmm4
				mulps xmm2, xmm4
				mulps xmm3, xmm4
				addps vecSum0, xmm0
				addps vecSum1, xmm1
				addps vecSum2, xmm2
				addps vecSum3, xmm3
				cmp rdx, kernSize
				jl .LoopKernelSize_Per16Bytes
				; EndOf_LoopKernelSize_Per16Bytes ;
			
			cvttps2dq vecSum0, vecSum0
			cvttps2dq vecSum1, vecSum1
			cvttps2dq vecSum2, vecSum2
			cvttps2dq vecSum3, vecSum3
			packssdw vecSum0, vecSum1
			packssdw vecSum2, vecSum3
			packuswb vecSum0, vecSum2
			movdqu [outPtr + i], vecSum0
			lea i, [i + 16]
			cmp i, widthMinus15
			jl .LoopWidth_Per16Bytes
			; EndOf_LoopWidth_Per16Bytes ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width - 3; i += 4)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, widthMinus3
		jge .EndOf_LoopWidth_Per4Bytes
		.LoopWidth_Per4Bytes:
			xorps vecSum0, vecSum0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i] ; rax = &inPtr[i]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per4Bytes:
				movss xmm2, [rax] ; xmm2 = vecInPtr
				lea rax, [rax + step]
				movss xmm3, [vthzKernPtr + rdx*COMPV_YASM_FLOAT32_SZ_BYTES]
				inc rdx
				punpcklbw xmm2, vecZero
				punpcklwd xmm2, vecZero
				cvtdq2ps xmm2, xmm2
				shufps xmm3, xmm3, 0x0 ; xmm3 = vecCoeff
				mulps xmm2, xmm3
				cmp rdx, kernSize
				addps vecSum0, xmm2
				jl .LoopKernelSize_Per4Bytes
				; EndOf_LoopKernelSize_Per4Bytes ;
			
			cvttps2dq vecSum0, vecSum0
			lea i, [i + 4]
			packssdw vecSum0, vecSum0
			cmp i, widthMinus3
			packuswb vecSum0, vecSum0
			movd [outPtr + i - 4], vecSum0
			jl .LoopWidth_Per4Bytes
			.EndOf_LoopWidth_Per4Bytes
			; EndOf_LoopWidth_Per4Bytes ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width; i += 1)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_LoopWidth_Per1Bytes
		.LoopWidth_Per1Bytes:
			xorps vecSum0, vecSum0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i] ; rax = &inPtr[i]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per1Bytes:
				movzx octet, byte [rax]
				movss xmm3, [vthzKernPtr + rdx*COMPV_YASM_FLOAT32_SZ_BYTES] ; xmm3 = vecCoeff
				cvtsi2ss xmm2, octet
				cvtdq2ps xmm2, xmm2
				mulss xmm2, xmm3
				inc rdx
				lea rax, [rax + step]
				cmp rdx, kernSize
				addss vecSum0, xmm2	
				jl .LoopKernelSize_Per1Bytes
				; EndOf_LoopKernelSize_Per1Bytes ;

			inc i
			cvttss2si rax, vecSum0
			cmp i, width
			mov [outPtr + i - 1], byte al
			jl .LoopWidth_Per1Bytes
			.EndOf_LoopWidth_Per1Bytes
			; EndOf_LoopWidth_Per1Bytes ;
		
		dec j
		lea inPtr, [inPtr + stride]
		lea outPtr, [outPtr + stride]
		jnz .LoopHeight
		; EndOf_LoopHeight ;

	%undef vecZero
	%undef vecSum0
	%undef vecSum1
	%undef vecSum2
	%undef vecSum3

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
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret


%endif ; COMPV_YASM_ABI_IS_64BIT

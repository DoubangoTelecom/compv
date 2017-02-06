;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(CompVMathConvlt1VtHz_8u32f8u_Asm_X86_AVX2)
global sym(CompVMathConvlt1VtHz_8u32f8u_Asm_X86_FMA3_AVX2)

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
; %1 -> 0: FMA3 not supported, 1: FMA3 supported
%macro CompVMathConvlt1VtHz_8u32f8u_Macro_X86_AVX2 1
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_YMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 32, rax
	sub rsp, (2*32)

	%define vecZero				rsp + 0
	%define vecSum0				vecZero + 32
	vpxor ymm0, ymm0
	vmovdqa [vecZero], ymm0

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
		; for (i = 0; i < width - 31; i += 32)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		.LoopWidth_Per32Bytes:
			vxorps ymm5, ymm5 ; vecSum1
			vxorps ymm6, ymm6 ; vecSum2
			vxorps ymm7, ymm7 ; vecSum3
			vmovaps [vecSum0], ymm5
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per32Bytes:
				vmovups ymm0, [rax] ; ymm0 = vecInPtr
				vmovss xmm4, [rbx + rdx*COMPV_YASM_FLOAT32_SZ_BYTES]
				inc rdx
				add rax, arg(argi_step)
				vpunpcklbw ymm1, ymm0, [vecZero]
				vpunpckhbw ymm3, ymm0, [vecZero]
				vpunpcklwd ymm0, ymm1, [vecZero]
				vpunpckhwd ymm1, ymm1, [vecZero]
				vpunpcklwd ymm2, ymm3, [vecZero]
				vpunpckhwd ymm3, ymm3, [vecZero]
				vcvtdq2ps ymm0, ymm0
				vcvtdq2ps ymm1, ymm1
				vcvtdq2ps ymm2, ymm2
				vcvtdq2ps ymm3, ymm3
				vbroadcastss ymm4, xmm4
				cmp rdx, arg(argi_kernSize)
				%if %1
					vfmadd213ps ymm0, ymm4, [vecSum0]
					vfmadd231ps ymm5, ymm1, ymm4
					vfmadd231ps ymm6, ymm2, ymm4
					vfmadd231ps ymm7, ymm3, ymm4
				%else
					vmulps ymm0, ymm0, ymm4
					vmulps ymm1, ymm1, ymm4
					vmulps ymm2, ymm2, ymm4
					vmulps ymm3, ymm3, ymm4
					vaddps ymm0, ymm0, [vecSum0]
					vaddps ymm5, ymm5, ymm1
					vaddps ymm6, ymm6, ymm2
					vaddps ymm7, ymm7, ymm3
				%else
				%endif
				vmovups [vecSum0], ymm0
				jl .LoopKernelSize_Per32Bytes
				; EndOf_LoopKernelSize_Per32Bytes ;
			
			vcvttps2dq ymm0, ymm0
			vcvttps2dq ymm5, ymm5
			vcvttps2dq ymm6, ymm6
			vcvttps2dq ymm7, ymm7
			mov rdx, arg(argi_outPtr)
			lea rax, [width - 31]
			lea i, [i + 32]
			cmp i, rax
			vpackssdw ymm0, ymm0, ymm5
			vpackssdw ymm6, ymm6, ymm7
			vpackuswb ymm0, ymm0, ymm6
			vmovdqu [rdx + i - 32], ymm0
			jl .LoopWidth_Per32Bytes
			; EndOf_LoopWidth_Per32Bytes ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for for (; i < width - 7; i += 8)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea rax, [width - 7]
		cmp i, rax
		jge .EndOf_LoopWidth_Per8Bytes
		.LoopWidth_Per8Bytes:
			vxorps ymm0, ymm0, ymm0 ; vecSum0
			vpxor ymm1, ymm1, ymm1 ; vecZero
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per8Bytes:
				vmovdqu xmm2, [rax]
				vmovss xmm3, [rbx + rdx*COMPV_YASM_FLOAT32_SZ_BYTES]
				inc rdx
				vpmovzxbd ymm2, xmm2 ; ymm2 = vecInPtr
				add rax, arg(argi_step)
				vcvtdq2ps ymm2, ymm2
				vbroadcastss ymm3, xmm3
				%if %1
					cmp rdx, arg(argi_kernSize)
					vfmadd231ps ymm0, ymm2, ymm3
				%else
					vmulps ymm2, ymm3
					cmp rdx, arg(argi_kernSize)
					vaddps ymm0, ymm2
				%endif
				jl .LoopKernelSize_Per8Bytes
				; EndOf_LoopKernelSize_Per8Bytes ;


			mov rdx, arg(argi_outPtr)
			lea rax, [width - 7]
			lea i, [i + 8]
			cmp i, rax
			vcvttps2dq ymm0, ymm0
			vpackssdw ymm0, ymm0
			vpermq ymm0, ymm0, 0xD8
			vpackuswb ymm0, ymm0
			vmovq [rdx + i - 8], xmm0
			jl .LoopWidth_Per8Bytes
			.EndOf_LoopWidth_Per8Bytes
			; EndOf_LoopWidth_Per8Bytes ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width; i += 1)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_LoopWidth_Per1Bytes
		.LoopWidth_Per1Bytes:
			vxorps xmm0, xmm0 ; vecSum0
			vpxor xmm1, xmm1 ; ymm2 = vecZero
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per1Bytes:
				vmovss xmm2, [rax] ; TODO(dmi): use movzx for x64, here we cannot because we''re out of 32b registers
				vmovss xmm3, [rbx + rdx*COMPV_YASM_FLOAT32_SZ_BYTES] ; ymm3 = vecCoeff
				inc rdx
				vpunpcklbw xmm2, xmm1
				vpunpcklwd xmm2, xmm1
				vcvtdq2ps xmm2, xmm2
				add rax, arg(argi_step)
				%if %1
					cmp rdx, arg(argi_kernSize)
					vfmadd231ss xmm0, xmm2, xmm3
				%else
					vmulss xmm2, xmm3
					cmp rdx, arg(argi_kernSize)
					vaddss xmm0, xmm2
				%endif
				jl .LoopKernelSize_Per1Bytes
				; EndOf_LoopKernelSize_Per1Bytes ;


			inc i
			mov rdx, arg(argi_outPtr)
			cmp i, width
			vcvttss2si rax, xmm0
			mov [rdx + i - 1], byte al
			jl .LoopWidth_Per1Bytes
			.EndOf_LoopWidth_Per1Bytes
			; EndOf_LoopWidth_Per1Bytes ;
		
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
	add rsp, (2*32)
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
sym(CompVMathConvlt1VtHz_8u32f8u_Asm_X86_AVX2):
	CompVMathConvlt1VtHz_8u32f8u_Macro_X86_AVX2 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVMathConvlt1VtHz_8u32f8u_Asm_X86_FMA3_AVX2):
	CompVMathConvlt1VtHz_8u32f8u_Macro_X86_AVX2 1
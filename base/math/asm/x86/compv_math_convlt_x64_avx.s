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

global sym(CompVMathConvlt1VtHz_8u32f8u_Asm_X64_AVX2)
global sym(CompVMathConvlt1VtHz_8u32f8u_Asm_X64_FMA3_AVX2)


section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(AVX) const uint8_t* inPtr
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

	%define vecZero				ymm8
	%define vecSum0				ymm9
	%define vecSum1				ymm5
	%define vecSum2				ymm6
	%define vecSum3				ymm7
	vpxor vecZero, vecZero

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
	%define widthMinus31	r12
	%define widthMinus7		r13
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
		.LoopWidth_Per32Bytes:
			vxorps vecSum0, vecSum0
			vxorps vecSum1, vecSum1
			vxorps vecSum2, vecSum2 
			vxorps vecSum3, vecSum3
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i] ; rax = &inPtr[i]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per32Bytes:
				vmovups ymm0, [rax] ; ymm0 = vecInPtr
				add rax, step
				vmovss xmm4, [vthzKernPtr + rdx*COMPV_YASM_FLOAT32_SZ_BYTES]
				inc rdx
				vpunpcklbw ymm1, ymm0, vecZero
				vpunpckhbw ymm3, ymm0, vecZero
				vpunpcklwd ymm0, ymm1, vecZero
				vpunpckhwd ymm1, ymm1, vecZero
				vpunpcklwd ymm2, ymm3, vecZero
				vpunpckhwd ymm3, ymm3, vecZero
				vcvtdq2ps ymm0, ymm0
				vcvtdq2ps ymm1, ymm1
				vcvtdq2ps ymm2, ymm2
				vcvtdq2ps ymm3, ymm3
				vbroadcastss ymm4, xmm4 ; ymm4 = vecCoeff
				%if %1
					cmp rdx, kernSize
					vfmadd231ps vecSum0, ymm0, ymm4
					vfmadd231ps vecSum1, ymm1, ymm4
					vfmadd231ps vecSum2, ymm2, ymm4
					vfmadd231ps vecSum3, ymm3, ymm4
				%else
					vmulps ymm0, ymm0, ymm4
					vmulps ymm1, ymm1, ymm4
					vmulps ymm2, ymm2, ymm4
					vmulps ymm3, ymm3, ymm4
					cmp rdx, kernSize
					vaddps vecSum0, vecSum0, ymm0
					vaddps vecSum1, vecSum1, ymm1
					vaddps vecSum2, vecSum2, ymm2
					vaddps vecSum3, vecSum3, ymm3
				%endif
				jl .LoopKernelSize_Per32Bytes
				; EndOf_LoopKernelSize_Per32Bytes ;
			
			vcvttps2dq vecSum0, vecSum0
			vcvttps2dq vecSum1, vecSum1
			vcvttps2dq vecSum2, vecSum2
			vcvttps2dq vecSum3, vecSum3
			vpackssdw vecSum0, vecSum1
			vpackssdw vecSum2, vecSum3
			vpackuswb vecSum0, vecSum2
			vmovdqu [outPtr + i], vecSum0
			lea i, [i + 32]
			cmp i, widthMinus31
			jl .LoopWidth_Per32Bytes
			; EndOf_LoopWidth_Per32Bytes ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width - 7; i += 8)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, widthMinus7
		jge .EndOf_LoopWidth_Per8Bytes
		.LoopWidth_Per8Bytes:
			vxorps ymm0, ymm0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i] ; rax = &inPtr[i]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per8Bytes:
				vmovdqu xmm2, [rax] ; ymm2 = vecInPtr
				lea rax, [rax + step]
				vmovss xmm3, [vthzKernPtr + rdx*COMPV_YASM_FLOAT32_SZ_BYTES]
				vpmovzxbd ymm2, xmm2 ; ymm2 = vecInPtr
				inc rdx
				vcvtdq2ps ymm2, ymm2
				vbroadcastss ymm3, xmm3
				%if %1
					cmp rdx, kernSize
					vfmadd231ps ymm0, ymm2, ymm3
				%else
					vmulps ymm2, ymm3
					cmp rdx, kernSize
					vaddps ymm0, ymm2
				%endif
				jl .LoopKernelSize_Per8Bytes
				; EndOf_LoopKernelSize_Per8Bytes ;
			
			vcvttps2dq ymm0, ymm0
			lea i, [i + 8]
			vpackssdw ymm0, ymm0
			vpermq ymm0, ymm0, 0xD8
			cmp i, widthMinus7
			vpackuswb ymm0, ymm0
			vmovq [outPtr + i - 8], xmm0
			jl .LoopWidth_Per8Bytes
			.EndOf_LoopWidth_Per8Bytes
			; EndOf_LoopWidth_Per8Bytes ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width; i += 1)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_LoopWidth_Per1Bytes
		.LoopWidth_Per1Bytes:
			vxorps xmm0, xmm0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			lea rax, [inPtr + i] ; rax = &inPtr[i]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per1Bytes:
				movzx octet, byte [rax]
				vmovss xmm3, [vthzKernPtr + rdx*COMPV_YASM_FLOAT32_SZ_BYTES] ; ymm3 = vecCoeff
				vcvtsi2ss xmm2, octet
				%if %1
					inc rdx
					lea rax, [rax + step]
					vfmadd231ss xmm0, xmm2, xmm3
					cmp rdx, kernSize
				%else
					vmulss xmm2, xmm3
					inc rdx
					lea rax, [rax + step]
					cmp rdx, kernSize
					vaddss xmm0, xmm2
				%endif
				jl .LoopKernelSize_Per1Bytes
				; EndOf_LoopKernelSize_Per1Bytes ;

			inc i
			vcvttss2si rax, xmm0
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


%endif ; COMPV_YASM_ABI_IS_64BIT

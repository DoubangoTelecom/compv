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

global sym(CompVMathConvlt1VtHz_8u32f8u_Asm_X86_AVX2)
global sym(CompVMathConvlt1VtHz_8u32f8u_Asm_X86_FMA3_AVX2)
global sym(CompVMathConvlt1VtHz_8u16s16s_Asm_X86_AVX2)
global sym(CompVMathConvlt1VtHz_16s16s16s_Asm_X86_AVX2)
global sym(CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_X86_AVX2)

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
		.LoopWidth_Per32Samples:
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
			.LoopKernelSize_Per32Samples:
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
				%endif
				vmovups [vecSum0], ymm0
				jl .LoopKernelSize_Per32Samples
				; EndOf_LoopKernelSize_Per32Samples ;
			
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
			jl .LoopWidth_Per32Samples
			; EndOf_LoopWidth_Per32Samples ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for for (; i < width - 7; i += 8)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea rax, [width - 7]
		cmp i, rax
		jge .EndOf_LoopWidth_Per8Samples
		.LoopWidth_Per8Samples:
			vxorps ymm0, ymm0, ymm0 ; vecSum0
			vpxor ymm1, ymm1, ymm1 ; vecZero
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per8Samples:
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
				jl .LoopKernelSize_Per8Samples
				; EndOf_LoopKernelSize_Per8Samples ;


			mov rdx, arg(argi_outPtr)
			lea rax, [width - 7]
			lea i, [i + 8]
			cmp i, rax
			vcvttps2dq ymm0, ymm0
			vpackssdw ymm0, ymm0
			vpermq ymm0, ymm0, 0xD8
			vpackuswb ymm0, ymm0
			vmovq [rdx + i - 8], xmm0
			jl .LoopWidth_Per8Samples
			.EndOf_LoopWidth_Per8Samples
			; EndOf_LoopWidth_Per8Samples ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width; i += 1)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_LoopWidth_Per1Samples
		.LoopWidth_Per1Samples:
			vxorps xmm0, xmm0 ; vecSum0
			vpxor xmm1, xmm1 ; ymm2 = vecZero
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per1Samples:
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
				jl .LoopKernelSize_Per1Samples
				; EndOf_LoopKernelSize_Per1Samples ;


			inc i
			mov rdx, arg(argi_outPtr)
			cmp i, width
			vcvttss2si rax, xmm0
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


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* inPtr
; arg(1) -> int16_t* outPtr
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> compv_uscalar_t step
; arg(5) -> compv_uscalar_t pad
; arg(6) -> const int16_t* vthzKernPtr
; arg(7) -> compv_uscalar_t kernSize
sym(CompVMathConvlt1VtHz_8u16s16s_Asm_X86_AVX2):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	push rsi
	push rdi
	push rbx
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
		.LoopWidth_Per32Samples:
			vpxor vecSum0, vecSum0
			vpxor vecSum1, vecSum1
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per32Samples:
				vmovdqu vec0, [rax]
				vmovd vecCoeffn, [rbx + rdx*COMPV_YASM_INT16_SZ_BYTES]
				vpbroadcastw vecCoeff, vecCoeffn
				vpunpckhbw vec1, vec0, vecZero
				vpmullw vec1, vec1, vecCoeff
				vpunpcklbw vec0, vec0, vecZero
				inc rdx
				vpmullw vec0, vec0, vecCoeff
				add rax, arg(argi_step)
				cmp rdx, arg(argi_kernSize)
				vpaddw vecSum1, vecSum1, vec1
				vpaddw vecSum0, vecSum0, vec0
				jl .LoopKernelSize_Per32Samples
				; EndOf_LoopKernelSize_Per32Samples ;
			
			vperm2i128 vec0, vecSum0, vecSum1, 0x20
			vperm2i128 vec1, vecSum0, vecSum1, 0x31
			mov rdx, arg(argi_outPtr)
			lea rax, [width - 31]
			vmovdqu [rdx + (i + 0)*COMPV_YASM_INT16_SZ_BYTES], vec0
			vmovdqu [rdx + (i + 16)*COMPV_YASM_INT16_SZ_BYTES], vec1
			lea i, [i + 32]
			cmp i, rax
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
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per16Samples:
				vmovdqu vec0n, [rax]
				vmovd vecCoeffn, [rbx + rdx*COMPV_YASM_INT16_SZ_BYTES]
				vpunpcklwd vecCoeffn, vecCoeffn
				vpshufd vecCoeffn, vecCoeffn, 0
				vpunpckhbw vec1n, vec0n, vecZeron
				vpmullw vec1n, vec1n, vecCoeffn
				vpunpcklbw vec0n, vec0n, vecZeron
				inc rdx
				vpmullw vec0n, vec0n, vecCoeffn
				add rax, arg(argi_step)
				cmp rdx, arg(argi_kernSize)
				vpaddw vecSum1n, vecSum1n, vec1n
				vpaddw vecSum0n, vecSum0n, vec0n
				jl .LoopKernelSize_Per16Samples
				; EndOf_LoopKernelSize_Per16Samples ;
			mov rdx, arg(argi_outPtr)
			vmovdqu [rdx + (i + 0)*COMPV_YASM_INT16_SZ_BYTES], vecSum0n
			vmovdqu [rdx + (i + 8)*COMPV_YASM_INT16_SZ_BYTES], vecSum1n
			lea i, [i + 16]
			.EndOf_If_Per16Samples
			; EndOf_If_Per16Samples ;


		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; i < width - 7
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea rax, [width - 7]
		cmp i, rax
		jge .EndOf_If_Per8Samples
		.If_Per8Samples:
			vpxor vecSum0n, vecSum0n, vecSum0n
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per8Samples:
				vmovq vec0n, [rax]
				vmovd vecCoeffn, [rbx + rdx*COMPV_YASM_INT16_SZ_BYTES]
				vpunpcklwd vecCoeffn, vecCoeffn
				vpshufd vecCoeffn, vecCoeffn, 0
				vpunpcklbw vec0n, vec0n, vecZeron
				inc rdx
				vpmullw vec0n, vec0n, vecCoeffn
				add rax, arg(argi_step)
				cmp rdx, arg(argi_kernSize)
				vpaddw vecSum0n, vecSum0n, vec0n
				jl .LoopKernelSize_Per8Samples
				; EndOf_LoopKernelSize_Per8Samples ;

			mov rdx, arg(argi_outPtr)
			vmovdqu [rdx + i*COMPV_YASM_INT16_SZ_BYTES], vecSum0n
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
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per4Samples:
				vmovd vec0n, [rax]
				vmovd vecCoeffn, [rbx + rdx*COMPV_YASM_INT16_SZ_BYTES]
				vpunpcklwd vecCoeffn, vecCoeffn
				vpshufd vecCoeffn, vecCoeffn, 0
				vpunpcklbw vec0n, vec0n, vecZeron
				inc rdx
				vpmullw vec0n, vec0n, vecCoeffn
				add rax, arg(argi_step)
				cmp rdx, arg(argi_kernSize)
				vpaddw vecSum0n, vecSum0n, vec0n
				jl .LoopKernelSize_Per4Samples
				; EndOf_LoopKernelSize_Per4Samples ;

			mov rdx, arg(argi_outPtr)
			vmovq [rdx + i*COMPV_YASM_INT16_SZ_BYTES], vecSum0n
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
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per1Samples:
				vmovd vec0n, [rax]
				vmovd vecCoeffn, [rbx + rdx*COMPV_YASM_INT16_SZ_BYTES]
				vpunpcklwd vecCoeffn, vecCoeffn
				vpshufd vecCoeffn, vecCoeffn, 0
				vpunpcklbw vec0n, vec0n, vecZeron
				inc rdx
				vpmullw vec0n, vec0n, vecCoeffn
				add rax, arg(argi_step)
				cmp rdx, arg(argi_kernSize)
				vpaddw vecSum0n, vecSum0n, vec0n
				jl .LoopKernelSize_Per1Samples
				; EndOf_LoopKernelSize_Per1Samples ;
			
			mov rdx, arg(argi_outPtr)
			%assign index 0
			%rep 4
				%if index == 0
					vmovd eax, vecSum0n
				%elif index == 2
					vpsrldq vecSum0n, 4
					vmovd eax, vecSum0n
				%endif
				mov [rdx + i*COMPV_YASM_INT16_SZ_BYTES], word ax
				inc i
				cmp i, width
				jge .EndOf_If_Per1Samples
				shr eax, 16
				%assign index index+1
			%endrep
			.EndOf_If_Per1Samples
			; EndOf_If_Per1Samples ;

		
		mov rax, arg(argi_inPtr)
		mov rbx, arg(argi_outPtr)
		mov rdx, arg(argi_stride)
		dec j
		lea rax, [rax + rdx*COMPV_YASM_UINT8_SZ_BYTES]
		lea rbx, [rbx + rdx*COMPV_YASM_INT16_SZ_BYTES]
		mov arg(argi_inPtr), rax
		mov arg(argi_outPtr), rbx
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
sym(CompVMathConvlt1VtHz_16s16s16s_Asm_X86_AVX2):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	push rsi
	push rdi
	push rbx
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
	lea rax, [rax*COMPV_YASM_INT16_SZ_BYTES]
	mov arg(argi_step), rax

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
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i*COMPV_YASM_INT16_SZ_BYTES] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per32Samples:
				vmovdqu vec0, [rax]
				vmovdqu vec1, [rax + 16*COMPV_YASM_INT16_SZ_BYTES]
				vmovd vecCoeffn, [rbx + rdx*COMPV_YASM_INT16_SZ_BYTES]
				vpbroadcastw vecCoeff, vecCoeffn
				vpmullw vec0, vec0, vecCoeff
				inc rdx
				vpmullw vec1, vec1, vecCoeff
				add rax, arg(argi_step)
				cmp rdx, arg(argi_kernSize)
				vpaddw vecSum0, vecSum0, vec0
				vpaddw vecSum1, vecSum1, vec1
				jl .LoopKernelSize_Per32Samples
				; EndOf_LoopKernelSize_Per32Samples ;
			
			mov rdx, arg(argi_outPtr)
			lea rax, [width - 31]
			vmovdqu [rdx + (i + 0)*COMPV_YASM_INT16_SZ_BYTES], vecSum0
			vmovdqu [rdx + (i + 16)*COMPV_YASM_INT16_SZ_BYTES], vecSum1
			lea i, [i + 32]
			cmp i, rax
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
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i*COMPV_YASM_INT16_SZ_BYTES] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per16Samples:
				vmovdqu vec0, [rax]
				vmovd vecCoeffn, [rbx + rdx*COMPV_YASM_INT16_SZ_BYTES]
				vpbroadcastw vecCoeff, vecCoeffn
				inc rdx
				vpmullw vec0, vec0, vecCoeff
				add rax, arg(argi_step)
				cmp rdx, arg(argi_kernSize)
				vpaddw vecSum0, vecSum0, vec0
				jl .LoopKernelSize_Per16Samples
				; EndOf_LoopKernelSize_Per16Samples ;
			mov rdx, arg(argi_outPtr)
			vmovdqu [rdx + i*COMPV_YASM_INT16_SZ_BYTES], vecSum0
			lea i, [i + 16]
			.EndOf_If_Per16Samples
			; EndOf_If_Per16Samples ;


		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; i < width - 7
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea rax, [width - 7]
		cmp i, rax
		jge .EndOf_If_Per8Samples
		.If_Per8Samples:
			vpxor vecSum0n, vecSum0n, vecSum0n
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i*COMPV_YASM_INT16_SZ_BYTES] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per8Samples:
				vmovdqu vec0n, [rax]
				vmovd vecCoeffn, [rbx + rdx*COMPV_YASM_INT16_SZ_BYTES]
				vpunpcklwd vecCoeffn, vecCoeffn
				vpshufd vecCoeffn, vecCoeffn, 0
				inc rdx
				vpmullw vec0n, vec0n, vecCoeffn
				add rax, arg(argi_step)
				cmp rdx, arg(argi_kernSize)
				vpaddw vecSum0n, vecSum0n, vec0n
				jl .LoopKernelSize_Per8Samples
				; EndOf_LoopKernelSize_Per8Samples ;

			mov rdx, arg(argi_outPtr)
			vmovdqu [rdx + i*COMPV_YASM_INT16_SZ_BYTES], vecSum0n
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
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i*COMPV_YASM_INT16_SZ_BYTES] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per4Samples:
				vmovq vec0n, [rax]
				vmovd vecCoeffn, [rbx + rdx*COMPV_YASM_INT16_SZ_BYTES]
				vpunpcklwd vecCoeffn, vecCoeffn
				vpshufd vecCoeffn, vecCoeffn, 0
				inc rdx
				vpmullw vec0n, vec0n, vecCoeffn
				add rax, arg(argi_step)
				cmp rdx, arg(argi_kernSize)
				vpaddw vecSum0n, vecSum0n, vec0n
				jl .LoopKernelSize_Per4Samples
				; EndOf_LoopKernelSize_Per4Samples ;

			mov rdx, arg(argi_outPtr)
			vmovq [rdx + i*COMPV_YASM_INT16_SZ_BYTES], vecSum0n
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
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i*COMPV_YASM_INT16_SZ_BYTES] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per1Samples:
				vmovq vec0n, [rax]
				vmovd vecCoeffn, [rbx + rdx*COMPV_YASM_INT16_SZ_BYTES]
				vpunpcklwd vecCoeffn, vecCoeffn
				vpshufd vecCoeffn, vecCoeffn, 0
				inc rdx
				vpmullw vec0n, vec0n, vecCoeffn
				add rax, arg(argi_step)
				cmp rdx, arg(argi_kernSize)
				vpaddw vecSum0n, vecSum0n, vec0n
				jl .LoopKernelSize_Per1Samples
				; EndOf_LoopKernelSize_Per1Samples ;
			
			mov rdx, arg(argi_outPtr)
			%assign index 0
			%rep 4
				%if index == 0
					vmovd eax, vecSum0n
				%elif index == 2
					vpsrldq vecSum0n, 4
					vmovd eax, vecSum0n
				%endif
				mov [rdx + i*COMPV_YASM_INT16_SZ_BYTES], word ax
				inc i
				cmp i, width
				jge .EndOf_If_Per1Samples
				shr eax, 16
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
	vzeroupper
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
sym(CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_X86_AVX2):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	push rsi
	push rdi
	push rbx
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
		.LoopWidth_Per32Samples:
			vpxor vecSum0, vecSum0
			vpxor vecSum1, vecSum1
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per32Samples:
				vmovups vec0, [rax]
				vmovd vecCoeffn, [rbx + rdx*COMPV_YASM_UINT16_SZ_BYTES]
				vpbroadcastw vecCoeff, vecCoeffn
				vpunpckhbw vec1, vec0, vecZero
				vpmulhuw vec1, vec1, vecCoeff
				vpunpcklbw vec0, vec0, vecZero
				inc rdx
				vpmulhuw vec0, vec0, vecCoeff
				add rax, arg(argi_step)
				cmp rdx, arg(argi_kernSize)
				vpaddusw vecSum1, vecSum1, vec1
				vpaddusw vecSum0, vecSum0, vec0
				jl .LoopKernelSize_Per32Samples
				; EndOf_LoopKernelSize_Per32Samples ;
			
			vpackuswb vecSum0, vecSum0, vecSum1
			mov rdx, arg(argi_outPtr)
			lea rax, [width - 31]
			vmovdqu [rdx + i], vecSum0
			lea i, [i + 32]
			cmp i, rax
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
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per16Samples:
				vmovups vec0n, [rax]
				vmovd vecCoeffn, [rbx + rdx*COMPV_YASM_UINT16_SZ_BYTES]
				vpunpcklwd vecCoeffn, vecCoeffn
				vpshufd vecCoeffn, vecCoeffn, 0
				vpunpckhbw vec1n, vec0n, vecZeron
				vpmulhuw vec1n, vec1n, vecCoeffn
				vpunpcklbw vec0n, vec0n, vecZeron
				inc rdx
				vpmulhuw vec0n, vec0n, vecCoeffn
				add rax, arg(argi_step)
				cmp rdx, arg(argi_kernSize)
				vpaddusw vecSum1n, vecSum1n, vec1n
				vpaddusw vecSum0n, vecSum0n, vec0n
				jl .LoopKernelSize_Per16Samples
				; EndOf_LoopKernelSize_Per16Samples ;

			vpackuswb vecSum0n, vecSum0n, vecSum1n
			mov rdx, arg(argi_outPtr)
			vmovdqu [rdx + i], vecSum0n
			lea i, [i + 16]
			.EndOf_If_Per16Samples
			; EndOf_If_Per16Samples ;


		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; i < width - 7
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea rax, [width - 7]
		cmp i, rax
		jge .EndOf_If_Per8Samples
		.If_Per8Samples:
			vpxor vecSum0n, vecSum0n, vecSum0n
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (row = 0, k = 0; row < kernSize; ++row, k += step)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per8Samples:
				vmovq vec0n, [rax]
				vmovd vecCoeffn, [rbx + rdx*COMPV_YASM_UINT16_SZ_BYTES]
				vpunpcklwd vecCoeffn, vecCoeffn
				vpshufd vecCoeffn, vecCoeffn, 0
				vpunpcklbw vec0n, vec0n, vecZeron
				inc rdx
				vpmulhuw vec0n, vec0n, vecCoeffn
				add rax, arg(argi_step)
				cmp rdx, arg(argi_kernSize)
				vpaddusw vecSum0n, vecSum0n, vec0n
				jl .LoopKernelSize_Per8Samples
				; EndOf_LoopKernelSize_Per8Samples ;

			vpackuswb vecSum0n, vecSum0n, vecSum0n
			mov rdx, arg(argi_outPtr)
			vmovq [rdx + i], vecSum0n
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
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per4Samples:
				vmovd vec0n, [rax]
				vmovd vecCoeffn, [rbx + rdx*COMPV_YASM_UINT16_SZ_BYTES]
				vpunpcklwd vecCoeffn, vecCoeffn
				vpshufd vecCoeffn, vecCoeffn, 0
				vpunpcklbw vec0n, vec0n, vecZeron
				inc rdx
				vpmulhuw vec0n, vec0n, vecCoeffn
				add rax, arg(argi_step)
				cmp rdx, arg(argi_kernSize)
				vpaddusw vecSum0n, vecSum0n, vec0n
				jl .LoopKernelSize_Per4Samples
				; EndOf_LoopKernelSize_Per4Samples ;

			vpackuswb vecSum0n, vecSum0n, vecSum0n
			mov rdx, arg(argi_outPtr)
			vmovd [rdx + i], vecSum0n
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
			mov rbx, arg(argi_inPtr)
			lea rax, [rbx + i] ; rax = &inPtr[i]
			mov rbx, arg(argi_vthzKernPtr) ; rbx = &vthzKernPtr[0]
			xor rdx, rdx ; rdx = row = 0
			.LoopKernelSize_Per1Samples:
				vmovd vec0n, [rax]
				vmovd vecCoeffn, [rbx + rdx*COMPV_YASM_UINT16_SZ_BYTES]
				vpunpcklwd vecCoeffn, vecCoeffn
				vpshufd vecCoeffn, vecCoeffn, 0
				vpunpcklbw vec0n, vec0n, vecZeron
				inc rdx
				vpmulhuw vec0n, vec0n, vecCoeffn
				add rax, arg(argi_step)
				cmp rdx, arg(argi_kernSize)
				vpaddusw vecSum0n, vecSum0n, vec0n
				jl .LoopKernelSize_Per1Samples
				; EndOf_LoopKernelSize_Per1Samples ;

			vpackuswb vecSum0n, vecSum0n, vecSum0n
			vmovd eax, vecSum0n
			mov rdx, arg(argi_outPtr)
			%assign index 0
			%rep 4
				mov [rdx + i], byte al
				inc i
				cmp i, width
				jge .EndOf_If_Per1Samples
				shr eax, 8
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
	vzeroupper
	ret
%endif
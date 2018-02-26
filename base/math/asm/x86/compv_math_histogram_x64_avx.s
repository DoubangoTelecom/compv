;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2032-2018 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(CompVMathHistogramBuildProjectionY_8u32s_Asm_X64_AVX2)

section .data
	
section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(AVX) const uint8_t* ptrIn
; arg(1) -> COMPV_ALIGNED(AVX) int32_t* ptrOut
; arg(2) -> const compv_uscalar_t width
; arg(3) -> const compv_uscalar_t height
; arg(4) -> COMPV_ALIGNED(AVX) const compv_uscalar_t stride
sym(CompVMathHistogramBuildProjectionY_8u32s_Asm_X64_AVX2)
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	COMPV_YASM_SAVE_YMM 8
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	%define ptrIn			rax
	%define ptrOut			rcx
	%define width			rdx
	%define height			rsi
	%define stride			rdi
	%define sum				rbx
	%define sumd			ebx
	%define i				r8
	%define width32			r9
	%define tmp				r10

	%define vec0			ymm0
	%define vec0n			xmm0
	%define vec1			ymm1
	%define vec1n			xmm1
	%define vec2			ymm2
	%define vec3			ymm3
	%define vec4			ymm4
	%define vec5			ymm5
	%define vec6			ymm6
	%define vec7			ymm7
	%define vecZero			ymm8

	mov ptrIn, arg(0)
	mov ptrOut, arg(1)
	mov width, arg(2)
	mov height, arg(3)
	mov stride, arg(4)
	mov width32, width
	and width32, -32

	vpxor vecZero, vecZero

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		; int32_t <- uint8_t
		vmovdqa vec1, [ptrIn + 0*COMPV_YASM_UINT8_SZ_BYTES]
		vpunpckhbw vec3, vec1, vecZero
		vpunpcklbw vec1, vec1, vecZero
		vpunpcklwd vec0, vec1, vecZero
		vpunpckhwd vec1, vec1, vecZero
		vpunpcklwd vec2, vec3, vecZero
		vpunpckhwd vec3, vec3, vecZero
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 32; i < width32; i += 32)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp width32, 32
		mov i, 32
		jle .EndOf_LoopWidth32
		.LoopWidth32:
			vmovdqa vec5, [ptrIn + (i*COMPV_YASM_UINT8_SZ_BYTES)]
			vpunpckhbw vec7, vec5, vecZero
			vpunpcklbw vec5, vec5, vecZero
			vpunpcklwd vec4, vec5, vecZero
			vpunpckhwd vec5, vec5, vecZero
			vpunpcklwd vec6, vec7, vecZero
			vpunpckhwd vec7, vec7, vecZero
			vpaddd vec0, vec4
			vpaddd vec1, vec5
			vpaddd vec2, vec6
			vpaddd vec3, vec7
			add i, 32
			cmp i, width32
			jl .LoopWidth32
		.EndOf_LoopWidth32:

		vpaddd vec0, vec0, vec2
		vpaddd vec1, vec1, vec3
		vphaddd vec0, vec0, vec1
		vextractf128 vec1n, vec0, 0x1
		vpaddd vec0n, vec0n, vec1n

		vpshufd vec1n, vec0n, 0xE
		vpaddd vec0n, vec0n, vec1n
		vpshufd vec1n, vec0n, 0x1
		vpaddd vec0n, vec0n, vec1n
		vmovd sumd, vec0n

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width; ++i)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_LoopWidth1
		.LoopWidth1:
			movzx tmp, byte [ptrIn + (i*COMPV_YASM_UINT8_SZ_BYTES)]
			inc i
			add sum, tmp
			cmp i, width
			jl .LoopWidth1
		.EndOf_LoopWidth1:

		dec height
		lea ptrIn, [ptrIn + stride*COMPV_YASM_UINT8_SZ_BYTES]
		mov dword [ptrOut], sumd
		lea ptrOut, [ptrOut + 1*COMPV_YASM_INT32_SZ_BYTES]
		jnz .LoopHeight
	.EndOf_LoopHeight:

	%undef ptrIn			
	%undef ptrOut			
	%undef width			
	%undef height			
	%undef stride			
	%undef sum				
	%undef sumd			
	%undef i				
	%undef width16			
	%undef tmp				

	%undef vec0
	%undef vec0n
	%undef vec1
	%undef vec1n	
	%undef vec2			
	%undef vec3			
	%undef vec4			
	%undef vec5			
	%undef vec6			
	%undef vec7			
	%undef vecZero

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

%endif ; COMPV_YASM_ABI_IS_64BIT

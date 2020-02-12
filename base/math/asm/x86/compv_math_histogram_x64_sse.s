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

global sym(CompVMathHistogramProcess_8u32s_Asm_X64_SSE2)
global sym(CompVMathHistogramBuildProjectionX_8u32s_Asm_X64_SSE2)
global sym(CompVMathHistogramBuildProjectionY_8u32s_Asm_X64_SSE2)

section .data
	
section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; !!! MUST NOT USE !!!
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* dataPtr
; arg(1) -> compv_uscalar_t width
; arg(2) -> compv_uscalar_t height
; arg(3) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
; arg(4) -> uint32_t *histogram0
sym(CompVMathHistogramProcess_8u32s_Asm_X64_SSE2)
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	COMPV_YASM_SAVE_XMM 15
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
	sub rsp, 3*(256*COMPV_YASM_UINT32_SZ_BYTES)
	%define histogram1	rsp + 0
	%define histogram2	histogram1 + (256*COMPV_YASM_UINT32_SZ_BYTES)
	%define histogram3	histogram2 + (256*COMPV_YASM_UINT32_SZ_BYTES)

	; Set memmory to zeros
	lea rdi, [rsp + 0] ; dest
	xor rax, rax ; zero
	mov rcx, (3*128)
	rep stosq

	%define dataPtr			rdi
	%define width			rsi 
	%define height			rbx
	%define stride			rax
	%define	histogram0		rdx
	%define i				rcx
	%define maxWidthStep1	r8

	mov dataPtr, arg(0)
	mov width, arg(1)
	mov height, arg(2)
	mov stride, arg(3)
	mov histogram0, arg(4)

	mov maxWidthStep1, width
	and maxWidthStep1, -4

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		xor i, i
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < maxWidthStep1; i += 4)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopWidth4:
			movzx r9, byte [dataPtr + i + 0]
			movzx r10, byte [dataPtr + i + 1]
			movzx r11, byte [dataPtr + i + 2]
			movzx r12, byte [dataPtr + i + 3]
			add i, 4
			inc dword ptr [histogram0 + r9*COMPV_YASM_UINT32_SZ_BYTES]
			inc dword ptr [histogram0 + r10*COMPV_YASM_UINT32_SZ_BYTES]
			inc dword ptr [histogram0 + r11*COMPV_YASM_UINT32_SZ_BYTES]
			inc dword ptr [histogram0 + r12*COMPV_YASM_UINT32_SZ_BYTES]
			cmp i, maxWidthStep1
			jl .LoopWidth4
			;; EndOf_LoopWidth4 ;;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width; ++i)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_LoopWidth1
		.LoopWidth1:
			movzx r9, byte [dataPtr + i + 0]
			inc i
			lea r9, [histogram0 + r9*COMPV_YASM_UINT32_SZ_BYTES]
			inc dword ptr [r9]
			cmp i, width
			jl .LoopWidth1
			.EndOf_LoopWidth1:
			;; EndOf_LoopWidth1 ;;

		dec height
		lea dataPtr, [dataPtr + stride]
		jnz .LoopHeight
		;; EndOf_LoopHeight ;;

	; free memory and unalign stack
	add rsp, 3*(256*COMPV_YASM_UINT32_SZ_BYTES)
	COMPV_YASM_UNALIGN_STACK
	%undef histogram1
	%undef histogram2
	%undef histogram3

	%undef dataPtr			
	%undef width			 
	%undef height			
	%undef stride			
	%undef	histogram0		
	%undef i
	%undef maxWidthStep1			

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
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* ptrIn
; arg(1) -> COMPV_ALIGNED(SSE) int32_t* ptrOut
; arg(2) -> const compv_uscalar_t width
; arg(3) -> const compv_uscalar_t height
; arg(4) -> COMPV_ALIGNED(SSE) const compv_uscalar_t stride
sym(CompVMathHistogramBuildProjectionX_8u32s_Asm_X64_SSE2)
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
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
	%define width16			r9

	%define vec0			xmm0
	%define vec1			xmm1
	%define vec2			xmm2
	%define vec3			xmm3
	%define vecZero			xmm4

	mov ptrIn, arg(0)
	mov ptrOut, arg(1)
	mov width, arg(2)
	mov height, arg(3)
	mov stride, arg(4)
	mov width16, width
	and width16, -16

	pxor vecZero, vecZero

	; Copy first row (to avoid using memset(0))
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (i = 0; i < width16; i += 16)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	xor i, i
	.MemsetLoopWidth16:
		movdqa vec2, [ptrIn + (i*COMPV_YASM_UINT8_SZ_BYTES)]
		movdqa vec0, vec2
		punpcklbw vec0, vecZero
		punpckhbw vec2, vecZero
		movdqa vec1, vec0
		movdqa vec3, vec2
		punpcklwd vec0, vecZero
		punpckhwd vec1, vecZero
		punpcklwd vec2, vecZero
		punpckhwd vec3, vecZero
		movdqa [ptrOut + ((i+0)*COMPV_YASM_INT32_SZ_BYTES)], vec0
		movdqa [ptrOut + ((i+4)*COMPV_YASM_INT32_SZ_BYTES)], vec1
		movdqa [ptrOut + ((i+8)*COMPV_YASM_INT32_SZ_BYTES)], vec2
		movdqa [ptrOut + ((i+12)*COMPV_YASM_INT32_SZ_BYTES)], vec3
		add i, 16
		cmp i, width16
		jl .MemsetLoopWidth16
	.EndOf_MemsetLoopWidth16:

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (; i < width; ++i)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp i, width
	jge .EndOf_MemsetLoopWidth1
	.MemsetLoopWidth1:
		movzx sumd, byte [ptrIn + (i*COMPV_YASM_UINT8_SZ_BYTES)]
		mov [ptrOut + ((i)*COMPV_YASM_INT32_SZ_BYTES)], dword sumd
		inc i
		cmp i, width
		jl .MemsetLoopWidth1
	.EndOf_MemsetLoopWidth1:

	; Move to next row ;
	dec height
	jz .EndOf_LoopHeight
	lea ptrIn, [ptrIn + stride*COMPV_YASM_UINT8_SZ_BYTES]

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 1; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width16; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		.LoopWidth16:
			movdqa vec2, [ptrIn + (i*COMPV_YASM_UINT8_SZ_BYTES)]
			movdqa vec0, vec2
			punpcklbw vec0, vecZero
			punpckhbw vec2, vecZero
			movdqa vec1, vec0
			movdqa vec3, vec2
			punpcklwd vec0, vecZero
			punpckhwd vec1, vecZero
			punpcklwd vec2, vecZero
			punpckhwd vec3, vecZero
			paddd vec0, [ptrOut + ((i+0)*COMPV_YASM_INT32_SZ_BYTES)]
			paddd vec1, [ptrOut + ((i+4)*COMPV_YASM_INT32_SZ_BYTES)]
			paddd vec2, [ptrOut + ((i+8)*COMPV_YASM_INT32_SZ_BYTES)]
			paddd vec3, [ptrOut + ((i+12)*COMPV_YASM_INT32_SZ_BYTES)]
			movdqa [ptrOut + ((i+0)*COMPV_YASM_INT32_SZ_BYTES)], vec0
			movdqa [ptrOut + ((i+4)*COMPV_YASM_INT32_SZ_BYTES)], vec1
			movdqa [ptrOut + ((i+8)*COMPV_YASM_INT32_SZ_BYTES)], vec2
			movdqa [ptrOut + ((i+12)*COMPV_YASM_INT32_SZ_BYTES)], vec3
			add i, 16
			cmp i, width16
			jl .LoopWidth16
		.EndOf_LoopWidth16:

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width; ++i)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_LoopWidth1
		.LoopWidth1:
			movzx sumd, byte [ptrIn + (i*COMPV_YASM_UINT8_SZ_BYTES)]
			add sumd, dword [ptrOut + ((i)*COMPV_YASM_INT32_SZ_BYTES)]
			mov [ptrOut + ((i)*COMPV_YASM_INT32_SZ_BYTES)], dword sumd
			inc i
			cmp i, width
			jl .LoopWidth1
		.EndOf_LoopWidth1:

		dec height
		lea ptrIn, [ptrIn + stride*COMPV_YASM_UINT8_SZ_BYTES]
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

	%undef vec0			
	%undef vec1			
	%undef vec2			
	%undef vec3					
	%undef vecZero			

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* ptrIn
; arg(1) -> COMPV_ALIGNED(SSE) int32_t* ptrOut
; arg(2) -> const compv_uscalar_t width
; arg(3) -> const compv_uscalar_t height
; arg(4) -> COMPV_ALIGNED(SSE) const compv_uscalar_t stride
sym(CompVMathHistogramBuildProjectionY_8u32s_Asm_X64_SSE2)
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	COMPV_YASM_SAVE_XMM 8
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
	%define width16			r9
	%define tmp				r10

	%define vec0			xmm0
	%define vec1			xmm1
	%define vec2			xmm2
	%define vec3			xmm3
	%define vec4			xmm4
	%define vec5			xmm5
	%define vec6			xmm6
	%define vec7			xmm7
	%define vecZero			xmm8

	mov ptrIn, arg(0)
	mov ptrOut, arg(1)
	mov width, arg(2)
	mov height, arg(3)
	mov stride, arg(4)
	mov width16, width
	and width16, -16

	pxor vecZero, vecZero

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		; int32_t <- uint8_t
		movdqa vec1, [ptrIn + 0*COMPV_YASM_UINT8_SZ_BYTES]
		movdqa vec3, vec1
		punpckhbw vec3, vecZero
		punpcklbw vec1, vecZero
		movdqa vec2, vec3
		movdqa vec0, vec1
		punpcklwd vec0, vecZero
		punpckhwd vec1, vecZero
		punpcklwd vec2, vecZero
		punpckhwd vec3, vecZero
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 16; i < width16; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp width16, 16
		mov i, 16
		jle .EndOf_LoopWidth16
		.LoopWidth16:
			movdqa vec5, [ptrIn + (i*COMPV_YASM_UINT8_SZ_BYTES)]
			movdqa vec7, vec5
			punpckhbw vec7, vecZero
			punpcklbw vec5, vecZero
			movdqa vec6, vec7
			movdqa vec4, vec5
			punpcklwd vec4, vecZero
			punpckhwd vec5, vecZero
			punpcklwd vec6, vecZero
			punpckhwd vec7, vecZero
			paddd vec0, vec4
			paddd vec1, vec5
			paddd vec2, vec6
			paddd vec3, vec7
			add i, 16
			cmp i, width16
			jl .LoopWidth16
		.EndOf_LoopWidth16:

		paddd vec0, vec2
		paddd vec1, vec3
		paddd vec0, vec1

		pshufd vec1, vec0, 0xE
		paddd vec0, vec1
		pshufd vec1, vec0, 0x1
		paddd vec0, vec1
		movd sum, vec0

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
	%undef vec1			
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
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

%endif ; COMPV_YASM_ABI_IS_64BIT

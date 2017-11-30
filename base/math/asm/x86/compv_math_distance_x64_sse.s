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

global sym(CompVMathDistanceHamming_Asm_X64_POPCNT_SSE42)
global sym(CompVMathDistanceHamming32_Asm_X64_POPCNT_SSE42)
global sym(CompVMathDistanceLine_32f_Asm_X64_SSE2)
global sym(CompVMathDistanceParabola_32f_Asm_X64_SSE2)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* dataPtr
; arg(1) -> compv_uscalar_t width
; arg(2) -> compv_uscalar_t height
; arg(3) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
; arg(4) -> COMPV_ALIGNED(SSE) const uint8_t* patch1xnPtr
; arg(5) -> int32_t* distPtr
sym(CompVMathDistanceHamming_Asm_X64_POPCNT_SSE42):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	push r14
	push r15
	;; end prolog ;;

	%define argi_width		1

	%define j				rsi
	%define cnt				rdi
	%define cntdword		edi
	%define i				rcx
	%define dataPtr			rbx
	%define patch1xnPtr		rdx
	%define width			r8
	%define widthmax		r9
	%define distPtr			r10
	%define stride			r11
	%define width_minus31	r12

	mov dataPtr, arg(0)
	mov j, arg(2)
	mov stride, arg(3)
	mov width, arg(argi_width)
	mov patch1xnPtr, arg(4)
	mov distPtr, arg(5)
	lea width_minus31, [width - 31]

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		xor i, i
		xor cnt, cnt

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width - 31; i += 32)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp width_minus31, 0 ; width is required to be > 15 but not to be > 31
		jle .EndOf_LoopWidth32 ; jump to the end of the loop if (width_minus31 <= 0), notice the "=", do not use "js" jump which tests the sign only (< 0)
		.LoopWidth32:
			movdqa xmm0, [dataPtr + i]
			movdqa xmm2, [patch1xnPtr + i]
			movdqa xmm1, [dataPtr + i + 16]
			movdqa xmm3, [patch1xnPtr + i + 16]
			pxor xmm0, xmm2
			pxor xmm1, xmm3
			movq rax, xmm0
			pextrq r13, xmm0, 1
			movq r14, xmm1
			pextrq r15, xmm1, 1
			popcnt rax, rax
			popcnt r13, r13
			popcnt r14, r14
			popcnt r15, r15
			add i, 32
			add rax, r13
			add r14, r15
			add cnt, rax
			add cnt, r14
			cmp i, width_minus31
			jl .LoopWidth32
			.EndOf_LoopWidth32
			; EndOf_LoopWidth32 ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width - 15)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea widthmax, [width - 15]
		cmp i, widthmax
		jge .EndOf_IfMoreThan16
		.IfMoreThan16:
			movdqa xmm0, [dataPtr + i]
			movdqa xmm1, [patch1xnPtr + i]
			pxor xmm0, xmm1
			movq rax, xmm0
			pextrq r13, xmm0, 1
			popcnt rax, rax
			popcnt r13, r13
			add i, 16
			add cnt, rax
			add cnt, r13
			.EndOf_IfMoreThan16
			; EndOf_IfMoreThan16 ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width - 7)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea widthmax, [width - 7]
		cmp i, widthmax
		jge .EndOf_IfMoreThan8
		.IfMoreThan8:
			movdqa xmm0, [dataPtr + i]
			movdqa xmm1, [patch1xnPtr + i]
			pxor xmm0, xmm1
			movq rax, xmm0
			popcnt rax, rax
			add i, 8
			add cnt, rax
			.EndOf_IfMoreThan8
			; EndOf_IfMoreThan8 ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width - 3)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea widthmax, [width - 3]
		cmp i, widthmax
		jge .EndOf_IfMoreThan4
		.IfMoreThan4:
			mov eax, dword [dataPtr + i]
			xor eax, dword [patch1xnPtr + i]
			popcnt eax, eax
			add i, 4
			add cnt, rax
			.EndOf_IfMoreThan4:
			; EndOf_IfMoreThan4 ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width - 1)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		lea widthmax, [width - 1]
		cmp i, widthmax
		jge .EndOf_IfMoreThan2
		.IfMoreThan2:
			mov ax, word [dataPtr + i]
			xor ax, word [patch1xnPtr + i]
			popcnt ax, ax
			add i, 2
			add cnt, rax
			.EndOf_IfMoreThan2:
			; EndOf_IfMoreThan2 ;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (i < width)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, arg(argi_width)
		jge .EndOf_IfMoreThan1
		.IfMoreThan1:
			mov al, byte [dataPtr + i]
			xor al, byte [patch1xnPtr + i]
			popcnt ax, ax
			add cnt, rax
			.EndOf_IfMoreThan1:
			; EndOf_IfMoreThan1 ;

		dec j
		mov [distPtr], dword cntdword
		lea dataPtr, [dataPtr + stride]
		lea distPtr, [distPtr + COMPV_YASM_INT32_SZ_BYTES]
		jnz .LoopHeight
		; EndOf_LoopHeight ;

	%undef argi_width

	%undef j
	%undef cnt
	%undef cntdword
	%undef i
	%undef dataPtr
	%undef patch1xnPtr
	%undef width
	%undef widthmax
	%undef distPtr
	%undef stride
	%undef width_minus31

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
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* dataPtr
; arg(1) -> compv_uscalar_t height
; arg(2) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
; arg(3) -> COMPV_ALIGNED(SSE) const uint8_t* patch1xnPtr
; arg(4) -> int32_t* distPtr
sym(CompVMathDistanceHamming32_Asm_X64_POPCNT_SSE42):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	push r12
	;; end prolog ;;

	%define j					r8
	%define cnt					rax
	%define cntdword			eax
	%define dataPtr				r9
	%define distPtr				rdx
	%define stride				rcx

	%define vecpatch1xnPtr0		xmm2		
	%define vecpatch1xnPtr1		xmm3			

	mov rax, arg(3)
	movdqa vecpatch1xnPtr0, [rax]
	movdqa vecpatch1xnPtr1, [rax + 16]

	mov dataPtr, arg(0)
	mov j, arg(1)
	mov stride, arg(2)
	mov distPtr, arg(4)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		movdqa xmm0, [dataPtr]
		movdqa xmm1, [dataPtr + 16]
		pxor xmm0, vecpatch1xnPtr0
		pxor xmm1, vecpatch1xnPtr1
		movq cnt, xmm0
		pextrq r10, xmm0, 1
		movq r11, xmm1
		pextrq r12, xmm1, 1
		popcnt cnt, cnt
		popcnt r10, r10
		popcnt r11, r11
		popcnt r12, r12
		lea dataPtr, [dataPtr + stride]
		add cnt, r10
		add r11, r12
		add cnt, r11
		dec j
		mov [distPtr], dword cntdword
		lea distPtr, [distPtr + COMPV_YASM_INT32_SZ_BYTES]
		jnz .LoopHeight
		; EndOf_LoopHeight ;

	%undef j
	%undef cnt
	%undef cntdword
	%undef dataPtr
	%undef distPtr
	%undef stride

	%undef vecpatch1xnPtr0				
	%undef vecpatch1xnPtr1			

	;; begin epilog ;;
	pop r12
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const compv_float32_t* xPtr
; arg(1) -> COMPV_ALIGNED(SSE) const compv_float32_t* yPtr
; arg(2) -> const compv_float32_t* Ascaled1
; arg(3) -> const compv_float32_t* Bscaled1
; arg(4) -> const compv_float32_t* Cscaled1
; arg(5) -> COMPV_ALIGNED(SSE) compv_float32_t* distPtr
; arg(6) -> const compv_uscalar_t count
sym(CompVMathDistanceLine_32f_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_XMM 11
	;; end prolog ;;

	%define xPtr		rax
	%define yPtr		rdx
	%define i			rcx
	%define count16		r8
	%define distPtr		r9
	%define count		r10

	%define vecA		xmm0
	%define vecB		xmm1
	%define vecC		xmm2
	%define vecMask		xmm3

	mov rax, arg(2)
	mov rdx, arg(3)
	mov rcx, arg(4)
	mov r8, 0x7fffffff
	movss vecA, [rax]
	movss vecB, [rdx]
	movss vecC, [rcx]
	movd vecMask, r8d
	shufps vecA, vecA, 0x0
	shufps vecB, vecB, 0x0
	shufps vecC, vecC, 0x0
	shufps vecMask, vecMask, 0x0

	mov xPtr, arg(0)
	mov yPtr, arg(1)
	mov distPtr, arg(5)
	mov count, arg(6)
	mov count16, count
	and count16, -16

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (i = 0; i < count16; i += 16)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	xor i, i
	test count16, count16
	jz .Endof_LoopCount16
	.LoopCount16:
		movaps xmm4, vecA
		movaps xmm5, vecA
		mulps xmm4, [xPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES]
		mulps xmm5, [xPtr + (i+4)*COMPV_YASM_FLOAT32_SZ_BYTES]
		movaps xmm6, vecA
		movaps xmm7, vecA
		mulps xmm6, [xPtr + (i+8)*COMPV_YASM_FLOAT32_SZ_BYTES]
		mulps xmm7, [xPtr + (i+12)*COMPV_YASM_FLOAT32_SZ_BYTES]
		movaps xmm8, vecB
		movaps xmm9, vecB
		mulps xmm8, [yPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES]
		mulps xmm9, [yPtr + (i+4)*COMPV_YASM_FLOAT32_SZ_BYTES]
		movaps xmm10, vecB
		movaps xmm11, vecB
		mulps xmm10, [yPtr + (i+8)*COMPV_YASM_FLOAT32_SZ_BYTES]
		mulps xmm11, [yPtr + (i+12)*COMPV_YASM_FLOAT32_SZ_BYTES]
		addps xmm4, vecC
		addps xmm5, vecC
		addps xmm6, vecC
		addps xmm7, vecC
		addps xmm4, xmm8
		addps xmm5, xmm9
		addps xmm6, xmm10
		addps xmm7, xmm11
		andps xmm4, vecMask
		andps xmm5, vecMask
		andps xmm6, vecMask
		andps xmm7, vecMask
		movaps [distPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES], xmm4
		movaps [distPtr + (i+4)*COMPV_YASM_FLOAT32_SZ_BYTES], xmm5
		movaps [distPtr + (i+8)*COMPV_YASM_FLOAT32_SZ_BYTES], xmm6
		movaps [distPtr + (i+12)*COMPV_YASM_FLOAT32_SZ_BYTES], xmm7
		add i, 16
		cmp i, count16
		jl .LoopCount16
	.Endof_LoopCount16:

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (; i < count; i += 4)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp i, count
	jge .EndOf_LoopCount4
	.LoopCount4:
		movaps xmm4, [xPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES]
		movaps xmm8, [yPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES]
		mulps xmm4, vecA
		mulps xmm8, vecB
		addps xmm4, vecC
		addps xmm4, xmm8
		andps xmm4, vecMask
		movaps [distPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES], xmm4
		add i, 4
		cmp i, count
		jl .LoopCount4
	.EndOf_LoopCount4:

	%undef xPtr		
	%undef yPtr		
	%undef i			
	%undef count16		
	%undef distPtr		
	%undef count		

	%undef vecA		
	%undef vecB		
	%undef vecC		
	%undef vecMask		

	;; begin epilog ;;
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const compv_float32_t* xPtr
; arg(1) -> COMPV_ALIGNED(SSE) const compv_float32_t* yPtr
; arg(2) -> const compv_float32_t* A1
; arg(3) -> const compv_float32_t* B1
; arg(4) -> const compv_float32_t* C1
; arg(5) -> COMPV_ALIGNED(SSE) compv_float32_t* distPtr
; arg(6) -> const compv_uscalar_t count
sym(CompVMathDistanceParabola_32f_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_XMM 11
	;; end prolog ;;

	%define xPtr		rax
	%define yPtr		rdx
	%define i			rcx
	%define count16		r8
	%define distPtr		r9
	%define count		r10

	%define vecA		xmm0
	%define vecB		xmm1
	%define vecC		xmm2
	%define vecMask		xmm3

	mov rax, arg(2)
	mov rdx, arg(3)
	mov rcx, arg(4)
	mov r8, 0x7fffffff
	movss vecA, [rax]
	movss vecB, [rdx]
	movss vecC, [rcx]
	movd vecMask, r8d
	shufps vecA, vecA, 0x0
	shufps vecB, vecB, 0x0
	shufps vecC, vecC, 0x0
	shufps vecMask, vecMask, 0x0

	mov xPtr, arg(0)
	mov yPtr, arg(1)
	mov distPtr, arg(5)
	mov count, arg(6)
	mov count16, count
	and count16, -16

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (i = 0; i < count16; i += 16)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	xor i, i
	test count16, count16
	jz .Endof_LoopCount16
	.LoopCount16:
		movaps xmm4, [xPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES]
		movaps xmm5, [xPtr + (i+4)*COMPV_YASM_FLOAT32_SZ_BYTES]
		movaps xmm6, [xPtr + (i+8)*COMPV_YASM_FLOAT32_SZ_BYTES]
		movaps xmm7, [xPtr + (i+12)*COMPV_YASM_FLOAT32_SZ_BYTES]
		movaps xmm8, xmm4
		movaps xmm9, xmm5
		movaps xmm10, xmm6
		movaps xmm11, xmm7
		mulps xmm8, xmm8
		mulps xmm4, vecB
		mulps xmm9, xmm9
		mulps xmm5, vecB
		mulps xmm10, xmm10
		mulps xmm6, vecB
		mulps xmm11, xmm11		
		mulps xmm7, vecB
		mulps xmm8, vecA
		mulps xmm9, vecA
		mulps xmm10, vecA
		mulps xmm11, vecA
		addps xmm8, vecC
		addps xmm9, vecC
		addps xmm10, vecC
		addps xmm11, vecC
		addps xmm8, xmm4
		addps xmm9, xmm5
		addps xmm10, xmm6
		addps xmm11, xmm7
		subps xmm8, [yPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES]
		subps xmm9, [yPtr + (i+4)*COMPV_YASM_FLOAT32_SZ_BYTES]
		subps xmm10, [yPtr + (i+8)*COMPV_YASM_FLOAT32_SZ_BYTES]
		subps xmm11, [yPtr + (i+12)*COMPV_YASM_FLOAT32_SZ_BYTES]
		andps xmm8, vecMask
		andps xmm9, vecMask
		andps xmm10, vecMask
		andps xmm11, vecMask
		movaps [distPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES], xmm8
		movaps [distPtr + (i+4)*COMPV_YASM_FLOAT32_SZ_BYTES], xmm9
		movaps [distPtr + (i+8)*COMPV_YASM_FLOAT32_SZ_BYTES], xmm10
		movaps [distPtr + (i+12)*COMPV_YASM_FLOAT32_SZ_BYTES], xmm11
		add i, 16
		cmp i, count16
		jl .LoopCount16
	.Endof_LoopCount16:

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (; i < count; i += 4)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp i, count
	jge .EndOf_LoopCount4
	.LoopCount4:
		movaps xmm4, [xPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES]
		movaps xmm8, xmm4
		mulps xmm8, xmm8
		mulps xmm4, vecB
		mulps xmm8, vecA
		addps xmm8, vecC
		addps xmm8, xmm4
		subps xmm8, [yPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES]
		andps xmm8, vecMask
		movaps [distPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES], xmm8
		add i, 4
		cmp i, count
		jl .LoopCount4
	.EndOf_LoopCount4:

	%undef xPtr		
	%undef yPtr		
	%undef i			
	%undef count16		
	%undef distPtr		
	%undef count		

	%undef vecA		
	%undef vecB		
	%undef vecC		
	%undef vecMask		

	;; begin epilog ;;
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
	

%endif ; COMPV_YASM_ABI_IS_64BIT

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

global sym(CompVMathDistanceHamming32_Asm_X64_POPCNT_AVX2)
global sym(CompVMathDistanceLine_32f_Asm_X64_AVX)
global sym(CompVMathDistanceLine_32f_Asm_X64_FMA3_AVX)
global sym(CompVMathDistanceParabola_32f_Asm_X64_AVX)
global sym(CompVMathDistanceParabola_32f_Asm_X64_FMA3_AVX)

section .data
	extern sym(kShuffleEpi8_Popcnt_32s)
	extern sym(k15_8s)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(AVX) const uint8_t* dataPtr
; arg(1) -> compv_uscalar_t height
; arg(2) -> COMPV_ALIGNED(AVX) compv_uscalar_t stride
; arg(3) -> COMPV_ALIGNED(AVX) const uint8_t* patch1xnPtr
; arg(4) -> int32_t* distPtr
sym(CompVMathDistanceHamming32_Asm_X64_POPCNT_AVX2):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	COMPV_YASM_SAVE_YMM 15
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	push r14
	push r15
	;; end prolog ;;

	%define height				rbx
	%define j					rsi
	%define cnt					rdi
	%define cntdword			edi
	%define dataPtr				rdx
	%define distPtr				rcx
	%define stride				r8
	%define height_minus3		r9
	%define dataPtr_plus_stride	r10
	%define stride_times4		r11
	
	%define vecZero			ymm12
	%define vecMaskLow		ymm13
	%define vecLookup		ymm14
	%define vecPatch		ymm15		

	mov rax, arg(3)
	vpxor vecZero, vecZero
	vmovdqa vecLookup, [sym(kShuffleEpi8_Popcnt_32s)]
	vmovdqa vecMaskLow, [sym(k15_8s)]
	vmovdqa vecPatch, [rax]

	mov height, arg(1)
	mov dataPtr, arg(0)
	mov stride, arg(2)
	mov distPtr, arg(4)
	lea height_minus3, [height - 3]
	lea dataPtr_plus_stride, [dataPtr + stride]
	lea stride_times4, [stride*4]

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height - 3; j += 4)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov j, height
	shr j, 2 ; div 4
	je .EndOf_LoopHeight4
	.LoopHeight4:
		vpxor ymm0, vecPatch, [dataPtr]
		vpxor ymm1, vecPatch, [dataPtr_plus_stride]
		vpxor ymm2, vecPatch, [dataPtr_plus_stride + stride *1]
		vpxor ymm3, vecPatch, [dataPtr_plus_stride + stride *2]
		vpand ymm4, ymm0, vecMaskLow
		vpand ymm5, ymm1, vecMaskLow
		vpand ymm6, ymm2, vecMaskLow
		vpand ymm7, ymm3, vecMaskLow
		vpshufb ymm8, vecLookup, ymm4
		vpshufb ymm9, vecLookup, ymm5
		vpshufb ymm10, vecLookup, ymm6
		vpshufb ymm11, vecLookup, ymm7
		vpsrld ymm4, ymm0, 4
		vpsrld ymm5, ymm1, 4
		vpsrld ymm6, ymm2, 4
		vpsrld ymm7, ymm3, 4
		vpand ymm0, ymm4, vecMaskLow
		vpand ymm1, ymm5, vecMaskLow
		vpand ymm2, ymm6, vecMaskLow
		vpand ymm3, ymm7, vecMaskLow
		vpshufb ymm0, vecLookup, ymm0
		vpshufb ymm1, vecLookup, ymm1
		vpshufb ymm2, vecLookup, ymm2
		vpshufb ymm3, vecLookup, ymm3
		vpaddb ymm4, ymm0, ymm8
		vpaddb ymm5, ymm1, ymm9
		vpaddb ymm6, ymm2, ymm10
		vpaddb ymm7, ymm3, ymm11
		vpsadbw ymm0, ymm4, vecZero
		vpsadbw ymm1, ymm5, vecZero
		vpsadbw ymm2, ymm6, vecZero
		vpsadbw ymm3, ymm7, vecZero
		vpunpcklqdq ymm8, ymm0, ymm1
		vpunpckhqdq ymm9, ymm0, ymm1
		vpunpcklqdq ymm10, ymm2, ymm3
		vpunpckhqdq ymm11, ymm2, ymm3
		vpaddq ymm1, ymm8, ymm9
		vpaddq ymm3, ymm10, ymm11
		vperm2i128 ymm0, ymm1, ymm3, 0x20
		vperm2i128 ymm2, ymm1, ymm3, 0x31
		vpaddq ymm4, ymm0, ymm2
		vpshufd ymm5, ymm4, 0x88
		vpermq ymm6, ymm5, 0x08
		add dataPtr_plus_stride, stride_times4
		add dataPtr, stride_times4
		dec j
		vmovdqu [distPtr], xmm6
		lea distPtr, [distPtr + (4*COMPV_YASM_INT32_SZ_BYTES)]
		jnz .LoopHeight4
		.EndOf_LoopHeight4:
		; EndOf_LoopHeight4 ;

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (; j < height; j += 1)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	and height, 3 ; modulo 4
	jz .EndOf_LoopHeight1
	.LoopHeight1:
		vpxor ymm0, vecPatch, [dataPtr]
		vextracti128 xmm1, ymm0, 0x1
		vmovq r12, xmm0
		vpextrq r13, xmm0, 0x1
		vmovq r14, xmm1
		vpextrq r15, xmm1, 0x1
		popcnt cnt, r12
		popcnt r13, r13
		popcnt r14, r14
		popcnt r15, r15
		add cnt, r13
		add r14, r15
		add dataPtr, stride
		add cnt, r14
		mov [distPtr], dword cntdword
		add distPtr, (1*COMPV_YASM_INT32_SZ_BYTES)
		dec height
		jnz .LoopHeight1
		.EndOf_LoopHeight1:
		; EndOf_LoopHeight1 ;

	%undef height
	%undef j
	%undef cnt
	%undef cntdword
	%undef dataPtr
	%undef distPtr
	%undef stride
	%undef height_minus3
	%undef dataPtr_plus_stride
	%undef stride_times4

	%undef vecZero
	%undef vecMaskLow
	%undef vecLookup
	%undef vecPatch	

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
; arg(0) -> COMPV_ALIGNED(AVX) const compv_float32_t* xPtr
; arg(1) -> COMPV_ALIGNED(AVX) const compv_float32_t* yPtr
; arg(2) -> const compv_float32_t* Ascaled1
; arg(3) -> const compv_float32_t* Bscaled1
; arg(4) -> const compv_float32_t* Cscaled1
; arg(5) -> COMPV_ALIGNED(AVX) compv_float32_t* distPtr
; arg(6) -> const compv_uscalar_t count
; %1 -> 0: FMA3 not supported, 1: FMA3 supported
%macro CompVMathDistanceLine_32f_Macro_X64_AVX 1
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_YMM 11
	;; end prolog ;;

	%define xPtr		rax
	%define yPtr		rdx
	%define i			rcx
	%define count32		r8
	%define distPtr		r9
	%define count		r10

	%define vecA		ymm0
	%define vecAn		xmm0
	%define vecB		ymm1
	%define vecBn		xmm1
	%define vecC		ymm2
	%define vecCn		xmm2
	%define vecMask		ymm3
	%define vecMaskn	xmm3

	mov rax, arg(2)
	mov rdx, arg(3)
	mov rcx, arg(4)
	mov r8, 0x7fffffff
	movss vecAn, [rax]
	movss vecBn, [rdx]
	movss vecCn, [rcx]
	movd vecMaskn, r8d
	vbroadcastss vecA, vecAn
	vbroadcastss vecB, vecBn
	vbroadcastss vecC, vecCn
	vpbroadcastd vecMask, vecMaskn

	mov xPtr, arg(0)
	mov yPtr, arg(1)
	mov distPtr, arg(5)
	mov count, arg(6)
	mov count32, count
	and count32, -32

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (i = 0; i < count32; i += 32)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	xor i, i
	test count32, count32
	jz .Endof_LoopCount32
	.LoopCount32:
		%if %1
			vmovaps ymm4, [xPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES]
			vmovaps ymm5, [xPtr + (i+8)*COMPV_YASM_FLOAT32_SZ_BYTES]
			vfmadd213ps ymm4, vecA, vecC
			vfmadd213ps ymm5, vecA, vecC
			vmovaps ymm6, [xPtr + (i+16)*COMPV_YASM_FLOAT32_SZ_BYTES]
			vmovaps ymm7, [xPtr + (i+24)*COMPV_YASM_FLOAT32_SZ_BYTES]			
			vfmadd213ps ymm6, vecA, vecC
			vfmadd213ps ymm7, vecA, vecC
			vfmadd231ps ymm4, vecB, [yPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES]
			vfmadd231ps ymm5, vecB, [yPtr + (i+8)*COMPV_YASM_FLOAT32_SZ_BYTES]
			vfmadd231ps ymm6, vecB, [yPtr + (i+16)*COMPV_YASM_FLOAT32_SZ_BYTES]
			vfmadd231ps ymm7, vecB, [yPtr + (i+24)*COMPV_YASM_FLOAT32_SZ_BYTES]
		%else
			vmulps ymm4, vecA, [xPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES]
			vmulps ymm5, vecA, [xPtr + (i+8)*COMPV_YASM_FLOAT32_SZ_BYTES]
			vmulps ymm6, vecA, [xPtr + (i+16)*COMPV_YASM_FLOAT32_SZ_BYTES]
			vmulps ymm7, vecA, [xPtr + (i+24)*COMPV_YASM_FLOAT32_SZ_BYTES]
			vmulps ymm8, vecB, [yPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES]
			vmulps ymm9, vecB, [yPtr + (i+8)*COMPV_YASM_FLOAT32_SZ_BYTES]
			vmulps ymm10, vecB, [yPtr + (i+16)*COMPV_YASM_FLOAT32_SZ_BYTES]
			vmulps ymm11, vecB, [yPtr + (i+24)*COMPV_YASM_FLOAT32_SZ_BYTES]
			vaddps ymm4, ymm4, vecC
			vaddps ymm5, ymm5, vecC
			vaddps ymm6, ymm6, vecC
			vaddps ymm7, ymm7, vecC
			vaddps ymm4, ymm4, ymm8
			vaddps ymm5, ymm5, ymm9
			vaddps ymm6, ymm6, ymm10
			vaddps ymm7, ymm7, ymm11
		%endif

		vandps ymm4, ymm4, vecMask
		vandps ymm5, ymm5, vecMask
		vandps ymm6, ymm6, vecMask
		vandps ymm7, ymm7, vecMask
		vmovaps [distPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES], ymm4
		vmovaps [distPtr + (i+8)*COMPV_YASM_FLOAT32_SZ_BYTES], ymm5
		vmovaps [distPtr + (i+16)*COMPV_YASM_FLOAT32_SZ_BYTES], ymm6
		vmovaps [distPtr + (i+24)*COMPV_YASM_FLOAT32_SZ_BYTES], ymm7
		add i, 32
		cmp i, count32
		jl .LoopCount32
	.Endof_LoopCount32:

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (; i < count; i += 8)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp i, count
	jge .EndOf_LoopCount8
	.LoopCount8:
		%if %1
			vmovaps ymm4, [xPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES]
			vfmadd213ps ymm4, vecA, vecC
			vfmadd231ps ymm4, vecB, [yPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES]
		%else
			vmulps ymm4, vecA, [xPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES]
			vmulps ymm8, vecB, [yPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES]
			vaddps ymm4, ymm4, vecC
			vaddps ymm4, ymm4, ymm8
		%endif
		vandps ymm4, ymm4, vecMask
		vmovaps [distPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES], ymm4
		add i, 8
		cmp i, count
		jl .LoopCount8
	.EndOf_LoopCount8:

	%undef xPtr		
	%undef yPtr		
	%undef i			
	%undef count32		
	%undef distPtr		
	%undef count		

	%undef vecA
	%undef vecAn	
	%undef vecB
	%undef vecBn	
	%undef vecC
	%undef vecCn		
	%undef vecMask
	%undef vecMaskn	

	;; begin epilog ;;
	COMPV_YASM_RESTORE_YMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVMathDistanceLine_32f_Asm_X64_AVX):
	CompVMathDistanceLine_32f_Macro_X64_AVX 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVMathDistanceLine_32f_Asm_X64_FMA3_AVX):
	CompVMathDistanceLine_32f_Macro_X64_AVX 1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const compv_float32_t* xPtr
; arg(1) -> COMPV_ALIGNED(SSE) const compv_float32_t* yPtr
; arg(2) -> const compv_float32_t* A1
; arg(3) -> const compv_float32_t* B1
; arg(4) -> const compv_float32_t* C1
; arg(5) -> COMPV_ALIGNED(SSE) compv_float32_t* distPtr
; arg(6) -> const compv_uscalar_t count
; %1 -> 0: FMA3 not supported, 1: FMA3 supported
%macro CompVMathDistanceParabola_32f_Macro_X64_AVX 1
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_XMM 11
	;; end prolog ;;

	%define xPtr		rax
	%define yPtr		rdx
	%define i			rcx
	%define count32		r8
	%define distPtr		r9
	%define count		r10

	%define vecA		ymm0
	%define vecAn		xmm0
	%define vecB		ymm1
	%define vecBn		xmm1
	%define vecC		ymm2
	%define vecCn		xmm2
	%define vecMask		ymm3
	%define vecMaskn	xmm3

	mov rax, arg(2)
	mov rdx, arg(3)
	mov rcx, arg(4)
	mov r8, 0x7fffffff
	movss vecAn, [rax]
	movss vecBn, [rdx]
	movss vecCn, [rcx]
	movd vecMaskn, r8d
	vbroadcastss vecA, vecAn
	vbroadcastss vecB, vecBn
	vbroadcastss vecC, vecCn
	vpbroadcastd vecMask, vecMaskn

	mov xPtr, arg(0)
	mov yPtr, arg(1)
	mov distPtr, arg(5)
	mov count, arg(6)
	mov count32, count
	and count32, -32

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (i = 0; i < count32; i += 32)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	xor i, i
	test count32, count32
	jz .Endof_LoopCount32
	.LoopCount32:
		vmovaps ymm4, [xPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES]
		vmovaps ymm5, [xPtr + (i+8)*COMPV_YASM_FLOAT32_SZ_BYTES]
		vmovaps ymm6, [xPtr + (i+16)*COMPV_YASM_FLOAT32_SZ_BYTES]
		vmovaps ymm7, [xPtr + (i+24)*COMPV_YASM_FLOAT32_SZ_BYTES]
		%if %1
			vmulps ymm8, ymm4, ymm4
			vmulps ymm9, ymm5, ymm5
			vmulps ymm10, ymm6, ymm6
			vmulps ymm11, ymm7, ymm7
			vfmadd213ps ymm8, vecA, vecC
			vfmadd213ps ymm9, vecA, vecC	
			vfmadd213ps ymm10, vecA, vecC
			vfmadd213ps ymm11, vecA, vecC
			vfmadd231ps ymm8, vecB, ymm4
			vfmadd231ps ymm9, vecB, ymm5	
			vfmadd231ps ymm10, vecB, ymm6
			vfmadd231ps ymm11, vecB, ymm7
		%else
			vmulps ymm8, ymm4, ymm4
			vmulps ymm9, ymm5, ymm5
			vmulps ymm10, ymm6, ymm6
			vmulps ymm11, ymm7, ymm7
			vmulps ymm4, vecB
			vmulps ymm5, vecB
			vmulps ymm6, vecB
			vmulps ymm7, vecB
			vmulps ymm8, vecA
			vmulps ymm9, vecA
			vmulps ymm10, vecA
			vmulps ymm11, vecA
			vaddps ymm8, vecC
			vaddps ymm9, vecC
			vaddps ymm10, vecC
			vaddps ymm11, vecC
			vaddps ymm8, ymm4
			vaddps ymm9, ymm5
			vaddps ymm10, ymm6
			vaddps ymm11, ymm7
		%endif
		vsubps ymm8, [yPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES]
		vsubps ymm9, [yPtr + (i+8)*COMPV_YASM_FLOAT32_SZ_BYTES]
		vsubps ymm10, [yPtr + (i+16)*COMPV_YASM_FLOAT32_SZ_BYTES]
		vsubps ymm11, [yPtr + (i+24)*COMPV_YASM_FLOAT32_SZ_BYTES]
		vandps ymm8, vecMask
		vandps ymm9, vecMask
		vandps ymm10, vecMask
		vandps ymm11, vecMask
		vmovaps [distPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES], ymm8
		vmovaps [distPtr + (i+8)*COMPV_YASM_FLOAT32_SZ_BYTES], ymm9
		vmovaps [distPtr + (i+16)*COMPV_YASM_FLOAT32_SZ_BYTES], ymm10
		vmovaps [distPtr + (i+24)*COMPV_YASM_FLOAT32_SZ_BYTES], ymm11
		add i, 32
		cmp i, count32
		jl .LoopCount32
	.Endof_LoopCount32:

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (; i < count; i += 8)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp i, count
	jge .EndOf_LoopCount8
	.LoopCount8:
		vmovaps ymm4, [xPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES]
		%if %1
			vmulps ymm8, ymm4, ymm4
			vfmadd213ps ymm8, vecA, vecC
			vfmadd231ps ymm8, vecB, ymm4
		%else
			vmulps ymm8, ymm4, ymm4
			vmulps ymm4, vecB
			vmulps ymm8, vecA
			vaddps ymm8, vecC
			vaddps ymm8, ymm4
		%endif
		vsubps ymm8, [yPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES]
		vandps ymm8, vecMask
		vmovaps [distPtr + (i+0)*COMPV_YASM_FLOAT32_SZ_BYTES], ymm8
		add i, 8
		cmp i, count
		jl .LoopCount8
	.EndOf_LoopCount8:

	%undef xPtr		
	%undef yPtr		
	%undef i			
	%undef count32		
	%undef distPtr		
	%undef count		

	%undef vecA
	%undef vecAn	
	%undef vecB
	%undef vecBn	
	%undef vecC
	%undef vecCn		
	%undef vecMask
	%undef vecMaskn	

	;; begin epilog ;;
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVMathDistanceParabola_32f_Asm_X64_AVX):
	CompVMathDistanceParabola_32f_Macro_X64_AVX 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVMathDistanceParabola_32f_Asm_X64_FMA3_AVX):
	CompVMathDistanceParabola_32f_Macro_X64_AVX 1

%endif ; COMPV_YASM_ABI_IS_64BIT

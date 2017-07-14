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

global sym(CompVMathDistanceHamming32_Asm_X64_POPCNT_AVX2)

section .data
	extern sym(kShuffleEpi8_Popcnt_i32)
	extern sym(k15_i8)

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
	vmovdqa vecLookup, [sym(kShuffleEpi8_Popcnt_i32)]
	vmovdqa vecMaskLow, [sym(k15_i8)]
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


%endif ; COMPV_YASM_ABI_IS_64BIT

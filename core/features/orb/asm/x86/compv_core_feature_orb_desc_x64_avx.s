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

global sym(CompVOrbBrief256_31_32f_Asm_X64_AVX2)
global sym(CompVOrbBrief256_31_32f_Asm_X64_FMA3_AVX2)

section .data
	extern COMPV_YASM_DLLIMPORT_DECL(k128_u8)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* img_center
; arg(1) -> compv_uscalar_t img_stride
; arg(2) -> const compv_float32_t* cos1
; arg(3) -> const compv_float32_t* sin1
; arg(4) -> COMPV_ALIGNED(AVX) const compv_float32_t* kBrief256Pattern31AX
; arg(5) -> COMPV_ALIGNED(AVX) const compv_float32_t* kBrief256Pattern31AY
; arg(6) -> COMPV_ALIGNED(AVX) const compv_float32_t* kBrief256Pattern31BX
; arg(7) -> COMPV_ALIGNED(AVX) const compv_float32_t* kBrief256Pattern31BY
; arg(8) -> void* out
; %1 -> 1: FMA3 enabled, 0: FMA3 disabled
%macro CompVOrbBrief256_31_32f_Macro_X64_AVX2 1
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 9
	COMPV_YASM_SAVE_YMM 15
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	push r14
	push r15
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 32, rax
	sub rsp, (32*COMPV_YASM_INT32_SZ_BYTES) + (32*COMPV_YASM_UINT8_SZ_BYTES) + (32*COMPV_YASM_UINT8_SZ_BYTES) + (4*COMPV_YASM_YMM_SZ_BYTES)

	%define vecIndex            rsp + 0
	%define vecA				vecIndex + (32*COMPV_YASM_INT32_SZ_BYTES)
	%define vecB				vecA + (32*COMPV_YASM_UINT8_SZ_BYTES)
	%define vecCosT				vecB + (32*COMPV_YASM_UINT8_SZ_BYTES)
	%define vecSinT				vecCosT + (1*COMPV_YASM_YMM_SZ_BYTES)
	%define vecStride			vecSinT + (1*COMPV_YASM_YMM_SZ_BYTES)
	%define vec128				vecStride + (1*COMPV_YASM_YMM_SZ_BYTES)

	%define argi_kBrief256Pattern31AX 4
	%define argi_kBrief256Pattern31AY 5
	%define argi_kBrief256Pattern31BX 6
	%define argi_kBrief256Pattern31BY 7

	%define i			rsi
	%define img_center	rbx
	%define outPtr		rdi

	mov img_center, arg(0)
	mov outPtr, arg(8)

	; Compute vecStride ;
	mov rax, arg(1) ; stride
	vmovd xmm7, eax
	vpbroadcastd ymm7, xmm7
	%if %1 == 1
		vcvtdq2ps ymm7, ymm7
	%endif
	vmovdqa [vecStride], ymm7

	; Compute vecCosT and vecSinT
	mov rax, arg(2) ; cos1
	mov rdx, arg(3) ; sin1
	vmovss xmm0, [rax]
	vmovss xmm1, [rdx]
	vbroadcastss  ymm0, xmm0
	vbroadcastss  ymm1, xmm1
	vmovaps[vecCosT], ymm0
	vmovaps[vecSinT], ymm1

	; Compute vec128 ;
	COMPV_YASM_DLLIMPORT_LOAD vmovdqa, ymm2, k128_u8, rax
	vmovdqa [vec128], ymm2

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (size_t i = 0; i < 256; i += 32)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	xor i, i
	.Loop256:
		%assign xy 0 ; 0...1
		%rep 2
			%if xy == 0
				mov rax, arg(argi_kBrief256Pattern31AX)
				mov rdx, arg(argi_kBrief256Pattern31AY)
			%else
				mov rax, arg(argi_kBrief256Pattern31BX)
				mov rdx, arg(argi_kBrief256Pattern31BY)
			%endif
			vmovaps ymm11, [vecSinT]
			vmovaps ymm15, [vecCosT]
			
			vmulps ymm8, ymm11, [rax + ((i + 0) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			vmulps ymm9, ymm11, [rax + ((i + 8) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			vmulps ymm10, ymm11, [rax + ((i + 16) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			vmulps ymm11, ymm11, [rax + ((i + 24) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			%if %1 == 1
				vfmadd231ps ymm8, ymm15, [rdx + ((i + 0) * COMPV_YASM_FLOAT32_SZ_BYTES)]
				vfmadd231ps ymm9, ymm15, [rdx + ((i + 8) * COMPV_YASM_FLOAT32_SZ_BYTES)]
				vfmadd231ps ymm10, ymm15, [rdx + ((i + 16) * COMPV_YASM_FLOAT32_SZ_BYTES)]
				vfmadd231ps ymm11, ymm15, [rdx + ((i + 24) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			%else
				vmulps ymm12, ymm15, [rdx + ((i + 0) * COMPV_YASM_FLOAT32_SZ_BYTES)]
				vmulps ymm13, ymm15, [rdx + ((i + 8) * COMPV_YASM_FLOAT32_SZ_BYTES)]
				vmulps ymm14, ymm15, [rdx + ((i + 16) * COMPV_YASM_FLOAT32_SZ_BYTES)]
				vmulps ymm15, ymm15, [rdx + ((i + 24) * COMPV_YASM_FLOAT32_SZ_BYTES)]			
				vaddps ymm8, ymm8, ymm12
				vaddps ymm9, ymm9, ymm13
				vaddps ymm10, ymm10, ymm14
				vaddps ymm11, ymm11, ymm15
				vcvtps2dq ymm8, ymm8
				vcvtps2dq ymm9, ymm9
				vcvtps2dq ymm10, ymm10
				vcvtps2dq ymm11, ymm11
				vpmulld ymm8, ymm8, [vecStride]
				vpmulld ymm9, ymm9, [vecStride]
				vpmulld ymm10, ymm10, [vecStride]
				vpmulld ymm11, ymm11, [vecStride]
			%endif			

			vmovaps ymm3, [vecCosT]
			vmovaps ymm7, [vecSinT]
			
			%if %1 == 1
				vmulps ymm4, ymm7, [rdx + ((i + 0) * COMPV_YASM_FLOAT32_SZ_BYTES)]
				vmulps ymm5, ymm7, [rdx + ((i + 8) * COMPV_YASM_FLOAT32_SZ_BYTES)]
				vmulps ymm6, ymm7, [rdx + ((i + 16) * COMPV_YASM_FLOAT32_SZ_BYTES)]
				vmulps ymm7, ymm7, [rdx + ((i + 24) * COMPV_YASM_FLOAT32_SZ_BYTES)]
				vroundps ymm8, ymm8, 0x8
				vroundps ymm9, ymm9, 0x8
				vfmsub231ps ymm4, ymm3, [rax + ((i + 0) * COMPV_YASM_FLOAT32_SZ_BYTES)]
				vfmsub231ps ymm5, ymm3, [rax + ((i + 8) * COMPV_YASM_FLOAT32_SZ_BYTES)]
				vfmsub231ps ymm6, ymm3, [rax + ((i + 16) * COMPV_YASM_FLOAT32_SZ_BYTES)]
				vfmsub231ps ymm7, ymm3, [rax + ((i + 24) * COMPV_YASM_FLOAT32_SZ_BYTES)]
				vroundps ymm10, ymm10, 0x8
				vroundps ymm11, ymm11, 0x8
				vroundps ymm0, ymm4, 0x8
				vroundps ymm1, ymm5, 0x8
				vroundps ymm2, ymm6, 0x8
				vroundps ymm3, ymm7, 0x8
				vfmadd132ps ymm8, ymm0, [vecStride]
				vfmadd132ps ymm9, ymm1, [vecStride]
				vfmadd132ps ymm10, ymm2, [vecStride]
				vfmadd132ps ymm11, ymm3, [vecStride]
				vcvtps2dq ymm8, ymm8
				vcvtps2dq ymm9, ymm9
				vcvtps2dq ymm10, ymm10
				vcvtps2dq ymm11, ymm11
			%else
				vmulps ymm0, ymm3, [rax + ((i + 0) * COMPV_YASM_FLOAT32_SZ_BYTES)]
				vmulps ymm1, ymm3, [rax + ((i + 8) * COMPV_YASM_FLOAT32_SZ_BYTES)]
				vmulps ymm2, ymm3, [rax + ((i + 16) * COMPV_YASM_FLOAT32_SZ_BYTES)]
				vmulps ymm3, ymm3, [rax + ((i + 24) * COMPV_YASM_FLOAT32_SZ_BYTES)]
				vmulps ymm4, ymm7, [rdx + ((i + 0) * COMPV_YASM_FLOAT32_SZ_BYTES)]
				vmulps ymm5, ymm7, [rdx + ((i + 8) * COMPV_YASM_FLOAT32_SZ_BYTES)]
				vmulps ymm6, ymm7, [rdx + ((i + 16) * COMPV_YASM_FLOAT32_SZ_BYTES)]
				vmulps ymm7, ymm7, [rdx + ((i + 24) * COMPV_YASM_FLOAT32_SZ_BYTES)]
				vsubps ymm0, ymm0, ymm4
				vsubps ymm1, ymm1, ymm5
				vsubps ymm2, ymm2, ymm6
				vsubps ymm3, ymm3, ymm7
				vcvtps2dq ymm0, ymm0
				vcvtps2dq ymm1, ymm1
				vcvtps2dq ymm2, ymm2
				vcvtps2dq ymm3, ymm3
				vpaddd ymm8, ymm8, ymm0
				vpaddd ymm9, ymm9, ymm1
				vpaddd ymm10, ymm10, ymm2
				vpaddd ymm11, ymm11, ymm3
			%endif
			vmovdqa[vecIndex + (0*COMPV_YASM_INT32_SZ_BYTES)], ymm8
			vmovdqa[vecIndex + (8*COMPV_YASM_INT32_SZ_BYTES)], ymm9
			vmovdqa[vecIndex + (16*COMPV_YASM_INT32_SZ_BYTES)], ymm10
			vmovdqa[vecIndex + (24*COMPV_YASM_INT32_SZ_BYTES)], ymm11
			%assign index 0 ; 0.....31
			%rep 4
				movsxd r8, dword [vecIndex + ((index+0)*COMPV_YASM_INT32_SZ_BYTES)]
				movsxd r9, dword [vecIndex + ((index+1)*COMPV_YASM_INT32_SZ_BYTES)]
				movsxd r10, dword [vecIndex + ((index+2)*COMPV_YASM_INT32_SZ_BYTES)]
				movsxd r11, dword [vecIndex + ((index+3)*COMPV_YASM_INT32_SZ_BYTES)]
				movsxd r12, dword [vecIndex + ((index+4)*COMPV_YASM_INT32_SZ_BYTES)]			
				movsxd r13, dword [vecIndex + ((index+5)*COMPV_YASM_INT32_SZ_BYTES)]
				movsxd r14, dword [vecIndex + ((index+6)*COMPV_YASM_INT32_SZ_BYTES)]
				movsxd r15, dword [vecIndex + ((index+7)*COMPV_YASM_INT32_SZ_BYTES)]
				movzx r8, byte [img_center + r8]
				movzx r9, byte [img_center + r9]
				movzx r10, byte [img_center + r10]
				movzx r11, byte [img_center + r11]
				movzx r12, byte [img_center + r12]
				movzx r13, byte [img_center + r13]
				movzx r14, byte [img_center + r14]
				movzx r15, byte [img_center + r15]
				%if xy == 0
					mov [vecA + ((index+0)*COMPV_YASM_UINT8_SZ_BYTES)], byte r8b
					mov [vecA + ((index+1)*COMPV_YASM_UINT8_SZ_BYTES)], byte r9b
					mov [vecA + ((index+2)*COMPV_YASM_UINT8_SZ_BYTES)], byte r10b
					mov [vecA + ((index+3)*COMPV_YASM_UINT8_SZ_BYTES)], byte r11b
					mov [vecA + ((index+4)*COMPV_YASM_UINT8_SZ_BYTES)], byte r12b
					mov [vecA + ((index+5)*COMPV_YASM_UINT8_SZ_BYTES)], byte r13b
					mov [vecA + ((index+6)*COMPV_YASM_UINT8_SZ_BYTES)], byte r14b
					mov [vecA + ((index+7)*COMPV_YASM_UINT8_SZ_BYTES)], byte r15b
				%else
					mov [vecB + ((index+0)*COMPV_YASM_UINT8_SZ_BYTES)], byte r8b
					mov [vecB + ((index+1)*COMPV_YASM_UINT8_SZ_BYTES)], byte r9b
					mov [vecB + ((index+2)*COMPV_YASM_UINT8_SZ_BYTES)], byte r10b
					mov [vecB + ((index+3)*COMPV_YASM_UINT8_SZ_BYTES)], byte r11b
					mov [vecB + ((index+4)*COMPV_YASM_UINT8_SZ_BYTES)], byte r12b
					mov [vecB + ((index+5)*COMPV_YASM_UINT8_SZ_BYTES)], byte r13b
					mov [vecB + ((index+6)*COMPV_YASM_UINT8_SZ_BYTES)], byte r14b
					mov [vecB + ((index+7)*COMPV_YASM_UINT8_SZ_BYTES)], byte r15b
				%endif
				%assign index index+8
			%endrep ; rep index
		%assign xy xy+1
		%endrep ; rep xy

		vmovdqa ymm0, [vecB] ; pcmpltb doesn''t exist -> inverse vecA/vecB
		vmovdqa ymm1, [vecA]
		vpsubb ymm0, ymm0, [vec128]
		vpsubb ymm1, ymm1, [vec128]
		vpcmpgtb ymm0, ymm0, ymm1
		vpmovmskb rax, ymm0
		add i, 32
		cmp i, 256
		mov [outPtr], dword eax
		lea outPtr, [outPtr + COMPV_YASM_UINT32_SZ_BYTES]
		jl .Loop256
		;; EndOf_Loop256 ;;


	%undef vecIndex
	%undef vecA
	%undef vecB
	%undef vecX
	%undef vecCosT
	%undef vecSinT
	%undef vecStride

	%undef argi_kBrief256Pattern31AX
	%undef argi_kBrief256Pattern31AY
	%undef argi_kBrief256Pattern31BX
	%undef argi_kBrief256Pattern31BY

	%undef i
	%undef img_center
	%undef outPtr

	; free memory and unalign stack
	add rsp, (32*COMPV_YASM_INT32_SZ_BYTES) + (32*COMPV_YASM_UINT8_SZ_BYTES) + (32*COMPV_YASM_UINT8_SZ_BYTES) + (4*COMPV_YASM_YMM_SZ_BYTES)
	COMPV_YASM_UNALIGN_STACK

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
sym(CompVOrbBrief256_31_32f_Asm_X64_AVX2):
	CompVOrbBrief256_31_32f_Macro_X64_AVX2 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVOrbBrief256_31_32f_Asm_X64_FMA3_AVX2):
	CompVOrbBrief256_31_32f_Macro_X64_AVX2 1

%endif ; COMPV_YASM_ABI_IS_64BIT
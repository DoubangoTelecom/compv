;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(CompVOrbBrief256_31_32f_Asm_X86_AVX2)

section .data
	k128_8u db 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128

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
sym(CompVOrbBrief256_31_32f_Asm_X86_AVX2):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 9
	COMPV_YASM_SAVE_YMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 32, rax
	sub rsp, (32*COMPV_YASM_INT32_SZ_BYTES) + (32*COMPV_YASM_UINT8_SZ_BYTES) + (32*COMPV_YASM_UINT8_SZ_BYTES) + (4*COMPV_YASM_YMM_SZ_BYTES) + (4*COMPV_YASM_YMM_SZ_BYTES)

	%define vecIndex            rsp + 0
	%define vecA				vecIndex + (32*COMPV_YASM_INT32_SZ_BYTES)
	%define vecB				vecA + (32*COMPV_YASM_UINT8_SZ_BYTES)
	%define vecX				vecB + (32*COMPV_YASM_UINT8_SZ_BYTES)
	%define vecCosT				vecX + (4*COMPV_YASM_YMM_SZ_BYTES)
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
	vmovdqu ymm2, [sym(k128_8u)]
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
			;; xf = (kBrief256Pattern31AX[i] * cosT - kBrief256Pattern31AY[i] * sinT) ;;
			vmovaps ymm3, [vecCosT]
			vmovaps ymm7, [vecSinT]
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
			vmovdqa[vecX + (0*COMPV_YASM_YMM_SZ_BYTES)], ymm0
			vmovdqa[vecX + (1*COMPV_YASM_YMM_SZ_BYTES)], ymm1
			vmovdqa[vecX + (2*COMPV_YASM_YMM_SZ_BYTES)], ymm2
			vmovdqa[vecX + (3*COMPV_YASM_YMM_SZ_BYTES)], ymm3
			;; yf = (kBrief256Pattern31AX[i] * sinT + kBrief256Pattern31AY[i] * cosT) ;;
			vmovaps ymm3, [vecSinT]
			vmovaps ymm7, [vecCosT]
			vmulps ymm0, ymm3, [rax + ((i + 0) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			vmulps ymm1, ymm3, [rax + ((i + 8) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			vmulps ymm2, ymm3, [rax + ((i + 16) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			vmulps ymm3, ymm3, [rax + ((i + 24) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			vmulps ymm4, ymm7, [rdx + ((i + 0) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			vmulps ymm5, ymm7, [rdx + ((i + 8) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			vmulps ymm6, ymm7, [rdx + ((i + 16) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			vmulps ymm7, ymm7, [rdx + ((i + 24) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			vaddps ymm0, ymm0, ymm4
			vaddps ymm1, ymm1, ymm5
			vaddps ymm2, ymm2, ymm6
			vaddps ymm3, ymm3, ymm7
			vcvtps2dq ymm0, ymm0
			vcvtps2dq ymm1, ymm1
			vcvtps2dq ymm2, ymm2
			vcvtps2dq ymm3, ymm3
			vpmulld ymm0, ymm0, [vecStride]
			vpmulld ymm1, ymm1, [vecStride]
			vpmulld ymm2, ymm2, [vecStride]
			vpmulld ymm3, ymm3, [vecStride]
			; vpmulld latency toooo high(10) -> delay vpaddd
			%assign index 0 ; 0.....31
			%rep 11
				%if index == 0
					vpaddd ymm0, ymm0, [vecX + (0*COMPV_YASM_YMM_SZ_BYTES)]
					vmovdqa[vecIndex + (0*COMPV_YASM_INT32_SZ_BYTES)], ymm0
				%elif index == 3*2
					vpaddd ymm1, ymm1, [vecX + (1*COMPV_YASM_YMM_SZ_BYTES)]
					vmovdqa[vecIndex + (8*COMPV_YASM_INT32_SZ_BYTES)], ymm1
				%elif index == 3*4
					vpaddd ymm2, ymm2, [vecX + (2*COMPV_YASM_YMM_SZ_BYTES)]
					vmovdqa[vecIndex + (16*COMPV_YASM_INT32_SZ_BYTES)], ymm2
				%elif index == 3*6
					vpaddd ymm3, ymm3, [vecX + (3*COMPV_YASM_YMM_SZ_BYTES)]
					vmovdqa[vecIndex + (24*COMPV_YASM_INT32_SZ_BYTES)], ymm3
				%endif
				movsxd rax, dword [vecIndex + ((index+0)*COMPV_YASM_INT32_SZ_BYTES)]
				movsxd rcx, dword [vecIndex + ((index+1)*COMPV_YASM_INT32_SZ_BYTES)]
				%if index < 30
					movsxd rdx, dword [vecIndex + ((index+2)*COMPV_YASM_INT32_SZ_BYTES)]
				%endif
				movzx rax, byte [img_center + rax]
				movzx rcx, byte [img_center + rcx]
				%if index < 30
					movzx rdx, byte [img_center + rdx]
				%endif
				%if xy == 0
					mov [vecA + ((index+0)*COMPV_YASM_UINT8_SZ_BYTES)], byte al
					mov [vecA + ((index+1)*COMPV_YASM_UINT8_SZ_BYTES)], byte cl
				%else
					mov [vecB + ((index+0)*COMPV_YASM_UINT8_SZ_BYTES)], byte al
					mov [vecB + ((index+1)*COMPV_YASM_UINT8_SZ_BYTES)], byte cl
				%endif
				%if index < 30
					%if xy == 0
						mov [vecA + ((index+2)*COMPV_YASM_UINT8_SZ_BYTES)], byte dl
					%else
						mov [vecB + ((index+2)*COMPV_YASM_UINT8_SZ_BYTES)], byte dl
					%endif
				%endif
				%assign index index+3
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
	add rsp, (32*COMPV_YASM_INT32_SZ_BYTES) + (32*COMPV_YASM_UINT8_SZ_BYTES) + (32*COMPV_YASM_UINT8_SZ_BYTES) + (4*COMPV_YASM_YMM_SZ_BYTES) + (4*COMPV_YASM_YMM_SZ_BYTES)
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
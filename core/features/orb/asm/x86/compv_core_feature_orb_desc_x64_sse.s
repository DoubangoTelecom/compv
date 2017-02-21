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

global sym(CompVOrbBrief256_31_32f_Asm_X64_SSE41)

section .data
	extern COMPV_YASM_DLLIMPORT_DECL(k128_u8)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* img_center
; arg(1) -> compv_uscalar_t img_stride
; arg(2) -> const compv_float32_t* cos1
; arg(3) -> const compv_float32_t* sin1
; arg(4) -> const compv_float32_t* kBrief256Pattern31AX
; arg(5) -> const compv_float32_t* kBrief256Pattern31AY
; arg(6) -> const compv_float32_t* kBrief256Pattern31BX
; arg(7) -> const compv_float32_t* kBrief256Pattern31BY
; arg(8) -> void* out
sym(CompVOrbBrief256_31_32f_Asm_X64_SSE41):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 9
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
	sub rsp, (16*COMPV_YASM_INT32_SZ_BYTES) + (16*COMPV_YASM_UINT8_SZ_BYTES) + (16*COMPV_YASM_UINT8_SZ_BYTES) + (4*COMPV_YASM_XMM_SZ_BYTES)

	%define vecIndex            rsp + 0
	%define vecA				vecIndex + (16*COMPV_YASM_INT32_SZ_BYTES)
	%define vecB				vecA + (16*COMPV_YASM_UINT8_SZ_BYTES)
	%define vecCosT				vecB + (16*COMPV_YASM_UINT8_SZ_BYTES)
	%define vecSinT				vecCosT + (1*COMPV_YASM_XMM_SZ_BYTES)
	%define vecStride			vecSinT + (1*COMPV_YASM_XMM_SZ_BYTES)
	%define vec128				vecStride + (1*COMPV_YASM_XMM_SZ_BYTES)

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
	movd xmm7, rax
	pshufd xmm7, xmm7, 0x0
	movdqa [vecStride], xmm7

	; Compute vecCosT and vecSinT
	mov rax, arg(2) ; cos1
	mov rdx, arg(3) ; sin1
	movss xmm0, [rax]
	movss xmm1, [rdx]
	shufps xmm0, xmm0, 0x0
	shufps xmm1, xmm1, 0x0
	movaps[vecCosT], xmm0
	movaps[vecSinT], xmm1

	; Compute vec128 ;
	COMPV_YASM_DLLIMPORT_LOAD movdqa, xmm2, k128_u8, rax
	movdqa [vec128], xmm2

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (size_t i = 0; i < 256; i += 16)
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

			movaps xmm8, [vecSinT]
			movaps xmm9, [vecSinT]
			movaps xmm10, xmm8
			movaps xmm11, xmm8
			movaps xmm12, [vecCosT]
			movaps xmm13, [vecCosT]
			movaps xmm14, xmm12
			movaps xmm15, xmm12
			mulps xmm8, [rax + ((i + 0) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			mulps xmm9, [rax + ((i + 4) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			mulps xmm10, [rax + ((i + 8) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			mulps xmm11, [rax + ((i + 12) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			mulps xmm12, [rdx + ((i + 0) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			mulps xmm13, [rdx + ((i + 4) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			mulps xmm14, [rdx + ((i + 8) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			mulps xmm15, [rdx + ((i + 12) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			movaps xmm0, [vecCosT]
			movaps xmm1, [vecCosT]
			movaps xmm2, xmm0
			movaps xmm3, xmm0
			movaps xmm4, [vecSinT]
			movaps xmm5, [vecSinT]
			movaps xmm6, xmm4
			movaps xmm7, xmm4
			addps xmm8, xmm12
			addps xmm9, xmm13
			addps xmm10, xmm14
			addps xmm11, xmm15
			cvtps2dq xmm8, xmm8
			cvtps2dq xmm9, xmm9
			cvtps2dq xmm10, xmm10
			cvtps2dq xmm11, xmm11
			pmulld xmm8, [vecStride]
			pmulld xmm9, [vecStride]
			pmulld xmm10, [vecStride]
			pmulld xmm11, [vecStride]
			mulps xmm0, [rax + ((i + 0) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			mulps xmm4, [rdx + ((i + 0) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			mulps xmm1, [rax + ((i + 4) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			mulps xmm5, [rdx + ((i + 4) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			mulps xmm2, [rax + ((i + 8) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			mulps xmm6, [rdx + ((i + 8) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			mulps xmm3, [rax + ((i + 12) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			mulps xmm7, [rdx + ((i + 12) * COMPV_YASM_FLOAT32_SZ_BYTES)]

			subps xmm0, xmm4
			subps xmm1, xmm5
			subps xmm2, xmm6
			subps xmm3, xmm7
			cvtps2dq xmm0, xmm0
			cvtps2dq xmm1, xmm1
			cvtps2dq xmm2, xmm2
			cvtps2dq xmm3, xmm3

			

			paddd xmm8, xmm0
			movdqa[vecIndex + (0*COMPV_YASM_INT32_SZ_BYTES)], xmm8
			paddd xmm9, xmm1
			
			movdqa[vecIndex + (4*COMPV_YASM_INT32_SZ_BYTES)], xmm9
			paddd xmm10, xmm2
			movdqa[vecIndex + (8*COMPV_YASM_INT32_SZ_BYTES)], xmm10
			paddd xmm11, xmm3
			
			movdqa[vecIndex + (12*COMPV_YASM_INT32_SZ_BYTES)], xmm11
			movdqa xmm3, [vec128]
			%assign index 0 ; 0.....15
			%rep 2
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

		movdqa xmm1, [vecA]
		movdqa xmm0, [vecB] ; pcmpltb doesn''t exist -> inverse vecA/vecB
		psubb xmm0, xmm3
		psubb xmm1, xmm3
		pcmpgtb xmm0, xmm1
		pmovmskb rax, xmm0
		add i, 16
		cmp i, 256
		mov [outPtr], word ax
		lea outPtr, [outPtr + COMPV_YASM_UINT16_SZ_BYTES]
		jl .Loop256
		;; EndOf_Loop256 ;;


	%undef vecIndex
	%undef vecA
	%undef vecB
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
	add rsp, (16*COMPV_YASM_INT32_SZ_BYTES) + (16*COMPV_YASM_UINT8_SZ_BYTES) + (16*COMPV_YASM_UINT8_SZ_BYTES) + (4*COMPV_YASM_XMM_SZ_BYTES)
	COMPV_YASM_UNALIGN_STACK

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

%endif ; COMPV_YASM_ABI_IS_64BIT
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(CompVOrbBrief256_31_32f_Asm_X86_SSE41)

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
sym(CompVOrbBrief256_31_32f_Asm_X86_SSE41):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 9
	COMPV_YASM_SAVE_XMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 16, rax
	sub rsp, (16*COMPV_YASM_INT32_SZ_BYTES) + (16*COMPV_YASM_UINT8_SZ_BYTES) + (16*COMPV_YASM_UINT8_SZ_BYTES) + (4*COMPV_YASM_XMM_SZ_BYTES) + (4*COMPV_YASM_XMM_SZ_BYTES)

	%define vecIndex            rsp + 0
	%define vecA				vecIndex + (16*COMPV_YASM_INT32_SZ_BYTES)
	%define vecB				vecA + (16*COMPV_YASM_UINT8_SZ_BYTES)
	%define vecX				vecB + (16*COMPV_YASM_UINT8_SZ_BYTES)
	%define vecCosT				vecX + (4*COMPV_YASM_XMM_SZ_BYTES)
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
			movaps xmm0, [rax + ((i + 0) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			movaps xmm1, [rax + ((i + 4) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			movaps xmm2, [rax + ((i + 8) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			movaps xmm3, [rax + ((i + 12) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			movaps xmm4, [rdx + ((i + 0) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			movaps xmm5, [rdx + ((i + 4) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			movaps xmm6, [rdx + ((i + 8) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			movaps xmm7, [rdx + ((i + 12) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			mulps xmm0, [vecCosT]
			mulps xmm1, [vecCosT]
			mulps xmm2, [vecCosT]
			mulps xmm3, [vecCosT]
			mulps xmm4, [vecSinT]
			mulps xmm5, [vecSinT]
			mulps xmm6, [vecSinT]
			mulps xmm7, [vecSinT]
			subps xmm0, xmm4
			subps xmm1, xmm5
			subps xmm2, xmm6
			subps xmm3, xmm7
			cvtps2dq xmm0, xmm0
			cvtps2dq xmm1, xmm1
			cvtps2dq xmm2, xmm2
			cvtps2dq xmm3, xmm3
			movdqa[vecX + (0*COMPV_YASM_XMM_SZ_BYTES)], xmm0
			movdqa[vecX + (1*COMPV_YASM_XMM_SZ_BYTES)], xmm1
			movdqa[vecX + (2*COMPV_YASM_XMM_SZ_BYTES)], xmm2
			movdqa[vecX + (3*COMPV_YASM_XMM_SZ_BYTES)], xmm3
			;; yf = (kBrief256Pattern31AX[i] * sinT + kBrief256Pattern31AY[i] * cosT) ;;
			movaps xmm0, [rax + ((i + 0) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			movaps xmm1, [rax + ((i + 4) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			movaps xmm2, [rax + ((i + 8) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			movaps xmm3, [rax + ((i + 12) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			movaps xmm4, [rdx + ((i + 0) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			movaps xmm5, [rdx + ((i + 4) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			movaps xmm6, [rdx + ((i + 8) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			movaps xmm7, [rdx + ((i + 12) * COMPV_YASM_FLOAT32_SZ_BYTES)]
			mulps xmm0, [vecSinT]
			mulps xmm1, [vecSinT]
			mulps xmm2, [vecSinT]
			mulps xmm3, [vecSinT]
			mulps xmm4, [vecCosT]
			mulps xmm5, [vecCosT]
			mulps xmm6, [vecCosT]
			mulps xmm7, [vecCosT]
			addps xmm0, xmm4
			addps xmm1, xmm5
			addps xmm2, xmm6
			addps xmm3, xmm7
			cvtps2dq xmm0, xmm0
			cvtps2dq xmm1, xmm1
			cvtps2dq xmm2, xmm2
			cvtps2dq xmm3, xmm3
			pmulld xmm0, [vecStride]
			pmulld xmm1, [vecStride]
			pmulld xmm2, [vecStride]
			pmulld xmm3, [vecStride]
			paddd xmm0, [vecX + (0*COMPV_YASM_XMM_SZ_BYTES)]
			paddd xmm1, [vecX + (1*COMPV_YASM_XMM_SZ_BYTES)]
			paddd xmm2, [vecX + (2*COMPV_YASM_XMM_SZ_BYTES)]
			paddd xmm3, [vecX + (3*COMPV_YASM_XMM_SZ_BYTES)]
			movdqa[vecIndex + (0*COMPV_YASM_INT32_SZ_BYTES)], xmm0
			movdqa[vecIndex + (4*COMPV_YASM_INT32_SZ_BYTES)], xmm1
			movdqa[vecIndex + (8*COMPV_YASM_INT32_SZ_BYTES)], xmm2
			movdqa[vecIndex + (12*COMPV_YASM_INT32_SZ_BYTES)], xmm3
			%assign index 0 ; 0.....15
			%rep 6
				movsxd rax, dword [vecIndex + ((index+0)*COMPV_YASM_INT32_SZ_BYTES)]
				%if index < 15
					movsxd rcx, dword [vecIndex + ((index+1)*COMPV_YASM_INT32_SZ_BYTES)]
					movsxd rdx, dword [vecIndex + ((index+2)*COMPV_YASM_INT32_SZ_BYTES)]
				%endif
				movzx rax, byte [img_center + rax]
				%if index < 15
					movzx rcx, byte [img_center + rcx]
					movzx rdx, byte [img_center + rdx]
				%endif
				%if xy == 0
					mov [vecA + ((index+0)*COMPV_YASM_UINT8_SZ_BYTES)], byte al
				%else
					mov [vecB + ((index+0)*COMPV_YASM_UINT8_SZ_BYTES)], byte al
				%endif
				%if index < 15
					%if xy == 0
						mov [vecA + ((index+1)*COMPV_YASM_UINT8_SZ_BYTES)], byte cl
						mov [vecA + ((index+2)*COMPV_YASM_UINT8_SZ_BYTES)], byte dl
					%else
						mov [vecB + ((index+1)*COMPV_YASM_UINT8_SZ_BYTES)], byte cl
						mov [vecB + ((index+2)*COMPV_YASM_UINT8_SZ_BYTES)], byte dl
					%endif
				%endif
				%assign index index+3
			%endrep ; rep index
		%assign xy xy+1
		%endrep ; rep xy

		movdqa xmm0, [vecB] ; pcmpltb doesn''t exist -> inverse vecA/vecB
		movdqa xmm1, [vecA]
		psubb xmm0, [vec128]
		psubb xmm1, [vec128]
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
	add rsp, (16*COMPV_YASM_INT32_SZ_BYTES) + (16*COMPV_YASM_UINT8_SZ_BYTES) + (16*COMPV_YASM_UINT8_SZ_BYTES) + (4*COMPV_YASM_XMM_SZ_BYTES) + (4*COMPV_YASM_XMM_SZ_BYTES)
	COMPV_YASM_UNALIGN_STACK

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
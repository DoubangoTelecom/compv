;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "../../compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(Brief256_31_Asm_X86_SSE41)

section .data
	extern sym(kBrief256Pattern31AX)
	extern sym(kBrief256Pattern31AY)
	extern sym(kBrief256Pattern31BX)
	extern sym(kBrief256Pattern31BY)
	extern sym(k128_u8)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; agr(0) -> const uint8_t* img_center
; agr(1) -> compv_scalar_t img_stride
; agr(2) -> const float* cos1
; agr(3) -> const float* sin1
; agr(4) -> COMPV_ALIGNED(SSE) void* out
; void Brief256_31_Asm_X86_SSE41(const uint8_t* img_center, compv_scalar_t img_stride, float cosT, float sinT, COMPV_ALIGNED(SSE) void* out)
sym(Brief256_31_Asm_X86_SSE41):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	COMPV_YASM_SAVE_XMM 7 ;XMM[6-7]
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	%define COMPV_SIZE_OF_FLOAT 4 ; up to the caller to make sure sizeof(float)=4
	%define COMPV_SIZE_OF_UIN16	2
	%define i_xmmIndex	rsp + 0
	%define i_xmmA		rsp + 16
	%define i_xmmB		rsp + 32

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 16, rax
	sub rsp, 4*4 + 16*1 + 16*1
	; [rsp + 0] = int32_t xmmIndex[4]
	; [rsp + 16] = uint8_t xmmA[16]
	; [rsp + 32] = uint8_t xmmB[16]
	
	; rsi = i
	xor rsi, rsi
	; rcx = u8_index
	xor rcx, rcx
	; rdi = outPtr
	mov rdi, arg(4)

	; xmm7 = xmmStride = _mm_set1_epi32((int)img_stride)
	mov rax, arg(1)
	movd xmm7, rax
	pshufd xmm7, xmm7, 0x0

	; xmm6 = xmmCosT = _mm_set1_ps(*cos1)
	mov rax, arg(2)
	movss xmm6, [rax]
	shufps xmm6, xmm6, 0x0

	; xmm5 = xmmSinT = _mm_set1_ps(*sin1)
	mov rax, arg(3)
	movss xmm5, [rax]
	shufps xmm5, xmm5, 0x0

	; xmm4 = xmm128
	movdqa xmm4, [sym(k128_u8)]

	;;;;;;;;;
	;	Loop
	;;;;;;;;
	.LoopStart
		;; xmmA ;;
		lea rax, [sym(kBrief256Pattern31AX)]
		lea rbx, [sym(kBrief256Pattern31AY)]
		movaps xmm2, [rax + rsi*COMPV_SIZE_OF_FLOAT] ; xmm2 = kBrief256Pattern31AX
		movaps xmm3, [rbx + rsi*COMPV_SIZE_OF_FLOAT] ; xmm3 = kBrief256Pattern31AY
		movaps xmm0, xmm2
		movaps xmm1, xmm3
		mulps xmm0, xmm6
		mulps xmm1, xmm5
		mulps xmm2, xmm5
		mulps xmm3, xmm6
		subps xmm0, xmm1
		addps xmm2, xmm3
		cvtps2dq xmm0, xmm0
		cvtps2dq xmm2, xmm2
		pmulld xmm2, xmm7
		paddd xmm2, xmm0
		movdqa [i_xmmIndex], xmm2
		mov rax, arg(0) ; rax = img_center
		movsxd rdx, dword [i_xmmIndex + 0*4] ; rdx = xmmIndex[0]
		movzx rbx, byte [rax + rdx] ; rbx = img_center[xmmIndex[0]]
		mov [i_xmmA + rcx + 0], byte bl ; xmmA[u8_index + 0] = img_center[xmmIndex[0]]
		movsxd rdx, dword [i_xmmIndex + 1*4] ; rdx = xmmIndex[1]
		movzx rbx, byte [rax + rdx] ; rbx = img_center[xmmIndex[1]]
		mov [i_xmmA + rcx + 1], byte bl ; xmmA[u8_index + 1] = img_center[xmmIndex[1]]
		movsxd rdx, dword [i_xmmIndex + 2*4] ; rdx = xmmIndex[2]
		movzx rbx, byte [rax + rdx] ; rbx = img_center[xmmIndex[2]]
		mov [i_xmmA + rcx + 2], byte bl ; xmmA[u8_index + 2] = img_center[xmmIndex[2]]
		movsxd rdx, dword [i_xmmIndex + 3*4] ; rdx = xmmIndex[3]
		movzx rbx, byte [rax + rdx] ; rbx = img_center[xmmIndex[3]]
		mov [i_xmmA + rcx + 3], byte bl ; xmmA[u8_index + 3] = img_center[xmmIndex[3]]

		;; xmmB ;;
		lea rax, [sym(kBrief256Pattern31BX)]
		lea rbx, [sym(kBrief256Pattern31BY)]
		movaps xmm2, [rax + rsi*COMPV_SIZE_OF_FLOAT] ; xmm2 = kBrief256Pattern31BX
		movaps xmm3, [rbx + rsi*COMPV_SIZE_OF_FLOAT] ; xmm3 = kBrief256Pattern31BY
		movaps xmm0, xmm2
		movaps xmm1, xmm3
		mulps xmm0, xmm6
		mulps xmm1, xmm5
		mulps xmm2, xmm5
		mulps xmm3, xmm6
		subps xmm0, xmm1
		addps xmm2, xmm3
		cvtps2dq xmm0, xmm0
		cvtps2dq xmm2, xmm2
		pmulld xmm2, xmm7
		paddd xmm2, xmm0
		movdqa [i_xmmIndex], xmm2
		mov rax, arg(0) ; rax = img_center
		movsxd rdx, dword [i_xmmIndex + 0*4] ; rdx = xmmIndex[0]
		movzx rbx, byte [rax + rdx] ; rbx = img_center[xmmIndex[0]]
		mov [i_xmmB + rcx + 0], byte bl ; xmmB[u8_index + 0] = img_center[xmmIndex[0]]
		movsxd rdx, dword [i_xmmIndex + 1*4] ; rdx = xmmIndex[1]
		movzx rbx, byte [rax + rdx] ; rbx = img_center[xmmIndex[1]]
		mov [i_xmmB + rcx + 1], byte bl ; xmmB[u8_index + 1] = img_center[xmmIndex[1]]
		movsxd rdx, dword [i_xmmIndex + 2*4] ; rdx = xmmIndex[2]
		movzx rbx, byte [rax + rdx] ; rbx = img_center[xmmIndex[2]]
		mov [i_xmmB + rcx + 2], byte bl ; xmmB[u8_index + 2] = img_center[xmmIndex[2]]
		movsxd rdx, dword [i_xmmIndex + 3*4] ; rdx = xmmIndex[3]
		movzx rbx, byte [rax + rdx] ; rbx = img_center[xmmIndex[3]]
		mov [i_xmmB + rcx + 3], byte bl ; xmmB[u8_index + 3] = img_center[xmmIndex[3]]
	
		add rcx, 4 ; u8_index += 4
		cmp rcx, 16
		jne .EndOfComputeDescription
			; _out[i] |= (a < b) ? (u64_1 << j) : 0;
			movdqa xmm0, [i_xmmB]
			movdqa xmm1, [i_xmmA]
			psubb xmm0, xmm4
			psubb xmm1, xmm4
			pcmpgtb xmm0, xmm1
			pmovmskb ebx, xmm0
			mov [rdi], bx

			xor rcx, rcx ; u8_index = 0
			add rdi, COMPV_SIZE_OF_UIN16 ; ++outPtr
		.EndOfComputeDescription

		lea rsi, [rsi + 4]
		cmp rsi, 256
		jl .LoopStart

	; unalign stack and free memory
	add rsp, 4*4 + 16*1 + 16*1
	COMPV_YASM_UNALIGN_STACK

	%undef COMPV_SIZE_OF_FLOAT
	%undef COMPV_SIZE_OF_UIN16
	%undef i_xmmIndex
	%undef i_xmmA
	%undef i_xmmB

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
	
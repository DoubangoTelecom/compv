;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(CompVCannyNMSGatherRow_8mpw_Asm_X86_SSE41)
global sym(CompVCannyNMSApply_Asm_X86_SSE2)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> uint8_t* nms
; arg(1) -> const uint16_t* g
; arg(2) -> const int16_t* gx
; arg(3) -> const int16_t* gy
; arg(4) -> const uint16_t* tLow1
; arg(5) -> compv_uscalar_t width
; arg(6) ->  compv_uscalar_t stride
sym(CompVCannyNMSGatherRow_8mpw_Asm_X86_SSE41):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_XMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 16, rax
	sub rsp, (3*COMPV_YASM_REG_SZ_BYTES) + (11*COMPV_YASM_XMM_SZ_BYTES)

	%define c0							rsp + 0
	%define c1							c0 + COMPV_YASM_REG_SZ_BYTES
	%define widthMinus7					c1 + COMPV_YASM_REG_SZ_BYTES
	%define vecG						widthMinus7 + COMPV_YASM_REG_SZ_BYTES
	%define vecGX						vecG + COMPV_YASM_XMM_SZ_BYTES
	%define vecAbsGX0					vecGX + COMPV_YASM_XMM_SZ_BYTES
	%define vecAbsGX1					vecAbsGX0 + COMPV_YASM_XMM_SZ_BYTES
	%define vecGY						vecAbsGX1 + COMPV_YASM_XMM_SZ_BYTES
	%define vecAbsGY0					vecGY + COMPV_YASM_XMM_SZ_BYTES
	%define vecAbsGY1					vecAbsGY0 + COMPV_YASM_XMM_SZ_BYTES
	%define vecTLow						vecAbsGY1 + COMPV_YASM_XMM_SZ_BYTES
	%define vecZero						vecTLow + COMPV_YASM_XMM_SZ_BYTES
	%define vecTangentPiOver8Int		vecZero + COMPV_YASM_XMM_SZ_BYTES
	%define vecTangentPiTimes3Over8Int	vecTangentPiOver8Int + COMPV_YASM_XMM_SZ_BYTES

	%define col							rcx
	%define nms							rdx
	%define g							rbx
	%define gx							rsi
	%define gy							rdi
	%define vecNMS						xmm7
	
	mov rax, arg(4) ; tLow1
	mov rcx, 27145 ; kCannyTangentPiOver8Int
	mov rbx, 158217; kCannyTangentPiTimes3Over8Int
	movzx edx, word [rax]
	movd xmm0, edx
	movd xmm1, ecx
	movd xmm2, ebx
	punpcklwd xmm0, xmm0
	pxor xmm3, xmm3
	pshufd xmm0, xmm0, 0x0
	pshufd xmm1, xmm1, 0x0
	pshufd xmm2, xmm2, 0x0
	movdqa [vecTLow], xmm0
	movdqa [vecTangentPiOver8Int], xmm1
	movdqa [vecTangentPiTimes3Over8Int], xmm2
	movdqa [vecZero], xmm3

	mov rax, arg(6) ; stride
	mov rdi, arg(5) ; width
	mov rbx, 1
	lea rcx, [rax + 1]
	lea rsi, [rdi - 7]
	sub rbx, rax
	mov [c1], rcx
	mov [c0], rbx
	mov [widthMinus7], rsi
	
	mov nms, arg(0)
	mov g, arg(1)
	mov gx, arg(2)
	mov gy, arg(3)
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (col = 1; col < width - 7; col += 8)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov col, 1
	.LoopWidth:
		movdqu xmm0, [g + (col * COMPV_YASM_UINT16_SZ_BYTES)]
		movdqa xmm2, xmm0 ; xmm2 = vecG
		pcmpgtw xmm0, [vecTLow] ; xmm0 = vec0
		pmovmskb rax, xmm0
		test rax, rax
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (_mm_movemask_epi8(vec0))
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		jz .EndOf_Ifvec00
		.Ifvec00:
			movdqa [vecG], xmm2 ; save vecG
			movdqu xmm5, [gy + (col * COMPV_YASM_INT16_SZ_BYTES)] ; vecGY
			movdqu xmm6, [gx + (col * COMPV_YASM_INT16_SZ_BYTES)] ; vecGX
			pxor vecNMS, vecNMS

			pabsw xmm2, xmm5
			pabsw xmm1, xmm6
			pxor xmm3, xmm3
			pxor xmm4, xmm4
			punpcklwd xmm3, xmm2 ; vecAbsGY0
			punpckhwd xmm4, xmm2 ; vecAbsGY1
			movdqa xmm2, xmm1
			punpcklwd xmm1, [vecZero] ; vecAbsGX0
			punpckhwd xmm2, [vecZero] ; vecAbsGX1

			movdqa [vecGY], xmm5
			movdqa [vecGX], xmm6
			movdqa [vecAbsGY0], xmm3
			movdqa [vecAbsGY1], xmm4
			movdqa [vecAbsGX0], xmm1
			movdqa [vecAbsGX1], xmm2

			;; angle = "0° / 180°" ;;
			pmulld xmm1, [vecTangentPiOver8Int]
			pmulld xmm2, [vecTangentPiOver8Int]
			pcmpgtd xmm1, xmm3
			pcmpgtd xmm2, xmm4
			packssdw xmm1, xmm2
			pand xmm1, xmm0 ; xmm1 = vec3
			pmovmskb rax, xmm1
			test rax, rax
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; if (_mm_movemask_epi8(vec3))
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			jz .EndOf_Ifvec30
			.Ifvec30:
				movdqa xmm2, [g + (col - 1)*COMPV_YASM_UINT16_SZ_BYTES]
				movdqu xmm3, [g + (col + 1)*COMPV_YASM_UINT16_SZ_BYTES]
				pcmpgtw xmm2, [vecG]
				pcmpgtw xmm3, [vecG]
				por xmm2, xmm3
				pand xmm2, xmm1 ; xmm1 is vec3
				packsswb xmm2, xmm2
				por vecNMS, xmm2
				.EndOf_Ifvec30:
				;; EndOf_Ifvec30 ;;

			;; angle = "45° / 225°" or "135 / 315" ;;
			movdqa xmm2, xmm1 ; xmm1 is vec3
			pandn xmm2, xmm0 ; xmm2 = vec4
			pmovmskb rax, xmm2
			test rax, rax
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; if (_mm_movemask_epi8(vec4)) - 0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			jz .EndOf_Ifvec40
			.Ifvec40:
				movdqa xmm3, [vecTangentPiTimes3Over8Int]
				movdqa xmm4, xmm3
				pmulld xmm3, [vecAbsGX0]
				pmulld xmm4, [vecAbsGX1]
				pcmpgtd xmm3, [vecAbsGY0]
				pcmpgtd xmm4, [vecAbsGY1]
				packssdw xmm3, xmm4
				pand xmm2, xmm3 ; xmm2 = old vec4, override
				pmovmskb rax, xmm2
				test rax, rax
				;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				; if (_mm_movemask_epi8(vec4)) - 1
				;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				jz .EndOf_Ifvec41
				.Ifvec41:
					movdqa xmm6, [vecGX]
					pxor xmm4, xmm4 ; vecZero
					pxor xmm6, [vecGY]
					pcmpgtw xmm4, xmm6
					pand xmm4, xmm2 ; xmm2 is vec4, xmm4 = vec1
					movdqa xmm5, xmm4
					pmovmskb rax, xmm4
					pandn xmm5, xmm2 ; xmm2 is vec4, xmm5 = vec2
					test rax, rax
					;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
					; if (_mm_movemask_epi8(vec1))
					;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
					jz .EndOf_Ifvec10
					.Ifvec10:
						mov rax, col
						sub rax, [c0]
						movdqa xmm6, [g + rax*COMPV_YASM_UINT16_SZ_BYTES]
						mov rax, col
						add rax, [c0]
						movdqu xmm3, [g + rax*COMPV_YASM_UINT16_SZ_BYTES]
						pcmpgtw xmm6, [vecG]
						pcmpgtw xmm3, [vecG]
						por xmm6, xmm3
						pand xmm4, xmm6 ; xmm4 is old vec1
						.EndOf_Ifvec10:
						;; EndOf_Ifvec10 ;;

					pmovmskb rax, xmm5 ; xmm5 is vec2
					test rax, rax
					;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
					; if (_mm_movemask_epi8(vec2))
					;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
					jz .EndOfIfvec20
					.Ifvec20:
						mov rax, col
						sub rax, [c1]
						movdqa xmm6, [g + rax*COMPV_YASM_UINT16_SZ_BYTES]
						mov rax, col
						add rax, [c1]
						movdqu xmm3, [g + rax*COMPV_YASM_UINT16_SZ_BYTES]
						pcmpgtw xmm6, [vecG]
						pcmpgtw xmm3, [vecG]
						por xmm6, xmm3
						pand xmm5, xmm6 ; xmm5 is old vec2
						.EndOfIfvec20:
						;; EndOfIfvec20 ;;

					por xmm4, xmm5 ; xmm4 is vec1 and xmm5 is vec2
					packsswb xmm4, xmm4
					por vecNMS, xmm4
					.EndOf_Ifvec41:
					;; EndOf_Ifvec41 ;;

				.EndOf_Ifvec40:
				;; EndOf_Ifvec40 ;;
			
			;; angle = "90° / 270°" ;;
			pandn xmm2, xmm0 ; xmm2 was vec4 and xmm0 is vec0
			pandn xmm1, xmm2 ; xmm1 was vec3, now vec5 is xmm1
			pmovmskb rax, xmm1
			test rax, rax
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; if (_mm_movemask_epi8(vec5))
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			jz .EndOf_Ifvec50
			.Ifvec50:
				mov rax, col
				sub rax, arg(6) ; sub stride
				movdqu xmm6, [g + rax*COMPV_YASM_UINT16_SZ_BYTES]
				mov rax, col
				add rax, arg(6) ; add stride
				movdqu xmm3, [g + rax*COMPV_YASM_UINT16_SZ_BYTES]
				pcmpgtw xmm6, [vecG]
				pcmpgtw xmm3, [vecG]
				por xmm6, xmm3
				pand xmm1, xmm6 ; xmm1 is old vec5
				packsswb xmm1, xmm1
				por vecNMS, xmm1
				.EndOf_Ifvec50:
				;; EndOf_Ifvec50 ;;
			
			movq [nms + col*COMPV_YASM_UINT8_SZ_BYTES], vecNMS

			.EndOf_Ifvec00:
			;; EndOf_Ifvec00 ;;

		add col, 8
		cmp col, [widthMinus7]
		jl .LoopWidth
		;; EndOf_LoopWidth ;;


	%undef c0						
	%undef c1
	%undef widthMinus7											
	%undef vecG						
	%undef vecGX						
	%undef vecAbsGX0					
	%undef vecAbsGX1					
	%undef vecGY						
	%undef vecAbsGY0					
	%undef vecAbsGY1					
	%undef vecTLow						
	%undef vecZero						
	%undef vecTangentPiOver8Int		
	%undef vecTangentPiTimes3Over8Int	

	%undef col
	%undef nms
	%undef g
	%undef gx
	%undef gy
	%undef vecNMS

	; free memory and unalign stack
	add rsp, (3*COMPV_YASM_REG_SZ_BYTES) + (11*COMPV_YASM_XMM_SZ_BYTES)
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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) uint16_t* grad
; arg(1) -> COMPV_ALIGNED(SSE) uint8_t* nms
; arg(2) -> compv_uscalar_t width
; arg(3) -> compv_uscalar_t height
; arg(4) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
sym(CompVCannyNMSApply_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	pxor xmm0, xmm0 ; xmm0 = vecZero
	mov rbx, arg(0) ; rbx = grad
	mov rdx, arg(1) ; rdx = nms
	mov rdi, arg(2) ; rdi = width
	mov rsi, arg(3) ; rsi = height
	; rcx = i
	; rax = local variable

	; row starts to #1
	dec rsi
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (row_ = 1; row_ < height; ++row_)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (col_ = 0; col_ < width; col_ += 8)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor rcx, rcx ; i = 0
		.LoopWidth:
			movq xmm1, [rdx + rcx*COMPV_YASM_UINT8_SZ_BYTES]
			pcmpeqb xmm1, xmm0
			pmovmskb eax, xmm1
			xor eax, 0xffff
			jz .NothingToSupress
				punpcklbw xmm1, xmm1
				pand xmm1, [rbx + rcx*COMPV_YASM_UINT16_SZ_BYTES]
				movq [rdx + rcx*COMPV_YASM_UINT8_SZ_BYTES], xmm0
				movdqa [rbx + rcx*COMPV_YASM_UINT16_SZ_BYTES], xmm1
				.NothingToSupress:

			add rcx, 8
			cmp rcx, rdi
			jl .LoopWidth
			;; EndOf_LoopWidth ;;

		mov rax, arg(4) ; stride
		dec rsi
		lea rdx, [rdx + rax*COMPV_YASM_UINT8_SZ_BYTES] ; nms += stride
		lea rbx, [rbx + rax*COMPV_YASM_UINT16_SZ_BYTES] ; grad += stride
		jnz .LoopHeight
		; EndOf_LoopHeight ;;

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

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

global sym(CompVCannyNMSGatherRow_8mpw_Asm_X64_SSE41)

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
sym(CompVCannyNMSGatherRow_8mpw_Asm_X64_SSE41):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
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
	sub rsp, (3*COMPV_YASM_XMM_SZ_BYTES)

	%define vecTLow						rsp + 0
	%define vecTangentPiOver8Int		vecTLow + COMPV_YASM_XMM_SZ_BYTES
	%define vecTangentPiTimes3Over8Int	vecTangentPiOver8Int + COMPV_YASM_XMM_SZ_BYTES


	%define col							rcx
	%define nms							rdx
	%define g							rbx
	%define gx							rsi
	%define gy							rdi
	%define c0							r8
	%define c1							r9
	%define widthMinus7					r10
	%define stride						r11
	%define minusStride					r12
	%define minusc0						r13
	%define minusc1						r14
	%define vecNMS						xmm7
	%define vecG						xmm8
	%define vecZero						xmm9
	%define vecGX						xmm10
	%define vecAbsGX0					xmm11
	%define vecAbsGX1					xmm12
	%define vecGY						xmm13
	%define vecAbsGY0					xmm14
	%define vecAbsGY1					xmm15

	
	mov rax, arg(4) ; tLow1
	mov rcx, 27145 ; kCannyTangentPiOver8Int
	mov rbx, 158217; kCannyTangentPiTimes3Over8Int
	movzx edx, word [rax]
	movd xmm0, edx
	movd xmm1, ecx
	movd xmm2, ebx
	punpcklwd xmm0, xmm0
	pxor vecZero, vecZero
	pshufd xmm0, xmm0, 0x0
	pshufd xmm1, xmm1, 0x0
	pshufd xmm2, xmm2, 0x0
	movdqa [vecTLow], xmm0
	movdqa [vecTangentPiOver8Int], xmm1
	movdqa [vecTangentPiTimes3Over8Int], xmm2

	mov stride, arg(6)
	mov rax, arg(5) ; width
	mov c0, 1
	lea c1, [stride + 1]
	lea widthMinus7, [rax - 7]
	sub c0, stride

	mov minusStride, stride
	mov minusc0, c0
	mov minusc1, c1
	neg minusStride
	neg minusc0
	neg minusc1
	
	mov nms, arg(0)
	mov g, arg(1)
	mov gx, arg(2)
	mov gy, arg(3)
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (col = 1; col < width - 7; col += 8)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov col, 1
	.LoopWidth:
		movdqu vecG, [g + (col * COMPV_YASM_UINT16_SZ_BYTES)]
		movdqa xmm0, vecG
		pcmpgtw xmm0, [vecTLow] ; xmm0 = vec0
		pmovmskb rax, xmm0
		test rax, rax
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (_mm_movemask_epi8(vec0))
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		jz .EndOf_Ifvec00
		.Ifvec00:
			movdqu vecGY, [gy + (col * COMPV_YASM_INT16_SZ_BYTES)]
			movdqu vecGX, [gx + (col * COMPV_YASM_INT16_SZ_BYTES)]
			movdqa xmm1, [vecTangentPiOver8Int]
			pxor vecNMS, vecNMS
			movdqa xmm2, xmm1

			pabsw xmm3, vecGY
			pabsw vecAbsGX0, vecGX
			pxor vecAbsGY0, vecAbsGY0
			pxor vecAbsGY1, vecAbsGY1
			punpcklwd vecAbsGY0, xmm3
			punpckhwd vecAbsGY1, xmm3
			movdqa vecAbsGX1, vecAbsGX0
			punpcklwd vecAbsGX0, vecZero
			punpckhwd vecAbsGX1, vecZero

			;; angle = "0° / 180°" ;;
			pmulld xmm1, vecAbsGX0
			pmulld xmm2, vecAbsGX1
			pcmpgtd xmm1, vecAbsGY0
			pcmpgtd xmm2, vecAbsGY1
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
				pcmpgtw xmm2, vecG
				pcmpgtw xmm3, vecG
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
				pmulld xmm3, vecAbsGX0
				pmulld xmm4, vecAbsGX1
				pcmpgtd xmm3, vecAbsGY0
				pcmpgtd xmm4, vecAbsGY1
				packssdw xmm3, xmm4
				pand xmm2, xmm3 ; xmm2 = old vec4, override
				pmovmskb rax, xmm2
				test rax, rax
				;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				; if (_mm_movemask_epi8(vec4)) - 1
				;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				jz .EndOf_Ifvec41
				.Ifvec41:
					movdqa xmm6, vecGX
					pxor xmm4, xmm4 ; vecZero
					pxor xmm6, vecGY
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
						lea rax, [col + minusc0]
						lea r15, [col + c0]
						movdqa xmm6, [g + rax*COMPV_YASM_UINT16_SZ_BYTES]			
						movdqu xmm3, [g + r15*COMPV_YASM_UINT16_SZ_BYTES]
						pcmpgtw xmm6, vecG
						pcmpgtw xmm3, vecG
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
						lea rax, [col + minusc1]
						lea r15, [col + c1]
						movdqa xmm6, [g + rax*COMPV_YASM_UINT16_SZ_BYTES]
						movdqu xmm3, [g + r15*COMPV_YASM_UINT16_SZ_BYTES]
						pcmpgtw xmm6, vecG
						pcmpgtw xmm3, vecG
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
				lea rax, [col + minusStride]
				lea r15, [col + stride]
				movdqu xmm6, [g + rax*COMPV_YASM_UINT16_SZ_BYTES]
				movdqu xmm3, [g + r15*COMPV_YASM_UINT16_SZ_BYTES]
				pcmpgtw xmm6, vecG
				pcmpgtw xmm3, vecG
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
		cmp col, widthMinus7
		jl .LoopWidth
		;; EndOf_LoopWidth ;;
											
			
	%undef vecTLow								
	%undef vecTangentPiOver8Int		
	%undef vecTangentPiTimes3Over8Int	

	%undef col
	%undef nms
	%undef g
	%undef gx
	%undef gy
	%undef c0						
	%undef c1
	%undef widthMinus7
	%undef stride
	%undef minusStride					
	%undef minusc0						
	%undef minusc1						
	%undef vecNMS
	%undef vecG
	%undef vecZero
	%undef vecGX						
	%undef vecAbsGX0					
	%undef vecAbsGX1					
	%undef vecGY						
	%undef vecAbsGY0					
	%undef vecAbsGY1	

	; free memory and unalign stack
	add rsp, (3*COMPV_YASM_XMM_SZ_BYTES)
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
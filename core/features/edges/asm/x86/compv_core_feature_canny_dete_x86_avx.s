;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(CompVCannyNMSGatherRow_16mpw_Asm_X86_AVX2)

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
sym(CompVCannyNMSGatherRow_16mpw_Asm_X86_AVX2):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_YMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 32, rax
	sub rsp, (3*COMPV_YASM_REG_SZ_BYTES) + (11*COMPV_YASM_YMM_SZ_BYTES)

	%define c0							rsp + 0
	%define c1							c0 + COMPV_YASM_REG_SZ_BYTES
	%define widthMinus15					c1 + COMPV_YASM_REG_SZ_BYTES
	%define vecG						widthMinus15 + COMPV_YASM_REG_SZ_BYTES
	%define vecGX						vecG + COMPV_YASM_YMM_SZ_BYTES
	%define vecAbsGX0					vecGX + COMPV_YASM_YMM_SZ_BYTES
	%define vecAbsGX1					vecAbsGX0 + COMPV_YASM_YMM_SZ_BYTES
	%define vecGY						vecAbsGX1 + COMPV_YASM_YMM_SZ_BYTES
	%define vecAbsGY0					vecGY + COMPV_YASM_YMM_SZ_BYTES
	%define vecAbsGY1					vecAbsGY0 + COMPV_YASM_YMM_SZ_BYTES
	%define vecTLow						vecAbsGY1 + COMPV_YASM_YMM_SZ_BYTES
	%define vecZero						vecTLow + COMPV_YASM_YMM_SZ_BYTES
	%define vecTangentPiOver8Int		vecZero + COMPV_YASM_YMM_SZ_BYTES
	%define vecTangentPiTimes3Over8Int	vecTangentPiOver8Int + COMPV_YASM_YMM_SZ_BYTES

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
	vmovd xmm0, edx
	vmovd xmm1, ecx
	vmovd xmm2, ebx
	vpxor ymm3, ymm3
	vpbroadcastw ymm0, xmm0
	vpbroadcastd ymm1, xmm1
	vpbroadcastd ymm2, xmm2
	vmovdqa [vecTLow], ymm0
	vmovdqa [vecTangentPiOver8Int], ymm1
	vmovdqa [vecTangentPiTimes3Over8Int], ymm2
	vmovdqa [vecZero], ymm3

	mov rax, arg(6) ; stride
	mov rdi, arg(5) ; width
	mov rbx, 1
	lea rcx, [rax + 1]
	lea rsi, [rdi - 15]
	sub rbx, rax
	mov [c1], rcx
	mov [c0], rbx
	mov [widthMinus15], rsi
	
	mov nms, arg(0)
	mov g, arg(1)
	mov gx, arg(2)
	mov gy, arg(3)
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (col = 1; col < width - 7; col += 8)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov col, 1
	.LoopWidth:
		vmovdqu ymm2, [g + (col * COMPV_YASM_UINT16_SZ_BYTES)] ; ymm2 = vecG
		vpcmpgtw ymm0, ymm2, [vecTLow] ; ymm0 = vec0
		vptest ymm0, ymm0
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (_mm_movemask_epi8(vec0))
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		jz .EndOf_Ifvec00
		.Ifvec00:
			vmovdqa [vecG], ymm2 ; save vecG
			vmovdqu ymm5, [gy + (col * COMPV_YASM_INT16_SZ_BYTES)] ; vecGY
			vmovdqu ymm6, [gx + (col * COMPV_YASM_INT16_SZ_BYTES)] ; vecGX
			vpxor vecNMS, vecNMS

			vpabsw ymm1, ymm5
			vpabsw ymm2, ymm6
			vpermq ymm1, ymm1, 0xD8
			vpxor ymm3, ymm3
			vpxor ymm4, ymm4
			vpunpcklwd ymm3, ymm1 ; vecAbsGY0
			vpunpckhwd ymm4, ymm1 ; vecAbsGY1	
			vpmovzxwd ymm1, xmm2 ; vecAbsGX0
			vextractf128 xmm2, ymm2, 0x1
			vpmovzxwd ymm2, xmm2 ; vecAbsGX1

			vmovdqa [vecGY], ymm5
			vmovdqa [vecGX], ymm6
			vmovdqa [vecAbsGY0], ymm3
			vmovdqa [vecAbsGY1], ymm4
			vmovdqa [vecAbsGX1], ymm2
			vmovdqa [vecAbsGX0], ymm1
			

			;; angle = "0° / 180°" ;;
			vpmulld ymm1, [vecTangentPiOver8Int]
			vpmulld ymm2, [vecTangentPiOver8Int]
			vpcmpgtd ymm1, ymm3
			vpcmpgtd ymm2, ymm4
			vpermq ymm6, ymm0, 0xD8
			vpackssdw ymm1, ymm2
			vpand ymm1, ymm6 ; ymm1 = vec3
			vptest ymm1, ymm1
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; if (_mm_movemask_epi8(vec3))
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			jz .EndOf_Ifvec30
			.Ifvec30:
				vmovdqa ymm2, [g + (col - 1)*COMPV_YASM_UINT16_SZ_BYTES]
				vmovdqu ymm3, [g + (col + 1)*COMPV_YASM_UINT16_SZ_BYTES]
				vpermq ymm1, ymm1, 0xD8
				vpcmpgtw ymm2, [vecG]
				vpcmpgtw ymm3, [vecG]
				vpor ymm2, ymm3
				vpand ymm2, ymm1 ; ymm1 is vec3
				vpacksswb ymm2, ymm2
				vpermq ymm2, ymm2, 0xD8
				vpor vecNMS, xmm2
				.EndOf_Ifvec30:
				;; EndOf_Ifvec30 ;;

			;; angle = "45° / 225°" or "135 / 315" ;;
			vpandn ymm2, ymm1, ymm0 ; ymm2 = vec4 and ymm1 is vec3
			vptest ymm2, ymm2
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; if (_mm_movemask_epi8(vec4)) - 0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			jz .EndOf_Ifvec40
			.Ifvec40:
				vmovdqa ymm4, [vecTangentPiTimes3Over8Int]
				vpmulld ymm3, ymm4, [vecAbsGX0]
				vpmulld ymm4, ymm4, [vecAbsGX1]
				vpcmpgtd ymm3, [vecAbsGY0]
				vpcmpgtd ymm4, [vecAbsGY1]
				vpackssdw ymm3, ymm4
				vpermq ymm2, ymm2, 0xD8 ; ymm2 = old vec4, override
				vpand ymm2, ymm3 ; ymm2 = old vec4, override
				vptest ymm2, ymm2
				;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				; if (_mm_movemask_epi8(vec4)) - 1
				;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				jz .EndOf_Ifvec41
				.Ifvec41:
					vpermq ymm2, ymm2, 0xD8 ; ymm2 = old vec4, override
					vmovdqa ymm6, [vecGX]
					vpxor ymm4, ymm4 ; vecZero
					vpxor ymm6, [vecGY]
					vpcmpgtw ymm4, ymm6
					vpand ymm4, ymm2 ; ymm2 is vec4, ymm4 = vec1
					vptest ymm4, ymm4
					vpandn ymm5, ymm4, ymm2 ; ymm2 is vec4, ymm5 = vec2
					;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
					; if (_mm_movemask_epi8(vec1))
					;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
					jz .EndOf_Ifvec10
					.Ifvec10:
						mov rax, col
						sub rax, [c0]
						vmovdqa ymm6, [g + rax*COMPV_YASM_UINT16_SZ_BYTES]
						mov rax, col
						add rax, [c0]
						vmovdqu ymm3, [g + rax*COMPV_YASM_UINT16_SZ_BYTES]
						vpcmpgtw ymm6, [vecG]
						vpcmpgtw ymm3, [vecG]
						vpor ymm6, ymm3
						vpand ymm4, ymm6 ; ymm4 is old vec1
						.EndOf_Ifvec10:
						;; EndOf_Ifvec10 ;;

					vptest ymm5, ymm5 ; ymm5 is vec2
					;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
					; if (_mm_movemask_epi8(vec2))
					;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
					jz .EndOfIfvec20
					.Ifvec20:
						mov rax, col
						sub rax, [c1]
						vmovdqa ymm6, [g + rax*COMPV_YASM_UINT16_SZ_BYTES]
						mov rax, col
						add rax, [c1]
						vmovdqu ymm3, [g + rax*COMPV_YASM_UINT16_SZ_BYTES]
						vpcmpgtw ymm6, [vecG]
						vpcmpgtw ymm3, [vecG]
						vpor ymm6, ymm3
						vpand ymm5, ymm6 ; ymm5 is old vec2
						.EndOfIfvec20:
						;; EndOfIfvec20 ;;

					vpor ymm4, ymm5 ; ymm4 is vec1 and ymm5 is vec2
					vpacksswb ymm4, ymm4
					vpermq ymm4, ymm4, 0xD8
					vpor vecNMS, xmm4
					.EndOf_Ifvec41:
					;; EndOf_Ifvec41 ;;

				.EndOf_Ifvec40:
				;; EndOf_Ifvec40 ;;
			
			;; angle = "90° / 270°" ;;
			vpandn ymm2, ymm0 ; ymm2 was vec4 and ymm0 is vec0
			vpandn ymm1, ymm2 ; ymm1 was vec3, now vec5 is ymm1
			vptest ymm1, ymm1
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; if (_mm_movemask_epi8(vec5))
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			jz .EndOf_Ifvec50
			.Ifvec50:
				mov rax, col
				sub rax, arg(6) ; sub stride
				vmovdqu ymm6, [g + rax*COMPV_YASM_UINT16_SZ_BYTES]
				mov rax, col
				add rax, arg(6) ; add stride
				vmovdqu ymm3, [g + rax*COMPV_YASM_UINT16_SZ_BYTES]
				vpcmpgtw ymm6, [vecG]
				vpcmpgtw ymm3, [vecG]
				vpor ymm6, ymm3
				vpand ymm1, ymm6 ; ymm1 is old vec5
				vpacksswb ymm1, ymm1
				vpermq ymm1, ymm1, 0xD8
				vpor vecNMS, xmm1
				.EndOf_Ifvec50:
				;; EndOf_Ifvec50 ;;
			
			vmovdqu [nms + col*COMPV_YASM_UINT8_SZ_BYTES], vecNMS

			.EndOf_Ifvec00:
			;; EndOf_Ifvec00 ;;

		add col, 16
		cmp col, [widthMinus15]
		jl .LoopWidth
		;; EndOf_LoopWidth ;;


	%undef c0						
	%undef c1
	%undef widthMinus15											
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
	add rsp, (3*COMPV_YASM_REG_SZ_BYTES) + (11*COMPV_YASM_YMM_SZ_BYTES)
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

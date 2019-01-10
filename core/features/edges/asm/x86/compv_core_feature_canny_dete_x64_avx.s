;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(CompVCannyNMSGatherRow_16mpw_Asm_X64_AVX2)

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
sym(CompVCannyNMSGatherRow_16mpw_Asm_X64_AVX2):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
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
	sub rsp, (3*COMPV_YASM_YMM_SZ_BYTES)

	%define vecTLow						rsp + 0
	%define vecTangentPiOver8Int		vecTLow + COMPV_YASM_YMM_SZ_BYTES
	%define vecTangentPiTimes3Over8Int	vecTangentPiOver8Int + COMPV_YASM_YMM_SZ_BYTES


	%define col							rcx
	%define nms							rdx
	%define g							rbx
	%define gx							rsi
	%define gy							rdi
	%define c0							r8
	%define c1							r9
	%define widthMinus15					r10
	%define stride						r11
	%define minusStride					r12
	%define minusc0						r13
	%define minusc1						r14
	%define vecNMS						xmm7
	%define vecG						ymm8
	%define vecZero						ymm9
	%define vecGX						ymm10
	%define vecAbsGX0					ymm11
	%define vecAbsGX1					ymm12
	%define vecGY						ymm13
	%define vecAbsGY0					ymm14
	%define vecAbsGY1					ymm15
	
	mov rax, arg(4) ; tLow1
	mov rcx, 27145 ; kCannyTangentPiOver8Int
	mov rbx, 158217; kCannyTangentPiTimes3Over8Int
	movzx edx, word [rax]
	vmovd xmm0, edx
	vmovd xmm1, ecx
	vmovd xmm2, ebx
	vpxor vecZero, vecZero
	vpbroadcastw ymm0, xmm0
	vpbroadcastd ymm1, xmm1
	vpbroadcastd ymm2, xmm2
	vmovdqa [vecTLow], ymm0
	vmovdqa [vecTangentPiOver8Int], ymm1
	vmovdqa [vecTangentPiTimes3Over8Int], ymm2

	mov stride, arg(6)
	mov rax, arg(5) ; width
	mov c0, 1
	lea c1, [stride + 1]
	lea widthMinus15, [rax - 15]
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
		vmovdqu vecG, [g + (col * COMPV_YASM_UINT16_SZ_BYTES)]
		vpcmpgtw ymm0, vecG, [vecTLow] ; ymm0 = vec0
		vptest ymm0, ymm0
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (_mm_movemask_epi8(vec0))
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		jz .EndOf_Ifvec00
		.Ifvec00:
			vmovdqu vecGY, [gy + (col * COMPV_YASM_INT16_SZ_BYTES)]
			vmovdqu vecGX, [gx + (col * COMPV_YASM_INT16_SZ_BYTES)]
			vmovdqa ymm2, [vecTangentPiOver8Int]
			vpxor vecNMS, vecNMS

			vpabsw ymm3, vecGY
			vpabsw vecAbsGX0, vecGX
			vpermq ymm3, ymm3, 0xD8
			vpxor vecAbsGY0, vecAbsGY0
			vpxor vecAbsGY1, vecAbsGY1
			vpunpcklwd vecAbsGY0, ymm3
			vpunpckhwd vecAbsGY1, ymm3
			vextractf128 xmm3, vecAbsGX0, 0x1
			vpmovzxwd vecAbsGX0, xmm11
			vpmovzxwd vecAbsGX1, xmm3

			;; angle = "0° / 180°" ;;
			vpmulld ymm1, ymm2, vecAbsGX0
			vpmulld ymm2, ymm2, vecAbsGX1
			vpcmpgtd ymm1, vecAbsGY0
			vpcmpgtd ymm2, vecAbsGY1
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
				vpcmpgtw ymm2, vecG
				vpcmpgtw ymm3, vecG
				vpor ymm2, ymm3
				vpand ymm2, ymm1 ; ymm1 is vec3
				vpacksswb ymm2, ymm2
				vpermq ymm2, ymm2, 0xD8
				vpor vecNMS, xmm2
				.EndOf_Ifvec30:
				;; EndOf_Ifvec30 ;;

			;; angle = "45° / 225°" or "135 / 315" ;;
			vpandn ymm2, ymm1, ymm0 ; ymm2 = vec4, ymm1 is vec3
			vptest ymm2, ymm2
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; if (_mm_movemask_epi8(vec4)) - 0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			jz .EndOf_Ifvec40
			.Ifvec40:
				vmovdqa ymm4, [vecTangentPiTimes3Over8Int]
				vpmulld ymm3, ymm4, vecAbsGX0
				vpmulld ymm4, ymm4, vecAbsGX1
				vpcmpgtd ymm3, vecAbsGY0
				vpcmpgtd ymm4, vecAbsGY1
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
					vpxor ymm6, vecGX, vecGY
					vpcmpgtw ymm4, vecZero, ymm6
					vpand ymm4, ymm2 ; ymm2 is vec4, ymm4 = vec1
					vptest ymm4, ymm4
					vpandn ymm5, ymm4, ymm2 ; ymm2 is vec4, ymm5 = vec2
					;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
					; if (_mm_movemask_epi8(vec1))
					;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
					jz .EndOf_Ifvec10
					.Ifvec10:
						lea rax, [col + minusc0]
						lea r15, [col + c0]
						vmovdqa ymm6, [g + rax*COMPV_YASM_UINT16_SZ_BYTES]			
						vmovdqu ymm3, [g + r15*COMPV_YASM_UINT16_SZ_BYTES]
						vpcmpgtw ymm6, vecG
						vpcmpgtw ymm3, vecG
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
						lea rax, [col + minusc1]
						lea r15, [col + c1]
						vmovdqa ymm6, [g + rax*COMPV_YASM_UINT16_SZ_BYTES]
						vmovdqu ymm3, [g + r15*COMPV_YASM_UINT16_SZ_BYTES]
						vpcmpgtw ymm6, vecG
						vpcmpgtw ymm3, vecG
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
				lea rax, [col + minusStride]
				lea r15, [col + stride]
				vmovdqu ymm6, [g + rax*COMPV_YASM_UINT16_SZ_BYTES]
				vmovdqu ymm3, [g + r15*COMPV_YASM_UINT16_SZ_BYTES]
				vpcmpgtw ymm6, vecG
				vpcmpgtw ymm3, vecG
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
		cmp col, widthMinus15
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
	%undef widthMinus15
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
	add rsp, (3*COMPV_YASM_YMM_SZ_BYTES)
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


%endif ; COMPV_YASM_ABI_IS_64BIT
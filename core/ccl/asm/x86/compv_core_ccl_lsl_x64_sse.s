;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_X64_SSSE3)

section .data
	extern sym(kShuffleEpi8_DUP_16s7_32s)
	extern sym(kShuffleEpi8_DUP_SHL0_8u_32s)
	extern sym(kShuffleEpi8_DUP_SHL1_8u_32s)
	extern sym(kShuffleEpi8_DUP_SHL2_8u_32s)
	extern sym(kShuffleEpi8_DUP_SHL3_8u_32s)
	extern sym(kShuffleEpi8_DUP_SHL4_8u_32s)
	extern sym(kShuffleEpi8_DUP_SHL5_8u_32s)
	extern sym(kShuffleEpi8_DUP_SHL6_8u_32s)
	extern sym(kShuffleEpi8_DUP_SHL7_8u_32s)
	extern sym(kShuffleEpi8_DUP_SHL8_8u_32s)
	extern sym(kShuffleEpi8_DUP_SHL9_8u_32s)
	extern sym(kShuffleEpi8_DUP_SHL10_8u_32s)
	extern sym(kShuffleEpi8_DUP_SHL11_8u_32s)
	extern sym(kShuffleEpi8_DUP_SHL12_8u_32s)
	extern sym(kShuffleEpi8_DUP_SHL13_8u_32s)
	extern sym(kShuffleEpi8_DUP_SHL14_8u_32s)
	extern sym(kShuffleEpi8_DUP_SHL15_8u_32s)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(AVX) const uint8_t* Xi
; arg(1) -> const compv_uscalar_t Xi_stride,
; arg(2) -> int16_t* ERi, 
; arg(3) -> const compv_uscalar_t ERi_stride,
; arg(4) -> int16_t* ner, 
; arg(5) -> int16_t* ner_max1, 
; arg(6) -> int32_t* ner_sum1,
; arg(7) -> const compv_uscalar_t width, 
; arg(8) -> const compv_uscalar_t height
sym(CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_X64_SSSE3):
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

	%define Xi				rax
	%define Xi_stride		rcx
	%define ERi				rdx
	%define ERi_stride		rbx
	%define ner				rsi
	%define width			rdi
	%define height			r8
	%define	er				r9
	%define	erw				r9w
	%define	erd				r9d
	%define ner_sum			r10
	%define ner_sumd		r10d
	%define ner_max			r11
	%define ner_maxw		r11w
	%define width16			r12
	%define i				r13
	%define t0				r14
	%define t0b				r14b
	%define t0w				r14w
	%define t0d				r14d

	%define vecER			xmm10
	%define vecZero			xmm11
	%define vecDup7th16s	xmm12
	%define vecOne			xmm13
	%define vecMask0		xmm14
	%define vecMask1		xmm15

	mov Xi, arg(0)
	mov Xi_stride, arg(1)
	mov ERi, arg(2)
	mov ERi_stride, arg(3)
	mov ner, arg(4)
	mov width, arg(7)
	mov height, arg(8)

	prefetcht0 [Xi + COMPV_YASM_CACHE_LINE_SIZE*0]
	prefetcht0 [Xi + COMPV_YASM_CACHE_LINE_SIZE*1]
	prefetcht0 [Xi + COMPV_YASM_CACHE_LINE_SIZE*2]
	prefetcht0 [Xi + COMPV_YASM_CACHE_LINE_SIZE*3]

	lea width16, [width - 1]
	mov t0, 1
	xor ner_sum, ner_sum
	movd vecOne, t0d
	xor ner_max, ner_max
	punpcklbw vecOne, vecOne 
	pxor vecZero, vecZero
	punpcklwd vecOne, vecOne 
	movdqa vecDup7th16s, [sym(kShuffleEpi8_DUP_16s7_32s)]
	movdqa vecMask0, [sym(kShuffleEpi8_DUP_SHL0_8u_32s)]
	movdqa vecMask1, [sym(kShuffleEpi8_DUP_SHL1_8u_32s)]	 
	pshufd vecOne, vecOne, 0
	and width16, -16
	shl ERi_stride, 1
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		movzx er, byte [Xi + 0*COMPV_YASM_UINT8_SZ_BYTES]
		and er, 1
		movd vecER, erd
		mov [ERi + 0*COMPV_YASM_INT16_SZ_BYTES], word erw
		punpcklwd vecER, vecER
		pshufd vecER, vecER, 0x00

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 1; i < width16; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		mov i, 1
		.LoopWidth16:
			prefetcht0 [Xi + COMPV_YASM_CACHE_LINE_SIZE*4] ; to avoid cache because we''re loading (xi) then (xi - 1)
			movdqu xmm8, [Xi + (i)*COMPV_YASM_UINT8_SZ_BYTES]
			pxor xmm8, [Xi + (i-1)*COMPV_YASM_UINT8_SZ_BYTES]
			pand xmm8, vecOne 
			movdqa xmm0, xmm8
			pcmpgtb xmm0, vecZero
			pmovmskb er, xmm0
			test er, er
			jz .XorIsZero
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; erUint8 += ((Xi[i - 1] ^ Xi[i]) & 1)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			.XorIsNotZero:
				movdqa xmm0, xmm8
				movdqa xmm1, xmm8
				movdqa xmm2, xmm8
				movdqa xmm3, xmm8
				movdqa xmm4, xmm8
				movdqa xmm5, xmm8
				movdqa xmm6, xmm8
				movdqa xmm7, xmm8
				pshufb xmm0, vecMask0
				pshufb xmm1, vecMask1
				pshufb xmm2, [sym(kShuffleEpi8_DUP_SHL2_8u_32s)]
				pshufb xmm3, [sym(kShuffleEpi8_DUP_SHL3_8u_32s)]
				pshufb xmm4, [sym(kShuffleEpi8_DUP_SHL4_8u_32s)]
				pshufb xmm5, [sym(kShuffleEpi8_DUP_SHL5_8u_32s)]
				pshufb xmm6, [sym(kShuffleEpi8_DUP_SHL6_8u_32s)]
				pshufb xmm7, [sym(kShuffleEpi8_DUP_SHL7_8u_32s)]
				paddb xmm0, xmm1
				paddb xmm2, xmm3
				paddb xmm4, xmm5
				paddb xmm6, xmm7
				paddb xmm0, xmm2
				paddb xmm4, xmm6
				movdqa xmm1, xmm8
				movdqa xmm2, xmm8
				movdqa xmm3, xmm8
				movdqa xmm5, xmm8
				movdqa xmm6, xmm8
				movdqa xmm7, xmm8
				movdqa xmm9, xmm8
				pshufb xmm1, [sym(kShuffleEpi8_DUP_SHL8_8u_32s)]
				pshufb xmm2, [sym(kShuffleEpi8_DUP_SHL9_8u_32s)]
				pshufb xmm3, [sym(kShuffleEpi8_DUP_SHL10_8u_32s)]
				pshufb xmm5, [sym(kShuffleEpi8_DUP_SHL11_8u_32s)]
				pshufb xmm6, [sym(kShuffleEpi8_DUP_SHL12_8u_32s)]
				pshufb xmm7, [sym(kShuffleEpi8_DUP_SHL13_8u_32s)]
				pshufb xmm8, [sym(kShuffleEpi8_DUP_SHL14_8u_32s)]
				pshufb xmm9, [sym(kShuffleEpi8_DUP_SHL15_8u_32s)]
				paddb xmm0, xmm4
				paddb xmm1, xmm2
				paddb xmm3, xmm5
				paddb xmm6, xmm7
				paddb xmm8, xmm9
				paddb xmm0, xmm1
				paddb xmm3, xmm6
				paddb xmm0, xmm8
				paddb xmm0, xmm3

				; Convert erUint8 to erInt16 and sum then store ;
				movdqa xmm1, xmm0
				punpcklbw xmm0, vecZero
				punpckhbw xmm1, vecZero
				paddw xmm0, vecER
				paddw vecER, xmm1
				movdqu [ERi + i*COMPV_YASM_INT16_SZ_BYTES], xmm0
				movdqu [ERi + (i+8)*COMPV_YASM_INT16_SZ_BYTES], vecER

				; Duplicate latest element ;
				pshufb vecER, vecDup7th16s
				jmp .EndOf_XorIsZero
			.EndOf_XorIsNotZero:
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; ERi[i] = ER
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			.XorIsZero:
				; Store previous ER ; 
				movdqu [ERi + i*COMPV_YASM_INT16_SZ_BYTES], vecER
				movdqu [ERi + (i+8)*COMPV_YASM_INT16_SZ_BYTES], vecER
			.EndOf_XorIsZero:

			add i, 16
			cmp i, width16
			jl .LoopWidth16
		.EndOf_LoopWidth16:

		; Get highest "er" before switching from SIMD to serial code ;
		pextrw er, vecER, 7

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width1; ++i)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_LoopWidth1
		.LoopWidth1:
			movzx t0, byte [Xi + (i-1)*COMPV_YASM_UINT8_SZ_BYTES]
			xor t0b, byte [Xi + (i)*COMPV_YASM_UINT8_SZ_BYTES]
			and t0b, byte 1
			add erw, word t0w
			mov [ERi + i*COMPV_YASM_INT16_SZ_BYTES], word erw
			inc i
			cmp i, width
			jl .LoopWidth1
		.EndOf_LoopWidth1:

		movzx t0, byte [Xi + (width-1)*COMPV_YASM_UINT8_SZ_BYTES]
		and t0, 1
		add er, t0
		mov [ner], word erw
		add ner_sumd, erd
		cmp ner_maxw, erw
		cmovl ner_maxw, erw

		; next ;
		dec height
		lea ner, [ner + 1*COMPV_YASM_INT16_SZ_BYTES]
		lea Xi, [Xi + Xi_stride] ; Xi_stride = Xi_stride * COMPV_YASM_UINT8_SZ_BYTES
		lea ERi, [ERi + ERi_stride] ; Xi_stride= Xi_stride * COMPV_YASM_INT16_SZ_BYTES
		jnz .LoopHeight
	.EndOf_LoopHeight:

	mov rax, arg(5)
	mov rdx, arg(6)
	mov [rax], word ner_maxw
	mov [rdx], dword ner_sumd

	%undef Xi				
	%undef Xi_stride		
	%undef ERi				
	%undef ERi_stride		
	%undef ner				
	%undef width			
	%undef height			
	%undef	er				
	%undef	erw				
	%undef	erd				
	%undef ner_sum			
	%undef ner_sumd		
	%undef ner_max			
	%undef ner_maxw		
	%undef width16			
	%undef i				
	%undef t0				
	%undef t0b				
	%undef t0w				
	%undef t0d				

	%undef vecER			
	%undef vecZero			
	%undef vecDup7th16s	
	%undef vecOne			
	%undef vecMask0		
	%undef vecMask1		

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
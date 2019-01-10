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

global sym(CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_X64_AVX2)
global sym(CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_X64_BMI1_AVX2)

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
; arg(3) -> int16_t* ner, 
; arg(4) -> int16_t* ner_max1, 
; arg(5) -> int32_t* ner_sum1,
; arg(6) -> const compv_uscalar_t width, 
; arg(7) -> const compv_uscalar_t height
sym(CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_X64_AVX2):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_YMM 15
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
	%define t1				r15

	%define vecER			xmm10
	%define vecDup7th16s	xmm11
	%define vecOne			xmm12
	%define vecMask0		xmm13
	%define vecMask1		xmm14
	%define vecMask2		xmm15

	mov Xi, arg(0)
	mov Xi_stride, arg(1)
	mov ERi, arg(2)
	mov ner, arg(3)
	mov width, arg(6)
	mov height, arg(7)

	lea width16, [width - 1]
	mov t0, 1
	xor ner_sum, ner_sum
	vmovd vecOne, t0d
	xor ner_max, ner_max
	vpbroadcastb vecOne, vecOne
	vmovdqa vecDup7th16s, [sym(kShuffleEpi8_DUP_16s7_32s)]
	vmovdqa vecMask0, [sym(kShuffleEpi8_DUP_SHL0_8u_32s)]
	vmovdqa vecMask1, [sym(kShuffleEpi8_DUP_SHL1_8u_32s)]
	vmovdqa vecMask2, [sym(kShuffleEpi8_DUP_SHL2_8u_32s)]
	and width16, -16
	lea ERi_stride, [width*COMPV_YASM_INT16_SZ_BYTES]
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		movzx er, byte [Xi + 0*COMPV_YASM_UINT8_SZ_BYTES]
		and er, 1
		vmovd vecER, erd
		mov [ERi + 0*COMPV_YASM_INT16_SZ_BYTES], word erw
		vpbroadcastw vecER, vecER

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 1; i < width16; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		mov i, 1
		.LoopWidth16:
			vmovdqa xmm8, [Xi + (i-1)*COMPV_YASM_UINT8_SZ_BYTES]
			lea t0, [Xi + i*COMPV_YASM_UINT8_SZ_BYTES]
			vpxor xmm8, [Xi + (i)*COMPV_YASM_UINT8_SZ_BYTES]
			lea t1, [ERi + i*COMPV_YASM_INT16_SZ_BYTES]
			vptest xmm8, xmm8
			prefetcht0 [t0 + Xi_stride]
			prefetchw [t1 + ERi_stride]
			jz .XorIsZero
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; erUint8 += ((Xi[i - 1] ^ Xi[i]) & 1)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			.XorIsNotZero:
				vpand xmm8, vecOne
				vpshufb xmm0, xmm8, vecMask0
				vpshufb xmm1, xmm8, vecMask1
				vpshufb xmm2, xmm8, vecMask2
				vpshufb xmm3, xmm8, [sym(kShuffleEpi8_DUP_SHL3_8u_32s)]
				vpshufb xmm4, xmm8, [sym(kShuffleEpi8_DUP_SHL4_8u_32s)]
				vpshufb xmm5, xmm8, [sym(kShuffleEpi8_DUP_SHL5_8u_32s)]
				vpshufb xmm6, xmm8, [sym(kShuffleEpi8_DUP_SHL6_8u_32s)]
				vpshufb xmm7, xmm8, [sym(kShuffleEpi8_DUP_SHL7_8u_32s)]
				vpaddb xmm0, xmm1
				vpaddb xmm2, xmm3
				vpaddb xmm4, xmm5
				vpaddb xmm6, xmm7
				vpaddb xmm0, xmm2
				vpaddb xmm4, xmm6
				vpshufb xmm1, xmm8, [sym(kShuffleEpi8_DUP_SHL8_8u_32s)]
				vpshufb xmm2, xmm8, [sym(kShuffleEpi8_DUP_SHL9_8u_32s)]
				vpshufb xmm3, xmm8, [sym(kShuffleEpi8_DUP_SHL10_8u_32s)]
				vpshufb xmm5, xmm8, [sym(kShuffleEpi8_DUP_SHL11_8u_32s)]
				vpshufb xmm6, xmm8, [sym(kShuffleEpi8_DUP_SHL12_8u_32s)]
				vpshufb xmm7, xmm8, [sym(kShuffleEpi8_DUP_SHL13_8u_32s)]
				vpshufb xmm9, xmm8, [sym(kShuffleEpi8_DUP_SHL15_8u_32s)]
				vpshufb xmm8, xmm8, [sym(kShuffleEpi8_DUP_SHL14_8u_32s)]
				vpaddb xmm0, xmm4
				vpaddb xmm1, xmm2
				vpaddb xmm3, xmm5
				vpaddb xmm6, xmm7
				vpaddb xmm8, xmm9
				vpaddb xmm0, xmm1
				vpaddb xmm3, xmm6
				vpaddb xmm0, xmm8
				vpaddb xmm0, xmm3

				; Convert erUint8 to erInt16 and sum then store ;
				vpmovzxbw ymm0, xmm0
				vextractf128 xmm1, ymm0, 0x1
				vpaddw xmm0, vecER
				vpaddw vecER, xmm1
				vmovdqu [ERi + i*COMPV_YASM_INT16_SZ_BYTES], xmm0
				vmovdqu [ERi + (i+8)*COMPV_YASM_INT16_SZ_BYTES], vecER

				; Duplicate latest element ;
				vpshufb vecER, vecDup7th16s
				jmp .EndOf_XorIsZero
			.EndOf_XorIsNotZero:
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; ERi[i] = ER
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			.XorIsZero:
				; Store previous ER ; 
				vmovdqu [ERi + i*COMPV_YASM_INT16_SZ_BYTES], vecER
				vmovdqu [ERi + (i+8)*COMPV_YASM_INT16_SZ_BYTES], vecER
			.EndOf_XorIsZero:

			add i, 16
			cmp i, width16
			jl .LoopWidth16
		.EndOf_LoopWidth16:

		; Get highest "er" before switching from SIMD to serial code ;
		vpextrw er, vecER, 7

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

	mov rax, arg(4)
	mov rdx, arg(5)
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
	%undef t1			

	%undef vecER					
	%undef vecDup7th16s	
	%undef vecOne			
	%undef vecMask0		
	%undef vecMask1
	%undef vecMask2	

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


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) ->const uint8_t* Xi, 
; arg(1) ->const compv_uscalar_t Xi_stride,
; arg(2) ->int16_t* ERi, 
; arg(3) ->int16_t* RLCi, 
; arg(4) ->const compv_uscalar_t RLCi_stride,
; arg(5) ->const compv_uscalar_t width, 
; arg(6) ->const compv_uscalar_t height
sym(CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_X64_BMI1_AVX2):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	push r14
	;; end prolog ;;

	%define	Xi			rax
	%define	Xi_stride	rcx
	%define	ERi			rdx
	%define	ERi_stride	rbx
	%define	RLCi		rsi
	%define	RLCi_stride rdi
	%define	width		r8
	%define	height		r9
	%define width32		r10
	%define t0			r11
	%define t0b			r11b
	%define t0w			r11w
	%define t0d			r11d
	%define t1			r12
	%define t1w			r12w
	%define t1d			r12d
	%define i			r14
	%define iw			r14w
	%define id			r14d
	%define er			r15
	%define erb			r15b

	mov Xi, arg(0) 
	mov Xi_stride, arg(1)
	mov ERi, arg(2) 
	mov RLCi, arg(3)
	mov RLCi_stride, arg(4)
	mov width, arg(5)
	mov height, arg(6)

	prefetchw [RLCi]

	lea width32, [width - 1]
	lea ERi_stride, [width*COMPV_YASM_INT16_SZ_BYTES]
	shl RLCi_stride, 1
	and width32, -32

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		lea t1, [RLCi + RLCi_stride*COMPV_YASM_INT16_SZ_BYTES]
		movzx er, byte [Xi + 0*COMPV_YASM_UINT8_SZ_BYTES]
		mov [RLCi], word 0
		and erb, 1
		prefetchw [t1] ; don''t expect more than 32 RLC and this is why the prefetch is outside the width loop
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 1; i < width32; i += 32)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		mov i, 1
		.LoopWidth32:
			vmovdqu ymm0, [ERi + (i-1)*COMPV_YASM_INT16_SZ_BYTES]
			vmovdqu ymm1, [ERi + (i+15)*COMPV_YASM_INT16_SZ_BYTES]
			vpcmpeqw ymm0, [ERi + (i)*COMPV_YASM_INT16_SZ_BYTES]
			vpcmpeqw ymm1, [ERi + (i+16)*COMPV_YASM_INT16_SZ_BYTES]
			vpacksswb ymm0, ymm1
			lea t1, [ERi + i*COMPV_YASM_INT16_SZ_BYTES]
			vpermq ymm0, ymm0, 0xD8
			prefetcht0 [t1 + ERi_stride]
			vpmovmskb t0d, ymm0
			xor t0d, 0xffffffff
			jz .EndOfMask
				.BeginOfWhile
					tzcnt t1d, t0d
					add t1d, id
					mov [RLCi + er*COMPV_YASM_INT16_SZ_BYTES], word t1w
					inc er
					blsr t0d, t0d
					jnz .BeginOfWhile
				.EndOfWhile
			.EndOfMask
			
			add i, 32
			cmp i, width32
			jl .LoopWidth32
		.EndOf_LoopWidth32:

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width1; ++i) 
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_LoopWidth1
		.LoopWidth1:
			mov t0w, word [ERi + (i - 1)*COMPV_YASM_INT16_SZ_BYTES]
			cmp t0w, word [ERi + i*COMPV_YASM_INT16_SZ_BYTES]
			jz .Next_LoopWidth1
				mov [RLCi + er*COMPV_YASM_INT16_SZ_BYTES], word iw
				inc er
			.Next_LoopWidth1:
			inc i
			cmp i, width
			jl .LoopWidth1
		.EndOf_LoopWidth1:

		mov t0, 1
		mov t1, width
		and t0b, byte [Xi + (width - 1)*COMPV_YASM_UINT8_SZ_BYTES]
		lea Xi, [Xi + Xi_stride]
		xor t0, 1
		sub t1, t0
		dec height
		lea ERi, [ERi + ERi_stride] ; ERi_stride = ERi_stride*COMPV_YASM_INT16_SZ_BYTES
		mov [RLCi + er*COMPV_YASM_INT16_SZ_BYTES], word t1w
		lea RLCi, [RLCi + RLCi_stride] ; RLCi_stride = RLCi_stride*COMPV_YASM_INT16_SZ_BYTES
		jnz .LoopHeight
	.EndOf_LoopHeight:

	%undef	Xi			
	%undef	Xi_stride	
	%undef	ERi			
	%undef	ERi_stride	
	%undef	RLCi		
	%undef	RLCi_stride 
	%undef	width		
	%undef	height		
	%undef width32		
	%undef t0			
	%undef t0b			
	%undef t0w
	%undef t0d		
	%undef t1			
	%undef t1w
	%undef t1d
	%undef i			
	%undef iw
	%undef id		
	%undef er			
	%undef erb	

	;; begin epilog ;;
	pop r14
	pop r13
	pop r12
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret

%endif ; COMPV_YASM_ABI_IS_64BIT
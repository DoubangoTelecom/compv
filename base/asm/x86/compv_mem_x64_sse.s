;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT
%include "compv_vldx_vstx_macros_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(CompVMemCopyNTA_Asm_Aligned11_X64_SSE2)
global sym(CompVMemUnpack4_Asm_X64_SSSE3)
global sym(CompVMemUnpack3_Asm_X64_SSSE3)
global sym(CompVMemUnpack2_Asm_X64_SSSE3)
global sym(CompVMemPack4_Asm_X64_SSE2)
global sym(CompVMemPack3_Asm_X64_SSSE3)
global sym(CompVMemPack2_Asm_X64_SSE2)

section .data
	extern sym(kShuffleEpi8_Deinterleave8uL4_32s)
	extern sym(kShuffleEpi8_Deinterleave8uL3_32s)
	extern sym(kShuffleEpi8_Deinterleave8uL2_32s)
	extern sym(kShuffleEpi8_Interleave8uL3_Step0_s32)
	extern sym(kShuffleEpi8_Interleave8uL3_Step1_s32)
	extern sym(kShuffleEpi8_Interleave8uL3_Step2_s32)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; size must be > 16 and it''s up to the caller to check it
; size should be multiple of 16, if not the remaining will be ignored
; arg(0) -> COMPV_ALIGNED(SSE) void* dataDstPtr
; arg(1) -> COMPV_ALIGNED(SSE) const void* dataSrcPtr
; arg(2) -> compv_uscalar_t size
; void CompVMemCopyNTA_Asm_Aligned11_X64_SSE2(COMPV_ALIGNED(SSE) void* dataDstPtr, COMPV_ALIGNED(SSE) const void* dataSrcPtr, compv_uscalar_t size)
sym(CompVMemCopyNTA_Asm_Aligned11_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 3
	COMPV_YASM_SAVE_XMM 15 ;XMM[6-n]
	push rsi
	push rdi
	push rbx
	; end prolog

	%define cache_line_size 64

	mov rax, arg(2) ; size
	mov rdi, arg(0) ; dataDstPtr
	mov rsi, arg(1) ; dataSrcPtr
	mov rcx, rax
	mov rdx, rax
	shr rcx, 8 ; rcx = count16x16
	shr rdx, 4 ; rdx = count16x1

	; Check if we have to execute Count16x16Loop
	test rcx, rcx
	jz .EndOfCount16x16Loop
	mov rbx, rcx
	shl rbx, 8
	sub rax, rbx
	shr rax, 4
	mov rdx, rax

	; Begin Count16x16Loop
	align 16
	.Count16x16Loop
		prefetchnta [rsi + cache_line_size*5]
		prefetchnta [rsi + cache_line_size*6]
		prefetchnta [rsi + cache_line_size*7]
		prefetchnta [rsi + cache_line_size*8]

		%assign j 0
		%rep 16
			movdqa xmm %+ j, [rsi + j*16]
			%assign j j+1
		%endrep
		%assign j 0
		%rep 16
			movntdq [rdi + j*16], xmm %+ j
			%assign j j+1
		%endrep
			
		add rsi, 16*16
		add rdi, 16*16

		dec rcx
		jnz .Count16x16Loop
		; End Count16x16Loop
	.EndOfCount16x16Loop

	; Check if we have to execute Count16x1Loop
	test rdx, rdx
	jz .EndOfCount16x1Loop

	; Begin Count16x1Loop
	align 16
	.Count16x1Loop
		movdqa xmm0, [rsi]
		add rsi, 16
		movntdq [rdi], xmm0
		add rdi, 16
	
		dec rdx
		jnz .Count16x1Loop
	; End Count16x1Loop
	.EndOfCount16x1Loop

	mfence ; flush latest WC (Write Combine) buffers to memory

	; begin epilog
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) uint8_t* dstPt0
; arg(1) -> COMPV_ALIGNED(SSE) uint8_t* dstPt1
; arg(2) -> COMPV_ALIGNED(SSE) uint8_t* dstPt2
; arg(3) -> COMPV_ALIGNED(SSE) uint8_t* dstPt3
; arg(4) -> COMPV_ALIGNED(SSE) const compv_uint8x4_t* srcPtr
; arg(5) -> compv_uscalar_t width
; arg(6) -> compv_uscalar_t height
; arg(7) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
sym(CompVMemUnpack4_Asm_X64_SSSE3):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	%define dstPt0		rax
	%define dstPt1		rdx
	%define dstPt2		rcx
	%define dstPt3		r8
	%define srcPtr		r9
	%define width		r10
	%define height		r11
	%define stride		rsi
	%define i			rbx
	%define k			rdi

	mov dstPt0, arg(0)
	mov dstPt1, arg(1)
	mov dstPt2, arg(2)
	mov dstPt3, arg(3)
	mov srcPtr, arg(4)
	mov width, arg(5)
	mov height, arg(6)
	mov stride, arg(7)

	prefetcht0 [srcPtr + COMPV_YASM_CACHE_LINE_SIZE*0]
	prefetcht0 [srcPtr + COMPV_YASM_CACHE_LINE_SIZE*1]
	prefetcht0 [srcPtr + COMPV_YASM_CACHE_LINE_SIZE*2]
	prefetcht0 [srcPtr + COMPV_YASM_CACHE_LINE_SIZE*3]

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (compv_uscalar_t i = 0; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		xor k, k
		.LoopWidth:
			prefetcht0 [srcPtr + k + COMPV_YASM_CACHE_LINE_SIZE*4]
			COMPV_VLD4_U8_SSSE3 srcPtr + k, xmm0, xmm1, xmm2, xmm3, xmm4, xmm5
			movdqa [dstPt0 + i*COMPV_YASM_UINT8_SZ_BYTES], xmm0
			movdqa [dstPt1 + i*COMPV_YASM_UINT8_SZ_BYTES], xmm1
			movdqa [dstPt2 + i*COMPV_YASM_UINT8_SZ_BYTES], xmm2
			movdqa [dstPt3 + i*COMPV_YASM_UINT8_SZ_BYTES], xmm3

			add i, (16*1*COMPV_YASM_UINT8_SZ_BYTES)
			cmp i, width
			lea k, [k + (16*4*COMPV_YASM_UINT8_SZ_BYTES)]
			jl .LoopWidth
		.EndOf_LoopWidth:

		dec height
		lea dstPt0, [dstPt0 + stride*COMPV_YASM_UINT8_SZ_BYTES]
		lea dstPt1, [dstPt1 + stride*COMPV_YASM_UINT8_SZ_BYTES]
		lea dstPt2, [dstPt2 + stride*COMPV_YASM_UINT8_SZ_BYTES]
		lea dstPt3, [dstPt3 + stride*COMPV_YASM_UINT8_SZ_BYTES]
		lea srcPtr, [srcPtr + stride*4*COMPV_YASM_UINT8_SZ_BYTES]
		jnz .LoopHeight
	.EndOf_LoopHeight:

	%undef dstPt0		
	%undef dstPt1		
	%undef dstPt2
	%undef dstPt3	
	%undef srcPtr		
	%undef width		
	%undef height		
	%undef stride
	%undef i			
	%undef k			

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) uint8_t* dstPt0
; arg(1) -> COMPV_ALIGNED(SSE) uint8_t* dstPt1
; arg(2) -> COMPV_ALIGNED(SSE) uint8_t* dstPt2
; arg(3) -> COMPV_ALIGNED(SSE) const compv_uint8x3_t* srcPtr
; arg(4) -> compv_uscalar_t width
; arg(5) -> compv_uscalar_t height
; arg(6) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
sym(CompVMemUnpack3_Asm_X64_SSSE3):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	%define dstPt0		rax
	%define dstPt1		rdx
	%define dstPt2		rcx
	%define srcPtr		r8
	%define width		r9
	%define height		r10
	%define stride		r11
	%define stride3		rsi
	%define i			rbx
	%define k			rdi

	mov dstPt0, arg(0)
	mov dstPt1, arg(1)
	mov dstPt2, arg(2)
	mov srcPtr, arg(3)
	mov width, arg(4)
	mov height, arg(5)
	mov stride, arg(6)
	lea stride3, [stride*3]

	prefetcht0 [srcPtr + COMPV_YASM_CACHE_LINE_SIZE*0]
	prefetcht0 [srcPtr + COMPV_YASM_CACHE_LINE_SIZE*1]
	prefetcht0 [srcPtr + COMPV_YASM_CACHE_LINE_SIZE*2]
	prefetcht0 [srcPtr + COMPV_YASM_CACHE_LINE_SIZE*3]

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (compv_uscalar_t i = 0; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		xor k, k
		.LoopWidth:
			prefetcht0 [srcPtr + k + COMPV_YASM_CACHE_LINE_SIZE*4]
			COMPV_VLD3_U8_SSSE3 srcPtr + k, xmm0, xmm1, xmm2, xmm3, xmm4, xmm5
			movdqa [dstPt0 + (i)*COMPV_YASM_UINT8_SZ_BYTES], xmm0
			movdqa [dstPt1 + (i)*COMPV_YASM_UINT8_SZ_BYTES], xmm1
			movdqa [dstPt2 + (i)*COMPV_YASM_UINT8_SZ_BYTES], xmm2

			add i, (16*1*COMPV_YASM_UINT8_SZ_BYTES)
			cmp i, width
			lea k, [k + (16*3*COMPV_YASM_UINT8_SZ_BYTES)]
			jl .LoopWidth
		.EndOf_LoopWidth:

		dec height
		lea dstPt0, [dstPt0 + stride*COMPV_YASM_UINT8_SZ_BYTES]
		lea dstPt1, [dstPt1 + stride*COMPV_YASM_UINT8_SZ_BYTES]
		lea dstPt2, [dstPt2 + stride*COMPV_YASM_UINT8_SZ_BYTES]
		lea srcPtr, [srcPtr + stride3*COMPV_YASM_UINT8_SZ_BYTES]
		jnz .LoopHeight
	.EndOf_LoopHeight:

	%undef dstPt0		
	%undef dstPt1		
	%undef dstPt2		
	%undef srcPtr		
	%undef width		
	%undef height		
	%undef stride
	%undef stride3	
	%undef i			
	%undef k			

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) uint8_t* dstPt0
; arg(1) -> COMPV_ALIGNED(SSE) uint8_t* dstPt1
; arg(2) -> COMPV_ALIGNED(SSE) const compv_uint8x3_t* srcPtr
; arg(3) -> compv_uscalar_t width
; arg(4) -> compv_uscalar_t height
; arg(5) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
sym(CompVMemUnpack2_Asm_X64_SSSE3):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	;; end prolog ;;

	%define dstPt0		rax
	%define dstPt1		rdx
	%define srcPtr		rcx
	%define width		r8
	%define height		r9
	%define stride		r10
	%define i			r11

	mov dstPt0, arg(0)
	mov dstPt1, arg(1)
	mov srcPtr, arg(2)
	mov width, arg(3)
	mov height, arg(4)
	mov stride, arg(5)

	movdqa xmm4, [sym(kShuffleEpi8_Deinterleave8uL2_32s)]

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (compv_uscalar_t i = 0; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		.LoopWidth:
			movdqa xmm0, [srcPtr + (i+0)*COMPV_YASM_UINT16_SZ_BYTES]
			movdqa xmm1, [srcPtr + (i+8)*COMPV_YASM_UINT16_SZ_BYTES]
			pshufb xmm0, xmm4
			pshufb xmm1, xmm4
			movdqa xmm3, xmm0
			punpcklqdq xmm0, xmm1
			punpckhqdq xmm3, xmm1
			movdqa [dstPt0 + (i)*COMPV_YASM_UINT8_SZ_BYTES], xmm0
			movdqa [dstPt1 + (i)*COMPV_YASM_UINT8_SZ_BYTES], xmm3

			add i, (16*COMPV_YASM_UINT8_SZ_BYTES)
			cmp i, width
			jl .LoopWidth
		.EndOf_LoopWidth:

		dec height
		lea dstPt0, [dstPt0 + stride*COMPV_YASM_UINT8_SZ_BYTES]
		lea dstPt1, [dstPt1 + stride*COMPV_YASM_UINT8_SZ_BYTES]
		lea srcPtr, [srcPtr + stride*COMPV_YASM_UINT16_SZ_BYTES]
		jnz .LoopHeight
	.EndOf_LoopHeight:

	%undef dstPt0		
	%undef dstPt1		
	%undef srcPtr		
	%undef width		
	%undef height		
	%undef stride
	%undef i						

	;; begin epilog ;;
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) compv_uint8x4_t* dstPtr
; arg(1) -> COMPV_ALIGNED(SSE) const uint8_t* srcPt0
; arg(2) -> COMPV_ALIGNED(SSE) const uint8_t* srcPt1
; arg(3) -> COMPV_ALIGNED(SSE) const uint8_t* srcPt2
; arg(4) -> COMPV_ALIGNED(SSE) const uint8_t* srcPt3
; arg(5) -> compv_uscalar_t width
; arg(6) -> compv_uscalar_t height
; arg(7) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
sym(CompVMemPack4_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	%define dstPtr		rax
	%define srcPt0		rdx
	%define srcPt1		rcx
	%define srcPt2		r8
	%define srcPt3		r9
	%define width		r10
	%define height		r11
	%define stride		rsi
	%define i			rdi
	%define k			rbx

	mov dstPtr, arg(0)
	mov srcPt0, arg(1)
	mov srcPt1, arg(2)
	mov srcPt2, arg(3)
	mov srcPt3, arg(4)
	mov width, arg(5)
	mov height, arg(6)
	mov stride, arg(7)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (compv_uscalar_t i = 0; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		xor k, k
		.LoopWidth:
			movdqa xmm0, [srcPt0 + i*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa xmm1, [srcPt1 + i*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa xmm2, [srcPt2 + i*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa xmm3, [srcPt3 + i*COMPV_YASM_UINT8_SZ_BYTES]
			COMPV_VST4_U8_SSE2 dstPtr + k, xmm0, xmm1, xmm2, xmm3, xmm4, xmm5
			add i, (16*1*COMPV_YASM_UINT8_SZ_BYTES)
			cmp i, width
			lea k, [k + (16*4*COMPV_YASM_UINT8_SZ_BYTES)]
			jl .LoopWidth
		.EndOf_LoopWidth:

		dec height
		lea srcPt0, [srcPt0 + stride*COMPV_YASM_UINT8_SZ_BYTES]
		lea srcPt1, [srcPt1 + stride*COMPV_YASM_UINT8_SZ_BYTES]
		lea srcPt2, [srcPt2 + stride*COMPV_YASM_UINT8_SZ_BYTES]
		lea srcPt3, [srcPt3 + stride*COMPV_YASM_UINT8_SZ_BYTES]
		lea dstPtr, [dstPtr + stride*4*COMPV_YASM_UINT8_SZ_BYTES]
		jnz .LoopHeight
	.EndOf_LoopHeight:

	%undef dstPtr		
	%undef srcPt0		
	%undef srcPt1	
	%undef srcPt2
	%undef srcPt3		
	%undef width		
	%undef height		
	%undef stride
	%undef i			
	%undef k			

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
	
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) compv_uint8x3_t* dstPtr
; arg(1) -> COMPV_ALIGNED(SSE) const uint8_t* srcPt0
; arg(2) -> COMPV_ALIGNED(SSE) const uint8_t* srcPt1
; arg(3) -> COMPV_ALIGNED(SSE) const uint8_t* srcPt2
; arg(4) -> compv_uscalar_t width
; arg(5) -> compv_uscalar_t height
; arg(6) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
sym(CompVMemPack3_Asm_X64_SSSE3):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	%define dstPtr		rax
	%define srcPt0		rdx
	%define srcPt1		rcx
	%define srcPt2		r8
	%define width		r9
	%define height		r10
	%define stride		r11
	%define stride3		rsi
	%define i			rdi
	%define k			rbx

	mov dstPtr, arg(0)
	mov srcPt0, arg(1)
	mov srcPt1, arg(2)
	mov srcPt2, arg(3)
	mov width, arg(4)
	mov height, arg(5)
	mov stride, arg(6)
	lea stride3, [stride*3*COMPV_YASM_UINT8_SZ_BYTES]

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (compv_uscalar_t i = 0; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		xor k, k
		.LoopWidth:
			movdqa xmm0, [srcPt0 + i*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa xmm1, [srcPt1 + i*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa xmm2, [srcPt2 + i*COMPV_YASM_UINT8_SZ_BYTES]
			COMPV_VST3_U8_SSSE3 dstPtr + k, xmm0, xmm1, xmm2, xmm3, xmm4, xmm5
			add i, (16*1*COMPV_YASM_UINT8_SZ_BYTES)
			cmp i, width
			lea k, [k + (16*3*COMPV_YASM_UINT8_SZ_BYTES)]
			jl .LoopWidth
		.EndOf_LoopWidth:

		dec height
		lea srcPt0, [srcPt0 + stride*COMPV_YASM_UINT8_SZ_BYTES]
		lea srcPt1, [srcPt1 + stride*COMPV_YASM_UINT8_SZ_BYTES]
		lea srcPt2, [srcPt2 + stride*COMPV_YASM_UINT8_SZ_BYTES]
		lea dstPtr, [dstPtr + stride3*COMPV_YASM_UINT8_SZ_BYTES]
		jnz .LoopHeight
	.EndOf_LoopHeight:

	%undef dstPtr		
	%undef srcPt0		
	%undef srcPt1		
	%undef srcPt2		
	%undef width		
	%undef height		
	%undef stride
	%undef stride3	
	%undef i			
	%undef k			

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) compv_uint8x3_t* dstPtr
; arg(1) -> COMPV_ALIGNED(SSE) const uint8_t* srcPt0
; arg(2) -> COMPV_ALIGNED(SSE) const uint8_t* srcPt1
; arg(3) -> compv_uscalar_t width
; arg(4) -> compv_uscalar_t height
; arg(5) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
sym(CompVMemPack2_Asm_X64_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	;; end prolog ;;

	%define dstPtr		rax
	%define srcPt0		rdx
	%define srcPt1		rcx
	%define width		r8
	%define height		r9
	%define stride		r10
	%define i			r11

	mov dstPtr, arg(0)
	mov srcPt0, arg(1)
	mov srcPt1, arg(2)
	mov width, arg(3)
	mov height, arg(4)
	mov stride, arg(5)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (compv_uscalar_t i = 0; i < width; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		.LoopWidth:
			movdqa xmm0, [srcPt0 + i*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa xmm1, [srcPt1 + i*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa xmm2, xmm0
			punpcklbw xmm0, xmm1
			punpckhbw xmm2, xmm1
			movdqa [dstPtr + (i+0)*COMPV_YASM_UINT16_SZ_BYTES], xmm0
			movdqa [dstPtr + (i+8)*COMPV_YASM_UINT16_SZ_BYTES], xmm2
			add i, (16*COMPV_YASM_UINT8_SZ_BYTES)
			cmp i, width
			jl .LoopWidth
		.EndOf_LoopWidth:

		dec height
		lea srcPt0, [srcPt0 + stride*COMPV_YASM_UINT8_SZ_BYTES]
		lea srcPt1, [srcPt1 + stride*COMPV_YASM_UINT8_SZ_BYTES]
		lea dstPtr, [dstPtr + stride*COMPV_YASM_UINT16_SZ_BYTES]
		jnz .LoopHeight
	.EndOf_LoopHeight:

	%undef dstPtr		
	%undef srcPt0
	%undef srcPt1		
	%undef width		
	%undef height		
	%undef stride
	%undef i					

	;; begin epilog ;;
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

%endif ; COMPV_YASM_ABI_IS_64BIT

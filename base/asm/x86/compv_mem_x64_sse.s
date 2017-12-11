;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   ;
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
global sym(CompVMemCopy3_Asm_X64_SSSE3)

section .data
	extern sym(kShuffleEpi8_Deinterleave8uL3_32s)

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
; arg(3) -> COMPV_ALIGNED(SSE) const compv_uint8x3_t* srcPtr
; arg(4) -> compv_uscalar_t width
; arg(5) -> compv_uscalar_t height
; arg(6) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
sym(CompVMemCopy3_Asm_X64_SSSE3):
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
	
	

%endif ; COMPV_YASM_ABI_IS_64BIT
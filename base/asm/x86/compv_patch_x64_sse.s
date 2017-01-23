;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(Moments0110_Asm_X64_SSE41)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* top
; arg(1) -> COMPV_ALIGNED(SSE)const uint8_t* bottom
; arg(2) -> COMPV_ALIGNED(SSE)const int16_t* x
; arg(3) -> COMPV_ALIGNED(SSE) const int16_t* y
; arg(4) -> compv_scalar_t count
; arg(5) -> compv_scalar_t* s01
; arg(6) -> compv_scalar_t* s10
; void Moments0110_Asm_X64_SSE41(COMPV_ALIGNED(SSE) const uint8_t* top, COMPV_ALIGNED(SSE)const uint8_t* bottom, COMPV_ALIGNED(SSE)const int16_t* x, COMPV_ALIGNED(SSE) const int16_t* y, compv_scalar_t count, compv_scalar_t* s01, compv_scalar_t* s10)
sym(Moments0110_Asm_X64_SSE41):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_XMM 7 ;XMM[6-7]
	push rsi
	push rdi
	push rbx
	push r12
	push r13
	;; end prolog ;;

	%define COMPV_SIZE_OF_INT16 2

	; xmm7 = xmmZero
	pxor xmm7, xmm7

	; rsi = i
	xor rsi, rsi

	; rdi = top
	mov rdi, arg(0)
	
	; rbx = bottom
	mov rbx, arg(1)

	; rcx = x
	mov rcx, arg(2)

	; rdx = y
	mov rdx, arg(3)

	mov r8, arg(5) ; s01
	mov r9, arg(6) ; s10

	; r11 = *s01
	mov r11, [r8]

	; r12 = *s10
	mov r12, [r9]

	; r13 = count
	mov r13, arg(4)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_scalar_t i = 0; i < count; i += 16)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopRows
		movdqa xmm0, [rdi + rsi] ; xmm0 = xmmTop
		movdqa xmm1, [rbx + rsi] ; xmm1 = xmmBottom
		movdqa xmm2, xmm0 ; xmm2 = xmmTop
		movdqa xmm3, xmm1 ; xmm3 = xmmBottom

		punpcklbw xmm0, xmm7
		punpcklbw xmm1, xmm7
		movdqa xmm4, xmm0
		movdqa xmm5, xmm1

		paddw xmm0, xmm1
		pxor xmm1, xmm1
		pmullw xmm0, [rcx + rsi*COMPV_SIZE_OF_INT16]
		pxor xmm6, xmm6
		punpcklwd xmm1, xmm0
		punpckhwd xmm6, xmm0
		psrad xmm1, 16
		psrad xmm6, 16
		psubw xmm4, xmm5
		phaddd xmm1, xmm6
		pmullw xmm4, [rdx + rsi*COMPV_SIZE_OF_INT16]
		pxor xmm0, xmm0
		pxor xmm5, xmm5
		punpcklwd xmm0, xmm4
		punpckhwd xmm5, xmm4
		psrad xmm0, 16
		psrad xmm5, 16
		punpckhbw xmm2, xmm7
		punpckhbw xmm3, xmm7
		phaddd xmm0, xmm5
		movdqa xmm4, xmm2
		movdqa xmm5, xmm3		
		paddw xmm2, xmm3
		phaddd xmm1, xmm0
		pmullw xmm2, [rcx + rsi*COMPV_SIZE_OF_INT16 + 8*COMPV_SIZE_OF_INT16]
		pxor xmm0, xmm0
		pxor xmm6, xmm6
		punpcklwd xmm0, xmm2
		punpckhwd xmm6, xmm2
		psubw xmm4, xmm5
		psrad xmm0, 16
		psrad xmm6, 16
		pmullw xmm4, [rdx + rsi*COMPV_SIZE_OF_INT16 + 8*COMPV_SIZE_OF_INT16]
		pxor xmm2, xmm2
		pxor xmm5, xmm5
		punpcklwd xmm2, xmm4
		punpckhwd xmm5, xmm4
		psrad xmm2, 16
		psrad xmm5, 16
		phaddd xmm0, xmm6
		phaddd xmm2, xmm5
		phaddd xmm0, xmm2
		phaddd xmm1, xmm0

		pextrd dword eax, xmm1, 0
		pextrd dword r8d, xmm1, 1
		pextrd dword r9d, xmm1, 2
		pextrd dword r10d, xmm1, 3

		add dword r12d, eax
		add dword r11d, r8d
		add dword r12d, r9d
		add dword r11d, r10d

		add rsi, 16
		cmp rsi, r13
		jl .LoopRows

	mov r8, arg(5) ; s01
	mov r9, arg(6) ; s10

	mov [r8], r11
	mov [r9], r12

	%undef COMPV_SIZE_OF_INT16

	;; begin epilog ;;
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
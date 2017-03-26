;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(CompVMathUtilsMax_16u_Asm_X86_SSE41)
global sym(CompVMathUtilsSum_8u32u_Asm_X86_SSE2)
global sym(CompVMathUtilsSumAbs_16s16u_Asm_X86_SSSE3)
global sym(CompVMathUtilsSum2_32s32s_Asm_X86_SSE2)
global sym(CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_X86_SSE2)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint16_t* data
; arg(1) -> compv_uscalar_t width
; arg(2) -> compv_uscalar_t height
; arg(3) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
; arg(4) -> uint16_t *max
sym(CompVMathUtilsMax_16u_Asm_X86_SSE41):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 16, rax
	sub rsp, (4*COMPV_YASM_UINT32_SZ_BYTES)

	%define data				rbx
	%define width				rdx
	%define height				rsi
	%define i					rcx
	%define widthMinus7			rdi
	%define widthMinus31		rax
	%define vecMax				xmm4
	%define vecOrphansSuppress	xmm5
	
	;; setting orphans ;;
	mov rax, arg(1) ; width
	and rax, 7
	jz .OrphansIsZero
		pcmpeqw xmm0, xmm0
		movdqa [rsp + 0], xmm0
		shl rax, 4
		mov rcx, 8*16
		sub rcx, rax
		xor rdi, rdi
		%assign index 0
		%rep 4
			test rcx, rcx
			js .OrphansCopied
			mov rax, 0xffffffff
			shr rax, cl
			cmp rcx, 31
			cmovg rax, rdi ; rdi = 0
			mov [rsp + (3-index)*COMPV_YASM_UINT32_SZ_BYTES], dword eax
			sub rcx, 32
			%assign index index+1
		%endrep
		.OrphansCopied:
			movdqa vecOrphansSuppress, [rsp + 0]
		.OrphansIsZero:

	;; convert stride from shorts to bytes ;;
	mov rax, arg(3) ; stride
	lea rax, [rax * COMPV_YASM_INT16_SZ_BYTES]
	mov arg(3), rax

	mov data, arg(0)
	mov width, arg(1)
	mov height, arg(2)
	lea widthMinus31, [width - 31]
	lea widthMinus7, [width - 7]
	
	pxor vecMax, vecMax

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		xor i, i
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < widthSigned - 31; i += 32)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		test widthMinus31, widthMinus31
		jle .EndOf_LoopWidth32
		.LoopWidth32:
			movdqa xmm0, [data + (i + 0)*COMPV_YASM_UINT16_SZ_BYTES]
			movdqa xmm1, [data + (i + 8)*COMPV_YASM_UINT16_SZ_BYTES]
			movdqa xmm2, [data + (i + 16)*COMPV_YASM_UINT16_SZ_BYTES]
			movdqa xmm3, [data + (i + 24)*COMPV_YASM_UINT16_SZ_BYTES]
			add i, 32
			pmaxuw xmm0, xmm1
			pmaxuw xmm2, xmm3
			cmp i, widthMinus31
			pmaxuw vecMax, xmm0
			pmaxuw vecMax, xmm2
			jl .LoopWidth32
			.EndOf_LoopWidth32
			;; EndOf_LoopWidth32 ;;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < widthSigned - 7; i += 8)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, widthMinus7
		jge .EndOf_LoopWidth7
		.LoopWidth7:
			pmaxuw vecMax, [data + (i + 0)*COMPV_YASM_UINT16_SZ_BYTES]
			add i, 8
			cmp i, widthMinus7
			jl .LoopWidth7
			.EndOf_LoopWidth7
			;; EndOf_LoopWidth7 ;;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (orphans)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		test width, 7
		jz .NoOrphans
			movdqa xmm0, [data + (i + 0)*COMPV_YASM_UINT16_SZ_BYTES]
			pand xmm0, vecOrphansSuppress
			pmaxuw vecMax, xmm0
			.NoOrphans

		add data, arg(3)
		dec height
		jnz .LoopHeight
		;; EndOf_LoopHeight ;;

	movdqa xmm0, vecMax
	movdqa xmm1, vecMax
	movdqa xmm2, vecMax
	psrldq xmm0, 8
	psrldq xmm1, 4
	psrldq xmm2, 2
	pmaxuw xmm0, xmm1
	pmaxuw vecMax, xmm2
	pmaxuw vecMax, xmm0

	pextrw eax, vecMax, 0
	mov rdx, arg(4)
	mov [rdx], word ax

	; free memory and unalign stack
	add rsp, (4*COMPV_YASM_UINT32_SZ_BYTES)
	COMPV_YASM_UNALIGN_STACK

	%undef data
	%undef width
	%undef height
	%undef i
	%undef widthMinus7
	%undef widthMinus31
	%undef vecMax
	%undef vecOrphansSuppress

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint8_t* data
; arg(1) -> compv_uscalar_t width
; arg(2) -> compv_uscalar_t height
; arg(3) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
; arg(4) -> uint32_t *sum1
sym(CompVMathUtilsSum_8u32u_Asm_X86_SSE2)
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	COMPV_YASM_SAVE_XMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 16, rax
	sub rsp, (2*COMPV_YASM_XMM_SZ_BYTES)
	
	%define vecOrphansSuppress		rsp + 0
	%define vecZero					vecOrphansSuppress + COMPV_YASM_XMM_SZ_BYTES

	%define data				rbx
	%define width				rsi
	%define height				rdi
	%define widthMinus15		rdx
	%define i					rcx
	%define widthMinus63		rax
	%define vecSumh				xmm6
	%define vecSuml				xmm7

	;; setting orphans ;;
	mov rax, arg(1) ; width
	and rax, 15
	jz .OrphansIsZero
		pcmpeqw xmm0, xmm0
		movdqa [vecOrphansSuppress], xmm0
		shl rax, 3
		mov rcx, 16*8
		sub rcx, rax
		xor rdi, rdi
		%assign index 0
		%rep 4
			test rcx, rcx
			js .OrphansCopied
			mov rax, 0xffffffff
			shr rax, cl
			cmp rcx, 31
			cmovg rax, rdi ; rdi = 0
			mov [vecOrphansSuppress + (3-index)*COMPV_YASM_UINT32_SZ_BYTES], dword eax
			sub rcx, 32
			%assign index index+1
		%endrep
		.OrphansCopied:
			; movdqa vecOrphansSuppress, [vecOrphansSuppress]
		.OrphansIsZero:

	mov data, arg(0)
	mov width, arg(1)
	mov height, arg(2)
	lea widthMinus63, [width - 63]
	lea widthMinus15, [width - 15]
	
	pxor vecSumh, vecSumh
	pxor vecSuml, vecSuml
	movdqa [vecZero], vecSumh

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		xor i, i
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < widthSigned - 63; i += 64)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		test widthMinus63, widthMinus63
		jle .EndOf_LoopWidth64
		.LoopWidth64:
			movdqa xmm0, [data + (i + 0)*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa xmm1, [data + (i + 16)*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa xmm2, [data + (i + 32)*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa xmm3, xmm0
			movdqa xmm4, xmm1
			movdqa xmm5, xmm2
			punpcklbw xmm0, [vecZero]
			punpcklbw xmm1, [vecZero]
			punpcklbw xmm2, [vecZero]
			punpckhbw xmm3, [vecZero]
			punpckhbw xmm4, [vecZero]
			punpckhbw xmm5, [vecZero]
			paddw xmm0, xmm1
			movdqa xmm1, [data + (i + 48)*COMPV_YASM_UINT8_SZ_BYTES]
			add i, 64
			paddw xmm2, xmm3
			movdqa xmm3, xmm1
			punpcklbw xmm1, [vecZero]
			punpckhbw xmm3, [vecZero]
			paddw xmm4, xmm5
			paddw xmm0, xmm2 
			paddw xmm1, xmm3
			paddw xmm0, xmm4
			paddw xmm0, xmm1
			cmp i, widthMinus63
			movdqa xmm1, xmm0
			punpcklwd xmm0, [vecZero]
			punpckhwd xmm1, [vecZero]
			paddd vecSuml, xmm0
			paddd vecSumh, xmm1
			jl .LoopWidth64
			.EndOf_LoopWidth64:
			;; EndOf_LoopWidth64 ;;

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < widthSigned - 15; i += 16)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, widthMinus15
		jge .EndOf_LoopWidth16
		.LoopWidth16:
			movdqa xmm0, [data + (i + 0)*COMPV_YASM_UINT8_SZ_BYTES]
			movdqa xmm1, xmm0
			punpcklbw xmm0, [vecZero]
			punpckhbw xmm1, [vecZero]
			paddw xmm0, xmm1
			add i, 16
			movdqa xmm1, xmm0
			punpcklwd xmm0, [vecZero]
			punpckhwd xmm1, [vecZero]
			cmp i, widthMinus15
			paddd vecSuml, xmm0
			paddd vecSumh, xmm1
			jl .LoopWidth16
			.EndOf_LoopWidth16:
			;; EndOf_LoopWidth16 ;;
		
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if (orphans)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		test width, 15
		jz .NoOrphans
			movdqa xmm0, [data + (i + 0)*COMPV_YASM_UINT8_SZ_BYTES]
			pand xmm0, [vecOrphansSuppress]
			movdqa xmm1, xmm0
			punpcklbw xmm0, [vecZero]
			punpckhbw xmm1, [vecZero]
			paddw xmm0, xmm1
			movdqa xmm1, xmm0
			punpcklwd xmm0, [vecZero]
			punpckhwd xmm1, [vecZero]
			paddd vecSuml, xmm0
			paddd vecSumh, xmm1
			.NoOrphans:

		
		add data, arg(3) ; data += stride
		dec height
		jnz .LoopHeight
		;; EndOf_LoopHeight ;;


	paddd vecSuml, vecSumh
	movdqa xmm0, vecSuml
	psrldq xmm0, 8
	paddd vecSuml, xmm0
	movdqa xmm1, vecSuml
	psrldq xmm1, 4
	paddd vecSuml, xmm1

	mov rax, arg(4) ; sum1
	movd edx, vecSuml
	mov [rax], dword edx

	%undef data
	%undef width
	%undef height
	%undef stride
	%undef i
	%undef widthMinus63
	%undef widthMinus15
	%undef vecZero
	%undef vecSumh
	%undef vecSuml
	%undef vecOrphansSuppress

	; free memory and unalign stack
	add rsp, (2*COMPV_YASM_XMM_SZ_BYTES)
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
; arg(0) -> const COMPV_ALIGNED(SSE) int16_t* a
; arg(1) -> const COMPV_ALIGNED(SSE) int16_t* b
; arg(2) -> COMPV_ALIGNED(SSE) uint16_t* r
; arg(3) -> compv_uscalar_t width
; arg(4) -> compv_uscalar_t height
; arg(5) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
sym(CompVMathUtilsSumAbs_16s16u_Asm_X86_SSSE3):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	COMPV_YASM_SAVE_XMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	mov rcx, arg(5)
	mov rax, arg(0) ; rax = a
	mov rdx, arg(1) ; rdx = b
	lea rcx, [rcx * COMPV_YASM_INT16_SZ_BYTES]
	mov rbx, arg(2) ; rbx = r
	mov rdi, arg(3) 
	lea rdi, [rdi - 31] ; rdi = width - 31
	mov rsi, arg(4) ; rsi = height
	mov arg(5), rcx ; strideInBytes

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopRows
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width_ - 31; i += 32)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor rcx, rcx ; rcx = i = 0
		test rdi, rdi
		jle .EndOfLoopCols32
		.LoopCols32:
			pabsw xmm0, [rax + (rcx + 0)*COMPV_YASM_INT16_SZ_BYTES]
			pabsw xmm1, [rax + (rcx + 8)*COMPV_YASM_INT16_SZ_BYTES]
			pabsw xmm2, [rax + (rcx + 16)*COMPV_YASM_INT16_SZ_BYTES]
			pabsw xmm3, [rax + (rcx + 24)*COMPV_YASM_INT16_SZ_BYTES]
			pabsw xmm4, [rdx + (rcx + 0)*COMPV_YASM_INT16_SZ_BYTES]
			pabsw xmm5, [rdx + (rcx + 8)*COMPV_YASM_INT16_SZ_BYTES]
			pabsw xmm6, [rdx + (rcx + 16)*COMPV_YASM_INT16_SZ_BYTES]
			pabsw xmm7, [rdx + (rcx + 24)*COMPV_YASM_INT16_SZ_BYTES]
			add rcx, 32
			paddusw xmm0, xmm4
			paddusw xmm1, xmm5
			paddusw xmm2, xmm6
			paddusw xmm3, xmm7
			cmp rcx, rdi
			movdqa [rbx + (rcx - 32)*COMPV_YASM_INT16_SZ_BYTES], xmm0
			movdqa [rbx + (rcx - 24)*COMPV_YASM_INT16_SZ_BYTES], xmm1
			movdqa [rbx + (rcx - 16)*COMPV_YASM_INT16_SZ_BYTES], xmm2
			movdqa [rbx + (rcx - 8)*COMPV_YASM_INT16_SZ_BYTES], xmm3
			jl .LoopCols32
		.EndOfLoopCols32:

		add rdi, 31

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width_; i += 8)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp rcx, rdi
		jge .EndOfLoopCols8
		.LoopCols8:
			pabsw xmm0, [rax + (rcx + 0)*COMPV_YASM_INT16_SZ_BYTES]
			pabsw xmm1, [rdx + (rcx + 0)*COMPV_YASM_INT16_SZ_BYTES]
			add rcx, 8
			paddusw xmm0, xmm1
			cmp rcx, rdi
			movdqa [rbx + (rcx - 8)*COMPV_YASM_INT16_SZ_BYTES], xmm0
			jl .LoopCols8
		.EndOfLoopCols8:

		sub rdi, 31

		add rax, arg(5)
		add rdx, arg(5)
		add rbx, arg(5)
		dec rsi
		jnz .LoopRows

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
; arg(0) -> COMPV_ALIGNED(SSE) const int32_t* a
; arg(1) -> COMPV_ALIGNED(SSE) const int32_t* b
; arg(2) -> COMPV_ALIGNED(SSE) int32_t* s
; arg(3) -> compv_uscalar_t width
; arg(4) -> compv_uscalar_t height
; arg(5) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
sym(CompVMathUtilsSum2_32s32s_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; alloc memory
	sub rsp, COMPV_YASM_REG_SZ_BYTES
	; [rsp + 0] = strideInBytes

	mov rax, arg(5)
	shl rax, 2
	mov [rsp + 0], rax

	mov rsi, arg(0) ; rsi = a
	mov rdi, arg(1) ; rdi = b
	mov rbx, arg(2) ; rbx = s
	mov rdx, arg(4) ; rdx = height
	mov rax, arg(3) ; rax = width

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopRows
		lea rax, [rax - 15] ; rax = width - 15
		xor rcx, rcx ; rcx = i = 0
		cmp rcx, rax
		jge .EndOfLoopCols16
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < width_ - 15; i += 16) 
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopCols16
			add rcx, 16
			movdqa xmm0, [rsi + rcx*COMPV_YASM_INT32_SZ_BYTES - 64]
			movdqa xmm1, [rsi + rcx*COMPV_YASM_INT32_SZ_BYTES - 48]
			movdqa xmm2, [rsi + rcx*COMPV_YASM_INT32_SZ_BYTES - 32]
			movdqa xmm3, [rsi + rcx*COMPV_YASM_INT32_SZ_BYTES - 16]
			paddd xmm0, [rdi + rcx*COMPV_YASM_INT32_SZ_BYTES - 64]
			paddd xmm1, [rdi + rcx*COMPV_YASM_INT32_SZ_BYTES - 48]
			cmp rcx, rax
			paddd xmm2, [rdi + rcx*COMPV_YASM_INT32_SZ_BYTES - 32]
			paddd xmm3, [rdi + rcx*COMPV_YASM_INT32_SZ_BYTES - 16]
			movdqa [rbx + rcx*COMPV_YASM_INT32_SZ_BYTES - 64], xmm0
			movdqa [rbx + rcx*COMPV_YASM_INT32_SZ_BYTES - 48], xmm1
			movdqa [rbx + rcx*COMPV_YASM_INT32_SZ_BYTES - 32], xmm2
			movdqa [rbx + rcx*COMPV_YASM_INT32_SZ_BYTES - 16], xmm3			
			jl .LoopCols16
		.EndOfLoopCols16:

		lea rax, [rax + 15] ; rax = width
		cmp rcx, rax
		jge .EndOfLoopCols4
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < width_; i += 4)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopCols4:
			add rcx, 4
			movdqa xmm0, [rsi + rcx*COMPV_YASM_INT32_SZ_BYTES - 16]
			cmp rcx, rax
			paddd xmm0, [rdi + rcx*COMPV_YASM_INT32_SZ_BYTES - 16]
			movdqa [rbx + rcx*COMPV_YASM_INT32_SZ_BYTES - 16], xmm0			
			jl .LoopCols4
		.EndOfLoopCols4
		
		add rsi, [rsp + 0]
		add rdi, [rsp + 0]
		add rbx, [rsp + 0]

		dec rdx
		jnz .LoopRows

	; free memory
	add rsp, COMPV_YASM_REG_SZ_BYTES

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) const uint16_t* in
; arg(1) -> const compv_float32_t* scale1
; arg(2) -> COMPV_ALIGNED(SSE) uint8_t* out
; arg(3) -> compv_uscalar_t width
; arg(4) -> compv_uscalar_t height
; arg(5) -> COMPV_ALIGNED(SSE) compv_uscalar_t stride
sym(CompVMathUtilsScaleAndClipPixel8_16u32f_Asm_X86_SSE2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	COMPV_YASM_SAVE_XMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 16, rax
	sub rsp, (1*COMPV_YASM_REG_SZ_BYTES) + (2*COMPV_YASM_XMM_SZ_BYTES)

	%define widthMinus31	rsp + 0
	%define vecZero			widthMinus31 + (1*COMPV_YASM_REG_SZ_BYTES)
	%define vecScale		vecZero + (1*COMPV_YASM_XMM_SZ_BYTES)

	%define i		rcx
	%define in		rbx
	%define out		rdx
	%define width	rsi
	%define height	rdi
	%define stride	rax

	mov width, arg(3)

	mov rax, arg(1) ; scale1
	movss xmm0, [rax]
	lea rax, [width - 31]
	shufps xmm0, xmm0, 0x0
	pxor xmm1, xmm1
	movdqa [vecScale], xmm0
	movdqa [vecZero], xmm1
	mov [widthMinus31], rax

	mov in, arg(0)
	mov out, arg(2)
	mov height, arg(4)
	mov stride, arg(5)

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 0; j < height; ++j) 
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		xor i, i
		cmp i, [widthMinus31]
		jge .EndOf_LoopWidth32
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < widthSigned - 31; i += 32)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopWidth32
			movdqa xmm0, [in + (i + 0)*COMPV_YASM_UINT16_SZ_BYTES]
			movdqa xmm2, [in + (i + 8)*COMPV_YASM_UINT16_SZ_BYTES]
			movdqa xmm4, [in + (i + 16)*COMPV_YASM_UINT16_SZ_BYTES]
			movdqa xmm6, [in + (i + 24)*COMPV_YASM_UINT16_SZ_BYTES]
			movdqa xmm1, xmm0
			movdqa xmm3, xmm2
			movdqa xmm5, xmm4
			movdqa xmm7, xmm6
			punpcklwd xmm0, [vecZero]
			punpckhwd xmm1, [vecZero]
			punpcklwd xmm2, [vecZero]
			punpckhwd xmm3, [vecZero]
			punpcklwd xmm4, [vecZero]
			punpckhwd xmm5, [vecZero]
			punpcklwd xmm6, [vecZero]
			punpckhwd xmm7, [vecZero]
			cvtdq2ps xmm0, xmm0
			cvtdq2ps xmm1, xmm1
			cvtdq2ps xmm2, xmm2
			cvtdq2ps xmm3, xmm3
			cvtdq2ps xmm4, xmm4
			cvtdq2ps xmm5, xmm5
			cvtdq2ps xmm6, xmm6
			cvtdq2ps xmm7, xmm7
			mulps xmm0, [vecScale]
			mulps xmm1, [vecScale]
			mulps xmm2, [vecScale]
			mulps xmm3, [vecScale]
			mulps xmm4, [vecScale]
			mulps xmm5, [vecScale]
			mulps xmm6, [vecScale]
			mulps xmm7, [vecScale]
			add i, 32
			cvttps2dq xmm0, xmm0
			cvttps2dq xmm1, xmm1
			cvttps2dq xmm2, xmm2
			cvttps2dq xmm3, xmm3
			cvttps2dq xmm4, xmm4
			cvttps2dq xmm5, xmm5
			cvttps2dq xmm6, xmm6
			cvttps2dq xmm7, xmm7
			cmp i, [widthMinus31]
			packssdw xmm0, xmm1
			packssdw xmm2, xmm3
			packssdw xmm4, xmm5
			packssdw xmm6, xmm7
			packuswb xmm0, xmm2
			packuswb xmm4, xmm6
			movdqa [out + (i - 32)*COMPV_YASM_UINT8_SZ_BYTES], xmm0
			movdqa [out + (i - 16)*COMPV_YASM_UINT8_SZ_BYTES], xmm4			
			jl .LoopWidth32
			.EndOf_LoopWidth32:
			;; EndOf_LoopWidth32 ;;
		
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (; i < widthSigned; i += 8)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_LoopWidth8
		.LoopWidth8:
			movdqa xmm0, [in + (i + 0)*COMPV_YASM_UINT16_SZ_BYTES]
			movdqa xmm1, xmm0
			punpcklwd xmm0, [vecZero]
			punpckhwd xmm1, [vecZero]
			cvtdq2ps xmm0, xmm0
			cvtdq2ps xmm1, xmm1
			mulps xmm0, [vecScale]
			mulps xmm1, [vecScale]
			add i, 8
			cvttps2dq xmm0, xmm0
			cvttps2dq xmm1, xmm1
			cmp i, width
			packssdw xmm0, xmm1
			packuswb xmm0, xmm0
			movq [out + (i - 8)*COMPV_YASM_UINT8_SZ_BYTES], xmm0
			jl .LoopWidth8
			.EndOf_LoopWidth8:
			;; EndOf_LoopWidth8 ;;

		dec height
		lea out, [out + stride*COMPV_YASM_UINT8_SZ_BYTES]
		lea in, [in + stride*COMPV_YASM_UINT16_SZ_BYTES]
		jnz .LoopHeight
		;; EndOf_LoopHeight ;;

	%undef vecZero
	%undef vecScale

	%undef widthMinus31
	%undef i		
	%undef in		
	%undef out		
	%undef width
	%undef height
	%undef stride

	; free memory and unalign stack
	add rsp, (1*COMPV_YASM_REG_SZ_BYTES) + (2*COMPV_YASM_XMM_SZ_BYTES)
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
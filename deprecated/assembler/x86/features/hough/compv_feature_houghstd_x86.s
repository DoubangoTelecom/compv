;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;%include "../../compv_common_x86.s"
;
;COMPV_YASM_DEFAULT_REL
;
;global sym(HoughStdAccGatherRowThreadSafe_Asm_X86)
;global sym(HoughStdAccGatherRowThreadSafe_Asm_X64)
;
;section .data
;
;section .text
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> int32_t* pACC
; arg(1) -> int32_t accStride
; arg(2) -> const uint8_t* pixels
; arg(3) -> int32_t maxCols
; arg(4) -> int32_t maxThetaCount
; arg(5) -> int32_t row, 
; arg(6) -> const int32_t* pCosRho
; arg(7) -> const int32_t* pSinRho
;sym(HoughStdAccGatherRowThreadSafe_Asm_X64):
;	push rbp
;	mov rbp, rsp
;	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
;	push rsi
;	push rdi
;	push rbx
;	push r12
;	push r13
;	push r14
;	push r15
;	;; end prolog ;;
;	
;	movsxd r8, dword arg(4) ; r8 = maxThetaCount
;	movsxd r9, dword arg(3) ; r9 = maxCols
;	movsxd r10, dword arg(5) ; r10 = row
;	movsxd r11, dword arg(1) ; r11 = accStride
;	mov r12, arg(2) ; r12 = pixels
;	mov r13, arg(0) ; r13 = pACC
;	mov rsi, arg(6) ; rsi = pCosRho
;	mov rbx, arg(7) ; rbx = pSinRho
;
;	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;	; for (col = 0; col < maxCols; ++col)
;	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;	xor rcx, rcx ; rcx = col = 0
;	.LoopCols
;		movzx rax, byte [r12 + rcx] 
;		test rax, rax
;		jz .PixelNull
;			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;			; for (theta = 0; theta < maxThetaCount; ++theta)
;			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;			xor rax, rax ; rax = theta = 0
;			.LoopTheta
;				movsxd rdx, dword [rbx + rax*4]
;				movsxd rdi, dword [rsi + rax*4]
;				imul rdx, r10
;				imul rdi, rcx
;				lea rdx, [rdx + rdi]
;				sar rdx, 16
;				imul rdx, r11
;				neg rdx
;				lea rdx, [rdx + rax]
;				inc rax
;				;lock inc dword ptr [r13 + rdx]
;				inc dword ptr [r13 + rdx*4]
;								
;				cmp rax, r8
;				jl .LoopTheta
;
;		.PixelNull
;
;		inc rcx
;		cmp rcx, r9
;		jl .LoopCols
;
;	;; begin epilog ;;
;	pop r15
;	pop r14
;	pop r13
;	pop r12
;	pop rbx
;	pop rdi
;	pop rsi
;	COMPV_YASM_UNSHADOW_ARGS
;	mov rsp, rbp
;	pop rbp
;	ret
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> int32_t* pACC
; arg(1) -> int32_t accStride
; arg(2) -> const uint8_t* pixels
; arg(3) -> int32_t maxCols
; arg(4) -> int32_t maxThetaCount
; arg(5) -> int32_t row, 
; arg(6) -> const int32_t* pCosRho
; arg(7) -> const int32_t* pSinRho
;sym(HoughStdAccGatherRowThreadSafe_Asm_X86):
;	push rbp
;	mov rbp, rsp
;	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
;	push rsi
;	push rdi
;	push rbx
;	;; end prolog ;;
;
;	; alloc memory
;	sub rsp, 8 + 8 + 8 + 8
;	; [rsp + 0] = maxThetaCount
;	; [rsp + 8] = maxCols
;	; [rsp + 16] = row
;	; [rsp + 24] = accStride
;	
;	movsxd rax, dword arg(4)
;	mov [rsp + 0], rax
;	movsxd rax, dword arg(3)
;	mov [rsp + 8], rax
;	movsxd rax, dword arg(5)
;	mov [rsp + 16], rax
;	movsxd rax, dword arg(1)
;	mov [rsp + 24], rax
;	mov rsi, arg(6) ; rsi = pCosRho
;	mov rbx, arg(7) ; rbx = pSinRho
;
;	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;	; for (col = 0; col < maxCols; ++col)
;	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;	xor rcx, rcx ; rcx = col = 0
;	.LoopCols
;		mov rdi, arg(2)
;		movzx rax, byte [rdi + rcx] 
;		test rax, rax
;		jz .PixelNull
;			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;			; for (theta = 0; theta < maxThetaCount; ++theta)
;			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;			xor rax, rax ; rax = theta = 0
;			.LoopTheta
;				movsxd rdx, dword [rbx + rax*4]
;				movsxd rdi, dword [rsi + rax*4]
;				imul rdx, [rsp + 16]
;				imul rdi, rcx
;				add rdx, rdi
;				sar rdx, 16
;				imul rdx, [rsp + 24]
;				neg rdx
;				add rdx, rax
;				mov rdi, arg(0)
;				;lock inc dword ptr [rdi + rdx]
;				inc dword ptr [rdi + rdx*4]
;
;				inc rax
;				cmp rax, [rsp + 0]
;				jl .LoopTheta
;
;
;		.PixelNull
;
;		inc rcx
;		cmp rcx, [rsp + 8]
;		jl .LoopCols
;
;	; free memory
;	add rsp, 8 + 8 + 8 + 8
;
;	;; begin epilog ;;
;	pop rbx
;	pop rdi
;	pop rsi
;	COMPV_YASM_UNSHADOW_ARGS
;	mov rsp, rbp
;	pop rbp
;	ret
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "../compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(MatrixMulGA_float64_Asm_X86_AVX)
global sym(MatrixMulGA_float32_Asm_X86_AVX)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) compv_float64_t* ri
; arg(1) -> COMPV_ALIGNED(SSE) compv_float64_t* rj
; arg(2) -> const compv_float64_t* c1
; arg(3) -> const compv_float64_t* s1
; arg(4) -> compv_uscalar_t count
; void MatrixMulGA_float64_Asm_X86_AVX(COMPV_ALIGNED(AVX) compv_float64_t* ri, COMPV_ALIGNED(AVX) compv_float64_t* rj, const compv_float64_t* c1, const compv_float64_t* s1, compv_uscalar_t count)
sym(MatrixMulGA_float64_Asm_X86_AVX):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	push rbx
	;; end prolog ;;

	xor rcx, rcx ; rcx = i
	mov rbx, arg(4) ; rbx = count
	imul rbx, 8

	mov rax, arg(2)
	mov rdx, arg(3)
	vbroadcastsd ymm0, [rax]
	vbroadcastsd ymm1, [rdx]

	mov rax, arg(0) ; ri
	mov rdx, arg(1) ; rj 

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; while (i < count)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopProcess
		vmovapd ymm2, [rax + rcx] ; ymmRI
		vmovapd ymm3, [rdx + rcx] ; ymmRJ

		add rcx, 32

%if 0 ; FMA3 disabled
		vmulpd ymm4, ymm1, ymm2
		vmulpd ymm2, ymm2, ymm0
		vfmadd231pd ymm2, ymm3, ymm1
		vfmsub231pd ymm4, ymm0, ymm3
		vmovapd [rax + rcx - 32], ymm2
		vmovapd [rdx + rcx - 32], ymm4		
%else
		vmulpd ymm4, ymm1, ymm2
		vmulpd ymm2, ymm2, ymm0
		vmulpd ymm5, ymm0, ymm3
		vmulpd ymm3, ymm3, ymm1
		vsubpd ymm5, ymm5, ymm4
		vaddpd ymm2, ymm2, ymm3
		vmovapd [rdx + rcx - 32], ymm5
		vmovapd [rax + rcx - 32], ymm2
%endif
		
		cmp rcx, rbx
		jl .LoopProcess

	;; begin epilog ;;
	pop rbx
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(SSE) compv_float32_t* ri
; arg(1) -> COMPV_ALIGNED(SSE) compv_float32_t* rj
; arg(2) -> const compv_float32_t* c1
; arg(3) -> const compv_float32_t* s1
; arg(4) -> compv_uscalar_t count
; void MatrixMulGA_float32_Asm_X86_AVX(COMPV_ALIGNED(AVX) compv_float32_t* ri, COMPV_ALIGNED(AVX) compv_float32_t* rj, const compv_float32_t* c1, const compv_float32_t* s1, compv_uscalar_t count)
sym(MatrixMulGA_float32_Asm_X86_AVX):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	push rbx
	;; end prolog ;;

	xor rcx, rcx ; rcx = i
	mov rbx, arg(4) ; rbx = count
	imul rbx, 4

	mov rax, arg(2)
	mov rdx, arg(3)
	vbroadcastss ymm0, [rax]
	vbroadcastss ymm1, [rdx]

	mov rax, arg(0) ; ri
	mov rdx, arg(1) ; rj 

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; while (i < count)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopProcess
		vmovaps ymm2, [rax + rcx] ; ymmRI
		vmovaps ymm3, [rdx + rcx] ; ymmRJ

		add rcx, 32

%if 0 ; FMA3 disabled
		vmulps ymm4, ymm1, ymm2
		vmulps ymm2, ymm2, ymm0
		vfmadd231ps ymm2, ymm3, ymm1
		vfmsub231ps ymm4, ymm0, ymm3
		vmovaps [rax + rcx - 32], ymm2
		vmovaps [rdx + rcx - 32], ymm4		
%else
		vmulps ymm4, ymm1, ymm2
		vmulps ymm2, ymm2, ymm0
		vmulps ymm5, ymm0, ymm3
		vmulps ymm3, ymm3, ymm1
		vsubps ymm5, ymm5, ymm4
		vaddps ymm2, ymm2, ymm3
		vmovaps [rdx + rcx - 32], ymm5
		vmovaps [rax + rcx - 32], ymm2
%endif
		
		cmp rcx, rbx
		jl .LoopProcess

	;; begin epilog ;;
	pop rbx
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret

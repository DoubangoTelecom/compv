; Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
;
; This file is part of Open Source ComputerVision (a.k.a CompV) project.
; Source code hosted at https://github.com/DoubangoTelecom/compv
; Website hosted at http://compv.org
;
; CompV is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; CompV is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with CompV.
;
%include "compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(compv_mathutils_maxval_asm_x86_cmov)
global sym(compv_mathutils_minval_asm_x86_cmov)
global sym(compv_mathutils_clip3_asm_x86_cmov)
global sym(compv_mathutils_clip2_asm_x86_cmov)

section .data


section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; compv_scalar_t maxVal(compv_scalar_t x, compv_scalar_t y);
sym(compv_mathutils_maxval_asm_x86_cmov):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 2
	mov rdx, arg(1)
	mov rax, arg(0)
	cmp rdx, rax 
	cmovg rax, rdx
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; compv_scalar_t minVal(compv_scalar_t x, compv_scalar_t y);
sym(compv_mathutils_minval_asm_x86_cmov):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 2
	mov rdx, arg(1)
	mov rax, arg(0)
	cmp rdx, rax 
	cmovl rax, rdx
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; compv_scalar_t clip3(compv_scalar_t min, compv_scalar_t max, compv_scalar_t val);
sym(compv_mathutils_clip3_asm_x86_cmov):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 3
	mov rax, arg(2)
	mov rdx, arg(1)
	mov rcx, arg(0)
	cmp rdx, rax 
	cmovl rax, rdx
	cmp rcx, rax
	cmovg rax, rcx
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; compv_scalar_t clip2(compv_scalar_t max, compv_scalar_t val);
sym(compv_mathutils_clip2_asm_x86_cmov):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 2
	xor rcx, rcx
	mov rax, arg(1)
	mov rdx, arg(0)
	cmp rdx, rax 
	cmovl rax, rdx
	cmp rax, 0
	cmovl rax, rcx
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
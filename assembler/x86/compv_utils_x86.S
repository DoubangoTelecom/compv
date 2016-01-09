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
%include "compv_common_x86.S"

global sym(compv_utils_thread_get_core_id_x86_asm)
global sym(compv_utils_rdtsc_x86_asm)


;;; void _compv_utils_rdtsc_x86_asm()
sym(compv_utils_rdtsc_x86_asm):	
	rdtsc
	ret

;;; int compv_utils_thread_get_core_id_x86_asm();
sym(compv_utils_thread_get_core_id_x86_asm):
	mov eax, 1
    cpuid
    shr ebx, 24
    mov eax, ebx
    ret

;;;
;;; void testPrologEpilog(int a, int b, int c)
sym(rgbaToI420Kernel11_CompY_Asm_X86_Aligned1x_SSSE3):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 3
	COMPV_YASM_SAVE_XMM 15 ;XMM[6-15]
	push rsi
	push rdi
	push rbx
	; end prolog

	; begin epilog
	pop rbx
	pop rdi
	pop rsi
    COMPV_YASM_RESTORE_XMM
    COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	ret
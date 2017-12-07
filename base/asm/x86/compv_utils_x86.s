;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

global sym(compv_utils_thread_get_core_id_x86_asm)
global sym(compv_utils_rdtsc_x86_asm)

section .data

section .text

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
sym(testPrologEpilog):
	; vzeroupper ; For AVX
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 3
	COMPV_YASM_SAVE_XMM 15 ;XMM[6-15] ; COMPV_YASM_SAVE_YMM for AVX
	push rsi
	push rdi
	push rbx
	;push r12
	;push r13
	;push r14
	;push r15
	;; end prolog ;;

	; align stack and alloc memory
	; COMPV_YASM_ALIGN_STACK 32, rax
	; sub rsp, xxx

	; free memory and unalign stack
	; add rsp, xxx
	; COMPV_YASM_UNALIGN_STACK

	;; begin epilog ;;
	;pop r15
	;pop r14
	;pop r13
	;pop r12
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_XMM ; COMPV_YASM_RESTORE_YMM for AVX
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	; vzeroupper ; For AVX
	ret
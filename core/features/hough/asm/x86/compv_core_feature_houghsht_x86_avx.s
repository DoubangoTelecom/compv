;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(CompVHoughShtAccGatherRow_8mpd_Asm_X86_AVX2)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> COMPV_ALIGNED(AVX) const int32_t* pCosRho
; arg(1) -> COMPV_ALIGNED(AVX) const int32_t* pRowTimesSinRho
; arg(2) -> compv_uscalar_t col
; arg(3) -> int32_t* pACC
; arg(4) -> compv_uscalar_t accStride
; arg(5) -> compv_uscalar_t maxTheta
sym(CompVHoughShtAccGatherRow_8mpd_Asm_X86_AVX2):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 6
	%if COMPV_YASM_ABI_IS_64BIT
		COMPV_YASM_SAVE_YMM 11
	%else
		COMPV_YASM_SAVE_YMM 7
	%endif
	push rsi
	push rdi
	push rbx
	%if COMPV_YASM_ABI_IS_64BIT
		push r12
		push r13
		push r14
		push r15
	%endif
	;; end prolog ;;

	; alloc memory
	sub rsp, (1*COMPV_YASM_REG_SZ_BYTES) + (8*COMPV_YASM_INT32_SZ_BYTES)

	%define maxThetaMinus31	rsp + 0
	%define vecThetaMem		maxThetaMinus31 + COMPV_YASM_REG_SZ_BYTES

	%define theta			rcx
	%define vec8			ymm4
	%define vecTheta		ymm5		
	%define vecStride		ymm6
	%define vecColInt32		ymm7

	mov rax, arg(5)
	lea rax, [rax - 31]
	mov [maxThetaMinus31], rax

	%assign index 0
	%rep 2
		mov rax, index + 0
		mov rcx, index + 1
		mov rdx, index + 2
		mov rbx, index + 3
		mov [vecThetaMem + (index + 0)*COMPV_YASM_INT32_SZ_BYTES], dword eax
		mov [vecThetaMem + (index + 1)*COMPV_YASM_INT32_SZ_BYTES], dword ecx
		mov [vecThetaMem + (index + 2)*COMPV_YASM_INT32_SZ_BYTES], dword edx
		mov [vecThetaMem + (index + 3)*COMPV_YASM_INT32_SZ_BYTES], dword ebx
		%assign index index+4
	%endrep
	vmovdqu vecTheta, [vecThetaMem]

	mov rax, 8
	mov rdx, arg(4)
	mov rcx, arg(2)
	vmovd xmm0, eax
	vmovd xmm1, edx
	vmovd xmm2, ecx
	vpbroadcastd vec8, xmm0
	vpbroadcastd vecStride, xmm1
	vpbroadcastd vecColInt32, xmm2

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (theta = 0; theta < maxTheta - 31; theta += 32)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	xor theta, theta
	.LoopTheta32:
		mov rax, arg(0) ; pCosRho
		mov rdx, arg(1) ; pRowTimesSinRho
		vpmulld ymm0, vecColInt32, [rax + (theta + 0)*COMPV_YASM_INT32_SZ_BYTES]
		vpmulld ymm1, vecColInt32, [rax + (theta + 8)*COMPV_YASM_INT32_SZ_BYTES]
		vpmulld ymm2, vecColInt32, [rax + (theta + 16)*COMPV_YASM_INT32_SZ_BYTES]
		vpmulld ymm3, vecColInt32, [rax + (theta + 24)*COMPV_YASM_INT32_SZ_BYTES]
		
		vpaddd ymm0, ymm0, [rdx + (theta + 0)*COMPV_YASM_INT32_SZ_BYTES]
		vpaddd ymm1, ymm1, [rdx + (theta + 8)*COMPV_YASM_INT32_SZ_BYTES]
		vpaddd ymm2, ymm2, [rdx + (theta + 16)*COMPV_YASM_INT32_SZ_BYTES]
		vpaddd ymm3, ymm3, [rdx + (theta + 24)*COMPV_YASM_INT32_SZ_BYTES]
		
		vpsrad ymm0, ymm0, 16
		vpsrad ymm1, ymm1, 16
		vpsrad ymm2, ymm2, 16
		vpsrad ymm3, ymm3, 16

		vpmulld ymm0, ymm0, vecStride
		vpmulld ymm1, ymm1, vecStride
		vpmulld ymm2, ymm2, vecStride
		vpmulld ymm3, ymm3, vecStride

		add theta, 32

		vpsubd ymm0, vecTheta, ymm0
		vpaddd vecTheta, vecTheta, vec8
		vpsubd ymm1, vecTheta, ymm1
		vpaddd vecTheta, vecTheta, vec8
		vpsubd ymm2, vecTheta, ymm2
		vpaddd vecTheta, vecTheta, vec8
		vpsubd ymm3, vecTheta, ymm3
		vpaddd vecTheta, vecTheta, vec8

		mov rax, arg(3) ; pACC
		
		%if COMPV_YASM_ABI_IS_64BIT
			vextracti128 xmm8, ymm0, 1
			vextracti128 xmm9, ymm1, 1
			vextracti128 xmm10, ymm2, 1
			vextracti128 xmm11, ymm3, 1
			vpmovsxdq ymm0, xmm0
			vpmovsxdq ymm1, xmm1
			vpmovsxdq ymm2, xmm2
			vpmovsxdq ymm3, xmm3
			vpmovsxdq ymm8, xmm8
			vpmovsxdq ymm9, xmm9
			vpmovsxdq ymm10, xmm10
			vpmovsxdq ymm11, xmm11
			vmovq qword rbx, xmm0
			vpextrq rsi, xmm0, 1
			vmovq qword rdi, xmm1
			vpextrq rdx, xmm1, 1
			vmovq qword r8, xmm2
			vpextrq r9, xmm2, 1
			vmovq qword r10, xmm3
			vpextrq r11, xmm3, 1
			vmovq qword r12, xmm8
			vpextrq r13, xmm8, 1
			vmovq qword r14, xmm9
			vpextrq r15, xmm9, 1
			inc dword ptr [rax+rbx*COMPV_YASM_INT32_SZ_BYTES]
			vmovq qword rbx, xmm10
			inc dword ptr [rax+rsi*COMPV_YASM_INT32_SZ_BYTES]
			vpextrq rsi, xmm10, 1
			inc dword ptr [rax+rdi*COMPV_YASM_INT32_SZ_BYTES]
			vmovq qword rdi, xmm11
			inc dword ptr [rax+rdx*COMPV_YASM_INT32_SZ_BYTES]
			vpextrq rdx, xmm11, 1
			inc dword ptr [rax+r8*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [rax+r9*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [rax+r10*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [rax+r11*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [rax+r12*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [rax+r13*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [rax+r14*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [rax+r15*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [rax+rbx*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [rax+rsi*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [rax+rdi*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [rax+rdx*COMPV_YASM_INT32_SZ_BYTES]
			vextracti128 xmm0, ymm0, 1
			vextracti128 xmm1, ymm1, 1
			vextracti128 xmm2, ymm2, 1
			vextracti128 xmm3, ymm3, 1
			vextracti128 xmm8, ymm8, 1
			vextracti128 xmm9, ymm9, 1
			vextracti128 xmm10, ymm10, 1
			vextracti128 xmm11, ymm11, 1
			vmovq qword rbx, xmm0
			vpextrq rsi, xmm0, 1
			vmovq qword rdi, xmm1
			vpextrq rdx, xmm1, 1
			vmovq qword r8, xmm2
			vpextrq r9, xmm2, 1
			vmovq qword r10, xmm3
			vpextrq r11, xmm3, 1
			vmovq qword r12, xmm8
			vpextrq r13, xmm8, 1
			vmovq qword r14, xmm9
			vpextrq r15, xmm9, 1
			inc dword ptr [rax+rbx*COMPV_YASM_INT32_SZ_BYTES]
			vmovq qword rbx, xmm10
			inc dword ptr [rax+rsi*COMPV_YASM_INT32_SZ_BYTES]
			vpextrq rsi, xmm10, 1
			inc dword ptr [rax+rdi*COMPV_YASM_INT32_SZ_BYTES]
			vmovq qword rdi, xmm11
			inc dword ptr [rax+rdx*COMPV_YASM_INT32_SZ_BYTES]
			vpextrq rdx, xmm11, 1
			inc dword ptr [rax+r8*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [rax+r9*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [rax+r10*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [rax+r11*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [rax+r12*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [rax+r13*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [rax+r14*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [rax+r15*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [rax+rbx*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [rax+rsi*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [rax+rdi*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [rax+rdx*COMPV_YASM_INT32_SZ_BYTES]
		%else
			vmovd ebx, xmm0
			vpextrd esi, xmm0, 1
			vpextrd edi, xmm0, 2
			vpextrd edx, xmm0, 3
			vextracti128 xmm0, ymm0, 1
			inc dword ptr [eax+ebx*COMPV_YASM_INT32_SZ_BYTES]
			vmovd ebx, xmm0
			inc dword ptr [eax+esi*COMPV_YASM_INT32_SZ_BYTES]
			vpextrd esi, xmm0, 1
			inc dword ptr [eax+edi*COMPV_YASM_INT32_SZ_BYTES]
			vpextrd edi, xmm0, 2
			inc dword ptr [eax+edx*COMPV_YASM_INT32_SZ_BYTES]
			vpextrd edx, xmm0, 3
			vextracti128 xmm0, ymm1, 1
			inc dword ptr [eax+ebx*COMPV_YASM_INT32_SZ_BYTES]
			vmovd ebx, xmm1
			inc dword ptr [eax+esi*COMPV_YASM_INT32_SZ_BYTES]
			vpextrd esi, xmm1, 1
			inc dword ptr [eax+edi*COMPV_YASM_INT32_SZ_BYTES]
			vpextrd edi, xmm1, 2
			inc dword ptr [eax+edx*COMPV_YASM_INT32_SZ_BYTES]
			vpextrd edx, xmm1, 3
			vextracti128 xmm1, ymm2, 1
			inc dword ptr [eax+ebx*COMPV_YASM_INT32_SZ_BYTES]
			vmovd ebx, xmm0
			inc dword ptr [eax+esi*COMPV_YASM_INT32_SZ_BYTES]
			vpextrd esi, xmm0, 1
			inc dword ptr [eax+edi*COMPV_YASM_INT32_SZ_BYTES]
			vpextrd edi, xmm0, 2
			inc dword ptr [eax+edx*COMPV_YASM_INT32_SZ_BYTES]
			vpextrd edx, xmm0, 3
			inc dword ptr [eax+ebx*COMPV_YASM_INT32_SZ_BYTES]
			vmovd ebx, xmm2
			inc dword ptr [eax+esi*COMPV_YASM_INT32_SZ_BYTES]
			vpextrd esi, xmm2, 1
			inc dword ptr [eax+edi*COMPV_YASM_INT32_SZ_BYTES]
			vpextrd edi, xmm2, 2
			inc dword ptr [eax+edx*COMPV_YASM_INT32_SZ_BYTES]
			vpextrd edx, xmm2, 3
			vextracti128 xmm2, ymm3, 1
			inc dword ptr [eax+ebx*COMPV_YASM_INT32_SZ_BYTES]
			vmovd ebx, xmm1
			inc dword ptr [eax+esi*COMPV_YASM_INT32_SZ_BYTES]
			vpextrd esi, xmm1, 1
			inc dword ptr [eax+edi*COMPV_YASM_INT32_SZ_BYTES]
			vpextrd edi, xmm1, 2
			inc dword ptr [eax+edx*COMPV_YASM_INT32_SZ_BYTES]
			vpextrd edx, xmm1, 3
			inc dword ptr [eax+ebx*COMPV_YASM_INT32_SZ_BYTES]
			vmovd ebx, xmm3
			inc dword ptr [eax+esi*COMPV_YASM_INT32_SZ_BYTES]
			vpextrd esi, xmm3, 1
			inc dword ptr [eax+edi*COMPV_YASM_INT32_SZ_BYTES]
			vpextrd edi, xmm3, 2
			inc dword ptr [eax+edx*COMPV_YASM_INT32_SZ_BYTES]
			vpextrd edx, xmm3, 3
			inc dword ptr [eax+ebx*COMPV_YASM_INT32_SZ_BYTES]
			vmovd ebx, xmm2
			inc dword ptr [eax+esi*COMPV_YASM_INT32_SZ_BYTES]
			vpextrd esi, xmm2, 1
			inc dword ptr [eax+edi*COMPV_YASM_INT32_SZ_BYTES]
			vpextrd edi, xmm2, 2
			inc dword ptr [eax+edx*COMPV_YASM_INT32_SZ_BYTES]
			vpextrd edx, xmm2, 3
			inc dword ptr [eax+ebx*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [eax+esi*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [eax+edi*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [eax+edx*COMPV_YASM_INT32_SZ_BYTES]
		%endif
		cmp theta, [maxThetaMinus31]
		jl .LoopTheta32
		;; EndOf_LoopTheta32 ;;

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (; theta < maxTheta; theta += 8)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp theta, arg(5)
	jge .EndOf_LoopTheta8
	.LoopTheta8:
		mov rax, arg(0) ; pCosRho
		mov rdx, arg(1) ; pRowTimesSinRho
		vpmulld ymm0, vecColInt32, [rax + (theta + 0)*COMPV_YASM_INT32_SZ_BYTES]		
		vpaddd ymm0, ymm0, [rdx + (theta + 0)*COMPV_YASM_INT32_SZ_BYTES]		
		vpsrad ymm0, ymm0, 16
		vpmulld ymm0, ymm0, vecStride
		add theta, 8
		vpsubd ymm0, vecTheta, ymm0
		vpaddd vecTheta, vecTheta, vec8

		mov rax, arg(3) ; pACC

		%if COMPV_YASM_ABI_IS_64BIT
			vextracti128 xmm1, ymm0, 1
			vpmovsxdq ymm0, xmm0
			vpmovsxdq ymm1, xmm1
			vextracti128 xmm2, ymm0, 1
			vextracti128 xmm3, ymm1, 1
			vmovq qword rbx, xmm0
			vpextrq rsi, xmm0, 1
			vmovq qword rdi, xmm1
			vpextrq rdx, xmm1, 1
			vmovq qword r8, xmm2
			vpextrq r9, xmm2, 1
			vmovq qword r10, xmm3
			vpextrq r11, xmm3, 1
			inc dword ptr [rax+rbx*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [rax+rsi*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [rax+rdi*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [rax+rdx*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [rax+r8*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [rax+r9*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [rax+r10*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [rax+r11*COMPV_YASM_INT32_SZ_BYTES]
		%else
			vmovd ebx, xmm0
			vpextrd esi, xmm0, 1
			vpextrd edi, xmm0, 2
			vpextrd edx, xmm0, 3
			vextracti128 xmm1, ymm0, 1
			inc dword ptr [eax+ebx*COMPV_YASM_INT32_SZ_BYTES]
			vmovd ebx, xmm1
			inc dword ptr [eax+esi*COMPV_YASM_INT32_SZ_BYTES]
			vpextrd esi, xmm1, 1
			inc dword ptr [eax+edi*COMPV_YASM_INT32_SZ_BYTES]
			vpextrd edi, xmm1, 2
			inc dword ptr [eax+edx*COMPV_YASM_INT32_SZ_BYTES]
			vpextrd edx, xmm1, 3
			inc dword ptr [eax+ebx*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [eax+esi*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [eax+edi*COMPV_YASM_INT32_SZ_BYTES]
			inc dword ptr [eax+edx*COMPV_YASM_INT32_SZ_BYTES]
		%endif
		cmp theta, arg(5)
		jl .LoopTheta8
		.EndOf_LoopTheta8:
		;; EndOf_LoopTheta8 ;;

	%undef maxThetaMinus31
	%undef vecThetaMem
	%undef theta
	%undef vec4
	%undef vecTheta	
	%undef vecStride
	%undef vecColInt32

	; free memory
	add rsp, (1*COMPV_YASM_REG_SZ_BYTES) + (8*COMPV_YASM_INT32_SZ_BYTES)

	;; begin epilog ;;
	%if COMPV_YASM_ABI_IS_64BIT
		pop r15
		pop r14
		pop r13
		pop r12
	%endif
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_YMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret
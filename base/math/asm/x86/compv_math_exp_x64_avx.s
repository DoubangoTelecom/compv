;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(CompVMathExpExp_minpack4_64f64f_Asm_X64_AVX2)

section .data

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const compv_float64_t* ptrIn
; arg(1) -> compv_float64_t* ptrOut
; arg(2) -> const compv_uscalar_t width
; arg(3) -> const compv_uscalar_t height
; arg(4) -> const compv_uscalar_t stride
; arg(5) -> const uint64_t* lut64u
; arg(6) -> const uint64_t* var64u
; arg(7) -> const compv_float64_t* var64f
sym(CompVMathExpExp_minpack4_64f64f_Asm_X64_AVX2):
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 8
	COMPV_YASM_SAVE_YMM 15
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 32, rax
	sub rsp, (1*COMPV_YASM_YMM_SZ_BYTES)

	%define ptrIn		rax
	%define ptrOut		rcx
	%define width		rdx
	%define height		rbx
	%define stride		rsi
	%define lut64u		rdi
	%define var64u		r8
	%define var64f		r9

	%define i			r10

	%define vecMax		ymm0
	%define vecX		ymm1
	%define vecDI		ymm2
	%define vecT		ymm3
	%define vecU		ymm4
	%define vecY		ymm5
	%define vecTemp		ymm6
	%define vecLUT		ymm7
	%define vecC10		ymm8
	%define vecMin		ymm9
	%define vecCA		ymm10
	%define vecB		ymm11
	%define vecCRA		ymm12
	%define vecCADJ		ymm13
	%define vecMask		ymm14
	%define vecC30		ymm15

	%define mem_vecC20		rsp + 0

	mov ptrIn, arg(0)
	mov ptrOut, arg(1)
	mov width, arg(2)
	mov height, arg(3)
	mov stride, arg(4)
	mov lut64u, arg(5)
	mov var64u, arg(6)
	mov var64f, arg(7)

	and width, -4
	test width, width
	jz .EndOf_LoopHeight ; Caller cannot check width >= 4 (SVM data)

	shl stride, COMPV_YASM_FLOAT64_SHIFT_BYTES

	vpbroadcastq vecMask, [var64u + (0 * COMPV_YASM_UINT64_SZ_BYTES)]
	vpbroadcastq vecCADJ, [var64u + (1 * COMPV_YASM_UINT64_SZ_BYTES)]

	vbroadcastsd vecB, [var64f + (0 * COMPV_YASM_FLOAT64_SZ_BYTES)]
	vbroadcastsd vecCA, [var64f + (1 * COMPV_YASM_FLOAT64_SZ_BYTES)]
	vbroadcastsd vecCRA, [var64f + (2 * COMPV_YASM_FLOAT64_SZ_BYTES)]
	vbroadcastsd vecC10, [var64f + (3 * COMPV_YASM_FLOAT64_SZ_BYTES)]
	vbroadcastsd vecT, [var64f + (4 * COMPV_YASM_FLOAT64_SZ_BYTES)] ; vecC20
	vbroadcastsd vecC30, [var64f + (5 * COMPV_YASM_FLOAT64_SZ_BYTES)]
	vbroadcastsd vecMin, [var64f + (6 * COMPV_YASM_FLOAT64_SZ_BYTES)]
	vbroadcastsd vecMax, [var64f + (7 * COMPV_YASM_FLOAT64_SZ_BYTES)]
	vmovapd [mem_vecC20], vecT

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (compv_uscalar_t j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopHeight:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (compv_uscalar_t i = 0; i < width4; i += 4)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor i, i
		.LoopWidth4:
			vminpd vecX, vecMax, [ptrIn + (i*COMPV_YASM_FLOAT64_SZ_BYTES)]
			vmaxpd vecX, vecX, vecMin

			vmulpd vecDI, vecX, vecCA
			vaddpd vecDI, vecDI, vecB
			vsubpd vecT, vecDI, vecB
			vmulpd vecT, vecT, vecCRA
			vsubpd vecT, vecT, vecX

			vpaddq vecU, vecDI, vecCADJ
			vpsrlq vecU, vecU, 11
			vpsllq vecU, vecU, 52
			vmulpd vecY, vecT, vecT
			vpand vecDI, vecDI, vecMask
			vpcmpeqb vecX, vecX, vecX ; vecX now contains condition mask: changed after executing "vpgatherqq"
			vpsllq vecDI, vecDI, COMPV_YASM_FLOAT64_SHIFT_BYTES
			vpgatherqq vecLUT, qword ptr [lut64u+vecDI], vecX
			vsubpd vecTemp, vecC30, vecT
			vmulpd vecY, vecY, vecTemp
			vpor vecU, vecU, vecLUT
			vmulpd vecY, vecY, [mem_vecC20]
			vsubpd vecY, vecY, vecT
			vaddpd vecY, vecY, vecC10
			vmulpd vecY, vecY, vecU
			add i, 4
			cmp i, width
			vmovupd [ptrOut + ((i - 4)*COMPV_YASM_FLOAT64_SZ_BYTES)], vecY
			jl .LoopWidth4
		.EndOf_LoopWidth4:
		
		dec height
		lea ptrIn, [ptrIn + stride]
		lea ptrOut, [ptrOut + stride]
		jnz .LoopHeight
	.EndOf_LoopHeight:


	%undef ptrIn		
	%undef ptrOut		
	%undef width		
	%undef height		
	%undef stride		
	%undef lut64u		
	%undef var64u		
	%undef var64f		

	%undef i			

	%undef vecMax		
	%undef vecX		
	%undef vecDI		
	%undef vecT		
	%undef vecU		
	%undef vecY		
	%undef vecTemp		
	%undef vecLUT		
	%undef vecC10	
	%undef vecMin		
	%undef vecCA		
	%undef vecB		
	%undef vecCRA		
	%undef vecCADJ		
	%undef vecMask		
	%undef vecC30		

	%undef mem_vecC20

	; free memory and unalign stack
	add rsp, (1*COMPV_YASM_YMM_SZ_BYTES)
	COMPV_YASM_UNALIGN_STACK

	;; begin epilog ;;
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_RESTORE_YMM
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret

%endif ; COMPV_YASM_ABI_IS_64BIT

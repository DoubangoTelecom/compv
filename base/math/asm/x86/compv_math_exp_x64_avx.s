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

global sym(CompVMathExpExp_minpack1_64f64f_Asm_X64_AVX2)
global sym(CompVMathExpExp_minpack1_64f64f_Asm_X64_FMA3_AVX2)

section .data

section .text

%define FMA_OFF	0
%define FMA_ON	1


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; %1 -> FMA
; %2 -> i-inc
; ret -> vecY
%macro COMPV_MATH_EXP_MACRO_64f64f 2
	vminpd vecX, vecMax, [ptrIn + (i*COMPV_YASM_FLOAT64_SZ_BYTES)]
	vmaxpd vecX, vecX, vecMin
	%if %1 == FMA_OFF
		vmulpd vecDI, vecX, vecCA
		vaddpd vecDI, vecDI, vecB
		vsubpd vecT, vecDI, vecB
		vmulpd vecT, vecT, vecCRA
		vsubpd vecT, vecT, vecX
	%else
		vmovapd vecDI, vecB
		vfmadd231pd vecDI, vecX, vecCA
		vsubpd vecT, vecDI, vecB
		vfmsub213pd vecT, vecCRA, vecX
	%endif
	vpaddq vecU, vecDI, vecCADJ
	vpsrlq vecU, vecU, 11
	vpsllq vecU, vecU, 52
	vmulpd vecY, vecT, vecT
	vpand vecDI, vecDI, vecMask
	vpcmpeqq vecX, vecX, vecX ; vecX now contains condition mask: changed after executing "vpgatherqq"
	vpsllq vecDI, vecDI, COMPV_YASM_FLOAT64_SHIFT_BYTES
	vsubpd vecTemp, vecC30, vecT
	vpgatherqq vecLUT, [lut64u+vecDI], vecX			
	vmulpd vecY, vecY, vecTemp
	vpor vecU, vecU, vecLUT
	%if %1 == FMA_OFF
		vmulpd vecY, vecY, [mem_vecC20]
		add i, %2
		vsubpd vecY, vecY, vecT
	%else
		vfmsub132pd vecY, vecT, [mem_vecC20]
		add i, %2
	%endif
	vaddpd vecY, vecY, vecC10
	vmulpd vecY, vecY, vecU
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const compv_float64_t* ptrIn
; arg(1) -> compv_float64_t* ptrOut
; arg(2) -> const compv_uscalar_t width
; arg(3) -> const compv_uscalar_t height
; arg(4) -> const compv_uscalar_t stride
; arg(5) -> const uint64_t* lut64u
; arg(6) -> const uint64_t* var64u
; arg(7) -> const compv_float64_t* var64f
; %1 -> FMA_[ON/OFF]
%macro CompVMathExpExp_minpack1_64f64f_Macro_X64_AVX2 1
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
	sub rsp, (2*COMPV_YASM_YMM_SZ_BYTES)

	%define ptrIn		rax
	%define ptrOut		rcx
	%define width		rdx
	%define height		rbx
	%define stride		rsi
	%define lut64u		rdi
	%define var64u		r8
	%define var64f		r9

	%define i			r10
	%define width4		r11

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
	%define mem_vecOrphans	mem_vecC20 + COMPV_YASM_YMM_SZ_BYTES

	mov ptrIn, arg(0)
	mov ptrOut, arg(1)
	mov width, arg(2)
	mov height, arg(3)
	mov stride, arg(4)
	mov lut64u, arg(5)
	mov var64u, arg(6)
	mov var64f, arg(7)

	mov width4, width
	and width4, -4

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
		test width4, width4
		jz .EndOf_LoopWidth4
		.LoopWidth4:
			COMPV_MATH_EXP_MACRO_64f64f %1, 4
			cmp i, width4
			vmovupd [ptrOut + ((i - 4)*COMPV_YASM_FLOAT64_SZ_BYTES)], vecY
			jl .LoopWidth4
		.EndOf_LoopWidth4:

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; Orphans
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		cmp i, width
		jge .EndOf_Orphans
		.Orphans:
			COMPV_MATH_EXP_MACRO_64f64f %1, 0
			vmovapd [mem_vecOrphans], vecY
			lea var64u, [mem_vecOrphans] ; var64u now contains @mem_vecOrphans
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (; i < width; i += 1)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			.LoopWidth1:
				inc i
				mov var64f, [var64u]
				cmp i, width
				mov [ptrOut + ((i - 1)*COMPV_YASM_FLOAT64_SZ_BYTES)], var64f
				lea var64u, [var64u + COMPV_YASM_FLOAT64_SZ_BYTES]
				jl .LoopWidth1
			.EndOf_LoopWidth1:
		.EndOf_Orphans:
		
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
	%undef width4		

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
	add rsp, (2*COMPV_YASM_YMM_SZ_BYTES)
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
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVMathExpExp_minpack1_64f64f_Asm_X64_AVX2):
	CompVMathExpExp_minpack1_64f64f_Macro_X64_AVX2 FMA_OFF

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(CompVMathExpExp_minpack1_64f64f_Asm_X64_FMA3_AVX2):
	CompVMathExpExp_minpack1_64f64f_Macro_X64_AVX2 FMA_ON

%endif ; COMPV_YASM_ABI_IS_64BIT

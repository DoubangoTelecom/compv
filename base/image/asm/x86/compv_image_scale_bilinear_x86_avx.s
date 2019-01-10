;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
%include "compv_common_x86.s"
%include "compv_image_scale_bilinear_macros.s"

COMPV_YASM_DEFAULT_REL

global sym(CompVImageScaleBilinear_Asm_X86_AVX2)

section .data
	extern sym(kShuffleEpi8_Deinterleave8uL2_32s)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* inPtr
; arg(1) -> compv_uscalar_t inStride,
; arg(2) -> COMPV_ALIGNED(SSE) uint8_t* outPtr
; arg(3) -> compv_uscalar_t outWidth
; arg(4) -> compv_uscalar_t outYStart
; arg(5) -> compv_uscalar_t outYEnd
; arg(6) -> COMPV_ALIGNED(SSE) compv_uscalar_t outStride
; arg(7) -> compv_uscalar_t sf_x
; arg(8) -> compv_uscalar_t sf_y
sym(CompVImageScaleBilinear_Asm_X86_AVX2)
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 9
	COMPV_YASM_SAVE_YMM 7
	push rsi
	push rdi
	push rbx
	;; end prolog ;;

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 32, rax
	sub rsp, (22*32)
	%define memNeighb0              rsp + 0
	%define memNeighb1              memNeighb0 + 32
	%define memNeighb2              memNeighb1 + 32
	%define memNeighb3              memNeighb2 + 32
	%define vecX0                   memNeighb3 + 32
	%define vecX1                   vecX0 + 32
	%define vecX2                   vecX1 + 32
	%define vecX3                   vecX2 + 32 
	%define vec4                    vecX3 + 32
	%define vec5                    vec4 + 32
	%define vec6                    vec5 + 32
	%define vec7                    vec6 + 32
	%define vecy0                   vec7 + 32 
	%define vecy1                   vecy0 + 32
	%define vecZero                 vecy1 + 32
	%define vec0xff_epi32           vecZero + 32
	%define vec0xff_epi16           vec0xff_epi32 + 32
	%define vecSfxTimes32           vec0xff_epi16 + 32
	%define vecSFX0                 vecSfxTimes32 + 32
	%define vecSFX1                 vecSFX0 + 32
	%define vecSFX2                 vecSFX1 + 32
	%define vecSFX3                 vecSFX2 + 32

	%define vecNeighb0              ymm4
	%define vecNeighb1              ymm5
	%define vecNeighb2              ymm6
	%define vecNeighb3              ymm7
	%define vecDeinterleave         sym(kShuffleEpi8_Deinterleave8uL2_32s)

	%define arg_inPtr               arg(0)
	%define arg_inStride            arg(1)
	%define arg_outPtr              arg(2)
	%define arg_outWidth            arg(3)
	%define arg_outYStart           arg(4)
	%define arg_outYEnd             arg(5)
	%define arg_outStride           arg(6)
	%define arg_sf_x                arg(7)
	%define arg_sf_y                arg(8)

	; compute vecZero, vec0xff_epi32 and vec0xff_epi16
	vpxor ymm0, ymm0
	vpcmpeqw ymm2, ymm2
	vpsrld ymm1, ymm2, 24
	vpsrlw ymm2, ymm2, 8
	vmovdqa [vecZero], ymm0
	vmovdqa [vec0xff_epi32], ymm1
	vmovdqa [vec0xff_epi16], ymm2

	; compute vecSfxTimes32
	mov rax, arg_sf_x
	shl rax, 5
	vmovd xmm0, eax
	vpbroadcastd ymm0, xmm0
	vmovdqa [vecSfxTimes32], ymm0

	; compute vecSFX0, vecSFX1, vecSFX2 and vecSFX3
	mov rsi, arg_sf_x ; sf_x_
	xor rax, rax ; sf_x_ * 0
	lea rbx, [rsi * 2] ; sf_x_ * 2
	lea rcx, [rbx + rsi] ; sf_x_ * 3
	mov [vecSFX0 + 0], dword eax ; sf_x_ * 0
	mov [vecSFX0 + 4], dword esi ; sf_x_ * 1
	mov [vecSFX0 + 8], dword ebx ; sf_x_ * 2
	mov [vecSFX0 + 12], dword ecx ; sf_x_ * 3
	lea rax, [rcx + rsi] ; sf_x_ * 4
	lea rbx, [rax + rsi] ; sf_x_ * 5
	lea rcx, [rbx + rsi] ; sf_x_ * 6
	lea rdx, [rcx + rsi] ; sf_x_ * 7
	mov [vecSFX0 + 16], dword eax ; sf_x_ * 4
	mov [vecSFX0 + 20], dword ebx ; sf_x_ * 5
	mov [vecSFX0 + 24], dword ecx ; sf_x_ * 6
	mov [vecSFX0 + 28], dword edx ; sf_x_ * 7
	lea rdx, [rdx + rsi] ; sf_x_ * 8
	vmovdqa ymm0, [vecSFX0] ; ymm0 = vecSFX0
	vmovd xmm1, edx
	vpbroadcastd ymm1, xmm1 ; ymm1 = vecSfxTimes8
	%assign sfxIndex 1
	%rep 3
		vpaddd ymm0, ymm1 ; ymm0 = vecSFXn = (vecSFX0 + vecSfxTimes8)
		vmovdqa [vecSFX %+ sfxIndex], ymm0
		%assign sfxIndex sfxIndex+1
	%endrep	

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; do
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.DoWhile:
		mov rax, arg_outYStart ; rax = outYStart
		mov rbx, arg_inPtr ; rbx = inPtr
		vmovd xmm0, eax
		vpbroadcastd ymm0, xmm0 ; ymm0 = vecYStart
		shr rax, 8 ; rax = (outYStart >> 8) = nearestY 
		imul rax, arg_inStride ; rax = (nearestY * inStride)
		lea rbx, [rbx + rax] ; rbx = inPtr_
		vmovdqa ymm1, [vec0xff_epi32]
		vpand ymm0, ymm0, ymm1 ; ymm0 = vecy0
		vpsubd ymm1, ymm1, ymm0 ; ymm1 = vecy1
		vpackssdw ymm0, ymm0, ymm0
		vpackssdw ymm1, ymm1, ymm1
		vmovdqa [vecy0], ymm0
		vmovdqa [vecy1], ymm1
		vmovdqa ymm0, [vecSFX0]
		vmovdqa ymm1, [vecSFX1]
		vmovdqa ymm2, [vecSFX2]
		vmovdqa ymm3, [vecSFX3]
		vmovdqa [vecX0], ymm0
		vmovdqa [vecX1], ymm1
		vmovdqa [vecX2], ymm2
		vmovdqa [vecX3], ymm3

		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; for (i = 0; i < outWidth; i += 32)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		xor rsi, rsi ; rsi = i = 0x0
		.LoopWidth:
			;;; nearest x-point ;;;
			vmovdqa ymm0, [vecX0]
			vmovdqa ymm1, [vecX1]
			vmovdqa ymm2, [vecX2]
			vmovdqa ymm3, [vecX3]
			vpsrld ymm0, ymm0, 8
			vpsrld ymm1, ymm1, 8
			vpsrld ymm2, ymm2, 8
			vpsrld ymm3, ymm3, 8

			;;; write memNeighbs ;;;
			_mm_bilinear_set_neighbs_x86_avx2 xmm0, memNeighb0, memNeighb2, 0, 1, rbx ; overrides rdx, rdi, rax and rcx
			vextractf128 xmm0, ymm0, 1
			_mm_bilinear_set_neighbs_x86_avx2 xmm0, memNeighb0, memNeighb2, 2, 3, rbx ; overrides rdx, rdi, rax and rcx
			_mm_bilinear_set_neighbs_x86_avx2 xmm1, memNeighb0, memNeighb2, 4, 5, rbx ; overrides rdx, rdi, rax and rcx
			vextractf128 xmm1, ymm1, 1
			_mm_bilinear_set_neighbs_x86_avx2 xmm1, memNeighb0, memNeighb2, 6, 7, rbx ; overrides rdx, rdi, rax and rcx
			_mm_bilinear_set_neighbs_x86_avx2 xmm2, memNeighb1, memNeighb3, 0, 1, rbx ; overrides rdx, rdi, rax and rcx
			vextractf128 xmm2, ymm2, 1
			_mm_bilinear_set_neighbs_x86_avx2 xmm2, memNeighb1, memNeighb3, 2, 3, rbx ; overrides rdx, rdi, rax and rcx
			_mm_bilinear_set_neighbs_x86_avx2 xmm3, memNeighb1, memNeighb3, 4, 5, rbx ; overrides rdx, rdi, rax and rcx
			vextractf128 xmm3, ymm3, 1
			_mm_bilinear_set_neighbs_x86_avx2 xmm3, memNeighb1, memNeighb3, 6, 7, rbx ; overrides rdx, rdi, rax and rcx

			;;; read memNeighbs ;;;
			vmovdqa vecNeighb0, [memNeighb0]
			vmovdqa vecNeighb1, [memNeighb1]
			vmovdqa vecNeighb2, [memNeighb2]
			vmovdqa vecNeighb3, [memNeighb3]

			;;; Deinterleave neighbs ;;;
			vpshufb vecNeighb0, [vecDeinterleave] ; 0,0,0,0,1,1,1,1
			vpshufb vecNeighb1, [vecDeinterleave] ; 0,0,0,0,1,1,1,1
			vpshufb vecNeighb2, [vecDeinterleave] ; 2,2,2,2,3,3,3,3
			vpshufb vecNeighb3, [vecDeinterleave] ; 2,2,2,2,3,3,3,3
			vpunpckhqdq ymm0, vecNeighb0, vecNeighb1              ; 1,1,1,1,1,1
			vpunpckhqdq ymm2, vecNeighb2, vecNeighb3              ; 3,3,3,3,3,3
			vpunpcklqdq vecNeighb0, vecNeighb0, vecNeighb1        ; 0,0,0,0,0,0
			vpunpcklqdq vecNeighb2, vecNeighb2, vecNeighb3        ; 2,2,2,2,2,2
			vmovdqa vecNeighb1, ymm0
			vmovdqa vecNeighb3, ymm2

			; compute x0 and x1 (first 8) and convert from epi32 and epi16
			vmovdqa ymm0, [vecX0]
			vmovdqa ymm3, [vecX1]
			vpand ymm0, ymm0, [vec0xff_epi32]
			vpand ymm3, ymm3, [vec0xff_epi32]
			vpackusdw ymm0, ymm0, ymm3
			vpermq ymm0, ymm0, 0xD8 ; ymm0 = vec0
			vpandn ymm1, ymm0, [vec0xff_epi16] ; ymm1 = vec1
			; compute vec4 = (neighb0 * x1) + (neighb1 * x0) -> 8 epi16
			vpunpcklbw ymm2, vecNeighb0, [vecZero]
			vpunpcklbw ymm3, vecNeighb1, [vecZero]
			vpmullw ymm2, ymm2, ymm1
			vpmullw ymm3, ymm3, ymm0
			vpaddusw ymm2, ymm2, ymm3
			vmovdqa [vec4], ymm2
			; compute vec5 = (neighb2 * x1) + (neighb3 * x0) -> 8 epi16
			vpunpcklbw ymm2, vecNeighb2, [vecZero]
			vpunpcklbw ymm3, vecNeighb3, [vecZero]
			vpmullw ymm2, ymm2, ymm1
			vpmullw ymm3, ymm3, ymm0
			vpaddusw ymm2, ymm2, ymm3
			vmovdqa [vec5], ymm2

			; compute x0 and x1 (second 8) and convert from epi32 and epi16
			vmovdqa ymm0, [vecX2]
			vmovdqa ymm3, [vecX3]
			vpand ymm0, ymm0, [vec0xff_epi32]
			vpand ymm3, ymm3, [vec0xff_epi32]
			vpackusdw ymm0, ymm0, ymm3
			vpermq ymm0, ymm0, 0xD8 ; ymm0 = vec0
			vpandn ymm1, ymm0, [vec0xff_epi16] ; ymm1 = vec1
			; compute vec6 = (neighb0 * x1) + (neighb1 * x0) -> 8 epi16
			vpunpckhbw ymm2, vecNeighb0, [vecZero]
			vpunpckhbw ymm3, vecNeighb1, [vecZero]
			vpmullw ymm2, ymm2, ymm1
			vpmullw ymm3, ymm3, ymm0
			vpaddusw ymm2, ymm2, ymm3
			vmovdqa [vec6], ymm2
			; compute vec7 = (neighb2 * x1) + (neighb3 * x0) -> #8 epi16
			vpunpckhbw ymm2, vecNeighb2, [vecZero]
			vpunpckhbw ymm3, vecNeighb3, [vecZero]
			vpmullw ymm2, ymm2, ymm1
			vpmullw ymm3, ymm3, ymm0
			vpaddusw ymm2, ymm2, ymm3
			vmovdqa [vec7], ymm2

			; Let''s say:
			;		A = ((neighb0 * x1) + (neighb1 * x0))
			;		B = ((neighb2 * x1) + (neighb3 * x0))
			; Then:
			;		A = vec4, vec6
			;		B = vec5, vec7
			;
			; We cannot use _mm_madd_epi16 to compute C and D because it operates on epi16 while A and B contain epu16 values

			vmovdqa ymm0, [vecy1]  ; ymm0 = vecy1
			vmovdqa ymm2, [vecy0]  ; ymm2 = vecy0
			vmovdqa ymm1, ymm0     ; ymm1 = vecy1
			vmovdqa ymm3, ymm2     ; ymm3 = vecy0

			; compute C = (y1 * A) >> 16
			vpmulhuw ymm0, ymm0, [vec4]
			vpmulhuw ymm1, ymm1, [vec6]

			; compute D = (y0 * B) >> 16
			vpmulhuw ymm2, ymm2, [vec5]
			vpmulhuw ymm3, ymm3, [vec7]

			; Compute R = (C + D)
			vpaddusw ymm0, ymm0, ymm2
			vpaddusw ymm1, ymm1, ymm3

			; Store the result
			mov rax, arg_outPtr
			vpackuswb ymm0, ymm0, ymm1
			vpermq ymm0, ymm0, 0xD8
			vmovdqa [rax + rsi], ymm0

			; move to next indices
			vmovdqa ymm0, [vecX0]
			vmovdqa ymm1, [vecX1]
			vmovdqa ymm2, [vecX2]
			vmovdqa ymm3, [vecX3]
			vpaddd ymm0, ymm0, [vecSfxTimes32]
			vpaddd ymm1, ymm1, [vecSfxTimes32]
			vpaddd ymm2, ymm2, [vecSfxTimes32]
			vpaddd ymm3, ymm3, [vecSfxTimes32]
			vmovdqa [vecX0], ymm0
			vmovdqa [vecX1], ymm1
			vmovdqa [vecX2], ymm2
			vmovdqa [vecX3], ymm3
			
			;;
			lea rsi, [rsi + 32]
			cmp rsi, arg_outWidth
			jl .LoopWidth
			; end-of-LoopWidth

		;;
		mov rcx, arg_outPtr
		mov rax, arg_outYStart
		add rcx, arg_outStride
		add rax, arg_sf_y
		mov arg_outPtr, rcx		
		mov arg_outYStart, rax
		cmp rax, arg_outYEnd
		jl .DoWhile
		; end-of-DoWhile

	%undef memNeighb0
	%undef memNeighb1
	%undef memNeighb2
	%undef memNeighb3
	%undef vecX0
	%undef vecX1
	%undef vecX2
	%undef vecX3
	%undef vec4
	%undef vec5
	%undef vec6
	%undef vec7
	%undef vecy0
	%undef vecy1
	%undef vecZero
	%undef vec0xff_epi32
	%undef vec0xff_epi16
	%undef vecSfxTimes32
	%undef vecSFX0
	%undef vecSFX1
	%undef vecSFX2
	%undef vecSFX3

	%undef vecNeighb0
	%undef vecNeighb1
	%undef vecNeighb2
	%undef vecNeighb3
	%undef vecDeinterleave

	%undef arg_inPtr
	%undef arg_inStride
	%undef arg_outPtr
	%undef arg_outWidth
	%undef arg_outYStart
	%undef arg_outYEnd
	%undef arg_outStride
	%undef arg_sf_x
	%undef arg_sf_y

	; free memory and unalign stack
	add rsp, (22*32)
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
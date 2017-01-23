;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "compv_common_x86.s"

%if COMPV_YASM_ABI_IS_64BIT

COMPV_YASM_DEFAULT_REL

global sym(Convlt1_verthz_float32_minpack16_Asm_X64_AVX2)
global sym(Convlt1_verthz_float32_minpack16_Asm_X64_FMA3_AVX2)
global sym(Convlt1_verthz_fxpq16_minpack16_Asm_X64_AVX2)

section .data
	extern sym(kAVXMaskstore_0_1_u64)

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; This function requires sizeof(float) = 4byte = 32bits
; arg(0) -> const uint8_t* in_ptr
; arg(1) -> uint8_t* out_ptr
; arg(2) -> compv_scalar_t width
; arg(3) -> compv_scalar_t height
; arg(4) -> compv_scalar_t stride
; arg(5) -> compv_scalar_t pad
; arg(6) -> const float* hkern_ptr
; arg(7) -> compv_scalar_t kern_size
; %1 -> 1: FMA3 enabled, 0: FMA3 disabled
; void Convlt1_verthz_float32_minpack16_Asm_X64_AVX2(const uint8_t* in_ptr, uint8_t* out_ptr, compv_scalar_t width, compv_scalar_t height, compv_scalar_t stride, compv_scalar_t pad, const float* hkern_ptr, compv_scalar_t kern_size)
%macro Convlt1_verthz_float32_minpack16_Macro_X64_AVX2 1
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	COMPV_YASM_SAVE_YMM 10 ;YMM[6-10]
	push rsi
	push rdi
	push rbx
	push r12
	;; end prolog ;;

	%define COMPV_SIZE_OF_FLOAT 4 ; up to the caller to make sure sizeof(float)=4

	; i = rdi
	; xor rdi, rdi

	; rcx = col

	; rbx = out_ptr
	mov rbx, arg(1)

	; j = rsi = height
	mov rsi, arg(3)

	; ymm7 = ymmZero
	vpxor ymm7, ymm7

	; ymm10 = ymmMaskToExtractFirst128Bits
	vmovdqa ymm10, [sym(kAVXMaskstore_0_1_u64)]

	; r9 = (pad += (width & 15))
	mov rdx, arg(2) ; width
	mov r9, arg(5) ; pad
	and rdx, 15
	add r9, rdx

	; rax = in_ptr
	mov rax, arg(0)

	; rdx = hkern_ptr
	mov rdx, arg(6)

	; r8 = kern_size
	mov r8, arg(7)

	; r10 = width
	mov r10, arg(2)

	; r11 = stride
	mov r11, arg(4)

	; r12 reserved
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopRows
		mov rdi, r10 ; i = width
		; jmp .EndOfLoopColumns32 ; uncomment this line to skip Loop-32
		cmp r10, 32
		jl .EndOfLoopColumns32
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; while (i > 31)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopColumns32
			mov r12, rax ; in_ptr
			xor rcx, rcx ; col = 0
			vxorps xmm5, xmm5, xmm5 ; xmm5 = xmmSF0 = xmmZero
			vxorps xmm6, xmm6, xmm6 ; xmm6 = xmmSF1 = xmmZero
			vxorps xmm4, xmm4, xmm4 ; xmm4 = xmmSF2 = xmmZero
			vxorps xmm9, xmm9, xmm9 ; xmm9 = xmmSF3 = xmmZero
			
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (col = 0; col < kern_size; ++col)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			.LoopColumns32Kern
				vmovdqu ymm0, [r12] ; ymm0 = ymmI0
				vmovss xmm1, [rdx + rcx*COMPV_SIZE_OF_FLOAT]
				
				vpunpcklbw ymm2, ymm0, ymm7
				vpunpcklbw ymm3, ymm0, ymm7
				vpunpckhbw ymm8, ymm0, ymm7
				vpunpckhbw ymm0, ymm0, ymm7
				vpunpcklwd ymm2, ymm2, ymm7
				vpunpckhwd ymm3, ymm3, ymm7
				vpunpcklwd ymm8, ymm8, ymm7
				vpunpckhwd ymm0, ymm0, ymm7

				vbroadcastss ymm1, xmm1 ; ymm1 = ymmCoeff

				vcvtdq2ps ymm2, ymm2
				vcvtdq2ps ymm3, ymm3
				vcvtdq2ps ymm8, ymm8
				vcvtdq2ps ymm0, ymm0
				%if %1 == 1 ; FMA3
					vfmadd231ps ymm5, ymm2, ymm1
					vfmadd231ps ymm6, ymm3, ymm1
					vfmadd231ps ymm4, ymm8, ymm1
					vfmadd231ps ymm9, ymm0, ymm1
				%else
					vmulps ymm2, ymm2, ymm1
					vmulps ymm3, ymm3, ymm1
					vmulps ymm8, ymm8, ymm1
					vmulps ymm0, ymm0, ymm1
					vaddps ymm5, ymm5, ymm2
					vaddps ymm6, ymm6, ymm3
					vaddps ymm4, ymm4, ymm8
					vaddps ymm9, ymm9, ymm0
				%endif

				inc rcx
				add r12, r11 ; += stride
				cmp rcx, r8 ; ==? kern_size
				jl .LoopColumns32Kern		

			vcvtps2dq ymm5, ymm5
			vcvtps2dq ymm6, ymm6
			vcvtps2dq ymm4, ymm4
			vcvtps2dq ymm9, ymm9
			vpackssdw ymm5, ymm5, ymm6
			vpackssdw ymm4, ymm4, ymm9
			vpackuswb ymm5, ymm5, ymm4
			lea rax, [rax + 32] ; in_ptr += 32
			vmovdqu [rbx], ymm5
			lea rbx, [rbx + 32] ; out_ptr += 32

			sub rdi, 32 ; i -= 32
			cmp rdi, 32
			jge .LoopColumns32
			.EndOfLoopColumns32

		cmp rdi, 16
		jl .EndOfLoopColumns16
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; while (i > 15)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopColumns16
			vxorps ymm4, ymm4 ; ymm4 = ymmSF0
			vxorps ymm5, ymm5 ; ymm5 = ymmSF1

			mov r12, rax ; in_ptr
			xor rcx, rcx ; col = 0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (col = 0; col < kern_size; ++col)
			; When width is mof 32 this code isn't executed, make sure to disable previous "while" if you change something
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			.LoopColumns16Kern
				vpmaskmovq ymm0, ymm10, [r12] ; ymm0 = ymmI0
				vmovss xmm1, [rdx + rcx*COMPV_SIZE_OF_FLOAT]
				vpermq ymm0, ymm0, 0xD8
				vbroadcastss ymm1, xmm1 ; ymm1 = ymmCoeff				
				vpunpcklbw ymm0, ymm0, ymm7 ; ymm0 = ymmI1

				vpunpcklwd ymm3, ymm0, ymm7
				vpunpckhwd ymm0, ymm0, ymm7
				vcvtdq2ps ymm3, ymm3
				vcvtdq2ps ymm0, ymm0
				%if %1 == 1 ; FMA3
					vfmadd231ps ymm4, ymm3, ymm1
					vfmadd231ps ymm5, ymm0, ymm1
				%else
					vmulps ymm3, ymm1
					vmulps ymm0, ymm1
					vaddps ymm4, ymm3				
					vaddps ymm5, ymm0
				%endif

				inc rcx
				add r12, r11 ; += stride
				cmp rcx, r8 ; ==? kern_size
				jl .LoopColumns16Kern

			vcvtps2dq ymm4, ymm4
			vcvtps2dq ymm5, ymm5
			vpackssdw ymm4, ymm5
			vpackuswb ymm4, ymm4
			vpermq ymm4, ymm4, 0xD8
			vpmaskmovq [rbx], ymm10, ymm4

			lea rbx, [rbx + 16] ; out_ptr += 16
			lea rax, [rax + 16] ; in_ptr += 16

			sub rdi, 16 ; i -= 16
			cmp rdi, 16
			jge .LoopColumns16
			.EndOfLoopColumns16
		
		lea rbx, [rbx + r9] ; out_ptr += pad
		lea rax, [rax + r9] ; in_ptr += pad

		dec rsi ; --j
		test rsi, rsi
		jnz .LoopRows

	%undef COMPV_SIZE_OF_FLOAT

	;; begin epilog ;;
	pop r12
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
sym(Convlt1_verthz_float32_minpack16_Asm_X64_AVX2):
	Convlt1_verthz_float32_minpack16_Macro_X64_AVX2 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
sym(Convlt1_verthz_float32_minpack16_Asm_X64_FMA3_AVX2):
	Convlt1_verthz_float32_minpack16_Macro_X64_AVX2 1



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; This function requires sizeof(float) = 4byte = 32bits
; arg(0) -> const uint8_t* in_ptr
; arg(1) -> uint8_t* out_ptr
; arg(2) -> compv_scalar_t width
; arg(3) -> compv_scalar_t height
; arg(4) -> compv_scalar_t stride
; arg(5) -> compv_scalar_t pad
; arg(6) -> const uint16_t* hkern_ptr
; arg(7) -> compv_scalar_t kern_size
; void Convlt1_verthz_fxpq16_minpack16_Asm_X64_AVX2(const uint8_t* in_ptr, uint8_t* out_ptr, compv_scalar_t width, compv_scalar_t height, compv_scalar_t stride, compv_scalar_t pad, const uint16_t* hkern_ptr, compv_scalar_t kern_size)
sym(Convlt1_verthz_fxpq16_minpack16_Asm_X64_AVX2):
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 7
	push rsi
	push rdi
	push rbx
	push r12
	;; end prolog ;;

	vzeroupper

	%define COMPV_SIZE_OF_INT16 2 ; up to the caller to make sure sizeof(float)=4

	; i = rdi
	; xor rdi, rdi

	; rcx = col

	; rbx = out_ptr
	mov rbx, arg(1)

	; j = rsi = height
	mov rsi, arg(3)

	; ymm6 = ymmZero
	vpxor ymm6, ymm6

	; ymm3 = ymmMaskToExtractFirst128Bits
	vmovdqa ymm3, [sym(kAVXMaskstore_0_1_u64)]

	; r9 = (pad += (width & 15))
	mov rdx, arg(2) ; width
	mov r9, arg(5) ; pad
	and rdx, 15
	add r9, rdx

	; rax = in_ptr
	mov rax, arg(0)

	; rdx = hkern_ptr
	mov rdx, arg(6)

	; r8 = kern_size
	mov r8, arg(7)

	; r10 = width
	mov r10, arg(2)

	; r11 = stride
	mov r11, arg(4)

	; r12 reserved
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; for (j = 0; j < height; ++j)
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.LoopRows
		mov rdi, r10 ; i = width
		; jmp .EndOfLoopColumns32 ; uncomment this line to skip Loop-32
		cmp r10, 32
		jl .EndOfLoopColumns32
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; while (i > 31)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopColumns32
			mov r12, rax ; in_ptr
			xor rcx, rcx ; col = 0
			vpxor ymm4, ymm4 ; ymm4 = ymmS0
			vpxor ymm5, ymm5 ; ymm5 = ymmS1
			
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (col = 0; col < kern_size; ++col)
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			.LoopColumns32Kern
				vmovd xmm1, [rdx + rcx*COMPV_SIZE_OF_INT16]
				vmovdqu ymm0, [r12] ; ymm0 = ymmI0		
				vpbroadcastw ymm1, xmm1 ; ymm1 = ymmCoeff
				vpunpckhbw ymm2, ymm0, ymm6
				vpunpcklbw ymm0, ymm0, ymm6
				vpmulhuw ymm2, ymm1
				vpmulhuw ymm0, ymm1
				vpaddw ymm5, ymm2
				vpaddw ymm4, ymm0

				inc rcx
				add r12, r11 ; += stride
				cmp rcx, r8 ; ==? kern_size
				jl .LoopColumns32Kern		

			vpackuswb ymm4, ymm5
			lea rax, [rax + 32] ; in_ptr += 32
			vmovdqu [rbx], ymm4
			lea rbx, [rbx + 32] ; out_ptr += 32

			sub rdi, 32 ; i -= 32
			cmp rdi, 32
			jge .LoopColumns32
			.EndOfLoopColumns32

		cmp rdi, 16
		jl .EndOfLoopColumns16
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; while (i > 15)
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		.LoopColumns16
			vpxor ymm4, ymm4 ; ymm4 = ymmS0

			mov r12, rax ; in_ptr
			xor rcx, rcx ; col = 0
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			; for (col = 0; col < kern_size; ++col)
			; When width is mof 32 this code isn't executed, make sure to disable previous "while" if you change something
			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
			.LoopColumns16Kern
				vmovd xmm1, [rdx + rcx*COMPV_SIZE_OF_INT16]
				vpmaskmovq ymm0, ymm3, [r12] ; ymm0 = ymmI0
				vpbroadcastw ymm1, xmm1 ; ymm1 = ymmCoeff

				vpermq ymm0, ymm0, 0xD8
				vpunpcklbw ymm0, ymm0, ymm6 
				vpmulhuw ymm0, ymm0, ymm1
				vpaddw ymm4, ymm4, ymm0

				inc rcx
				add r12, r11 ; += stride
				cmp rcx, r8 ; ==? kern_size
				jl .LoopColumns16Kern

			vpackuswb ymm4, ymm4, ymm4
			vpermq ymm4, ymm4, 0xD8
			vpmaskmovq [rbx], ymm3, ymm4

			lea rbx, [rbx + 16] ; out_ptr += 16
			lea rax, [rax + 16] ; in_ptr += 16

			sub rdi, 16 ; i -= 16
			cmp rdi, 16
			jge .LoopColumns16
			.EndOfLoopColumns16
		
		lea rbx, [rbx + r9] ; out_ptr += pad
		lea rax, [rax + r9] ; in_ptr += pad

		dec rsi ; --j
		test rsi, rsi
		jnz .LoopRows

	%undef COMPV_SIZE_OF_INT16

	;; begin epilog ;;
	pop r12
	pop rbx
	pop rdi
	pop rsi
	COMPV_YASM_UNSHADOW_ARGS
	mov rsp, rbp
	pop rbp
	vzeroupper
	ret

%endif ; COMPV_YASM_ABI_IS_64BIT
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "../../compv_common_x86.s"

COMPV_YASM_DEFAULT_REL

global sym(rgbToRgbaKernel31_Asm_X86_Aligned00_AVX2)
global sym(rgbToRgbaKernel31_Asm_X86_Aligned01_AVX2)
global sym(rgbToRgbaKernel31_Asm_X86_Aligned10_AVX2)
global sym(rgbToRgbaKernel31_Asm_X86_Aligned11_AVX2)

section .data
	extern sym(k_0_0_0_255_u8)
	extern sym(kAVXPermutevar8x32_ABCDDEFG_i32)
	extern sym(kAVXPermutevar8x32_CDEFFGHX_i32)
	extern sym(kAVXPermutevar8x32_XXABBCDE_i32)
	extern sym(kShuffleEpi8_RgbToRgba_i32)

section .text


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arg(0) -> const uint8_t* rgb, 
; arg(1) -> uint8_t* rgba
; arg(2) -> compv_scalar_t height
; arg(3) -> compv_scalar_t width
; arg(4) -> compv_scalar_t stride
; %1 -> 1: rgb aligned, 0: rgb not aligned
; %2 -> 1: rgba aligned, 0: rgba not aligned
%macro rgbToRgbaKernel31_Asm_X86_AVX2 2
	vzeroupper
	push rbp
	mov rbp, rsp
	COMPV_YASM_SHADOW_ARGS_TO_STACK 5
	COMPV_YASM_SAVE_YMM 7 ;XMM[6-n]
	push rsi
	push rdi
	push rbx
	; end prolog

	; align stack and alloc memory
	COMPV_YASM_ALIGN_STACK 32, rax
	sub rsp, 32

	mov rax, arg(3)
	add rax, 31
	and rax, -32
	neg rax
	add rax, arg(4)
	mov rdx, rax
	imul rdx, 3
	mov rsi, rdx ; padRGB
	shl rax, 2
	mov rbx, rax ; padRAGB

	mov rax, arg(0) ; rgb
	mov rdx, arg(1) ; rgba
	mov rcx, arg(2) ; height

	vmovdqa ymm4, [sym(kAVXPermutevar8x32_CDEFFGHX_i32)]
	vmovdqa ymm5, [sym(kAVXPermutevar8x32_XXABBCDE_i32)] ; ymmXXABBCDE
	vmovdqa ymm6, [sym(kShuffleEpi8_RgbToRgba_i32)] ; ymmMaskRgbToRgba
	vmovdqa ymm7, [sym(kAVXPermutevar8x32_ABCDDEFG_i32)] ; ymmABCDDEFG

	.LoopHeight:
		xor rdi, rdi
		.LoopWidth:
			;;;;;;;;;; Line-0 ;;;;;;;;;;
			%if %1==1
			vmovdqa ymm0, [rax + 0] ; load first 32 samples
			%else
			vmovdqu ymm0, [rax + 0] ; load first 32 samples
			%endif
			vpermd ymm1, ymm7, ymm0
			vpshufb ymm1, ymm1, ymm6
			vpaddb ymm1, ymm1, [sym(k_0_0_0_255_u8)]
			%if %2==1
			vmovdqa [rdx + 0], ymm1
			%else
			vmovdqu [rdx + 0], ymm1
			%endif

			;;;;;;;;;; Line-1 ;;;;;;;;;;
			%if %1==1
			vmovdqa ymm1, [rax + 32] ; load next 32 samples
			%else
			vmovdqu ymm1, [rax + 32] ; load next 32 samples
			%endif
			vpermq ymm0, ymm0, 0xff ; duplicate lost0
			vextractf128 [rsp + 0], ymm1, 0x1
			vbroadcasti128 ymm3, [rsp + 0] ; ymmLost = ymm3 = high-128 = low-lost = lost0 || lost1
			vpermd ymm1, ymm5, ymm1
			vpblendd ymm1, ymm1, ymm0, 0x03 ; ymm0(64bits)||ymm1(192bits)
			vpshufb ymm1, ymm1, ymm6
			vpaddb ymm1, ymm1, [sym(k_0_0_0_255_u8)]
			%if %2==1
			vmovdqa [rdx + 32], ymm1
			%else
			vmovdqu [rdx + 32], ymm1
			%endif

			;;;;;;;;;; Line-2 ;;;;;;;;;;
			%if %1==1
			vmovdqa ymm0, [rax + 64] ; load next 32 samples
			%else
			vmovdqu ymm0, [rax + 64] ; load next 32 samples
			%endif
			vpermd ymm1, ymm4, ymm0 ; lost0 || lost1 || lost2 || garbage
			vextractf128 [rsp + 0], ymm0, 0x0
			vinserti128 ymm3, ymm3, [rsp + 0], 0x1
			vpermd ymm0, ymm7, ymm3
			vpshufb ymm0, ymm0, ymm6
			vpaddb ymm0, ymm0, [sym(k_0_0_0_255_u8)]
			%if %2==1
			vmovdqa [rdx + 64], ymm0
			%else
			vmovdqu [rdx + 64], ymm0
			%endif

			;;;;;;;;;; Line-3 ;;;;;;;;;;
			vpshufb ymm1, ymm1, ymm6
			vpaddb ymm1, ymm1, [sym(k_0_0_0_255_u8)]
			%if %2==1
			vmovdqa [rdx + 96], ymm1
			%else
			vmovdqu [rdx + 96], ymm1
			%endif
			
			add rax, 96 ; rgb += 96
			add rdx, 128 ; rgba += 128

			; end-of-LoopWidth
			add rdi, 32
			cmp rdi, arg(3)
			jl .LoopWidth
	add rax, rsi ; rgb += padRGB
	add rdx, rbx ; rgba += padRGBA
	; end-of-LoopHeight
	dec rcx
	jnz .LoopHeight

	; unalign stack and alloc memory
	add rsp, 32
	COMPV_YASM_UNALIGN_STACK

	; begin epilog
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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbToRgbaKernel31_Asm_X86_Aligned00_AVX2(const uint8_t* rgb, uint8_t* rgba, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride)
sym(rgbToRgbaKernel31_Asm_X86_Aligned00_AVX2):
	rgbToRgbaKernel31_Asm_X86_AVX2 0, 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbToRgbaKernel31_Asm_X86_Aligned01_AVX2(const uint8_t* rgb, COMPV_ALIGNED(AVX2) uint8_t* rgba, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride)
sym(rgbToRgbaKernel31_Asm_X86_Aligned01_AVX2):
	rgbToRgbaKernel31_Asm_X86_AVX2 0, 1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbToRgbaKernel31_Asm_X86_Aligned10_AVX2(COMPV_ALIGNED(AVX2) const uint8_t* rgb, uint8_t* rgba, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride)
sym(rgbToRgbaKernel31_Asm_X86_Aligned10_AVX2):
	rgbToRgbaKernel31_Asm_X86_AVX2 1, 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; void rgbToRgbaKernel31_Asm_X86_Aligned11_AVX2(COMPV_ALIGNED(AVX2) const uint8_t* rgb, COMPV_ALIGNED(AVX2) uint8_t* rgba, compv_scalar_t height, compv_scalar_t width, compv_scalar_t stride)
sym(rgbToRgbaKernel31_Asm_X86_Aligned11_AVX2):
	rgbToRgbaKernel31_Asm_X86_AVX2 1, 1

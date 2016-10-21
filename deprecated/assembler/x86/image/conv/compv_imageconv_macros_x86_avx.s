;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	;
; File author: Mamadou DIOP (Doubango Telecom, France).					;
; License: GPLv3. For commercial license please contact us.				;
; Source code: https://github.com/DoubangoTelecom/compv					;
; WebSite: http://compv.org												;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Macro used to convert 3x16RGB to 4x16RGBA samples
; This macro will override [ymm4 - ymm7] registers
; The aplha channel will contain garbage instead of 0xff beacuse this macro is used to fetch the 3samples to 4samples
; %1 -> in @rgb, e.g. rax
; %2 -> out @rgba, e.g. rsp
; %3 -> @tmp128, temporary aligned m128 variable
; %4 -> whether %1 is SSE-aligned. 0: not aligned, 1: aligned
; %5 -> whether %2 is SSE-aligned. 0: not aligned, 1: aligned
; example: COMPV_3RGB_TO_4RGBA_AVX2 rax, rsp+0, rsp+0, 1, 1
%macro COMPV_3RGB_TO_4RGBA_AVX2  5
	;;;;;;;;;; Line-0 ;;;;;;;;;;
	vmovdqa ymm4, [sym(kAVXPermutevar8x32_ABCDDEFG_i32)]
	%if %4==1
	vmovdqa ymm7, [%1 + 0] ; load first 32 samples
	%else
	vmovdqu ymm7, [%1 + 0] ; load first 32 samples
	%endif
	vpermd ymm6, ymm4, ymm7
	vpshufb ymm6, ymm6, [sym(kShuffleEpi8_RgbToRgba_i32)]
	vpaddb ymm6, ymm6, [sym(k_0_0_0_255_u8)]
	%if %5==1
	vmovdqa [%2 + 0], ymm6
	%else
	vmovdqu [%2 + 0], ymm6
	%endif

	;;;;;;;;;; Line-1 ;;;;;;;;;;
	vmovdqa ymm4, [sym(kAVXPermutevar8x32_XXABBCDE_i32)]
	%if %4==1
	vmovdqa ymm6, [%1 + 32] ; load next 32 samples
	%else
	vmovdqu ymm6, [%1 + 32] ; load next 32 samples
	%endif
	vpermq ymm7, ymm7, 0xff ; duplicate lost0
	vextractf128 [%3 + 0], ymm6, 0x1
	vbroadcasti128 ymm5, [%3 + 0] ; ymmLost = ymm5 = high-128 = low-lost = lost0 || lost1
	vpermd ymm6, ymm4, ymm6
	vpblendd ymm6, ymm6, ymm7, 0x03 ; ymm7(64bits)||ymm6(192bits)
	vpshufb ymm6, ymm6, [sym(kShuffleEpi8_RgbToRgba_i32)]
	vpaddb ymm6, ymm6, [sym(k_0_0_0_255_u8)]
	%if %5==1
	vmovdqa [%2 + 32], ymm6
	%else
	vmovdqu [%2 + 32], ymm6
	%endif

	;;;;;;;;;; Line-2 ;;;;;;;;;;
	vmovdqa ymm4, [sym(kAVXPermutevar8x32_CDEFFGHX_i32)]
	%if %4==1
	vmovdqa ymm7, [%1 + 64] ; load next 32 samples
	%else
	vmovdqu ymm7, [%1 + 64] ; load next 32 samples
	%endif
	vpermd ymm6, ymm4, ymm7 ; lost0 || lost1 || lost2 || garbage
	vmovdqa ymm4, [sym(kAVXPermutevar8x32_ABCDDEFG_i32)]
	vextractf128 [%3 + 0], ymm7, 0x0
	vinserti128 ymm5, ymm5, [%3 + 0], 0x1
	vpermd ymm7, ymm4, ymm5
	vpshufb ymm7, ymm7, [sym(kShuffleEpi8_RgbToRgba_i32)]
	vpaddb ymm7, ymm7, [sym(k_0_0_0_255_u8)]
	%if %5==1
	vmovdqa [%2 + 64], ymm7
	%else
	vmovdqu [%2 + 64], ymm7
	%endif

	;;;;;;;;;; Line-3 ;;;;;;;;;;
	vpshufb ymm6, ymm6, [sym(kShuffleEpi8_RgbToRgba_i32)]
	vpaddb ymm6, ymm6, [sym(k_0_0_0_255_u8)]
	%if %5==1
	vmovdqa [%2 + 96], ymm6
	%else
	vmovdqu [%2 + 96], ymm6
	%endif
%endmacro
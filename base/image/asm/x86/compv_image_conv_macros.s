;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Macro used to convert 16 RGB to 16 RGBA samples
; 16 RGB samples requires 48 Bytes(3 XMM registers), will be converted to 16 RGBA samples
; requiring 64 Bytes (4 XMM registers)
; The aplha channel will contain zeros instead of 0xff because this macro is used to fetch samples in place
; %1 -> in @rgb, e.g. rax
; %2 -> @rgba[0], e.g. xmm0
; %3 -> @rgba[1], e.g. xmm1
; %4 -> @rgba[2], e.g. xmm2
; %5 -> @rgba[3], e.g. xmm3
; %6 -> @RgbToRgba mask, e.g. xmm4 or [sym(kShuffleEpi8_RgbToRgba_i32)]
; example: COMPV_16xRGB_TO_16xRGBA_SSSE3 rax, xmm0, xmm1, xmm2, xmm3, xmm4
%macro COMPV_16xRGB_TO_16xRGBA_SSSE3  6
	movdqa %2, [%1 + 0]
	movdqu %3, [%1 + 12]
	movdqu %4, [%1 + 24]
	movdqu %5, [%1 + 36]
	pshufb %2, %6
	pshufb %3, %6
	pshufb %4, %6
	pshufb %5, %6	
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Macro used to convert 32 RGB to 32 RGBA samples
; 32 RGB samples requires 96 Bytes(3 YMM registers), will be converted to 32 RGBA samples
; requiring 128 Bytes (4 YMM registers)
; The aplha channel will contain zeros instead of 0xff because this macro is used to fetch samples in place
; %1 -> in @rgb, e.g. rax
; %2 -> @rgba[0], e.g. ymm0
; %3 -> @rgba[1], e.g. ymm1
; %4 -> @rgba[2], e.g. ymm2
; %5 -> @rgba[3], e.g. ymm3
; %6 -> ymmABCDDEFG, e.g. ymm4
; %7 -> ymmMaskRgbToRgba, e.g. ymm5 or [sym(kShuffleEpi8_RgbToRgba_i32)]
; example: COMPV_16xRGB_TO_16xRGBA_SSSE3 rax, ymm0, ymm1, ymm2, ymm3, ymm4, ymm5
%macro COMPV_32xRGB_TO_32xRGBA_AVX2  7
	vmovdqa %2, [%1 + 0]
	vmovdqu %3, [%1 + 24]
	vmovdqu %4, [%1 + 48]
	vmovdqu %5, [%1 + 72]
	vpermd %2, %6, %2
	vpermd %3, %6, %3
	vpermd %4, %6, %4
	vpermd %5, %6, %5
	vpshufb %2, %2, %7
	vpshufb %3, %3, %7
	vpshufb %4, %4, %7
	vpshufb %5, %5, %7
%endmacro

; Interleave "vecLane0", "vecLane1" and "vecLane3" then store into "ptr"
; !!! "vecLane0", "vecLane1" and "vecLane3" ARE modified !!!
; e.g. [RRRR], [GGGG], [BBBB] -> RGBRGBRGB
; Signatue: COMPV_VST3_I8_SSSE3(ptr, vecLane0, vecLane1, vecLane2, vectmp0, vectmp1, vectmp2)
; Example: COMPV_VST3_I8_SSSE3(rax + rcx, xmm0, xmm1, xmm2, xmm3, xmm4, xmm5)
%macro COMPV_VST3_I8_SSSE3 7
	%define ptr			%1
	%define vecLane0	%2
	%define vecLane1	%3
	%define vecLane2	%4
	%define vectmp0		%5
	%define vectmp1		%6
	%define vectmp2		%7
	movdqa vectmp1, vecLane0
	movdqa vectmp2, vecLane0
	punpcklbw vectmp1, vecLane1
	punpckhbw vectmp2, vecLane1
	movdqa vectmp0, vectmp1
	movdqa vecLane0, vecLane2
	pslldq vectmp0, 5
	movdqa vecLane1, vectmp2
	palignr vecLane1, vectmp1, 11
	palignr vecLane0, vectmp0, 5
	movdqa vectmp0, [sym(kShuffleEpi8_InterleaveRGB24_Step0_i32)]
	psrldq vecLane2, 5
	pslldq vecLane1, 5
	movdqa vectmp1, vecLane2
	palignr vectmp1, vecLane1, 5
	pshufb vecLane0, vectmp0
	movdqa vectmp0, [sym(kShuffleEpi8_InterleaveRGB24_Step1_i32)]
	psrldq vecLane2, 5
	palignr vecLane2, vectmp2, 6
	movdqa vecLane1, vectmp1
	pshufb vecLane1, vectmp0
	pshufb vecLane2, [sym(kShuffleEpi8_InterleaveRGB24_Step2_i32)]		
	movdqa [ptr + (0*COMPV_YASM_XMM_SZ_BYTES)], vecLane0
	movdqa [ptr + (1*COMPV_YASM_XMM_SZ_BYTES)], vecLane1
	movdqa [ptr + (2*COMPV_YASM_XMM_SZ_BYTES)], vecLane2
	%undef ptr
	%undef vecLane0
	%undef vecLane1
	%undef vecLane2
	%undef vectmp0
	%undef vectmp1
	%undef vectmp2
%endmacro
%define COMPV_VST3_U8_SSSE3 COMPV_VST3_I8_SSSE3


; Interleave "vecLane0", "vecLane1" and "vecLane3" then store into "ptr"
; !!! "vecLane0", "vecLane1" and "vecLane3" ARE modified !!!
; e.g. [RRRR], [GGGG], [BBBB] -> RGBRGBRGB
; Signatue: COMPV_VST3_I8_SSSE3_VEX(ptr, vecLane0, vecLane1, vecLane2, vectmp0, vectmp1, vectmp2)
; Example: COMPV_VST3_I8_SSSE3_VEX(rax + rcx, xmm0, xmm1, xmm2, xmm3, xmm4, xmm5)
%macro COMPV_VST3_I8_SSSE3_VEX 7
	%define ptr			%1
	%define vecLane0	%2
	%define vecLane1	%3
	%define vecLane2	%4
	%define vectmp0		%5
	%define vectmp1		%6
	%define vectmp2		%7
	vpunpcklbw vectmp1, vecLane0, vecLane1
	vpunpckhbw vectmp2, vecLane0, vecLane1
	vpslldq vectmp0, vectmp1, 5
	vpalignr vecLane1, vectmp2, vectmp1, 11
	vpalignr vecLane0, vecLane2, vectmp0, 5
	vmovdqa vectmp0, [sym(kShuffleEpi8_InterleaveRGB24_Step0_i32)]
	vpsrldq vecLane2, vecLane2, 5
	vpslldq vecLane1, vecLane1, 5
	vpalignr vectmp1, vecLane2, vecLane1, 5
	vpshufb vecLane0, vecLane0, vectmp0
	vmovdqa vectmp0, [sym(kShuffleEpi8_InterleaveRGB24_Step1_i32)]
	vpsrldq vecLane2, vecLane2, 5
	vpalignr vecLane2, vecLane2, vectmp2, 6
	vmovdqa vectmp2, [sym(kShuffleEpi8_InterleaveRGB24_Step2_i32)]	
	vpshufb vecLane1, vectmp1, vectmp0
	vpshufb vecLane2, vecLane2, vectmp2
	vmovdqa [ptr + (0*COMPV_YASM_XMM_SZ_BYTES)], vecLane0
	vmovdqa [ptr + (1*COMPV_YASM_XMM_SZ_BYTES)], vecLane1
	vmovdqa [ptr + (2*COMPV_YASM_XMM_SZ_BYTES)], vecLane2
	%undef ptr
	%undef vecLane0
	%undef vecLane1
	%undef vecLane2
	%undef vectmp0
	%undef vectmp1
	%undef vectmp2
%endmacro
%define COMPV_VST3_U8_SSSE3_VEX COMPV_VST3_I8_SSSE3_VEX

;// De-Interleave "ptr" into  "vecLane0", "vecLane1", "vecLane2" and "vecLane3"
;// e.g. RGBARGBARGBA -> [RRRR], [GGGG], [BBBB], [AAAA]
; Example: COMPV_VLD4_I8_SSSE3(rax + rcx, xmm0, xmm1, xmm2, xmm3, xmm4, xmm5)
%macro COMPV_VLD4_I8_SSSE3 7
	%define ptr			%1
	%define vecLane0	%2
	%define	vecLane1	%3
	%define	vecLane2	%4
	%define vecLane3	%5
	%define vectmp0		%6
	%define vectmp1		%7

	movdqa vecLane0, [ptr + (0*COMPV_YASM_XMM_SZ_BYTES)]
	movdqa vecLane1, [ptr + (1*COMPV_YASM_XMM_SZ_BYTES)]
	movdqa vecLane2, [ptr + (2*COMPV_YASM_XMM_SZ_BYTES)]
	movdqa vecLane3, [ptr + (3*COMPV_YASM_XMM_SZ_BYTES)]

	;; first round ;;
	movdqa vectmp0, vecLane0
	movdqa vectmp1, vecLane0
	punpcklbw vectmp0, vecLane1
	punpckhbw vectmp1, vecLane1
	movdqa vecLane0, vectmp0
	movdqa vecLane1, vectmp0
	punpcklwd vecLane0, vectmp1
	punpckhwd vecLane1, vectmp1
	movdqa vectmp0, vecLane0
	movdqa vectmp1, vecLane0
	punpckldq vectmp0, vecLane1
	punpckhdq vectmp1, vecLane1

	;; second round ;;
	movdqa vecLane0, vecLane2
	movdqa vecLane1, vecLane2
	punpcklbw vecLane0, vecLane3
	punpckhbw vecLane1, vecLane3
	movdqa vecLane2, vecLane0
	movdqa vecLane3, vecLane0
	punpcklwd vecLane2, vecLane1
	punpckhwd vecLane3, vecLane1
	movdqa vecLane0, vecLane2
	punpckldq vecLane0, vecLane3
	punpckhdq vecLane2, vecLane3

	;; final round ;;
	movdqa vecLane1, vecLane0
	punpckhqdq vecLane1, vectmp0
	punpcklqdq vecLane0, vectmp0
	movdqa vecLane3, vecLane2
	punpckhqdq vecLane3, vectmp1
	punpcklqdq vecLane2, vectmp1

	;; re-order ;;
	pshufb vecLane0, [sym(kShuffleEpi8_DeinterleaveRGBA32_i32)]
	pshufb vecLane1, [sym(kShuffleEpi8_DeinterleaveRGBA32_i32)]
	pshufb vecLane2, [sym(kShuffleEpi8_DeinterleaveRGBA32_i32)]
	pshufb vecLane3, [sym(kShuffleEpi8_DeinterleaveRGBA32_i32)]

	%undef ptr
	%undef vecLane0
	%undef vecLane1
	%undef vecLane2
	%undef vecLane3
	%undef vectmp0
	%undef vectmp1
%endmacro

%define COMPV_VLD4_U8_SSSE3 COMPV_VLD4_I8_SSSE3

;// De-Interleave "ptr" into  "vecLane0", "vecLane1" and "vecLane2"
;// e.g. RGBRGBRGB -> [RRRR], [GGGG], [BBBB]
;//!\\ You should not need to use this function -> FASTER: convert to RGBX then process (more info: see RGB24 -> YUV)
; Example: COMPV_VLD3_I8_SSSE3(rax + rcx, xmm0, xmm1, xmm2, xmm3, xmm4, xmm5)
%macro COMPV_VLD3_I8_SSSE3 7
	%define ptr			%1
	%define vecLane0	%2
	%define	vecLane1	%3
	%define	vecLane2	%4
	%define vectmp0		%5
	%define vectmp1		%6
	%define vectmp2		%7
	movdqa vecLane0, [ptr + (0*COMPV_YASM_XMM_SZ_BYTES)]
	movdqa vecLane1, [ptr + (1*COMPV_YASM_XMM_SZ_BYTES)]
	movdqa vecLane2, [ptr + (2*COMPV_YASM_XMM_SZ_BYTES)]
	movdqa vectmp2, [sym(kShuffleEpi8_DeinterleaveRGB24_i32)]
	movdqa vectmp0, vecLane0
	movdqa vectmp1, vecLane1
	pshufb vecLane2, vectmp2
	pshufb vectmp0, vectmp2
	pshufb vectmp1, vectmp2
	movdqa vecLane0, vecLane2
	movdqa vecLane1, vecLane2
	movdqa vectmp2, vectmp1
	psrldq vecLane0, 6
	pslldq vectmp2, 10
	psrldq vecLane1, 11
	palignr vecLane0, vectmp1, 11
	psrldq vectmp1, 6
	palignr vecLane1, vectmp2, 10
	palignr vectmp1, vectmp0, 11
	pslldq vectmp0, 5
	palignr vecLane1, vectmp0, 11
	pslldq vectmp0, 5
	palignr vecLane0, vectmp0, 10
	pslldq vectmp1, 6
	palignr vecLane2, vectmp1, 6
	%undef ptr
	%undef vecLane0
	%undef	vecLane1
	%undef	vecLane2
	%undef vectmp0
	%undef vectmp1
	%undef vectmp2
%endmacro
%define COMPV_VLD3_U8_SSSE3 COMPV_VLD3_I8_SSSE3

;// De-Interleave "ptr" into  "vecLane0", "vecLane1" and "vecLane2"
;// e.g. RGBRGBRGB -> [RRRR], [GGGG], [BBBB]
;//!\\ You should not need to use this function -> FASTER: convert to RGBX then process (more info: see RGB24 -> YUV)
; Example: COMPV_VLD3_I8_SSSE3_VEX(rax + rcx, xmm0, xmm1, xmm2, xmm3, xmm4, xmm5)
%macro COMPV_VLD3_I8_SSSE3_VEX 7
	%define ptr			%1
	%define vecLane0	%2
	%define	vecLane1	%3
	%define	vecLane2	%4
	%define vectmp0		%5
	%define vectmp1		%6
	%define vectmp2		%7
	vmovdqa vectmp2, [sym(kShuffleEpi8_DeinterleaveRGB24_i32)]
	vmovdqa vecLane0, [ptr + (0*COMPV_YASM_XMM_SZ_BYTES)]
	vmovdqa vecLane1, [ptr + (1*COMPV_YASM_XMM_SZ_BYTES)]
	vmovdqa vecLane2, [ptr + (2*COMPV_YASM_XMM_SZ_BYTES)]
	vpshufb vecLane2, vectmp2
	vpshufb vectmp0, vecLane0, vectmp2
	vpshufb vectmp1, vecLane1, vectmp2
	vpsrldq vecLane0, vecLane2, 6
	vpslldq vectmp2, vectmp1, 10
	vpsrldq vecLane1, vecLane2, 11
	vpalignr vecLane0, vecLane0, vectmp1, 11
	vpsrldq vectmp1, vectmp1, 6
	vpalignr vecLane1, vecLane1, vectmp2, 10
	vpalignr vectmp1, vectmp1, vectmp0, 11
	vpslldq vectmp0, vectmp0, 5
	vpalignr vecLane1, vecLane1, vectmp0, 11
	vpslldq vectmp0, vectmp0, 5
	vpalignr vecLane0, vecLane0, vectmp0, 10
	vpslldq vectmp1, vectmp1, 6
	vpalignr vecLane2, vecLane2, vectmp1, 6
	%undef ptr
	%undef vecLane0
	%undef	vecLane1
	%undef	vecLane2
	%undef vectmp0
	%undef vectmp1
	%undef vectmp2
%endmacro
%define COMPV_VLD3_U8_SSSE3_VEX COMPV_VLD3_I8_SSSE3_VEX

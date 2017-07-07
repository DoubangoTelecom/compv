;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

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
	movdqa vectmp0, [sym(kShuffleEpi8_InterleaveL3_Step0_i32)]
	psrldq vecLane2, 5
	pslldq vecLane1, 5
	movdqa vectmp1, vecLane2
	palignr vectmp1, vecLane1, 5
	pshufb vecLane0, vectmp0
	movdqa vectmp0, [sym(kShuffleEpi8_InterleaveL3_Step1_i32)]
	psrldq vecLane2, 5
	palignr vecLane2, vectmp2, 6
	movdqa vecLane1, vectmp1
	pshufb vecLane1, vectmp0
	pshufb vecLane2, [sym(kShuffleEpi8_InterleaveL3_Step2_i32)]		
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
	vmovdqa vectmp0, [sym(kShuffleEpi8_InterleaveL3_Step0_i32)]
	vpsrldq vecLane2, vecLane2, 5
	vpslldq vecLane1, vecLane1, 5
	vpalignr vectmp1, vecLane2, vecLane1, 5
	vpshufb vecLane0, vecLane0, vectmp0
	vmovdqa vectmp0, [sym(kShuffleEpi8_InterleaveL3_Step1_i32)]
	vpsrldq vecLane2, vecLane2, 5
	vpalignr vecLane2, vecLane2, vectmp2, 6
	vmovdqa vectmp2, [sym(kShuffleEpi8_InterleaveL3_Step2_i32)]	
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
	movdqa vectmp1, vecLane0
	punpcklbw vecLane0, vecLane1
	punpckhbw vectmp1, vecLane1
	movdqa vecLane1, vecLane0
	punpcklwd vecLane0, vectmp1
	punpckhwd vecLane1, vectmp1
	movdqa vectmp0, vecLane0
	movdqa vectmp1, vecLane0
	movdqa vecLane0, vecLane2
	punpckldq vectmp0, vecLane1
	punpcklbw vecLane0, vecLane3
	punpckhbw vecLane2, vecLane3
	movdqa vecLane3, vecLane0
	punpcklwd vecLane0, vecLane2
	punpckhwd vecLane3, vecLane2
	movdqa vecLane2, vecLane0
	punpckldq vecLane0, vecLane3
	punpckhdq vecLane2, vecLane3
	punpckhdq vectmp1, vecLane1
	movdqa vecLane1, vecLane0
	punpcklqdq vecLane0, vectmp0
	punpckhqdq vecLane1, vectmp0
	vmovdqa vectmp0, [sym(kShuffleEpi8_DeinterleaveL4_i32)]
	movdqa vecLane3, vecLane2
	punpcklqdq vecLane2, vectmp1
	punpckhqdq vecLane3, vectmp1
	pshufb vecLane0, vectmp0
	pshufb vecLane1, vectmp0
	pshufb vecLane2, vectmp0
	pshufb vecLane3, vectmp0
	%undef ptr
	%undef vecLane0
	%undef vecLane1
	%undef vecLane2
	%undef vecLane3
	%undef vectmp0
	%undef vectmp1
%endmacro

%define COMPV_VLD4_U8_SSSE3 COMPV_VLD4_I8_SSSE3


;// De-Interleave "ptr" into  "vecLane0", "vecLane1", "vecLane2" and "vecLane3"
;// e.g. RGBARGBARGBA -> [RRRR], [GGGG], [BBBB], [AAAA]
; Example: COMPV_VLD4_I8_SSSE3_VEX(rax + rcx, xmm0, xmm1, xmm2, xmm3, xmm4, xmm5)
%macro COMPV_VLD4_I8_SSSE3_VEX 7
	%define ptr         %1
    %define vecLane0    %2
    %define vecLane1    %3
    %define vecLane2    %4
    %define vecLane3    %5
    %define vectmp0     %6
    %define vectmp1     %7
    vmovdqa vecLane0, [ptr + (0*COMPV_YASM_XMM_SZ_BYTES)]
    vmovdqa vecLane1, [ptr + (1*COMPV_YASM_XMM_SZ_BYTES)]
    vmovdqa vecLane2, [ptr + (2*COMPV_YASM_XMM_SZ_BYTES)]
    vmovdqa vecLane3, [ptr + (3*COMPV_YASM_XMM_SZ_BYTES)]
    vpunpcklbw vectmp0, vecLane0, vecLane1
    vpunpckhbw vectmp1, vecLane0, vecLane1
    vpunpcklwd vecLane0, vectmp0, vectmp1
    vpunpckhwd vecLane1, vectmp0, vectmp1
    vpunpckldq vectmp0, vecLane0, vecLane1
    vpunpckhdq vectmp1, vecLane0, vecLane1
    vpunpcklbw vecLane0, vecLane2, vecLane3
    vpunpckhbw vecLane1, vecLane2, vecLane3
    vpunpcklwd vecLane2, vecLane0, vecLane1
    vpunpckhwd vecLane3, vecLane0, vecLane1
    vpunpckldq vecLane0, vecLane2, vecLane3
    vpunpckhdq vecLane2, vecLane2, vecLane3
    vpunpckhqdq vecLane1, vecLane0, vectmp0
    vpunpcklqdq vecLane0, vecLane0, vectmp0
	vmovdqa vectmp0, [sym(kShuffleEpi8_DeinterleaveL4_i32)]
    vpunpckhqdq vecLane3, vecLane2, vectmp1
    vpunpcklqdq vecLane2, vecLane2, vectmp1
    vpshufb vecLane0, vectmp0
    vpshufb vecLane1, vectmp0
    vpshufb vecLane2, vectmp0
    vpshufb vecLane3, vectmp0
    %undef ptr
    %undef vecLane0
    %undef vecLane1
    %undef vecLane2
    %undef vecLane3
    %undef vectmp0
    %undef vectmp1 
%endmacro

%define COMPV_VLD4_U8_SSSE3_VEX COMPV_VLD4_I8_SSSE3_VEX

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
	movdqa vectmp2, [sym(kShuffleEpi8_DeinterleaveL3_i32)]
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
	vmovdqa vectmp2, [sym(kShuffleEpi8_DeinterleaveL3_i32)]
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
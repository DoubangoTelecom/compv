;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   ;
; File author: Mamadou DIOP (Doubango Telecom, France).                 ;
; License: GPLv3. For commercial license please contact us.             ;
; Source code: https://github.com/DoubangoTelecom/compv                 ;
; WebSite: http://compv.org                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Macros for FastDataRow
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
%macro _mm_fast_check 2
	mov rax, [circle + (%1*8)]
	mov rsi, [circle + (%2*8)]
	%if COMPV_YASM_ABI_IS_64BIT
		vmovdqu ymm14, [rax + i]
		vmovdqu ymm15, [rsi + i]		
		vpsubusb ymm4, vecDarker1, ymm14
		vpsubusb ymm5, vecDarker1, ymm15
		vpsubusb ymm6, ymm14, vecBrighter1
		vpsubusb ymm7, ymm15, vecBrighter1
		vpcmpeqb ymm4, ymm4, vecZero
		vpcmpeqb ymm5, ymm5, vecZero
		vpcmpeqb ymm6, ymm6, vecZero
		vpcmpeqb ymm7, ymm7, vecZero
		vpandn ymm4, ymm4, vec0xFF
		vpandn ymm5, ymm5, vec0xFF
		vpandn ymm6, ymm6, vec0xFF
		vpandn ymm7, ymm7, vec0xFF
		vpor ymm4, ymm4, ymm5
		vpor ymm6, ymm6, ymm7
		vpor ymm4, ymm4, ymm6
		vpmovmskb rax, ymm4
		test rax, rax
		jz .Next
		vmovdqa [vecCircle16 + (%1*32)], ymm14
		vmovdqa [vecCircle16 + (%2*32)], ymm15
	%else
		vmovdqu ymm6, [rax + i]
		vmovdqu ymm7, [rsi + i]
		vmovdqa ymm4, vecDarker1
		vmovdqa ymm5, vecDarker1
		vmovdqa [vecCircle16 + (%1*32)], ymm6
		vmovdqa [vecCircle16 + (%2*32)], ymm7
		vpsubusb ymm4, ymm6
		vpsubusb ymm5, ymm7
		vpsubusb ymm6, vecBrighter1
		vpsubusb ymm7, vecBrighter1
		vpcmpeqb ymm4, vecZero
		vpcmpeqb ymm5, vecZero
		vpcmpeqb ymm6, vecZero
		vpcmpeqb ymm7, vecZero
		vpandn ymm4, vec0xFF
		vpandn ymm5, vec0xFF
		vpandn ymm6, vec0xFF
		vpandn ymm7, vec0xFF
		vpor ymm4, ymm5
		vpor ymm6, ymm7
		vpor ymm4, ymm6
		vpmovmskb rax, ymm4
		test rax, rax
		jz .Next
	%endif
%endmacro
%macro _mm_fast_compute_Darkers 4
	%if COMPV_YASM_ABI_IS_64BIT
		vpsubusb ymm4, vecDarker1, [vecCircle16 + (%1*32)]
		vpsubusb ymm5, vecDarker1, [vecCircle16 + (%2*32)]
		vpsubusb ymm6, vecDarker1, [vecCircle16 + (%3*32)]
		vpsubusb ymm7, vecDarker1, [vecCircle16 + (%4*32)]
	%else
		vmovdqa ymm4, vecDarker1
		vmovdqa ymm5, vecDarker1
		vmovdqa ymm6, vecDarker1
		vmovdqa ymm7, vecDarker1
		vpsubusb ymm4, [vecCircle16 + (%1*32)]
		vpsubusb ymm5, [vecCircle16 + (%2*32)]
		vpsubusb ymm6, [vecCircle16 + (%3*32)]
		vpsubusb ymm7, [vecCircle16 + (%4*32)]
	%endif
	vmovdqa [vecDiff16 + (%1*32)], ymm4
	vmovdqa [vecDiff16 + (%2*32)], ymm5
	vmovdqa [vecDiff16 + (%3*32)], ymm6
	vmovdqa [vecDiff16 + (%4*32)], ymm7
%endmacro
%macro _mm_fast_compute_Brighters 4
	vmovdqa ymm4, [vecCircle16 + (%1*32)]
	vmovdqa ymm5, [vecCircle16 + (%2*32)]
	vmovdqa ymm6, [vecCircle16 + (%3*32)]
	vmovdqa ymm7, [vecCircle16 + (%4*32)]
	vpsubusb ymm4, vecBrighter1
	vpsubusb ymm5, vecBrighter1
	vpsubusb ymm6, vecBrighter1
	vpsubusb ymm7, vecBrighter1
	vmovdqa [vecDiff16 + (%1*32)], ymm4
	vmovdqa [vecDiff16 + (%2*32)], ymm5
	vmovdqa [vecDiff16 + (%3*32)], ymm6
	vmovdqa [vecDiff16 + (%4*32)], ymm7
%endmacro
%macro _mm_fast_load 4
	;_mm_fast_compute_ %+ %5 %1, %2, %3, %4
	vpcmpeqb ymm4, vecZero
	vpcmpeqb ymm5, vecZero
	vpcmpeqb ymm6, vecZero
	vpcmpeqb ymm7, vecZero
	vpandn ymm4, vecOne
	vpandn ymm5, vecOne
	vpandn ymm6, vecOne
	vpandn ymm7, vecOne
	vmovdqa [vecDiffBinary16 + (%1*32)], ymm4
	vmovdqa [vecDiffBinary16 + (%2*32)], ymm5
	vmovdqa [vecDiffBinary16 + (%3*32)], ymm6
	vmovdqa [vecDiffBinary16 + (%4*32)], ymm7
	vpaddb ymm4, ymm5
	vpaddb ymm6, ymm7
	vpaddb vecSum1, ymm4
	vpaddb vecSum1, ymm6
%endmacro
%macro _mm_fast_load_Darkers 4
	_mm_fast_compute_Darkers %1, %2, %3, %4
	_mm_fast_load %1, %2, %3, %4
%endmacro
%macro _mm_fast_load_Brighters 4
	_mm_fast_compute_Brighters %1, %2, %3, %4
	_mm_fast_load %1, %2, %3, %4
%endmacro
%macro _mm_fast_init_diffbinarysum 1
	vmovdqa vecSum1, [vecDiffBinary16 + 0*32]
	vmovdqa ymm4, [vecDiffBinary16 + 1*32]
	vmovdqa ymm5, [vecDiffBinary16 + 2*32]
	vmovdqa ymm6, [vecDiffBinary16 + 3*32]
	vpaddb vecSum1, [vecDiffBinary16 + 4*32]
	vpaddb ymm4, [vecDiffBinary16 + 5*32]
	vpaddb ymm5, [vecDiffBinary16 + 6*32]
	vpaddb ymm6, [vecDiffBinary16 + 7*32]
	vpaddb vecSum1, ymm4
	vpaddb ymm5, ymm6
	vpaddb vecSum1, ymm5
	%if %1 == 12
		vmovdqa ymm4, [vecDiffBinary16 + 8*32]
		vmovdqa ymm5, [vecDiffBinary16 + 9*32]
		vmovdqa ymm6, [vecDiffBinary16 + 10*32]
		vpaddb ymm4, ymm5
		vpaddb vecSum1, ymm6
		vpaddb vecSum1, ymm4
	%endif
%endmacro
%macro _mm_fast_strength 2
	vpaddb vecSum1, [vecDiffBinary16 + ((%2-1+%1)&15)*32]
	vpcmpgtb ymm4, vecSum1, vecNMinusOne
	vpmovmskb rax, ymm4
	test rax, rax
	jz %%LessThanNMinusOne
	vmovdqa ymm4, [vecDiff16 + ((%1+0)&15)*32]
	vmovdqa ymm5, [vecDiff16 + ((%1+1)&15)*32]
	vmovdqa ymm6, [vecDiff16 + ((%1+2)&15)*32]
	vmovdqa ymm7, [vecDiff16 + ((%1+3)&15)*32]

	vpminub ymm4, ymm4, [vecDiff16 + ((%1+4)&15)*32]
	vpminub ymm5, ymm5, [vecDiff16 + ((%1+5)&15)*32]
	vpminub ymm6, ymm6, [vecDiff16 + ((%1+6)&15)*32]
	vpminub ymm7, ymm7, [vecDiff16 + ((%1+7)&15)*32]
	vpminub ymm4, ymm4, ymm5
	vpminub ymm6, ymm6, ymm7
	vpminub ymm4, ymm4, ymm6
	vpminub ymm4, ymm4, [vecDiff16 + ((%1+8)&15)*32]
	%if %2 == 12
		vmovdqa ymm5, [vecDiff16 + ((%1+9)&15)*32]
		vmovdqa ymm6, [vecDiff16 + ((%1+10)&15)*32]
		vmovdqa ymm7, [vecDiff16 + ((%1+11)&15)*32]
		vpminub ymm5, ymm5, ymm6
		vpminub ymm4, ymm4, ymm7
		vpminub ymm4, ymm4, ymm5
	%endif
	vpmaxub vecStrengths, vecStrengths, ymm4
	%%LessThanNMinusOne:
	vpsubb vecSum1, [vecDiffBinary16 + (%1&15)*32]
%endmacro
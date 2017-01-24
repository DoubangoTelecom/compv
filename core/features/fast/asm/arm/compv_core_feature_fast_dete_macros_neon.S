#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__arm__)

#########################################################################
.macro _neon32_fast_check a b FastType
	ldr r10, [sp, #(circle + \a*COMPV_GAS_REG_SZ_BYTES)]
	ldr r11, [sp, #(circle + \b*COMPV_GAS_REG_SZ_BYTES)]
	add r10, r10, i
	add r11, r11, i
	vld1.u8 {q14}, [r10]
	vld1.u8 {q15}, [r11]
	vclt.u8 q10, q14, vecDarker1
	vclt.u8 q11, q15, vecDarker1
	vcgt.u8 q12, q14, vecBrighter1
	vcgt.u8 q13, q15, vecBrighter1
	vorr.u8 q10, q10, q11
	vorr.u8 q12, q12, q13
	vorr.u8 q10, q10, q12
	vorr.u8 q11x, q10x, q10y
	vmov.32	r10, q11x[0]
	vmov.32	r11, q11x[1]
	orrs r11, r11, r10
	beq Next_CompVFastDataRow_Asm_NEON32\FastType
	add r10, sp, #(vecCircle16 + \a*16)
	vst1.u8 { q14 }, [r10 :128], eightTimesSixteen
	vst1.u8 { q15 }, [r10 :128]
.endm

#########################################################################
.macro _neon32_fast_compute_darkers a b c d
	add r8, sp, #(vecCircle16 + \a*16)
	# r9 is equal to (c - b) and initialized in _neon32_fast_load
	add r10, sp, #(vecDiff16 + \a*16)
	vld1.u8 {q12}, [r8 :128], eightTimesSixteen
	vld1.u8 {q13}, [r8 :128], r9
	vld1.u8 {q14}, [r8 :128], eightTimesSixteen
	vld1.u8 {q15}, [r8 :128]
	vqsub.u8 q12, vecDarker1, q12
	vqsub.u8 q13, vecDarker1, q13
	vqsub.u8 q14, vecDarker1, q14
	vqsub.u8 q15, vecDarker1, q15
	vst1.u8 { q12 }, [r10 :128], eightTimesSixteen
	vst1.u8 { q13 }, [r10 :128], r9
	vst1.u8 { q14 }, [r10 :128], eightTimesSixteen
	vst1.u8 { q15 }, [r10 :128]
.endm

#########################################################################
.macro _neon32_fast_compute_brighters a b c d
	add r8, sp, #(vecCircle16 + \a*16)
	# r9 is equal to (c - b) and initialized in _neon32_fast_load
	add r10, sp, #(vecDiff16 + \a*16)
	vld1.u8 {q12}, [r8 :128], eightTimesSixteen
	vld1.u8 {q13}, [r8 :128], r9
	vld1.u8 {q14}, [r8 :128], eightTimesSixteen
	vld1.u8 {q15}, [r8 :128]
	vqsub.u8 q12, q12, vecBrighter1
	vqsub.u8 q13, q13, vecBrighter1
	vqsub.u8 q14, q14, vecBrighter1
	vqsub.u8 q15, q15, vecBrighter1
	vst1.u8 { q12 }, [r10 :128], eightTimesSixteen
	vst1.u8 { q13 }, [r10 :128], r9
	vst1.u8 { q14 }, [r10 :128], eightTimesSixteen
	vst1.u8 { q15 }, [r10 :128]
.endm

#########################################################################
.macro _neon32_fast_load a b c d type
	mov r9, #(\c-\b)*16
	_neon32_fast_compute_\type \a, \b, \c, \d
	add r8, sp, #(vecDiffBinary16 + \a*16)
	vcgt.u8 q12, q12, vecZero
	vcgt.u8 q13, q13, vecZero
	vcgt.u8 q14, q14, vecZero
	vcgt.u8 q15, q15, vecZero
	vand.u8 q12, q12, vecOne
	vand.u8 q13, q13, vecOne
	vand.u8 q14, q14, vecOne
	vand.u8 q15, q15, vecOne
	vst1.u8 { q12 }, [r8 :128], eightTimesSixteen
	vst1.u8 { q13 }, [r8 :128], r9
	vst1.u8 { q14 }, [r8 :128], eightTimesSixteen
	vst1.u8 { q15 }, [r8 :128]
	vadd.u8 q12, q12, q13
	vadd.u8 q14, q14, q15
	vadd.u8 vecSum1, vecSum1, q12
	vadd.u8 vecSum1, vecSum1, q14
.endm

#########################################################################
.macro _neon32_fast_init_diffbinarysum FastType
	add r8, sp, #(vecDiffBinary16 + 0*16)
	vld1.u8 {q12}, [r8 :128], sixteen
	vld1.u8 {q13}, [r8 :128], sixteen
	vld1.u8 {q14}, [r8 :128], sixteen
	vld1.u8 {q15}, [r8 :128], sixteen
	vadd.u8 q12, q12, q13
	vadd.u8 q14, q14, q15
	vadd.u8 vecSum1, q12, q14 @ setting vecSum1 for the first time
	vld1.u8 {q12}, [r8 :128], sixteen
	vld1.u8 {q13}, [r8 :128], sixteen
	vld1.u8 {q14}, [r8 :128], sixteen
	vld1.u8 {q15}, [r8 :128], sixteen
	vadd.u8 q12, q12, q13
	vadd.u8 q14, q14, q15
	vadd.u8 vecSum1, vecSum1, q12
	vadd.u8 vecSum1, vecSum1, q14
	.if \FastType == 12
		vld1.u8 {q12}, [r8 :128], sixteen
		vld1.u8 {q13}, [r8 :128], sixteen
		vld1.u8 {q14}, [r8 :128], sixteen
		vadd.u8 q12, q12, q13
		vadd.u8 vecSum1, vecSum1, q14
		vadd.u8 vecSum1, vecSum1, q12
	.endif
.endm

#########################################################################
.macro _neon32_fast_strength ii FastType
	.if \FastType == 12
		add r11, sp, #(vecDiffBinary16 + (((11 + \ii)&15)*16)) @ 11 = NminusOne
	.else
		add r11, sp, #(vecDiffBinary16 + (((8 + \ii)&15)*16)) @ 8 = NminusOne
	.endif
	vld1.u8 {q15}, [r11 :128]
	vadd.u8 vecSum1, vecSum1, q15 @ add tail
	vcge.u8 q11, vecSum1, vecN
	vorr q15x, q11x, q11y
	vmov.32	r10, q15x[0]
	vmov.32	r11, q15x[1]
	orrs r11, r11, r10
	beq FastStrength_AllZeros_CompVFastDataRow_Asm_NEON32\@
	add r8, sp, #(vecDiff16 + (((0 + \ii)&15)*16))
	add r9, sp, #(vecDiff16 + (((1 + \ii)&15)*16))
	add r10, sp, #(vecDiff16 + (((2 + \ii)&15)*16))
	add r11, sp, #(vecDiff16 + (((3 + \ii)&15)*16))
	vld1.u8 {q11}, [r8 :128] @ q11 = vecTemp
	vld1.u8 {q13}, [r9 :128]
	vld1.u8 {q14}, [r10 :128]
	vld1.u8 {q15}, [r11 :128]
	vmin.u8 q11, q11, q13
	vmin.u8 q14, q14, q15
	add r8, sp, #(vecDiff16 + (((4 + \ii)&15)*16))
	add r9, sp, #(vecDiff16 + (((5 + \ii)&15)*16))
	add r10, sp, #(vecDiff16 + (((6 + \ii)&15)*16))
	add r11, sp, #(vecDiff16 + (((7 + \ii)&15)*16))
	vld1.u8 {q12}, [r8 :128]
	vmin.u8 q11, q11, q14
	vld1.u8 {q13}, [r9 :128]
	vld1.u8 {q14}, [r10 :128]
	vld1.u8 {q15}, [r11 :128]
	vmin.u8 q12, q12, q13
	add r8, sp, #(vecDiff16 + (((8 + \ii)&15)*16))
	vld1.u8 {q13}, [r8 :128]
	vmin.u8 q14, q14, q15
	vmin.u8 q12, q12, q13
	vmin.u8 q11, q11, q14
	vmin.u8 q11, q11, q12
	.if \FastType == 12
		add r8, sp, #(vecDiff16 + (((9 + \ii)&15)*16))
		add r9, sp, #(vecDiff16 + (((10 + \ii)&15)*16))
		add r10, sp, #(vecDiff16 + (((11 + \ii)&15)*16))
		vld1.u8 {q12}, [r8 :128]
		vld1.u8 {q13}, [r9 :128]
		vld1.u8 {q14}, [r10 :128]
		vmin.u8 q12, q12, q13
		vmin.u8 q11, q11, q14
		vmin.u8 q11, q11, q12
	.endif
	vmax.u8 vecStrengths, vecStrengths, q11
	FastStrength_AllZeros_CompVFastDataRow_Asm_NEON32\@:
	add r8, sp, #(vecDiffBinary16 + ((\ii&15)*16))
	vld1.u8 {q15}, [r8 :128]
	vsub.u8 vecSum1, vecSum1, q15 @ sub head
.endm

#endif /* defined(__arm__) */

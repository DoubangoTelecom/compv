#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__arm__)

#########################################################################
.macro _neon64_fast_check a b FastType vecDarker1 vecBrighter1
	ldr r10, [sp, #(circle + \a*COMPV_GAS_REG_SZ_BYTES)]
	ldr r11, [sp, #(circle + \b*COMPV_GAS_REG_SZ_BYTES)]
	add r10, r10, i
	add r11, r11, i
	ld1 {v14.16b}, [r10]
	ld1 {v15.16b}, [r11]
	cmlo v10.16b, v14.16b, \vecDarker1
	cmlo v11.16b, v15.16b, \vecDarker1
	cmhi v12.16b, v14.16b, \vecBrighter1
	cmhi v13.16b, v15.16b, \vecBrighter1
	orr v10.16b, v10.16b, v11.16b
	orr v12.16b, v12.16b, v13.16b
	orr v10.16b, v10.16b, v12.16b
	mov r10, v10.d[0]
    mov r11, v10.d[1]
	orr r10, r10, r11 // orrs not avail on Aarch64
	cbz r10, Next_CompVFastDataRow_Asm_NEON64\FastType
	add r10, sp, #(vecCircle16 + \a*16)
	st1 { v14.16b }, [r10], eightTimesSixteen
	st1 { v15.16b }, [r10]
.endm

#########################################################################
.macro _neon64_fast_compute_darkers a b c d vecDarker1
	add r8, sp, #(vecCircle16 + \a*16)
	# r9 is equal to (c - b) and initialized in _neon64_fast_load
	add r10, sp, #(vecDiff16 + \a*16)
	ld1 {v12.16b}, [r8], eightTimesSixteen
	ld1 {v13.16b}, [r8], r9
	ld1 {v14.16b}, [r8], eightTimesSixteen
	ld1 {v15.16b}, [r8]
	uqsub v12.16b, \vecDarker1, v12.16b
	uqsub v13.16b, \vecDarker1, v13.16b
	uqsub v14.16b, \vecDarker1, v14.16b
	uqsub v15.16b, \vecDarker1, v15.16b
	st1 { v12.16b }, [r10], eightTimesSixteen
	st1 { v13.16b }, [r10], r9
	st1 { v14.16b }, [r10], eightTimesSixteen
	st1 { v15.16b }, [r10]
.endm

#########################################################################
.macro _neon64_fast_compute_brighters a b c d vecBrighter1
	add r8, sp, #(vecCircle16 + \a*16)
	# r9 is equal to (c - b) and initialized in _neon64_fast_load
	add r10, sp, #(vecDiff16 + \a*16)
	ld1 {v12.16b}, [r8], eightTimesSixteen
	ld1 {v13.16b}, [r8], r9
	ld1 {v14.16b}, [r8], eightTimesSixteen
	ld1 {v15.16b}, [r8]
	uqsub v12.16b, v12.16b, \vecBrighter1
	uqsub v13.16b, v13.16b, \vecBrighter1
	uqsub v14.16b, v14.16b, \vecBrighter1
	uqsub v15.16b, v15.16b, \vecBrighter1
	st1 { v12.16b }, [r10], eightTimesSixteen
	st1 { v13.16b }, [r10], r9
	st1 { v14.16b }, [r10], eightTimesSixteen
	st1 { v15.16b }, [r10]
.endm

#########################################################################
.macro _neon64_fast_load a b c d type vecDarkerOrvecBrighter1 vecZero vecOne  vecSum1
	mov r9, #(\c-\b)*16
	_neon64_fast_compute_\type \a, \b, \c, \d, \vecDarkerOrvecBrighter1
	add r8, sp, #(vecDiffBinary16 + \a*16)
	cmhi v12.16b, v12.16b, \vecZero
	cmhi v13.16b, v13.16b, \vecZero
	cmhi v14.16b, v14.16b, \vecZero
	cmhi v15.16b, v15.16b, \vecZero
	and v12.16b, v12.16b, \vecOne
	and v13.16b, v13.16b, \vecOne
	and v14.16b, v14.16b, \vecOne
	and v15.16b, v15.16b, \vecOne
	st1 { v12.16b }, [r8], eightTimesSixteen
	st1 { v13.16b }, [r8], r9
	st1 { v14.16b }, [r8], eightTimesSixteen
	st1 { v15.16b }, [r8]
	add v16.16b, v12.16b, v13.16b
	add v17.16b, v14.16b, v15.16b
	add \vecSum1, \vecSum1, v16.16b
	add \vecSum1, \vecSum1, v17.16b
.endm

#########################################################################
.macro _neon64_fast_init_diffbinarysum FastType vecSum1
	add r8, sp, #(vecDiffBinary16 + 0*16)
	ld1 {v12.16b}, [r8], sixteen
	ld1 {v13.16b}, [r8], sixteen
	ld1 {v14.16b}, [r8], sixteen
	ld1 {v15.16b}, [r8], sixteen
	ld1 {v16.16b}, [r8], sixteen
	ld1 {v17.16b}, [r8], sixteen
	ld1 {v18.16b}, [r8], sixteen
	ld1 {v19.16b}, [r8], sixteen
    add v20.16b, v12.16b, v13.16b
    add v21.16b, v14.16b, v15.16b
    add v22.16b, v16.16b, v17.16b
    add v23.16b, v18.16b, v19.16b
    add v24.16b, v20.16b, v21.16b
    add \vecSum1, v22.16b, v23.16b // setting vecSum1 for the first time
    add \vecSum1, \vecSum1, v24.16b
	.if \FastType == 12
		ld1 {v12.16b}, [r8], sixteen
		ld1 {v13.16b}, [r8], sixteen
		ld1 {v14.16b}, [r8], sixteen
		add v15.16b, v12.16b, v13.16b
		add \vecSum1, \vecSum1, v14.16b
		add \vecSum1, \vecSum1, v15.16b
	.endif
.endm

#########################################################################
.macro _neon64_fast_strength ii FastType vecSum1 vecStrengths vecN
	.if \FastType == 12
		add r11, sp, #(vecDiffBinary16 + (((11 + \ii)&15)*16)) // 11 = NminusOne
	.else
		add r11, sp, #(vecDiffBinary16 + (((8 + \ii)&15)*16)) // 8 = NminusOne
	.endif
	ld1 {v15.16b}, [r11]
	add \vecSum1, \vecSum1, v15.16b // add tail
	cmhs v11.16b, \vecSum1, \vecN
    mov r10, v11.d[0]
    mov r11, v11.d[1]
    orr r10, r10, r11 // orrs not avail on Aarch64
	cbz r10, FastStrength_AllZeros_CompVFastDataRow_Asm_NEON64\FastType\@
	add r8, sp, #(vecDiff16 + (((0 + \ii)&15)*16))
	add r9, sp, #(vecDiff16 + (((1 + \ii)&15)*16))
	add r10, sp, #(vecDiff16 + (((2 + \ii)&15)*16))
	add r11, sp, #(vecDiff16 + (((3 + \ii)&15)*16))
    add r12, sp, #(vecDiff16 + (((4 + \ii)&15)*16))
	add r13, sp, #(vecDiff16 + (((5 + \ii)&15)*16))
	add r14, sp, #(vecDiff16 + (((6 + \ii)&15)*16))
	add r15, sp, #(vecDiff16 + (((7 + \ii)&15)*16))
    add r16, sp, #(vecDiff16 + (((8 + \ii)&15)*16))
	ld1 {v11.16b}, [r8]
	ld1 {v12.16b}, [r9]
	ld1 {v13.16b}, [r10]
	ld1 {v14.16b}, [r11]
    ld1 {v15.16b}, [r12]
    ld1 {v16.16b}, [r13]
    ld1 {v17.16b}, [r14]
    ld1 {v18.16b}, [r15]
    ld1 {v19.16b}, [r16]
    umin v20.16b, v11.16b, v12.16b
    umin v21.16b, v13.16b, v14.16b
    umin v22.16b, v15.16b, v16.16b
    umin v23.16b, v17.16b, v18.16b
    umin v24.16b, v19.16b, v20.16b
    umin v25.16b, v21.16b, v22.16b
    umin v26.16b, v23.16b, v24.16b
    umin v26.16b, v25.16b, v26.16b
	.if \FastType == 12
		add r8, sp, #(vecDiff16 + (((9 + \ii)&15)*16))
		add r9, sp, #(vecDiff16 + (((10 + \ii)&15)*16))
		add r10, sp, #(vecDiff16 + (((11 + \ii)&15)*16))
		ld1 {v12.16b}, [r8]
		ld1 {v13.16b}, [r9]
		ld1 {v14.16b}, [r10]
		umin v12.16b, v12.16b, v13.16b
		umin v26.16b, v26.16b, v14.16b
		umin v26.16b, v26.16b, v12.16b
	.endif
	umax \vecStrengths, \vecStrengths, v26.16b
	FastStrength_AllZeros_CompVFastDataRow_Asm_NEON64\FastType\@:
	add r8, sp, #(vecDiffBinary16 + ((\ii&15)*16))
	ld1 {v15.16b}, [r8]
	sub \vecSum1, \vecSum1, v15.16b // sub head
.endm

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
	beq FastStrength_AllZeros_CompVFastDataRow_Asm_NEON32\FastType\@
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
	FastStrength_AllZeros_CompVFastDataRow_Asm_NEON32\FastType\@:
	add r8, sp, #(vecDiffBinary16 + ((\ii&15)*16))
	vld1.u8 {q15}, [r8 :128]
	vsub.u8 vecSum1, vecSum1, q15 @ sub head
.endm

#endif /* defined(__arm__) */
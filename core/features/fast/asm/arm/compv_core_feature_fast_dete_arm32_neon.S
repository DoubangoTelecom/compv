@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   @
@ File author: Mamadou DIOP (Doubango Telecom, France).                 @
@ License: GPLv3. For commercial license please contact us.             @
@ Source code: https://github.com/DoubangoTelecom/compv                 @
@ WebSite: http://compv.org                                             @
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.include "compv_common_arm.S"

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> const uint8_t* Iptr
@ arg(1) -> COMPV_ALIGNED(NEON) compv_uscalar_t width
@ arg(2) -> COMPV_ALIGNED(NEON) const compv_scalar_t *pixels16
@ arg(3) -> compv_uscalar_t N
@ arg(4) -> compv_uscalar_t threshold
@ arg(5) -> uint8_t* strengths
COMPV_GAS_FUNCTION_DECLARE CompVFastDataRow_Asm_NEON32
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 6
	COMPV_GAS_SAVE_NEON_REGS
	COMPV_GAS_ALIGN_STACK 16, r11
	COMPV_GAS_MEMALLOC (COMPV_GAS_REG_SZ_BYTES*16) + (16*16) + (16*16) + (16*16)
	
	ldmia_args r0-r5
	Iptr .req r0
	width .req r1
	pixels16 .req r2
	N .req r3
	threshold .req r4
	strengths .req r5

	.equ circle					, 0
	.equ vecDiffBinary16		, (circle + (COMPV_GAS_REG_SZ_BYTES*16))
	.equ vecDiff16				, (vecDiffBinary16 + (16*16))
	.equ vecCircle16			, (vecDiff16 + (16*16))

	vecDarker1 .req q0
	vecBrighter1 .req q1
	vecThreshold .req q2
	vecStrengths .req q3
	vecZero .req q4
	vecSum1 .req q5
	vecMinSum .req q6
	vecN .req q7
	vecOne .req q8

	mov r11, #0
	vdup.8 vecZero, r11

	mov r11, #2 @ FIXME: 3 for FAST12
	vdup.8 vecMinSum, r11

	mov r11, #9 @FIXME: 12 for FAST12
	vdup.8 vecN, r11

	mov r11, #1
	vdup.8 vecOne, r11
	
	vdup.8 vecThreshold, threshold
	@ threshold(r4) not longer needed

	@ Load circle @
	add r10, sp, #circle
	.rept 16
		ldr r11, [pixels16], #COMPV_GAS_REG_SZ_BYTES
		add r11, Iptr, r11
		str r11, [r10], #COMPV_GAS_REG_SZ_BYTES
	.endr
	@ pixels16 no longer needed -> use it as i counter
	i .req pixels16

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (i = 0; i < width; i += 16)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	mov i, #0
	LoopWidth_CompVFastDataRow_Asm_NEON32:
		vld1.u8 { vecDarker1 }, [Iptr]!
		vmov.u8 vecStrengths, vecZero
		vqadd.u8 vecBrighter1, vecDarker1, vecThreshold 
		vqsub.u8 vecDarker1, vecDarker1, vecThreshold

		
		.macro _neon32_fast_check a b
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
			beq Next_CompVFastDataRow_Asm_NEON32
			add r10, sp, #(vecCircle16 + \a*16)
			add r11, sp, #(vecCircle16 + \b*16)
			vst1.u8 { q14 }, [r10 :128]
			vst1.u8 { q15 }, [r11 :128]
		.endm

		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ Check
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		_neon32_fast_check 0 8
		_neon32_fast_check 4 12
		_neon32_fast_check 1 9
		_neon32_fast_check 5 13
		_neon32_fast_check 2 10
		_neon32_fast_check 6 14
		_neon32_fast_check 3 11
		_neon32_fast_check 7 15

		.macro _neon32_fast_compute_darkers a b c d
			add r8, sp, #(vecCircle16 + \a*16)
			add r9, sp, #(vecCircle16 + \b*16)
			add r10, sp, #(vecCircle16 + \c*16)
			add r11, sp, #(vecCircle16 + \d*16)
			vld1.u8 {q12}, [r8 :128]
			vld1.u8 {q13}, [r9 :128]
			vld1.u8 {q14}, [r10 :128]
			vld1.u8 {q15}, [r11 :128]
			add r8, sp, #(vecDiff16 + \a*16)
			add r9, sp, #(vecDiff16 + \b*16)
			add r10, sp, #(vecDiff16 + \c*16)
			add r11, sp, #(vecDiff16 + \d*16)
			vqsub.u8 q12, vecDarker1, q12
			vqsub.u8 q13, vecDarker1, q13
			vqsub.u8 q14, vecDarker1, q14
			vqsub.u8 q15, vecDarker1, q15
			vst1.u8 { q12 }, [r8 :128]
			vst1.u8 { q13 }, [r9 :128]
			vst1.u8 { q14 }, [r10 :128]
			vst1.u8 { q15 }, [r11 :128]
		.endm
		.macro _neon32_fast_compute_brighters a b c d
			add r8, sp, #(vecCircle16 + \a*16)
			add r9, sp, #(vecCircle16 + \b*16)
			add r10, sp, #(vecCircle16 + \c*16)
			add r11, sp, #(vecCircle16 + \d*16)
			vld1.u8 {q12}, [r8 :128]
			vld1.u8 {q13}, [r9 :128]
			vld1.u8 {q14}, [r10 :128]
			vld1.u8 {q15}, [r11 :128]
			add r8, sp, #(vecDiff16 + \a*16)
			add r9, sp, #(vecDiff16 + \b*16)
			add r10, sp, #(vecDiff16 + \c*16)
			add r11, sp, #(vecDiff16 + \d*16)
			vqsub.u8 q12, q12, vecBrighter1
			vqsub.u8 q13, q13, vecBrighter1
			vqsub.u8 q14, q14, vecBrighter1
			vqsub.u8 q15, q15, vecBrighter1
			vst1.u8 { q12 }, [r8 :128]
			vst1.u8 { q13 }, [r9 :128]
			vst1.u8 { q14 }, [r10 :128]
			vst1.u8 { q15 }, [r11 :128]
		.endm
		.macro _neon32_fast_load a b c d type
			_neon32_fast_compute_\type \a \b \c \d
			add r8, sp, #(vecDiffBinary16 + \a*16)
			add r9, sp, #(vecDiffBinary16 + \b*16)
			add r10, sp, #(vecDiffBinary16 + \c*16)
			add r11, sp, #(vecDiffBinary16 + \d*16)
			vcgt.u8 q12, q12, vecZero
			vcgt.u8 q13, q13, vecZero
			vcgt.u8 q14, q14, vecZero
			vcgt.u8 q15, q15, vecZero
			vand.u8 q12, q12, vecOne
			vand.u8 q13, q13, vecOne
			vand.u8 q14, q14, vecOne
			vand.u8 q15, q15, vecOne
			vst1.u8 { q12 }, [r8 :128]
			vst1.u8 { q13 }, [r9 :128]
			vst1.u8 { q14 }, [r10 :128]
			vst1.u8 { q15 }, [r11 :128]
			vadd.u8 q12, q12, q13
			vadd.u8 q14, q14, q15
			vadd.u8 vecSum1, vecSum1, q12
			vadd.u8 vecSum1, vecSum1, q14
		.endm
		.macro _neon32_fast_init_diffbinarysum
			add r8, sp, #(vecDiffBinary16 + 0*16)
			add r9, sp, #(vecDiffBinary16 + 1*16)
			add r10, sp, #(vecDiffBinary16 + 2*16)
			add r11, sp, #(vecDiffBinary16 + 3*16)
			vld1.u8 {q12}, [r8 :128]
			vld1.u8 {q13}, [r9 :128]
			vld1.u8 {q14}, [r10 :128]
			vld1.u8 {q15}, [r11 :128]
			vadd.u8 q12, q12, q13
			vadd.u8 q14, q14, q15
			vadd.u8 vecSum1, q12, q14 @ setting vecSum1 for the first time
			add r8, sp, #(vecDiffBinary16 + 4*16)
			add r9, sp, #(vecDiffBinary16 + 5*16)
			add r10, sp, #(vecDiffBinary16 + 6*16)
			add r11, sp, #(vecDiffBinary16 + 7*16)
			vld1.u8 {q12}, [r8 :128]
			vld1.u8 {q13}, [r9 :128]
			vld1.u8 {q14}, [r10 :128]
			vld1.u8 {q15}, [r11 :128]
			vadd.u8 q12, q12, q13
			vadd.u8 q14, q14, q15
			vadd.u8 vecSum1, vecSum1, q12
			vadd.u8 vecSum1, vecSum1, q14
			@FIXME: 3 more adds for FAST12
		.endm
		.macro _neon32_fast_strength ii
			add r11, sp, #(vecDiffBinary16 + (((8 + \ii)&15)*16)) @FIXME: NminusOne = 11 for FAST12 and 8 for FAST9
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
			vmin.u8 q11, q11, q14 @ FIXME: hide latency -> q14 just above
			add r8, sp, #(vecDiff16 + (((4 + \ii)&15)*16))
			add r9, sp, #(vecDiff16 + (((5 + \ii)&15)*16))
			add r10, sp, #(vecDiff16 + (((6 + \ii)&15)*16))
			add r11, sp, #(vecDiff16 + (((7 + \ii)&15)*16))
			vld1.u8 {q12}, [r8 :128]
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
			@ FIXME: for FAST12 there are three more loads
			vmax.u8 vecStrengths, vecStrengths, q11
			FastStrength_AllZeros_CompVFastDataRow_Asm_NEON32\@:
			add r8, sp, #(vecDiffBinary16 + ((\ii&15)*16))
			vld1.u8 {q15}, [r8 :128]
			vsub.u8 vecSum1, vecSum1, q15 @ sub head
		.endm

		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ Darkers
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			vmov.u8 vecSum1, vecZero
			_neon32_fast_load 0 8 4 12 darkers
			vcge.u8 q14, vecSum1, vecMinSum
			vorr q15x, q14x, q14y
			vmov.32	r10, q15x[0]
			vmov.32	r11, q15x[1]
			orrs r11, r11, r10
			beq EndOfDarkers_CompVFastDataRow_Asm_NEON32
			_neon32_fast_load 1 9 5 13 darkers
			_neon32_fast_load 2 10 6 14 darkers
			_neon32_fast_load 3 11 7 15 darkers
			vcge.u8 q14, vecSum1, vecN
			vorr q15x, q14x, q14y
			vmov.32	r10, q15x[0]
			vmov.32	r11, q15x[1]
			orrs r11, r11, r10
			beq EndOfDarkers_CompVFastDataRow_Asm_NEON32

			_neon32_fast_init_diffbinarysum
			.irp ii, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
				_neon32_fast_strength \ii
			.endr
			EndOfDarkers_CompVFastDataRow_Asm_NEON32:

		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ Brighters
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			vmov.u8 vecSum1, vecZero
			_neon32_fast_load 0 8 4 12 brighters
			vcge.u8 q14, vecSum1, vecMinSum
			vorr q15x, q14x, q14y
			vmov.32	r10, q15x[0]
			vmov.32	r11, q15x[1]
			orrs r11, r11, r10
			beq EndOfBrighters_CompVFastDataRow_Asm_NEON32
			_neon32_fast_load 1 9 5 13 brighters
			_neon32_fast_load 2 10 6 14 brighters
			_neon32_fast_load 3 11 7 15 brighters
			vcge.u8 q14, vecSum1, vecN
			vorr q15x, q14x, q14y
			vmov.32	r10, q15x[0]
			vmov.32	r11, q15x[1]
			orrs r11, r11, r10
			beq EndOfBrighters_CompVFastDataRow_Asm_NEON32

			_neon32_fast_init_diffbinarysum
			.irp ii, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
				_neon32_fast_strength \ii
			.endr
			EndOfBrighters_CompVFastDataRow_Asm_NEON32:

		Next_CompVFastDataRow_Asm_NEON32:
		vst1.u8 {vecStrengths}, [strengths]!

		add i, i, #16
		cmp i, width
		blt LoopWidth_CompVFastDataRow_Asm_NEON32
		@End_of_LoopWidth@
	
	
	.unreq Iptr
	.unreq width
	.unreq pixels16
	.unreq N
	.unreq threshold
	.unreq strengths
	.unreq i

	.unreq vecDarker1
	.unreq vecBrighter1
	.unreq vecThreshold
	.unreq vecStrengths
	.unreq vecZero
	.unreq vecSum1
	.unreq vecMinSum
	.unreq vecN
	.unreq vecOne

	COMPV_GAS_MEMFREE (COMPV_GAS_REG_SZ_BYTES*16) + (16*16) + (16*16) + (16*16)
	COMPV_GAS_UNALIGN_STACK r11
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 6
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) uint8_t* pcStrengthsMap
@ arg(1) -> COMPV_ALIGNED(NEON) uint8_t* pNMS
@ arg(2) -> compv_uscalar_t width
@ arg(3) -> compv_uscalar_t heigth
@ arg(4) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVFastNmsApply_Asm_NEON32
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 5
	COMPV_GAS_SAVE_NEON_REGS
	
	ldmia_args r0-r4
	pcStrengthsMap .req r0
	pNMS .req r1
	width .req r2
	heigth .req r3
	stride .req r4

	nms .req r5
	i .req r6
	vec0 .req q0	
	vecZero .req q1

	mov r11, #0
	vdup.8 vecZero, r11
	add r11, stride, stride, LSL #1 @ r11 = stride * 3
	sub heigth, heigth, #6 @ [j starts at #3 and end at heigth - #3] -> loop executed (heigth - #6) times
	add pcStrengthsMap, pcStrengthsMap, r11
	add pNMS, pNMS, r11

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (j = 3; j < heigth - 3; ++j)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopHeight_CompVFastNmsApply_Asm_NEON32:
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (i = 0; i < width; i += 16)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		mov i, #0
		mov nms, pNMS
		LoopWidth_CompVFastNmsApply_Asm_NEON32:
			pld [nms, #(CACHE_LINE_SIZE*3)]
			vld1.u8 { vec0 }, [nms :128]!
			vcgt.u8 vec0, vec0, vecZero
			vorr.u8 q15x, q0x, q0y
			vmov.32	r10, q15x[0]
			vmov.32	r11, q15x[1]
			orrs r11, r11, r10
			beq AllZeros_CompVFastNmsApply_Asm_NEON32
			add r10, pNMS, i
			add r11, pcStrengthsMap, i
			vld1.u8 { q15 }, [r11 :128]
			vst1.u8 { vecZero }, [r10 :128]
			vbic.u8 q15, q15, vec0
			vst1.u8 { q15 }, [r11 : 128]
			AllZeros_CompVFastNmsApply_Asm_NEON32:

			add i, i, #16
			cmp i, width
			blt LoopWidth_CompVFastNmsApply_Asm_NEON32
			@End_of_LoopWidth@

		add pcStrengthsMap, pcStrengthsMap, stride
		add pNMS, pNMS, stride
		subs heigth, heigth, #1
		bne LoopHeight_CompVFastNmsApply_Asm_NEON32
		@End_of_LoopHeight@

	.unreq pcStrengthsMap
	.unreq pNMS
	.unreq width
	.unreq heigth
	.unreq stride
	.unreq nms
	.unreq i
	.unreq vec0
	.unreq vecZero

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 5
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> const uint8_t* pcStrengthsMap
@ arg(1) -> uint8_t* pNMS
@ arg(2) -> const compv_uscalar_t width
@ arg(3) -> compv_uscalar_t heigth
@ arg(4) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVFastNmsGather_Asm_NEON32
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 5
	COMPV_GAS_SAVE_NEON_REGS

	ldmia_args r0-r4
	pcStrengthsMap .req r0
	pNMS .req r1
	width .req r2
	heigth .req r3
	stride .req r4

	pld [pcStrengthsMap, #(CACHE_LINE_SIZE*0)]
	pld [pcStrengthsMap, #(CACHE_LINE_SIZE*1)]
	pld [pcStrengthsMap, #(CACHE_LINE_SIZE*2)]

	i .req r5
	strengths .req r6
	minusStrideMinusSeventeen .req r7
	one .req r8
	minusTwo .req r9
	vec0 .req q0
	vec1 .req q1
	vecStrength .req q2
	vecZero .req q3

	mov minusStrideMinusSeventeen, #-17
	mov one, #1
	mov minusTwo, #-2
	mov i, #0
	add r11, stride, stride, LSL #1 @ r11 = stride * 3
	vdup.8 vecZero, i
	sub width, width, #3
	sub heigth, heigth, #6 @ [j starts at #3 and end at heigth - #3] -> loop executed (heigth - #6) times
	sub minusStrideMinusSeventeen, minusStrideMinusSeventeen, stride
	add pcStrengthsMap, pcStrengthsMap, r11
	add pNMS, pNMS, r11

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (j = 3; j < heigth - 3; ++j)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopHeight_CompVFastNmsGather_Asm_NEON32:
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (i = 3; i < width - 3; i += 16)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		mov i, #3
		add strengths, pcStrengthsMap, #3
		LoopWidth_CompVFastNmsGather_Asm_NEON32:
			pld [strengths, #(CACHE_LINE_SIZE*3)]
			vld1.u8 { vecStrength }, [strengths]!                     @ indexOf(strengths) = i + 16
			vcgt.u8 vec1, vecStrength, vecZero
			vorr q15x, q1x, q1y
			vmov.32	r10, q15x[0]
			vmov.32	r11, q15x[1]
			orrs r11, r11, r10
			beq AllZeros_CompVFastNmsGather_Asm_NEON32
			add r11, strengths, minusStrideMinusSeventeen             
			vld1.u8 { vec0 }, [r11], one                              @ r11 = i - stride - 1
			vld1.u8 { q15 }, [r11], one                               @ r11 = i - stride
			vld1.u8 { q14 }, [r11], stride                            @ r11 = i - stride + 1
			vld1.u8 { q13 }, [r11], minusTwo                          @ r11 = i + 1
			vld1.u8 { q12 }, [r11], stride                            @ r11 = i - 1
			vld1.u8 { q11 }, [r11], one                               @ r11 = i + stride - 1
			vld1.u8 { q10 }, [r11], one                               @ r11 = i + stride
			vld1.u8 { q9 }, [r11]                                     @ r11 = i + stride + 1
			add r10, pNMS, i
			vcge.u8 vec0, vec0, vecStrength
			vcge.u8 q15, q15, vecStrength
			vcge.u8 q14, q14, vecStrength
			vcge.u8 q13, q13, vecStrength
			vcge.u8 q12, q12, vecStrength
			vcge.u8 q11, q11, vecStrength
			vcge.u8 q10, q10, vecStrength
			vcge.u8 q9, q9, vecStrength
			vorr.u8 q15, q15, q14
			vorr.u8 q13, q13, q12
			vorr.u8 q11, q11, q10
			vorr.u8 vec0, vec0, q9
			vorr.u8 q15, q13, q15
			vorr.u8 vec0, vec0, q11
			vorr.u8 vec0, vec0, q15
			vand.u8 vec0, vec0, vec1
			vst1.u8 { vec0 }, [r10]
			AllZeros_CompVFastNmsGather_Asm_NEON32:

			add i, i, #16
			cmp i, width
			blt LoopWidth_CompVFastNmsGather_Asm_NEON32
			@End_of_LoopWidth@

		add pcStrengthsMap, pcStrengthsMap, stride
		add pNMS, pNMS, stride
		subs heigth, heigth, #1
		bne LoopHeight_CompVFastNmsGather_Asm_NEON32
		@End_of_LoopHeight@

	.unreq pcStrengthsMap
	.unreq pNMS
	.unreq width
	.unreq heigth
	.unreq stride
	.unreq i
	.unreq strengths
	.unreq minusStrideMinusSeventeen
	.unreq one
	.unreq minusTwo
	.unreq vec0
	.unreq vec1
	.unreq vecStrength
	.unreq vecZero

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 5
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
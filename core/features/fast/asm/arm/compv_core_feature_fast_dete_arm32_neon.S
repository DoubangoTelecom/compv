@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   @
@ File author: Mamadou DIOP (Doubango Telecom, France).                 @
@ License: GPLv3. For commercial license please contact us.             @
@ Source code: https://github.com/DoubangoTelecom/compv                 @
@ WebSite: http://compv.org                                             @
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.include "compv_common_arm.S"

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
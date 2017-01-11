@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   @
@ File author: Mamadou DIOP (Doubango Telecom, France).                 @
@ License: GPLv3. For commercial license please contact us.             @
@ Source code: https://github.com/DoubangoTelecom/compv                 @
@ WebSite: http://compv.org                                             @
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.include "compv_common_arm.S"

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
	vec0 .req q0
	vec1 .req q1
	vecStrength .req q2
	vecZero .req q3

	mov i, #0
	vdup.8 vecZero, i
	sub width, width, #3
	sub heigth, heigth, #6 @ [j start at #3 and end at heigth - #3] -> loop executed (heigth - #6) times

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (j = 3; j < heigth - 3; ++j)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopHeight_CompVFastNmsGather_Asm_NEON32:
		mov i, #3
		mov strengths, pcStrengthsMap

		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (i = 3; i < width - 3; i += 16)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		LoopWidth_CompVFastNmsGather_Asm_NEON32:
			add i, i, #16
			pld [pcStrengthsMap, #(CACHE_LINE_SIZE*3)]
			vld1.u8 { vecStrength }, [strengths]!
			vcgt.u8 vec1, vecStrength, vecZero
			vorr q1x, q1x, q1y
			vmov.32	r10, q1x[0] 
			vmov.32	r11, q1x[1]
			orrs r11, r11, r10
			beq AllZeros_CompVFastNmsGather_Asm_NEON32
			sub r8, i, #1 @ r8 = (i - 1) = left
			add r9, i, #1 @ r9 = (i + 1) = right
			sub r10, r8, stride @ r10 = (r8 - stride) = (i - 1 - stride) = lef-top
			sub r11, i, stride @ r11 = (i - stride) = top
			vld1.u8 { vec0 }, [r8]
			vld1.u8 { q13 }, [r9]
			vld1.u8 { q14 }, [r10]
			vld1.u8 { q15 }, [r11]
			vcge.u8 vec0, vec0, vecStrength
			vcge.u8 q13, q13, vecStrength
			vcge.u8 q14, q14, vecStrength
			vcge.u8 q15, q15, vecStrength
			vorr vec0, vec0, q13
			vorr q14, q14, q15
			vorr vec0, vec0, q14 @ FIXME: too close to previous line defining q14, hide latency
			add r8, r11, #1 @ r8 = (r11 + 1) = (i - stride + 1) = right-top
			add r9, i, stride @ r9 = (i + stride) = bottom
			sub r10, r9, #1 @ r10 = (r9 - 1) = (i + stride - 1) = left-bottom
			add r11, r9, #1 @ r11 = (r9 + 1) = (i + stride + 1) = right-bottom
			vld1.u8 { q12 }, [r8]
			vld1.u8 { q13 }, [r9]
			vld1.u8 { q14 }, [r10]
			vld1.u8 { q15 }, [r11]
			vcge.u8 q12, q12, vecStrength
			vcge.u8 q13, q13, vecStrength
			vcge.u8 q14, q14, vecStrength
			vcge.u8 q15, q15, vecStrength
			vorr q12, q12, q13
			vorr q14, q14, q15
			vorr vec0, vec0, q12
			vorr vec0, vec0, q14
			add r7, pNMS, i
			vst1.u8 { vec0 }, [r7]
			AllZeros_CompVFastNmsGather_Asm_NEON32:

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
	.unreq vec0
	.unreq vec1
	.unreq vecStrength
	.unreq vecZero

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 5
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
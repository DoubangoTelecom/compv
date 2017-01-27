#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__aarch64__)
.include "compv_common_arm64.S"
.include "compv_core_feature_fast_dete_macros_neon.S"

#########################################################################
# arg(0) -> const uint8_t* pcStrengthsMap
# arg(1) -> uint8_t* pNMS
# arg(2) -> const compv_uscalar_t width
# arg(3) -> compv_uscalar_t heigth
# arg(4) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVFastNmsGather_Asm_NEON64
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 5
	COMPV_GAS_SAVE_NEON_REGS

	ldp_arg 0, r0, r1
    ldp_arg 2, r2, r3
    ldr_arg 4, r4
	pcStrengthsMap .req r0
	pNMS .req r1
	width .req r2
	heigth .req r3
	stride .req r4

	prfm pldl1keep, [pcStrengthsMap, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [pcStrengthsMap, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [pcStrengthsMap, #(CACHE_LINE_SIZE*2)]

	i .req r5
	strengths .req r6
	minusStrideMinusSeventeen .req r7
	one .req r8
	minusTwo .req r9
    pNMS_ .req r10
	#define vec0 v0
	#define vec1 v1
	#define vecStrength v2
	#define vecZero v3

	mov minusStrideMinusSeventeen, #-17
	mov one, #1
	mov minusTwo, #-2
    lsl r28, stride, #1
	mov r27, #0
	add r28, r28, stride // r28 = stride * 3
	dup vecZero.16b, r27w
	sub width, width, #3
	sub heigth, heigth, #6 // [j starts at #3 and end at heigth - #3] -> loop executed (heigth - #6) times
	sub minusStrideMinusSeventeen, minusStrideMinusSeventeen, stride
	add pcStrengthsMap, pcStrengthsMap, r28
	add pNMS, pNMS, r28

	#######################################
	# for (j = 3; j < heigth - 3; ++j)
	#######################################
	LoopHeight_CompVFastNmsGather_Asm_NEON64:
		#######################################
		# for (i = 3; i < width - 3; i += 16)
		#######################################
		mov i, #3
		add strengths, pcStrengthsMap, #3
        add pNMS_, pNMS, #3
		LoopWidth_CompVFastNmsGather_Asm_NEON64:
			prfm pldl1keep, [strengths, #(CACHE_LINE_SIZE*3)]
			ld1 { vecStrength.16b }, [strengths], #16                     // indexOf(strengths) = i + 16
			cmhi vec1.16b, vecStrength.16b, vecZero.16b
            mov r27, v21.d[0]
            mov r28, v21.d[1]
			orr r27, r27, r28 // orrs not avail on Aarch64
            cmp r27, #0
			beq AllZeros_CompVFastNmsGather_Asm_NEON64
			add r11, strengths, minusStrideMinusSeventeen             
			ld1 { vec0.16b }, [r11], one                              // r11 = i - stride - 1
			ld1 { v15.16b }, [r11], one                               // r11 = i - stride
			ld1 { v14.16b }, [r11], stride                            // r11 = i - stride + 1
			ld1 { v13.16b }, [r11], minusTwo                          // r11 = i + 1
			ld1 { v12.16b }, [r11], stride                            // r11 = i - 1
			ld1 { v11.16b }, [r11], one                               // r11 = i + stride - 1
			ld1 { v10.16b }, [r11], one                               // r11 = i + stride
			ld1 { v9.16b }, [r11]                                     // r11 = i + stride + 1
			cmhs vec0.16b, vec0.16b, vecStrength.16b
			cmhs v15.16b, v15.16b, vecStrength.16b
			cmhs v14.16b, v14.16b, vecStrength.16b
			cmhs v13.16b, v13.16b, vecStrength.16b
			cmhs v12.16b, v12.16b, vecStrength.16b
			cmhs v11.16b, v11.16b, vecStrength.16b
			cmhs v10.16b, v10.16b, vecStrength.16b
			cmhs v9.16b, v9.16b, vecStrength.16b
            
			orr v15.16b, v15.16b, v14.16b
			orr v13.16b, v13.16b, v12.16b
			orr v11.16b, v11.16b, v10.16b
			orr vec0.16b, vec0.16b, v9.16b
			orr v15.16b, v13.16b, v15.16b
			orr vec0.16b, vec0.16b, v11.16b
			orr vec0.16b, vec0.16b, v15.16b
			and vec0.16b, vec0.16b, vec1.16b
            st1 { vec0.16b }, [pNMS_], #16
			AllZeros_CompVFastNmsGather_Asm_NEON64:

			add i, i, #16
			cmp i, width
			blt LoopWidth_CompVFastNmsGather_Asm_NEON64
			#End_of_LoopWidth#

		add pcStrengthsMap, pcStrengthsMap, stride
		add pNMS, pNMS, stride
		subs heigth, heigth, #1
		bne LoopHeight_CompVFastNmsGather_Asm_NEON64
		#End_of_LoopHeight#

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
    .unreq pNMS_
	#undef vec0
	#undef vec1
	#undef vecStrength
	#undef vecZero

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__aarch64__) */

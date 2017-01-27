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
# arg(0) -> const uint8_t* Iptr
# arg(1) -> COMPV_ALIGNED(NEON) compv_uscalar_t width
# arg(2) -> COMPV_ALIGNED(NEON) const compv_scalar_t *pixels16
# arg(3) -> compv_uscalar_t N
# arg(4) -> compv_uscalar_t threshold
# arg(5) -> uint8_t* strengths
.macro CompVFastDataRow_Macro_NEON64 FastType
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 6
	COMPV_GAS_SAVE_NEON_REGS
	COMPV_GAS_ALIGN_STACK 16, r9
	COMPV_GAS_MEMALLOC (COMPV_GAS_REG_SZ_BYTES*16) + (16*16) + (16*16) + (16*16)
	
	ldp_arg 0, r0, r1
    ldp_arg 2, r2, r3
    ldp_arg 4, r4, r5
	Iptr .req r0
	width .req r1
	pixels16 .req r2
	N .req r3
	threshold .req r4
	strengths .req r5

	eightTimesSixteen .req r6

	.equ circle					, 0
	.equ vecDiffBinary16		, (circle + (COMPV_GAS_REG_SZ_BYTES*16))
	.equ vecDiff16				, (vecDiffBinary16 + (16*16))
	.equ vecCircle16			, (vecDiff16 + (16*16))

	#define vecDarker1 v0
	#define vecBrighter1 v1
	#define vecThreshold v2
	#define vecStrengths v3
	#define vecZero v4
	#define vecSum1 v5
	#define vecMinSum v6
	#define vecN v7
	#define vecOne v8

	mov eightTimesSixteen, #(8*16)

	mov r11, #0
	dup vecZero.16b, r11w

	.if  \FastType == 9
		mov r10, #9
		mov r11, #2
	.else
		mov r10, #12
		mov r11, #3
	.endif
	dup vecN.16b, r10w
	dup vecMinSum.16b, r11w

	mov r11, #1
	dup vecOne.16b, r11w
	
	dup vecThreshold.16b, r4w
	# threshold(r4) not longer needed -> use it as sixteen
	sixteen .req threshold
    .unreq threshold
	mov sixteen, #16

	## Load circle ##
	add r10, sp, #circle
	.rept 16
		ldr r11, [pixels16], #COMPV_GAS_REG_SZ_BYTES
		add r11, Iptr, r11
		str r11, [r10], #COMPV_GAS_REG_SZ_BYTES
	.endr
	# pixels16(r2) no longer needed -> use it as i counter
	i .req pixels16
    .unreq pixels16

	#######################################
	# for (i = 0; i < width; i += 16)
	#######################################
	mov i, #0
	LoopWidth_CompVFastDataRow_Asm_NEON64\@:
		ld1 { vecDarker1.16b }, [Iptr], #16
		mov vecStrengths.16b, vecZero.16b
		uqadd vecBrighter1.16b, vecDarker1.16b, vecThreshold.16b
		uqsub vecDarker1.16b, vecDarker1.16b, vecThreshold.16b

		#######################################
		# Check
		#######################################
		_neon64_fast_check 0, 8, \FastType, vecDarker1.16b, vecBrighter1.16b
		_neon64_fast_check 4, 12, \FastType, vecDarker1.16b, vecBrighter1.16b
		_neon64_fast_check 1, 9, \FastType, vecDarker1.16b, vecBrighter1.16b
		_neon64_fast_check 5, 13, \FastType, vecDarker1.16b, vecBrighter1.16b
		_neon64_fast_check 2, 10, \FastType, vecDarker1.16b, vecBrighter1.16b
		_neon64_fast_check 6, 14, \FastType, vecDarker1.16b, vecBrighter1.16b
		_neon64_fast_check 3, 11, \FastType, vecDarker1.16b, vecBrighter1.16b
		_neon64_fast_check 7, 15, \FastType, vecDarker1.16b, vecBrighter1.16b

		#######################################
		# Darkers
		#######################################
			mov vecSum1.16b, vecZero.16b
			_neon64_fast_load 0, 8, 4, 12, darkers, vecDarker1.16b, vecZero.16b, vecOne.16b,  vecSum1.16b
            cmhs v14.16b, vecSum1.16b, vecMinSum.16b
            mov r10, v14.d[0]
            mov r11, v14.d[1]
            orr r10, r10, r11 // orrs not avail on Aarch64
            tst r10, r10
			beq EndOfDarkers_CompVFastDataRow_Asm_NEON64\@
			_neon64_fast_load 1, 9, 5, 13, darkers, vecDarker1.16b, vecZero.16b, vecOne.16b,  vecSum1.16b
			_neon64_fast_load 2, 10, 6, 14, darkers, vecDarker1.16b, vecZero.16b, vecOne.16b,  vecSum1.16b
			_neon64_fast_load 3, 11, 7, 15, darkers, vecDarker1.16b, vecZero.16b, vecOne.16b,  vecSum1.16b
			cmhs v14.16b, vecSum1.16b, vecN.16b
            mov r10, v14.d[0]
            mov r11, v14.d[1]
            orr r10, r10, r11 // orrs not avail on Aarch64
            tst r10, r10
			beq EndOfDarkers_CompVFastDataRow_Asm_NEON64\@

			_neon64_fast_init_diffbinarysum \FastType, vecSum1.16b
			.irp ii, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
				_neon64_fast_strength \ii, \FastType,  vecSum1.16b, vecStrengths.16b, vecN.16b
			.endr
			EndOfDarkers_CompVFastDataRow_Asm_NEON64\@:

		#######################################
		# Brighters
		#######################################
			mov vecSum1.16b, vecZero.16b
			_neon64_fast_load 0, 8, 4, 12, brighters, vecBrighter1.16b, vecZero.16b, vecOne.16b,  vecSum1.16b
			cmhs v14.16b, vecSum1.16b, vecMinSum.16b
            mov r10, v14.d[0]
            mov r11, v14.d[1]
            orr r10, r10, r11 // orrs not avail on Aarch64
            tst r10, r10
			beq EndOfBrighters_CompVFastDataRow_Asm_NEON64\@
			_neon64_fast_load 1, 9, 5, 13, brighters, vecBrighter1.16b, vecZero.16b, vecOne.16b,  vecSum1.16b
			_neon64_fast_load 2, 10, 6, 14, brighters, vecBrighter1.16b, vecZero.16b, vecOne.16b,  vecSum1.16b
			_neon64_fast_load 3, 11, 7, 15, brighters, vecBrighter1.16b, vecZero.16b, vecOne.16b,  vecSum1.16b
			cmhs v14.16b, vecSum1.16b, vecN.16b
            mov r10, v14.d[0]
            mov r11, v14.d[1]
            orr r10, r10, r11 // orrs not avail on Aarch64
            tst r10, r10
			beq EndOfBrighters_CompVFastDataRow_Asm_NEON64\@

			_neon64_fast_init_diffbinarysum \FastType, vecSum1.16b
			.irp ii, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
				_neon64_fast_strength \ii, \FastType,  vecSum1.16b, vecStrengths.16b, vecN.16b
			.endr
			EndOfBrighters_CompVFastDataRow_Asm_NEON64\@:


		Next_CompVFastDataRow_Asm_NEON64\FastType:
		st1 {vecStrengths.16b}, [strengths], #16

		add i, i, #16
		cmp i, width
		blt LoopWidth_CompVFastDataRow_Asm_NEON64\@
		#End_of_LoopWidth#
	
	.unreq Iptr
	.unreq width
	.unreq pixels16
	.unreq N
	.unreq threshold
	.unreq strengths
	.unreq i
	.unreq sixteen
	.unreq eightTimesSixteen

	#undef vecDarker1
	#undef vecBrighter1
	#undef vecThreshold
	#undef vecStrengths
	#undef vecZero
	#undef vecSum1
	#undef vecMinSum
	#undef vecN
	#undef vecOne

	COMPV_GAS_MEMFREE (COMPV_GAS_REG_SZ_BYTES*16) + (16*16) + (16*16) + (16*16)
	COMPV_GAS_UNALIGN_STACK r9, r10
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVFast9DataRow_Asm_NEON64
	CompVFastDataRow_Macro_NEON64 9

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVFast12DataRow_Asm_NEON64
	CompVFastDataRow_Macro_NEON64 12

#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) uint8_t* pcStrengthsMap
# arg(1) -> COMPV_ALIGNED(NEON) uint8_t* pNMS
# arg(2) -> compv_uscalar_t width
# arg(3) -> compv_uscalar_t heigth
# arg(4) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVFastNmsApply_Asm_NEON64
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

	nms .req r5
	i .req r6
	#define vec0 v0	
	#define vecZero v1

	lsl r28, stride, #1
	mov r27, #0
	add r28, r28, stride // r28 = stride * 3
	dup vecZero.16b, r27w
	sub heigth, heigth, #6 // [j starts at #3 and end at heigth - #3] -> loop executed (heigth - #6) times
	add pcStrengthsMap, pcStrengthsMap, r28
	add pNMS, pNMS, r28

	#######################################
	# for (j = 3; j < heigth - 3; ++j)
	#######################################
	LoopHeight_CompVFastNmsApply_Asm_NEON64:
		#######################################
		# for (i = 0; i < width; i += 16)
		#######################################
		mov i, #0
		mov nms, pNMS
		LoopWidth_CompVFastNmsApply_Asm_NEON64:
			prfm pldl1keep, [nms, #(CACHE_LINE_SIZE*3)]
			ld1 { vec0.16b }, [nms], #16
			cmhi vec0.16b, vec0.16b, vecZero.16b
			mov r27, vec0.d[0]
            mov r28, vec0.d[1]
			orr r27, r27, r28 // orrs not avail on Aarch64
            tst r27, r27
			beq AllZeros_CompVFastNmsApply_Asm_NEON64
			add r11, pcStrengthsMap, i
			add r10, pNMS, i
			ld1 { v15.16b }, [r11]
			st1 { vecZero.16b }, [r10]
			bic v15.16b, v15.16b, vec0.16b
			st1 { v15.16b }, [r11]
			AllZeros_CompVFastNmsApply_Asm_NEON64:

			add i, i, #16
			cmp i, width
			blt LoopWidth_CompVFastNmsApply_Asm_NEON64
			#End_of_LoopWidth#

		add pcStrengthsMap, pcStrengthsMap, stride
		add pNMS, pNMS, stride
		subs heigth, heigth, #1
		bne LoopHeight_CompVFastNmsApply_Asm_NEON64
		#End_of_LoopHeight#

	.unreq pcStrengthsMap
	.unreq pNMS
	.unreq width
	.unreq heigth
	.unreq stride
	.unreq nms
	.unreq i
	#undef vec0
	#undef vecZero

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

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
		LoopWidth_CompVFastNmsGather_Asm_NEON64:
			prfm pldl1keep, [strengths, #(CACHE_LINE_SIZE*3)]
			ld1 { vecStrength.16b }, [strengths], #16                     // indexOf(strengths) = i + 16
			cmhi vec1.16b, vecStrength.16b, vecZero.16b
            mov r27, vec1.d[0]
            mov r28, vec1.d[1]
			orr r27, r27, r28 // orrs not avail on Aarch64
            tst r27, r27
			beq AllZeros_CompVFastNmsGather_Asm_NEON64
			add r11, strengths, minusStrideMinusSeventeen             
            add r10, pNMS, i
			ld1 { v9.16b }, [r11], one                              // r11 = i - stride - 1
			ld1 { v10.16b }, [r11], one                               // r11 = i - stride
			ld1 { v11.16b }, [r11], stride                            // r11 = i - stride + 1
			ld1 { v12.16b }, [r11], minusTwo                          // r11 = i + 1
			ld1 { v13.16b }, [r11], stride                            // r11 = i - 1
			ld1 { v14.16b }, [r11], one                               // r11 = i + stride - 1
			ld1 { v15.16b }, [r11], one                               // r11 = i + stride
			ld1 { v16.16b }, [r11]                                     // r11 = i + stride + 1
			cmhs v17.16b, v9.16b, vecStrength.16b
			cmhs v18.16b, v10.16b, vecStrength.16b
			cmhs v19.16b, v11.16b, vecStrength.16b
			cmhs v20.16b, v12.16b, vecStrength.16b
			cmhs v21.16b, v13.16b, vecStrength.16b
			cmhs v22.16b, v14.16b, vecStrength.16b
			cmhs v23.16b, v15.16b, vecStrength.16b
			cmhs v24.16b, v16.16b, vecStrength.16b
            orr v9.16b, v17.16b, v18.16b
            orr v10.16b, v19.16b, v20.16b
            orr v11.16b, v21.16b, v22.16b
            orr v12.16b, v23.16b, v24.16b
            orr v17.16b, v9.16b, v10.16b
            orr v18.16b, v11.16b, v12.16b
            orr v17.16b, v17.16b, v18.16b
			and v17.16b, v17.16b, vec1.16b
            st1 { v17.16b }, [r10]
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
	#undef vec0
	#undef vec1
	#undef vecStrength
	#undef vecZero

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__aarch64__) */

#########################################################################
# Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>   #
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
	COMPV_GAS_MEMALLOC (COMPV_GAS_REG_SZ_BYTES*16) + (COMPV_GAS_V_SZ_BYTES*16) + (COMPV_GAS_V_SZ_BYTES*16) + (COMPV_GAS_V_SZ_BYTES*16)
	
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
	.equ vecDiff16				, (vecDiffBinary16 + (COMPV_GAS_V_SZ_BYTES*16))
	.equ vecCircle16			, (vecDiff16 + (COMPV_GAS_V_SZ_BYTES*16))

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
	
	movi vecZero.16b, #0
	.if  \FastType == 9
		movi vecN.16b, #9
		movi vecMinSum.16b, #2
	.else
		movi vecN.16b, #12
		movi vecMinSum.16b, #3
	.endif
	movi vecOne.16b, #1
	
	dup vecThreshold.16b, r4w
	# threshold(r4) not longer needed -> use it as sixteen
	sixteen .req threshold
    .unreq threshold
	mov sixteen, #16

	## Load circle ##
    add r25, sp, #circle
    ldp r7, r8, [pixels16], #(2*COMPV_GAS_REG_SZ_BYTES)
    ldp r9, r10, [pixels16], #(2*COMPV_GAS_REG_SZ_BYTES)
    ldp r11, r12, [pixels16], #(2*COMPV_GAS_REG_SZ_BYTES)
    ldp r13, r14, [pixels16], #(2*COMPV_GAS_REG_SZ_BYTES)
    ldp r15, r16, [pixels16], #(2*COMPV_GAS_REG_SZ_BYTES)
    ldp r19, r20, [pixels16], #(2*COMPV_GAS_REG_SZ_BYTES)
    ldp r21, r22, [pixels16], #(2*COMPV_GAS_REG_SZ_BYTES)
    ldp r23, r24, [pixels16], #(2*COMPV_GAS_REG_SZ_BYTES)
    add r7, Iptr, r7
    add r8, Iptr, r8
    add r9, Iptr, r9
    add r10, Iptr, r10
    add r11, Iptr, r11
    add r12, Iptr, r12
    add r13, Iptr, r13
    add r14, Iptr, r14
    add r15, Iptr, r15
    add r16, Iptr, r16
    add r19, Iptr, r19
    add r20, Iptr, r20
    add r21, Iptr, r21
    add r22, Iptr, r22
    add r23, Iptr, r23
    add r24, Iptr, r24
    stp r7, r8, [r25], #(2*COMPV_GAS_REG_SZ_BYTES)
    stp r9, r10, [r25], #(2*COMPV_GAS_REG_SZ_BYTES)
    stp r11, r12, [r25], #(2*COMPV_GAS_REG_SZ_BYTES)
    stp r13, r14, [r25], #(2*COMPV_GAS_REG_SZ_BYTES)
    stp r15, r16, [r25], #(2*COMPV_GAS_REG_SZ_BYTES)
    stp r19, r20, [r25], #(2*COMPV_GAS_REG_SZ_BYTES)
    stp r21, r22, [r25], #(2*COMPV_GAS_REG_SZ_BYTES)
    stp r23, r24, [r25], #(2*COMPV_GAS_REG_SZ_BYTES)
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
        ldr r10, [sp, #(circle + 0*COMPV_GAS_REG_SZ_BYTES)]
        ldr r11, [sp, #(circle + 8*COMPV_GAS_REG_SZ_BYTES)]
        ldr q12, [r10, i]
        ldr q20, [r11, i]
        cmhi v19.16b, vecDarker1.16b, v12.16b
        cmhi v27.16b, vecDarker1.16b, v20.16b
        cmhi v28.16b, v12.16b, vecBrighter1.16b
        cmhi v29.16b, v20.16b, vecBrighter1.16b
        orr v19.16b, v19.16b, v27.16b
        orr v27.16b, v28.16b, v29.16b
        orr v28.16b, v19.16b, v27.16b
        mov r10, v28.d[0]
        mov r11, v28.d[1]
        orr r10, r10, r11 // orrs not avail on Aarch64
        cbz r10, Next_CompVFastDataRow_Asm_NEON64\FastType    
        ldr r10, [sp, #(circle + 4*COMPV_GAS_REG_SZ_BYTES)]
        ldr r11, [sp, #(circle + 12*COMPV_GAS_REG_SZ_BYTES)]
        ldr q16, [r10, i]
        ldr q24, [r11, i]
        cmhi v19.16b, vecDarker1.16b, v16.16b
        cmhi v27.16b, vecDarker1.16b, v24.16b
        cmhi v28.16b, v16.16b, vecBrighter1.16b
        cmhi v29.16b, v24.16b, vecBrighter1.16b
        orr v19.16b, v19.16b, v27.16b
        orr v27.16b, v28.16b, v29.16b
        orr v28.16b, v19.16b, v27.16b
        mov r10, v28.d[0]
        mov r11, v28.d[1]
        orr r10, r10, r11 // orrs not avail on Aarch64
        cbz r10, Next_CompVFastDataRow_Asm_NEON64\FastType        
        ldr r10, [sp, #(circle + 1*COMPV_GAS_REG_SZ_BYTES)]
        ldr r11, [sp, #(circle + 9*COMPV_GAS_REG_SZ_BYTES)]
        ldr q13, [r10, i]
        ldr q21, [r11, i]
        cmhi v19.16b, vecDarker1.16b, v13.16b
        cmhi v27.16b, vecDarker1.16b, v21.16b
        cmhi v28.16b, v13.16b, vecBrighter1.16b
        cmhi v29.16b, v21.16b, vecBrighter1.16b
        orr v19.16b, v19.16b, v27.16b
        orr v27.16b, v28.16b, v29.16b
        orr v28.16b, v19.16b, v27.16b
        mov r10, v28.d[0]
        mov r11, v28.d[1]
        orr r10, r10, r11 // orrs not avail on Aarch64
        cbz r10, Next_CompVFastDataRow_Asm_NEON64\FastType        
        ldr r10, [sp, #(circle + 5*COMPV_GAS_REG_SZ_BYTES)]
        ldr r11, [sp, #(circle + 13*COMPV_GAS_REG_SZ_BYTES)]
        ldr q17, [r10, i]
        ldr q25, [r11, i]
        cmhi v19.16b, vecDarker1.16b, v17.16b
        cmhi v27.16b, vecDarker1.16b, v25.16b
        cmhi v28.16b, v17.16b, vecBrighter1.16b
        cmhi v29.16b, v25.16b, vecBrighter1.16b
        orr v19.16b, v19.16b, v27.16b
        orr v27.16b, v28.16b, v29.16b
        orr v28.16b, v19.16b, v27.16b
        mov r10, v28.d[0]
        mov r11, v28.d[1]
        orr r10, r10, r11 // orrs not avail on Aarch64
        cbz r10, Next_CompVFastDataRow_Asm_NEON64\FastType
        ldr r10, [sp, #(circle + 2*COMPV_GAS_REG_SZ_BYTES)]
        ldr r11, [sp, #(circle + 10*COMPV_GAS_REG_SZ_BYTES)]
        ldr q14, [r10, i]
        ldr q22, [r11, i]
        cmhi v19.16b, vecDarker1.16b, v14.16b
        cmhi v27.16b, vecDarker1.16b, v22.16b
        cmhi v28.16b, v14.16b, vecBrighter1.16b
        cmhi v29.16b, v22.16b, vecBrighter1.16b
        orr v19.16b, v19.16b, v27.16b
        orr v27.16b, v28.16b, v29.16b
        orr v28.16b, v19.16b, v27.16b
        mov r10, v28.d[0]
        mov r11, v28.d[1]
        orr r10, r10, r11 // orrs not avail on Aarch64
        cbz r10, Next_CompVFastDataRow_Asm_NEON64\FastType   
        ldr r10, [sp, #(circle + 6*COMPV_GAS_REG_SZ_BYTES)]
        ldr r11, [sp, #(circle + 14*COMPV_GAS_REG_SZ_BYTES)]
        ldr q18, [r10, i]
        ldr q26, [r11, i]
        cmhi v19.16b, vecDarker1.16b, v18.16b
        cmhi v27.16b, vecDarker1.16b, v26.16b
        cmhi v28.16b, v18.16b, vecBrighter1.16b
        cmhi v29.16b, v26.16b, vecBrighter1.16b
        orr v19.16b, v19.16b, v27.16b
        orr v27.16b, v28.16b, v29.16b
        orr v28.16b, v19.16b, v27.16b
        mov r10, v28.d[0]
        mov r11, v28.d[1]
        orr r10, r10, r11 // orrs not avail on Aarch64
        cbz r10, Next_CompVFastDataRow_Asm_NEON64\FastType  
        ldr r10, [sp, #(circle + 3*COMPV_GAS_REG_SZ_BYTES)]
        ldr r11, [sp, #(circle + 11*COMPV_GAS_REG_SZ_BYTES)]
        ldr q15, [r10, i]
        ldr q23, [r11, i]
        cmhi v19.16b, vecDarker1.16b, v15.16b
        cmhi v27.16b, vecDarker1.16b, v23.16b
        cmhi v28.16b, v15.16b, vecBrighter1.16b
        cmhi v29.16b, v23.16b, vecBrighter1.16b
        orr v19.16b, v19.16b, v27.16b
        orr v27.16b, v28.16b, v29.16b
        orr v28.16b, v19.16b, v27.16b
        mov r10, v28.d[0]
        mov r11, v28.d[1]
        orr r10, r10, r11 // orrs not avail on Aarch64
        cbz r10, Next_CompVFastDataRow_Asm_NEON64\FastType  
        ldr r10, [sp, #(circle + 7*COMPV_GAS_REG_SZ_BYTES)]
        ldr r11, [sp, #(circle + 15*COMPV_GAS_REG_SZ_BYTES)]
        ldr q19, [r10, i]
        ldr q27, [r11, i]
        cmhi v28.16b, vecDarker1.16b, v19.16b
        cmhi v29.16b, vecDarker1.16b, v27.16b
        orr v28.16b, v28.16b, v29.16b
        cmhi v29.16b, v19.16b, vecBrighter1.16b
        orr v28.16b, v28.16b, v29.16b
        cmhi v29.16b, v27.16b, vecBrighter1.16b
        orr v28.16b, v28.16b, v29.16b
        mov r10, v28.d[0]
        mov r11, v28.d[1]
        orr r10, r10, r11 // orrs not avail on Aarch64
        cbz r10, Next_CompVFastDataRow_Asm_NEON64\FastType
		stp q12, q13, [sp, #(vecCircle16 + 0*16)]
		stp q14, q15, [sp, #(vecCircle16 + 2*16)]
		stp q16, q17, [sp, #(vecCircle16 + 4*16)]
		stp q18, q19, [sp, #(vecCircle16 + 6*16)]
		stp q20, q21, [sp, #(vecCircle16 + 8*16)]
		stp q22, q23, [sp, #(vecCircle16 + 10*16)]
		stp q24, q25, [sp, #(vecCircle16 + 12*16)]
		stp q26, q27, [sp, #(vecCircle16 + 14*16)]


		#######################################
		# Darkers
		#######################################
			uqsub v18.16b, vecDarker1.16b, v16.16b
			uqsub v16.16b, vecDarker1.16b, v12.16b
			uqsub v17.16b, vecDarker1.16b, v20.16b
			uqsub v19.16b, vecDarker1.16b, v24.16b
			cmhi v12.16b, v16.16b, vecZero.16b
			cmhi v13.16b, v17.16b, vecZero.16b
			cmhi v14.16b, v18.16b, vecZero.16b
			cmhi v15.16b, v19.16b, vecZero.16b
			and v20.16b, v12.16b, vecOne.16b
			and v21.16b, v13.16b, vecOne.16b
			and v22.16b, v14.16b, vecOne.16b
			and v23.16b, v15.16b, vecOne.16b
			add v24.16b, v20.16b, v21.16b
			add vecSum1.16b, v22.16b, v23.16b // setting vecSum1 for the first time
			add vecSum1.16b, vecSum1.16b, v24.16b // updating vecSum1
            cmhs v14.16b, vecSum1.16b, vecMinSum.16b
            mov r10, v14.d[0]
            mov r11, v14.d[1]
            orr r10, r10, r11 // orrs not avail on Aarch64
			cbz r10, EndOfDarkers_CompVFastDataRow_Asm_NEON64\@
			str q16, [sp, #(vecDiff16 + 0*16)]
			str q17, [sp, #(vecDiff16 + 8*16)]
			str q18, [sp, #(vecDiff16 + 4*16)]
			str q19, [sp, #(vecDiff16 + 12*16)]
			str q20, [sp, #(vecDiffBinary16 + 0*16)]
			str q21, [sp, #(vecDiffBinary16 + 8*16)]
			str q22, [sp, #(vecDiffBinary16 + 4*16)]
			str q23, [sp, #(vecDiffBinary16 + 12*16)]
            ldp q12, q16, [sp, #(vecCircle16 + 1*16)]
            ldr q20, [sp, #(vecCircle16 + 3*16)]
            ldp q14, q18, [sp, #(vecCircle16 + 5*16)]
            ldr q22, [sp, #(vecCircle16 + 7*16)]
            ldp q13, q17, [sp, #(vecCircle16 + 9*16)]
            ldr q21, [sp, #(vecCircle16 + 11*16)]
            ldp q15, q19, [sp, #(vecCircle16 + 13*16)]
            ldr q23, [sp, #(vecCircle16 + 15*16)]
            uqsub v12.16b, vecDarker1.16b, v12.16b
			uqsub v16.16b, vecDarker1.16b, v16.16b
			uqsub v20.16b, vecDarker1.16b, v20.16b
			uqsub v14.16b, vecDarker1.16b, v14.16b
			uqsub v18.16b, vecDarker1.16b, v18.16b
			uqsub v22.16b, vecDarker1.16b, v22.16b
            uqsub v13.16b, vecDarker1.16b, v13.16b
            uqsub v17.16b, vecDarker1.16b, v17.16b
			uqsub v21.16b, vecDarker1.16b, v21.16b
            uqsub v15.16b, vecDarker1.16b, v15.16b
            uqsub v19.16b, vecDarker1.16b, v19.16b
            uqsub v23.16b, vecDarker1.16b, v23.16b
            stp q12, q16, [sp, #(vecDiff16 + 1*16)]
			str q20, [sp, #(vecDiff16 + 3*16)]
            stp q14, q18, [sp, #(vecDiff16 + 5*16)]
			str q22, [sp, #(vecDiff16 + 7*16)]
			stp q13, q17, [sp, #(vecDiff16 + 9*16)]
			str q21, [sp, #(vecDiff16 + 11*16)]
            stp q15, q19, [sp, #(vecDiff16 + 13*16)]
            str q23, [sp, #(vecDiff16 + 15*16)]
            cmhi v12.16b, v12.16b, vecZero.16b
			cmhi v16.16b, v16.16b, vecZero.16b
			cmhi v20.16b, v20.16b, vecZero.16b
			cmhi v14.16b, v14.16b, vecZero.16b
			cmhi v18.16b, v18.16b, vecZero.16b
			cmhi v22.16b, v22.16b, vecZero.16b
            cmhi v13.16b, v13.16b, vecZero.16b
            cmhi v17.16b, v17.16b, vecZero.16b
			cmhi v21.16b, v21.16b, vecZero.16b
            cmhi v15.16b, v15.16b, vecZero.16b
            cmhi v19.16b, v19.16b, vecZero.16b
            cmhi v23.16b, v23.16b, vecZero.16b
            and v12.16b, v12.16b, vecOne.16b
			and v16.16b, v16.16b, vecOne.16b
			and v20.16b, v20.16b, vecOne.16b
			and v14.16b, v14.16b, vecOne.16b
			and v18.16b, v18.16b, vecOne.16b
			and v22.16b, v22.16b, vecOne.16b
            and v13.16b, v13.16b, vecOne.16b
            and v17.16b, v17.16b, vecOne.16b
			and v21.16b, v21.16b, vecOne.16b
            and v15.16b, v15.16b, vecOne.16b        
            and v19.16b, v19.16b, vecOne.16b
            and v23.16b, v23.16b, vecOne.16b
            add v24.16b, v12.16b, v16.16b
            add v25.16b, v20.16b, v14.16b
            add v26.16b, v18.16b, v22.16b
            add v27.16b, v13.16b, v17.16b
            add v28.16b, v21.16b, v15.16b
            add v29.16b, v19.16b, v23.16b
            add v24.16b, v24.16b, v25.16b
            add v25.16b, v26.16b, v27.16b
            add v26.16b, v28.16b, v29.16b
            add v27.16b, v24.16b, v25.16b
            add vecSum1.16b, vecSum1.16b, v26.16b
            add vecSum1.16b, vecSum1.16b, v27.16b
			cmhs v24.16b, vecSum1.16b, vecN.16b
            mov r20, v24.d[0]
            mov r21, v24.d[1]
            orr r20, r20, r21 // orrs not avail on Aarch64
			cbz r20, EndOfDarkers_CompVFastDataRow_Asm_NEON64\@
            stp q12, q16, [sp, #(vecDiffBinary16 + 1*16)]
			str q20, [sp, #(vecDiffBinary16 + 3*16)]
            stp q14, q18, [sp, #(vecDiffBinary16 + 5*16)]
			str q22, [sp, #(vecDiffBinary16 + 7*16)]
			stp q13, q17, [sp, #(vecDiffBinary16 + 9*16)]
			str q21, [sp, #(vecDiffBinary16 + 11*16)]
            stp q15, q19, [sp, #(vecDiffBinary16 + 13*16)]
            str q23, [sp, #(vecDiffBinary16 + 15*16)]
			_neon64_fast_init_diffbinarysum \FastType, vecSum1.16b
			.irp ii, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
				_neon64_fast_strength \ii, \FastType,  vecSum1.16b, vecStrengths.16b, vecN.16b
			.endr
			EndOfDarkers_CompVFastDataRow_Asm_NEON64\@:
			
		#######################################
		# Brighters
		#######################################
			ldr q12, [sp, #(vecCircle16 + 0*16)]
			ldr q13, [sp, #(vecCircle16 + 8*16)]
			ldr q14, [sp, #(vecCircle16 + 4*16)]
			ldr q15, [sp, #(vecCircle16 + 12*16)]
			uqsub v16.16b, v12.16b, vecBrighter1.16b
			uqsub v17.16b, v13.16b, vecBrighter1.16b
			uqsub v18.16b, v14.16b, vecBrighter1.16b
			uqsub v19.16b, v15.16b, vecBrighter1.16b
			cmhi v12.16b, v16.16b, vecZero.16b
			cmhi v13.16b, v17.16b, vecZero.16b
			cmhi v14.16b, v18.16b, vecZero.16b
			cmhi v15.16b, v19.16b, vecZero.16b
			and v20.16b, v12.16b, vecOne.16b
			and v21.16b, v13.16b, vecOne.16b
			and v22.16b, v14.16b, vecOne.16b
			and v23.16b, v15.16b, vecOne.16b
			add v24.16b, v20.16b, v21.16b
			add vecSum1.16b, v22.16b, v23.16b // setting vecSum1 for the first time
			add vecSum1.16b, vecSum1.16b, v24.16b // updating vecSum1
			cmhs v14.16b, vecSum1.16b, vecMinSum.16b
            mov r10, v14.d[0]
            mov r11, v14.d[1]
            orr r10, r10, r11 // orrs not avail on Aarch64
			cbz r10, EndOfBrighters_CompVFastDataRow_Asm_NEON64\@
			str q16, [sp, #(vecDiff16 + 0*16)]
			str q17, [sp, #(vecDiff16 + 8*16)]
			str q18, [sp, #(vecDiff16 + 4*16)]
			str q19, [sp, #(vecDiff16 + 12*16)]
			str q20, [sp, #(vecDiffBinary16 + 0*16)]
			str q21, [sp, #(vecDiffBinary16 + 8*16)]
			str q22, [sp, #(vecDiffBinary16 + 4*16)]
			str q23, [sp, #(vecDiffBinary16 + 12*16)]
            ldp q12, q16, [sp, #(vecCircle16 + 1*16)]
			ldr q20, [sp, #(vecCircle16 + 3*16)]
			ldp q14, q18, [sp, #(vecCircle16 + 5*16)]
			ldr q22, [sp, #(vecCircle16 + 7*16)]
            ldp q13, q17, [sp, #(vecCircle16 + 9*16)]
            ldr q21, [sp, #(vecCircle16 + 11*16)]
            ldp q15, q19, [sp, #(vecCircle16 + 13*16)]
            ldr q23, [sp, #(vecCircle16 + 15*16)]
            uqsub v12.16b, v12.16b, vecBrighter1.16b
			uqsub v16.16b, v16.16b, vecBrighter1.16b
			uqsub v20.16b, v20.16b, vecBrighter1.16b
			uqsub v14.16b, v14.16b, vecBrighter1.16b
			uqsub v18.16b, v18.16b, vecBrighter1.16b
			uqsub v22.16b, v22.16b, vecBrighter1.16b
            uqsub v13.16b, v13.16b, vecBrighter1.16b
            uqsub v17.16b, v17.16b, vecBrighter1.16b
			uqsub v21.16b, v21.16b, vecBrighter1.16b
            uqsub v15.16b, v15.16b, vecBrighter1.16b
            uqsub v19.16b, v19.16b, vecBrighter1.16b
            uqsub v23.16b, v23.16b, vecBrighter1.16b
            stp q12, q16, [sp, #(vecDiff16 + 1*16)]
			str q20, [sp, #(vecDiff16 + 3*16)]
            stp q14, q18, [sp, #(vecDiff16 + 5*16)]
			str q22, [sp, #(vecDiff16 + 7*16)]
			stp q13, q17, [sp, #(vecDiff16 + 9*16)]
			str q21, [sp, #(vecDiff16 + 11*16)]
            stp q15, q19, [sp, #(vecDiff16 + 13*16)]
            str q23, [sp, #(vecDiff16 + 15*16)]
            cmhi v12.16b, v12.16b, vecZero.16b
			cmhi v16.16b, v16.16b, vecZero.16b
			cmhi v20.16b, v20.16b, vecZero.16b
			cmhi v14.16b, v14.16b, vecZero.16b
			cmhi v18.16b, v18.16b, vecZero.16b
			cmhi v22.16b, v22.16b, vecZero.16b
            cmhi v13.16b, v13.16b, vecZero.16b
            cmhi v17.16b, v17.16b, vecZero.16b
			cmhi v21.16b, v21.16b, vecZero.16b
            cmhi v15.16b, v15.16b, vecZero.16b
            cmhi v19.16b, v19.16b, vecZero.16b
            cmhi v23.16b, v23.16b, vecZero.16b
            and v12.16b, v12.16b, vecOne.16b
			and v16.16b, v16.16b, vecOne.16b
			and v20.16b, v20.16b, vecOne.16b
			and v14.16b, v14.16b, vecOne.16b
			and v18.16b, v18.16b, vecOne.16b
			and v22.16b, v22.16b, vecOne.16b
            and v13.16b, v13.16b, vecOne.16b
            and v17.16b, v17.16b, vecOne.16b
			and v21.16b, v21.16b, vecOne.16b
            and v15.16b, v15.16b, vecOne.16b
            and v19.16b, v19.16b, vecOne.16b
            and v23.16b, v23.16b, vecOne.16b
            add v24.16b, v12.16b, v16.16b
            add v25.16b, v20.16b, v14.16b
            add v26.16b, v18.16b, v22.16b
            add v27.16b, v13.16b, v17.16b
            add v28.16b, v21.16b, v15.16b
            add v29.16b, v19.16b, v23.16b
            add v24.16b, v24.16b, v25.16b
            add v25.16b, v26.16b, v27.16b
            add v26.16b, v28.16b, v29.16b
            add v27.16b, v24.16b, v25.16b
            add vecSum1.16b, vecSum1.16b, v26.16b
            add vecSum1.16b, vecSum1.16b, v27.16b
			cmhs v24.16b, vecSum1.16b, vecN.16b
            mov r20, v24.d[0]
            mov r21, v24.d[1]
            orr r20, r20, r21 // orrs not avail on Aarch64
			cbz r20, EndOfBrighters_CompVFastDataRow_Asm_NEON64\@
            stp q12, q16, [sp, #(vecDiffBinary16 + 1*16)]
			str q20, [sp, #(vecDiffBinary16 + 3*16)]
            stp q14, q18, [sp, #(vecDiffBinary16 + 5*16)]
			str q22, [sp, #(vecDiffBinary16 + 7*16)]
			stp q13, q17, [sp, #(vecDiffBinary16 + 9*16)]
			str q21, [sp, #(vecDiffBinary16 + 11*16)]
            stp q15, q19, [sp, #(vecDiffBinary16 + 13*16)]
            str q23, [sp, #(vecDiffBinary16 + 15*16)]
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
	.unreq N
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

	COMPV_GAS_MEMFREE (COMPV_GAS_REG_SZ_BYTES*16) + (COMPV_GAS_V_SZ_BYTES*16) + (COMPV_GAS_V_SZ_BYTES*16) + (COMPV_GAS_V_SZ_BYTES*16)
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
			cbz r27, AllZeros_CompVFastNmsApply_Asm_NEON64
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
	#define vec0 v0
	#define vec1 v1
	#define vecStrength v2
	#define vecZero v3

	mov minusStrideMinusSeventeen, #-17
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
			cbz r27, AllZeros_CompVFastNmsGather_Asm_NEON64
			add r11, strengths, minusStrideMinusSeventeen             
            add r10, pNMS, i
			ldr q9, [r11], #1                            // r11 = i - stride - 1
			ldr q10, [r11], #1                           // r11 = i - stride
			ld1 { v11.16b }, [r11], stride               // r11 = i - stride + 1
			ldr q12, [r11], #-2                          // r11 = i + 1
			ld1 { v13.16b }, [r11], stride               // r11 = i - 1
			ldr q14, [r11], #1                           // r11 = i + stride - 1
			ldr q15, [r11], #1                           // r11 = i + stride
			ld1 {v16.16b}, [r11]						 // r11 = i + stride + 1
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
	#undef vec0
	#undef vec1
	#undef vecStrength
	#undef vecZero

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__aarch64__) */

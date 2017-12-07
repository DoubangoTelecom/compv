#########################################################################
# Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__aarch64__)
.include "compv_common_arm64.S"

#if defined(__APPLE__)
#   define sym(funcname) _##funcname
#else
#   define sym(funcname) funcname
#endif

.data

.extern

.text

#########################################################################
# arg(0) -> uint8_t* nms
# arg(1) -> const uint16_t* g
# arg(2) -> const int16_t* gx
# arg(3) -> const int16_t* gy
# arg(4) -> const uint16_t* tLow1
# arg(5) -> compv_uscalar_t width
# arg(6) -> compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVCannyNMSGatherRow_8mpw_Asm_NEON64
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Set arguments ##
	nms .req r0
	g .req r1
	gx .req r2
	gy .req r3
	tLow1 .req r4
	width .req r5
	stride .req r6

    col .req r7
    cZero .req r8
    gTmp .req r9

    vecNMS .req v8
	vecG .req v9
	vecGX .req v10
	vecAbsGX .req v11
	vecTLow .req v12
	vecGY .req v13
	vecAbsGY0 .req v14
	vecAbsGY1 .req v15
    vecTangentPiOver8Int .req v16
    vecTangentPiTimes3Over8Int .req v17
	
    movz r8w, #0x6a09
    movz r9w, #0x2, lsl #16
    movk r9w, #0x6a09
    ldrh r10w, [tLow1]

    dup vecTangentPiOver8Int.8h, r8w
    dup vecTangentPiTimes3Over8Int.4s, r9w
    dup vecTLow.8h, r10w

    .unreq tLow1 // tLow no longer needed
    cOne .req r4
    lsl stride, stride, #1 // from bytes to shorts
    add cOne, stride, #(1*COMPV_GAS_UINT16_SZ_BYTES)
    mov cZero, #(1*COMPV_GAS_UINT16_SZ_BYTES)
    sub cZero, cZero, stride

    lsl width, width, #1 // from bytes to shorts
    sub width, width, #(7*COMPV_GAS_UINT16_SZ_BYTES)

    mov col, #(1*COMPV_GAS_UINT16_SZ_BYTES)
    add gTmp, g, #(1*COMPV_GAS_UINT16_SZ_BYTES) // col starts at 1

    #########################################################################
    # for (col = 1; col < width - 7; col += 8)
    #########################################################################
    LoopWidth_CompVCannyNMSGatherRow_8mpw_Asm_NEON64:
        ld1 {vecG.8h}, [gTmp], #(1*COMPV_GAS_Q_SZ_BYTES)
        cmhi v0.8h, vecG.8h, vecTLow.8h
        mov r10, v0.d[0]
        mov r11, v0.d[1]
        orr r10, r10, r11
        #########################################################################
        # .if (COMPV_ARM_NEON_NEQ_ZERO(vec0))
        #########################################################################
        cbz r10, EndOf_Ifvec00_CompVCannyNMSGatherRow_8mpw_Asm_NEON64
		Ifvec00_CompVCannyNMSGatherRow_8mpw_Asm_NEON64:
            ldr q10, [gx, col] // vecGX = v10			
            abs vecAbsGX.8h, vecGX.8h
            umull v18.4s, vecTangentPiOver8Int.4h, vecAbsGX.4h
            umull2 v19.4s, vecTangentPiOver8Int.8h, vecAbsGX.8h
			ldr q13, [gy, col] // vecGY = v13
            abs v3.8h, vecGY.8h
            shll vecAbsGY0.4s, v3.4h, #16
            shll2 vecAbsGY1.4s, v3.8h, #16
            movi vecNMS.8b, #0   
            cmhi v18.4s, v18.4s, vecAbsGY0.4s
            cmhi v19.4s, v19.4s, vecAbsGY1.4s
            uqxtn v4.4h, v18.4s
            uqxtn2 v4.8h, v19.4s
            and v1.16b, v0.16b, v4.16b // q1 = vec3
            mov r10, v1.d[0]
            mov r11, v1.d[1]
            orr r10, r10, r11
			#########################################################################
			# .if (COMPV_ARM_NEON_NEQ_ZERO(vec3))
			#########################################################################
			cbz r10, EndOf_Ifvec30_CompVCannyNMSGatherRow_8mpw_Asm_NEON64
			Ifvec30_CompVCannyNMSGatherRow_8mpw_Asm_NEON64:
                add r10, g, col
                ldr q2, [r10, #-(1*COMPV_GAS_UINT16_SZ_BYTES)]
			    ldr q3, [r10, #(1*COMPV_GAS_UINT16_SZ_BYTES)]
				cmhi v2.8h, v2.8h, vecG.8h
                cmhi v3.8h, v3.8h, vecG.8h
                orr v2.16b, v2.16b, v3.16b
                and v2.16b, v2.16b, v1.16b // q1 is vec3
                uqxtn v2.8b, v2.8h
                orr vecNMS.8b, vecNMS.8b, v2.8b
				EndOf_Ifvec30_CompVCannyNMSGatherRow_8mpw_Asm_NEON64:
				## EndOf_Ifvec30_CompVCannyNMSGatherRow_8mpw_Asm_NEON64 ##

			## angle = "45° / 225°" or "135 / 315" ##
            bic v2.16b, v0.16b, v1.16b // q1 is vec3, now q2 = vec4
            mov r10, v2.d[0]
            mov r11, v2.d[1]
            orr r10, r10, r11
			#########################################################################
			# .if (COMPV_ARM_NEON_NEQ_ZERO(vec4)) - 0
			#########################################################################
			cbz r10, EndOf_Ifvec40_CompVCannyNMSGatherRow_8mpw_Asm_NEON64
			Ifvec40_CompVCannyNMSGatherRow_8mpw_Asm_NEON64:
                uxtl v6.4s, vecAbsGX.4h
                mul v6.4s, v6.4s, vecTangentPiTimes3Over8Int.4s
                uxtl2 v4.4s, vecAbsGX.8h
                mul v4.4s, v4.4s, vecTangentPiTimes3Over8Int.4s
                cmhi v6.4s, v6.4s, vecAbsGY0.4s
                cmhi v4.4s, v4.4s, vecAbsGY1.4s
                uqxtn v6.4h, v6.4s
                uqxtn2 v6.8h, v4.4s
                and v2.16b, v2.16b, v6.16b // q2 = old vec4, override
                mov r10, v2.d[0]
                mov r11, v2.d[1]
                orr r10, r10, r11
				#########################################################################
				# .if (COMPV_ARM_NEON_NEQ_ZERO(vec4)) - 1
				#########################################################################
				cbz r10, EndOf_Ifvec41_CompVCannyNMSGatherRow_8mpw_Asm_NEON64
				Ifvec41_CompVCannyNMSGatherRow_8mpw_Asm_NEON64:
                    eor v4.16b, vecGX.16b, vecGY.16b
                    cmlt v4.8h, v4.8h, #0
                    and v4.16b, v4.16b, v2.16b // q2 is vec4, q4 = vec1
                    bic v5.16b, v2.16b, v4.16b // q2 is vec4, q5 = vec2
                    mov r10, v4.d[0]
                    mov r11, v4.d[1]
                    orr r10, r10, r11
					#########################################################################
					# .if (COMPV_ARM_NEON_NEQ_ZERO(vec1))
					#########################################################################
					cbz r10, EndOf_Ifvec10_CompVCannyNMSGatherRow_8mpw_Asm_NEON64
					Ifvec10_CompVCannyNMSGatherRow_8mpw_Asm_NEON64:
                        sub r10, col, cZero
                        add r11, col, cZero
                        ldr q6, [g, r10]
						ldr q3, [g, r11]
                        cmhi v6.8h, v6.8h, vecG.8h
                        cmhi v3.8h, v3.8h, vecG.8h
                        orr v6.16b, v6.16b, v3.16b
                        and v4.16b, v4.16b, v6.16b // q4 is old vec1
						EndOf_Ifvec10_CompVCannyNMSGatherRow_8mpw_Asm_NEON64:
						## EndOf_Ifvec10_CompVCannyNMSGatherRow_8mpw_Asm_NEON64 ##


                    // q5 is vec2
                    mov r10, v5.d[0]
                    mov r11, v5.d[1]
                    orr r10, r10, r11
					#########################################################################
					# .if (COMPV_ARM_NEON_NEQ_ZERO(vec2))
					#########################################################################
					cbz r10, EndOfIfvec20_CompVCannyNMSGatherRow_8mpw_Asm_NEON64
					Ifvec20_CompVCannyNMSGatherRow_8mpw_Asm_NEON64:
                        sub r10, col, cOne
                        add r11, col, cOne
                        ldr q6, [g, r10]
						ldr q3, [g, r11]
                        cmhi v6.8h, v6.8h, vecG.8h
                        cmhi v3.8h, v3.8h, vecG.8h
                        orr v6.16b, v6.16b, v3.16b
                        and v5.16b, v5.16b, v6.16b // q5 is old vec2
						EndOfIfvec20_CompVCannyNMSGatherRow_8mpw_Asm_NEON64:
						## EndOfIfvec20_CompVCannyNMSGatherRow_8mpw_Asm_NEON64 ##


                    orr v4.16b, v4.16b, v5.16b // q4 is vec1 and q5 is vec2
                    uqxtn v4.8b, v4.8h
                    orr vecNMS.8b, vecNMS.8b, v4.8b
					EndOf_Ifvec41_CompVCannyNMSGatherRow_8mpw_Asm_NEON64:
					## EndOf_Ifvec41_CompVCannyNMSGatherRow_8mpw_Asm_NEON64 ##

				EndOf_Ifvec40_CompVCannyNMSGatherRow_8mpw_Asm_NEON64:
				## EndOf_Ifvec40_CompVCannyNMSGatherRow_8mpw_Asm_NEON64 ##
			
			## angle = "90° / 270°" ##
            bic v2.16b, v0.16b, v2.16b // q2 was vec4 and q0 is vec0
            bic v1.16b, v2.16b, v1.16b // q1 was vec3, now vec5 is q1
            mov r10, v1.d[0]
            mov r11, v1.d[1]
            orr r10, r10, r11
			#########################################################################
			# .if (COMPV_ARM_NEON_NEQ_ZERO(vec5))
			#########################################################################
			cbz r10, EndOf_Ifvec50_CompVCannyNMSGatherRow_8mpw_Asm_NEON64
			Ifvec50_CompVCannyNMSGatherRow_8mpw_Asm_NEON64:
                sub r10, col, stride
                add r11, col, stride
                ldr q6, [g, r10]
				ldr q3, [g, r11]
                cmhi v6.8h, v6.8h, vecG.8h
                cmhi v3.8h, v3.8h, vecG.8h
                orr v6.16b, v6.16b, v3.16b
                and v1.16b, v1.16b, v6.16b // q1 is old vec5
                uqxtn v1.8b, v1.8h
                orr vecNMS.8b, vecNMS.8b, v1.8b
				EndOf_Ifvec50_CompVCannyNMSGatherRow_8mpw_Asm_NEON64:
				## EndOf_Ifvec50_CompVCannyNMSGatherRow_8mpw_Asm_NEON64 ##
			
            ## Update NMS ##
            add r10, nms, col, LSR #1
            st1 {vecNMS.8b}, [r10]

			EndOf_Ifvec00_CompVCannyNMSGatherRow_8mpw_Asm_NEON64:
			## EndOf_Ifvec00_CompVCannyNMSGatherRow_8mpw_Asm_NEON64 ##

		add col, col, #(8*COMPV_GAS_UINT16_SZ_BYTES)
		cmp col, width // width contains (width - 7)
		blt LoopWidth_CompVCannyNMSGatherRow_8mpw_Asm_NEON64
        ##EndOf_LoopWidth_CompVCannyNMSGatherRow_8mpw_Asm_NEON64##
    
	.unreq nms
    .unreq g
    .unreq gx
    .unreq gy
    .unreq width
    .unreq stride

    .unreq col
    .unreq cOne
    .unreq cZero
    .unreq gTmp

    .unreq vecNMS
	.unreq vecG
	.unreq vecGX
	.unreq vecAbsGX
	.unreq vecTLow
	.unreq vecGY
	.unreq vecAbsGY0
	.unreq vecAbsGY1
    .unreq vecTangentPiOver8Int
	.unreq vecTangentPiTimes3Over8Int
    
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) uint16_t* grad
# arg(1) -> COMPV_ALIGNED(NEON) uint8_t* nms
# arg(2) -> compv_uscalar_t width
# arg(3) -> compv_uscalar_t height
# arg(4) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVCannyNMSApply_Asm_NEON64
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Set arguments ##
	grad .req r0
	nms .req r1
	width .req r2
	height .req r3
	stride .req r4

    # nmsTmp = r5
    # col = r6
    # vecZero = v15

    sub height, height, #1 // row start at 1

    movi v15.16b, #0

    #########################################################################
    # for (row_ = 1; row_ < height; ++row_)
    #########################################################################
    LoopHeight_CompVCannyNMSApply_Asm_NEON64:
        #########################################################################
        # for (col_ = 0; col_ < width; col_ += 8)
        #########################################################################
        mov r6, #0
        mov r5, nms
        LoopWidth8_CompVCannyNMSApply_Asm_NEON64:
            ld1 { v0.8b }, [r5], #(8*COMPV_GAS_UINT8_SZ_BYTES)
            cmhi v0.8b, v0.8b, v15.8b
            mov r10, v0.d[0]
            cbz r10, NothingToSupress_CompVCannyNMSApply_Asm_NEON64
                uxtl v0.8h, v0.8b
                add r10, nms, r6
                add r11, grad, r6, LSL #1
                ld1 { v1.8h }, [r11]
                sli v0.8h, v0.8h, #8
                st1 {v15.8b}, [r10]
                bic v1.16b, v1.16b, v0.16b // supress
                st1 {v1.8h}, [r11]
                NothingToSupress_CompVCannyNMSApply_Asm_NEON64:

            add r6, r6, #8
            cmp r6, width
            blt LoopWidth8_CompVCannyNMSApply_Asm_NEON64
            ## EndOf_CompVCannyNMSApply_Asm_NEON64 ##

        subs height, height, #1
        add grad, grad, stride, LSL #1
        add nms, nms, stride
        bne LoopHeight_CompVCannyNMSApply_Asm_NEON64
        ## EndOf_CompVCannyNMSApply_Asm_NEON64 ##

	.unreq grad
    .unreq nms
    .unreq width
    .unreq height
    .unreq stride
	
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__aarch64__) */

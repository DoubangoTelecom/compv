#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
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
    c0 .req r8
    gTmp .req r9

    vecNMS .req v7
	vecG .req v8
	vecZero .req v9
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

    movi vecZero.16b, #0

    .unreq tLow1 // tLow no longer needed
    c1 .req r4
    add c1, stride, #1
    mov c0, #1
    sub c0, c0, stride

    sub width, width, #7

    mov col, #1
    add gTmp, g, #(1*COMPV_GAS_UINT16_SZ_BYTES) // col starts at 1

    #########################################################################
    # for (col = 1; col < width - 7; col += 8)
    #########################################################################
    LoopWidth_CompVCannyNMSGatherRow_8mpw_Asm_NEON64:
        prfm pldl1keep, [gTmp, #(CACHE_LINE_SIZE*3)]
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
            add r10, gy, col, LSL #1
            add r11, gx, col, LSL #1
            movi vecNMS.8b, #0
			ld1 {vecGY.8h}, [r10]
			ld1 {vecGX.8h}, [r11]
			
			abs v3.8h, vecGY.8h
            abs vecAbsGX.8h, vecGX.8h

            shll vecAbsGY0.4s, v3.4h, #16
            shll2 vecAbsGY1.4s, v3.8h, #16

			## angle = "0° / 180°" ##
            umull v2.4s, vecTangentPiOver8Int.4h, vecAbsGX.4h
            umull2 v3.4s, vecTangentPiOver8Int.8h, vecAbsGX.8h
            cmhi v2.4s, v2.4s, vecAbsGY0.4s
            cmhi v3.4s, v3.4s, vecAbsGY1.4s
            uqxtn v4.4h, v2.4s
            uqxtn2 v4.8h, v3.4s
            and v1.16b, v0.16b, v4.16b // q1 = vec3
            mov r10, v1.d[0]
            mov r11, v1.d[1]
            orr r10, r10, r11
			#########################################################################
			# .if (COMPV_ARM_NEON_NEQ_ZERO(vec3))
			#########################################################################
			cbz r10, EndOf_Ifvec30_CompVCannyNMSGatherRow_8mpw_Asm_NEON64
			Ifvec30_CompVCannyNMSGatherRow_8mpw_Asm_NEON64:
                sub r10, col, #1 // g[col - 1]
                add r11, col, #1 // g[col + 1]
                add r10, g, r10, LSL #1
                add r11, g, r11, LSL #1
                ldr q2, [r10]
			    ld1 {v3.8h}, [r11]
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
                uxtl2 v4.4s, vecAbsGX.8h
                mul v6.4s, v6.4s, vecTangentPiTimes3Over8Int.4s
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
                    cmgt v4.8h, vecZero.8h, v4.8h
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
                        sub r10, col, c0
                        add r11, col, c0
                        add r10, g, r10, LSL #1
                        add r11, g, r11, LSL #1
                        ldr q6, [r10]
                        ld1 {v3.8h}, [r11]
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
                        sub r10, col, c1
                        add r11, col, c1
                        add r10, g, r10, LSL #1
                        add r11, g, r11, LSL #1
                        ldr q6, [r10]
                        ld1 {v3.8h}, [r11]
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
                add r10, g, r10, LSL #1
                add r11, g, r11, LSL #1
                ld1 {v6.8h}, [r10]
                ld1 {v3.8h}, [r11]
                cmhi v6.8h, v6.8h, vecG.8h
                cmhi v3.8h, v3.8h, vecG.8h
                orr v6.16b, v6.16b, v3.16b
                and v1.16b, v1.16b, v6.16b // q1 is old vec5
                uqxtn v1.8b, v1.8h
                orr vecNMS.8b, vecNMS.8b, v1.8b
				EndOf_Ifvec50_CompVCannyNMSGatherRow_8mpw_Asm_NEON64:
				## EndOf_Ifvec50_CompVCannyNMSGatherRow_8mpw_Asm_NEON64 ##
			
            ## Update NMS ##
            add r10, nms, col
            st1 {vecNMS.8b}, [nms]

			EndOf_Ifvec00_CompVCannyNMSGatherRow_8mpw_Asm_NEON64:
			## EndOf_Ifvec00_CompVCannyNMSGatherRow_8mpw_Asm_NEON64 ##

		add col, col, #8
		cmp col, width // width contains (width - 7)
		blt LoopWidth_CompVCannyNMSGatherRow_8mpw_Asm_NEON64
        ##EndOf_LoopWidth_CompVCannyNMSGatherRow_8mpw_Asm_NEON64##
    
	.unreq nms
    .unreq g
    .unreq gx
    .unreq gy
    .unreq tLow1
    .unreq width
    .unreq stride

    .unreq col
    .unreq c1
    .unreq c0
    .unreq gTmp

    .unreq vecNMS
	.unreq vecG
	.unreq vecZero
	.unreq vecGX
	.unreq vecAbsGX
    .unreq vecAbsGXx
    .unreq vecAbsGXy
	.unreq vecTLow
	.unreq vecGY
	.unreq vecAbsGY0
	.unreq vecAbsGY1
    .unreq vecTangentPiOver8Int
	.unreq vecTangentPiTimes3Over8Int
    
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#endif /* defined(__aarch64__) */

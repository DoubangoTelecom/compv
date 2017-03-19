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
.align 2
kCannyTangentPiOver8Int: .word 27145
kCannyTangentPiTimes3Over8Int: .word 158217

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
	
    ldr r8, =kCannyTangentPiOver8Int
    ldr r9, =kCannyTangentPiTimes3Over8Int
    ldrh r8w, [r8]
    ldr r9w, [r9]
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

            ushll vecAbsGY0.4s, v3.4h, #16
            ushll2 vecAbsGY1.4s, v3.8h, #16

			## angle = "0° / 180°" ##
            umull v2.4s, vecTangentPiOver8Int.4h, vecAbsGX.4h
            umull2 v3.4s, vecTangentPiOver8Int.8h, vecAbsGX.8h
            cmhi v2.4s, v2.4s, vecAbsGY0.4s
            cmhi v3.4s, v3.4s, vecAbsGY1.4s
            vqmovn.u32 q4x, q2
            vqmovn.u32 q4y, q3
            vand.u16 q1, q0, q4 @ q1 = vec3
            mov r10, v1.d[0]
            mov r11, v1.d[1]
            orr r10, r10, r11
			#########################################################################
			# .if (COMPV_ARM_NEON_NEQ_ZERO(vec3))
			#########################################################################
			cbz r10, EndOf_Ifvec30_CompVCannyNMSGatherRow_8mpw_Asm_NEON64
			Ifvec30_CompVCannyNMSGatherRow_8mpw_Asm_NEON64:
                sub r10, col, #1 @ g[col - 1]
                add r11, col, #1 @ g[col + 1]
                add r10, g, r10, LSL #1
                add r11, g, r11, LSL #1
                vld1.u16 {q2}, [r10 :128]
			    vld1.u16 {q3}, [r11]
                cmhi.u16 q2, q2, vecG
                cmhi.u16 q3, q3, vecG
                vorr.s16 q2, q2, q3
                vand.s16 q2, q2, q1 @ q1 is vec3
                vqmovn.u16 q2x, q2
                vorr.u8 vecNMSx, vecNMSx, q2x
				EndOf_Ifvec30_CompVCannyNMSGatherRow_8mpw_Asm_NEON64:
				@@ EndOf_Ifvec30_CompVCannyNMSGatherRow_8mpw_Asm_NEON64 @@

			@@ angle = "45° / 225°" or "135 / 315" @@
            vbic.s16 q2, q0, q1 @ q1 is vec3, now q2 = vec4
            mov r10, v2.d[0]
            mov r11, v2.d[1]
            orr r10, r10, r11
			#########################################################################
			# .if (COMPV_ARM_NEON_NEQ_ZERO(vec4)) - 0
			#########################################################################
			cbz r10, EndOf_Ifvec40_CompVCannyNMSGatherRow_8mpw_Asm_NEON64
			Ifvec40_CompVCannyNMSGatherRow_8mpw_Asm_NEON64:
                add r10, sp, #vecTangentPiTimes3Over8Int
                vld1.s16 {q3}, [r10]
                vmovl.u16 q6, vecAbsGXx
                vmovl.u16 q4, vecAbsGXy
                vmul.u32 q6, q6, q3
                vmul.u32 q4, q4, q3
                cmhi.u32 q6, q6, vecAbsGY0
                cmhi.u32 q4, q4, vecAbsGY1
                vqmovn.u32 q6x, q6
                vqmovn.u32 q6y, q4
                vand.u16 q2, q2, q6 @ q2 = old vec4, override
                mov r10, v2.d[0]
                mov r11, v2.d[1]
                orr r10, r10, r11
				#########################################################################
				# .if (COMPV_ARM_NEON_NEQ_ZERO(vec4)) - 1
				#########################################################################
				cbz r10, EndOf_Ifvec41_CompVCannyNMSGatherRow_8mpw_Asm_NEON64
				Ifvec41_CompVCannyNMSGatherRow_8mpw_Asm_NEON64:
                    veor.s16 q4, vecGX, vecGY
                    cmgt.s16 q4, vecZero, q4
                    vand.u16 q4, q4, q2 @ q2 is vec4, q4 = vec1
                    vbic.u16 q5, q2, q4 @ q2 is vec4, q5 = vec2
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
                        vld1.u16 {q6}, [r10 :128]
                        vld1.u16 {q3}, [r11]
                        cmhi.s16 q6, q6, vecG
                        cmhi.s16 q3, q3, vecG
                        vorr.u16 q6, q6, q3
                        vand.s16 q4, q4, q6 @ q4 is old vec1
						EndOf_Ifvec10_CompVCannyNMSGatherRow_8mpw_Asm_NEON64:
						@@ EndOf_Ifvec10_CompVCannyNMSGatherRow_8mpw_Asm_NEON64 @@


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
                        vld1.u16 {q6}, [r10 :128]
                        vld1.u16 {q3}, [r11]
                        cmhi.s16 q6, q6, vecG
                        cmhi.s16 q3, q3, vecG
                        vorr.u16 q6, q6, q3
                        vand.s16 q5, q5, q6 @ q5 is old vec2
						EndOfIfvec20_CompVCannyNMSGatherRow_8mpw_Asm_NEON64:
						@@ EndOfIfvec20_CompVCannyNMSGatherRow_8mpw_Asm_NEON64 @@


                    vorr.u16 q4, q4, q5 @ q4 is vec1 and q5 is vec2
                    vqmovn.u16 q4x, q4
                    vorr.u8 vecNMSx, vecNMSx, q4x
					EndOf_Ifvec41_CompVCannyNMSGatherRow_8mpw_Asm_NEON64:
					@@ EndOf_Ifvec41_CompVCannyNMSGatherRow_8mpw_Asm_NEON64 @@

				EndOf_Ifvec40_CompVCannyNMSGatherRow_8mpw_Asm_NEON64:
				@@ EndOf_Ifvec40_CompVCannyNMSGatherRow_8mpw_Asm_NEON64 @@
			
			@@ angle = "90° / 270°" @@
            vbic.s16 q2, q0, q2 @ q2 was vec4 and q0 is vec0
            vbic.s16 q1, q2, q1 @ q1 was vec3, now vec5 is q1
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
                vld1.u16 {q6}, [r10]
                vld1.u16 {q3}, [r11]
                cmhi.s16 q6, q6, vecG
                cmhi.s16 q3, q3, vecG
                vorr.u16 q6, q6, q3
                vand.s16 q1, q1, q6 @ q1 is old vec5
                vqmovn.u16 q1x, q1
                vorr.u8 vecNMSx, vecNMSx, q1x
				EndOf_Ifvec50_CompVCannyNMSGatherRow_8mpw_Asm_NEON64:
				@@ EndOf_Ifvec50_CompVCannyNMSGatherRow_8mpw_Asm_NEON64 @@
			
            @@ Update NMS @@
            add r10, nms, col
            vst1.u8 {vecNMSx}, [r10]

			EndOf_Ifvec00_CompVCannyNMSGatherRow_8mpw_Asm_NEON64:
			@ EndOf_Ifvec00_CompVCannyNMSGatherRow_8mpw_Asm_NEON64 @@

		add col, col, #8
		cmp col, width @ width contains (width - 7)
		blt LoopWidth_CompVCannyNMSGatherRow_8mpw_Asm_NEON64
        @@EndOf_LoopWidth_CompVCannyNMSGatherRow_8mpw_Asm_NEON64@@
    
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

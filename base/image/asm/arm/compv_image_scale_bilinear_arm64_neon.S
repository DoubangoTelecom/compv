#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__aarch64__)
.include "compv_common_arm64.S"

.data

.text

#########################################################################
# arg(0) -> const uint8_t* inPtr
# arg(1) -> compv_uscalar_t inStride
# arg(2) -> COMPV_ALIGNED(NEON) uint8_t* outPtr
# arg(3) -> compv_uscalar_t outWidth
# arg(4) -> compv_uscalar_t outYStart
# arg(5) -> compv_uscalar_t outYEnd
# arg(6) -> COMPV_ALIGNED(NEON) compv_uscalar_t outStride
# arg(7) -> compv_uscalar_t sf_x
# arg(8) -> compv_uscalar_t sf_y
COMPV_GAS_FUNCTION_DECLARE CompVImageScaleBilinear_Asm_NEON64
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 9
	COMPV_GAS_SAVE_NEON_REGS

	## Declare input arguments ##
	ldp_arg 0, r0, r1
    ldp_arg 2, r2, r3
    ldp_arg 4, r4, r5
    ldp_arg 6, r6, r7
    ldr_arg 8, r8
	inPtr .req r0 
	inStride .req r1
	outPtr .req r2
	outWidth .req r3
	outYStart .req r4
	outYEnd .req r5
	outStride .req r6
	sf_x .req r7
	sf_y .req r8

	## Declare local vectors ##
    #define vecy0            v0
	#define vecy1            v1
	#define vecSfxTimes16    v2
	#define vecSFX0          v3
	#define vecSFX1          v4
	#define vecSFX2          v5
	#define vecSFX3          v6
	#define vecNeighb0       v7
	#define vecNeighb1       v8
	#define vecNeighb2       v9
	#define vecNeighb3       v10
	#define vecX0            v11
	#define vecX1            v12
	#define vecX2            v13
	#define vecX3            v14
	#define vec4             v15
	#define vec5             v16
	#define vec6             v17
	#define vec7             v18
    #define vec0xff_epi16    v19
    #define vec0xff_epi32    v20

	## compute vecSfxTimes16 ##
	lsl r11, sf_x, #4
	dup vecSfxTimes16.4s, r11w

	## compute vecSFX0, vecSFX1, vecSFX2 and vecSFX3 ##
	mov r11, #0 
	lsl r10, sf_x, #1
	mov vecSFX0.s[0], r11w // sf_x * 0
	add r11, r10, sf_x
	mov vecSFX0.s[1], r7w // sf_x * 1
	mov vecSFX0.s[2], r10w   // sf_x * 2
	mov vecSFX0.s[3], r11w  // sfx * 3
	lsl r11, sf_x, #2 // sfx * 3
	dup v30.4s, r11w // v30 = vecSfxTimes4
	add vecSFX1.4s, vecSFX0.4s, v30.4s
	add vecSFX2.4s, vecSFX1.4s, v30.4s
	add vecSFX3.4s, vecSFX2.4s, v30.4s
	.unreq sf_x // r7 is no longer needed
    inPtr_ .req r7

    ## compute vec0xff_epi16 and vec0xff_epi32 ##
    mov r11, #0xff
    dup vec0xff_epi16.8h, r11w
    dup vec0xff_epi32.4s, r11w

	i .req r9
    outPtr_ .req r17

	#####################################
	# do
	#####################################
	DoWhile:
        lsr r20, outYStart, #8 // r20 = nearestY
        mov r23, #0xff
        mul r21, r20, inStride // r21 = (nearestY * inStride)
        and r22, outYStart, r23
        add inPtr_, inPtr, r21 // inPtr_ = &inPtr[nearestY * inStride]
        mov i, #0
        dup vecy0.8h, r22w
        bic vecy1.16b, vec0xff_epi16.16b, vecy0.16b
        mov vecX0.16b, vecSFX0.16b // 4s
        mov vecX1.16b, vecSFX1.16b // 4s
        mov vecX2.16b, vecSFX2.16b // 4s
        mov vecX3.16b, vecSFX3.16b // 4s
        mov outPtr_, outPtr
		#####################################
		# for (i = 0; i < outWidth; i += 16)
		#####################################
		LoopWidth:
			## nearest x-point ##
			ushr v21.4s, vecX0.4s, #8
			ushr v22.4s, vecX1.4s, #8
			ushr v23.4s, vecX2.4s, #8
			ushr v24.4s, vecX3.4s, #8

			## write memNeighbs ##
            // v26 -> tmp(vecNeighb0)
            // v27 -> tmp(vecNeighb1)
            // v28 -> tmp(vecNeighb2)
            // v29 -> tmp(vecNeighb3)
            mov r10w, v21.s[0]
            mov r11w, v21.s[1]
            mov r12w, v21.s[2]
            mov r13w, v21.s[3]
            mov r14w, v22.s[0]
            mov r15w, v22.s[1]
            mov r26w, v22.s[2]
            mov r27w, v22.s[3]
            add r28, r10, inStride
            add r19, r11, inStride
            add r20, r12, inStride
            add r21, r13, inStride
            add r22, r14, inStride
            add r23, r15, inStride
            add r24, r26, inStride
            add r25, r27, inStride
            ldrh r10w, [inPtr_, r10]
            ldrh r11w, [inPtr_, r11]
            ldrh r12w, [inPtr_, r12]
            ldrh r13w, [inPtr_, r13]
            ldrh r14w, [inPtr_, r14]
            ldrh r15w, [inPtr_, r15]
            ldrh r26w, [inPtr_, r26]
            ldrh r27w, [inPtr_, r27]
            ldrh r28w, [inPtr_, r28]
            ldrh r19w, [inPtr_, r19]
            ldrh r20w, [inPtr_, r20]
            ldrh r21w, [inPtr_, r21]
            ldrh r22w, [inPtr_, r22]
            ldrh r23w, [inPtr_, r23]
            ldrh r24w, [inPtr_, r24]
            ldrh r25w, [inPtr_, r25]
            add r10, r10, r11, LSL #16
            add r12, r12, r13, LSL #16
            add r14, r14, r15, LSL #16
            add r26, r26, r27, LSL #16
            add r28, r28, r19, LSL #16
            add r20, r20, r21, LSL #16
            add r22, r22, r23, LSL #16
            add r24, r24, r25, LSL #16
            mov v26.s[0], r10w
            mov v26.s[1], r12w
            mov v26.s[2], r14w
            mov v26.s[3], r26w
            mov v28.s[0], r28w
            mov v28.s[1], r20w
            mov v28.s[2], r22w
            mov v28.s[3], r24w

            mov r10w, v23.s[0]
            mov r11w, v23.s[1]
            mov r12w, v23.s[2]
            mov r13w, v23.s[3]
            mov r14w, v24.s[0]
            mov r15w, v24.s[1]
            mov r26w, v24.s[2]
            mov r27w, v24.s[3]
            add r28, r10, inStride
            add r19, r11, inStride
            add r20, r12, inStride
            add r21, r13, inStride
            add r22, r14, inStride
            add r23, r15, inStride
            add r24, r26, inStride
            add r25, r27, inStride
            ldrh r10w, [inPtr_, r10]
            ldrh r11w, [inPtr_, r11]
            ldrh r12w, [inPtr_, r12]
            ldrh r13w, [inPtr_, r13]
            ldrh r14w, [inPtr_, r14]
            ldrh r15w, [inPtr_, r15]
            ldrh r26w, [inPtr_, r26]
            ldrh r27w, [inPtr_, r27]
            ldrh r28w, [inPtr_, r28]
            ldrh r19w, [inPtr_, r19]
            ldrh r20w, [inPtr_, r20]
            ldrh r21w, [inPtr_, r21]
            ldrh r22w, [inPtr_, r22]
            ldrh r23w, [inPtr_, r23]
            ldrh r24w, [inPtr_, r24]
            ldrh r25w, [inPtr_, r25]
            add r10, r10, r11, LSL #16 
            add r12, r12, r13, LSL #16
            add r14, r14, r15, LSL #16
            add r26, r26, r27, LSL #16
            add r28, r28, r19, LSL #16
            add r20, r20, r21, LSL #16
            add r22, r22, r23, LSL #16
            add r24, r24, r25, LSL #16
            mov v27.s[0], r10w
            mov v27.s[1], r12w
            mov v27.s[2], r14w
            mov v27.s[3], r26w
            mov v29.s[0], r28w
            mov v29.s[1], r20w
            mov v29.s[2], r22w
            mov v29.s[3], r24w

			## Deinterleave neighbs ##
            // v26[tmp(vecNeighb0)] = 0,1,0,1,0,1,0,1,0,1
            // v27[tmp(vecNeighb1)] = 0,1,0,1,0,1,0,1,0,1
            // v28[tmp(vecNeighb2)] = 2,3,2,3,2,3,2,3,2,3
            // v29[tmp(vecNeighb3)] = 2,3,2,3,2,3,2,3,2,3
            uzp1 vecNeighb0.16b, v26.16b, v27.16b // 0,0,0,0,0,0,0,0
            uzp2 vecNeighb1.16b, v26.16b, v27.16b // 1,1,1,1,1,1,1,1
            uzp1 vecNeighb2.16b, v28.16b, v29.16b // 2,2,2,2,2,2,2,2
            uzp2 vecNeighb3.16b, v28.16b, v29.16b // 3,3,3,3,3,3,3,3

            ## Compute other values ##
            and v21.16b, vecX0.16b, vec0xff_epi32.16b
            and v22.16b, vecX1.16b, vec0xff_epi32.16b
            and v27.16b, vecX2.16b, vec0xff_epi32.16b
            and v28.16b, vecX3.16b, vec0xff_epi32.16b
            xtn v21.4h, v21.4s
            xtn v27.4h, v27.4s
            uxtl v23.8h, vecNeighb0.8b
            xtn2 v21.8h, v22.4s
            xtn2 v27.8h, v28.4s
            uxtl v24.8h, vecNeighb1.8b
            uxtl v26.8h, vecNeighb3.8b
            mul vec4.8h, v24.8h, v21.8h
            mul vec5.8h, v26.8h, v21.8h
            bic v28.16b, vec0xff_epi16.16b, v27.16b
            uxtl2 v29.8h, vecNeighb0.16b
            uxtl2 v30.8h, vecNeighb2.16b
            mul vec6.8h, v29.8h, v28.8h
            mul vec7.8h, v30.8h, v28.8h
            bic v22.16b, vec0xff_epi16.16b, v21.16b
            uxtl v25.8h, vecNeighb2.8b
            uxtl2 vecNeighb0.8h, vecNeighb1.16b
            uxtl2 vecNeighb1.8h, vecNeighb3.16b
            add vecX2.4s, vecX2.4s, vecSfxTimes16.4s
            add vecX3.4s, vecX3.4s, vecSfxTimes16.4s
            mla vec4.8h, v23.8h, v22.8h
            mla vec5.8h, v25.8h, v22.8h
            mla vec6.8h, vecNeighb0.8h, v27.8h    
            mla vec7.8h, vecNeighb1.8h, v27.8h
            umull v21.4s, vecy1.4h, vec4.4h
            umull2 v22.4s, vecy1.8h, vec4.8h
            umull v23.4s, vecy1.4h, vec6.4h
            umull2 v24.4s, vecy1.8h, vec6.8h
            umull v25.4s, vecy0.4h, vec5.4h
            umull2 v26.4s, vecy0.8h, vec5.8h
            umull v27.4s, vecy0.4h, vec7.4h
            umull2 v28.4s, vecy0.8h, vec7.8h
            shrn vec4.4h, v21.4s, #16
            shrn vec5.4h, v25.4s, #16
            shrn vec6.4h, v23.4s, #16
            shrn vec7.4h, v27.4s, #16
            shrn2 vec4.8h, v22.4s, #16
            shrn2 vec5.8h, v26.4s, #16
            shrn2 vec6.8h, v24.4s, #16
            shrn2 vec7.8h, v28.4s, #16
            uqadd vec4.8h, vec4.8h, vec5.8h
            uqadd vec6.8h, vec6.8h, vec7.8h
            xtn v21.8b, vec4.8h
            add vecX0.4s, vecX0.4s, vecSfxTimes16.4s
            xtn2 v21.16b, vec6.8h
            add vecX1.4s, vecX1.4s, vecSfxTimes16.4s
            st1 {v21.16b}, [outPtr_], #16

			##
			add i, i, #16
			cmp i, outWidth
			blt LoopWidth
			# end-of-LoopWidth #

		##
		add outYStart, outYStart, sf_y
		add outPtr, outPtr, outStride
		cmp outYStart, outYEnd
		blt DoWhile
		# end-of-DoWhile #
	
	.unreq outPtr
	.unreq outWidth
	.unreq outYStart
	.unreq inStride
    .unreq sf_y
    .unreq outStride
    .unreq outYEnd

	#undef vecNeighb0
	#undef vecNeighb1
	#undef vecNeighb2
	#undef vecNeighb3
	#undef vecX0
	#undef vecX1
	#undef vecX2
	#undef vecX3
	#undef vec4
	#undef vec5
	#undef vec6
	#undef vec7
    #undef vec0xff_epi16
    #undef vec0xff_epi32

	.unreq i
	.unreq inPtr_
    .unreq outPtr_

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN



#endif /* defined(__aarch64__) */

#########################################################################
# Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__arm__) && !defined(__aarch64__)
.include "compv_common_arm32.S"

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

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> uint8_t* nms
@ arg(1) -> const uint16_t* g
@ arg(2) -> const int16_t* gx
@ arg(3) -> const int16_t* gy
@ arg(4) -> const uint16_t* tLow1
@ arg(5) -> compv_uscalar_t width
@ arg(6) -> compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVCannyNMSGatherRow_8mpw_Asm_NEON32
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 7
	COMPV_GAS_SAVE_NEON_REGS

	@@ Load arguments @@
	ldm_args r0-r6
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
    
    vecNMSx .req q7x
    vecTangentPiOver8Intx .req q7y
	vecG .req q8
	vecTangentPiTimes3Over8Int .req q9
	vecGX .req q10
	vecAbsGX .req q11
    vecAbsGXx .req q11x
    vecAbsGXy .req q11y
	vecTLow .req q12
	vecGY .req q13
	vecAbsGY0 .req q14
	vecAbsGY1 .req q15
	
    ldr r8, =kCannyTangentPiOver8Int
    ldr r9, =kCannyTangentPiTimes3Over8Int
    ldrh r8, [r8]
    ldr r9, [r9]
    ldrh r10, [tLow1]

    vdup.u16 vecTangentPiOver8Intx, r8
    vdup.u32 vecTangentPiTimes3Over8Int, r9
    vdup.u16 vecTLow, r10

    .unreq tLow1 @ tLow no longer needed
    cOne .req r4
    add cOne, stride, #1
    mov cZero, #1
    sub cZero, cZero, stride

    sub width, width, #7

    mov col, #1
    add gTmp, g, #(1*COMPV_GAS_UINT16_SZ_BYTES) @ col starts at 1

    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (col = 1; col < width - 7; col += 8)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    LoopWidth_CompVCannyNMSGatherRow_8mpw_Asm_NEON32:
        pld [gTmp, #(CACHE_LINE_SIZE*3)]
        vld1.u16 {vecG}, [gTmp]!
        vcgt.u16 q0, vecG, vecTLow
        vorr.u16 q1x, q0x, q0y
        vmov.u32 r10, q1x[0]
        vmov.u32 r11, q1x[1]
        orrs r11, r11, r10
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ if (COMPV_ARM_NEON_NEQ_ZEROQ(vec0))
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        beq EndOf_Ifvec0_CompVCannyNMSGatherRow_8mpw_Asm_NEON32
		Ifvec0_CompVCannyNMSGatherRow_8mpw_Asm_NEON32:
            add r11, gx, col, LSL #1
            add r10, gy, col, LSL #1
			vld1.s16 {vecGX}, [r11]
            vabs.s16 vecAbsGX, vecGX
            vmull.u16 q4, vecTangentPiOver8Intx, vecAbsGXx
            vmull.u16 q5, vecTangentPiOver8Intx, vecAbsGXy
            vld1.s16 {vecGY}, [r10]
			vabs.s16 q3, vecGY
            vshll.u16 vecAbsGY0, q3x, #16
            vshll.u16 vecAbsGY1, q3y, #16
			veor.u8 vecNMSx, vecNMSx, vecNMSx
            vcgt.u32 q4, q4, vecAbsGY0
            vcgt.u32 q5, q5, vecAbsGY1
            vqmovn.u32 q4x, q4
            vqmovn.u32 q4y, q5
            vand.u16 q1, q0, q4 @ q1 = vec3
            vorr.u16 q4x, q1x, q1y
			vmov.u32 r10, q4x[0]
            vmov.u32 r11, q4x[1]
            orrs r11, r11, r10
			@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			@ if (COMPV_ARM_NEON_NEQ_ZEROQ(vec3))
			@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			beq EndOf_Ifvec30_CompVCannyNMSGatherRow_8mpw_Asm_NEON32
			Ifvec30_CompVCannyNMSGatherRow_8mpw_Asm_NEON32:
                sub r10, col, #1 @ g[col - 1]
                add r10, g, r10, LSL #1
                mov r11, #(2*COMPV_GAS_UINT16_SZ_BYTES)
                vld1.u16 {q2}, [r10 :128], r11
                vcgt.u16 q2, q2, vecG
			    vld1.u16 {q3}, [r10]
                vcgt.u16 q3, q3, vecG
                vorr.s16 q2, q2, q3
                vand.s16 q2, q2, q1 @ q1 is vec3
                vqmovn.u16 q2x, q2
                vorr.u8 vecNMSx, vecNMSx, q2x
				EndOf_Ifvec30_CompVCannyNMSGatherRow_8mpw_Asm_NEON32:
				@@ EndOf_Ifvec30_CompVCannyNMSGatherRow_8mpw_Asm_NEON32 @@

			@@ angle = "45° / 225°" or "135 / 315" @@
            vbic.s16 q2, q0, q1 @ q1 is vec3, now q2 = vec4
			vorr.u16 q3x, q2x, q2y
			vmov.u32 r10, q3x[0]
            vmov.u32 r11, q3x[1]
            orrs r11, r11, r10
			@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			@ if (COMPV_ARM_NEON_NEQ_ZEROQ(vec4)) - 0
			@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			beq EndOf_Ifvec40_CompVCannyNMSGatherRow_8mpw_Asm_NEON32
			Ifvec40_CompVCannyNMSGatherRow_8mpw_Asm_NEON32:
                vmovl.u16 q5, vecAbsGXx
                vmul.u32 q5, q5, vecTangentPiTimes3Over8Int
				vmovl.u16 q4, vecAbsGXy
                vmul.u32 q4, q4, vecTangentPiTimes3Over8Int
                vcgt.u32 q5, q5, vecAbsGY0
                vcgt.u32 q4, q4, vecAbsGY1
                vqmovn.u32 q5x, q5
                vqmovn.u32 q5y, q4
                vand.u16 q2, q2, q5 @ q2 = old vec4, override
                vorr.u16 q5x, q2x, q2y
                vmov.u32 r10, q5x[0]
                vmov.u32 r11, q5x[1]
                orrs r11, r11, r10
				@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
				@ if (COMPV_ARM_NEON_NEQ_ZEROQ(vec4)) - 1
				@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
				beq EndOf_Ifvec41_CompVCannyNMSGatherRow_8mpw_Asm_NEON32
				Ifvec41_CompVCannyNMSGatherRow_8mpw_Asm_NEON32:
                    veor.s16 q4, vecGX, vecGY
                    vclt.s16 q4, q4, #0
                    vand.u16 q4, q4, q2 @ q2 is vec4, q4 = vec1
                    vbic.u16 q5, q2, q4 @ q2 is vec4, q5 = vec2
                    vorr.u16 q3x, q4x, q4y
                    vmov.u32 r10, q3x[0]
                    vmov.u32 r11, q3x[1]
                    orrs r11, r10
					@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
					@ if (COMPV_ARM_NEON_NEQ_ZEROQ(vec1))
					@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
					beq EndOf_Ifvec10_CompVCannyNMSGatherRow_8mpw_Asm_NEON32
					Ifvec10_CompVCannyNMSGatherRow_8mpw_Asm_NEON32:
                        sub r10, col, cZero
						lsl r11, cZero, #(1+1) @ r11 = g[col + c0] LSL #1 for 2*c0 and #1 for conversion for bytes to shorts    
						add r10, g, r10, LSL #1 @ r10 = g[col - c0]
                        vld1.u16 {q6}, [r10 :128], r11
                        vld1.u16 {q3}, [r10]
                        vcgt.u16 q6, q6, vecG
                        vcgt.u16 q3, q3, vecG
                        vorr.u16 q6, q6, q3
                        vand.s16 q4, q4, q6 @ q4 is old vec1
						EndOf_Ifvec10_CompVCannyNMSGatherRow_8mpw_Asm_NEON32:
						@@ EndOf_Ifvec10_CompVCannyNMSGatherRow_8mpw_Asm_NEON32 @@

                    vorr.u16 q6x, q5x, q5y @ q5 is vec2
                    vmov.u32 r10, q6x[0]
                    vmov.u32 r11, q6x[1]
                    orrs r11, r11, r10
					@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
					@ if (COMPV_ARM_NEON_NEQ_ZEROQ(vec2))
					@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
					beq EndOfIfvec20_CompVCannyNMSGatherRow_8mpw_Asm_NEON32
					Ifvec20_CompVCannyNMSGatherRow_8mpw_Asm_NEON32:
                        sub r10, col, cOne
						lsl r11, cOne, #(1+1) @ r11 = g[col + c1] LSL #1 for 2*c1 and #1 for conversion for bytes to shorts  
                        add r10, g, r10, LSL #1
                        vld1.u16 {q6}, [r10 :128], r11
                        vcgt.u16 q6, q6, vecG
                        vld1.u16 {q3}, [r10]
                        vcgt.u16 q3, q3, vecG
                        vorr.u16 q6, q6, q3
                        vand.s16 q5, q5, q6 @ q5 is old vec2
						EndOfIfvec20_CompVCannyNMSGatherRow_8mpw_Asm_NEON32:
						@@ EndOfIfvec20_CompVCannyNMSGatherRow_8mpw_Asm_NEON32 @@


                    vorr.u16 q4, q4, q5 @ q4 is vec1 and q5 is vec2
                    vqmovn.u16 q4x, q4
                    vorr.u8 vecNMSx, vecNMSx, q4x
					EndOf_Ifvec41_CompVCannyNMSGatherRow_8mpw_Asm_NEON32:
					@@ EndOf_Ifvec41_CompVCannyNMSGatherRow_8mpw_Asm_NEON32 @@

				EndOf_Ifvec40_CompVCannyNMSGatherRow_8mpw_Asm_NEON32:
				@@ EndOf_Ifvec40_CompVCannyNMSGatherRow_8mpw_Asm_NEON32 @@
			
			@@ angle = "90° / 270°" @@
            vbic.s16 q2, q0, q2 @ q2 was vec4 and q0 is vec0
            vbic.s16 q1, q2, q1 @ q1 was vec3, now vec5 is q1
            vorr.u16 q6x, q1x, q1y
            vmov.u32 r10, q6x[0]
            vmov.u32 r11, q6x[1]
            orrs r11, r11, r10
			@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			@ if (COMPV_ARM_NEON_NEQ_ZEROQ(vec5))
			@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			beq EndOf_Ifvec50_CompVCannyNMSGatherRow_8mpw_Asm_NEON32
			Ifvec50_CompVCannyNMSGatherRow_8mpw_Asm_NEON32:
                sub r10, col, stride
				lsl r11, stride, #(1+1) @ r11 = g[col + stride] LSL #1 for 2*stride and #1 for conversion for bytes to shorts 
				add r10, g, r10, LSL #1
                vld1.u16 {q6}, [r10], r11
                vcgt.u16 q6, q6, vecG
                vld1.u16 {q3}, [r10]
                vcgt.u16 q3, q3, vecG
                vorr.u16 q6, q6, q3
                vand.s16 q1, q1, q6 @ q1 is old vec5
                vqmovn.u16 q1x, q1
                vorr.u8 vecNMSx, vecNMSx, q1x
				EndOf_Ifvec50_CompVCannyNMSGatherRow_8mpw_Asm_NEON32:
				@@ EndOf_Ifvec50_CompVCannyNMSGatherRow_8mpw_Asm_NEON32 @@
			
            @@ Update NMS @@
            add r10, nms, col
            vst1.u8 {vecNMSx}, [r10]

			EndOf_Ifvec0_CompVCannyNMSGatherRow_8mpw_Asm_NEON32:
			@ EndOf_Ifvec0_CompVCannyNMSGatherRow_8mpw_Asm_NEON32 @@

		add col, col, #8
		cmp col, width @ width contains (width - 7)
		blt LoopWidth_CompVCannyNMSGatherRow_8mpw_Asm_NEON32
        @@EndOf_LoopWidth_CompVCannyNMSGatherRow_8mpw_Asm_NEON32@@
    
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

    .unreq vecNMSx
    .unreq vecTangentPiOver8Intx
	.unreq vecG
	.unreq vecTangentPiTimes3Over8Int
	.unreq vecGX
	.unreq vecAbsGX
    .unreq vecAbsGXx
    .unreq vecAbsGXy
	.unreq vecTLow
	.unreq vecGY
	.unreq vecAbsGY0
	.unreq vecAbsGY1

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 7
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) uint16_t* grad
@ arg(1) -> COMPV_ALIGNED(NEON) uint8_t* nms
@ arg(2) -> compv_uscalar_t width
@ arg(3) -> compv_uscalar_t height
@ arg(4) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVCannyNMSApply_Asm_NEON32
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 5
	COMPV_GAS_SAVE_NEON_REGS

	@@ Load arguments @@
	ldm_args r0-r4
	grad .req r0
	nms .req r1
	width .req r2
	height .req r3
	stride .req r4

    @ nmsTmp = r5
    @ col = r6
    @ vecZero = q15

    sub height, height, #1 @ row start at 1

    veor.u8 q15, q15, q15

    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (row_ = 1; row_ < height; ++row_)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    LoopHeight_CompVCannyNMSApply_Asm_NEON32:
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (col_ = 0; col_ < width; col_ += 8)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        mov r6, #0
        mov r5, nms
        LoopWidth8_CompVCannyNMSApply_Asm_NEON32:
            pld [r5, #(CACHE_LINE_SIZE*3)]
            vld1.u8 { q0x }, [r5 :64]!
            vcgt.u8 q0x, q0x, q15x
            vmov.u32 r10, q0x[0]
            vmov.u32 r11, q0x[1]
            orrs r10, r10, r11
            beq NothingToSupress_CompVCannyNMSApply_Asm_NEON32
                vmovl.u8 q0, q0x
                add r10, nms, r6
                add r11, grad, r6, LSL #1
                vld1.u16 { q1 }, [r11 :128]
                vsli.u16 q0, q0, #8
                vst1.u8 {q15x}, [r10 :64]
                vbic.u16 q1, q1, q0 @ supress
                vst1.u16 {q1}, [r11 :128]
                NothingToSupress_CompVCannyNMSApply_Asm_NEON32:

            add r6, r6, #8
            cmp r6, width
            blt LoopWidth8_CompVCannyNMSApply_Asm_NEON32
            @@ EndOf_CompVCannyNMSApply_Asm_NEON32 @@

        subs height, height, #1
        add grad, grad, stride, LSL #1
        add nms, nms, stride
        bne LoopHeight_CompVCannyNMSApply_Asm_NEON32
        @@ EndOf_CompVCannyNMSApply_Asm_NEON32 @@

	.unreq grad
    .unreq nms
    .unreq width
    .unreq height
    .unreq stride
	
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 5
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN




#endif /* defined(__arm__) && !defined(__aarch64__) */

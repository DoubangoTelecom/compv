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

.align 4
mask: .word 0x8040201, 0x80402010, 0x8040201, 0x80402010

.extern
 
.text

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> const uint8_t* img_center
@ arg(1) -> compv_uscalar_t img_stride
@ arg(2) -> const compv_float32_t* cos1
@ arg(3) -> const compv_float32_t* sin1
@ arg(4) -> COMPV_ALIGNED(NEON) const compv_float32_t* kBrief256Pattern31AX
@ arg(5) -> COMPV_ALIGNED(NEON) const compv_float32_t* kBrief256Pattern31AY
@ arg(6) -> COMPV_ALIGNED(NEON) const compv_float32_t* kBrief256Pattern31BX
@ arg(7) -> COMPV_ALIGNED(NEON) const compv_float32_t* kBrief256Pattern31BY
@ arg(8) -> void* out
.macro CompVOrbBrief256_31_32f_Macro_NEON32 fusedMultiplyAdd
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 9
	COMPV_GAS_SAVE_NEON_REGS
    COMPV_GAS_ALIGN_STACK 16, r11
	COMPV_GAS_MEMALLOC ((16*COMPV_GAS_INT32_SZ_BYTES) + (16*COMPV_GAS_UINT8_SZ_BYTES) + (16*COMPV_GAS_UINT8_SZ_BYTES))

    .equ vecIndex		, 0
	.equ vecA		    , (vecIndex + (16*COMPV_GAS_INT32_SZ_BYTES))
	.equ vecB			, (vecA + (16*COMPV_GAS_UINT8_SZ_BYTES))

	
	@@ Load arguments @@
	ldm_args r0-r8 @ load arguments in [r0-r8]
	img_center .req r0
	img_stride .req r1
	cos1 .req r2
	sin1 .req r3
	kBrief256Pattern31AX .req r4
	kBrief256Pattern31AY .req r5
    kBrief256Pattern31BX .req r6
    kBrief256Pattern31BY .req r7
    out .req r8
    @ r9, r10 and r11 are free

    #define argi_img_stride 1
    #define argi_cos1 2
    #define argi_sin1 3
    
    vecMask .req q0
    vecMaskx .req q0x
    vecMasky .req q0y
    vecCosT .req q1
	vecCosTx .req q1x
    vecSinT .req q2
	vecSinTx .req q2x
    vecStride .req q3
	vecStridex .req q3x

    ldr r9, [cos1]
    ldr r10, [sin1]
    vdup.s32 vecStride, img_stride
    vdup.f32 vecCosT, r9
    vdup.f32 vecSinT, r10

#if 0 // movw and movt are ARMv7 instructions
    .equ mask0, 0x8040201
    .equ mask1, 0x80402010
    movw r9, #:lower16:mask0
    movt r9, #:upper16:mask0
    movw r10, #:lower16:mask1
    movt r10, #:upper16:mask1
    vmov vecMaskx, r9, r10
    vmov vecMasky, r9, r10
#else
	ldr r9, =mask
	vld1.u32 {vecMask}, [r9]
#endif

    .unreq img_stride
    .unreq cos1
    .unreq sin1
    i .req r1 @ was img_stride
    t0 .req r2 @ was cos1
    t1 .req r3 @ was sin1
    t2 .req r9
    t3 .req r10
    t4 .req r11
    
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (size_t i = 0; i < 256; i += 16)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    mov i, #256
    Loop256_CompVOrbBrief256_31_32f_Asm_NEON32\@:
        .set xy, 0
        .rept 2
            .if xy == 0
                vld1.f32 { q12, q13 }, [kBrief256Pattern31AX :128]!
                vld1.f32 { q14, q15 }, [kBrief256Pattern31AX :128]!
            .else
                vld1.f32 { q12, q13 }, [kBrief256Pattern31BX :128]!
                vld1.f32 { q14, q15 }, [kBrief256Pattern31BX :128]!
            .endif
            vmul.f32 q4, q12, vecCosTx[0]
            vmul.f32 q5, q13, vecCosTx[0]
            vmul.f32 q6, q14, vecCosTx[0]
            vmul.f32 q7, q15, vecCosTx[0]
            vmul.f32 q8, q12, vecSinTx[0]
            vmul.f32 q9, q13, vecSinTx[0]
            vmul.f32 q10, q14, vecSinTx[0]
            vmul.f32 q11, q15, vecSinTx[0]
            .if xy == 0
                vld1.f32 { q12, q13 }, [kBrief256Pattern31AY :128]!
                vld1.f32 { q14, q15 }, [kBrief256Pattern31AY :128]!
            .else
                vld1.f32 { q12, q13 }, [kBrief256Pattern31BY :128]!
                vld1.f32 { q14, q15 }, [kBrief256Pattern31BY :128]!
            .endif
            
            .if \fusedMultiplyAdd
                vfms.f32 q4, q12, vecSinT
                vfms.f32 q5, q13, vecSinT
                vfms.f32 q6, q14, vecSinT
                vfms.f32 q7, q15, vecSinT
                vfma.f32 q8, q12, vecCosT
                vfma.f32 q9, q13, vecCosT
                vfma.f32 q10, q14, vecCosT
                vfma.f32 q11, q15, vecCosT
            .else
                vmls.f32 q4, q12, vecSinTx[0]
                vmls.f32 q5, q13, vecSinTx[0]
                vmls.f32 q6, q14, vecSinTx[0]
                vmls.f32 q7, q15, vecSinTx[0]
                vmla.f32 q8, q12, vecCosTx[0]
                vmla.f32 q9, q13, vecCosTx[0]
                vmla.f32 q10, q14, vecCosTx[0]
                vmla.f32 q11, q15, vecCosTx[0]
            .endif
            mov t4, #0x3f000000 @ 0.5f
            vdup.s32 q15, t4 @ q15 = vecHalf
            vdup.32 q15, r11
            vshr.u32 q14, q4, #0x1f
            vcvt.f32.u32 q12, q14
            vadd.f32 q13, q4, q15
            vshr.u32 q14, q5, #0x1f
            vsub.f32 q13, q13, q12
            vcvt.f32.u32 q12, q14
            vcvt.s32.f32 q4, q13
            vadd.f32 q13, q5, q15
            vshr.u32 q14, q6, #0x1f
            vsub.f32 q13, q13, q12
            vcvt.f32.u32 q12, q14
            vcvt.s32.f32 q5, q13
            vadd.f32 q13, q6, q15
            vshr.u32 q14, q7, #0x1f
            vsub.f32 q13, q13, q12
            vcvt.f32.u32 q12, q14
            vcvt.s32.f32 q6, q13
            vadd.f32 q13, q7, q15
            vshr.u32 q14, q8, #0x1f
            vsub.f32 q13, q13, q12
            vcvt.f32.u32 q12, q14
            vcvt.s32.f32 q7, q13
            vadd.f32 q13, q8, q15
            vshr.u32 q14, q9, #0x1f
            vsub.f32 q13, q13, q12
            vcvt.f32.u32 q12, q14
            vcvt.s32.f32 q8, q13
            vadd.f32 q13, q9, q15
            vshr.u32 q14, q10, #0x1f
            vsub.f32 q13, q13, q12
            vcvt.f32.u32 q12, q14
            vcvt.s32.f32 q9, q13
            vadd.f32 q13, q10, q15
            vshr.u32 q14, q11, #0x1f
            vsub.f32 q13, q13, q12
            vcvt.f32.u32 q12, q14
            vcvt.s32.f32 q10, q13
            vadd.f32 q13, q11, q15
            vsub.f32 q13, q13, q12
            vcvt.s32.f32 q11, q13
            vmla.s32 q4, q8, vecStridex[0]
            vmla.s32 q5, q9, vecStridex[0]
            vmla.s32 q6, q10, vecStridex[0]
			vmla.s32 q7, q11, vecStridex[0]
            add t4, sp, #vecIndex
            vst1.s32 {q4, q5}, [t4]!
            vst1.s32 {q6, q7}, [t4]!
            .set index, 0
            .rept 4
                ldr t0, [sp, #(vecIndex + (index + 0)*COMPV_GAS_INT32_SZ_BYTES)]
                ldr t1, [sp, #(vecIndex + (index + 1)*COMPV_GAS_INT32_SZ_BYTES)]
                ldr t2, [sp, #(vecIndex + (index + 2)*COMPV_GAS_INT32_SZ_BYTES)]
                ldr t3, [sp, #(vecIndex + (index + 3)*COMPV_GAS_INT32_SZ_BYTES)]
                add t0, img_center, t0
                add t1, img_center, t1
                add t2, img_center, t2
                add t3, img_center, t3
                ldrb t0, [t0]
                ldrb t1, [t1]
                ldrb t2, [t2]
                ldrb t3, [t3]
                orr t0, t0, t1, LSL #(1*8)
                orr t0, t0, t2, LSL #(2*8)
                orr t0, t0, t3, LSL #(3*8)
                .if xy == 0
                    str t0, [sp, #(vecA + (index*COMPV_GAS_UINT8_SZ_BYTES))]
                .else
                    str t0, [sp, #(vecB + (index*COMPV_GAS_UINT8_SZ_BYTES))]
                .endif
                .set index, index+4
            .endr @@ .rep index @@
        .set xy, xy+1
        .endr @@ .rep xy @@

        add t4, sp, #vecA
        vld1.u8 { q10, q11 }, [t4 :128]
        vclt.u8 q10, q10, q11
        vand.u8 q10, q10, vecMask
        vpadd.u8 q10x, q10x, q10y
        vpadd.u8 q10x, q10x, q10x
        vpadd.u8 q10x, q10x, q10x
        vmov.u16 t3, q10x[0]
        strh t3, [out], #COMPV_GAS_UINT16_SZ_BYTES

        subs i, i, #16
        bne Loop256_CompVOrbBrief256_31_32f_Asm_NEON32\@
        @@ EndOf_Loop256_CompVOrbBrief256_31_32f_Asm_NEON32 @@

	.unreq img_center
	.unreq kBrief256Pattern31AX
	.unreq kBrief256Pattern31AY
    .unreq kBrief256Pattern31BX
    .unreq kBrief256Pattern31BY
    .unreq out

    #undef argi_img_stride
    #undef argi_cos1
    #undef argi_sin1

    .unreq i
    .unreq t0
    .unreq t1
    .unreq t2
    .unreq t3
    .unreq t4

    .unreq vecMask
    .unreq vecMaskx
    .unreq vecMasky
    .unreq vecCosT
	.unreq vecCosTx
    .unreq vecSinT
	.unreq vecSinTx
    .unreq vecStride
	.unreq vecStridex

    COMPV_GAS_MEMFREE ((16*COMPV_GAS_INT32_SZ_BYTES) + (16*COMPV_GAS_UINT8_SZ_BYTES) + (16*COMPV_GAS_UINT8_SZ_BYTES))
	COMPV_GAS_UNALIGN_STACK r11
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 9
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVOrbBrief256_31_32f_Asm_NEON32
    CompVOrbBrief256_31_32f_Macro_NEON32 0

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVOrbBrief256_31_32f_Asm_FMA_NEON32
    CompVOrbBrief256_31_32f_Macro_NEON32 1


#endif /* defined(__arm__) && !defined(__aarch64__) */

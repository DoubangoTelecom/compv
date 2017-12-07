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
# arg(0) -> const uint8_t* img_center
# arg(1) -> compv_uscalar_t img_stride
# arg(2) -> const compv_float32_t* cos1
# arg(3) -> const compv_float32_t* sin1
# arg(4) -> COMPV_ALIGNED(NEON) const compv_float32_t* kBrief256Pattern31AX
# arg(5) -> COMPV_ALIGNED(NEON) const compv_float32_t* kBrief256Pattern31AY
# arg(6) -> COMPV_ALIGNED(NEON) const compv_float32_t* kBrief256Pattern31BX
# arg(7) -> COMPV_ALIGNED(NEON) const compv_float32_t* kBrief256Pattern31BY
# arg(8) -> void* out
.macro CompVOrbBrief256_31_32f_Macro_NEON64 fusedMultiplyAdd
   COMPV_GAS_FUNCTION_PROLOG
   COMPV_GAS_SAVE_NEON_REGS
   COMPV_GAS_ALIGN_STACK 16, r11
   COMPV_GAS_MEMALLOC (16*COMPV_GAS_INT32_SZ_BYTES)

   .equ vecIndex		, 0

	## Load arguments ##
   ldr r8, [bp, #(prolog_bytes + (0*COMPV_GAS_REG_SZ_BYTES))]
   img_center .req r0
   img_stride .req r1
   cos1 .req r2
   sin1 .req r3
   kBrief256Pattern31AX .req r4
   kBrief256Pattern31AY .req r5
   kBrief256Pattern31BX .req r6
   kBrief256Pattern31BY .req r7
   out .req r8
   # r9, r10 and r11 are free

   #define argi_img_stride 1
   #define argi_cos1 2
   #define argi_sin1 3
   #define img_stridew r1w

   vecMask .req v0
   vecCosT .req v1
   vecSinT .req v2
   vecStride .req v3
   vecA .req v26
   vecB .req v27

   ldr r9w, [cos1]
   ldr r10w, [sin1]
   dup vecStride.4s, img_stridew
   dup vecCosT.4s, r9w
   dup vecSinT.4s, r10w

   .equ mask0, 0x8040
   .equ mask1, 0x2010
   .equ mask2, 0x804
   .equ mask3, 0x201
   movz r9, #mask0, lsl #48
   movk r9, #mask1, lsl #32
   movk r9, #mask2, lsl #16
   movk r9, #mask3
   dup vecMask.2d, r9

   .unreq img_stride
   .unreq cos1
   .unreq sin1
   i .req r1 // was img_stride
   t0 .req r2 // was cos1
   t1 .req r3 // was sin1
   t2 .req r9
   t3 .req r10
   t4 .req r11

   ###########################################################
   # for (size_t i = 0; i < 256; i += 16)
   ###########################################################
   mov i, #256
   Loop256_CompVOrbBrief256_31_32f_Asm_NEON64\@:
       .set xy, 0
       .rept 2
           .if xy == 0
               ldp q12, q13, [kBrief256Pattern31AX], #32
               ldp q14, q15, [kBrief256Pattern31AX], #32
           .else
               ldp q12, q13, [kBrief256Pattern31BX], #32
               ldp q14, q15, [kBrief256Pattern31BX], #32
           .endif
           fmul v4.4s, v12.4s, vecCosT.s[0]
           fmul v5.4s, v13.4s, vecCosT.s[0]
           fmul v6.4s, v14.4s, vecCosT.s[0]
           fmul v7.4s, v15.4s, vecCosT.s[0]
           fmul v8.4s, v12.4s, vecSinT.s[0]
           fmul v9.4s, v13.4s, vecSinT.s[0]
           fmul v10.4s, v14.4s, vecSinT.s[0]
           fmul v11.4s, v15.4s, vecSinT.s[0]
           .if xy == 0
               ldp q12, q13, [kBrief256Pattern31AY], #32
               ldp q14, q15, [kBrief256Pattern31AY], #32
           .else
               ldp q12, q13, [kBrief256Pattern31BY], #32
               ldp q14, q15, [kBrief256Pattern31BY], #32
           .endif
           .if \fusedMultiplyAdd
                fmls v4.4s, v12.4s, vecSinT.s[0]
                fmls v5.4s, v13.4s, vecSinT.s[0]
                fmls v6.4s, v14.4s, vecSinT.s[0]
                fmls v7.4s, v15.4s, vecSinT.s[0]
                fmla v8.4s, v12.4s, vecCosT.s[0]
                fmla v9.4s, v13.4s, vecCosT.s[0]
                fmla v10.4s, v14.4s, vecCosT.s[0]
                fmla v11.4s, v15.4s, vecCosT.s[0]
            .else
                fmul v16.4s, v12.4s, vecSinT.s[0]
                fmul v17.4s, v13.4s, vecSinT.s[0]
                fmul v18.4s, v14.4s, vecSinT.s[0]
                fmul v19.4s, v15.4s, vecSinT.s[0]
                fmul v20.4s, v12.4s, vecCosT.s[0]
                fmul v21.4s, v13.4s, vecCosT.s[0]
                fmul v22.4s, v14.4s, vecCosT.s[0]
                fmul v23.4s, v15.4s, vecCosT.s[0]
                fsub v4.4s, v4.4s, v16.4s
                fsub v5.4s, v5.4s, v17.4s
                fsub v6.4s, v6.4s, v18.4s
                fsub v7.4s, v7.4s, v19.4s
                fadd v8.4s, v8.4s, v20.4s
                fadd v9.4s, v9.4s, v21.4s
                fadd v10.4s, v10.4s, v22.4s
                fadd v11.4s, v11.4s, v23.4s 
           .endif
           fcvtas v4.4s, v4.4s
           fcvtas v8.4s, v8.4s
           fcvtas v5.4s, v5.4s
           fcvtas v9.4s, v9.4s
           fcvtas v6.4s, v6.4s
           fcvtas v10.4s, v10.4s
           fcvtas v7.4s, v7.4s
           fcvtas v11.4s, v11.4s
           mla v4.4s, v8.4s, vecStride.s[0]
           mla v5.4s, v9.4s, vecStride.s[0]
           mla v6.4s, v10.4s, vecStride.s[0]
           mla v7.4s, v11.4s, vecStride.s[0]
           stp q4, q5, [sp, #(vecIndex + (0*COMPV_GAS_INT32_SZ_BYTES))]
           stp q6, q7, [sp, #(vecIndex + (8*COMPV_GAS_INT32_SZ_BYTES))]
           ldp r9w, r10w, [sp, #(vecIndex + (0*COMPV_GAS_INT32_SZ_BYTES))]
           ldp r11w, r12w, [sp, #(vecIndex + (2*COMPV_GAS_INT32_SZ_BYTES))]
           ldp r13w, r14w, [sp, #(vecIndex + (4*COMPV_GAS_INT32_SZ_BYTES))]
           ldp r15w, r16w, [sp, #(vecIndex + (6*COMPV_GAS_INT32_SZ_BYTES))]
           ldp r19w, r20w, [sp, #(vecIndex + (8*COMPV_GAS_INT32_SZ_BYTES))]
           ldp r21w, r22w, [sp, #(vecIndex + (10*COMPV_GAS_INT32_SZ_BYTES))]
           ldp r23w, r24w, [sp, #(vecIndex + (12*COMPV_GAS_INT32_SZ_BYTES))]
           ldp r25w, r26w, [sp, #(vecIndex + (14*COMPV_GAS_INT32_SZ_BYTES))]
           add r9, img_center, r9w, SXTW
           add r10, img_center, r10w, SXTW
           add r11, img_center, r11w, SXTW
           add r12, img_center, r12w, SXTW
           add r13, img_center, r13w, SXTW
           add r14, img_center, r14w, SXTW
           add r15, img_center, r15w, SXTW
           add r16, img_center, r16w, SXTW
           add r19, img_center, r19w, SXTW
           add r20, img_center, r20w, SXTW
           add r21, img_center, r21w, SXTW
           add r22, img_center, r22w, SXTW
           add r23, img_center, r23w, SXTW
           add r24, img_center, r24w, SXTW
           add r25, img_center, r25w, SXTW
           add r26, img_center, r26w, SXTW
           .if xy == 0
                ld1 {vecA.b}[0], [r9]
                ld1 {vecA.b}[1], [r10]
                ld1 {vecA.b}[2], [r11]
                ld1 {vecA.b}[3], [r12]
                ld1 {vecA.b}[4], [r13]
                ld1 {vecA.b}[5], [r14]
                ld1 {vecA.b}[6], [r15]
                ld1 {vecA.b}[7], [r16]
                ld1 {vecA.b}[8], [r19]
                ld1 {vecA.b}[9], [r20]
                ld1 {vecA.b}[10], [r21]
                ld1 {vecA.b}[11], [r22]
                ld1 {vecA.b}[12], [r23]
                ld1 {vecA.b}[13], [r24]
                ld1 {vecA.b}[14], [r25]
                ld1 {vecA.b}[15], [r26]
           .else
                ld1 {vecB.b}[0], [r9]
                ld1 {vecB.b}[1], [r10]
                ld1 {vecB.b}[2], [r11]
                ld1 {vecB.b}[3], [r12]
                ld1 {vecB.b}[4], [r13]
                ld1 {vecB.b}[5], [r14]
                ld1 {vecB.b}[6], [r15]
                ld1 {vecB.b}[7], [r16]
                ld1 {vecB.b}[8], [r19]
                ld1 {vecB.b}[9], [r20]
                ld1 {vecB.b}[10], [r21]
                ld1 {vecB.b}[11], [r22]
                ld1 {vecB.b}[12], [r23]
                ld1 {vecB.b}[13], [r24]
                ld1 {vecB.b}[14], [r25]
                ld1 {vecB.b}[15], [r26]
           .endif
       .set xy, xy+1
       .endr // .rep xy //
       
       cmhi v10.16b, vecB.16b, vecA.16b
       and v10.16b, v10.16b, vecMask.16b
       addp v10.16b, v10.16b, v10.16b
       addp v10.8b, v10.8b, v10.8b
       addp v10.8b, v10.8b, v10.8b
       st1 {v10.h}[0], [out], #COMPV_GAS_UINT16_SZ_BYTES
       
       subs i, i, #16
       bne Loop256_CompVOrbBrief256_31_32f_Asm_NEON64\@
       ## EndOf_Loop256_CompVOrbBrief256_31_32f_Asm_NEON64 ##

   .unreq img_center
   .unreq kBrief256Pattern31AX
   .unreq kBrief256Pattern31AY
   .unreq kBrief256Pattern31BX
   .unreq kBrief256Pattern31BY
   .unreq out

   #undef argi_img_stride
   #undef argi_cos1
   #undef argi_sin1
   #undef img_stridew

   .unreq i
   .unreq t0
   .unreq t1
   .unreq t2
   .unreq t3
   .unreq t4

   .unreq vecMask
   .unreq vecCosT
   .unreq vecSinT
   .unreq vecStride
   .unreq vecA
   .unreq vecB


   COMPV_GAS_MEMFREE (16*COMPV_GAS_INT32_SZ_BYTES)
   COMPV_GAS_UNALIGN_STACK r9, r10 
   COMPV_GAS_RESTORE_NEON_REGS
   COMPV_GAS_FUNCTION_EPILOG
   COMPV_GAS_FUNCTION_RETURN
.endm


#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVOrbBrief256_31_32f_Asm_NEON64
    CompVOrbBrief256_31_32f_Macro_NEON64 0

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVOrbBrief256_31_32f_Asm_FMA_NEON64
    CompVOrbBrief256_31_32f_Macro_NEON64 1

#endif /* defined(__aarch64__) */

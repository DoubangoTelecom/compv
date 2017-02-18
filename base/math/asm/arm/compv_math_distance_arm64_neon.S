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


###########################################################
# Overrides q13, q14 and returns the result in q15
.macro __hamming4x16 vecPatch
    ld1 { vec0.16b }, [dataPtr], #16
    ld1 { vec1.16b }, [t0 ], #16 // t0 = dataPtr[stride * 1]
    ld1 { vec2.16b }, [t1], #16 // t1 = dataPtr[stride * 2]
    ld1 { vec3.16b }, [t2], #16 // t2 = dataPtr[stride * 3]
    eor vec0.16b, vec0.16b, \vecPatch
    eor vec1.16b, vec1.16b, \vecPatch
    eor vec2.16b, vec2.16b, \vecPatch
    eor vec3.16b, vec3.16b, \vecPatch
    cnt vec0.16b, vec0.16b
    cnt vec1.16b, vec1.16b
    cnt vec2.16b, vec2.16b
    cnt vec3.16b, vec3.16b
    addp v13.16b, vec0.16b, vec1.16b
    addp v14.16b, vec2.16b, vec3.16b
    addp v15.16b, v13.16b, v14.16b
.endm

###########################################################
# Overrides q13, q14 and returns the result in q15
.macro __hamming4x16_orphans vecPatch
    ld1 { vec0.16b }, [dataPtr], #16
    ld1 { vec1.16b }, [t0], #16 // t0 = dataPtr[stride * 1]
    ld1 { vec2.16b }, [t1], #16 // t1 = dataPtr[stride * 2]
    ld1 { vec3.16b }, [t2], #16 // t2 = dataPtr[stride * 3]
    eor vec0.16b, vec0.16b, \vecPatch
    eor vec1.16b, vec1.16b, \vecPatch
    eor vec2.16b, vec2.16b, \vecPatch
    eor vec3.16b, vec3.16b, \vecPatch
    and vec0.16b, vec0.16b, vecMask.16b
    and vec1.16b, vec1.16b, vecMask.16b
    and vec2.16b, vec2.16b, vecMask.16b
    and vec3.16b, vec3.16b, vecMask.16b
    cnt vec0.16b, vec0.16b
    cnt vec1.16b, vec1.16b
    cnt vec2.16b, vec2.16b
    cnt vec3.16b, vec3.16b
    addp v13.16b, vec0.16b, vec1.16b
    addp v14.16b, vec2.16b, vec3.16b
    addp v15.16b, v13.16b, v14.16b
.endm

###########################################################
#  Overrides q9..q14 and returns the result in q15
.macro __hamming1x64
    ld1 { vec0.16b, vec1.16b }, [dataPtr], #32
    ld1 { vec2.16b, vec3.16b }, [dataPtr], #32
    ld1 { v9.16b, v10.16b }, [patch1xnPtr], #32
    ld1 { v11.16b, v12.16b }, [patch1xnPtr], #32
    eor vec0.16b, vec0.16b, v9.16b
    eor vec1.16b, vec1.16b, v10.16b
    eor vec2.16b, vec2.16b, v11.16b
    eor vec3.16b, vec3.16b, v12.16b
    cnt vec0.16b, vec0.16b
    cnt vec1.16b, vec1.16b
    cnt vec2.16b, vec2.16b
    cnt vec3.16b, vec3.16b
    addp v13.16b, vec0.16b, vec1.16b
    addp v14.16b, vec2.16b, vec3.16b
    addp v15.16b, v13.16b, v14.16b
.endm

###########################################################
# Overrides q13, q14 and returns the result in q15
.macro __hamming1x16
    ld1 { vec0.16b }, [dataPtr], #16
    ld1 { v13.16b }, [patch1xnPtr], #16
    eor v14.16b, vec0.16b, v13.16b
    cnt v15.16b, v14.16b
.endm

###########################################################
# Overrides q12, q13, q14 and returns the result in q15
.macro __hamming1x16_orphans
    ld1 { vec0.16b }, [dataPtr], #16
    ld1 { v12.16b }, [patch1xnPtr], #16
    eor v13.16b, vec0.16b, v12.16b
    and v14.16b, v13.16b, vecMask.16b
    cnt v15.16b, v14.16b
.endm

#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* dataPtr
# arg(1) -> compv_uscalar_t width
# arg(2) -> compv_uscalar_t height
# arg(3) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
# arg(4) -> COMPV_ALIGNED(NEON) const uint8_t* patch1xnPtr
# arg(5) -> int22_t* distPtr
COMPV_GAS_FUNCTION_DECLARE CompVMathDistanceHamming_Asm_NEON64
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS
	
	## Set arguments ##
	dataPtr .req r0
	width .req r1
	height .req r2
	stride .req r3
	patch1xnPtr_ .req r4
	distPtr .req r5

    i .req r6
    j .req r7
    pad .req r8
    t0 .req r9
    t1 .req r10
    t2 .req r11
    patch1xnPtr .req r12
    t3 .req r13
    t4 .req r15

    vec0 .req v0
    vec1 .req v1
    vec2 .req v2
    vec3 .req v3
    veccnt .req v4
    vecPatch .req v5
    vecMask .req v6

    # compute pad
    # maxI = ((width + 15) & -16), pad = (stride - maxI)
    add pad, width, #15
    and pad, pad, #-16
    sub pad, stride, pad    

    ###########################################################
    # .if (height > 3)
    ###########################################################
    lsr j, height, #2 // div 4
    cbz j, EndOf_IfHeightGt4_CompVMathDistanceHamming_Asm_NEON64
    IfHeightGt4_CompVMathDistanceHamming_Asm_NEON64:
        ###########################################################
        # for (; j < height - 3; j += 4)
        ###########################################################
        LoopH4_CompVMathDistanceHamming_Asm_NEON64:
            movi veccnt.4s, #0
            mov patch1xnPtr, patch1xnPtr_
            lsr i, width, #4 // div 16 (not need to test, width is always > 15)
            add t0, dataPtr, stride // t0 = dataPtr[stride * 1]
            add t1, dataPtr, stride, LSL #1 // t1 = dataPtr[stride * 2]
            add t2, t0, stride, LSL #1 // t2 = dataPtr[stride * 3]
            ###########################################################
            # for (; i < width - 15; i += 16)
            ###########################################################
            LoopH4W16_CompVMathDistanceHamming_Asm_NEON64:
                ld1 { vecPatch.16b }, [patch1xnPtr], #16
                __hamming4x16 vecPatch.16b
                addp v10.16b, v15.16b, v15.16b
                uaddlp v11.4h, v10.8b
                uaddw veccnt.4s, veccnt.4s, v11.4h
                subs i, i, #1
                bne LoopH4W16_CompVMathDistanceHamming_Asm_NEON64
                ## EndOf_LoopH4W16_CompVMathDistanceHamming_Asm_NEON64 ##

            ###########################################################
            # .if (orphans =  static_cast<compv_scalar_t>(width & 15))
            ###########################################################
            ands t4, width, 15
            beq EndOf_IfH4Orphans_CompVMathDistanceHamming_Asm_NEON64
            IfH4Orphans_CompVMathDistanceHamming_Asm_NEON64:
                mov t3, #0
                ld1 { vecPatch.16b }, [patch1xnPtr], #16
                cmeq vecMask.16b, vec0.16b, vec0.16b
                eor v15.16b, v15.16b, v15.16b // TODO(dmi): for ARM64 no need to set zero and change next 'vmov.s32' to 'vmov.s64'
                sub t4, t3, t4, LSL #3 // t4 = ((0 - t0) << 3) = -t0 << 3
                cmp t4, #-64
                bge NotLtMinus64_IfH4Orphans_CompVMathDistanceHamming_Asm_NEON64
                    add t3, t4, #64 // t3 = 0 if (t4 < -64) otherwise unchanged (#0)
                NotLtMinus64_IfH4Orphans_CompVMathDistanceHamming_Asm_NEON64:
                mov v15.d[1], t4
                mov v15.d[0], t3
                ushl vecMask.16b, vecMask.16b, v15.16b
                __hamming4x16_orphans vecPatch.16b
                addp v10.16b, v15.16b, v15.16b
                uaddlp v11.4h, v10.8b
                uaddw veccnt.4s, veccnt.4s, v11.4h
                EndOf_IfH4Orphans_CompVMathDistanceHamming_Asm_NEON64:
                ## EndOf_IfH4Orphans_CompVMathDistanceHamming_Asm_NEON64 ##

            
            st1 { veccnt.4s }, [distPtr], #16
            add dataPtr, t2, pad // t2 = dataPtr[stride * 3] 
            subs j, j, #1
            bne LoopH4_CompVMathDistanceHamming_Asm_NEON64
            ## EnofOf_LoopH4_CompVMathDistanceHamming_Asm_NEON64 ##

        EndOf_IfHeightGt4_CompVMathDistanceHamming_Asm_NEON64:


    ###########################################################
    # for (; j < height; j += 1)
    ###########################################################
    ands j, height, #3 // modulo 4
    beq EndOf_LoopH1_CompVMathDistanceHamming_Asm_NEON64
    LoopH1_CompVMathDistanceHamming_Asm_NEON64:
        movi veccnt.4s, #0
        mov patch1xnPtr, patch1xnPtr_
        lsr i, width, #6 // div 64
        cbz i, EndOf_LoopH1W64_CompVMathDistanceHamming_Asm_NEON64
        ###########################################################
        # for (; i < width - 63; i += 64)
        ###########################################################
        LoopH1W64_CompVMathDistanceHamming_Asm_NEON64:
            __hamming1x64
            addp v10.16b, v15.16b, v15.16b
            uaddlp v11.4h, v10.8b
            uaddw veccnt.4s, veccnt.4s, v11.4h
            subs i, i, #1
            bne LoopH1W64_CompVMathDistanceHamming_Asm_NEON64
            EndOf_LoopH1W64_CompVMathDistanceHamming_Asm_NEON64:
            ## EndOf_LoopH1W64_CompVMathDistanceHamming_Asm_NEON64 ##

        
        ###########################################################
        # for (; i < width - 15; i += 16)
        ###########################################################
        and t0, width, #63 // modulo 64 
        lsr i, t0, #4 // div 16
        cbz i, EndOf_LoopH1W16_CompVMathDistanceHamming_Asm_NEON64
        LoopH1W16_CompVMathDistanceHamming_Asm_NEON64:
            __hamming1x16
            addp v10.16b, v15.16b, v15.16b
            uaddlp v11.4h, v10.8b
            uaddw veccnt.4s, veccnt.4s, v11.4h
            subs i, i, #1
            bne LoopH1W16_CompVMathDistanceHamming_Asm_NEON64
            EndOf_LoopH1W16_CompVMathDistanceHamming_Asm_NEON64:
            ## EndOf_LoopH1W16_CompVMathDistanceHamming_Asm_NEON64 ##


        ###########################################################
        # .if (orphans =  static_cast<compv_scalar_t>(width & 15))
        ###########################################################
        ands t4, width, 15
        beq EndOf_IfH1Orphans_CompVMathDistanceHamming_Asm_NEON64
        IfH1Orphans_CompVMathDistanceHamming_Asm_NEON64:
            mov t3, #0
            cmeq vecMask.16b, vec0.16b, vec0.16b
            eor v15.16b, v15.16b, v15.16b // TODO(dmi): for ARM64 no need to set zero and change next 'vmov.s32' to 'vmov.s64'
            sub t4, t3, t4, LSL #3 // t4 = ((0 - t0) << 3) = -t0 << 3
            cmp t4, #-64
            bge NotLtMinus64_IfH1Orphans_CompVMathDistanceHamming_Asm_NEON64
                add t3, t4, #64 // t3 = 0 if (t4 < -64) otherwise unchanged (#0)
            NotLtMinus64_IfH1Orphans_CompVMathDistanceHamming_Asm_NEON64:
            mov v15.d[1], t4
            mov v15.d[0], t3
            ushl vecMask.16b, vecMask.16b, v15.16b
            __hamming1x16_orphans
            addp v10.16b, v15.16b, v15.16b
            uaddlp v11.4h, v10.8b
            uaddw veccnt.4s, veccnt.4s, v11.4h
            EndOf_IfH1Orphans_CompVMathDistanceHamming_Asm_NEON64:
            ## EndOf_IfH1Orphans_CompVMathDistanceHamming_Asm_NEON64 ##


        # TODO(dmi): not same code as ARM64
        addp v15.4s, veccnt.4s, veccnt.4s
        addp v14.4s, v15.4s, v15.4s
        st1 {v14.s}[0], [distPtr], #4

        add dataPtr, dataPtr, pad
        subs j, j, #1
        bne LoopH1_CompVMathDistanceHamming_Asm_NEON64
        EndOf_LoopH1_CompVMathDistanceHamming_Asm_NEON64:
        ## EndOf_LoopH1_CompVMathDistanceHamming_Asm_NEON64 ##


	.unreq dataPtr
	.unreq width
	.unreq height
	.unreq stride
	.unreq patch1xnPtr_
	.unreq distPtr

    .unreq i
    .unreq j
    .unreq pad
    .unreq t0
    .unreq t0w
    .unreq t1
    .unreq t2
    .unreq t3
    .unreq t4
    .unreq patch1xnPtr

    .unreq vec0
    .unreq vec1
    .unreq vec2
    .unreq vec3
    .unreq veccnt
    .unreq vecPatch
    .unreq vecMask

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__aarch64__) */

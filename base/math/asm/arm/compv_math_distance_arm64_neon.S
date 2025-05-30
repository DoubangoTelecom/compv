#########################################################################
# Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__aarch64__)
.include "compv_common_arm64.S"

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
    eor v16.16b, vec0.16b, \vecPatch
    eor v17.16b, vec1.16b, \vecPatch
    eor v18.16b, vec2.16b, \vecPatch
    eor v19.16b, vec3.16b, \vecPatch
    cnt v20.16b, v16.16b
    cnt v21.16b, v17.16b
    cnt v22.16b, v18.16b
    cnt v23.16b, v19.16b
    addp v13.16b, v20.16b, v21.16b
    addp v14.16b, v22.16b, v23.16b
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
    eor v16.16b, vec0.16b, v9.16b
    eor v17.16b, vec1.16b, v10.16b
    eor v18.16b, vec2.16b, v11.16b
    eor v19.16b, vec3.16b, v12.16b
    cnt v20.16b, v16.16b
    cnt v21.16b, v17.16b
    cnt v22.16b, v18.16b
    cnt v23.16b, v19.16b
    addp v13.16b, v20.16b, v21.16b
    addp v14.16b, v22.16b, v23.16b
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
    t5 .req r16
    t6 .req r17

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

	# compute vecMask for orphans
	ands t4, width, 15
    beq NoOrphans_CompVMathDistanceHamming_Asm_NEON64
		cmeq vecMask.16b, vecMask.16b, vecMask.16b
		mov t3, #-(16<<3)
		mov t6, #0
		add t4, t3, t4, LSL #3 // ((orphans - 16) << 3) = (-16<<3) + (orphans << 3)
		adds t5, t4, #64
		csel t3, t5, t6, mi
		mov v15.d[0], t3
		mov v15.d[1], t4
		ushl vecMask.2d, vecMask.2d, v15.2d
		NoOrphans_CompVMathDistanceHamming_Asm_NEON64:

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
            tst width, 15
            beq EndOf_IfH4Orphans_CompVMathDistanceHamming_Asm_NEON64
            IfH4Orphans_CompVMathDistanceHamming_Asm_NEON64:
                ld1 { vecPatch.16b }, [patch1xnPtr], #16
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
        tst width, 15
        beq EndOf_IfH1Orphans_CompVMathDistanceHamming_Asm_NEON64
        IfH1Orphans_CompVMathDistanceHamming_Asm_NEON64:
            __hamming1x16_orphans
            addp v10.16b, v15.16b, v15.16b
            uaddlp v11.4h, v10.8b
            uaddw veccnt.4s, veccnt.4s, v11.4h
            EndOf_IfH1Orphans_CompVMathDistanceHamming_Asm_NEON64:
            ## EndOf_IfH1Orphans_CompVMathDistanceHamming_Asm_NEON64 ##

        addp v15.4s, veccnt.4s, veccnt.4s
        addp v14.4s, v15.4s, v15.4s
        st1 {v14.s}[0], [distPtr], #COMPV_GAS_INT32_SZ_BYTES

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
    .unreq t1
    .unreq t2
    .unreq t3
    .unreq t4
    .unreq t5
    .unreq t6
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



#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* dataPtr
# arg(1) -> compv_uscalar_t height
# arg(2) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
# arg(3) -> COMPV_ALIGNED(NEON) const uint8_t* patch1xnPtr
# arg(4) -> int22_t* distPtr
COMPV_GAS_FUNCTION_DECLARE CompVMathDistanceHamming32_Asm_NEON64
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS
	
	## Load arguments ##
	dataPtr .req r0
	height .req r1
	stride .req r2
	patch1xnPtr .req r3
	distPtr .req r4

   j .req r5
   pad .req r6
   t0 .req r7
   t1 .req r8
   t2 .req r9

   veccnt .req v0
   vecPatch0 .req v1
   vecPatch1 .req v2

   prfm pldl1keep, [dataPtr, #(CACHE_LINE_SIZE*4)]
   prfm pldl1keep, [dataPtr, #(CACHE_LINE_SIZE*5)]
   prfm pldl1keep, [dataPtr, #(CACHE_LINE_SIZE*6)]
   prfm pldl1keep, [dataPtr, #(CACHE_LINE_SIZE*7)]

   ld1 { vecPatch0.16b, vecPatch1.16b }, [patch1xnPtr], #32
   .unreq patch1xnPtr // patch1xnPtr no longer needed

   # compute pad
   sub pad, stride, 32    

   ###########################################################
   # .if (height > 3)
   ###########################################################
   lsr j, height, #2 // div 4
   cbz j, EndOf_IfHeightGt4_CompVMathDistanceHamming32_Asm_NEON64
   IfHeightGt4_CompVMathDistanceHamming32_Asm_NEON64:
       ###########################################################
       # for (; j < height - 3; j += 4)
       ###########################################################
       LoopH4_CompVMathDistanceHamming32_Asm_NEON64:
           add t0, dataPtr, stride // t0 = dataPtr[stride * 1]
           add t1, dataPtr, stride, LSL #1 // t1 = dataPtr[stride * 2]
           add t2, t0, stride, LSL #1 // t2 = dataPtr[stride * 3]
		   // vcnt has high latency and throughput and this is why they are interleaved with veors (low latency and throughput)
           ldp q4, q5, [dataPtr], #32
           ldp q6, q7, [t0], #32 // t0 = dataPtr[stride * 1]
           eor v4.16b, v4.16b, vecPatch0.16b
           eor v5.16b, v5.16b, vecPatch1.16b
		   cnt v4.16b, v4.16b
           cnt v5.16b, v5.16b
		   prfm pstl1keep, [distPtr, #(CACHE_LINE_SIZE*3)]
           eor v6.16b, v6.16b, vecPatch0.16b
           eor v7.16b, v7.16b, vecPatch1.16b
		   cnt v6.16b, v6.16b
           cnt v7.16b, v7.16b
		   ldp q8, q9, [t1], #32 // t1 = dataPtr[stride * 2]
           ldp q10, q11, [t2], #32 // t2 = dataPtr[stride * 3]
		   prfm pldl1keep, [t2, #(CACHE_LINE_SIZE*3)]
           eor v8.16b, v8.16b, vecPatch0.16b
           eor v9.16b, v9.16b, vecPatch1.16b
		   cnt v8.16b, v8.16b
		   prfm pldl1keep, [t2, #(CACHE_LINE_SIZE*4)]
           cnt v9.16b, v9.16b
		   prfm pldl1keep, [t2, #(CACHE_LINE_SIZE*5)]
           eor v10.16b, v10.16b, vecPatch0.16b    
           eor v11.16b, v11.16b, vecPatch1.16b
           cnt v10.16b, v10.16b
		   prfm pldl1keep, [t2, #(CACHE_LINE_SIZE*6)]
           cnt v11.16b, v11.16b
           addp v12.16b, v4.16b, v6.16b
           addp v14.16b, v5.16b, v7.16b
           addp v13.16b, v8.16b, v10.16b
           addp v15.16b, v9.16b, v11.16b
           addp v4.16b, v12.16b, v13.16b
           addp v5.16b, v14.16b, v15.16b
           addp veccnt.16b, v4.16b, v4.16b
           addp v9.16b, v5.16b, v5.16b
           uaddlp veccnt.4h, veccnt.8b
           uadalp veccnt.4h, v9.8b
           uxtl v14.4s, veccnt.4h
           st1 {v14.4s}, [distPtr], #16
           add dataPtr, t2, pad // t2 = dataPtr[stride * 3] 
           subs j, j, #1
           bne LoopH4_CompVMathDistanceHamming32_Asm_NEON64
           ## EnofOf_LoopH4_CompVMathDistanceHamming32_Asm_NEON64 ##

       EndOf_IfHeightGt4_CompVMathDistanceHamming32_Asm_NEON64:


   ###########################################################
   # for (; j < height; j += 1)
   ###########################################################
   ands j, height, #3 // modulo 4
   beq EndOf_LoopH1_CompVMathDistanceHamming32_Asm_NEON64
   LoopH1_CompVMathDistanceHamming32_Asm_NEON64:
       ldp q4, q5, [dataPtr], #32
       eor v4.16b, v4.16b, vecPatch0.16b
	   cnt v4.16b, v4.16b
       eor v5.16b, v5.16b, vecPatch1.16b
       cnt v5.16b, v5.16b
       addp v6.16b, v4.16b, v5.16b
       addp v7.16b, v6.16b, v6.16b
       uaddlp v8.4h, v7.8b
       uaddlp v9.2s, v8.4h
       uaddlp veccnt.1d, v9.2s
       st1 {veccnt.s}[0], [distPtr], #COMPV_GAS_INT32_SZ_BYTES
       add dataPtr, dataPtr, pad
       subs j, j, #1
       bne LoopH1_CompVMathDistanceHamming32_Asm_NEON64
       EndOf_LoopH1_CompVMathDistanceHamming32_Asm_NEON64:
       ## EndOf_LoopH1_CompVMathDistanceHamming33_Asm_NEON64 ##

	.unreq dataPtr
	.unreq height
	.unreq stride
	.unreq distPtr

   .unreq j
   .unreq pad
   .unreq t0
   .unreq t1
   .unreq t2

   .unreq veccnt
   .unreq vecPatch0
   .unreq vecPatch1

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) const compv_float32_t* xPtr, 
# arg(1) -> COMPV_ALIGNED(NEON) const compv_float32_t* yPtr, 
# arg(2) -> const compv_float32_t* Ascaled1, 
# arg(3) -> const compv_float32_t* Bscaled1, 
# arg(4) -> const compv_float32_t* Cscaled1, 
# arg(5) -> COMPV_ALIGNED(NEON) compv_float32_t* distPtr, 
# arg(6) -> const compv_uscalar_t count
.macro CompVMathDistanceLine_32f_Macro_NEON64 fusedMultiplyAdd
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Load arguments ##
	xPtr .req r0
	yPtr .req r1
	Ascaled1 .req r2
	Bscaled1 .req r3
	Cscaled1 .req r4
	distPtr .req r5
	count .req r6

	prfm pldl1keep, [xPtr, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [xPtr, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [xPtr, #(CACHE_LINE_SIZE*2)]
	prfm pldl1keep, [xPtr, #(CACHE_LINE_SIZE*3)]

	prfm pldl1keep, [yPtr, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [yPtr, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [yPtr, #(CACHE_LINE_SIZE*2)]
	prfm pldl1keep, [yPtr, #(CACHE_LINE_SIZE*3)]

	# Local variables
	i .req r7
	vecA .req v0
	vecB .req v1
	vecC .req v2

	ldr r8w, [Ascaled1]
	ldr r9w, [Bscaled1]
	ldr r10w, [Cscaled1]
	dup vecA.4s, r8w
	dup vecB.4s, r9w
	dup vecC.4s, r10w

	########################################
	# for (i = 0; i < count16; i += 16)
	########################################
	ands i, count, #-16
	beq EndOf_LoopCount16_CompVMathDistanceLine_32f_Asm_NEON64\@
	LoopCount16_CompVMathDistanceLine_32f_Asm_NEON64\@:
		ld1 { v4.4s-v7.4s }, [xPtr], #(4*COMPV_GAS_V_SZ_BYTES)
		prfm pldl1keep, [xPtr, #(CACHE_LINE_SIZE*4)]
		.if \fusedMultiplyAdd
			mov v8.16b, vecC.16b
			mov v9.16b, vecC.16b
			fmla v8.4s, vecA.4s, v4.4s
			fmla v9.4s, vecA.4s, v5.4s
			mov v10.16b, vecC.16b
			mov v11.16b, vecC.16b
			fmla v10.4s, vecA.4s, v6.4s
			fmla v11.4s, vecA.4s, v7.4s
			ld1 { v12.4s-v15.4s }, [yPtr], #(4*COMPV_GAS_V_SZ_BYTES)
			prfm pldl1keep, [yPtr, #(CACHE_LINE_SIZE*4)]
			fmla v8.4s, vecB.4s, v12.4s
			fmla v9.4s, vecB.4s, v13.4s
			fmla v10.4s, vecB.4s, v14.4s
			fmla v11.4s, vecB.4s, v15.4s
		.else
			fmul v8.4s, vecA.4s, v4.4s
			fmul v9.4s, vecA.4s, v5.4s
			fmul v10.4s, vecA.4s, v6.4s
			fmul v11.4s, vecA.4s, v7.4s
			ld1 { v12.4s-v15.4s }, [yPtr], #(4*COMPV_GAS_V_SZ_BYTES)
			prfm pldl1keep, [yPtr, #(CACHE_LINE_SIZE*4)]
			fmul v12.4s, v12.4s, vecB.4s
			fmul v13.4s, v13.4s, vecB.4s
			fmul v14.4s, v14.4s, vecB.4s
			fmul v15.4s, v15.4s, vecB.4s
			fadd v8.4s, v8.4s, vecC.4s
			fadd v9.4s, v9.4s, vecC.4s
			fadd v10.4s, v10.4s, vecC.4s
			fadd v11.4s, v11.4s, vecC.4s
			fadd v8.4s, v8.4s, v12.4s
			fadd v9.4s, v9.4s, v13.4s
			fadd v10.4s, v10.4s, v14.4s
			fadd v11.4s, v11.4s, v15.4s
		.endif
		subs i, i, #16
		fabs v8.4s, v8.4s
		fabs v9.4s, v9.4s
		fabs v10.4s, v10.4s
		fabs v11.4s, v11.4s
		st1 {v8.4s-v11.4s}, [distPtr], #(4*COMPV_GAS_V_SZ_BYTES)
		bne LoopCount16_CompVMathDistanceLine_32f_Asm_NEON64\@
	EndOf_LoopCount16_CompVMathDistanceLine_32f_Asm_NEON64\@:

	################################
	# for (; i < count; i += 4)
	################################
	ands i, count, #15
	beq EndOf_LoopCount4_CompVMathDistanceLine_32f_Asm_NEON64\@
	LoopCount4_CompVMathDistanceLine_32f_Asm_NEON64\@:
		ld1 { v4.4s }, [xPtr], #(1*COMPV_GAS_V_SZ_BYTES)
		.if \fusedMultiplyAdd
			mov v8.16b, vecC.16b
			fmla v8.4s, vecA.4s, v4.4s
			ld1 { v12.4s }, [yPtr], #(1*COMPV_GAS_V_SZ_BYTES)
			fmla v8.4s, vecB.4s, v12.4s
		.else
			fmul v8.4s, vecA.4s, v4.4s
			ld1 { v12.4s }, [yPtr], #(1*COMPV_GAS_V_SZ_BYTES)
			fmul v12.4s, v12.4s, vecB.4s
			fadd v8.4s, v8.4s, vecC.4s
			fadd v8.4s, v8.4s, v12.4s
		.endif
		subs i, i, #4
		fabs v8.4s, v8.4s
		st1 {v8.4s}, [distPtr],	#(1*COMPV_GAS_V_SZ_BYTES)
		bgt LoopCount4_CompVMathDistanceLine_32f_Asm_NEON64\@
	EndOf_LoopCount4_CompVMathDistanceLine_32f_Asm_NEON64\@:
	
	.unreq xPtr
	.unreq yPtr
	.unreq Ascaled1
	.unreq Bscaled1
	.unreq Cscaled1
	.unreq distPtr
	.unreq count

	.unreq i
	.unreq vecA
	.unreq vecB
	.unreq vecC

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVMathDistanceLine_32f_Asm_NEON64
	CompVMathDistanceLine_32f_Macro_NEON64 0

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVMathDistanceLine_32f_Asm_FMA_NEON64
	CompVMathDistanceLine_32f_Macro_NEON64 1

#########################################################################
# arg(0) -> COMPV_ALIGNED(NEON) const compv_float32_t* xPtr, 
# arg(1) -> COMPV_ALIGNED(NEON) const compv_float32_t* yPtr, 
# arg(2) -> const compv_float32_t* A1, 
# arg(3) -> const compv_float32_t* B1, 
# arg(4) -> const compv_float32_t* C1, 
# arg(5) -> COMPV_ALIGNED(NEON) compv_float32_t* distPtr, 
# arg(6) -> const compv_uscalar_t count
.macro CompVMathDistanceParabola_32f_Macro_NEON64 fusedMultiplyAdd
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Load arguments ##
	xPtr .req r0
	yPtr .req r1
	A1Ptr .req r2
	B1Ptr .req r3
	C1Ptr .req r4
	distPtr .req r5
	count .req r6

	prfm pldl1keep, [xPtr, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [xPtr, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [xPtr, #(CACHE_LINE_SIZE*2)]
	prfm pldl1keep, [xPtr, #(CACHE_LINE_SIZE*3)]

	prfm pldl1keep, [yPtr, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [yPtr, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [yPtr, #(CACHE_LINE_SIZE*2)]
	prfm pldl1keep, [yPtr, #(CACHE_LINE_SIZE*3)]

	# Local variables
	i .req r7
	vecA .req v0
	vecB .req v1
	vecC .req v2

	ldr r8w, [A1Ptr]
	ldr r9w, [B1Ptr]
	ldr r10w, [C1Ptr]
	dup vecA.4s, r8w
	dup vecB.4s, r9w
	dup vecC.4s, r10w

	########################################
	# for (i = 0; i < count16; i += 16)
	########################################
	ands i, count, #-16
	beq EndOf_LoopCount16_CompVMathDistanceParabola_32f_Asm_NEON64\@
	LoopCount16_CompVMathDistanceParabola_32f_Asm_NEON64\@:
		ld1 { v4.4s-v7.4s }, [xPtr], #(4*COMPV_GAS_V_SZ_BYTES)
		prfm pldl1keep, [xPtr, #(CACHE_LINE_SIZE*4)]
		.if \fusedMultiplyAdd
			fmul v12.4s, v4.4s, v4.4s
			fmul v13.4s, v5.4s, v5.4s
			fmul v14.4s, v6.4s, v6.4s
			fmul v15.4s, v7.4s, v7.4s
			mov v8.16b, vecC.16b
			mov v9.16b, vecC.16b
			fmla v8.4s, vecA.4s, v12.4s
			fmla v9.4s, vecA.4s, v13.4s
			mov v10.16b, vecC.16b
			mov v11.16b, vecC.16b
			fmla v10.4s, vecA.4s, v14.4s
			fmla v11.4s, vecA.4s, v15.4s
			ld1 { v12.4s-v15.4s }, [yPtr], #(4*COMPV_GAS_V_SZ_BYTES)
			prfm pldl1keep, [yPtr, #(CACHE_LINE_SIZE*4)]
			fmla v8.4s, vecB.4s, v4.4s
			fmla v9.4s, vecB.4s, v5.4s
			fmla v10.4s, vecB.4s, v6.4s
			fmla v11.4s, vecB.4s, v7.4s
			fsub v8.4s, v8.4s, v12.4s
			fsub v9.4s, v9.4s, v13.4s
			fsub v10.4s, v10.4s, v14.4s
			fsub v11.4s, v11.4s, v15.4s
		.else
			fmul v8.4s, v4.4s, v4.4s
			fmul v9.4s, v5.4s, v5.4s
			fmul v10.4s, v6.4s, v6.4s
			fmul v11.4s, v7.4s, v7.4s
			fmul v4.4s, vecB.4s, v4.4s
			fmul v5.4s, vecB.4s, v5.4s
			fmul v6.4s, vecB.4s, v6.4s
			fmul v7.4s, vecB.4s, v7.4s
			fmul v8.4s, v8.4s, vecA.4s
			fmul v9.4s, v9.4s, vecA.4s
			fmul v10.4s, v10.4s, vecA.4s
			fmul v11.4s, v11.4s, vecA.4s
			ld1 { v12.4s-v15.4s }, [yPtr], #(4*COMPV_GAS_V_SZ_BYTES)
			prfm pldl1keep, [yPtr, #(CACHE_LINE_SIZE*4)]
			fadd v8.4s, v8.4s, vecC.4s
			fadd v9.4s, v9.4s, vecC.4s
			fadd v10.4s, v10.4s, vecC.4s
			fadd v11.4s, v11.4s, vecC.4s
			fadd v8.4s, v8.4s, v4.4s
			fadd v9.4s, v9.4s, v5.4s
			fadd v10.4s, v10.4s, v6.4s
			fadd v11.4s, v11.4s, v7.4s
			fsub v8.4s, v8.4s, v12.4s
			fsub v9.4s, v9.4s, v13.4s
			fsub v10.4s, v10.4s, v14.4s
			fsub v11.4s, v11.4s, v15.4s
		.endif
		subs i, i, #16
		fabs v8.4s, v8.4s
		fabs v9.4s, v9.4s
		fabs v10.4s, v10.4s
		fabs v11.4s, v11.4s
		st1 {v8.4s-v11.4s}, [distPtr], #(4*COMPV_GAS_V_SZ_BYTES)
		bne LoopCount16_CompVMathDistanceParabola_32f_Asm_NEON64\@
	EndOf_LoopCount16_CompVMathDistanceParabola_32f_Asm_NEON64\@:

	################################
	# for (; i < count; i += 4)
	################################
	ands i, count, #15
	beq EndOf_LoopCount4_CompVMathDistanceParabola_32f_Asm_NEON64\@
	LoopCount4_CompVMathDistanceParabola_32f_Asm_NEON64\@:
		ld1 { v4.4s }, [xPtr], #(1*COMPV_GAS_V_SZ_BYTES)
		.if \fusedMultiplyAdd
			mov v8.16b, vecC.16b
			fmul v12.4s, v4.4s, v4.4s
			fmla v8.4s, vecA.4s, v12.4s
			ld1 { v12.4s }, [yPtr], #(1*COMPV_GAS_V_SZ_BYTES)
			fmla v8.4s, vecB.4s, v4.4s
			fsub v8.4s, v8.4s, v12.4s
		.else
			fmul v8.4s, v4.4s, v4.4s
			fmul v4.4s, vecB.4s, v4.4s
			fmul v8.4s, v8.4s, vecA.4s
			ld1 { v12.4s }, [yPtr], #(1*COMPV_GAS_V_SZ_BYTES)
			fadd v8.4s, v8.4s, vecC.4s
			fadd v8.4s, v8.4s, v4.4s
			fsub v8.4s, v8.4s, v12.4s
		.endif
		subs i, i, #4
		fabs v8.4s, v8.4s
		st1 {v8.4s}, [distPtr],	#(1*COMPV_GAS_V_SZ_BYTES)
		bgt LoopCount4_CompVMathDistanceParabola_32f_Asm_NEON64\@
	EndOf_LoopCount4_CompVMathDistanceParabola_32f_Asm_NEON64\@:
	
	.unreq xPtr
	.unreq yPtr
	.unreq A1Ptr
	.unreq B1Ptr
	.unreq C1Ptr
	.unreq distPtr
	.unreq count

	.unreq i
	.unreq vecA
	.unreq vecB
	.unreq vecC

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVMathDistanceParabola_32f_Asm_NEON64
	CompVMathDistanceParabola_32f_Macro_NEON64 0

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVMathDistanceParabola_32f_Asm_FMA_NEON64
	CompVMathDistanceParabola_32f_Macro_NEON64 1

#endif /* defined(__aarch64__) */

#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
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

.extern
 
.text

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Overrides q13, q14 and returns the result in q15
.macro __hamming4x16 vecPatch
    vld1.u8 { vec0 }, [dataPtr :128]!
    vld1.u8 { vec1 }, [t0 :128]! @ t0 = dataPtr[stride * 1]
    vld1.u8 { vec2 }, [t1 :128]! @ t1 = dataPtr[stride * 2]
    vld1.u8 { vec3 }, [t2 :128]! @ t2 = dataPtr[stride * 3]
    veor.u8 vec0, vec0, \vecPatch
    veor.u8 vec1, vec1, \vecPatch
    veor.u8 vec2, vec2, \vecPatch
    veor.u8 vec3, vec3, \vecPatch
    vcnt.u8 vec0, vec0
    vcnt.u8 vec1, vec1
    vcnt.u8 vec2, vec2
    vcnt.u8 vec3, vec3
    vpadd.u8 q13x, vec0x, vec0y
    vpadd.u8 q14x, vec2x, vec2y
    vpadd.u8 q13y, vec1x, vec1y
    vpadd.u8 q14y, vec3x, vec3y
    vpadd.u8 q15x, q13x, q13y
    vpadd.u8 q15y, q14x, q14y
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Overrides q13, q14 and returns the result in q15
.macro __hamming4x16_orphans vecPatch
    vld1.u8 { vec0 }, [dataPtr :128]!
    vld1.u8 { vec1 }, [t0 :128]! @ t0 = dataPtr[stride * 1]
    vld1.u8 { vec2 }, [t1 :128]! @ t1 = dataPtr[stride * 2]
    vld1.u8 { vec3 }, [t2 :128]! @ t2 = dataPtr[stride * 3]
    veor.u8 vec0, vec0, \vecPatch
    veor.u8 vec1, vec1, \vecPatch
    veor.u8 vec2, vec2, \vecPatch
    veor.u8 vec3, vec3, \vecPatch
    vand.u8 vec0, vec0, vecMask
    vand.u8 vec1, vec1, vecMask
    vand.u8 vec2, vec2, vecMask
    vand.u8 vec3, vec3, vecMask
    vcnt.u8 vec0, vec0
    vcnt.u8 vec1, vec1
    vcnt.u8 vec2, vec2
    vcnt.u8 vec3, vec3
    vpadd.u8 q13x, vec0x, vec0y
    vpadd.u8 q14x, vec2x, vec2y
    vpadd.u8 q13y, vec1x, vec1y
    vpadd.u8 q14y, vec3x, vec3y
    vpadd.u8 q15x, q13x, q13y
    vpadd.u8 q15y, q14x, q14y
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@  Overrides q9..q14 and returns the result in q15
.macro __hamming1x64
    vld1.u8 { vec0x, vec0y, vec1x, vec1y }, [dataPtr :128]!
    vld1.u8 { vec2x, vec2y, vec3x, vec3y }, [dataPtr :128]!
    vld1.u8 { q9x, q9y, q10x, q10y }, [patch1xnPtr :128]!
    vld1.u8 { q11x, q11y, q12x, q12y }, [patch1xnPtr :128]!
    veor.u8 vec0, vec0, q9
    veor.u8 vec1, vec1, q10
    veor.u8 vec2, vec2, q11
    veor.u8 vec3, vec3, q12
    vcnt.u8 vec0, vec0
    vcnt.u8 vec1, vec1
    vcnt.u8 vec2, vec2
    vcnt.u8 vec3, vec3
    vpadd.u8 q13x, vec0x, vec0y
    vpadd.u8 q14x, vec2x, vec2y
    vpadd.u8 q13y, vec1x, vec1y
    vpadd.u8 q14y, vec3x, vec3y
    vpadd.u8 q15x, q13x, q13y
    vpadd.u8 q15y, q14x, q14y
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Overrides q13, q14 and returns the result in q15
.macro __hamming1x16
    vld1.u8 { vec0 }, [dataPtr :128]!
    vld1.u8 { q13 }, [patch1xnPtr :128]!
    veor.u8 q14, vec0, q13
    vcnt.u8 q15, q14
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Overrides q12, q13, q14 and returns the result in q15
.macro __hamming1x16_orphans
    vld1.u8 { vec0 }, [dataPtr :128]!
    vld1.u8 { q12 }, [patch1xnPtr :128]!
    veor.u8 q13, vec0, q12
    vand.u8 q14, q13, vecMask
    vcnt.u8 q15, q14
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* dataPtr
@ arg(1) -> compv_uscalar_t width
@ arg(2) -> compv_uscalar_t height
@ arg(3) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
@ arg(4) -> COMPV_ALIGNED(NEON) const uint8_t* patch1xnPtr
@ arg(5) -> int22_t* distPtr
COMPV_GAS_FUNCTION_DECLARE CompVMathDistanceHamming_Asm_NEON32
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 6
	COMPV_GAS_SAVE_NEON_REGS
	
	@@ Load arguments @@
	ldm_args r0-r5 @ load arguments in [r0-r5]
	dataPtr .req r0
	width .req r1
	height .req r2
	stride .req r3
	patch1xnPtr .req r4
	distPtr .req r5

    #define argi_patch1xnPtr 4

    i .req r6
    j .req r7
    pad .req r8
    t0 .req r9
    t1 .req r10
    t2 .req r11

    vec0 .req q0
    vec0x .req q0x
    vec0y .req q0y
    vec1 .req q1
    vec1x .req q1x
    vec1y .req q1y
    vec2 .req q2
    vec2x .req q2x
    vec2y .req q2y
    vec3 .req q3
    vec3x .req q3x
    vec3y .req q3y
    veccnt .req q4
    veccntx .req q4x
    veccnty .req q4y
    vecPatch .req q5
    vecMask .req q6

    @ compute pad
    @ maxI = ((width + 15) & -16), pad = (stride - maxI)
    add pad, width, #15
    and pad, pad, #-16
    sub pad, stride, pad 
	
	@ compute vecMask for orphans
	ands t0, width, #15
    beq NoOrphans_CompVMathDistanceHamming_Asm_NEON32
		mov t1, #-(16<<3)
		vceq.u8 vecMask, vec0, vec0
		veor.u8 q15, q15, q15
		add t0, t1, t0, LSL #3 @ ((orphans - 16) << 3) = (-16<<3) + (orphans << 3)
		mov t1, #0
		cmp t0, #-64
		addlt t1, t0, #64 @ t1 = 0 if (t0 < -64) otherwise unchanged (#0)
		vmov.s32 q15y[0], t0
		vmov.s32 q15x[0], t1
		vshl.u64 vecMask, vecMask, q15
		NoOrphans_CompVMathDistanceHamming_Asm_NEON32:

    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ if (height > 3)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    lsrs j, height, #2 @ div 4
    beq EndOf_IfHeightGt4_CompVMathDistanceHamming_Asm_NEON32
    IfHeightGt4_CompVMathDistanceHamming_Asm_NEON32:
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (; j < height - 3; j += 4)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        LoopH4_CompVMathDistanceHamming_Asm_NEON32:
            veor.u8 veccnt, veccnt, veccnt
            ldr_arg argi_patch1xnPtr, patch1xnPtr
            lsr i, width, #4 @ div 16 (not need to test, width is always > 15)
            add t0, dataPtr, stride @ t0 = dataPtr[stride * 1]
            add t1, dataPtr, stride, LSL #1 @ t1 = dataPtr[stride * 2]
            add t2, t0, stride, LSL #1 @ t2 = dataPtr[stride * 3]
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            @ for (; i < width - 15; i += 16)
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            LoopH4W16_CompVMathDistanceHamming_Asm_NEON32:
                vld1.u8 { vecPatch }, [patch1xnPtr :128]!
                __hamming4x16 vecPatch
                vpadd.u8 q10x, q15x, q15y
                vpaddl.u8 q11x, q10x
                vaddw.s16 veccnt, veccnt, q11x
                subs i, i, #1
                bne LoopH4W16_CompVMathDistanceHamming_Asm_NEON32
                @@ EndOf_LoopH4W16_CompVMathDistanceHamming_Asm_NEON32 @@

            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            @ if (orphans =  static_cast<compv_scalar_t>(width & 15))
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            tst width, #15
            beq EndOf_IfH4Orphans_CompVMathDistanceHamming_Asm_NEON32
            IfH4Orphans_CompVMathDistanceHamming_Asm_NEON32:
                vld1.u8 { vecPatch }, [patch1xnPtr :128]!
                __hamming4x16_orphans vecPatch
                vpadd.u8 q10x, q15x, q15y
                vpaddl.u8 q11x, q10x
                vaddw.s16 veccnt, veccnt, q11x
                EndOf_IfH4Orphans_CompVMathDistanceHamming_Asm_NEON32:
                @@ EndOf_IfH4Orphans_CompVMathDistanceHamming_Asm_NEON32 @@

            
            vst1.s32 {veccnt}, [distPtr]!
            add dataPtr, t2, pad @ t2 = dataPtr[stride * 3] 
            subs j, j, #1
            bne LoopH4_CompVMathDistanceHamming_Asm_NEON32
            @@ EnofOf_LoopH4_CompVMathDistanceHamming_Asm_NEON32 @@

        EndOf_IfHeightGt4_CompVMathDistanceHamming_Asm_NEON32:

    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (; j < height; j += 1)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    ands j, height, #3 @ modulo 4
    beq EndOf_LoopH1_CompVMathDistanceHamming_Asm_NEON32
    LoopH1_CompVMathDistanceHamming_Asm_NEON32:
        veor.u8 veccnt, veccnt, veccnt
        ldr_arg argi_patch1xnPtr, patch1xnPtr
        lsrs i, width, #6 @ div 64
        beq EndOf_LoopH1W64_CompVMathDistanceHamming_Asm_NEON32
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (; i < width - 63; i += 64)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        LoopH1W64_CompVMathDistanceHamming_Asm_NEON32:
            __hamming1x64
            vpadd.u8 q10x, q15x, q15y
            vpaddl.u8 q11x, q10x
            vaddw.s16 veccnt, veccnt, q11x
            subs i, i, #1
            bne LoopH1W64_CompVMathDistanceHamming_Asm_NEON32
            EndOf_LoopH1W64_CompVMathDistanceHamming_Asm_NEON32:
            @@ EndOf_LoopH1W64_CompVMathDistanceHamming_Asm_NEON32 @@

        
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (; i < width - 15; i += 16)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        and t0, width, #63 @ modulo 64 
        lsrs i, t0, #4 @ div 16
        beq EndOf_LoopH1W16_CompVMathDistanceHamming_Asm_NEON32
        LoopH1W16_CompVMathDistanceHamming_Asm_NEON32:
            __hamming1x16
            vpadd.u8 q10x, q15x, q15y
            vpaddl.u8 q11x, q10x
            vaddw.s16 veccnt, veccnt, q11x
            subs i, i, #1
            bne LoopH1W16_CompVMathDistanceHamming_Asm_NEON32
            EndOf_LoopH1W16_CompVMathDistanceHamming_Asm_NEON32:
            @@ EndOf_LoopH1W16_CompVMathDistanceHamming_Asm_NEON32 @@


        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ if (orphans =  static_cast<compv_scalar_t>(width & 15))
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        tst width, #15
        beq EndOf_IfH1Orphans_CompVMathDistanceHamming_Asm_NEON32
        IfH1Orphans_CompVMathDistanceHamming_Asm_NEON32:
            __hamming1x16_orphans
            vpadd.u8 q10x, q15x, q15y
            vpaddl.u8 q11x, q10x
            vaddw.s16 veccnt, veccnt, q11x
            EndOf_IfH1Orphans_CompVMathDistanceHamming_Asm_NEON32:
            @@ EndOf_IfH1Orphans_CompVMathDistanceHamming_Asm_NEON32 @@


        vpadd.s32 q15x, veccntx, veccnty
        vpadd.s32 q14x, q15x, q15x
        vmov.s32 t0, q14x[0]
        str t0, [distPtr], #COMPV_GAS_INT32_SZ_BYTES

        add dataPtr, dataPtr, pad
        subs j, j, #1
        bne LoopH1_CompVMathDistanceHamming_Asm_NEON32
        EndOf_LoopH1_CompVMathDistanceHamming_Asm_NEON32:
        @@ EndOf_LoopH1_CompVMathDistanceHamming_Asm_NEON32 @@

	.unreq dataPtr
	.unreq width
	.unreq height
	.unreq stride
	.unreq patch1xnPtr
	.unreq distPtr

    #undef argi_patch1xnPtr

    .unreq i
    .unreq j
    .unreq pad
    .unreq t0
    .unreq t1
    .unreq t2

    .unreq vec0
    .unreq vec0x
    .unreq vec0y
    .unreq vec1
    .unreq vec1x
    .unreq vec1y
    .unreq vec2
    .unreq vec2x
    .unreq vec2y
    .unreq vec3
    .unreq vec3x
    .unreq vec3y
    .unreq veccnt
    .unreq veccntx
    .unreq veccnty
    .unreq vecPatch
    .unreq vecMask

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 6
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN




@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* dataPtr
@ arg(1) -> compv_uscalar_t height
@ arg(2) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
@ arg(3) -> COMPV_ALIGNED(NEON) const uint8_t* patch1xnPtr
@ arg(4) -> int22_t* distPtr
COMPV_GAS_FUNCTION_DECLARE CompVMathDistanceHamming32_Asm_NEON32 
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 6
	COMPV_GAS_SAVE_NEON_REGS
	
	@@ Load arguments @@
	ldm_args r0-r4 @ load arguments in [r0-r4]
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
    
    vecPatch0 .req q0
    vecPatch1 .req q1

    vld1.u8 { vecPatch0 }, [patch1xnPtr : 128]!
    vld1.u8 { vecPatch1 }, [patch1xnPtr : 128]!
    .unreq patch1xnPtr @ patch1xnPtr no longer needed

    @ compute pad
    sub pad, stride, #32    

    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ if (height > 3)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    lsrs j, height, #2 @ div 4
    beq EndOf_IfHeightGt4_CompVMathDistanceHamming32_Asm_NEON32
    IfHeightGt4_CompVMathDistanceHamming32_Asm_NEON32:
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (; j < height - 3; j += 4)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        LoopH4_CompVMathDistanceHamming32_Asm_NEON32:
            add t0, dataPtr, stride @ t0 = dataPtr[stride * 1]
            add t1, dataPtr, stride, LSL #1 @ t1 = dataPtr[stride * 2]
            add t2, t0, stride, LSL #1 @ t2 = dataPtr[stride * 3]
			@ vcnt has high latency and throughput and this is why veors (low latency and throughput) are interleaved
            vld1.u8 { q4x, q4y, q5x, q5y }, [dataPtr :128]!			
            veor.u8 q4, q4, vecPatch0
			veor.u8 q5, q5, vecPatch1
            vcnt.u8 q4, q4
			pld [t2, #(CACHE_LINE_SIZE*1)]
			vld1.u8 { q6x, q6y, q7x, q7y }, [t0 :128]!
            vcnt.u8 q5, q5
			pld [t2, #(CACHE_LINE_SIZE*2)]
			veor.u8 q6, q6, vecPatch0
			veor.u8 q7, q7, vecPatch1
            vcnt.u8 q6, q6
			pld [t2, #(CACHE_LINE_SIZE*3)]
			vld1.u8 { q8x, q8y, q9x, q9y }, [t1 :128]!
            vcnt.u8 q7, q7
			pld [t2, #(CACHE_LINE_SIZE*4)]
			veor.u8 q8, q8, vecPatch0
			veor.u8 q9, q9, vecPatch1
            vcnt.u8 q8, q8
			vld1.u8 { q10x, q10y, q11x, q11y }, [t2 :128]!
            vcnt.u8 q9, q9
			veor.u8 q10, q10, vecPatch0
			veor.u8 q11, q11, vecPatch1
            vcnt.u8 q10, q10
            vcnt.u8 q11, q11
			adds dataPtr, t2, pad @ t2 = dataPtr[stride * 3] 
            subs j, j, #1
            vpadd.u8 q4x, q4x, q4y
            vpadd.u8 q5x, q5x, q5y
            vpadd.u8 q8x, q8x, q8y
            vpadd.u8 q9x, q9x, q9y
			vpadd.u8 q4y, q6x, q6y
            vpadd.u8 q5y, q7x, q7y
			vpadd.u8 q8y, q10x, q10y
            vpadd.u8 q9y, q11x, q11y
            vpadd.u8 q4x, q4x, q4y
            vpadd.u8 q5x, q5x, q5y
            vpadd.u8 q4y, q8x, q8y
            vpadd.u8 q5y, q9x, q9y            
            vpadd.u8 q4x, q4x, q4y
            vpadd.u8 q5x, q5x, q5y
            vpaddl.u8 q4x, q4x
            vpadal.u8 q4x, q5x
            vmovl.s16 q4, q4x
            vst1.s32 {q4}, [distPtr]!
            bne LoopH4_CompVMathDistanceHamming32_Asm_NEON32
            @@ EnofOf_LoopH4_CompVMathDistanceHamming32_Asm_NEON32 @@

        EndOf_IfHeightGt4_CompVMathDistanceHamming32_Asm_NEON32:


    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (; j < height; j += 1)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    ands j, height, #3 @ modulo 4
    beq EndOf_LoopH1_CompVMathDistanceHamming32_Asm_NEON32
    LoopH1_CompVMathDistanceHamming32_Asm_NEON32:
        vld1.u8 { q4x, q4y, q5x, q5y }, [dataPtr :128]!
        veor.u8 q4, q4, vecPatch0
        vcnt.u8 q4, q4
		veor.u8 q5, q5, vecPatch1
        vcnt.u8 q5, q5
        vpadd.u8 q6x, q4x, q4y
        vpadd.u8 q6y, q5x, q5y
        vpadd.u8 q7x, q6x, q6y
        vpaddl.u8 q8x, q7x
        vpaddl.u16 q9x, q8x
        vpaddl.u32 q9x, q9x
        vmov.s32 t0, q9x[0]
        add dataPtr, dataPtr, pad
        str t0, [distPtr], #COMPV_GAS_INT32_SZ_BYTES
        subs j, j, #1
        bne LoopH1_CompVMathDistanceHamming32_Asm_NEON32
        EndOf_LoopH1_CompVMathDistanceHamming32_Asm_NEON32:
        @@ EndOf_LoopH1_CompVMathDistanceHamming33_Asm_NEON32 @@

	.unreq dataPtr
	.unreq height
	.unreq stride
	.unreq distPtr

    .unreq j
    .unreq pad
    .unreq t0
    .unreq t1
    .unreq t2

    .unreq vecPatch0
    .unreq vecPatch1

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 6
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const compv_float32_t* xPtr, 
@ arg(1) -> COMPV_ALIGNED(NEON) const compv_float32_t* yPtr, 
@ arg(2) -> const compv_float32_t* Ascaled1, 
@ arg(3) -> const compv_float32_t* Bscaled1, 
@ arg(4) -> const compv_float32_t* Cscaled1, 
@ arg(5) -> COMPV_ALIGNED(NEON) compv_float32_t* distPtr, 
@ arg(6) -> const compv_uscalar_t count
.macro CompVMathDistanceLine_32f_Macro_NEON32 fusedMultiplyAdd
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 7
	COMPV_GAS_SAVE_NEON_REGS

	@@ Load arguments @@
	ldm_args r0-r6
	xPtr .req r0
	yPtr .req r1
	Ascaled1 .req r2
	Bscaled1 .req r3
	Cscaled1 .req r4
	distPtr .req r5
	count .req r6

	pld [xPtr, #(CACHE_LINE_SIZE*0)]
	pld [xPtr, #(CACHE_LINE_SIZE*1)]
	pld [xPtr, #(CACHE_LINE_SIZE*2)]
	pld [xPtr, #(CACHE_LINE_SIZE*3)]

	pld [yPtr, #(CACHE_LINE_SIZE*0)]
	pld [yPtr, #(CACHE_LINE_SIZE*1)]
	pld [yPtr, #(CACHE_LINE_SIZE*2)]
	pld [yPtr, #(CACHE_LINE_SIZE*3)]

	@ Local variables
	i .req r7
	vecA .req q0
	vecB .req q1
	vecC .req q2

	ldr r8, [Ascaled1]
	ldr r9, [Bscaled1]
	ldr r10, [Cscaled1]
	vdup.f32 vecA, r8
	vdup.f32 vecB, r9
	vdup.f32 vecC, r10

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (i = 0; i < count16; i += 16)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	ands i, count, #-16
	beq EndOf_LoopCount16_CompVMathDistanceLine_32f_Asm_NEON32\@
	LoopCount16_CompVMathDistanceLine_32f_Asm_NEON32\@:
		vld1.f32 { q4,q5 }, [xPtr :128]!
		vld1.f32 { q6,q7 }, [xPtr :128]!
		pld [xPtr, #(CACHE_LINE_SIZE*4)]
		.if \fusedMultiplyAdd
			vmov q8, vecC
			vmov q9, vecC
			vfma.f32 q8, vecA, q4
			vfma.f32 q9, vecA, q5
			vmov q10, vecC
			vmov q11, vecC
			vfma.f32 q10, vecA, q6
			vfma.f32 q11, vecA, q7
			vld1.f32 { q12,q13 }, [yPtr :128]!
			vld1.f32 { q14,q15 }, [yPtr :128]!
			pld [yPtr, #(CACHE_LINE_SIZE*4)]
			vfma.f32 q8, vecB, q12
			vfma.f32 q9, vecB, q13
			vfma.f32 q10, vecB, q14
			vfma.f32 q11, vecB, q15
		.else
			vmul.f32 q8, vecA, q4
			vmul.f32 q9, vecA, q5
			vmul.f32 q10, vecA, q6
			vmul.f32 q11, vecA, q7
			vld1.f32 { q12,q13 }, [yPtr :128]!
			vld1.f32 { q14,q15 }, [yPtr :128]!
			pld [yPtr, #(CACHE_LINE_SIZE*4)]
			vmul.f32 q12, q12, vecB
			vmul.f32 q13, q13, vecB
			vmul.f32 q14, q14, vecB
			vmul.f32 q15, q15, vecB
			vadd.f32 q8, q8, vecC
			vadd.f32 q9, q9, vecC
			vadd.f32 q10, q10, vecC
			vadd.f32 q11, q11, vecC
			vadd.f32 q8, q8, q12
			vadd.f32 q9, q9, q13
			vadd.f32 q10, q10, q14
			vadd.f32 q11, q11, q15
		.endif
		subs i, i, #16
		vabs.f32 q8, q8
		vabs.f32 q9, q9
		vst1.f32 {q8, q9}, [distPtr :128]!
		vabs.f32 q10, q10
		vabs.f32 q11, q11
		vst1.f32 {q10, q11}, [distPtr :128]!
		bne LoopCount16_CompVMathDistanceLine_32f_Asm_NEON32\@
	EndOf_LoopCount16_CompVMathDistanceLine_32f_Asm_NEON32\@:

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (; i < count; i += 4)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	ands i, count, #15
	beq EndOf_LoopCount4_CompVMathDistanceLine_32f_Asm_NEON32\@
	LoopCount4_CompVMathDistanceLine_32f_Asm_NEON32\@:
		vld1.f32 { q4 }, [xPtr :128]!
		.if \fusedMultiplyAdd
			vmov q8, vecC
			vfma.f32 q8, vecA, q4
			vld1.f32 { q12 }, [yPtr :128]!
			vfma.f32 q8, vecB, q12
		.else
			vmul.f32 q8, vecA, q4
			vld1.f32 { q12 }, [yPtr :128]!
			vmul.f32 q12, q12, vecB
			vadd.f32 q8, q8, vecC
			vadd.f32 q8, q8, q12
		.endif
		subs i, i, #4
		vabs.f32 q8, q8
		vst1.f32 {q8}, [distPtr :128]!		
		bgt LoopCount4_CompVMathDistanceLine_32f_Asm_NEON32\@
	EndOf_LoopCount4_CompVMathDistanceLine_32f_Asm_NEON32\@:
	
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
	COMPV_GAS_UNSHADOW_ARGS 7
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVMathDistanceLine_32f_Asm_NEON32
	CompVMathDistanceLine_32f_Macro_NEON32 0

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVMathDistanceLine_32f_Asm_FMA_NEON32
	CompVMathDistanceLine_32f_Macro_NEON32 1

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const compv_float32_t* xPtr, 
@ arg(1) -> COMPV_ALIGNED(NEON) const compv_float32_t* yPtr, 
@ arg(2) -> const compv_float32_t* A1, 
@ arg(3) -> const compv_float32_t* B1, 
@ arg(4) -> const compv_float32_t* C1, 
@ arg(5) -> COMPV_ALIGNED(NEON) compv_float32_t* distPtr, 
@ arg(6) -> const compv_uscalar_t count
.macro CompVMathDistanceParabola_32f_Macro_NEON32 fusedMultiplyAdd
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 7
	COMPV_GAS_SAVE_NEON_REGS

	@@ Load arguments @@
	ldm_args r0-r6
	xPtr .req r0
	yPtr .req r1
	A1Ptr .req r2
	B1Ptr .req r3
	C1Ptr .req r4
	distPtr .req r5
	count .req r6

	pld [xPtr, #(CACHE_LINE_SIZE*0)]
	pld [xPtr, #(CACHE_LINE_SIZE*1)]
	pld [xPtr, #(CACHE_LINE_SIZE*2)]
	pld [xPtr, #(CACHE_LINE_SIZE*3)]

	pld [yPtr, #(CACHE_LINE_SIZE*0)]
	pld [yPtr, #(CACHE_LINE_SIZE*1)]
	pld [yPtr, #(CACHE_LINE_SIZE*2)]
	pld [yPtr, #(CACHE_LINE_SIZE*3)]

	@ Local variables
	i .req r7
	vecA .req q0
	vecB .req q1
	vecC .req q2

	ldr r8, [A1Ptr]
	ldr r9, [B1Ptr]
	ldr r10, [C1Ptr]
	vdup.f32 vecA, r8
	vdup.f32 vecB, r9
	vdup.f32 vecC, r10

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (i = 0; i < count16; i += 16)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	ands i, count, #-16
	beq EndOf_LoopCount16_CompVMathDistanceLine_32f_Asm_NEON32\@
	LoopCount16_CompVMathDistanceLine_32f_Asm_NEON32\@:
		vld1.f32 { q4,q5 }, [xPtr :128]!
		vld1.f32 { q6,q7 }, [xPtr :128]!
		pld [xPtr, #(CACHE_LINE_SIZE*4)]
		.if \fusedMultiplyAdd
			vmov.f32 q8, vecC
			vmov.f32 q9, vecC
			vmov.f32 q10, vecC
			vmov.f32 q11, vecC

			vmul.f32 q12, q4, q4
			vmul.f32 q13, q5, q5
			vmul.f32 q14, q6, q6
			vmul.f32 q15, q7, q7

			vfma.f32 q8, vecA, q12
			vfma.f32 q9, vecA, q13
			vfma.f32 q10, vecA, q14
			vfma.f32 q11, vecA, q15

			vld1.f32 { q12,q13 }, [yPtr :128]!
			vld1.f32 { q14,q15 }, [yPtr :128]!
			pld [yPtr, #(CACHE_LINE_SIZE*4)]

			vfma.f32 q8, vecB, q4
			vfma.f32 q9, vecB, q5
			vfma.f32 q10, vecB, q6
			vfma.f32 q11, vecB, q7			

			vsub.f32 q8, q8, q12
			vsub.f32 q9, q9, q13
			vsub.f32 q10, q10, q14
			vsub.f32 q11, q11, q15
		.else
			vmul.f32 q8, q4, q4
			vmul.f32 q9, q5, q5
			vmul.f32 q10, q6, q6
			vmul.f32 q11, q7, q7
			vmul.f32 q4, vecB, q4
			vmul.f32 q5, vecB, q5
			vmul.f32 q6, vecB, q6
			vmul.f32 q7, vecB, q7
			vmul.f32 q8, q8, vecA
			vmul.f32 q9, q9, vecA
			vmul.f32 q10, q10, vecA
			vmul.f32 q11, q11, vecA
			vld1.f32 { q12,q13 }, [yPtr :128]!
			vld1.f32 { q14,q15 }, [yPtr :128]!
			pld [yPtr, #(CACHE_LINE_SIZE*4)]
			vadd.f32 q8, q8, vecC
			vadd.f32 q9, q9, vecC
			vadd.f32 q10, q10, vecC
			vadd.f32 q11, q11, vecC
			vadd.f32 q8, q8, q4
			vadd.f32 q9, q9, q5
			vadd.f32 q10, q10, q6
			vadd.f32 q11, q11, q7
			vsub.f32 q8, q8, q12
			vsub.f32 q9, q9, q13
			vsub.f32 q10, q10, q14
			vsub.f32 q11, q11, q15
		.endif
		subs i, i, #16
		vabs.f32 q8, q8
		vabs.f32 q9, q9
		vst1.f32 {q8, q9}, [distPtr :128]!
		vabs.f32 q10, q10
		vabs.f32 q11, q11
		vst1.f32 {q10, q11}, [distPtr :128]!
		bne LoopCount16_CompVMathDistanceLine_32f_Asm_NEON32\@
	EndOf_LoopCount16_CompVMathDistanceLine_32f_Asm_NEON32\@:

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (; i < count; i += 4)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	ands i, count, #15
	beq EndOf_LoopCount4_CompVMathDistanceLine_32f_Asm_NEON32\@
	LoopCount4_CompVMathDistanceLine_32f_Asm_NEON32\@:
		vld1.f32 { q4 }, [xPtr :128]!
		.if \fusedMultiplyAdd
			vmov.f32 q8, vecC

			vmul.f32 q12, q4, q4

			vfma.f32 q8, vecA, q12

			vld1.f32 { q12 }, [yPtr :128]!

			vfma.f32 q8, vecB, q4	

			vsub.f32 q8, q8, q12
		.else
			vmul.f32 q8, q4, q4
			vmul.f32 q4, vecB, q4
			vmul.f32 q8, q8, vecA
			vld1.f32 { q12 }, [yPtr :128]!
			vadd.f32 q8, q8, vecC
			vadd.f32 q8, q8, q4
			vsub.f32 q8, q8, q12
		.endif
		subs i, i, #4
		vabs.f32 q8, q8
		vst1.f32 {q8}, [distPtr :128]!		
		bgt LoopCount4_CompVMathDistanceLine_32f_Asm_NEON32\@
	EndOf_LoopCount4_CompVMathDistanceLine_32f_Asm_NEON32\@:
	
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
	COMPV_GAS_UNSHADOW_ARGS 7
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVMathDistanceParabola_32f_Asm_NEON32
	CompVMathDistanceParabola_32f_Macro_NEON32 0

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVMathDistanceParabola_32f_Asm_FMA_NEON32
	CompVMathDistanceParabola_32f_Macro_NEON32 1

#endif /* defined(__arm__) && !defined(__aarch64__) */

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

.equ mask0, 0x8040
.equ mask1, 0x2010
.equ mask2, 0x804
.equ mask3, 0x201

############################################################################################################
# arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* Xi
# arg(1) -> const compv_uscalar_t Xi_stride
# arg(2) -> int16_t* ERi
# arg(3) -> int16_t* ner
# arg(4) -> int16_t* ner_max1
# arg(5) -> int32_t* ner_sum1
# arg(6) -> const compv_uscalar_t width
# arg(7) -> const compv_uscalar_t height
COMPV_GAS_FUNCTION_DECLARE CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Load arguments ##
	Xi .req r0
	Xi_stride .req r1
	ERi .req r2
	ner .req r3
	ner_max1 .req r4
	ner_sum1 .req r5
	width .req r6
	height .req r7

	er .req r8
	erw .req r8w
	i .req r9
	t0 .req r10
	t0w .req r10w
	t1 .req r11
	t1w .req r11w
	ner_max .req r12
	ner_maxw .req r12w
	ner_sum .req r13
	ner_sumw .req r13w
	width16 .req r14

	vecER .req v30 // must be v30 (q30 used as alias within the code)
	vecOne .req v31

	prfm pldl1keep, [Xi, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [Xi, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [Xi, #(CACHE_LINE_SIZE*2)]
	prfm pldl1keep, [Xi, #(CACHE_LINE_SIZE*3)]

	mov t0, #1
	sub width16, width, #1
	eor ner_max, ner_max, ner_max
	eor ner_sum, ner_sum, ner_sum
	dup vecOne.16b, t0w
	sub Xi_stride, Xi_stride, width // now Xi_stride contains padding
	and width16, width16, #-16

	#########################################################
	# for (compv_uscalar_t j = 0; j < height; ++j)
	#########################################################
	LoopHeight_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64:
		ldrb erw, [Xi], #(1*COMPV_GAS_UINT8_SZ_BYTES)
		and er, er, #1
		strh erw, [ERi], #(1*COMPV_GAS_INT16_SZ_BYTES)
		dup vecER.8h, erw
		
		#########################################################
		# for (i = 1; i < width16; i += 16)
		#########################################################
		mov i, #1
		LoopWidth16_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64:
			ldr q0, [Xi, #-(1*COMPV_GAS_UINT8_SZ_BYTES)]
			ldr q1, [Xi], #COMPV_GAS_V_SZ_BYTES
			prfm pldl1keep, [Xi, #(CACHE_LINE_SIZE*4)]
			eor v0.16b, v0.16b, v1.16b
			and v0.16b, v0.16b, vecOne.16b
			#################################################
			// if (mask & 0xff)
			#################################################
			mov t0, v0.d[0]
			cbnz t0, TestMask1_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64
			str q30, [ERi], #(1*COMPV_GAS_Q_SZ_BYTES)
			b EndOf_TestMask1_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64
			TestMask1_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64:
				.set index, 0
				.rep 8
					tst t0, #(1<<index)
					cinc erw, erw, ne
					strh erw, [ERi], #(1*COMPV_GAS_INT16_SZ_BYTES)
					.set index, index+8
				.endr
				dup vecER.8h, erw
			EndOf_TestMask1_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64:

			#################################################
			// if (mask & 0xff00)
			#################################################
			mov t0, v0.d[1]
			cbnz t0, TestMask2_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64
			str q30, [ERi], #(1*COMPV_GAS_Q_SZ_BYTES)
			b EndOf_TestMask2_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64
			TestMask2_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64:
				.set index, 0
				.rep 8
					tst t0, #(1<<index)
					cinc erw, erw, ne
					strh erw, [ERi], #(1*COMPV_GAS_INT16_SZ_BYTES)
					.set index, index+8
				.endr
				dup vecER.8h, erw
			EndOf_TestMask2_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64:

			add i, i, #16
			cmp i, width16
			blt LoopWidth16_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64
		EndOf_LoopWidth16_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64:
		
		## Get highest "er" before switching from SIMD to serial code ##
		umov erw, vecER.h[7]

		#########################################################
		# for (; i < width1; ++i) #
		#########################################################
		cmp i, width
		bge EndOf_LoopWidth1_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64
		LoopWidth1_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64:
			ldrb t0w, [Xi, #-(1*COMPV_GAS_UINT8_SZ_BYTES)]
			ldrb t1w, [Xi], #(1*COMPV_GAS_UINT8_SZ_BYTES)
			eor t0w, t0w, t1w
			and t0w, t0w, #1
			add erw, erw, t0w
			strh erw, [ERi], #(1*COMPV_GAS_INT16_SZ_BYTES)
			add i, i, #1
			cmp i, width
			blt LoopWidth1_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64
		EndOf_LoopWidth1_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64:

		ldrb t1w, [Xi, #-(1*COMPV_GAS_UINT8_SZ_BYTES)]
		add Xi, Xi, Xi_stride
		and t1w, t1w, #1
		add erw, erw, t1w
		add ner_sum, ner_sum, er
		strh erw, [ner], #(1*COMPV_GAS_INT16_SZ_BYTES)
		cmp ner_max, er
		csel ner_max, er, ner_max, lt
		
		subs height, height, #1
		bne LoopHeight_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64
	EndOf_LoopHeight_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64:

	strh ner_maxw, [ner_max1] // int16
	str ner_sumw, [ner_sum1] // int32

	.unreq Xi
	.unreq Xi_stride
	.unreq ERi
	.unreq ner
	.unreq ner_max1
	.unreq ner_sum1
	.unreq width
	.unreq height

	.unreq er
	.unreq erw
	.unreq i
	.unreq t0
	.unreq t0w
	.unreq t1
	.unreq t1w
	.unreq ner_max
	.unreq ner_maxw
	.unreq ner_sum
	.unreq ner_sumw
	.unreq width16

	.unreq vecER
	.unreq vecOne

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


############################################################################################################
# arg(0) -> const uint8_t* Xi, 
# arg(1) -> const compv_uscalar_t Xi_stride,
# arg(2) -> int16_t* ERi, 
# arg(3) -> int16_t* RLCi, 
# arg(4) -> const compv_uscalar_t RLCi_stride,
# arg(5) -> const compv_uscalar_t width, 
# arg(6) -> const compv_uscalar_t height
COMPV_GAS_FUNCTION_DECLARE CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON64
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Load arguments ##
	Xi .req r0
	Xi_stride .req r1
	ERi .req r2
	RLCi .req r3
	RLCi_stride .req r4
    width .req r5
    height .req r6

	vecMask .req v10
	width16 .req r7
	i .req r8
	iw .req r8w
	t0 .req r9
	t0w .req r9w
	t1 .req r10
	t1w .req r10w
	rMask .req r11 // Mask to simulate x86 'blsr' instruction
	rMaskw .req r11w
	ERi_stride .req r12
	t2 .req r13
	t2w .req r13w
	RLCi2 .req r14

	lsl RLCi_stride, RLCi_stride, #(COMPV_GAS_INT16_SHIFT_BYTES) // int16 -> byte
	lsl ERi_stride, width, #(COMPV_GAS_INT16_SHIFT_BYTES) // int16 -> byte
	sub width16, width, #1
	and width16, width16, #-16
	
	movz t0, #mask0, lsl #48
	movk t0, #mask1, lsl #32
	movk t0, #mask2, lsl #16
	movk t0, #mask3
	dup vecMask.2d, t0
	mov rMaskw, #0x80000000
	
	#########################################################
	# for (compv_uscalar_t j = 0; j < height; ++j)
	#########################################################
	LoopHeight_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON64:
		add ERi, ERi, #(1*COMPV_GAS_INT16_SZ_BYTES) // i starts at #1
		mov RLCi2, RLCi
		mov t1, #0
		ldrb t0w, [Xi]
		and t0, t0, #1
		strh t1w, [RLCi2]
		add RLCi2, RLCi2, t0, LSL #(COMPV_GAS_INT16_SHIFT_BYTES)

		#########################################################
		# for (i = 1; i < width16; i += 16)
		#########################################################
		mov i, #1
		LoopWidth16_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON64:
			add t0, ERi, #-(1*COMPV_GAS_INT16_SZ_BYTES)
			prfm pldl1keep, [ERi, ERi_stride]
			ldp q0, q1, [t0]
			ldp q2, q3, [ERi], #(2*COMPV_GAS_Q_SZ_BYTES)
			cmeq v0.8h, v0.8h, v2.8h
			cmeq v1.8h, v1.8h, v3.8h
			uqxtn v0.8b, v0.8h
			uqxtn2 v0.16b, v1.8h
			mvn v0.16b, v0.16b
			mov t0, v0.d[0]
			mov t1, v0.d[1]
			orr t0, t0, t1 // orrs not avail on Aarch64
			cbz t0, EndOfMask_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON64
				and v0.16b, v0.16b, vecMask.16b
				addp v0.16b, v0.16b, v0.16b
				addp v0.8b, v0.8b, v0.8b
				addp v0.8b, v0.8b, v0.8b
				umov t0w, v0.h[0]
				rbit t0w, t0w
				BeginOfWhile_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON64:
					// ARM there is no ctz (Count Trailing Zero): ctz = clz(rbit(t0))
					clz t1w, t0w
					lsr t2w, rMaskw, t1w
					bics t0w, t0w, t2w // clear bit at t1
					add t1w, t1w, iw
					strh t1w, [RLCi2], #(1*COMPV_GAS_INT16_SZ_BYTES)	
					bne BeginOfWhile_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON64
				EndOf_BeginOfWhile_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON64:
			EndOfMask_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON64:

			add i, i, #16
			cmp i, width16
			blt LoopWidth16_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON64
		EndOf_LoopWidth16_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON64:

		#########################################################
		# for (; i < width1; ++i)
		#########################################################
		cmp i, width
		bge EndOf_LoopWidth1_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON64
		LoopWidth1_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON64:
			ldrh t0w, [ERi, #-(1*COMPV_GAS_INT16_SZ_BYTES)]
			ldrh t1w, [ERi], #(1*COMPV_GAS_INT16_SZ_BYTES)
			eor t0w, t0w, t1w
			cbz t0w, Next_LoopWidth1_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON64
				strh iw, [RLCi2], #(1*COMPV_GAS_INT16_SZ_BYTES)
			Next_LoopWidth1_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON64:
			add i, i, #1
			cmp i, width
			blt LoopWidth1_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON64
		EndOf_LoopWidth1_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON64:

		sub t0, width, #1
		ldrb t0w, [Xi, t0]
		and t0w, t0w, #1
		eor t0w, t0w, #1
		sub t0, width, t0
		strh t0w, [RLCi2]

		# next #
		add Xi, Xi, Xi_stride
		add RLCi, RLCi, RLCi_stride
		subs height, height, #1
		bne LoopHeight_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON64
	EndOf_LoopHeight_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON64:


	.unreq Xi
	.unreq Xi_stride
	.unreq ERi
	.unreq RLCi
	.unreq RLCi_stride
    .unreq width
    .unreq height

	.unreq vecMask
	.unreq width16
	.unreq i
	.unreq iw
	.unreq t0
	.unreq t0w
	.unreq t1
	.unreq t1w
	.unreq rMask
	.unreq rMaskw
	.unreq ERi_stride
	.unreq t2
	.unreq t2w
	.unreq RLCi2

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#endif /* defined(__aarch64__) */

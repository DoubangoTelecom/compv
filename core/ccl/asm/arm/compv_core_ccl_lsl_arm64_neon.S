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

	vecER .req v30
	vecOne .req v31

	prfm pldl1keep, [Xi, #(CACHE_LINE_SIZE*0)]
	prfm pldl1keep, [Xi, #(CACHE_LINE_SIZE*1)]
	prfm pldl1keep, [Xi, #(CACHE_LINE_SIZE*2)]
	prfm pldl1keep, [Xi, #(CACHE_LINE_SIZE*3)]

	mov t0, #1
	eor ner_max, ner_max, ner_max
	eor ner_sum, ner_sum, ner_sum
	dup vecOne.16b, t0w
	sub Xi_stride, Xi_stride, width // now Xi_stride contains padding

	#########################################################
	# for (compv_uscalar_t j = 0; j < height; ++j)
	#########################################################
	LoopHeight_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64:
		ldrb erw, [Xi], #(1*COMPV_GAS_UINT8_SZ_BYTES)
		sub i, width, #1
		and er, er, #1
		and i, i, #-16 // now 'i' is equal to width16 (FIXME(dmi): on ARM64, pre-compute width16)
		strh erw, [ERi], #(1*COMPV_GAS_INT16_SZ_BYTES)
		dup vecER.8h, erw
		
		#########################################################
		# for (i = 1; i < width16; i += 16)
		#########################################################
		LoopWidth16_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64:
			add t0, Xi, #-(1*COMPV_GAS_UINT8_SZ_BYTES)
			prfm pldl1keep, [Xi, #(CACHE_LINE_SIZE*4)]
			ld1 {v0.16b}, [t0]
			ld1 {v1.16b}, [Xi], #COMPV_GAS_V_SZ_BYTES
			eor v0.16b, v0.16b, v1.16b
			and v0.16b, v0.16b, vecOne.16b
			mov t0, v0.d[0]
			mov t1, v0.d[1]
			orr t0, t0, t1 // orrs not avail on Aarch64
			cbz t0, EndOf_TestBoth_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64
			TestBoth_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64:
				eor v1.16b, v1.16b, v1.16b
				## == Low == ##
				mov t0, v0.d[0]
				cbz t0, EndOf_TestLow_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64
				TestLow_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64:
					dup v1.16b, v0.b[0]
					dup v2.16b, v0.b[1]
					dup v3.16b, v0.b[2]
					dup v4.16b, v0.b[3]
					dup v5.16b, v0.b[4]
					dup v6.16b, v0.b[5]
					dup v7.16b, v0.b[6]
					dup v8.16b, v0.b[7]
					shl d2, d2, #8
					shl d3, d3, #16
					shl d4, d4, #24
					shl d5, d5, #32
					shl d6, d6, #40
					shl d7, d7, #48
					shl d8, d8, #56
					add v2.16b, v2.16b, v3.16b
					add v4.16b, v4.16b, v5.16b
					add v6.16b, v6.16b, v7.16b
					add v2.16b, v2.16b, v8.16b
					add v4.16b, v4.16b, v6.16b
					add v1.16b, v1.16b, v2.16b
					add v1.16b, v1.16b, v4.16b
				EndOf_TestLow_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64:

				## == High == ##
				mov t0, v0.d[1]
				cbz t0, EndOf_TestHigh_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64
				TestHigh_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64:
					dup v29.8b, v0.b[8]
					dup v3.8b, v0.b[9]
					dup v4.8b, v0.b[10]
					dup v5.8b, v0.b[11]
					dup v6.8b, v0.b[12]
					dup v7.8b, v0.b[13]
					dup v8.8b, v0.b[14]
					dup v0.8b, v0.b[15]
					shl d3, d3, #8
					shl d4, d4, #16
					shl d5, d5, #24
					shl d6, d6, #32
					shl d7, d7, #40
					shl d8, d8, #48
					shl d0, d0, #56
					add v29.8b, v29.8b, v3.8b
					add v4.8b, v4.8b, v5.8b
					add v6.8b, v6.8b, v7.8b
					add v8.8b, v8.8b, v0.8b
					add v29.8b, v29.8b, v4.8b
					add v6.8b, v6.8b, v8.8b
					eor v2.8b, v2.8b, v2.8b // now q2x contains zeros
					add v29.8b, v29.8b, v6.8b
					mov	v2.d[1], v29.d[0] // setting q2y = d29					
					add v1.16b, v1.16b, v2.16b
				EndOf_TestHigh_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64:

				## Convert erUint8 to erInt16 and sum then store ##
				uaddw v0.8h, vecER.8h, v1.8b
				uaddw2 vecER.8h, vecER.8h, v1.16b
				st1 {v0.8h}, [ERi], #COMPV_GAS_V_SZ_BYTES
				st1 {vecER.8h}, [ERi], #COMPV_GAS_V_SZ_BYTES // FIXME(dmi): ARM64 -> use stp

				## Duplicate latest element ##
				dup vecER.8h, vecER.h[7]

				## Skip stroring previous ER and goto Next ##
				b Next16_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64
			EndOf_TestBoth_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64:

			## Store previous ER (FIXME(dmi): ARM64 -> stp) ##
			st1 {vecER.8h}, [ERi], #COMPV_GAS_V_SZ_BYTES
			st1 {vecER.8h}, [ERi], #COMPV_GAS_V_SZ_BYTES

			Next16_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64:
			subs i, i, #16
			bne LoopWidth16_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64
		EndOf_LoopWidth16_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON64:
		
		## Get highest "er" before switching from SIMD to serial code ##
		and i, width, #-16
		umov erw, vecER.h[7]
		add i, i, #1 // now i is equal to (width16 + 1) (FIXME(dmi): ARM64 -> precompute width16 and don't override i)

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

	.unreq vecER
	.unreq vecOne

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#endif /* defined(__aarch64__) */

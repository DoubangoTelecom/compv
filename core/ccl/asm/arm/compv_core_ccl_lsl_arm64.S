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
# arg(0) -> const int16_t* RLCi
# arg(1) -> const compv_uscalar_t RLCi_stride
# arg(2) -> int32_t* ERAi
# arg(3) -> const compv_uscalar_t ERAi_stride
# arg(4) -> const int16_t* ERiminus1
# arg(5) -> const int16_t* ner
# arg(6) -> const compv_uscalar_t width
# arg(7) -> const compv_uscalar_t height
COMPV_GAS_FUNCTION_DECLARE CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_ARM64
	COMPV_GAS_FUNCTION_PROLOG

	## Load arguments ##
	RLCi .req r0
	RLCi_stride .req r1
	ERAi .req r2
	ERAi_stride .req r3
	ERiminus1 .req r4
	ner .req r5
	width .req r6
	height .req r7

	er .req r8
	j0 .req r9
	j0w .req r9w
	j1 .req r10
	j1w .req r10w
	nerj .req r11
	nerjw .req r11w
	wminus1 .req r12
	f0 .req r13
	zero .req r14

	eor zero, zero, zero
	sub wminus1, width, #1
	lsl width, width, #COMPV_GAS_INT16_SHIFT_BYTES
	lsl RLCi_stride, RLCi_stride, #(COMPV_GAS_INT16_SHIFT_BYTES)
	lsl ERAi_stride, ERAi_stride, #(COMPV_GAS_INT32_SHIFT_BYTES)

	######################################################
	# for (compv_uscalar_t j = 0; j < height; ++j)
	######################################################
	LoopHeight_CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_ARM64:
		ldrh nerjw, [ner], #(1*COMPV_GAS_INT16_SZ_BYTES)
		prfm pldl1keep, [RLCi, #(CACHE_LINE_SIZE*0)]
		prfm pldl1keep, [RLCi, #(CACHE_LINE_SIZE*1)] // we don't expect more than #32 RLCi values
		lsl nerj, nerj, #COMPV_GAS_INT16_SHIFT_BYTES
		######################################################
		# for (er = 1; er < nerj; er += 2)
		######################################################
		mov er, #(1*COMPV_GAS_INT16_SZ_BYTES)
		cmp er, nerj
		bge EndOf_LoopER_CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_ARM64
		LoopER_CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_ARM64:
			sub j0, er, #(1*COMPV_GAS_INT16_SZ_BYTES)
			ldrh j0w, [RLCi, j0]
			ldrh j1w, [RLCi, er]
			sub j1, j1, #1
			cmp j0, #0
			cset f0, gt
			cmp j1, wminus1
			sub j0, j0, f0
			cinc j1, j1, lt
			ldrh j0w, [ERiminus1, j0, LSL #(COMPV_GAS_INT16_SHIFT_BYTES)]
			ldrh j1w, [ERiminus1, j1, LSL #(COMPV_GAS_INT16_SHIFT_BYTES)]
			tst j0, #1
			cinc j0, j0, eq
			tst j1, #1
			cset f0, eq
			sub j1, j1, f0
			cmp j1, j0
			orr j0, j0, j1, LSL #16
			add j1, ERAi, er, LSL #1 // ERAi is in int32-units while er is in int16-units
			csel j0, zero, j0, lt // reset to zero if (er1 < er0)
			str j0w, [j1] // ERAi is in int32-units while er is in int16-units
			add er, er, #(2*COMPV_GAS_INT16_SZ_BYTES)
			cmp er, nerj
			blt LoopER_CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_ARM64
		EndOf_LoopER_CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_ARM64:

		add ERiminus1, ERiminus1, width
		add RLCi, RLCi, RLCi_stride
		add ERAi, ERAi, ERAi_stride
		subs height, height, #1
		bne LoopHeight_CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_ARM64
	EndOf_LoopHeight_CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_ARM64:


	.unreq RLCi
	.unreq RLCi_stride
	.unreq ERAi
	.unreq ERAi_stride
	.unreq ERiminus1
	.unreq ner
	.unreq width
	.unreq height

	.unreq er
	.unreq j0
	.unreq j0w
	.unreq j1
	.unreq j1w
	.unreq nerj
	.unreq nerjw
	.unreq wminus1
	.unreq f0
	.unreq zero

	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__aarch64__) */

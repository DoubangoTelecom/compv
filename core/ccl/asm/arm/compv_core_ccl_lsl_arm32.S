#########################################################################
# Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__arm__) && !defined(__aarch64__)
.include "compv_common_arm32.S"

.data

.extern
 
.text

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> const int16_t* RLCi
@ arg(1) -> const compv_uscalar_t RLCi_stride
@ arg(2) -> int32_t* ERAi
@ arg(3) -> const compv_uscalar_t ERAi_stride
@ arg(4) -> const int16_t* ERiminus1
@ arg(5) -> const int16_t* ner
@ arg(6) -> const compv_uscalar_t width
@ arg(7) -> const compv_uscalar_t height
COMPV_GAS_FUNCTION_DECLARE CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_ARM32
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 8

	@@ Load arguments @@
	ldm_args r0-r7 @ load arguments in [r0-r7]
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
	j1 .req r10
	nerj .req r11

	sub width, width, #1 @ Now width = wminus1 (FIXME(dmi): ARM64 -> not needed create wminus1)
	lsl RLCi_stride, RLCi_stride, #COMPV_GAS_INT16_SHIFT_BYTES
	lsl ERAi_stride, ERAi_stride, #COMPV_GAS_INT32_SHIFT_BYTES

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (compv_uscalar_t j = 0; j < height; ++j)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopHeight_CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_ARM32:
		ldrh nerj, [ner], #(1*COMPV_GAS_INT16_SZ_BYTES)
		pld [RLCi, #(CACHE_LINE_SIZE*0)]
		pld [RLCi, #(CACHE_LINE_SIZE*1)] @ we don't expect more than #32 RLCi values
		lsl nerj, nerj, #COMPV_GAS_INT16_SHIFT_BYTES
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (er = 1; er < nerj; er += 2)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		mov er, #(1*COMPV_GAS_INT16_SZ_BYTES)
		cmp er, nerj
		bge EndOf_LoopER_CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_ARM32
		LoopER_CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_ARM32:
			sub j0, er, #(1*COMPV_GAS_INT16_SZ_BYTES)
			ldrh j0, [RLCi, j0]
			ldrh j1, [RLCi, er]
			sub j1, j1, #1
			cmp j0, #0
			subgt j0, j0, #1
			cmp j1, width @ width contains wminus1 (not true for ARM64)
			addlt j1, j1, #1
			lsl j0, j0, #COMPV_GAS_INT16_SHIFT_BYTES
			lsl j1, j1, #COMPV_GAS_INT16_SHIFT_BYTES
			ldrh j0, [ERiminus1, j0]
			ldrh j1, [ERiminus1, j1]
			tst j0, #1
			addeq j0, j0, #1
			tst j1, #1
			subeq j1, j1, #1
			cmp j1, j0
			orr j0, j0, j1, LSL #16
			eorlt j0, j0, j0 @ reset to zero if (er1 < er0)
			str j0, [ERAi, er, LSL #1] @ ERAi is in int32-units while er is in int16-units
			add er, er, #4 @ 4 = 2*sizeof(int16)
			cmp er, nerj
			blt LoopER_CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_ARM32
		EndOf_LoopER_CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_ARM32:

		add ERiminus1, ERiminus1, width, LSL #COMPV_GAS_INT16_SHIFT_BYTES
		add RLCi, RLCi, RLCi_stride
		add ERAi, ERAi, ERAi_stride
		add ERiminus1, ERiminus1, #(1*COMPV_GAS_INT16_SZ_BYTES) @ adding #1 because width contains (wminus1) - not the case for ARM64
		subs height, height, #1
		bne LoopHeight_CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_ARM32
	EndOf_LoopHeight_CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_ARM32:

	COMPV_GAS_UNSHADOW_ARGS 8
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__arm__) && !defined(__aarch64__) */

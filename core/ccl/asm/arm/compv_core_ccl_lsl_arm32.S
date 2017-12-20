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

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (compv_uscalar_t j = 0; j < height; ++j)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopHeight_CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_ARM32:
		ldrh nerj, [ner]!
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (er = 1; er < nerj; er += 2)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		mov er, #1
		cmp er, nerj
		bge EndOf_LoopER_CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_ARM32
		LoopER_CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_ARM32:
			@add j0, ERi, #-(1*COMPV_GAS_INT16_SZ_BYTES)
			@ldrh j0, [RLCi,]
		EndOf_LoopER_CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_ARM32:

		subs height, height, #1
		bne LoopHeight_CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_ARM32
	EndOf_LoopHeight_CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_ARM32:

	COMPV_GAS_UNSHADOW_ARGS 8
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__arm__) && !defined(__aarch64__) */

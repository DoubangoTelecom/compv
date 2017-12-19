#########################################################################
# Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__arm__) && !defined(__aarch64__)
.include "compv_common_arm32.S" // "

#if !defined(__APPLE__)
.arch armv7-a @ for rbit
#endif

.data

.align 4
mask: .word 0x8040201, 0x80402010, 0x8040201, 0x80402010

.extern
 
.text

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* Xi
@ arg(1) -> const compv_uscalar_t Xi_stride
@ arg(2) -> int16_t* ERi
@ arg(3) -> const compv_uscalar_t ERi_stride,
@ arg(4) -> int16_t* ner
@ arg(5) -> int16_t* ner_max1
@ arg(6) -> int32_t* ner_sum1
@ arg(7) -> const compv_uscalar_t width
@ arg(8) -> const compv_uscalar_t height
COMPV_GAS_FUNCTION_DECLARE CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON32
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 9
	COMPV_GAS_SAVE_NEON_REGS

	@@ Load arguments @@
	ldm_args r0-r8 @ load arguments in [r0-r8]
	Xi .req r0



	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 9
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN



@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> const uint8_t* Xi, 
@ arg(1) -> const compv_uscalar_t Xi_stride,
@ arg(2) -> int16_t* ERi, 
@ arg(3) -> const compv_uscalar_t ERi_stride,
@ arg(4) -> int16_t* RLCi, 
@ arg(5) -> const compv_uscalar_t RLCi_stride,
@ arg(6) -> const compv_uscalar_t width, 
@ arg(7) -> const compv_uscalar_t height
COMPV_GAS_FUNCTION_DECLARE CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON32
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 8
	COMPV_GAS_SAVE_NEON_REGS

	@@ Load arguments @@
	ldm_args r0-r7 @ load arguments in [r0-r7]
	Xi .req r0
	Xi_stride .req r1
	ERi .req r2
	ERi_stride .req r3
	RLCi .req r4
	RLCi_stride .req r5
    width .req r6
    height .req r7

	vecMask .req q10
	width16 .req r8
	i .req r9
	t0 .req r10
	t1 .req r11

	sub ERi_stride, width, ERi_stride @ ERi_stride now contains pad
	lsl RLCi_stride, RLCi_stride, #(COMPV_GAS_INT16_SHIFT_BYTES) @ int16 -> byte
	lsl ERi_stride, ERi_stride, #(COMPV_GAS_INT16_SHIFT_BYTES) @ int16 -> byte
	sub width16, width, #1
	and width16, width16, #-16
	

	ldr t0, =mask
	vld1.u32 {vecMask}, [t0]

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (compv_uscalar_t j = 0; j < height; ++j)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopHeight_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON32:
		add ERi, ERi, #(1*COMPV_GAS_INT16_SZ_BYTES) @ i starts at #1
		mov i, #0
		ldrb t0, [Xi]
		and t0, #1
		strh i, [RLCi]
		add RLCi, RLCi, t0, LSL #(COMPV_GAS_INT16_SHIFT_BYTES)

		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (i = 1; i < width16; i += 16)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		mov i, #1
		LoopWidth16_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON32:
			add t0, ERi, #-(1*COMPV_GAS_INT16_SZ_BYTES)
			vld1.s16 {q0, q1}, [t0]
			vld1.s16 {q2, q3}, [ERi]!
			vceq.s16 q0, q0, q2
			vceq.s16 q1, q1, q3
			vqmovn.u16 q0x, q0
			vqmovn.u16 q0y, q1
			vmvn.u8 q0, q0
			vorr q1x, q0x, q0y
			vmov.32	t0, q1x[0]
			vmov.32	t1, q1x[1]
			orrs t0, t0, t1
			beq EndOfMask_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON32
				vand.u8 q0, q0, vecMask
				vpadd.u8 q0x, q0x, q0y
				vpadd.u8 q0x, q0x, q0x
				vpadd.u8 q0x, q0x, q0x
				vmov.u16 t0, q0x[0]
				rbit t0, t0
				BeginOfWhile_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON32:
				@ ARM there is no ctz (Count Trailing Zero): ctz = 31 - clz(t0)
				@ FIXME(dmi): ARM64 -> code could be better with more registers: lsl #1, t1 then xor
				clz t1, t0
				lsl t0, t0, t1
				bic t0, t0, #0x80000000
				lsrs t0, t0, t1
				add t1, t1, i
				strh t1, [RLCi], #(1*COMPV_GAS_INT16_SZ_BYTES)	
				bne BeginOfWhile_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON32
				EndOf_BeginOfWhile_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON32:
			EndOfMask_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON32:

			add i, i, #16
			cmp i, width16
			blt LoopWidth16_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON32
		EndOf_LoopWidth16_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON32:

		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (; i < width1; ++i)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		cmp i, width
		bge EndOf_LoopWidth1_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON32
		LoopWidth1_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON32:
			ldrh t0, [ERi, #-(1*COMPV_GAS_INT16_SZ_BYTES)]
			ldrh t1, [ERi], #(1*COMPV_GAS_INT16_SZ_BYTES)
			eors t0, t0, t1
			beq Next_LoopWidth1_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON32
				strh i, [RLCi], #(1*COMPV_GAS_INT16_SZ_BYTES)
			Next_LoopWidth1_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON32:
			add i, i, #1
			cmp i, width
			blt LoopWidth1_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON32
		EndOf_LoopWidth1_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON32:

		sub t0, width, #1
		ldrb t0, [Xi, t0]
		and t0, t0, #1
		eor t0, t0, #1
		sub t0, width, t0
		strh t0, [RLCi]

		@ next @
		ldr_arg 4, RLCi
		add Xi, Xi, Xi_stride
		add RLCi, RLCi, RLCi_stride
		add ERi, ERi, ERi_stride
		str_arg 4, RLCi
		subs height, height, #1
		bne LoopHeight_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON32
	EndOf_LoopHeight_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON32:

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 8
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__arm__) && !defined(__aarch64__) */

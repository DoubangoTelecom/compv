#########################################################################
# Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__arm__) && !defined(__aarch64__)
.include "compv_common_arm32.S"

#if !defined(PIC) // Position-Independent Code
#	define PIC	1
#endif

.data
#if !PIC
	.align 4
	mask: .word 0x8040201, 0x80402010, 0x8040201, 0x80402010
#else
	.equ mask0, 0x8040201
    .equ mask1, 0x80402010
#	if !defined(__APPLE__)
		.arch armv7-a @ for movw and movt
#	endif
#endif

.extern
 
.text

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* Xi
@ arg(1) -> const compv_uscalar_t Xi_stride
@ arg(2) -> int16_t* ERi
@ arg(3) -> int16_t* ner
@ arg(4) -> int16_t* ner_max1
@ arg(5) -> int32_t* ner_sum1
@ arg(6) -> const compv_uscalar_t width
@ arg(7) -> const compv_uscalar_t height
COMPV_GAS_FUNCTION_DECLARE CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON32
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 8
	COMPV_GAS_SAVE_NEON_REGS

	@@ Load arguments @@
	ldm_args r0-r7 @ load arguments in [r0-r7]
	Xi .req r0
	Xi_stride .req r1
	ERi .req r2
	ner .req r3
	ner_max1 .req r4
	ner_sum1 .req r5
	width .req r6
	height .req r7

	er .req r8
	i .req r9
	t0 .req r10
	t1 .req r11

	vecER .req q14
	vecERx .req q14x
	vecERy .req q14y
	vecOne .req q15

	pld [Xi, #(CACHE_LINE_SIZE*0)]
	pld [Xi, #(CACHE_LINE_SIZE*1)]
	pld [Xi, #(CACHE_LINE_SIZE*2)]
	pld [Xi, #(CACHE_LINE_SIZE*3)]

	mov t0, #1
	eor ner_max1, ner_max1
	eor ner_sum1, ner_sum1
	vdup.u8 vecOne, t0
	sub Xi_stride, Xi_stride, width @ now Xi_stride contains padding

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (compv_uscalar_t j = 0; j < height; ++j)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopHeight_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON32:
		ldrb er, [Xi], #(1*COMPV_GAS_UINT8_SZ_BYTES)
		and er, er, #1
		strh er, [ERi], #(1*COMPV_GAS_INT16_SZ_BYTES)
		vdup.s16 vecER, er
		
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (i = 1; i < width16; i += 16)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		mov i, #(1 + 16) @ +16 to be 1 step ahea
		LoopWidth16_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON32:
			add t0, Xi, #-(1*COMPV_GAS_UINT8_SZ_BYTES)
			pld [Xi, #(CACHE_LINE_SIZE*4)]
			vld1.u8 {q0}, [t0 :128]
			vld1.u8 {q1}, [Xi]!
			veor.u8 q0, q0, q1
			vand.u8 q0, q0, vecOne
			@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			@ if (mask & 0xff)
			@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			vcmp.f64 q0x, #0
			vmrs APSR_nzcv, fpscr	
			bne TestMask1_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON32
			vst1.s16 {vecER}, [ERi]!
			b EndOf_TestMask1_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON32
			TestMask1_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON32:
				vmov.32 t0, q0x[0]
				vmov.32 t1, q0x[1]
				tst t0, #(1 << (0*8))
				addne er, er, #1
				strh er, [ERi], #(1*COMPV_GAS_INT16_SZ_BYTES)
				tst t0, #(1 << (1*8))
				addne er, er, #1
				strh er, [ERi], #(1*COMPV_GAS_INT16_SZ_BYTES)
				tst t0, #(1 << (2*8))
				addne er, er, #1
				strh er, [ERi], #(1*COMPV_GAS_INT16_SZ_BYTES)
				tst t0, #(1 << (3*8))
				addne er, er, #1
				strh er, [ERi], #(1*COMPV_GAS_INT16_SZ_BYTES)
				tst t1, #(1 << (0*8))
				addne er, er, #1
				strh er, [ERi], #(1*COMPV_GAS_INT16_SZ_BYTES)
				tst t1, #(1 << (1*8))
				addne er, er, #1
				strh er, [ERi], #(1*COMPV_GAS_INT16_SZ_BYTES)
				tst t1, #(1 << (2*8))
				addne er, er, #1
				strh er, [ERi], #(1*COMPV_GAS_INT16_SZ_BYTES)
				tst t1, #(1 << (3*8))
				addne er, er, #1
				strh er, [ERi], #(1*COMPV_GAS_INT16_SZ_BYTES)
				vdup.s16 vecER, er
			EndOf_TestMask1_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON32:

			@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			@ if (mask & 0xff00)
			@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			vcmp.f64 q0y, #0
			vmrseq APSR_nzcv, fpscr
			bne TestMask2_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON32
			vst1.s16 {vecER}, [ERi]!
			b EndOf_TestMask2_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON32
			TestMask2_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON32:
				vmov.32 t0, q0y[0]
				vmov.32 t1, q0y[1]
				tst t0, #(1 << (0*8))
				addne er, er, #1
				strh er, [ERi], #(1*COMPV_GAS_INT16_SZ_BYTES)
				tst t0, #(1 << (1*8))
				addne er, er, #1
				strh er, [ERi], #(1*COMPV_GAS_INT16_SZ_BYTES)
				tst t0, #(1 << (2*8))
				addne er, er, #1
				strh er, [ERi], #(1*COMPV_GAS_INT16_SZ_BYTES)
				tst t0, #(1 << (3*8))
				addne er, er, #1
				strh er, [ERi], #(1*COMPV_GAS_INT16_SZ_BYTES)
				tst t1, #(1 << (0*8))
				addne er, er, #1
				strh er, [ERi], #(1*COMPV_GAS_INT16_SZ_BYTES)
				tst t1, #(1 << (1*8))
				addne er, er, #1
				strh er, [ERi], #(1*COMPV_GAS_INT16_SZ_BYTES)
				tst t1, #(1 << (2*8))
				addne er, er, #1
				strh er, [ERi], #(1*COMPV_GAS_INT16_SZ_BYTES)
				tst t1, #(1 << (3*8))
				addne er, er, #1
				strh er, [ERi], #(1*COMPV_GAS_INT16_SZ_BYTES)
				vdup.s16 vecER, er
			EndOf_TestMask2_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON32:
			
			add i, i, #16
			cmp i, width
			blt LoopWidth16_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON32
		EndOf_LoopWidth16_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON32:
		
		sub i, i, #16 @ was 1 step ahead, return

		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (; i < width1; ++i) @
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		cmp i, width
		bge EndOf_LoopWidth1_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON32
		LoopWidth1_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON32:
			ldrb t0, [Xi, #-(1*COMPV_GAS_UINT8_SZ_BYTES)]
			ldrb t1, [Xi], #(1*COMPV_GAS_UINT8_SZ_BYTES)
			eor t0, t0, t1
			and t0, t0, #1
			add i, i, #1
			add er, er, t0
			cmp i, width
			strh er, [ERi], #(1*COMPV_GAS_INT16_SZ_BYTES)			
			blt LoopWidth1_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON32
		EndOf_LoopWidth1_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON32:

		ldrb t1, [Xi, #-(1*COMPV_GAS_UINT8_SZ_BYTES)]
		add Xi, Xi, Xi_stride
		and t1, t1, #1
		add er, er, t1
		add ner_sum1, ner_sum1, er
		strh er, [ner], #(1*COMPV_GAS_INT16_SZ_BYTES)
		cmp ner_max1, er
		movlt ner_max1, er
		
		subs height, height, #1
		bne LoopHeight_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON32
	EndOf_LoopHeight_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_NEON32:

	ldr_arg 4, t0
	ldr_arg 5, t1
	strh ner_max1, [t0] @ int16
	str ner_sum1, [t1] @ int32

	.unreq Xi
	.unreq Xi_stride
	.unreq ERi
	.unreq ner
	.unreq ner_max1
	.unreq ner_sum1
	.unreq width
	.unreq height

	.unreq er
	.unreq i
	.unreq t0
	.unreq t1

	.unreq vecER
	.unreq vecERx
	.unreq vecERy
	.unreq vecOne

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 8
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN



@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> const uint8_t* Xi, 
@ arg(1) -> const compv_uscalar_t Xi_stride,
@ arg(2) -> int16_t* ERi, 
@ arg(3) -> int16_t* RLCi, 
@ arg(4) -> const compv_uscalar_t RLCi_stride,
@ arg(5) -> const compv_uscalar_t width, 
@ arg(6) -> const compv_uscalar_t height
COMPV_GAS_FUNCTION_DECLARE CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON32
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 7
	COMPV_GAS_SAVE_NEON_REGS

	@@ Load arguments @@
	ldm_args r0-r6 @ load arguments in [r0-r6]
	Xi .req r0
	Xi_stride .req r1
	ERi .req r2
	RLCi .req r3
	RLCi_stride .req r4
    width .req r5
    height .req r6

	vecMask .req q10
	vecMaskx .req q10x
	vecMasky .req q10y
	width16 .req r7
	i .req r8
	t0 .req r9
	t1 .req r10
	rMask .req r11 @ Mask to simulate x86 'blsr' instruction

	lsl RLCi_stride, RLCi_stride, #(COMPV_GAS_INT16_SHIFT_BYTES) @ int16 -> byte
	sub width16, width, #1
	and width16, width16, #-16

#if !PIC
	ldr t0, =mask
	vld1.u32 {vecMask}, [t0]
#else
	movw t0, #:lower16:mask0
    movt t0, #:upper16:mask0
    movw t1, #:lower16:mask1
    movt t1, #:upper16:mask1
    vmov vecMaskx, t0, t1
    vmov vecMasky, t0, t1
#endif
	mov rMask, #0x80000000 @ << #0x8000000000000000 for ARM64

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
			pld [ERi, width, LSL #(COMPV_GAS_INT16_SHIFT_BYTES)]
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
				rbit t0, t0 @ prepare ctz
				BeginOfWhile_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON32:
					@ ARM there is no ctz (Count Trailing Zero): ctz = clz(rbit(t0))
					clz t1, t0
					bics t0, t0, rMask, LSR t1 @ clear bit at t1
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
			teq t0, t1 @ can also be "eors t0, t1"
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
		ldr_arg 3, RLCi
		add Xi, Xi, Xi_stride
		add RLCi, RLCi, RLCi_stride
		str_arg 3, RLCi
		subs height, height, #1
		bne LoopHeight_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON32
	EndOf_LoopHeight_CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_NEON32:


	.unreq Xi
	.unreq Xi_stride
	.unreq ERi
	.unreq RLCi
	.unreq RLCi_stride
    .unreq width
    .unreq height

	.unreq vecMask
	.unreq vecMaskx
	.unreq vecMasky
	.unreq width16
	.unreq i
	.unreq t0
	.unreq t1
	.unreq rMask

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 7
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__arm__) && !defined(__aarch64__) */

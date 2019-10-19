#########################################################################
# Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__arm__) && !defined(__aarch64__)
.include "compv_common_arm32.S" @

.data
#if 0 // not PIC (Position-Independent Code)
	.align 4
	data130048: .word 130048, 130048, 130048, 130048
	data1023: .word 1023, 1023, 1023, 1023
#else
	.equ data130048_word, 130048
	.equ data1023_word, 1023
#	if !defined(__APPLE__)
		.arch armv7-a @ for movw and movt
#	endif
#endif

.text

.equ FMA_YES, 1
.equ FMA_NO, 0

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const compv_float32_t* ptrIn
@ arg(1) -> COMPV_ALIGNED(NEON) compv_float32_t* ptrOut
@ arg(2) -> const compv_uscalar_t width
@ arg(3) -> const compv_uscalar_t height
@ arg(4) -> COMPV_ALIGNED(NEON) const compv_uscalar_t stride
@ arg(5) -> COMPV_ALIGNED(NEON) const uint32_t* lut32u
@ arg(6) -> COMPV_ALIGNED(NEON) const compv_float32_t* var32f
.macro CompVMathExpExp_minpack1_32f32f_Macro_NEON32 fusedMultiplyAdd
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 7
	COMPV_GAS_SAVE_NEON_REGS

	@@ Load arguments @@
	ldm_args r0-r6
	ptrIn .req r0
	ptrOut .req r1
	width .req r2
	height .req r3
	stride .req r4
	lut32u .req r5
	var32f .req r6

	i .req r7
	t0 .req r8
	t1 .req r9
	t2 .req r10
	t3 .req r11

	vecMagic .req q0
	vecMagicx .req q0x
	vecMagicy .req q0y
	vecA0 .req q1
	vecA0x .req q1x
	vecA0y .req q1y
	vecB0 .req q2
	vecB0x .req q2x
	vecB0y .req q2y
	vecMaxX .req q3
	vecMaxXx .req q3x
	vecMaxXy .req q3y
	vecMinX .req q4
	vecMinXx .req q4x
	vecMinXy .req q4y
	vec130048 .req q5
	vec1023 .req q6
	vecX .req q7
	vecFi .req q8
	vecT .req q9
	vecU .req q10
	vecV .req q11
	vecVx .req q11x
	vecVy .req q11y

	pld [ptrIn, #(CACHE_LINE_SIZE*0)]
	pld [ptrIn, #(CACHE_LINE_SIZE*1)]
	pld [ptrIn, #(CACHE_LINE_SIZE*2)]
	pld [ptrIn, #(CACHE_LINE_SIZE*3)]

	@ Transform stride to padding @
	add i, width, #3
	and i, i, #-4
	sub stride, stride, i
	lsl stride, stride, #(COMPV_GAS_FLOAT32_SHIFT_BYTES)

	vld1.32 {vecMagicx[], vecMagicy[]}, [var32f]! @ duplicate
	vld1.32 {vecA0x[], vecA0y[]}, [var32f]! @ duplicate
	vld1.32 {vecB0x[], vecB0y[]}, [var32f]! @ duplicate
	vld1.32 {vecMaxXx[], vecMaxXy[]}, [var32f]! @ duplicate
	vld1.32 {vecMinXx[], vecMinXy[]}, [var32f]! @ duplicate
	
#if 0 // not PIC (Position-Independent Code)
	ldr i, =data130048
	vld1.u32 {vec130048}, [i]
	ldr i, =data1023
	vld1.u32 {vec1023}, [i]
#else
	movw t0, #:lower16:data130048_word
    movt t0, #:upper16:data130048_word
	movw t1, #:lower16:data1023_word
    movt t1, #:upper16:data1023_word
	vdup.u32 vec130048, t0
	vdup.u32 vec1023, t1
#endif

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (compv_uscalar_t j = 0; j < height; ++j)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopHeigth_CompVMathExpExp_minpack1_32f32f_Asm_NEON32\@:
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (i = 0; i < width; i += 4)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		mov i, #0
		LoopWidth_CompVMathExpExp_minpack1_32f32f_Asm_NEON32\@:
			vld1.f32 { vecX }, [ptrIn :128]!
			pld [ptrIn, #(CACHE_LINE_SIZE*4)]

			vmin.f32 vecX, vecX, vecMaxX
			vmax.f32 vecX, vecX, vecMinX
			vmov vecFi, vecMagic
			.if \fusedMultiplyAdd == FMA_YES
				vfma.f32 vecFi, vecX, vecA0
			.else
				vmla.f32 vecFi, vecX, vecA0
			.endif
			vsub.f32 vecT, vecFi, vecMagic
			.if \fusedMultiplyAdd == FMA_YES
				vfms.f32 vecX, vecT, vecB0 @ vecX = vecT from intrin code
			.else
				vmls.f32 vecX, vecT, vecB0 @ vecX = vecT from intrin code
			.endif

			vadd.u32 vecU, vecFi, vec130048
			vand.u32 vecV, vecFi, vec1023
			vshr.u32 vecU, vecU, #10
			vshl.u32 vecU, vecU, #23

			vmov t0, t1, vecVx
			vmov t2, t3, vecVy
			ldr t0, [lut32u, t0, LSL #(COMPV_GAS_UINT32_SHIFT_BYTES)]
			ldr t1, [lut32u, t1, LSL #(COMPV_GAS_UINT32_SHIFT_BYTES)]
			ldr t2, [lut32u, t2, LSL #(COMPV_GAS_UINT32_SHIFT_BYTES)]
			ldr t3, [lut32u, t3, LSL #(COMPV_GAS_UINT32_SHIFT_BYTES)]			
			vmov vecVx, t0, t1
			vmov vecVy, t2, t3
			vorr.u32 vecV, vecV, vecU

			.if \fusedMultiplyAdd == FMA_YES
				vfma.f32 vecV, vecX, vecV
			.else
				vmla.f32 vecV, vecX, vecV
			.endif
			vst1.f32 { vecV }, [ptrOut :128]!

			add i, i, #4
			cmp i, width
			blt LoopWidth_CompVMathExpExp_minpack1_32f32f_Asm_NEON32\@
		EndOf_LoopWidth_CompVMathExpExp_minpack1_32f32f_Asm_NEON32\@:

		subs height, height, #1
		add ptrIn, ptrIn, stride
		add ptrOut, ptrOut, stride
		bne  LoopHeigth_CompVMathExpExp_minpack1_32f32f_Asm_NEON32\@
	EndOf_LoopHeigth_CompVMathExpExp_minpack1_32f32f_Asm_NEON32\@:

	.unreq ptrIn
	.unreq ptrOut
	.unreq width
	.unreq height
	.unreq stride
	.unreq lut32u
	.unreq var32f

	.unreq i
	.unreq t0
	.unreq t1
	.unreq t2
	.unreq t3

	.unreq vecMagic
	.unreq vecMagicx
	.unreq vecMagicy
	.unreq vecA0
	.unreq vecA0x
	.unreq vecA0y
	.unreq vecB0
	.unreq vecB0x
	.unreq vecB0y
	.unreq vecMaxX
	.unreq vecMaxXx
	.unreq vecMaxXy
	.unreq vecMinX
	.unreq vecMinXx
	.unreq vecMinXy
	.unreq vec130048
	.unreq vec1023
	.unreq vecX
	.unreq vecFi
	.unreq vecT
	.unreq vecU
	.unreq vecV
	.unreq vecVx
	.unreq vecVy

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 7
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVMathExpExp_minpack1_32f32f_Asm_NEON32
	CompVMathExpExp_minpack1_32f32f_Macro_NEON32 FMA_NO

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVMathExpExp_minpack1_32f32f_Asm_FMA_NEON32
	CompVMathExpExp_minpack1_32f32f_Macro_NEON32 FMA_YES

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> const compv_float64_t* ptrIn
@ arg(1) -> compv_float64_t* ptrOut
@ arg(2) -> const compv_uscalar_t width
@ arg(3) -> const compv_uscalar_t height
@ arg(4) -> const compv_uscalar_t stride
@ arg(5) -> const uint64_t* lut64u
@ arg(6) -> const uint64_t* var64u
@ arg(7) -> const compv_float64_t* var64f
.macro CompVMathExpExp_minpack1_64f64f_Macro_NEON32 fusedMultiplyAdd
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 8
	COMPV_GAS_SAVE_NEON_REGS
	@@ end prolog @@

	@ Load arguments @
	ldm_args r0-r7
	ptrIn .req r0
	ptrOut .req r1
	width .req r2
	height .req r3
	stride .req r4
	lut64u .req r5
	var64u .req r6
	var64f .req r7

	@ Local Variables @
    i .req r8
	rt0 .req r9
	vecC10 .req q0x
	vecMax .req q0y
	vecMin .req q1x
	vecDI .req q1y
	vecB .req q2x
	vecCA .req q2y
	vecT .req q3x
	vec0 .req q3y
	vecCRA .req q4x
	vecCADJ .req q4y
	vecU .req q5x
	vecY .req q5y
	vecMask .req q6x
	vecC30 .req q6y
	vecLUT .req q7x
	vecC20 .req q7y

	pld [ptrIn, #(CACHE_LINE_SIZE*0)]
	pld [ptrIn, #(CACHE_LINE_SIZE*1)]
	pld [ptrIn, #(CACHE_LINE_SIZE*2)]

	vld1.u64 { vecMask }, [var64u :64]!
	vld1.u64 { vecCADJ }, [var64u :64]

	vld1.f64 { vecB }, [var64f :64]!
	vld1.f64 { vecCA }, [var64f :64]!
	vld1.f64 { vecCRA }, [var64f :64]!
	vld1.f64 { vecC10 }, [var64f :64]!
	vld1.f64 { vecC20 }, [var64f :64]!
	vld1.f64 { vecC30 }, [var64f :64]!
	vld1.f64 { vecMin }, [var64f :64]!
	vld1.f64 { vecMax }, [var64f :64]

	@ Transform stride to padding
	sub stride, stride, width
	lsl stride, stride, #(COMPV_GAS_FLOAT64_SHIFT_BYTES)

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (compv_uscalar_t j = 0; j < height; ++j)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopHeight_CompVMathExpExp_minpack1_64f64f_Asm_NEON32\@:
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (compv_uscalar_t i = 0; i < width; i += 1)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		mov i, width
		LoopWidth1_CompVMathExpExp_minpack1_64f64f_Asm_NEON32\@:
			vld1.f64 { vecT }, [ptrIn]!
			pld [ptrIn, #(CACHE_LINE_SIZE*3)]
			vmov.f64 vecDI, vecB @ Needed only for FMA or ARM32 
			@vmin.f64 vecT, vecT, vecMax
			vcmp.f64 vecT, vecMax
			vmrs APSR_nzcv, fpscr
			vmovpl.f64 vecT, vecMax
			@vmax.f64 vecT, vecT, vecMin
			vcmp.f64 vecT, vecMin
			vmrs APSR_nzcv, fpscr
			vmovmi.f64 vecT, vecMin
			
			.if \fusedMultiplyAdd
				vfma.f64 vecDI, vecT, vecCA
			.else
				vmla.f64 vecDI, vecT, vecCA
			.endif
			vsub.f64 vec0, vecDI, vecB
			vadd.u64 vecU, vecDI, vecCADJ
			.if \fusedMultiplyAdd
				vfms.f64 vecT, vec0, vecCRA
			.else
				vmls.f64 vecT, vec0, vecCRA
			.endif
			vshr.u64 vecU, vecU, #11
			vshl.u64 vecU, vecU, #52
			vmul.f64 vecY, vecT, vecT
			vand.u64 vecDI, vecDI, vecMask
			vadd.f64 vec0, vecC30, vecT
			vmov.u32 rt0, vecDI[0]
			vmul.f64 vecY, vecY, vec0
			add rt0, lut64u, rt0, LSL #(COMPV_GAS_UINT64_SHIFT_BYTES)
			vld1.f64 { vecLUT }, [rt0]
			.if \fusedMultiplyAdd
				vfma.f64 vecT, vecY, vecC20
			.else
				vmla.f64 vecT, vecY, vecC20
			.endif
			vorr.u64 vecU, vecU, vecLUT
			vadd.f64 vecY, vecC10, vecT
			vmul.f64 vecY, vecY, vecU
			subs i, i, #1
			vst1.f64 { vecY }, [ptrOut]!
			bne LoopWidth1_CompVMathExpExp_minpack1_64f64f_Asm_NEON32\@
		EndOf_LoopWidth1_CompVMathExpExp_minpack1_64f64f_Asm_NEON32\@:

		add ptrIn, ptrIn, stride
		add ptrOut, ptrOut, stride
		subs height, height, #1
		bne LoopHeight_CompVMathExpExp_minpack1_64f64f_Asm_NEON32\@
	EndOf_LoopHeight_CompVMathExpExp_minpack1_64f64f_Asm_NEON32\@:

	.unreq ptrIn
	.unreq ptrOut
	.unreq width
	.unreq height
	.unreq stride
	.unreq lut64u
	.unreq var64u
	.unreq var64f

	.unreq i
	.unreq rt0
	.unreq vecC10
	.unreq vecMax
	.unreq vecMin
	.unreq vecDI
	.unreq vecB
	.unreq vecCA
	.unreq vecT
	.unreq vec0
	.unreq vecCRA
	.unreq vecCADJ
	.unreq vecU
	.unreq vecY
	.unreq vecMask
	.unreq vecC30
	.unreq vecLUT
	.unreq vecC20

	@@ begin epilog @@
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 8
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVMathExpExp_minpack1_64f64f_Asm_NEON32
	CompVMathExpExp_minpack1_64f64f_Macro_NEON32 0

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVMathExpExp_minpack1_64f64f_Asm_FMA_NEON32
	CompVMathExpExp_minpack1_64f64f_Macro_NEON32 1

#endif /* defined(__arm__) && !defined(__aarch64__) */

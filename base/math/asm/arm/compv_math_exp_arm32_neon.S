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

.text

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> const compv_float64_t* ptrIn
@ arg(1) -> compv_float64_t* ptrOut
@ arg(2) -> const compv_uscalar_t width
@ arg(3) -> const compv_uscalar_t height
@ arg(4) -> const compv_uscalar_t stride
@ arg(5) -> const uint64_t* lut64u
@ arg(6) -> const uint64_t* var64u
@ arg(7) -> const compv_float64_t* var64f
COMPV_GAS_FUNCTION_DECLARE CompVMathExpExp_minpack1_64f64f_Asm_NEON32
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
	LoopHeight_CompVMathExpExp_minpack1_64f64f_Asm_NEON32:
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (compv_uscalar_t i = 0; i < width; i += 1)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		mov i, width
		LoopWidth1_CompVMathExpExp_minpack1_64f64f_Asm_NEON32:
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
			
			vmla.f64 vecDI, vecT, vecCA
			vsub.f64 vec0, vecDI, vecB
			vadd.u64 vecU, vecDI, vecCADJ
			vmls.f64 vecT, vec0, vecCRA
			vshr.u64 vecU, vecU, #11
			vshl.u64 vecU, vecU, #52
			vmul.f64 vecY, vecT, vecT
			vand.u64 vecDI, vecDI, vecMask
			vadd.f64 vec0, vecC30, vecT
			vmov.u32 rt0, vecDI[0]
			vmul.f64 vecY, vecY, vec0
			add rt0, lut64u, rt0, LSL #(COMPV_GAS_UINT64_SHIFT_BYTES)
			vld1.f64 { vecLUT }, [rt0]
			vmla.f64 vecT, vecY, vecC20
			vorr.u64 vecU, vecU, vecLUT
			vadd.f64 vecY, vecC10, vecT
			vmul.f64 vecY, vecY, vecU
			subs i, i, #1
			vst1.f64 { vecY }, [ptrOut]!
			bne LoopWidth1_CompVMathExpExp_minpack1_64f64f_Asm_NEON32
		EndOf_LoopWidth1_CompVMathExpExp_minpack1_64f64f_Asm_NEON32:

		add ptrIn, ptrIn, stride
		add ptrOut, ptrOut, stride
		subs height, height, #1
		bne LoopHeight_CompVMathExpExp_minpack1_64f64f_Asm_NEON32
	EndOf_LoopHeight_CompVMathExpExp_minpack1_64f64f_Asm_NEON32:

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

#endif /* defined(__arm__) && !defined(__aarch64__) */

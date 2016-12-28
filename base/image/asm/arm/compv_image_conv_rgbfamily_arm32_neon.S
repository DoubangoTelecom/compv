@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>	@
@ File author: Mamadou DIOP (Doubango Telecom, France).					@
@ License: GPLv3. For commercial license please contact us.				@
@ Source code: https://github.com/DoubangoTelecom/compv					@
@ WebSite: http://compv.org												@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.include "compv_common_arm.S"

.section .data

.extern k16_i16

.section .text
  
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* rgb24Ptr
@ arg(1) -> COMPV_ALIGNED(NEON) uint8_t* outYPtr
@ arg(2) -> compv_uscalar_t width
@ arg(3) -> compv_uscalar_t height
@ arg(4) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
@ arg(5) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_YCoeffs8
COMPV_GAS_FUNCTION_DECLARE CompVImageConvRgb24family_to_y_Asm_NEON32
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK(6)
	COMPV_GAS_SAVE_NEON_REGS

	@ load additional arguments
	ldr_arg 4, r4
	ldr_arg 5, r5

	add r6, r2, #15
	and r6, r6, #-16
	sub r7, r4, r6 @r7 = padY
	add r8, r7, r7, LSL #1 @r8 = padRGB

	ldr r9, =k16_i16
	vld1.16 {d0, d1}, [r9] @ {d0, d1} = q0 = xmm16
	vld4.8 {d2, d3, d4, d5}, [r5] @ d2 = xmmCoeff0, d3 = xmmCoeff1, d4, xmmCoeff2

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (j = 0; j < height; ++j)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopHeight:
		mov r9, #0
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (i = 0; i < width; i += 16)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		LoopWidth:
			add r9, r9, #16
			vld3.8 {d5, d6, d7}, [r0]!
			vld3.8 {d8, d9, d10}, [r0]!
			vmull.u8 q6, d2, d5
			vmull.u8 q7, d2, d8
			vmlal.u8 q6, d3, d6
			vmlal.u8 q7, d3, d9
			vmlal.u8 q6, d4, d7
			vmlal.u8 q7, d4, d10
			vshr.u16 q6, q6, #7
			vshr.u16 q7, q7, #7
			vadd.u16 q6, q6, q0
			vadd.u16 q7, q7, q0
			vqmovun.s16 d5, q6
			vqmovun.s16 d6, q7
			vstmia r1!, {d5, d6}
			cmp r9, r2
			blt LoopWidth
			@End_of_LoopWidth

		add r1, r1, r7
		add r0, r0, r8
		subs r3, r3, #1
		bne LoopHeight		
		@End_of_LoopHeight
	
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS(6)
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


@ CompVImageConvRgb24family_to_uv_planar_11_Asm_NEON32


	
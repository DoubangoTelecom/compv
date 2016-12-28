@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   @
@ File author: Mamadou DIOP (Doubango Telecom, France).                 @
@ License: GPLv3. For commercial license please contact us.             @
@ Source code: https://github.com/DoubangoTelecom/compv                 @
@ WebSite: http://compv.org                                             @
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.include "compv_common_arm.S"

.section .data

.extern k16_i16, k128_i16
 
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
	COMPV_GAS_SHADOW_ARGS_TO_STACK 6
	COMPV_GAS_SAVE_NEON_REGS
	
	ldmia_args r0-r5

	add r6, r2, #15
	and r6, r6, #-16
	sub r7, r4, r6 @r7 = padY
	add r8, r7, r7, LSL #1 @r8 = padRGB

	ldr r9, =k16_i16
	vld1.16 {d0, d1}, [r9 : 128] @ {d0, d1} = q0 = xmm16
	vld4.8 {d2, d3, d4, d5}, [r5 : 128] @ d2 = xmmCoeff0, d3 = xmmCoeff1, d4, xmmCoeff2

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (j = 0; j < height; ++j)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopHeight_CompVImageConvRgb24family_to_y_Asm_NEON32:
		mov r9, #0
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (i = 0; i < width; i += 16)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		LoopWidth_CompVImageConvRgb24family_to_y_Asm_NEON32:
			add r9, r9, #16
			vld3.8 {d5, d6, d7}, [r0 : 64]! @ FIXME For RGB32: add alignment
			vld3.8 {d8, d9, d10}, [r0]! @ FIXME For RGB32: add alignment
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
			vst1.u8 {d5, d6}, [r1 : 128]! @ vstmia r1!, {d5, d6}
			cmp r9, r2
			blt LoopWidth_CompVImageConvRgb24family_to_y_Asm_NEON32
			@End_of_LoopWidth

		add r1, r1, r7
		add r0, r0, r8
		subs r3, r3, #1
		bne LoopHeight_CompVImageConvRgb24family_to_y_Asm_NEON32	
		@End_of_LoopHeight
	
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 6
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* rgb24Ptr
@ arg(1) -> COMPV_ALIGNED(NEON) uint8_t* outUPtr
@ arg(2) -> COMPV_ALIGNED(NEON) uint8_t* outVPtr
@ arg(3) -> compv_uscalar_t width
@ arg(4) -> compv_uscalar_t height
@ arg(5) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
@ arg(6) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_UCoeffs8
@ arg(7) -> COMPV_ALIGNED(DEFAULT) const int8_t* kRGBfamilyToYUV_VCoeffs8
COMPV_GAS_FUNCTION_DECLARE CompVImageConvRgb24family_to_uv_planar_11_Asm_NEON32
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 8
	COMPV_GAS_SAVE_NEON_REGS

	ldmia_args r0-r7

	add r8, r3, #15
	and r8, r8, #-16
	sub r9, r5, r8 @r9 = padUV
	add r8, r9, r9, LSL #1 @r8 = padRGB
	
	vld4.s8 {d0, d1, d2, d3}, [r6 : 128]
	vmovl.s8 q2, d0 @ q2 = xmmCoeffU0
	vmovl.s8 q3, d1 @ q3 = xmmCoeffU1
	vmovl.s8 q4, d2 @ q4 = xmmCoeffU2
	vld4.s8 {d0, d1, d2, d3}, [r7 : 128]
	vmovl.s8 q5, d0 @ q5 = xmmCoeffV0
	vmovl.s8 q6, d1 @ q6 = xmmCoeffV1
	vmovl.s8 q7, d2 @ q7 = xmmCoeffV2

	ldr r10, =k128_i16
	vld1.u16 {d0, d1}, [r10 : 128] @ {d0, d1} = q0 = xmm128

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ for (j = 0; j < height; ++j)
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	LoopHeight_CompVImageConvRgb24family_to_uv_planar_11_Asm_NEON32:
		mov r6, #0
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (i = 0; i < width; i += 16)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		LoopWidth_CompVImageConvRgb24family_to_uv_planar_11_Asm_NEON32:
			add r6, r6, #16
			vld3.8 {d16, d17, d18}, [r0 : 64]! @ FIXME For RGB32: add alignment
			vld3.8 {d19, d20, d21}, [r0]! @ FIXME For RGB24: add alignment
			@ First part(R)
			vmovl.u8 q11, d16 @q11 = xmm0
			vmovl.u8 q12, d19 @q12 = xmm1
			vmul.i16 q13, q2, q11
			vmul.s16 q14, q5, q11
			vmovl.u8 q11, d17 @q11 = xmm0
			vmul.s16 q15, q2, q12
			vmul.s16 q1, q5, q12
			vmovl.u8 q12, d20 @q12 = xmm1
			@ Second part(G)
			vmlaq.s16 q13, q3, q11
			vmlaq.s16 q14, q6, q11
			vmovl.u8 q11, d18 @q11 = xmm0
			vmlaq.s16 q15, q3, q12
			vmlaq.s16 q1, q6, q12
			vmovl.u8 q12, d21 @q12 = xmm1
			@ Third part(B)
			vmlaq.s16 q13, q4, q11
			vmlaq.s16 q14, q7, q11
			vmlaq.s16 q15, q4, q12
			vmlaq.s16 q1, q7, q12
			@ >> 8
			vshr.s16 q13, q13, #8
			vshr.s16 q14, q14, #8
			vshr.s16 q15, q15, #8
			vshr.s16 q1, q1, #8
			@ + 128
			vadd.s16 q13, q13, q0
			vadd.s16 q14, q14, q0
			vadd.s16 q15, q15, q0
			vadd.s16 q1, q1, q0
			@ outPtr = concat(saturate(xmm0), saturate(xmm1))
			vqmovun.s16 d22, q13
			vqmovun.s16 d23, q15
			vqmovun.s16 d24, q14
			vqmovun.s16 d25, q1
			cmp r6, r3
			vst1.u8 {d22, d23}, [r1 : 128]! @ vstmia r1!, {d22, d23}
			vst1.u8 {d24, d25}, [r2 : 128]! @ vstmia r2!, {d24, d25}			
			blt LoopWidth_CompVImageConvRgb24family_to_uv_planar_11_Asm_NEON32
			@End_of_LoopWidth

		add r0, r0, r8
		add r1, r1, r9
		add r2, r2, r9
		subs r4, r4, #1
		bne LoopHeight_CompVImageConvRgb24family_to_uv_planar_11_Asm_NEON32
		@End_of_LoopHeight
	
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 8
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


	
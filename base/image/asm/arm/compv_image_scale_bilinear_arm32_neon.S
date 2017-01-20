@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   @
@ File author: Mamadou DIOP (Doubango Telecom, France).                 @
@ License: GPLv3. For commercial license please contact us.             @
@ Source code: https://github.com/DoubangoTelecom/compv                 @
@ WebSite: http://compv.org                                             @
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.include "compv_common_arm.S"

.section .data

.extern kShuffleEpi8_Deinterleave_i32
 
.section .text

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> const uint8_t* inPtr
@ arg(1) -> compv_uscalar_t inWidth
@ arg(2) -> compv_uscalar_t inHeight
@ arg(3) -> compv_uscalar_t inStride,
@ arg(4) -> COMPV_ALIGNED(NEON) uint8_t* outPtr
@ arg(5) -> compv_uscalar_t outWidth
@ arg(6) -> compv_uscalar_t outYStart
@ arg(7) -> compv_uscalar_t outYEnd
@ arg(8) -> COMPV_ALIGNED(NEON) compv_uscalar_t outStride
@ arg(9) -> compv_uscalar_t sf_x
@ arg(10) -> compv_uscalar_t sf_y
COMPV_GAS_FUNCTION_DECLARE CompVImageScaleBilinear_Asm_NEON32
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 11
	COMPV_GAS_SAVE_NEON_REGS
	COMPV_GAS_ALIGN_STACK 16, r11
	COMPV_GAS_MEMALLOC (7*16)
	
	@@ Declare allocated stack memory @@
	.equ vecy0                   , 0
	.equ vecy1                   , vecy0 + 16
	.equ vecSfxTimes16           , vecy1 + 16
	.equ vecSFX0                 , vecSfxTimes16 + 16
	.equ vecSFX1                 , vecSFX0 + 16
	.equ vecSFX2                 , vecSFX1 + 16
	.equ vecSFX3                 , vecSFX2 + 16

	@@ Declare input arguments @@
	ldmia_args r0-r10
	inPtr .req r0 
	inWidth .req r1 
	inHeight .req r2 
	inStride .req r3
	outPtr .req r4
	outWidth .req r5
	outYStart .req r6
	outYEnd .req r7
	outStride .req r8
	sf_x .req r9
	sf_y .req r10

	@@ Declare local vectors @@
	vecNeighb0 .req q4
	vecNeighb1 .req q5
	vecNeighb2 .req q6
	vecNeighb3 .req q7
	vecX0 .req q8
	vecX1 .req q9
	vecX2 .req q10
	vecX3 .req q11
	vec4 .req q12
	vec5 .req q13
	vec6 .req q14
	vec7 .req q15

	vecNeighb0x .dn q4x
	vecNeighb0y .dn q4y
	vecNeighb1x .dn q5x
	vecNeighb1y .dn q5y
	vecNeighb2x .dn q6x
	vecNeighb2y .dn q6y
	vecNeighb3x .dn q7x
	vecNeighb3y .dn q7y	
	vec4x .dn q12x	
	vec4y .dn q12y
	vec5x .dn q13x	
	vec5y .dn q13y
	vec6x .dn q14x	
	vec6y .dn q14y
	vec7x .dn q15x	
	vec7y .dn q15y 
 

	@@ Declare input arguments indexes @@
	.equ argi_inPtr, 0
	.equ argi_outYEnd, 7
	.equ argi_outStride, 8
	.equ argi_sf_y, 10

	.unreq inPtr    @ inPtr will be loaded on demand, we can use r0 as temp reg
	.unreq inWidth  @ inWidth is useless, we can use r1 as temp reg
	.unreq inHeight @ inHeight is useless, we can use r2 as temp reg
	.unreq outStride @ inStride will be loaded on demand, we can use r8 as temp reg
	.unreq sf_y      @sf_y will be loaded on demand, we can use r10 as temp reg
	.unreq outYEnd  @outYEnd will be loaded on demand, we can use r7 as temp reg

	@@ r0, r10 r11, r1, r2, r8, r7 can be used as temp regs @@

	@@ compute vecSfxTimes16 @@
	mov r11, sf_x, LSL #4
	add r2, sp, #vecSfxTimes16
	vdup.u32 q0, r11
	vst1.u32 {q0}, [r2 :128]

	@@ compute vecSFX0, vecSFX1, vecSFX2 and vecSFX3 @@
	mov r11, #0 
	mov r2, sf_x, LSL #1
	vmov.u32 q0x[0], r11 @ sf_x * 0
	add r11, r2, sf_x
	vmov.u32 q0x[1], sf_x @ sf_x * 1
	vmov.u32 q0y[0], r2   @ sf_x * 2
	add r2, sp, #vecSFX0
	vmov.u32 q0y[1], r11  @ sfx * 3
	mov r11, sf_x, LSL #2
	vst1.u32 {q0}, [r2 :128]! @ post-increment -> after write r2 will contain vecSFX1
	vdup.u32 q4, r11 @ q4 = vecSfxTimes4
	vadd.u32 q1, q0, q4 @ q1 = vecSFX1
	vadd.u32 q2, q1, q4 @ q2 = vecSFX2
	vadd.u32 q3, q2, q4 @ q2 = vecSFX2
	vst1.u32 {q1}, [r2 :128]! @ post-increment -> after write r2 will contain vecSFX2
	vst1.u32 {q2}, [r2 :128]! @ post-increment -> after write r2 will contain vecSFX3
	vst1.u32 {q3}, [r2 :128]

	@@ sf_x (r9) no longer needed @@
	@@ r0, r1, r2, r8, r9, r10, r11, r7 can be used as temp regs @@
	i .req r0
	inPtr_ .req r1
	t0 .req r10
	t1 .req r2
	t2 .req r9
	t3 .req r11
	t4 .req r8
	t5 .req r7

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ do
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	DoWhile:
		ldr_arg argi_inPtr, inPtr_
		mov t1, outYStart, LSR #8 @ t1 = nearestY
		mov t3, #0xff @ t3 = 0xff
		mul t2, t1, inStride @ t2 = nearestY * inStride	
		and t0, outYStart, t3
		add t1, sp, #vecSFX0
		vdup.u16 q1, t3 @ q1 = vec0xff_epi16
		vdup.u16 q0, t0 @ q0 = vecy0
		vbic.u16 q1, q1, q0 @ q1 = vecy1
		add t0, sp, #vecy0
		vst1.u32 {q0}, [t0 :128]!
		vst1.u32 {q1}, [t0 :128]		
		vld1.u32 {vecX0}, [t1 :128]!
		vld1.u32 {vecX1}, [t1 :128]!
		vld1.u32 {vecX2}, [t1 :128]!
		vld1.u32 {vecX3}, [t1 :128]
		mov i, #0
		add inPtr_, inPtr_, t2 @ inPtr_ = &inPtr[nearestY * inStride]	
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (i = 0; i < outWidth; i += 16)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		LoopWidth:
			@@ nearest x-point @@
			vshr.u32 q0, vecX0, #8
			vshr.u32 q1, vecX1, #8
			vshr.u32 q2, vecX2, #8
			vshr.u32 q3, vecX3, #8

			@@  deprecated macro function @@
			.macro _neon32_bilinear_extract_then_insert vecNeareastX, vecNeighbA, vecNeighbB, neighbIndex
				vmov.u32 t0, \vecNeareastX[0] @ t0 = nearestX0
				vmov.u32 t1, \vecNeareastX[1] @ t1 = nearestX1
				add t4, t0, inStride
				add t5, t1, inStride
				ldrh t2, [inPtr_, t0]
				ldrh t3, [inPtr_, t1]
				ldrh t4, [inPtr_, t4]
				ldrh t5, [inPtr_, t5]
				orr t2, t3, LSL #16
				orr t4, t5, LSL #16
				vmov.u32 \vecNeighbA[\neighbIndex], t2
				vmov.u32 \vecNeighbB[\neighbIndex], t4
			.endm

			@@ write memNeighbs @@
			vmov.u32 t0, q0x[0] @ t0 = nearestX0
			vmov.u32 t1, q0x[1] @ t1 = nearestX1
			vmov.u32 t2, q2x[0] @ t2 = nearestX0
			vmov.u32 t3, q2x[1] @ t3 = nearestX1
			ldrh t4, [inPtr_, t0]
			ldrh t5, [inPtr_, t1]
			add t0, t0, inStride
			orr t4, t5, LSL #16
			ldrh t5, [inPtr_, t3]
			add t1, t1, inStride
			vmov.u32 vecNeighb0x[0], t4
			ldrh t4, [inPtr_, t2]
			add t2, t2, inStride
			orr t4, t5, LSL #16
			add t3, t3, inStride
			vmov.u32 vecNeighb1x[0], t4
			ldrh t4, [inPtr_, t0]
			ldrh t5, [inPtr_, t1]
			orr t4, t5, LSL #16
			ldrh t5, [inPtr_, t3]
			vmov.u32 t0, q0y[0] @ t0 = nearestX0
			vmov.u32 t1, q0y[1] @ t1 = nearestX1
			vmov.u32 vecNeighb2x[0], t4
			ldrh t4, [inPtr_, t2]
			orr t4, t5, LSL #16
			vmov.u32 t2, q2y[0] @ t2 = nearestX0
			vmov.u32 t3, q2y[1] @ t3 = nearestX1
			vmov.u32 vecNeighb3x[0], t4
			ldrh t4, [inPtr_, t0]
			ldrh t5, [inPtr_, t1]
			orr t4, t5, LSL #16
			add t0, t0, inStride
			vmov.u32 vecNeighb0x[1], t4
			ldrh t4, [inPtr_, t2]
			ldrh t5, [inPtr_, t3]
			add t1, t1, inStride
			orr t4, t5, LSL #16
			add t2, t2, inStride
			add t3, t3, inStride
			ldrh t5, [inPtr_, t1]
			vmov.u32 vecNeighb1x[1], t4
			ldrh t4, [inPtr_, t0]
			orr t4, t5, LSL #16
			vmov.u32 t0, q1x[0] @ t0 = nearestX0
			vmov.u32 t1, q1x[1] @ t1 = nearestX1
			vmov.u32 vecNeighb2x[1], t4
			ldrh t4, [inPtr_, t2]
			ldrh t5, [inPtr_, t3]
			orr t4, t5, LSL #16
			ldrh t5, [inPtr_, t1]
			vmov.u32 t2, q3x[0] @ t2 = nearestX0
			vmov.u32 t3, q3x[1] @ t3 = nearestX1
			vmov.u32 vecNeighb3x[1], t4
			ldrh t4, [inPtr_, t0]
			add t0, t0, inStride
			orr t4, t5, LSL #16
			add t1, t1, inStride
			vmov.u32 vecNeighb0y[0], t4
			ldrh t4, [inPtr_, t2]
			ldrh t5, [inPtr_, t3]
			add t2, t2, inStride
			orr t4, t5, LSL #16
			add t3, t3, inStride
			ldrh t5, [inPtr_, t1]
			vmov.u32 vecNeighb1y[0], t4
			ldrh t4, [inPtr_, t0]
			orr t4, t5, LSL #16
			vmov.u32 t0, q1y[0] @ t0 = nearestX0
			vmov.u32 t1, q1y[1] @ t1 = nearestX1
			vmov.u32 vecNeighb2y[0], t4
			ldrh t4, [inPtr_, t2]
			ldrh t5, [inPtr_, t3]
			orr t4, t5, LSL #16
			ldrh t5, [inPtr_, t1]
			vmov.u32 t2, q3y[0] @ t2 = nearestX0
			vmov.u32 t3, q3y[1] @ t3 = nearestX1
			vmov.u32 vecNeighb3y[0], t4
			ldrh t4, [inPtr_, t0]
			add t0, t0, inStride
			orr t4, t5, LSL #16
			add t1, t1, inStride
			vmov.u32 vecNeighb0y[1], t4
			ldrh t4, [inPtr_, t2]
			ldrh t5, [inPtr_, t3]
			add t2, t2, inStride
			orr t4, t5, LSL #16
			add t3, t3, inStride
			ldrh t5, [inPtr_, t1]
			vmov.u32 vecNeighb1y[1], t4
			ldrh t4, [inPtr_, t0]
			orr t4, t5, LSL #16
			vmov.u32 vecNeighb2y[1], t4
			ldrh t4, [inPtr_, t2]
			ldrh t5, [inPtr_, t3]
			orr t4, t5, LSL #16
			ldr t0, =kShuffleEpi8_Deinterleave_i32
			add t1, sp, #vecy1
			add t2, sp, #vecSfxTimes16
			mov t3, #0xff
			vmov.u32 vecNeighb3y[1], t4			
			add t4, sp, #vecy0
			add t5, outPtr, i

			@@ Deinterleave neighbs @@
			vld1.u8 {q3}, [t0 :128] @ q3 = vecMask
			vtbx.u8 q0x, {vecNeighb0}, q3x @ q0x = low(vec0)
			vtbx.u8 q1x, {vecNeighb0}, q3y @ q1x = high(vec0)
			vtbx.u8 vec4x, {vecNeighb2}, q3x @ vec4x = low(vec2)
			vtbx.u8 vec5x, {vecNeighb2}, q3y @ vec5x = high(vec2)
			vtbx.u8 q0y, {vecNeighb1}, q3x @ q0y = low(vec1)
			vtbx.u8 q1y, {vecNeighb1}, q3y @ q1y = high(vec1)		
			vtbx.u8 vec4y, {vecNeighb3}, q3x @ vec4y = low(vec3)
			vtbx.u8 vec5y, {vecNeighb3}, q3y @ vec5y = high(vec3)
			vswp vecNeighb0, q0 @ vecNeighb0 = q0 = [low(vec0), low(vec1)]
			vswp vecNeighb1, q1 @ vecNeighb1 = q1 = [high(vec0), high(vec1)]
			vswp vecNeighb2, vec4 @ vecNeighb2 = vec4 = [low(vec2), low(vec3)]
			vswp vecNeighb3, vec5 @ vecNeighb3 = vec5 = [high(vec2), high(vec3)]

			@@ Starting is line q0 equal vec0xff_epi32 and q1 equal vec0xff_epi16
			vec0xff_epi32 .req q0
			vec0xff_epi16 .req q1			
			vdup.u32 vec0xff_epi32, t3
			vdup.u16 vec0xff_epi16, t3

			@@ compute x0 and x1 (first 8) and convert from epi32 and epi16 @@
			@ compute vec4 = (neighb0 * x1) + (neighb1 * x0) -> 8 epi16 @
			@ compute vec5 = (neighb2 * x1) + (neighb3 * x0) -> 8 epi16 @
			@@ compute x0 and x1 (second 8) and convert from epi32 and epi16 @@
			@ compute vec6 = (neighb0 * x1) + (neighb1 * x0) -> 8 epi16 @
			@ compute vec7 = (neighb2 * x1) + (neighb3 * x0) -> 8 epi16 @
			@ Let''s say:
			@		A = ((neighb0 * x1) + (neighb1 * x0))
			@		B = ((neighb2 * x1) + (neighb3 * x0))
			@ Then:
			@		A = vec4, vec6
			@		B = vec5, vec7
			@
			@ We cannot use vshrq_n_s16(vqdmulhq_s16(a, b), 1) to compute C and D because it operates on epi16 while A and B contain epu16 values
			@@ compute C = (y1 * A) >> 16 @@
			@@ compute D = (y0 * B) >> 16 @@	
			@@ Compute R = (C + D) @@
			@@ Store the result @@
			@@ move to next indices @@

			vand.u32 q2, vecX0, vec0xff_epi32
			vand.u32 q3, vecX1, vec0xff_epi32
			vmovn.u32 q2x, q2
			vmovn.u32 q2y, q3 @ q2 = vec0
			vbic q3, vec0xff_epi16, q2 @ q3 = vec1
			vmovl.u8 vec4, vecNeighb0x
			vmul.u16 vec4, vec4, q3
			vmovl.u8 vec5, vecNeighb2x
			vmul.u16 vec5, vec5, q3
			vand.u32 q3, vecX3, vec0xff_epi32
			vmovl.u8 vec6, vecNeighb1x
			vmovl.u8 vec7, vecNeighb3x
			vmla.u16 vec4, vec6, q2
			vmla.u16 vec5, vec7, q2
			vand.u32 q2, vecX2, vec0xff_epi32
			vmovn.u32 q2x, q2
			vmovn.u32 q2y, q3 @ q2 = vec0
			vbic q3, vec0xff_epi16, q2 @ q3 = vec1
			.unreq vec0xff_epi32 @ q0 no longer vec0xff_epi32
			.unreq vec0xff_epi16 @ q1 no longer vec0xff_epi16
			vmovl.u8 vecNeighb0, vecNeighb0y
			vmul.u16 vec6, vecNeighb0, q3
			vmovl.u8 vecNeighb2, vecNeighb2y
			vmul.u16 vec7, vecNeighb2, q3
			vmovl.u8 vecNeighb1, vecNeighb1y
			vmovl.u8 vecNeighb3, vecNeighb3y			
			vld1.u8 {q0}, [t1 : 128] @ q0 = vecy1
			vmull.u16 q1, q0x, vec4x
			vmull.u16 q3, q0y, vec4y
			vmla.u16 vec6, vecNeighb1, q2		
			vmla.u16 vec7, vecNeighb3, q2			
			vshrn.u32 vecNeighb0x, q1, #16
			vshrn.u32 vecNeighb0y, q3, #16
			vmull.u16 q1, q0x, vec6x
			vmull.u16 q3, q0y, vec6y
			vld1.u8 {vec4}, [t4 : 128] @ vec4 = vecy0
			vmull.u16 q2, vec4y, vec5y
			vshrn.u32 vecNeighb1x, q1, #16
			vshrn.u32 vecNeighb1y, q3, #16
			vld1.u8 {q3}, [t2 : 128] @ q3 = vecSfxTimes16
			vadd.u32 vecX0, vecX0, q3
			vmull.u16 q1, vec4x, vec5x
			vshrn.u32 vecNeighb2y, q2, #16
			vmull.u16 q2, vec4y, vec7y
			vadd.u32 vecX1, vecX1, q3
			vadd.u32 vecX2, vecX2, q3
			vshrn.u32 vecNeighb2x, q1, #16
			vmull.u16 q1, vec4x, vec7x
			vqadd.u16 vecNeighb0, vecNeighb0, vecNeighb2
			vadd.u32 vecX3, vecX3, q3
			vshrn.u32 vecNeighb3y, q2, #16
			vmovn.u16 q0x, vecNeighb0
			vshrn.u32 vecNeighb3x, q1, #16
			vqadd.u16 vecNeighb1, vecNeighb1, vecNeighb3
			vmovn.u16 q0y, vecNeighb1
			vst1.u8 {q0}, [t5 :128]

			@@
			add i, i, #16
			cmp i, outWidth
			blt LoopWidth
			@ end-of-LoopWidth @

		@@
		ldr_arg argi_sf_y, t1 @ t1 = sf_y
		ldr_arg argi_outStride, t0 @ t0 = outStride
		ldr_arg argi_outYEnd, t2 @ t2 = outYEnd
		add outYStart, outYStart, t1
		add outPtr, outPtr, t0
		cmp outYStart, t2
		blt DoWhile
		@ end-of-DoWhile @
	
	.unreq outPtr
	.unreq outWidth
	.unreq outYStart
	.unreq inStride
	.unreq sf_x

	.unreq vecNeighb0
	.unreq vecNeighb1
	.unreq vecNeighb2
	.unreq vecNeighb3
	.unreq vecX0
	.unreq vecX1
	.unreq vecX2
	.unreq vecX3
	.unreq vec4
	.unreq vec5
	.unreq vec6
	.unreq vec7

	.unreq vecNeighb0x
	.unreq vecNeighb0y
	.unreq vecNeighb1x
	.unreq vecNeighb1y
	.unreq vecNeighb2x
	.unreq vecNeighb2y
	.unreq vecNeighb3x
	.unreq vecNeighb3y
	.unreq vec4x
	.unreq vec4y
	.unreq vec5x
	.unreq vec5y
	.unreq vec6x
	.unreq vec6y
	.unreq vec7x	
	.unreq vec7y

	.unreq i
	.unreq inPtr_
	.unreq t0
	.unreq t1
	.unreq t2

	COMPV_GAS_MEMFREE (7*16)
	COMPV_GAS_UNALIGN_STACK r11
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 11
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
	
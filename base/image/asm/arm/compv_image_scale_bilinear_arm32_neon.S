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
	.equ argi_outStride, 8
	.equ argi_sf_y, 10

	.unreq inPtr    @ inPtr will be loaded on demand, we can use r0 as temp reg
	.unreq inWidth  @ inWidth is useless, we can use r1 as temp reg
	.unreq inHeight @ inHeight is useless, we can use r2 as temp reg
	.unreq outStride @ inStride will be loaded on demand, we can use r8 as temp reg
	.unreq sf_y      @sf_y will be loaded on demand, we can use r10 as temp reg

	@@ r0, r10 r11, r1, r2, r8 can be used as temp regs @@

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
	@@ r0, r1, r2, r8, r9, r10, r11 can be used as temp regs @@
	i .req r0
	inPtr_ .req r1
	outPtr_ .req r10
	tmp0 .req r2
	tmp1 .req r9
	tmp2 .req r11
	tmp3 .req r8

	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	@ do
	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	DoWhile:
		ldr_arg argi_inPtr, tmp0 @ tmp0 = inPtr @ FIXME replace direct with inPtr_
		mov tmp1, outYStart, LSR #8 @ tmp1 = nearestY
		mov tmp3, #0xff @ tmp3 = 0xff
		mul tmp2, tmp1, inStride @ tmp2 = nearestY * inStride
		add inPtr_, tmp0, tmp2 @ inPtr_ = &inPtr[nearestY * inStride]			
		and tmp0, outYStart, tmp3
		vdup.u16 q1, tmp3 @ q1 = vec0xff_epi16
		vdup.u16 q0, tmp0 @ q0 = vecy0
		vbic.u16 q1, q1, q0 @ q1 = vecy1
		add tmp0, sp, #vecy0
		add tmp1, sp, #vecSFX0
		vst1.u32 {q0}, [tmp0 :128]!
		vst1.u32 {q1}, [tmp0 :128]		
		vld1.u32 {vecX0}, [tmp1 :128]!
		vld1.u32 {vecX1}, [tmp1 :128]!
		vld1.u32 {vecX2}, [tmp1 :128]!
		vld1.u32 {vecX3}, [tmp1 :128]
		mov outPtr_, outPtr
		mov i, #0
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		@ for (i = 0; i < outWidth; i += 16)
		@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		LoopWidth:
			@@ nearest x-point @@
			vshr.u32 q0, vecX0, #8
			vshr.u32 q1, vecX1, #8
			vshr.u32 q2, vecX2, #8
			vshr.u32 q3, vecX3, #8

			@@  write memNeighbs @@
			.macro _neon32_bilinear_extract_then_insert vecNeareastX, vecNeighbA, vecNeighbB, neighbIndex0, neighbIndex1
				@ Extract indices(neareastIndex0, neareastIndex1)
				vmov.u32 tmp0, \vecNeareastX[0] @ tmp0 = nearestX0
				vmov.u32 tmp1, \vecNeareastX[1] @ tmp1 = nearestX1
				add tmp0, inPtr_, tmp0 @ tmp0 = &inPtr_[nearestX0]
				add tmp1, inPtr_, tmp1 @ tmp1 = &inPtr_[nearestX1]
				@ Insert in vecNeighbA andincrement tmp0 and tmp1 by inStride
				ldrh tmp2, [tmp0], inStride @ suffix H -> for unsigned Halfword
				ldrh tmp3, [tmp1], inStride
				vmov.u16 \vecNeighbA[\neighbIndex0], tmp2
				vmov.u16 \vecNeighbA[\neighbIndex1], tmp3
				@ Insert in vecNeighbA
				ldrh tmp2, [tmp0]
				ldrh tmp3, [tmp1]
				vmov.u16 \vecNeighbB[\neighbIndex0], tmp2
				vmov.u16 \vecNeighbB[\neighbIndex1], tmp3
			.endm

			@@ write memNeighbs @@
			_neon32_bilinear_extract_then_insert q0x, vecNeighb0x, vecNeighb2x, 0, 1
			_neon32_bilinear_extract_then_insert q0y, vecNeighb0x, vecNeighb2x, 2, 3
			_neon32_bilinear_extract_then_insert q1x, vecNeighb0y, vecNeighb2y, 0, 1
			_neon32_bilinear_extract_then_insert q1y, vecNeighb0y, vecNeighb2y, 2, 3
			_neon32_bilinear_extract_then_insert q2x, vecNeighb1x, vecNeighb3x, 0, 1
			_neon32_bilinear_extract_then_insert q2y, vecNeighb1x, vecNeighb3x, 2, 3
			_neon32_bilinear_extract_then_insert q3x, vecNeighb1y, vecNeighb3y, 0, 1
			_neon32_bilinear_extract_then_insert q3y, vecNeighb1y, vecNeighb3y, 2, 3

			@@ Deinterleave neighbs @@
			ldr tmp0, =kShuffleEpi8_Deinterleave_i32
			vld1.u8 {q3}, [tmp0 :128] @ q3 = vecMask
			vtbx.u8 q0x, {vecNeighb0}, q3x @ q0x = low(vec0)
			vtbx.u8 q1x, {vecNeighb0}, q3y @ q1x = high(vec0)
			vtbx.u8 q0y, {vecNeighb1}, q3x @ q0y = low(vec1)
			vtbx.u8 q1y, {vecNeighb1}, q3y @ q1y = high(vec1)
			vswp vecNeighb0, q0 @ vecNeighb0 = q0 = [low(vec0), low(vec1)]
			vswp vecNeighb1, q1 @ vecNeighb1 = q1 = [high(vec0), high(vec1)]
			vtbx.u8 q0x, {vecNeighb2}, q3x @ q0x = low(vec2)
			vtbx.u8 q1x, {vecNeighb2}, q3y @ q1x = high(vec2)
			vtbx.u8 q0y, {vecNeighb3}, q3x @ q0y = low(vec3)
			vtbx.u8 q1y, {vecNeighb3}, q3y @ q1y = high(vec3)
			vswp vecNeighb2, q0 @ vecNeighb2 = q0 = [low(vec2), low(vec3)]
			vswp vecNeighb3, q1 @ vecNeighb3 = q1 = [high(vec2), high(vec3)]
			
			@@ Starting is line q0 equal vec0xff_epi32 and q1 equal vec0xff_epi16
			vec0xff_epi32 .req q0
			vec0xff_epi16 .req q1
			mov tmp0, #0xff
			vdup.u32 vec0xff_epi32, tmp0
			vdup.u16 vec0xff_epi16, tmp0

			@@ compute x0 and x1 (first 8) and convert from epi32 and epi16 @@
			vand.u32 q2, vecX0, vec0xff_epi32
			vand.u32 q3, vecX1, vec0xff_epi32
			vmovn.u32 q2x, q2
			vmovn.u32 q2y, q3 @ q2 = vec0
			vbic q3, vec0xff_epi16, q2 @ q3 = vec1
			@ compute vec4 = (neighb0 * x1) + (neighb1 * x0) -> 8 epi16 @
			vmovl.u8 vec5, vecNeighb0x
			vmovl.u8 vec6, vecNeighb1x
			vmul.u16 vec4, vec5, q3
			vmla.u16 vec4, vec6, q2
			@ compute vec5 = (neighb2 * x1) + (neighb3 * x0) -> 8 epi16 @
			vmovl.u8 vec6, vecNeighb2x
			vmovl.u8 vec7, vecNeighb3x
			vmul.u16 vec5, vec6, q3
			vmla.u16 vec5, vec7, q2
			
			@@ compute x0 and x1 (second 8) and convert from epi32 and epi16 @@
			vand.u32 q2, vecX2, vec0xff_epi32
			vand.u32 q3, vecX3, vec0xff_epi32
			vmovn.u32 q2x, q2
			vmovn.u32 q2y, q3 @ q2 = vec0
			vbic q3, vec0xff_epi16, q2 @ q3 = vec1
			.unreq vec0xff_epi32 @ q0 no longer vec0xff_epi32
			.unreq vec0xff_epi16 @ q1 no longer vec0xff_epi16
			@ compute vec6 = (neighb0 * x1) + (neighb1 * x0) -> 8 epi16 @
			vmovl.u8 q0, vecNeighb0y
			vmovl.u8 q1, vecNeighb1y
			vmul.u16 vec6, q0, q3
			vmla.u16 vec6, q1, q2
			@ compute vec7 = (neighb2 * x1) + (neighb3 * x0) -> 8 epi16 @
			vmovl.u8 q0, vecNeighb2y
			vmovl.u8 q1, vecNeighb3y
			vmul.u16 vec7, q0, q3
			vmla.u16 vec7, q1, q2

			@ Let''s say:
			@		A = ((neighb0 * x1) + (neighb1 * x0))
			@		B = ((neighb2 * x1) + (neighb3 * x0))
			@ Then:
			@		A = vec4, vec6
			@		B = vec5, vec7
			@
			@ We cannot use vshrq_n_s16(vqdmulhq_s16(a, b), 1) to compute C and D because it operates on epi16 while A and B contain epu16 values

			@@ preload vecy0 and vecy1 @@
			add tmp0, sp, #vecy0
			add tmp1, sp, #vecSfxTimes16
			vld1.u8 {q3}, [tmp0 : 128]! @ q3 = vecy0
			vld1.u8 {q0}, [tmp0 : 128] @ q0 = vecy1

			@@ compute D = (y0 * B) >> 16 @@			
			vmull.u16 q1, q3x, vec5x
			vmull.u16 q2, q3y, vec5y
			vshrn.u32 vecNeighb2x, q1, #16
			vshrn.u32 vecNeighb2y, q2, #16
			vmull.u16 q1, q3x, vec7x
			vmull.u16 q2, q3y, vec7y
			vshrn.u32 vecNeighb3x, q1, #16
			vshrn.u32 vecNeighb3y, q2, #16

			@@ compute C = (y1 * A) >> 16 @@			
			vmull.u16 q1, q0x, vec4x
			vmull.u16 q2, q0y, vec4y
			vshrn.u32 vecNeighb0x, q1, #16
			vshrn.u32 vecNeighb0y, q2, #16
			vmull.u16 q1, q0x, vec6x
			vmull.u16 q2, q0y, vec6y
			vshrn.u32 vecNeighb1x, q1, #16
			vshrn.u32 vecNeighb1y, q2, #16

			@@ Compute R = (C + D) @@
			vqadd.u16 vecNeighb0, vecNeighb0, vecNeighb2
			vqadd.u16 vecNeighb1, vecNeighb1, vecNeighb3

			@@ Store the result @@
			vmovn.u16 q0x, vecNeighb0
			vmovn.u16 q0y, vecNeighb1
			vld1.u8 {q1}, [tmp1 : 128] @ q1 = vecSfxTimes16
			vst1.u8 {q0}, [outPtr_ :128]!

			@@ move to next indices @@
			vadd.u32 vecX0, vecX0, q1
			vadd.u32 vecX1, vecX1, q1
			vadd.u32 vecX2, vecX2, q1
			vadd.u32 vecX3, vecX3, q1

			@@
			add i, i, #16
			cmp i, outWidth
			blt LoopWidth
			@ end-of-LoopWidth @

		@@
		ldr_arg argi_sf_y, tmp1 @ tmp1 = sf_y
		ldr_arg argi_outStride, tmp0 @ tmp0 = outStride
		add outYStart, outYStart, tmp1
		add outPtr, outPtr, tmp0
		cmp outYStart, outYEnd
		blt DoWhile
		@ end-of-DoWhile @
	
	.unreq outPtr
	.unreq outWidth
	.unreq outYStart
	.unreq outYEnd
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
	.unreq tmp0
	.unreq tmp1
	.unreq tmp2

	COMPV_GAS_MEMFREE (7*16)
	COMPV_GAS_UNALIGN_STACK r11
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 11
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
	
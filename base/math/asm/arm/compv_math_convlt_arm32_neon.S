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

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> const uint8_t* inPtr
@ arg(1) -> uint8_t* outPtr
@ arg(2) -> compv_uscalar_t width
@ arg(3) -> compv_uscalar_t height
@ arg(4) -> compv_uscalar_t step
@ arg(5) -> compv_uscalar_t pad
@ arg(6) -> const uint16_t* vthzKernPtr
@ arg(7) -> compv_uscalar_t kernSize
COMPV_GAS_FUNCTION_DECLARE CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON32
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 8
	COMPV_GAS_SAVE_NEON_REGS
    COMPV_GAS_ALIGN_STACK 16, r11
	COMPV_GAS_MEMALLOC (16*COMPV_GAS_UINT8_SZ_BYTES)

	@@ Load arguments @@
	ldm_args r0-r7
	inPtr .req r0
	outPtr .req r1
	width .req r2
	height .req r3
	step .req r4
	pad .req r5
	vthzKernPtr .req r6
	kernSize .req r7

    @@ Local variables @@
    .equ mem		, 0
    i .req r8
    row .req r9
    inPtrPlusI .req r10
    coeff .req r11

    vecSum0 .req q0
    vecSum0x .req q0x
    vecSum0y .req q0y
    vecSum1 .req q1
    vecSum1x .req q1x
    vecSum1y .req q1y
    vecCoeff .req q2
    vecCoeffx .req q2x
    vec0 .req q3
    vec0x .req q3x
    vec0y .req q3y
    vec1 .req q4
    vec2 .req q5
    vec2x .req q5x
    vec2y .req q5y
    vec3 .req q6
    vec3x .req q6x
    vec3y .req q6y
    vec4 .req q7
    vec4x .req q7x
    vec4y .req q7y
    vec5 .req q8
    vec5x .req q8x
    vec5y .req q8y

    @ Change step in bytes unit (for inPtrPlus)
    lsl step, step, #COMPV_GAS_UINT8_SHIFT_BYTES

    @ Change kernSize in bytes (for vthzKernPtr)
    lsl kernSize, kernSize, #COMPV_GAS_UINT16_SHIFT_BYTES
	
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (j = 0; j < height; ++j)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    LoopHeight_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON32:
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (i = 0; i < width16; i += 16)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        mov i, width
        LoopWidth_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON32:
            veor.u16 vecSum0, vecSum0, vecSum0
            veor.u16 vecSum1, vecSum1, vecSum1
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            @ for (row = 0, k = 0; row < kernSize; ++row, k += step)
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            mov inPtrPlusI, inPtr
            mov row, #0
            LoopKernel_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON32:
                vld1.u8 { vec0 }, [inPtrPlusI], step
                ldrh coeff, [vthzKernPtr, row]
                add row, row, #COMPV_GAS_UINT16_SZ_BYTES
                vdup.u16 vecCoeffx, coeff
                vmovl.u8 vec2, vec0x
                vmovl.u8 vec3, vec0y
                vmull.u16 vec0, vec2x, vecCoeffx[0]
                vmull.u16 vec1, vec2y, vecCoeffx[0]
                vmull.u16 vec2, vec3x, vecCoeffx[0]
                vmull.u16 vec3, vec3y, vecCoeffx[0]
                vshrn.u32 vec4x, vec0, #16
                vshrn.u32 vec4y, vec1, #16
                vshrn.u32 vec5x, vec2, #16
                vshrn.u32 vec5y, vec3, #16
                vqadd.u16 vecSum0, vecSum0, vec4
                vqadd.u16 vecSum1, vecSum1, vec5 
                cmp row, kernSize
                blt LoopKernel_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON32
            EndOf_LoopKernel_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON32:
            
            vqmovn.u16 vec4x, vecSum0
            vqmovn.u16 vec4y, vecSum1
            subs i, i, #16
            bmi MoreThanWidth16_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON32

            @@ if (i < width16) @@
			LessThanWidth16_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON32:
				vst1.u8 {vec4}, [outPtr]!
                add inPtr, inPtr, #(16 << COMPV_GAS_UINT8_SHIFT_BYTES)
				b EndOf_MoreThanWidth16_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON32
			EndOf_LessThanWidth16_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON32:

			@@ if (i >= width16) @@
			MoreThanWidth16_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON32:
                add coeff, sp, #mem @ coeff now contains mem address
				vst1.u8 {vec4}, [coeff :128]
				@@ for (; i < width; ++i, ++k) @@
                add i, i, #16 @ was negative and now contains '(width - (width & -16))'
                add inPtr, inPtr, i, LSL #COMPV_GAS_UINT8_SHIFT_BYTES
				LoopMoreThanWidth16_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON32:
                    ldrb row, [coeff], #(1*COMPV_GAS_UINT8_SZ_BYTES) @ read uint8_t into row and increment
                    strb row, [outPtr], #(1*COMPV_GAS_UINT8_SZ_BYTES) @ write uint8_t from row and increment
					subs i, i, #1
					bne LoopMoreThanWidth16_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON32
				EndOf_LoopMoreThanWidth16_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON32:
                b EndOf_LoopWidth_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON32
			EndOf_MoreThanWidth16_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON32:
            
            bgt LoopWidth_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON32 @ branch for far above 'subs i, i, #16' instruction
        EndOf_LoopWidth_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON32:

        subs height, height, #1
        add inPtr, inPtr, pad @ ,LSL #COMPV_GAS_UINT8_SHIFT_BYTES omitted
        add outPtr, outPtr, pad @ ,LSL #COMPV_GAS_UINT8_SHIFT_BYTES omitted
        bne LoopHeight_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON32
    EndOf_LoopHeight_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON32:

    .unreq inPtr
	.unreq outPtr
	.unreq width
	.unreq height
	.unreq step
	.unreq pad
	.unreq vthzKernPtr

    .unreq i
    .unreq row
    .unreq inPtrPlusI
    .unreq coeff

    .unreq vecSum0
    .unreq vecSum0x
    .unreq vecSum0y
    .unreq vecSum1
    .unreq vecSum1x
    .unreq vecSum1y
    .unreq vecCoeff
    .unreq vecCoeffx
    .unreq vec0
    .unreq vec0x
    .unreq vec0y
    .unreq vec1
    .unreq vec2
    .unreq vec2x
    .unreq vec2y
    .unreq vec3
    .unreq vec3x
    .unreq vec3y
    .unreq vec4
    .unreq vec4x
    .unreq vec4y
    .unreq vec5
    .unreq vec5x
    .unreq vec5y

    COMPV_GAS_MEMFREE (16*COMPV_GAS_UINT8_SZ_BYTES)
	COMPV_GAS_UNALIGN_STACK r11
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 8
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> const uint8_t* inPtr
@ arg(1) -> uint8_t* outPtr
@ arg(2) -> compv_uscalar_t width
@ arg(3) -> compv_uscalar_t height
@ arg(4) -> compv_uscalar_t step
@ arg(5) -> compv_uscalar_t pad
@ arg(6) -> const compv_float32_t* vthzKernPtr
@ arg(7) -> compv_uscalar_t kernSize
.macro CompVMathConvlt1VtHz_8u32f8u_Macro_NEON32 fusedMultiplyAdd
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 8
	COMPV_GAS_SAVE_NEON_REGS
    COMPV_GAS_ALIGN_STACK 16, r11
	COMPV_GAS_MEMALLOC (16*COMPV_GAS_UINT8_SZ_BYTES)

	@@ Load arguments @@
	ldm_args r0-r7
	inPtr .req r0
	outPtr .req r1
	width .req r2
	height .req r3
	step .req r4
	pad .req r5
	vthzKernPtr .req r6
	kernSize .req r7

    @@ Local variables @@
    .equ mem		, 0
    i .req r8
    row .req r9
    inPtrPlusI .req r10
    coeff .req r11

    vecSum0 .req q0
    vecSum0x .req q0x
    vecSum0y .req q0y
    vecSum1 .req q1
    vecSum1x .req q1x
    vecSum1y .req q1y
    vecSum2 .req q2
    vecSum2x .req q2x
    vecSum2y .req q2y
    vecSum3 .req q3
    vecSum3x .req q3x
    vecSum3y .req q3y
    vecCoeff .req q4
    vecCoeffx .req q4x
    vec0 .req q5
    vec0x .req q5x
    vec0y .req q5y
    vec1 .req q6
    vec2 .req q7
    vec2x .req q7x
    vec2y .req q7y
    vec3 .req q8
    vec3x .req q8x
    vec3y .req q8y
    vec4 .req q9
    vec4x .req q9x
    vec4y .req q9y
    vec5 .req q10
    vec5x .req q10x
    vec5y .req q10y

    @ Change step in bytes unit (for inPtrPlus)
    lsl step, step, #COMPV_GAS_UINT8_SHIFT_BYTES

    @ Change kernSize in bytes (for vthzKernPtr)
    lsl kernSize, kernSize, #COMPV_GAS_FLOAT32_SHIFT_BYTES
	
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (j = 0; j < height; ++j)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    LoopHeight_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32\@:
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (i = 0; i < width16; i += 16)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        mov i, width
        LoopWidth_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32\@:
            veor.f32 vecSum0, vecSum0, vecSum0
            veor.f32 vecSum1, vecSum1, vecSum1
            veor.f32 vecSum2, vecSum2, vecSum2
            veor.f32 vecSum3, vecSum3, vecSum3
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            @ for (row = 0, k = 0; row < kernSize; ++row, k += step)
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            mov inPtrPlusI, inPtr
            mov row, #0
            LoopKernel_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32\@:
                vld1.u8 { vec0 }, [inPtrPlusI], step
                ldr coeff, [vthzKernPtr, row]
                add row, row, #COMPV_GAS_FLOAT32_SZ_BYTES
                vmov.f32 vecCoeffx[0], coeff
                vmovl.u8 vec2, vec0x
                vmovl.u8 vec3, vec0y
                vmovl.u16 vec0, vec2x
				vmovl.u16 vec1, vec2y
				vmovl.u16 vec2, vec3x
				vmovl.u16 vec3, vec3y
                vcvt.f32.u32 vec0, vec0
                vcvt.f32.u32 vec1, vec1
                vcvt.f32.u32 vec2, vec2
                vcvt.f32.u32 vec3, vec3
                cmp row, kernSize
                .if \fusedMultiplyAdd
                    vfma.f32 vecSum0, vec0, vecCoeffx[0]
                    vfma.f32 vecSum1, vec1, vecCoeffx[0]
                    vfma.f32 vecSum2, vec2, vecCoeffx[0]
                    vfma.f32 vecSum3, vec3, vecCoeffx[0]
                .else
                    vmla.f32 vecSum0, vec0, vecCoeffx[0]
                    vmla.f32 vecSum1, vec1, vecCoeffx[0]
                    vmla.f32 vecSum2, vec2, vecCoeffx[0]
                    vmla.f32 vecSum3, vec3, vecCoeffx[0]
                .endif
                blt LoopKernel_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32\@
            EndOf_LoopKernel_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32\@:
            
            vcvt.u32.f32 vec0, vecSum0
            vcvt.u32.f32 vec1, vecSum1
            vcvt.u32.f32 vec2, vecSum2
            vcvt.u32.f32 vec3, vecSum3
            subs i, i, #16
            vqmovn.u32 vec4x, vec0
            vqmovn.u32 vec4y, vec1
            vqmovn.u32 vec5x, vec2
            vqmovn.u32 vec5y, vec3
            vqmovn.u16 vec0x, vec4
            vqmovn.u16 vec0y, vec5
            bmi MoreThanWidth16_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32\@

            @@ if (i < width16) @@
			LessThanWidth16_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32\@:
				vst1.u8 {vec0}, [outPtr]!
                add inPtr, inPtr, #(16 << COMPV_GAS_UINT8_SHIFT_BYTES)
				b EndOf_MoreThanWidth16_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32\@
			EndOf_LessThanWidth16_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32\@:

			@@ if (i >= width16) @@
			MoreThanWidth16_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32\@:
                add coeff, sp, #mem @ coeff now contains mem address
				vst1.u8 {vec0}, [coeff :128]
				@@ for (; i < width; ++i, ++k) @@
                add i, i, #16 @ was negative and now contains '(width - (width & -16))'
                add inPtr, inPtr, i, LSL #COMPV_GAS_UINT8_SHIFT_BYTES
				LoopMoreThanWidth16_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32\@:
                    ldrb row, [coeff], #(1*COMPV_GAS_UINT8_SZ_BYTES) @ read uint8_t into row and increment
                    strb row, [outPtr], #(1*COMPV_GAS_UINT8_SZ_BYTES) @ write uint8_t from row and increment
					subs i, i, #1
					bne LoopMoreThanWidth16_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32\@
				EndOf_LoopMoreThanWidth16_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32\@:
                b EndOf_LoopWidth_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32\@
			EndOf_MoreThanWidth16_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32\@:
            
            bgt LoopWidth_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32\@ @ branch for far above 'subs i, i, #16' instruction
        EndOf_LoopWidth_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32\@:

        subs height, height, #1
        add inPtr, inPtr, pad @ ,LSL #COMPV_GAS_UINT8_SHIFT_BYTES omitted
        add outPtr, outPtr, pad @ ,LSL #COMPV_GAS_UINT8_SHIFT_BYTES omitted
        bne LoopHeight_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32\@
    EndOf_LoopHeight_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32\@:

    .unreq inPtr
	.unreq outPtr
	.unreq width
	.unreq height
	.unreq step
	.unreq pad
	.unreq vthzKernPtr

    .unreq i
    .unreq row
    .unreq inPtrPlusI
    .unreq coeff

    .unreq vecSum0
    .unreq vecSum0x
    .unreq vecSum0y
    .unreq vecSum1
    .unreq vecSum1x
    .unreq vecSum1y
    .unreq vecSum2
    .unreq vecSum2x
    .unreq vecSum2y
    .unreq vecSum3
    .unreq vecSum3x
    .unreq vecSum3y
    .unreq vecCoeff
    .unreq vecCoeffx
    .unreq vec0
    .unreq vec0x
    .unreq vec0y
    .unreq vec1
    .unreq vec2
    .unreq vec2x
    .unreq vec2y
    .unreq vec3
    .unreq vec3x
    .unreq vec3y
    .unreq vec4
    .unreq vec4x
    .unreq vec4y
    .unreq vec5
    .unreq vec5x
    .unreq vec5y

    COMPV_GAS_MEMFREE (16*COMPV_GAS_UINT8_SZ_BYTES)
	COMPV_GAS_UNALIGN_STACK r11
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 8
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32
    CompVMathConvlt1VtHz_8u32f8u_Macro_NEON32 0

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVMathConvlt1VtHz_8u32f8u_Asm_FMA_NEON32
    CompVMathConvlt1VtHz_8u32f8u_Macro_NEON32 0

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> const uint8_t* inPtr
@ arg(1) -> compv_float32_t* outPtr
@ arg(2) -> compv_uscalar_t width
@ arg(3) -> compv_uscalar_t height
@ arg(4) -> compv_uscalar_t step
@ arg(5) -> compv_uscalar_t pad
@ arg(6) -> const compv_float32_t* vthzKernPtr
@ arg(7) -> compv_uscalar_t kernSize
.macro CompVMathConvlt1VtHz_8u32f32f_Macro_NEON32 fusedMultiplyAdd
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 8
	COMPV_GAS_SAVE_NEON_REGS
    COMPV_GAS_ALIGN_STACK 16, r11
	COMPV_GAS_MEMALLOC (16*COMPV_GAS_FLOAT32_SZ_BYTES)

	@@ Load arguments @@
	ldm_args r0-r7
	inPtr .req r0
	outPtr .req r1
	width .req r2
	height .req r3
	step .req r4
	pad .req r5
	vthzKernPtr .req r6
	kernSize .req r7

    @@ Local variables @@
    .equ mem		, 0
    i .req r8
    row .req r9
    inPtrPlusI .req r10
    coeff .req r11

    vecSum0 .req q0
    vecSum0x .req q0x
    vecSum0y .req q0y
    vecSum1 .req q1
    vecSum1x .req q1x
    vecSum1y .req q1y
    vecSum2 .req q2
    vecSum2x .req q2x
    vecSum2y .req q2y
    vecSum3 .req q3
    vecSum3x .req q3x
    vecSum3y .req q3y
    vecCoeff .req q4
    vecCoeffx .req q4x
    vec0 .req q5
    vec0x .req q5x
    vec0y .req q5y
    vec1 .req q6
    vec2 .req q7
    vec2x .req q7x
    vec2y .req q7y
    vec3 .req q8
    vec3x .req q8x
    vec3y .req q8y
    vec4 .req q9
    vec4x .req q9x
    vec4y .req q9y
    vec5 .req q10
    vec5x .req q10x
    vec5y .req q10y

    @ Change step in bytes unit (for inPtrPlus)
    lsl step, step, #COMPV_GAS_UINT8_SHIFT_BYTES

    @ Change kernSize in bytes (for vthzKernPtr)
    lsl kernSize, kernSize, #COMPV_GAS_FLOAT32_SHIFT_BYTES
	
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (j = 0; j < height; ++j)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    LoopHeight_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON32\@:
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (i = 0; i < width16; i += 16)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        mov i, width
        LoopWidth_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON32\@:
            veor.f32 vecSum0, vecSum0, vecSum0
            veor.f32 vecSum1, vecSum1, vecSum1
            veor.f32 vecSum2, vecSum2, vecSum2
            veor.f32 vecSum3, vecSum3, vecSum3
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            @ for (row = 0, k = 0; row < kernSize; ++row, k += step)
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            mov inPtrPlusI, inPtr
            mov row, #0
            LoopKernel_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON32\@:
                vld1.u8 { vec0 }, [inPtrPlusI], step
                ldr coeff, [vthzKernPtr, row]
                add row, row, #COMPV_GAS_FLOAT32_SZ_BYTES
                .if \fusedMultiplyAdd
                    vdup.f32 vecCoeff, coeff
                .else
                    vmov.f32 vecCoeffx[0], coeff
                .endif
                vmovl.u8 vec2, vec0x
                vmovl.u8 vec3, vec0y
                vmovl.u16 vec0, vec2x
				vmovl.u16 vec1, vec2y
				vmovl.u16 vec2, vec3x
				vmovl.u16 vec3, vec3y
                vcvt.f32.u32 vec0, vec0
                vcvt.f32.u32 vec1, vec1
                vcvt.f32.u32 vec2, vec2
                vcvt.f32.u32 vec3, vec3
                cmp row, kernSize
                .if \fusedMultiplyAdd
                    vfma.f32 vecSum0, vec0, vecCoeff
                    vfma.f32 vecSum1, vec1, vecCoeff
                    vfma.f32 vecSum2, vec2, vecCoeff
                    vfma.f32 vecSum3, vec3, vecCoeff
                .else
                    vmla.f32 vecSum0, vec0, vecCoeffx[0]
                    vmla.f32 vecSum1, vec1, vecCoeffx[0]
                    vmla.f32 vecSum2, vec2, vecCoeffx[0]
                    vmla.f32 vecSum3, vec3, vecCoeffx[0]
                .endif
                blt LoopKernel_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON32\@
            EndOf_LoopKernel_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON32\@:
            
            subs i, i, #16
            bmi MoreThanWidth16_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON32\@

            @@ if (i < width16) @@
			LessThanWidth16_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON32\@:
				vst1.f32 {vecSum0, vecSum1}, [outPtr]!
                vst1.f32 {vecSum2, vecSum3}, [outPtr]!
                add inPtr, inPtr, #(16 << COMPV_GAS_UINT8_SHIFT_BYTES)
				b EndOf_MoreThanWidth16_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON32\@
			EndOf_LessThanWidth16_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON32\@:

			@@ if (i >= width16) @@
			MoreThanWidth16_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON32\@:
                add coeff, sp, #mem @ coeff now contains mem address
                add inPtrPlusI, coeff, #(8*COMPV_GAS_FLOAT32_SZ_BYTES)
                vst1.f32 {vecSum0, vecSum1}, [coeff :128]
                vst1.f32 {vecSum2, vecSum3}, [inPtrPlusI :128]
				@@ for (; i < width; ++i, ++k) @@
                add i, i, #16 @ was negative and now contains '(width - (width & -16))'
                add inPtr, inPtr, i, LSL #COMPV_GAS_UINT8_SHIFT_BYTES
				LoopMoreThanWidth16_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON32\@:
                    ldr row, [coeff], #(1*COMPV_GAS_FLOAT32_SZ_BYTES) @ read float32_t into row and increment
                    str row, [outPtr], #(1*COMPV_GAS_FLOAT32_SZ_BYTES) @ write float32_t from row and increment
					subs i, i, #1
					bne LoopMoreThanWidth16_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON32\@
				EndOf_LoopMoreThanWidth16_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON32\@:
                b EndOf_LoopWidth_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON32\@
			EndOf_MoreThanWidth16_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON32\@:
            
            bgt LoopWidth_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON32\@ @ branch for far above 'subs i, i, #16' instruction
        EndOf_LoopWidth_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON32\@:

        subs height, height, #1
        add inPtr, inPtr, pad @, LSL #COMPV_GAS_UINT8_SHIFT_BYTES omitted
        add outPtr, outPtr, pad, LSL #COMPV_GAS_FLOAT32_SHIFT_BYTES
        bne LoopHeight_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON32\@
    EndOf_LoopHeight_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON32\@:

    .unreq inPtr
	.unreq outPtr
	.unreq width
	.unreq height
	.unreq step
	.unreq pad
	.unreq vthzKernPtr

    .unreq i
    .unreq row
    .unreq inPtrPlusI
    .unreq coeff

    .unreq vecSum0
    .unreq vecSum0x
    .unreq vecSum0y
    .unreq vecSum1
    .unreq vecSum1x
    .unreq vecSum1y
    .unreq vecSum2
    .unreq vecSum2x
    .unreq vecSum2y
    .unreq vecSum3
    .unreq vecSum3x
    .unreq vecSum3y
    .unreq vecCoeff
    .unreq vecCoeffx
    .unreq vec0
    .unreq vec0x
    .unreq vec0y
    .unreq vec1
    .unreq vec2
    .unreq vec2x
    .unreq vec2y
    .unreq vec3
    .unreq vec3x
    .unreq vec3y
    .unreq vec4
    .unreq vec4x
    .unreq vec4y
    .unreq vec5
    .unreq vec5x
    .unreq vec5y

    COMPV_GAS_MEMFREE (16*COMPV_GAS_FLOAT32_SZ_BYTES)
	COMPV_GAS_UNALIGN_STACK r11
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 8
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVMathConvlt1VtHz_8u32f32f_Asm_NEON32
    CompVMathConvlt1VtHz_8u32f32f_Macro_NEON32 0

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVMathConvlt1VtHz_8u32f32f_Asm_FMA_NEON32
    CompVMathConvlt1VtHz_8u32f32f_Macro_NEON32 1



@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> const compv_float32_t* inPtr
@ arg(1) -> compv_float32_t* outPtr
@ arg(2) -> compv_uscalar_t width
@ arg(3) -> compv_uscalar_t height
@ arg(4) -> compv_uscalar_t step
@ arg(5) -> compv_uscalar_t pad
@ arg(6) -> const compv_float32_t* vthzKernPtr
@ arg(7) -> compv_uscalar_t kernSize
.macro CompVMathConvlt1VtHz_32f32f32f_Macro_NEON32 fusedMultiplyAdd
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 8
	COMPV_GAS_SAVE_NEON_REGS
    COMPV_GAS_ALIGN_STACK 16, r11
	COMPV_GAS_MEMALLOC (16*COMPV_GAS_FLOAT32_SZ_BYTES)

	@@ Load arguments @@
	ldm_args r0-r7
	inPtr .req r0
	outPtr .req r1
	width .req r2
	height .req r3
	step .req r4
	pad .req r5
	vthzKernPtr .req r6
	kernSize .req r7

    @@ Local variables @@
    .equ mem		, 0
    i .req r8
    row .req r9
    inPtrPlusI .req r10
    coeff .req r11

    vecSum0 .req q0
    vecSum0x .req q0x
    vecSum0y .req q0y
    vecSum1 .req q1
    vecSum1x .req q1x
    vecSum1y .req q1y
    vecSum2 .req q2
    vecSum2x .req q2x
    vecSum2y .req q2y
    vecSum3 .req q3
    vecSum3x .req q3x
    vecSum3y .req q3y
    vecCoeff .req q4
    vecCoeffx .req q4x
    vec0 .req q5
    vec0x .req q5x
    vec0y .req q5y
    vec1 .req q6
    vec2 .req q7
    vec2x .req q7x
    vec2y .req q7y
    vec3 .req q8
    vec3x .req q8x
    vec3y .req q8y
    vec4 .req q9
    vec4x .req q9x
    vec4y .req q9y
    vec5 .req q10
    vec5x .req q10x
    vec5y .req q10y

    @ Change step in bytes unit (for inPtrPlus)
    lsl step, step, #COMPV_GAS_FLOAT32_SHIFT_BYTES
    sub step, step, #(2*COMPV_GAS_Q_SZ_BYTES) @ hack to remove vec0 and vec1 -> needed because we cannot read #4 quad registers (not needed for ARM64)

    @ Change kernSize in bytes (for vthzKernPtr)
    lsl kernSize, kernSize, #COMPV_GAS_FLOAT32_SHIFT_BYTES

    @ Change pad in bytes (for inPtr and outPtr)
    lsl pad, pad, #COMPV_GAS_FLOAT32_SHIFT_BYTES
	
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (j = 0; j < height; ++j)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    LoopHeight_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON32\@:
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (i = 0; i < width16; i += 16)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        mov i, width
        LoopWidth_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON32\@:
            veor.f32 vecSum0, vecSum0, vecSum0
            veor.f32 vecSum1, vecSum1, vecSum1
            veor.f32 vecSum2, vecSum2, vecSum2
            veor.f32 vecSum3, vecSum3, vecSum3
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            @ for (row = 0, k = 0; row < kernSize; ++row, k += step)
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            mov inPtrPlusI, inPtr
            mov row, #0
            LoopKernel_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON32\@:
                vld1.f32 { vec0,vec1 }, [inPtrPlusI]!
                vld1.f32 { vec2,vec3 }, [inPtrPlusI], step @ step is hacked above
                ldr coeff, [vthzKernPtr, row]
                add row, row, #COMPV_GAS_FLOAT32_SZ_BYTES
                .if \fusedMultiplyAdd
                    vdup.f32 vecCoeff, coeff
                .else
                    vmov.f32 vecCoeffx[0], coeff
                .endif
                cmp row, kernSize
                .if \fusedMultiplyAdd
                    vfma.f32 vecSum0, vec0, vecCoeff
                    vfma.f32 vecSum1, vec1, vecCoeff
                    vfma.f32 vecSum2, vec2, vecCoeff
                    vfma.f32 vecSum3, vec3, vecCoeff
                .else
                    vmla.f32 vecSum0, vec0, vecCoeffx[0]
                    vmla.f32 vecSum1, vec1, vecCoeffx[0]
                    vmla.f32 vecSum2, vec2, vecCoeffx[0]
                    vmla.f32 vecSum3, vec3, vecCoeffx[0]
                .endif
                blt LoopKernel_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON32\@
            EndOf_LoopKernel_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON32\@:
            
            subs i, i, #16
            bmi MoreThanWidth16_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON32\@

            @@ if (i < width16) @@
			LessThanWidth16_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON32\@:
				vst1.f32 {vecSum0, vecSum1}, [outPtr]!
                vst1.f32 {vecSum2, vecSum3}, [outPtr]!
                add inPtr, inPtr, #(16 << COMPV_GAS_FLOAT32_SHIFT_BYTES)
				b EndOf_MoreThanWidth16_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON32\@
			EndOf_LessThanWidth16_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON32\@:

			@@ if (i >= width16) @@
			MoreThanWidth16_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON32\@:
                add coeff, sp, #mem @ coeff now contains mem address
                add inPtrPlusI, coeff, #(8*COMPV_GAS_FLOAT32_SZ_BYTES)
                vst1.f32 {vecSum0, vecSum1}, [coeff :128]
                vst1.f32 {vecSum2, vecSum3}, [inPtrPlusI :128]
				@@ for (; i < width; ++i, ++k) @@
                add i, i, #16 @ was negative and now contains '(width - (width & -16))'
                add inPtr, inPtr, i, LSL #COMPV_GAS_FLOAT32_SHIFT_BYTES
				LoopMoreThanWidth16_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON32\@:
                    ldr row, [coeff], #(1*COMPV_GAS_FLOAT32_SZ_BYTES) @ read float32_t into row and increment
                    str row, [outPtr], #(1*COMPV_GAS_FLOAT32_SZ_BYTES) @ write float32_t from row and increment
					subs i, i, #1
					bne LoopMoreThanWidth16_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON32\@
				EndOf_LoopMoreThanWidth16_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON32\@:
                b EndOf_LoopWidth_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON32\@
			EndOf_MoreThanWidth16_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON32\@:
            
            bgt LoopWidth_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON32\@ @ branch for far above 'subs i, i, #16' instruction
        EndOf_LoopWidth_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON32\@:

        subs height, height, #1
        add inPtr, inPtr, pad @, LSL #COMPV_GAS_FLOAT32_SHIFT_BYTES omitted
        add outPtr, outPtr, pad @, LSL #COMPV_GAS_FLOAT32_SHIFT_BYTES omitted
        bne LoopHeight_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON32\@
    EndOf_LoopHeight_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON32\@:

    .unreq inPtr
	.unreq outPtr
	.unreq width
	.unreq height
	.unreq step
	.unreq pad
	.unreq vthzKernPtr

    .unreq i
    .unreq row
    .unreq inPtrPlusI
    .unreq coeff

    .unreq vecSum0
    .unreq vecSum0x
    .unreq vecSum0y
    .unreq vecSum1
    .unreq vecSum1x
    .unreq vecSum1y
    .unreq vecSum2
    .unreq vecSum2x
    .unreq vecSum2y
    .unreq vecSum3
    .unreq vecSum3x
    .unreq vecSum3y
    .unreq vecCoeff
    .unreq vecCoeffx
    .unreq vec0
    .unreq vec0x
    .unreq vec0y
    .unreq vec1
    .unreq vec2
    .unreq vec2x
    .unreq vec2y
    .unreq vec3
    .unreq vec3x
    .unreq vec3y
    .unreq vec4
    .unreq vec4x
    .unreq vec4y
    .unreq vec5
    .unreq vec5x
    .unreq vec5y

    COMPV_GAS_MEMFREE (16*COMPV_GAS_FLOAT32_SZ_BYTES)
	COMPV_GAS_UNALIGN_STACK r11
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 8
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVMathConvlt1VtHz_32f32f32f_Asm_NEON32
    CompVMathConvlt1VtHz_32f32f32f_Macro_NEON32 0

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVMathConvlt1VtHz_32f32f32f_Asm_FMA_NEON32
    CompVMathConvlt1VtHz_32f32f32f_Macro_NEON32 1


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> const compv_float32_t* inPtr
@ arg(1) -> uint8_t* outPtr
@ arg(2) -> compv_uscalar_t width
@ arg(3) -> compv_uscalar_t height
@ arg(4) -> compv_uscalar_t step
@ arg(5) -> compv_uscalar_t pad
@ arg(6) -> const compv_float32_t* vthzKernPtr
@ arg(7) -> compv_uscalar_t kernSize
.macro CompVMathConvlt1VtHz_32f32f8u_Macro_NEON32 fusedMultiplyAdd
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 8
	COMPV_GAS_SAVE_NEON_REGS
    COMPV_GAS_ALIGN_STACK 16, r11
	COMPV_GAS_MEMALLOC (16*COMPV_GAS_FLOAT32_SZ_BYTES)

	@@ Load arguments @@
	ldm_args r0-r7
	inPtr .req r0
	outPtr .req r1
	width .req r2
	height .req r3
	step .req r4
	pad .req r5
	vthzKernPtr .req r6
	kernSize .req r7

    @@ Local variables @@
    .equ mem		, 0
    i .req r8
    row .req r9
    inPtrPlusI .req r10
    coeff .req r11

    vecSum0 .req q0
    vecSum0x .req q0x
    vecSum0y .req q0y
    vecSum1 .req q1
    vecSum1x .req q1x
    vecSum1y .req q1y
    vecSum2 .req q2
    vecSum2x .req q2x
    vecSum2y .req q2y
    vecSum3 .req q3
    vecSum3x .req q3x
    vecSum3y .req q3y
    vecCoeff .req q4
    vecCoeffx .req q4x
    vec0 .req q5
    vec0x .req q5x
    vec0y .req q5y
    vec1 .req q6
    vec2 .req q7
    vec2x .req q7x
    vec2y .req q7y
    vec3 .req q8
    vec3x .req q8x
    vec3y .req q8y
    vec4 .req q9
    vec4x .req q9x
    vec4y .req q9y
    vec5 .req q10
    vec5x .req q10x
    vec5y .req q10y

    @ Change step in bytes unit (for inPtrPlus)
    lsl step, step, #COMPV_GAS_FLOAT32_SHIFT_BYTES
    sub step, step, #(2*COMPV_GAS_Q_SZ_BYTES) @ hack to remove vec0 and vec1 -> needed because we cannot read #4 quad registers (not needed for ARM64)

    @ Change kernSize in bytes (for vthzKernPtr)
    lsl kernSize, kernSize, #COMPV_GAS_FLOAT32_SHIFT_BYTES
	
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (j = 0; j < height; ++j)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    LoopHeight_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON32\@:
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (i = 0; i < width16; i += 16)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        mov i, width
        LoopWidth_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON32\@:
            veor.f32 vecSum0, vecSum0, vecSum0
            veor.f32 vecSum1, vecSum1, vecSum1
            veor.f32 vecSum2, vecSum2, vecSum2
            veor.f32 vecSum3, vecSum3, vecSum3
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            @ for (row = 0, k = 0; row < kernSize; ++row, k += step)
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            mov inPtrPlusI, inPtr
            mov row, #0
            LoopKernel_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON32\@:
                vld1.f32 { vec0,vec1 }, [inPtrPlusI]!
                vld1.f32 { vec2,vec3 }, [inPtrPlusI], step @ step is hacked above
                ldr coeff, [vthzKernPtr, row]
                add row, row, #COMPV_GAS_FLOAT32_SZ_BYTES
                .if \fusedMultiplyAdd
                    vdup.f32 vecCoeff, coeff
                .else
                    vmov.f32 vecCoeffx[0], coeff
                .endif
                cmp row, kernSize
                .if \fusedMultiplyAdd
                    vfma.f32 vecSum0, vec0, vecCoeff
                    vfma.f32 vecSum1, vec1, vecCoeff
                    vfma.f32 vecSum2, vec2, vecCoeff
                    vfma.f32 vecSum3, vec3, vecCoeff
                .else
                    vmla.f32 vecSum0, vec0, vecCoeffx[0]
                    vmla.f32 vecSum1, vec1, vecCoeffx[0]
                    vmla.f32 vecSum2, vec2, vecCoeffx[0]
                    vmla.f32 vecSum3, vec3, vecCoeffx[0]
                .endif
                blt LoopKernel_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON32\@
            EndOf_LoopKernel_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON32\@:
            
            vcvt.u32.f32 vec0, vecSum0
            vcvt.u32.f32 vec1, vecSum1
            vcvt.u32.f32 vec2, vecSum2
            vcvt.u32.f32 vec3, vecSum3
            subs i, i, #16
            vqmovn.u32 vec4x, vec0
            vqmovn.u32 vec4y, vec1
            vqmovn.u32 vec5x, vec2
            vqmovn.u32 vec5y, vec3
            vqmovn.u16 vec0x, vec4
            vqmovn.u16 vec0y, vec5
            bmi MoreThanWidth16_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON32\@

            @@ if (i < width16) @@
			LessThanWidth16_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON32\@:
				vst1.u8 {vec0}, [outPtr]!
                add inPtr, inPtr, #(16 << COMPV_GAS_FLOAT32_SHIFT_BYTES)
				b EndOf_MoreThanWidth16_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON32\@
			EndOf_LessThanWidth16_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON32\@:

			@@ if (i >= width16) @@
			MoreThanWidth16_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON32\@:
                add coeff, sp, #mem @ coeff now contains mem address
                vst1.u8 {vec0}, [coeff :128]
				@@ for (; i < width; ++i, ++k) @@
                add i, i, #16 @ was negative and now contains '(width - (width & -16))'
                add inPtr, inPtr, i, LSL #COMPV_GAS_FLOAT32_SHIFT_BYTES
				LoopMoreThanWidth16_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON32\@:
                    ldrb row, [coeff], #(1*COMPV_GAS_UINT8_SZ_BYTES) @ read uint8_t into row and increment
                    strb row, [outPtr], #(1*COMPV_GAS_UINT8_SZ_BYTES) @ write uint8_t from row and increment
					subs i, i, #1
					bne LoopMoreThanWidth16_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON32\@
				EndOf_LoopMoreThanWidth16_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON32\@:
                b EndOf_LoopWidth_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON32\@
			EndOf_MoreThanWidth16_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON32\@:
            
            bgt LoopWidth_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON32\@ @ branch for far above 'subs i, i, #16' instruction
        EndOf_LoopWidth_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON32\@:

        subs height, height, #1
        add inPtr, inPtr, pad, LSL #COMPV_GAS_FLOAT32_SHIFT_BYTES
        add outPtr, outPtr, pad @, LSL #COMPV_GAS_UINT8_SHIFT_BYTES omitted
        bne LoopHeight_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON32\@
    EndOf_LoopHeight_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON32\@:

    .unreq inPtr
	.unreq outPtr
	.unreq width
	.unreq height
	.unreq step
	.unreq pad
	.unreq vthzKernPtr

    .unreq i
    .unreq row
    .unreq inPtrPlusI
    .unreq coeff

    .unreq vecSum0
    .unreq vecSum0x
    .unreq vecSum0y
    .unreq vecSum1
    .unreq vecSum1x
    .unreq vecSum1y
    .unreq vecSum2
    .unreq vecSum2x
    .unreq vecSum2y
    .unreq vecSum3
    .unreq vecSum3x
    .unreq vecSum3y
    .unreq vecCoeff
    .unreq vecCoeffx
    .unreq vec0
    .unreq vec0x
    .unreq vec0y
    .unreq vec1
    .unreq vec2
    .unreq vec2x
    .unreq vec2y
    .unreq vec3
    .unreq vec3x
    .unreq vec3y
    .unreq vec4
    .unreq vec4x
    .unreq vec4y
    .unreq vec5
    .unreq vec5x
    .unreq vec5y

    COMPV_GAS_MEMFREE (16*COMPV_GAS_FLOAT32_SZ_BYTES)
	COMPV_GAS_UNALIGN_STACK r11
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 8
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVMathConvlt1VtHz_32f32f8u_Asm_NEON32
    CompVMathConvlt1VtHz_32f32f8u_Macro_NEON32 0
    
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
COMPV_GAS_FUNCTION_DECLARE CompVMathConvlt1VtHz_32f32f8u_Asm_FMA_NEON32
    CompVMathConvlt1VtHz_32f32f8u_Macro_NEON32 1

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> const uint8_t* inPtr
@ arg(1) -> int16_t* outPtr
@ arg(2) -> compv_uscalar_t width
@ arg(3) -> compv_uscalar_t height
@ arg(4) -> compv_uscalar_t step
@ arg(5) -> compv_uscalar_t pad
@ arg(6) -> const int16_t* vthzKernPtr
@ arg(7) -> compv_uscalar_t kernSize
COMPV_GAS_FUNCTION_DECLARE CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 8
	COMPV_GAS_SAVE_NEON_REGS
    COMPV_GAS_ALIGN_STACK 16, r11
	COMPV_GAS_MEMALLOC (16*COMPV_GAS_INT16_SZ_BYTES)

	@@ Load arguments @@
	ldm_args r0-r7
	inPtr .req r0
	outPtr .req r1
	width .req r2
	height .req r3
	step .req r4
	pad .req r5
	vthzKernPtr .req r6
	kernSize .req r7

    @@ Local variables @@
    .equ mem		, 0
    i .req r8
    row .req r9
    inPtrPlusI .req r10
    coeff .req r11

    vecSum0 .req q0
    vecSum0x .req q0x
    vecSum0y .req q0y
    vecSum1 .req q1
    vecSum1x .req q1x
    vecSum1y .req q1y
    vecSum2 .req q2
    vecSum2x .req q2x
    vecSum2y .req q2y
    vecSum3 .req q3
    vecSum3x .req q3x
    vecSum3y .req q3y
    vecCoeff .req q4
    vecCoeffx .req q4x
    vec0 .req q5
    vec0x .req q5x
    vec0y .req q5y
    vec1 .req q6
    vec1x .req q6x
    vec1y .req q6y
    vec2 .req q7
    vec2x .req q7x
    vec2y .req q7y

    @ Change step in bytes unit (for inPtrPlus)
    lsl step, step, #COMPV_GAS_UINT8_SHIFT_BYTES

    @ Change kernSize in bytes (for vthzKernPtr)
    lsl kernSize, kernSize, #COMPV_GAS_INT16_SHIFT_BYTES
	
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (j = 0; j < height; ++j)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    LoopHeight_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32:
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (i = 0; i < width16; i += 16)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        mov i, width
        LoopWidth_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32:
            veor.s32 vecSum0, vecSum0, vecSum0
            veor.s32 vecSum1, vecSum1, vecSum1
            veor.s32 vecSum2, vecSum2, vecSum2
            veor.s32 vecSum3, vecSum3, vecSum3
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            @ for (row = 0, k = 0; row < kernSize; ++row, k += step)
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            mov inPtrPlusI, inPtr
            mov row, #0
            LoopKernel_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32:
                vld1.u8 { vec0 }, [inPtrPlusI], step
                ldrh coeff, [vthzKernPtr, row]
                add row, row, #COMPV_GAS_INT16_SZ_BYTES
                vdup.s16 vecCoeffx, coeff
                vmovl.u8 vec1, vec0x
                vmovl.u8 vec2, vec0y
                vmlal.s16 vecSum0, vec1x, vecCoeffx
                vmlal.s16 vecSum1, vec1y, vecCoeffx
                vmlal.s16 vecSum2, vec2x, vecCoeffx
                vmlal.s16 vecSum3, vec2y, vecCoeffx
                cmp row, kernSize
                blt LoopKernel_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32
            EndOf_LoopKernel_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32:
            
            vqmovn.s32 vec0x, vecSum0
            vqmovn.s32 vec0y, vecSum1
            vqmovn.s32 vec1x, vecSum2
            vqmovn.s32 vec1y, vecSum3
            subs i, i, #16
            bmi MoreThanWidth16_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32

            @@ if (i < width16) @@
			LessThanWidth16_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32:
				vst1.u8 {vec0, vec1}, [outPtr]!
                add inPtr, inPtr, #(16 << COMPV_GAS_UINT8_SHIFT_BYTES)
				b EndOf_MoreThanWidth16_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32
			EndOf_LessThanWidth16_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32:

			@@ if (i >= width16) @@
			MoreThanWidth16_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32:
                add coeff, sp, #mem @ coeff now contains mem address
				vst1.u8 {vec0, vec1}, [coeff :128]
				@@ for (; i < width; ++i, ++k) @@
                add i, i, #16 @ was negative and now contains '(width - (width & -16))'
                add inPtr, inPtr, i, LSL #COMPV_GAS_UINT8_SHIFT_BYTES
				LoopMoreThanWidth16_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32:
                    ldrh row, [coeff], #(1*COMPV_GAS_INT16_SZ_BYTES) @ read int16_t into row and increment
                    strh row, [outPtr], #(1*COMPV_GAS_INT16_SZ_BYTES) @ write int16_t from row and increment
					subs i, i, #1
					bne LoopMoreThanWidth16_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32
				EndOf_LoopMoreThanWidth16_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32:
                b EndOf_LoopWidth_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32
			EndOf_MoreThanWidth16_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32:
            
            bgt LoopWidth_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32 @ branch for far above 'subs i, i, #16' instruction
        EndOf_LoopWidth_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32:

        subs height, height, #1
        add inPtr, inPtr, pad @ ,LSL #COMPV_GAS_UINT8_SHIFT_BYTES omitted
        add outPtr, outPtr, pad, LSL #COMPV_GAS_INT16_SHIFT_BYTES
        bne LoopHeight_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32
    EndOf_LoopHeight_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32:

    .unreq inPtr
	.unreq outPtr
	.unreq width
	.unreq height
	.unreq step
	.unreq pad
	.unreq vthzKernPtr

    .unreq i
    .unreq row
    .unreq inPtrPlusI
    .unreq coeff

    .unreq vecSum0
    .unreq vecSum0x
    .unreq vecSum0y
    .unreq vecSum1
    .unreq vecSum1x
    .unreq vecSum1y
    .unreq vecSum2
    .unreq vecSum2x
    .unreq vecSum2y
    .unreq vecSum3
    .unreq vecSum3x
    .unreq vecSum3y
    .unreq vecCoeff
    .unreq vecCoeffx
    .unreq vec0
    .unreq vec0x
    .unreq vec0y
    .unreq vec1
    .unreq vec1x
    .unreq vec1y
    .unreq vec2
    .unreq vec2x
    .unreq vec2y

    COMPV_GAS_MEMFREE (16*COMPV_GAS_INT16_SZ_BYTES)
	COMPV_GAS_UNALIGN_STACK r11
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 8
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> const int16_t* inPtr
@ arg(1) -> int16_t* outPtr
@ arg(2) -> compv_uscalar_t width
@ arg(3) -> compv_uscalar_t height
@ arg(4) -> compv_uscalar_t step
@ arg(5) -> compv_uscalar_t pad
@ arg(6) -> const int16_t* vthzKernPtr
@ arg(7) -> compv_uscalar_t kernSize
COMPV_GAS_FUNCTION_DECLARE CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 8
	COMPV_GAS_SAVE_NEON_REGS
    COMPV_GAS_ALIGN_STACK 16, r11
	COMPV_GAS_MEMALLOC (16*COMPV_GAS_INT16_SZ_BYTES)

	@@ Load arguments @@
	ldm_args r0-r7
	inPtr .req r0
	outPtr .req r1
	width .req r2
	height .req r3
	step .req r4
	pad .req r5
	vthzKernPtr .req r6
	kernSize .req r7

    @@ Local variables @@
    .equ mem		, 0
    i .req r8
    row .req r9
    inPtrPlusI .req r10
    coeff .req r11

    vecSum0 .req q0
    vecSum0x .req q0x
    vecSum0y .req q0y
    vecSum1 .req q1
    vecSum1x .req q1x
    vecSum1y .req q1y
    vecSum2 .req q2
    vecSum2x .req q2x
    vecSum2y .req q2y
    vecSum3 .req q3
    vecSum3x .req q3x
    vecSum3y .req q3y
    vecCoeff .req q4
    vecCoeffx .req q4x
    vec0 .req q5
    vec0x .req q5x
    vec0y .req q5y
    vec1 .req q6
    vec1x .req q6x
    vec1y .req q6y

    @ Change step in bytes unit (for inPtrPlus)
    lsl step, step, #COMPV_GAS_INT16_SHIFT_BYTES

    @ Change kernSize in bytes (for vthzKernPtr)
    lsl kernSize, kernSize, #COMPV_GAS_INT16_SHIFT_BYTES

    @ Change pad in bytes (for inPtr and outPtr)
    lsl pad, pad, #COMPV_GAS_INT16_SHIFT_BYTES
	
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (j = 0; j < height; ++j)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    LoopHeight_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32:
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (i = 0; i < width16; i += 16)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        mov i, width
        LoopWidth_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32:
            veor.s32 vecSum0, vecSum0, vecSum0
            veor.s32 vecSum1, vecSum1, vecSum1
            veor.s32 vecSum2, vecSum2, vecSum2
            veor.s32 vecSum3, vecSum3, vecSum3
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            @ for (row = 0, k = 0; row < kernSize; ++row, k += step)
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            mov inPtrPlusI, inPtr
            mov row, #0
            LoopKernel_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32:
                vld1.s16 { vec0, vec1 }, [inPtrPlusI], step
                ldrh coeff, [vthzKernPtr, row]
                add row, row, #COMPV_GAS_INT16_SZ_BYTES
                vdup.s16 vecCoeffx, coeff
                vmlal.s16 vecSum0, vec0x, vecCoeffx
                vmlal.s16 vecSum1, vec0y, vecCoeffx
                vmlal.s16 vecSum2, vec1x, vecCoeffx
                vmlal.s16 vecSum3, vec1y, vecCoeffx
                cmp row, kernSize
                blt LoopKernel_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32
            EndOf_LoopKernel_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32:
            
            vqmovn.s32 vec0x, vecSum0
            vqmovn.s32 vec0y, vecSum1
            vqmovn.s32 vec1x, vecSum2
            vqmovn.s32 vec1y, vecSum3
            subs i, i, #16
            bmi MoreThanWidth16_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32

            @@ if (i < width16) @@
			LessThanWidth16_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32:
				vst1.u8 {vec0, vec1}, [outPtr]!
                add inPtr, inPtr, #(16 << COMPV_GAS_INT16_SHIFT_BYTES)
				b EndOf_MoreThanWidth16_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32
			EndOf_LessThanWidth16_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32:

			@@ if (i >= width16) @@
			MoreThanWidth16_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32:
                add coeff, sp, #mem @ coeff now contains mem address
				vst1.u8 {vec0, vec1}, [coeff :128]
				@@ for (; i < width; ++i, ++k) @@
                add i, i, #16 @ was negative and now contains '(width - (width & -16))'
                add inPtr, inPtr, i, LSL #COMPV_GAS_INT16_SHIFT_BYTES
				LoopMoreThanWidth16_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32:
                    ldrh row, [coeff], #(1*COMPV_GAS_INT16_SZ_BYTES) @ read int16_t into row and increment
                    strh row, [outPtr], #(1*COMPV_GAS_INT16_SZ_BYTES) @ write int16_t from row and increment
					subs i, i, #1
					bne LoopMoreThanWidth16_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32
				EndOf_LoopMoreThanWidth16_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32:
                b EndOf_LoopWidth_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32
			EndOf_MoreThanWidth16_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32:
            
            bgt LoopWidth_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32 @ branch for far above 'subs i, i, #16' instruction
        EndOf_LoopWidth_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32:

        subs height, height, #1
        add inPtr, inPtr, pad @ ,LSL #COMPV_GAS_INT16_SHIFT_BYTES omitted
        add outPtr, outPtr, pad @, LSL #COMPV_GAS_INT16_SHIFT_BYTES
        bne LoopHeight_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32
    EndOf_LoopHeight_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32:

    .unreq inPtr
	.unreq outPtr
	.unreq width
	.unreq height
	.unreq step
	.unreq pad
	.unreq vthzKernPtr

    .unreq i
    .unreq row
    .unreq inPtrPlusI
    .unreq coeff

    .unreq vecSum0
    .unreq vecSum0x
    .unreq vecSum0y
    .unreq vecSum1
    .unreq vecSum1x
    .unreq vecSum1y
    .unreq vecSum2
    .unreq vecSum2x
    .unreq vecSum2y
    .unreq vecSum3
    .unreq vecSum3x
    .unreq vecSum3y
    .unreq vecCoeff
    .unreq vecCoeffx
    .unreq vec0
    .unreq vec0x
    .unreq vec0y
    .unreq vec1
    .unreq vec1x
    .unreq vec1y

    COMPV_GAS_MEMFREE (16*COMPV_GAS_INT16_SZ_BYTES)
	COMPV_GAS_UNALIGN_STACK r11
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 8
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__arm__) && !defined(__aarch64__) */

#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__arm__) && !defined(__aarch64__)
.include "compv_common_arm32.S" /*"*/

#if defined(__APPLE__)
#   define sym(funcname) _##funcname
#else
#   define sym(funcname) funcname
#endif

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
COMPV_GAS_FUNCTION_DECLARE CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32
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
    LoopHeight_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32:
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (i = 0; i < width16; i += 16)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        mov i, width
        LoopWidth_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32:
            veor.f32 vecSum0, vecSum0, vecSum0
            veor.f32 vecSum1, vecSum1, vecSum1
            veor.f32 vecSum2, vecSum2, vecSum2
            veor.f32 vecSum3, vecSum3, vecSum3
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            @ for (row = 0, k = 0; row < kernSize; ++row, k += step)
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            mov inPtrPlusI, inPtr
            mov row, #0
            LoopKernel_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32:
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
                vmla.f32 vecSum0, vec0, vecCoeffx[0]
                vmla.f32 vecSum1, vec1, vecCoeffx[0]
                vmla.f32 vecSum2, vec2, vecCoeffx[0]
                vmla.f32 vecSum3, vec3, vecCoeffx[0]
                blt LoopKernel_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32
            EndOf_LoopKernel_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32:
            
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
            bmi MoreThanWidth16_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32

            @@ if (i < width16) @@
			LessThanWidth16_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32:
				vst1.u8 {vec0}, [outPtr]!
                add inPtr, inPtr, #(16 << COMPV_GAS_UINT8_SHIFT_BYTES)
				b EndOf_MoreThanWidth16_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32
			EndOf_LessThanWidth16_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32:

			@@ if (i >= width16) @@
			MoreThanWidth16_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32:
                add coeff, sp, #mem @ coeff now contains mem address
				vst1.u8 {vec0}, [coeff :128]
				@@ for (; i < width; ++i, ++k) @@
                add i, i, #16 @ was negative and now contains '(width - (width & -16))'
                add inPtr, inPtr, i, LSL #COMPV_GAS_UINT8_SHIFT_BYTES
				LoopMoreThanWidth16_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32:
                    ldrb row, [coeff], #(1*COMPV_GAS_UINT8_SZ_BYTES) @ read uint8_t into row and increment
                    strb row, [outPtr], #(1*COMPV_GAS_UINT8_SZ_BYTES) @ write uint8_t from row and increment
					subs i, i, #1
					bne LoopMoreThanWidth16_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32
				EndOf_LoopMoreThanWidth16_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32:
                b EndOf_LoopWidth_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32
			EndOf_MoreThanWidth16_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32:
            
            bgt LoopWidth_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32 @ branch for far above 'subs i, i, #16' instruction
        EndOf_LoopWidth_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32:

        subs height, height, #1
        add inPtr, inPtr, pad @ ,LSL #COMPV_GAS_UINT8_SHIFT_BYTES omitted
        add outPtr, outPtr, pad @ ,LSL #COMPV_GAS_UINT8_SHIFT_BYTES omitted
        bne LoopHeight_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32
    EndOf_LoopHeight_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32:

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

    @ Change kernelSize to kernSizeInBytes = kernSize * sizeof(int16_t)
    kernSizeInBytes .req kernSize
    mov kernSizeInBytes, kernSize, LSL #1 @ From samples(int16_t) to bytes
    .unreq kernSize

    i .req r8
    row .req r9
    inPtr_ .req r10
    coeff .req r11

    vecSum0 .req q0
    #define vecSum0x q0x
    #define vecSum0y q0y
    vecSum1 .req q1
    #define vecSum1x q1x
    #define vecSum1y q1y
    vecCoeff .req q2
    #define vecCoeffx q2x
    #define vec0 q3
    #define vec0x q3x
    #define vec0y q3y
    #define vec1 q4
    #define vec1x q4x
    #define vec1y q4y
	
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (j = 0; j < height; ++j)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    LoopHeight_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32:
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (i = 0; i < width - 15; i += 16)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        and i, width, #-16 @ Align backward
        LoopWidthPer16Samples_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32:
            veor.u16 vecSum0, vecSum0, vecSum0
            veor.u16 vecSum1, vecSum1, vecSum1
            @pld [inPtr, #(CACHE_LINE_SIZE*3)]
            @pst [outPtr, #(CACHE_LINE_SIZE*3)]
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            @ for (row = 0, k = 0; row < kernSize; ++row, k += step)
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            mov row, #0
            mov inPtr_, inPtr
            LoopKernelPer16Samples_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32:
                vld1.u8 { q15 }, [inPtr_], step @ q15 = vecInPtr
                ldrh coeff, [vthzKernPtr, row]
                add row, row, #COMPV_GAS_INT16_SZ_BYTES
                cmp row, kernSizeInBytes
                vmovl.u8 vec0, q15x
                vdup.s16 vecCoeffx, coeff
                vmovl.u8 vec1, q15y
                vmla.s16 vecSum0, vec0, vecCoeffx[0]
                vmla.s16 vecSum1, vec1, vecCoeffx[0]    
                blt LoopKernelPer16Samples_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32
                @@ EndOf_LoopKernelPer16Samples_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32 @@
            
            vst1.s16 {vecSum0}, [outPtr]!
            vst1.s16 {vecSum1}, [outPtr]!
            subs i, i, #16
            add inPtr, inPtr, #16
            bne LoopWidthPer16Samples_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32
            @@ EndOf_LoopWidthPer16Samples_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32 @@        


        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ if (i < width - 7)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        and i, width, #15 @  modulo 16
        lsrs i, i, #3 @ div 8
        beq EndOf_IfPer8Samples_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32
        IfPer8Samples_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32:
            veor.u16 vecSum0, vecSum0, vecSum0
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            @ for (row = 0, k = 0; row < kernSize; ++row, k += step)
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            mov row, #0
            mov inPtr_, inPtr   
            LoopKernelPer8Samples_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32:
                vld1.u8 { q15x }, [inPtr_], step @ q15x = vecInPtr
                ldrh coeff, [vthzKernPtr, row]
                add row, row, #COMPV_GAS_INT16_SZ_BYTES
                vdup.s16 vecCoeffx, coeff
                vmovl.u8 vec0, q15x
                cmp row, kernSizeInBytes
                vmla.s16 vecSum0, vec0, vecCoeffx[0]
                blt LoopKernelPer8Samples_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32
                @@ EndOf_LoopKernelPer8Samples_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32 @@
            
            vst1.s16 {vecSum0}, [outPtr]!
            add inPtr, inPtr, #8
            EndOf_IfPer8Samples_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32:
            @@ EndOf_IfPer8Samples_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32 @@


        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ if (i < width - 3)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        and i, width, #7 @  modulo 8
        lsrs i, i, #2 @ div 4
        beq EndOf_IfPer4Samples_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32
        IfPer4Samples_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32:
            veor.u16 vecSum0x, vecSum0x, vecSum0x
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            @ for (row = 0, k = 0; row < kernSize; ++row, k += step)
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            mov row, #0
            mov inPtr_, inPtr
            LoopKernelPer4Samples_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32:
                vld1.u8 { q15x }, [inPtr_], step @ q15x = vecInPtr
                ldrh coeff, [vthzKernPtr, row]
                add row, row, #COMPV_GAS_INT16_SZ_BYTES
                cmp row, kernSizeInBytes
                vmovl.u8 vec0, q15x
                vdup.s16 vecCoeffx, coeff
                vmla.s16 vecSum0x, vec0x, vecCoeffx[0]
                blt LoopKernelPer4Samples_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32
                @@ EndOf_LoopKernelPer4Samples_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32 @@
            
            vst1.s16 {vecSum0x}, [outPtr]!
            add inPtr, inPtr, #4
            EndOf_IfPer4Samples_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32:
            @@ EndOf_IfPer4Samples_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32 @@


        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ if (i < width)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        ands i, width, #3 @  modulo 4
        beq EndOf_IfPer1Samples_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32
        IfPer1Samples_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32:
            veor.u16 vecSum0x, vecSum0x, vecSum0x
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            @ for (row = 0, k = 0; row < kernSize; ++row, k += step)
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            mov row, #0
            mov inPtr_, inPtr
            LoopKernelPer1Samples_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32:
                vld1.u8 { q15x }, [inPtr_], step @ q15x = vecInPtr
                ldrh coeff, [vthzKernPtr, row]
                add row, row, #COMPV_GAS_INT16_SZ_BYTES
                cmp row, kernSizeInBytes
                vmovl.u8 vec0, q15x
                vdup.s16 vecCoeffx, coeff
                vmla.s16 vecSum0x, vec0x, vecCoeffx[0]
                blt LoopKernelPer1Samples_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32
                @@ EndOf_LoopKernelPer1Samples_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32 @@
            
              add inPtr, inPtr, i
            .set counter, 0
            .rept 4
                vmov.s16 row, vecSum0x[counter]
                .set counter, counter+1
                strh row, [outPtr], #COMPV_GAS_INT16_SZ_BYTES
                subs i, i, #1
                beq EndOf_IfPer1Samples_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32
            .endr
            EndOf_IfPer1Samples_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32:
            @@ EndOf_IfPer1Samples_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32 @@


        subs height, height, #1
        add inPtr, inPtr, pad @ intPtr contains int16_t samples
        add outPtr, outPtr, pad, LSL #1 @ outPtr contains int16_t samples
		bne LoopHeight_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32
        @@ EndOf_LoopHeight_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32 @@


    .unreq inPtr
	.unreq outPtr
	.unreq width
	.unreq height
	.unreq step
	.unreq pad
	.unreq vthzKernPtr

    .unreq kernSizeInBytes

    .unreq i
    .unreq row
    .unreq inPtr_
    .unreq coeff

    .unreq vecSum0
    #undef vecSum0x
    #undef vecSum0y
    .unreq vecSum1
    #undef vecSum1x
    #undef vecSum1y
    .unreq vecCoeff
    #undef vecCoeffx
    #undef vec0
    #undef vec0x
    #undef vec0y
    #undef vec1
    #undef vec1x
    #undef vec1y

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

    #define argi_vthzKernPtr 6

    @ Change step to stepInBytes = step * sizeof(int16_t)
    mov step, step, LSL #1

    @ Change kernelSize to kernSizeInBytes = kernSize * sizeof(int16_t)
    kernSizeInBytes .req kernSize
    mov kernSizeInBytes, kernSize, LSL #1 @ From samples(int16_t) to bytes
    .unreq kernSize

    i .req r8
    row .req r9
    inPtr_ .req r10
    coeff .req r11

    vecSum0 .req q0
    #define vecSum0x q0x
    #define vecSum0y q0y
    vecSum1 .req q1
    #define vecSum1x q1x
    #define vecSum1y q1y
    vecCoeff .req q2
    #define vecCoeffx q2x
    #define vec0 q3
    #define vec0x q3x
    #define vec0y q3y
    #define vec1 q4
    #define vec1x q4x
    #define vec1y q4y


    .equ COMPV_GAS_INT16_SZ_BYTES, 2
    .equ COMPV_GAS_UINT8_SZ_BYTES, 1
	
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (j = 0; j < height; ++j)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    LoopHeight_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32:
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (i = 0; i < width - 15; i += 16)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        and i, width, #-16 @ Align backward
        LoopWidthPer16Samples_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32:
            veor.u16 vecSum0, vecSum0, vecSum0
            veor.u16 vecSum1, vecSum1, vecSum1
            @pld [inPtr, #(CACHE_LINE_SIZE*3)]
            @pst [outPtr, #(CACHE_LINE_SIZE*3)]
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            @ for (row = 0, k = 0; row < kernSize; ++row, k += step)
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            mov row, kernSizeInBytes
            mov inPtr_, inPtr
            ldr_arg argi_vthzKernPtr, vthzKernPtr
            LoopKernelPer16Samples_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32:
                vld1.s16 { vec0, vec1 }, [inPtr_], step
                ldrh coeff, [vthzKernPtr], #COMPV_GAS_INT16_SZ_BYTES
                subs row, row, #COMPV_GAS_INT16_SZ_BYTES
                vdup.s16 vecCoeffx, coeff
                vmla.s16 vecSum1, vec1, vecCoeffx[0]
                vmla.s16 vecSum0, vec0, vecCoeffx[0]
                bne LoopKernelPer16Samples_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32
                @@ EndOf_LoopKernelPer16Samples_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32 @@
            
            vst1.s16 {vecSum0}, [outPtr]!
            vst1.s16 {vecSum1}, [outPtr]!
            subs i, i, #16
            add inPtr, inPtr, #(16 * COMPV_GAS_INT16_SZ_BYTES)
            bne LoopWidthPer16Samples_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32
            @@ EndOf_LoopWidthPer16Samples_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32 @@        


        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ if (i < width - 7)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        and i, width, #15 @  modulo 16
        lsrs i, i, #3 @ div 8
        beq EndOf_IfPer8Samples_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32
        IfPer8Samples_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32:
            veor.u16 vecSum0, vecSum0, vecSum0
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            @ for (row = 0, k = 0; row < kernSize; ++row, k += step)
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            mov row, kernSizeInBytes
            mov inPtr_, inPtr
            ldr_arg argi_vthzKernPtr, vthzKernPtr
            LoopKernelPer8Samples_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32:
                vld1.s16 { vec0 }, [inPtr_], step
                ldrh coeff, [vthzKernPtr], #COMPV_GAS_INT16_SZ_BYTES
                subs row, row, #COMPV_GAS_INT16_SZ_BYTES
                vdup.s16 vecCoeffx, coeff
                vmla.s16 vecSum0, vec0, vecCoeffx[0]
                bne LoopKernelPer8Samples_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32
                @@ EndOf_LoopKernelPer8Samples_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32 @@
            
            vst1.s16 {vecSum0}, [outPtr]!
            add inPtr, inPtr, #(8 * COMPV_GAS_INT16_SZ_BYTES)
            EndOf_IfPer8Samples_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32:
            @@ EndOf_IfPer8Samples_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32 @@


        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ if (i < width - 3)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        and i, width, #7 @  modulo 8
        lsrs i, i, #2 @ div 4
        beq EndOf_IfPer4Samples_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32
        IfPer4Samples_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32:
            veor.u16 vecSum0x, vecSum0x, vecSum0x
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            @ for (row = 0, k = 0; row < kernSize; ++row, k += step)
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            mov row, kernSizeInBytes
            mov inPtr_, inPtr
            ldr_arg argi_vthzKernPtr, vthzKernPtr
            LoopKernelPer4Samples_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32:
                vld1.u8 { vec0x }, [inPtr_], step
                ldrh coeff, [vthzKernPtr], #COMPV_GAS_INT16_SZ_BYTES
                vdup.s16 vecCoeffx, coeff
                vmla.s16 vecSum0x, vec0x, vecCoeffx[0]
                subs row, row, #COMPV_GAS_INT16_SZ_BYTES
                bne LoopKernelPer4Samples_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32
                @@ EndOf_LoopKernelPer4Samples_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32 @@
            
            vst1.s16 {vecSum0x}, [outPtr]!
            add inPtr, inPtr, #(4 * COMPV_GAS_INT16_SZ_BYTES)
            EndOf_IfPer4Samples_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32:
            @@ EndOf_IfPer4Samples_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32 @@


        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ if (i < width)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        ands i, width, #3 @  modulo 4
        beq EndOf_IfPer1Samples_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32
        IfPer1Samples_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32:
            veor.u16 vecSum0x, vecSum0x, vecSum0x
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            @ for (row = 0, k = 0; row < kernSize; ++row, k += step)
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            mov row, kernSizeInBytes
            mov inPtr_, inPtr
            ldr_arg argi_vthzKernPtr, vthzKernPtr
            LoopKernelPer1Samples_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32:
                vld1.u8 { vec0x }, [inPtr_], step
                ldrh coeff, [vthzKernPtr], #COMPV_GAS_INT16_SZ_BYTES
                subs row, row, #COMPV_GAS_INT16_SZ_BYTES
                vdup.s16 vecCoeffx, coeff
                vmla.s16 vecSum0x, vec0x, vecCoeffx[0]
                bne LoopKernelPer1Samples_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32
                @@ EndOf_LoopKernelPer1Samples_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32 @@
            
              add inPtr, inPtr, i, LSL #1
            .set counter, 0
            .rept 4
                vmov.s16 row, vecSum0x[counter]
                .set counter, counter+1
                strh row, [outPtr], #COMPV_GAS_INT16_SZ_BYTES
                subs i, i, #1
                beq EndOf_IfPer1Samples_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32
            .endr
            EndOf_IfPer1Samples_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32:
            @@ EndOf_IfPer1Samples_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32 @@


        subs height, height, #1
        add inPtr, inPtr, pad, LSL #1 @ inPtr contains int16_t samples
        add outPtr, outPtr, pad, LSL #1 @ outPtr contains int16_t samples
		bne LoopHeight_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32
        @@ EndOf_LoopHeight_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32 @@


    .unreq inPtr
	.unreq outPtr
	.unreq width
	.unreq height
	.unreq step
	.unreq pad
	.unreq vthzKernPtr
    #undef argi_vthzKernPtr

    .unreq kernSizeInBytes

    .unreq i
    .unreq row
    .unreq inPtr_
    .unreq coeff

    .unreq vecSum0
    #undef vecSum0x
    #undef vecSum0y
    .unreq vecSum1
    #undef vecSum1x
    #undef vecSum1y
    .unreq vecCoeff
    #undef vecCoeffx
    #undef vec0
    #undef vec0x
    #undef vec0y
    #undef vec1
    #undef vec1x
    #undef vec1y

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 8
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__arm__) && !defined(__aarch64__) */

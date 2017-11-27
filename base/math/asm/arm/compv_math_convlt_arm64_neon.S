#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__aarch64__)
.include "compv_common_arm64.S" //"

#if defined(__APPLE__)
#   define sym(funcname) _##funcname
#else
#   define sym(funcname) funcname
#endif

.data

.extern
 
.text

#########################################################################
# arg(0) -> const uint8_t* inPtr
# arg(1) -> uint8_t* outPtr
# arg(2) -> compv_uscalar_t width
# arg(3) -> compv_uscalar_t height
# arg(4) -> compv_uscalar_t step
# arg(5) -> compv_uscalar_t pad
# arg(6) -> const uint16_t* vthzKernPtr
# arg(7) -> compv_uscalar_t kernSize
COMPV_GAS_FUNCTION_DECLARE CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS
	COMPV_GAS_MEMALLOC (16*COMPV_GAS_UINT8_SZ_BYTES)

	## Set arguments ##
	inPtr .req r0
	outPtr .req r1
	width .req r2
	height .req r3
	step .req r4
	pad .req r5
	vthzKernPtr .req r6
	kernSize .req r7

    ## Local variables ##
    .equ mem		, 0
    i .req r8
    row .req r9
    inPtrPlusI .req r10
    coeff .req r11
    coeffw .req r11w
    memPtr .req r12

    vecSum0 .req v0
    vecSum1 .req v1
    vecCoeff .req v2
    vec0 .req v3
    vec1 .req v4
    vec2 .req v5
    vec3 .req v6
    vec4 .req v7
    vec5 .req v8

    # Change step in bytes unit (for inPtrPlus)
    lsl step, step, #COMPV_GAS_UINT8_SHIFT_BYTES

    # Change kernSize in bytes (for vthzKernPtr)
    lsl kernSize, kernSize, #COMPV_GAS_UINT16_SHIFT_BYTES
	
    ###########################################################
    # for (j = 0; j < height; ++j)
    ###########################################################
    LoopHeight_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64:
        ###########################################################
        # for (i = 0; i < width16; i += 16)
        ###########################################################
        mov i, width
        LoopWidth_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64:
            eor vecSum0.16b, vecSum0.16b, vecSum0.16b
            eor vecSum1.16b, vecSum1.16b, vecSum1.16b
            ###########################################################
            # for (row = 0, k = 0; row < kernSize; ++row, k += step)
            ###########################################################
            mov inPtrPlusI, inPtr
            mov row, #0
            LoopKernel_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64:
                ld1 { vec0.16b }, [inPtrPlusI], step
                ldrh coeffw, [vthzKernPtr, row]
                add row, row, #COMPV_GAS_UINT16_SZ_BYTES
                dup vecCoeff.8h, coeffw
                uxtl vec4.8h, vec0.8b
                uxtl2 vec5.8h, vec0.16b
                umull vec0.4s, vec4.4h, vecCoeff.4h
                umull2 vec1.4s, vec4.8h, vecCoeff.8h
                umull vec2.4s, vec5.4h, vecCoeff.4h
                umull2 vec3.4s, vec5.8h, vecCoeff.8h
                cmp row, kernSize
                shrn vec4.4h, vec0.4s, #16
                shrn2 vec4.8h, vec1.4s, #16
                uqadd vecSum0.8h, vecSum0.8h, vec4.8h
                shrn vec5.4h, vec2.4s, #16
                shrn2 vec5.8h, vec3.4s, #16
                uqadd vecSum1.8h, vecSum1.8h, vec5.8h
                blt LoopKernel_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64
            EndOf_LoopKernel_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64:
            
            uqxtn vec4.8b, vecSum0.8h
            uqxtn2 vec4.16b, vecSum1.8h
            subs i, i, #16
            bmi MoreThanWidth16_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64

            ## if (i < width16) ##
			LessThanWidth16_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64:
				st1 {vec4.16b}, [outPtr], #(16*COMPV_GAS_UINT8_SZ_BYTES)
                add inPtr, inPtr, #(16*COMPV_GAS_UINT8_SZ_BYTES)
				b EndOf_MoreThanWidth16_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64
			EndOf_LessThanWidth16_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64:

			## if (i >= width16) ##
			MoreThanWidth16_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64:
                add memPtr, sp, #mem
				st1 {vec4.16b}, [memPtr]
				## for (; i < width; ++i, ++k) ##
                add i, i, #16 // was negative and now contains '(width - (width & -16))'
                add inPtr, inPtr, i, LSL #(COMPV_GAS_UINT8_SHIFT_BYTES)
				LoopMoreThanWidth16_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64:
                    ldrb coeffw, [memPtr], #(1*COMPV_GAS_UINT8_SZ_BYTES) // read uint8_t into coeffw and increment
                    subs i, i, #1
                    strb coeffw, [outPtr], #(1*COMPV_GAS_UINT8_SZ_BYTES) // write uint8_t from coeffw and increment
					bne LoopMoreThanWidth16_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64
				EndOf_LoopMoreThanWidth16_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64:
                b EndOf_LoopWidth_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64
			EndOf_MoreThanWidth16_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64:
            
            bgt LoopWidth_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64 // branch for far above 'subs i, i, #16' instruction
        EndOf_LoopWidth_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64:

        subs height, height, #1
        add inPtr, inPtr, pad // ,LSL #COMPV_GAS_UINT8_SHIFT_BYTES omitted
        add outPtr, outPtr, pad // ,LSL #COMPV_GAS_UINT8_SHIFT_BYTES omitted
        bne LoopHeight_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64
    EndOf_LoopHeight_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64:

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
    .unreq coeffw
    .unreq memPtr

    .unreq vecSum0
    .unreq vecSum1
    .unreq vecCoeff
    .unreq vec0
    .unreq vec1
    .unreq vec2
    .unreq vec3
    .unreq vec4
    .unreq vec5

    COMPV_GAS_MEMFREE (16*COMPV_GAS_UINT8_SZ_BYTES)
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#########################################################################
# arg(0) -> const uint8_t* inPtr
# arg(1) -> uint8_t* outPtr
# arg(2) -> compv_uscalar_t width
# arg(3) -> compv_uscalar_t height
# arg(4) -> compv_uscalar_t step
# arg(5) -> compv_uscalar_t pad
# arg(6) -> const compv_float32_t* vthzKernPtr
# arg(7) -> compv_uscalar_t kernSize
COMPV_GAS_FUNCTION_DECLARE CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS
	COMPV_GAS_MEMALLOC (16*COMPV_GAS_UINT8_SZ_BYTES)

	## Set arguments ##
	inPtr .req r0
	outPtr .req r1
	width .req r2
	height .req r3
	step .req r4
	pad .req r5
	vthzKernPtr .req r6
	kernSize .req r7

    ## Local variables ##
    .equ mem		, 0
    i .req r8
    row .req r9
    inPtrPlusI .req r10
    coeff .req r11
    coeffw .req r11w
    memPtr .req r12

    vecSum0 .req v0
    vecSum1 .req v1
    vecSum2 .req v2
    vecSum3 .req v3
    vecCoeff .req v4
    vec0 .req v5
    vec1 .req v6
    vec2 .req v7
    vec3 .req v8
    vec4 .req v9
    vec5 .req v10

    # Change step in bytes unit (for inPtrPlus)
    lsl step, step, #COMPV_GAS_UINT8_SHIFT_BYTES

    # Change kernSize in bytes (for vthzKernPtr)
    lsl kernSize, kernSize, #COMPV_GAS_FLOAT32_SHIFT_BYTES
	
    ###########################################################
    # for (j = 0; j < height; ++j)
    ###########################################################
    LoopHeight_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64:
        ###########################################################
        # for (i = 0; i < width16; i += 16)
        ###########################################################
        mov i, width
        LoopWidth_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64:
            eor vecSum0.16b, vecSum0.16b, vecSum0.16b
            eor vecSum1.16b, vecSum1.16b, vecSum1.16b
            eor vecSum2.16b, vecSum2.16b, vecSum2.16b
            eor vecSum3.16b, vecSum3.16b, vecSum3.16b
            ###########################################################
            # for (row = 0, k = 0; row < kernSize; ++row, k += step)
            ###########################################################
            mov inPtrPlusI, inPtr
            mov row, #0
            LoopKernel_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64:
                ld1 { vec0.16b }, [inPtrPlusI], step
                ldr coeffw, [vthzKernPtr, row]
                uxtl vec4.8h, vec0.8b
                uxtl2 vec5.8h, vec0.16b
                dup vecCoeff.4s, coeffw
                uxtl vec0.4s, vec4.4h
				uxtl2 vec1.4s, vec4.8h
				uxtl vec2.4s, vec5.4h
				uxtl2 vec3.4s, vec5.8h
                ucvtf vec0.4s, vec0.4s
                ucvtf vec1.4s, vec1.4s
                ucvtf vec2.4s, vec2.4s
                ucvtf vec3.4s, vec3.4s
                add row, row, #COMPV_GAS_FLOAT32_SZ_BYTES
                fmul v20.4s, vec0.4s, vecCoeff.s[0]
                fmul v21.4s, vec1.4s, vecCoeff.s[0]
                fmul v22.4s, vec2.4s, vecCoeff.s[0]
                fmul v23.4s, vec3.4s, vecCoeff.s[0]
                cmp row, kernSize
                fadd vecSum0.4s, vecSum0.4s, v20.4s
                fadd vecSum1.4s, vecSum1.4s, v21.4s
                fadd vecSum2.4s, vecSum2.4s, v22.4s
                fadd vecSum3.4s, vecSum3.4s, v23.4s
                blt LoopKernel_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64
            EndOf_LoopKernel_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64:

            fcvtzu vec0.4s, vecSum0.4s
            fcvtzu vec1.4s, vecSum1.4s
            fcvtzu vec2.4s, vecSum2.4s
            fcvtzu vec3.4s, vecSum3.4s
            subs i, i, #16
            uqxtn v13.4h, vec0.4s
            uqxtn2 v13.8h, vec1.4s
            uqxtn v14.4h, vec2.4s
            uqxtn2 v14.8h, vec3.4s
            uqxtn vec4.8b, v13.8h
            uqxtn2 vec4.16b, v14.8h             
            bmi MoreThanWidth16_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64

            ## if (i < width16) ##
			LessThanWidth16_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64:
				st1 {vec4.16b}, [outPtr], #(16*COMPV_GAS_UINT8_SZ_BYTES)
                add inPtr, inPtr, #(16*COMPV_GAS_UINT8_SZ_BYTES)
				b EndOf_MoreThanWidth16_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64
			EndOf_LessThanWidth16_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64:

			## if (i >= width16) ##
			MoreThanWidth16_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64:
                add memPtr, sp, #mem
				st1 {vec4.16b}, [memPtr]
				## for (; i < width; ++i, ++k) ##
                add i, i, #16 // was negative and now contains '(width - (width & -16))'
                add inPtr, inPtr, i, LSL #(COMPV_GAS_UINT8_SHIFT_BYTES)
				LoopMoreThanWidth16_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64:
                    ldrb coeffw, [memPtr], #(1*COMPV_GAS_UINT8_SZ_BYTES) // read uint8_t into coeffw and increment
                    subs i, i, #1
                    strb coeffw, [outPtr], #(1*COMPV_GAS_UINT8_SZ_BYTES) // write uint8_t from coeffw and increment
					bne LoopMoreThanWidth16_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64
				EndOf_LoopMoreThanWidth16_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64:
                b EndOf_LoopWidth_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64
			EndOf_MoreThanWidth16_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64:
            
            bgt LoopWidth_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64 // branch for far above 'subs i, i, #16' instruction
        EndOf_LoopWidth_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64:

        subs height, height, #1
        add inPtr, inPtr, pad // ,LSL #COMPV_GAS_UINT8_SHIFT_BYTES omitted
        add outPtr, outPtr, pad // ,LSL #COMPV_GAS_UINT8_SHIFT_BYTES omitted
        bne LoopHeight_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64
    EndOf_LoopHeight_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64:

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
    .unreq coeffw
    .unreq memPtr

    .unreq vecSum0
    .unreq vecSum1
    .unreq vecSum2
    .unreq vecSum3
    .unreq vecCoeff
    .unreq vec0
    .unreq vec1
    .unreq vec2
    .unreq vec3
    .unreq vec4
    .unreq vec5

    COMPV_GAS_MEMFREE (16*COMPV_GAS_UINT8_SZ_BYTES)
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN



#########################################################################
# arg(0) -> const uint8_t* inPtr
# arg(1) -> compv_float32_t* outPtr
# arg(2) -> compv_uscalar_t width
# arg(3) -> compv_uscalar_t height
# arg(4) -> compv_uscalar_t step
# arg(5) -> compv_uscalar_t pad
# arg(6) -> const compv_float32_t* vthzKernPtr
# arg(7) -> compv_uscalar_t kernSize
COMPV_GAS_FUNCTION_DECLARE CompVMathConvlt1VtHz_8u32f32f_Asm_NEON64
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS
	COMPV_GAS_MEMALLOC (16*COMPV_GAS_FLOAT32_SZ_BYTES)

	## Set arguments ##
	inPtr .req r0
	outPtr .req r1
	width .req r2
	height .req r3
	step .req r4
	pad .req r5
	vthzKernPtr .req r6
	kernSize .req r7

    ## Local variables ##
    .equ mem		, 0
    i .req r8
    row .req r9
    inPtrPlusI .req r10
    coeff .req r11
    coeffw .req r11w
    memPtr .req r12

    vecSum0 .req v0
    vecSum1 .req v1
    vecSum2 .req v2
    vecSum3 .req v3
    vecCoeff .req v4
    vec0 .req v5
    vec1 .req v6
    vec2 .req v7
    vec3 .req v8
    vec4 .req v9
    vec5 .req v10

    # Change step in bytes unit (for inPtrPlus)
    lsl step, step, #COMPV_GAS_UINT8_SHIFT_BYTES

    # Change kernSize in bytes (for vthzKernPtr)
    lsl kernSize, kernSize, #COMPV_GAS_FLOAT32_SHIFT_BYTES
	
    ###########################################################
    # for (j = 0; j < height; ++j)
    ###########################################################
    LoopHeight_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON64:
        ###########################################################
        # for (i = 0; i < width16; i += 16)
        ###########################################################
        mov i, width
        LoopWidth_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON64:
            eor vecSum0.16b, vecSum0.16b, vecSum0.16b
            eor vecSum1.16b, vecSum1.16b, vecSum1.16b
            eor vecSum2.16b, vecSum2.16b, vecSum2.16b
            eor vecSum3.16b, vecSum3.16b, vecSum3.16b
            ###########################################################
            # for (row = 0, k = 0; row < kernSize; ++row, k += step)
            ###########################################################
            mov inPtrPlusI, inPtr
            mov row, #0
            LoopKernel_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON64:
                ld1 { vec0.16b }, [inPtrPlusI], step
                ldr coeffw, [vthzKernPtr, row]
                uxtl vec4.8h, vec0.8b
                uxtl2 vec5.8h, vec0.16b
                dup vecCoeff.4s, coeffw
                uxtl vec0.4s, vec4.4h
				uxtl2 vec1.4s, vec4.8h
				uxtl vec2.4s, vec5.4h
				uxtl2 vec3.4s, vec5.8h
                ucvtf vec0.4s, vec0.4s
                ucvtf vec1.4s, vec1.4s
                ucvtf vec2.4s, vec2.4s
                ucvtf vec3.4s, vec3.4s
                add row, row, #COMPV_GAS_FLOAT32_SZ_BYTES
                fmul v20.4s, vec0.4s, vecCoeff.s[0]
                fmul v21.4s, vec1.4s, vecCoeff.s[0]
                fmul v22.4s, vec2.4s, vecCoeff.s[0]
                fmul v23.4s, vec3.4s, vecCoeff.s[0]
                cmp row, kernSize
                fadd vecSum0.4s, vecSum0.4s, v20.4s
                fadd vecSum1.4s, vecSum1.4s, v21.4s
                fadd vecSum2.4s, vecSum2.4s, v22.4s
                fadd vecSum3.4s, vecSum3.4s, v23.4s
                blt LoopKernel_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON64
            EndOf_LoopKernel_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON64:

            subs i, i, #16          
            bmi MoreThanWidth16_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON64

            ## if (i < width16) ##
			LessThanWidth16_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON64:
				st1 {vecSum0.4s-vecSum3.4s}, [outPtr], #(16*COMPV_GAS_FLOAT32_SZ_BYTES)
                add inPtr, inPtr, #(16*COMPV_GAS_UINT8_SZ_BYTES)
				b EndOf_MoreThanWidth16_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON64
			EndOf_LessThanWidth16_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON64:

			## if (i >= width16) ##
			MoreThanWidth16_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON64:
                add memPtr, sp, #mem
				st1 {vecSum0.4s-vecSum3.4s}, [memPtr]
				## for (; i < width; ++i, ++k) ##
                add i, i, #16 // was negative and now contains '(width - (width & -16))'
                add inPtr, inPtr, i, LSL #(COMPV_GAS_UINT8_SHIFT_BYTES)
				LoopMoreThanWidth16_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON64:
                    ldr coeffw, [memPtr], #(1*COMPV_GAS_FLOAT32_SZ_BYTES) // read float32_t into coeffw and increment
                    subs i, i, #1
                    str coeffw, [outPtr], #(1*COMPV_GAS_FLOAT32_SZ_BYTES) // write float32_t from coeffw and increment
					bne LoopMoreThanWidth16_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON64
				EndOf_LoopMoreThanWidth16_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON64:
                b EndOf_LoopWidth_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON64
			EndOf_MoreThanWidth16_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON64:
            
            bgt LoopWidth_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON64 // branch for far above 'subs i, i, #16' instruction
        EndOf_LoopWidth_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON64:

        subs height, height, #1
        add inPtr, inPtr, pad // ,LSL #COMPV_GAS_UINT8_SHIFT_BYTES omitted
        add outPtr, outPtr, pad, LSL #(COMPV_GAS_FLOAT32_SHIFT_BYTES)
        bne LoopHeight_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON64
    EndOf_LoopHeight_CompVMathConvlt1VtHz_8u32f32f_Asm_NEON64:

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
    .unreq coeffw
    .unreq memPtr

    .unreq vecSum0
    .unreq vecSum1
    .unreq vecSum2
    .unreq vecSum3
    .unreq vecCoeff
    .unreq vec0
    .unreq vec1
    .unreq vec2
    .unreq vec3
    .unreq vec4
    .unreq vec5

    COMPV_GAS_MEMFREE (16*COMPV_GAS_FLOAT32_SZ_BYTES)
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN



#########################################################################
# arg(0) -> const compv_float32_t* inPtr
# arg(1) -> compv_float32_t* outPtr
# arg(2) -> compv_uscalar_t width
# arg(3) -> compv_uscalar_t height
# arg(4) -> compv_uscalar_t step
# arg(5) -> compv_uscalar_t pad
# arg(6) -> const compv_float32_t* vthzKernPtr
# arg(7) -> compv_uscalar_t kernSize
COMPV_GAS_FUNCTION_DECLARE CompVMathConvlt1VtHz_32f32f32f_Asm_NEON64
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS
	COMPV_GAS_MEMALLOC (16*COMPV_GAS_FLOAT32_SZ_BYTES)

	## Set arguments ##
	inPtr .req r0
	outPtr .req r1
	width .req r2
	height .req r3
	step .req r4
	pad .req r5
	vthzKernPtr .req r6
	kernSize .req r7

    ## Local variables ##
    .equ mem		, 0
    i .req r8
    row .req r9
    inPtrPlusI .req r10
    coeff .req r11
    coeffw .req r11w
    memPtr .req r12

    vecSum0 .req v0
    vecSum1 .req v1
    vecSum2 .req v2
    vecSum3 .req v3
    vecCoeff .req v4
    vec0 .req v5
    vec1 .req v6
    vec2 .req v7
    vec3 .req v8
    vec4 .req v9
    vec5 .req v10

    # Change step in bytes unit (for inPtrPlus)
    lsl step, step, #COMPV_GAS_FLOAT32_SHIFT_BYTES

    # Change kernSize in bytes (for vthzKernPtr)
    lsl kernSize, kernSize, #COMPV_GAS_FLOAT32_SHIFT_BYTES

    # Change pad in bytes (for inPtr and outPtr)
    lsl pad, pad, #COMPV_GAS_FLOAT32_SHIFT_BYTES
	
    ###########################################################
    # for (j = 0; j < height; ++j)
    ###########################################################
    LoopHeight_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON64:
        ###########################################################
        # for (i = 0; i < width16; i += 16)
        ###########################################################
        mov i, width
        LoopWidth_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON64:
            eor vecSum0.16b, vecSum0.16b, vecSum0.16b
            eor vecSum1.16b, vecSum1.16b, vecSum1.16b
            eor vecSum2.16b, vecSum2.16b, vecSum2.16b
            eor vecSum3.16b, vecSum3.16b, vecSum3.16b
            ###########################################################
            # for (row = 0, k = 0; row < kernSize; ++row, k += step)
            ###########################################################
            mov inPtrPlusI, inPtr
            mov row, #0
            LoopKernel_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON64:
                ld1 { vec0.4s-vec3.4s }, [inPtrPlusI], step
                ldr coeffw, [vthzKernPtr, row]
                add row, row, #COMPV_GAS_FLOAT32_SZ_BYTES
                dup vecCoeff.4s, coeffw
                fmul vec0.4s, vec0.4s, vecCoeff.s[0]
                fmul vec1.4s, vec1.4s, vecCoeff.s[0]
                fmul vec2.4s, vec2.4s, vecCoeff.s[0]
                fmul vec3.4s, vec3.4s, vecCoeff.s[0]
                cmp row, kernSize
                fadd vecSum0.4s, vecSum0.4s, vec0.4s
                fadd vecSum1.4s, vecSum1.4s, vec1.4s
                fadd vecSum2.4s, vecSum2.4s, vec2.4s
                fadd vecSum3.4s, vecSum3.4s, vec3.4s
                blt LoopKernel_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON64
            EndOf_LoopKernel_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON64:

            subs i, i, #16          
            bmi MoreThanWidth16_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON64

            ## if (i < width16) ##
			LessThanWidth16_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON64:
				st1 {vecSum0.4s-vecSum3.4s}, [outPtr], #(16*COMPV_GAS_FLOAT32_SZ_BYTES)
                add inPtr, inPtr, #(16*COMPV_GAS_FLOAT32_SZ_BYTES)
				b EndOf_MoreThanWidth16_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON64
			EndOf_LessThanWidth16_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON64:

			## if (i >= width16) ##
			MoreThanWidth16_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON64:
                add memPtr, sp, #mem
				st1 {vecSum0.4s-vecSum3.4s}, [memPtr]
				## for (; i < width; ++i, ++k) ##
                add i, i, #16 // was negative and now contains '(width - (width & -16))'
                add inPtr, inPtr, i, LSL #(COMPV_GAS_FLOAT32_SHIFT_BYTES)
				LoopMoreThanWidth16_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON64:
                    ldr coeffw, [memPtr], #(1*COMPV_GAS_FLOAT32_SZ_BYTES) // read float32_t into coeffw and increment
                    subs i, i, #1
                    str coeffw, [outPtr], #(1*COMPV_GAS_FLOAT32_SZ_BYTES) // write float32_t from coeffw and increment
					bne LoopMoreThanWidth16_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON64
				EndOf_LoopMoreThanWidth16_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON64:
                b EndOf_LoopWidth_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON64
			EndOf_MoreThanWidth16_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON64:
            
            bgt LoopWidth_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON64 // branch for far above 'subs i, i, #16' instruction
        EndOf_LoopWidth_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON64:

        subs height, height, #1
        add inPtr, inPtr, pad // ,LSL #COMPV_GAS_FLOAT32_SHIFT_BYTES omitted
        add outPtr, outPtr, pad //, LSL #(COMPV_GAS_FLOAT32_SHIFT_BYTES) omitted
        bne LoopHeight_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON64
    EndOf_LoopHeight_CompVMathConvlt1VtHz_32f32f32f_Asm_NEON64:

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
    .unreq coeffw
    .unreq memPtr

    .unreq vecSum0
    .unreq vecSum1
    .unreq vecSum2
    .unreq vecSum3
    .unreq vecCoeff
    .unreq vec0
    .unreq vec1
    .unreq vec2
    .unreq vec3
    .unreq vec4
    .unreq vec5

    COMPV_GAS_MEMFREE (16*COMPV_GAS_FLOAT32_SZ_BYTES)
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#########################################################################
# arg(0) -> const compv_float32_t* inPtr
# arg(1) -> uint8_t* outPtr
# arg(2) -> compv_uscalar_t width
# arg(3) -> compv_uscalar_t height
# arg(4) -> compv_uscalar_t step
# arg(5) -> compv_uscalar_t pad
# arg(6) -> const compv_float32_t* vthzKernPtr
# arg(7) -> compv_uscalar_t kernSize
COMPV_GAS_FUNCTION_DECLARE CompVMathConvlt1VtHz_32f32f8u_Asm_NEON64
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS
	COMPV_GAS_MEMALLOC (16*COMPV_GAS_UINT8_SZ_BYTES)

	## Set arguments ##
	inPtr .req r0
	outPtr .req r1
	width .req r2
	height .req r3
	step .req r4
	pad .req r5
	vthzKernPtr .req r6
	kernSize .req r7

    ## Local variables ##
    .equ mem		, 0
    i .req r8
    row .req r9
    inPtrPlusI .req r10
    coeff .req r11
    coeffw .req r11w
    memPtr .req r12

    vecSum0 .req v0
    vecSum1 .req v1
    vecSum2 .req v2
    vecSum3 .req v3
    vecCoeff .req v4
    vec0 .req v5
    vec1 .req v6
    vec2 .req v7
    vec3 .req v8
    vec4 .req v9
    vec5 .req v10

    # Change step in bytes unit (for inPtrPlus)
    lsl step, step, #COMPV_GAS_FLOAT32_SHIFT_BYTES

    # Change kernSize in bytes (for vthzKernPtr)
    lsl kernSize, kernSize, #COMPV_GAS_FLOAT32_SHIFT_BYTES
	
    ###########################################################
    # for (j = 0; j < height; ++j)
    ###########################################################
    LoopHeight_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON64:
        ###########################################################
        # for (i = 0; i < width16; i += 16)
        ###########################################################
        mov i, width
        LoopWidth_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON64:
            eor vecSum0.16b, vecSum0.16b, vecSum0.16b
            eor vecSum1.16b, vecSum1.16b, vecSum1.16b
            eor vecSum2.16b, vecSum2.16b, vecSum2.16b
            eor vecSum3.16b, vecSum3.16b, vecSum3.16b
            ###########################################################
            # for (row = 0, k = 0; row < kernSize; ++row, k += step)
            ###########################################################
            mov inPtrPlusI, inPtr
            mov row, #0
            LoopKernel_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON64:
                ld1 { vec0.4s-vec3.4s }, [inPtrPlusI], step
                ldr coeffw, [vthzKernPtr, row]
                add row, row, #COMPV_GAS_FLOAT32_SZ_BYTES
                dup vecCoeff.4s, coeffw
                fmul vec0.4s, vec0.4s, vecCoeff.s[0]
                fmul vec1.4s, vec1.4s, vecCoeff.s[0]
                fmul vec2.4s, vec2.4s, vecCoeff.s[0]
                fmul vec3.4s, vec3.4s, vecCoeff.s[0]
                cmp row, kernSize
                fadd vecSum0.4s, vecSum0.4s, vec0.4s
                fadd vecSum1.4s, vecSum1.4s, vec1.4s
                fadd vecSum2.4s, vecSum2.4s, vec2.4s
                fadd vecSum3.4s, vecSum3.4s, vec3.4s
                blt LoopKernel_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON64
            EndOf_LoopKernel_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON64:

            fcvtzu vec0.4s, vecSum0.4s
            fcvtzu vec1.4s, vecSum1.4s
            fcvtzu vec2.4s, vecSum2.4s
            fcvtzu vec3.4s, vecSum3.4s
            subs i, i, #16
            uqxtn v13.4h, vec0.4s
            uqxtn2 v13.8h, vec1.4s
            uqxtn v14.4h, vec2.4s
            uqxtn2 v14.8h, vec3.4s
            uqxtn vec4.8b, v13.8h
            uqxtn2 vec4.16b, v14.8h           
            bmi MoreThanWidth16_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON64

            ## if (i < width16) ##
			LessThanWidth16_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON64:
				st1 {vec4.16b}, [outPtr], #(16*COMPV_GAS_UINT8_SZ_BYTES)
                add inPtr, inPtr, #(16*COMPV_GAS_FLOAT32_SZ_BYTES)
				b EndOf_MoreThanWidth16_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON64
			EndOf_LessThanWidth16_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON64:

			## if (i >= width16) ##
			MoreThanWidth16_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON64:
                add memPtr, sp, #mem
				st1 {vec4.16b}, [memPtr]
				## for (; i < width; ++i, ++k) ##
                add i, i, #16 // was negative and now contains '(width - (width & -16))'
                add inPtr, inPtr, i, LSL #(COMPV_GAS_FLOAT32_SHIFT_BYTES)
				LoopMoreThanWidth16_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON64:
                    ldrb coeffw, [memPtr], #(1*COMPV_GAS_UINT8_SZ_BYTES) // read uint8_t into coeffw and increment
                    subs i, i, #1
                    strb coeffw, [outPtr], #(1*COMPV_GAS_UINT8_SZ_BYTES) // write uint8_t from coeffw and increment
					bne LoopMoreThanWidth16_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON64
				EndOf_LoopMoreThanWidth16_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON64:
                b EndOf_LoopWidth_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON64
			EndOf_MoreThanWidth16_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON64:
            
            bgt LoopWidth_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON64 // branch for far above 'subs i, i, #16' instruction
        EndOf_LoopWidth_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON64:

        subs height, height, #1
        add inPtr, inPtr, pad, LSL #(COMPV_GAS_FLOAT32_SHIFT_BYTES)
        add outPtr, outPtr, pad //, LSL #(COMPV_GAS_UINT8_SHIFT_BYTES) omitted
        bne LoopHeight_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON64
    EndOf_LoopHeight_CompVMathConvlt1VtHz_32f32f8u_Asm_NEON64:

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
    .unreq coeffw
    .unreq memPtr

    .unreq vecSum0
    .unreq vecSum1
    .unreq vecSum2
    .unreq vecSum3
    .unreq vecCoeff
    .unreq vec0
    .unreq vec1
    .unreq vec2
    .unreq vec3
    .unreq vec4
    .unreq vec5

    COMPV_GAS_MEMFREE (16*COMPV_GAS_UINT8_SZ_BYTES)
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN


#########################################################################
# arg(0) -> const uint8_t* inPtr
# arg(1) -> int16_t* outPtr
# arg(2) -> compv_uscalar_t width
# arg(3) -> compv_uscalar_t height
# arg(4) -> compv_uscalar_t step
# arg(5) -> compv_uscalar_t pad
# arg(6) -> const int16_t* vthzKernPtr
# arg(7) -> compv_uscalar_t kernSize
COMPV_GAS_FUNCTION_DECLARE CompVMathConvlt1VtHz_8u16s16s_Asm_NEON64
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS
	COMPV_GAS_MEMALLOC (16*COMPV_GAS_INT16_SZ_BYTES)

	## Set arguments ##
	inPtr .req r0
	outPtr .req r1
	width .req r2
	height .req r3
	step .req r4
	pad .req r5
	vthzKernPtr .req r6
	kernSize .req r7

    ## Local variables ##
    .equ mem		, 0
    i .req r8
    row .req r9
    inPtrPlusI .req r10
    coeff .req r11
    coeffw .req r11w
    memPtr .req r12

    vecSum0 .req v0
    vecSum1 .req v1
    vecSum2 .req v2
    vecSum3 .req v3
    vecCoeff .req v4
    vec0 .req v5
    vec1 .req v6
    vec2 .req v7
    vec3 .req v8
    vec4 .req v9
    vec5 .req v10

    # Change step in bytes unit (for inPtrPlus)
    lsl step, step, #COMPV_GAS_UINT8_SHIFT_BYTES

    # Change kernSize in bytes (for vthzKernPtr)
    lsl kernSize, kernSize, #COMPV_GAS_INT16_SHIFT_BYTES
	
    ###########################################################
    # for (j = 0; j < height; ++j)
    ###########################################################
    LoopHeight_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON64:
        ###########################################################
        # for (i = 0; i < width16; i += 16)
        ###########################################################
        mov i, width
        LoopWidth_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON64:
            eor vecSum0.16b, vecSum0.16b, vecSum0.16b
            eor vecSum1.16b, vecSum1.16b, vecSum1.16b
            eor vecSum2.16b, vecSum2.16b, vecSum2.16b
            eor vecSum3.16b, vecSum3.16b, vecSum3.16b
            ###########################################################
            # for (row = 0, k = 0; row < kernSize; ++row, k += step)
            ###########################################################
            mov inPtrPlusI, inPtr
            mov row, #0
            LoopKernel_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON64:
                ld1 { vec0.16b }, [inPtrPlusI], step
                ldrh coeffw, [vthzKernPtr, row]
                add row, row, #COMPV_GAS_INT16_SZ_BYTES
                dup vecCoeff.8h, coeffw
                uxtl vec4.8h, vec0.8b
                uxtl2 vec5.8h, vec0.16b
                smlal vecSum0.4s, vec4.4h, vecCoeff.4h
                smlal2 vecSum1.4s, vec4.8h, vecCoeff.8h
                smlal vecSum2.4s, vec5.4h, vecCoeff.4h
                smlal2 vecSum3.4s, vec5.8h, vecCoeff.8h
                cmp row, kernSize
                blt LoopKernel_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON64
            EndOf_LoopKernel_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON64:
            
            sqxtn vec0.4h, vecSum0.4s
            sqxtn2 vec0.8h, vecSum1.4s
            sqxtn vec1.4h, vecSum2.4s
            sqxtn2 vec1.8h, vecSum3.4s
            subs i, i, #16
            bmi MoreThanWidth16_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON64

            ## if (i < width16) ##
			LessThanWidth16_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON64:
				st1 {vec0.8h,vec1.8h}, [outPtr], #(16*COMPV_GAS_INT16_SZ_BYTES)
                add inPtr, inPtr, #(16*COMPV_GAS_UINT8_SZ_BYTES)
				b EndOf_MoreThanWidth16_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON64
			EndOf_LessThanWidth16_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON64:

			## if (i >= width16) ##
			MoreThanWidth16_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON64:
                add memPtr, sp, #mem
				st1 {vec0.8h,vec1.8h}, [memPtr]
				## for (; i < width; ++i, ++k) ##
                add i, i, #16 // was negative and now contains '(width - (width & -16))'
                add inPtr, inPtr, i, LSL #(COMPV_GAS_UINT8_SHIFT_BYTES)
				LoopMoreThanWidth16_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON64:
                    ldrh coeffw, [memPtr], #(1*COMPV_GAS_INT16_SZ_BYTES) // read int16_t into coeffw and increment
                    subs i, i, #1
                    strh coeffw, [outPtr], #(1*COMPV_GAS_INT16_SZ_BYTES) // write int16_t from coeffw and increment
					bne LoopMoreThanWidth16_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON64
				EndOf_LoopMoreThanWidth16_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON64:
                b EndOf_LoopWidth_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON64
			EndOf_MoreThanWidth16_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON64:
            
            bgt LoopWidth_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON64 // branch for far above 'subs i, i, #16' instruction
        EndOf_LoopWidth_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON64:

        subs height, height, #1
        add inPtr, inPtr, pad // ,LSL #COMPV_GAS_UINT8_SHIFT_BYTES omitted
        add outPtr, outPtr, pad, LSL #(COMPV_GAS_INT16_SHIFT_BYTES)
        bne LoopHeight_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON64
    EndOf_LoopHeight_CompVMathConvlt1VtHz_8u16s16s_Asm_NEON64:

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
    .unreq coeffw
    .unreq memPtr

    .unreq vecSum0
    .unreq vecSum1
    .unreq vecSum2
    .unreq vecSum3
    .unreq vecCoeff
    .unreq vec0
    .unreq vec1
    .unreq vec2
    .unreq vec3
    .unreq vec4
    .unreq vec5

    COMPV_GAS_MEMFREE (16*COMPV_GAS_INT16_SZ_BYTES)
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN



#########################################################################
# arg(0) -> const int16_t* inPtr
# arg(1) -> int16_t* outPtr
# arg(2) -> compv_uscalar_t width
# arg(3) -> compv_uscalar_t height
# arg(4) -> compv_uscalar_t step
# arg(5) -> compv_uscalar_t pad
# arg(6) -> const int16_t* vthzKernPtr
# arg(7) -> compv_uscalar_t kernSize
COMPV_GAS_FUNCTION_DECLARE CompVMathConvlt1VtHz_16s16s16s_Asm_NEON64
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS
	COMPV_GAS_MEMALLOC (16*COMPV_GAS_INT16_SZ_BYTES)

	## Set arguments ##
	inPtr .req r0
	outPtr .req r1
	width .req r2
	height .req r3
	step .req r4
	pad .req r5
	vthzKernPtr .req r6
	kernSize .req r7

    ## Local variables ##
    .equ mem		, 0
    i .req r8
    row .req r9
    inPtrPlusI .req r10
    coeff .req r11
    coeffw .req r11w
    memPtr .req r12

    vecSum0 .req v0
    vecSum1 .req v1
    vecSum2 .req v2
    vecSum3 .req v3
    vecCoeff .req v4
    vec0 .req v5
    vec1 .req v6

    # Change step in bytes unit (for inPtrPlus)
    lsl step, step, #COMPV_GAS_INT16_SHIFT_BYTES

    # Change kernSize in bytes (for vthzKernPtr)
    lsl kernSize, kernSize, #COMPV_GAS_INT16_SHIFT_BYTES

    # Change pad in bytes (for inPtr and outPtr)
    lsl pad, pad, #COMPV_GAS_INT16_SHIFT_BYTES
	
    ###########################################################
    # for (j = 0; j < height; ++j)
    ###########################################################
    LoopHeight_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON64:
        ###########################################################
        # for (i = 0; i < width16; i += 16)
        ###########################################################
        mov i, width
        LoopWidth_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON64:
            eor vecSum0.16b, vecSum0.16b, vecSum0.16b
            eor vecSum1.16b, vecSum1.16b, vecSum1.16b
            eor vecSum2.16b, vecSum2.16b, vecSum2.16b
            eor vecSum3.16b, vecSum3.16b, vecSum3.16b
            ###########################################################
            # for (row = 0, k = 0; row < kernSize; ++row, k += step)
            ###########################################################
            mov inPtrPlusI, inPtr
            mov row, #0
            LoopKernel_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON64:
                ld1 {vec0.8h,vec1.8h}, [inPtrPlusI], step
                ldrh coeffw, [vthzKernPtr, row]
                add row, row, #COMPV_GAS_INT16_SZ_BYTES
                dup vecCoeff.8h, coeffw
                smlal vecSum0.4s, vec0.4h, vecCoeff.4h
                smlal2 vecSum1.4s, vec0.8h, vecCoeff.8h
                smlal vecSum2.4s, vec1.4h, vecCoeff.4h
                smlal2 vecSum3.4s, vec1.8h, vecCoeff.8h
                cmp row, kernSize
                blt LoopKernel_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON64
            EndOf_LoopKernel_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON64:
            
            sqxtn vec0.4h, vecSum0.4s
            sqxtn2 vec0.8h, vecSum1.4s
            sqxtn vec1.4h, vecSum2.4s
            sqxtn2 vec1.8h, vecSum3.4s
            subs i, i, #16
            bmi MoreThanWidth16_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON64

            ## if (i < width16) ##
			LessThanWidth16_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON64:
				st1 {vec0.8h,vec1.8h}, [outPtr], #(16*COMPV_GAS_INT16_SZ_BYTES)
                add inPtr, inPtr, #(16*COMPV_GAS_INT16_SZ_BYTES)
				b EndOf_MoreThanWidth16_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON64
			EndOf_LessThanWidth16_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON64:

			## if (i >= width16) ##
			MoreThanWidth16_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON64:
                add memPtr, sp, #mem
				st1 {vec0.8h,vec1.8h}, [memPtr]
				## for (; i < width; ++i, ++k) ##
                add i, i, #16 // was negative and now contains '(width - (width & -16))'
                add inPtr, inPtr, i, LSL #(COMPV_GAS_INT16_SHIFT_BYTES)
				LoopMoreThanWidth16_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON64:
                    ldrh coeffw, [memPtr], #(1*COMPV_GAS_INT16_SZ_BYTES) // read int16_t into coeffw and increment
                    subs i, i, #1
                    strh coeffw, [outPtr], #(1*COMPV_GAS_INT16_SZ_BYTES) // write int16_t from coeffw and increment
					bne LoopMoreThanWidth16_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON64
				EndOf_LoopMoreThanWidth16_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON64:
                b EndOf_LoopWidth_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON64
			EndOf_MoreThanWidth16_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON64:
            
            bgt LoopWidth_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON64 // branch for far above 'subs i, i, #16' instruction
        EndOf_LoopWidth_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON64:

        subs height, height, #1
        add inPtr, inPtr, pad // ,LSL #(COMPV_GAS_INT16_SHIFT_BYTES) omitted
        add outPtr, outPtr, pad // ,LSL #(COMPV_GAS_INT16_SHIFT_BYTES) omitted
        bne LoopHeight_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON64
    EndOf_LoopHeight_CompVMathConvlt1VtHz_16s16s16s_Asm_NEON64:

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
    .unreq coeffw
    .unreq memPtr

    .unreq vecSum0
    .unreq vecSum1
    .unreq vecSum2
    .unreq vecSum3
    .unreq vecCoeff
    .unreq vec0
    .unreq vec1

    COMPV_GAS_MEMFREE (16*COMPV_GAS_INT16_SZ_BYTES)
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__aarch64__) */

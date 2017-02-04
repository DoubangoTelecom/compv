#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__arm__) && !defined(__aarch64__)
.include "compv_common_arm32.S"

#if defined(__APPLE__)
#   define sym(funcname) _##funcname
#else
#   define sym(funcname) funcname
#endif

.data

.extern
 
.text

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* inPtr
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

    @ Change kernelSize to kernSizeInBytes = kernSize * sizeof(float32)
    kernSizeInBytes .req kernSize
    mov kernSizeInBytes, kernSize, LSL #2
    .unreq kernSize

    i .req r8
    row .req r9
    inPtr_ .req r10
    vthzKernPtr_ .req r11

    vecSum0 .req q0
    #define vecSum0x q0x
    vecSum1 .req q1
    vecSum2 .req q2
    vecSum3 .req q3
    vecCoeff .req q4
    #define vecCoeffx q4x
    #define vec0 q5
    #define vec0x q5x
    #define vec0y q5y
    #define vec1 q6
    #define vec1x q6x
    #define vec1y q6y
    #define vec2 q7
    #define vec2x q7x
    #define vec2y q7y
    #define vec3 q8
    #define vec3x q8x
    #define vec3y q8y

    .equ SizeOfFloat32InBytes, 4
	
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (j = 0; j < height; ++j)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    LoopHeight_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32:
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (i = 0; i < width - 15; i += 16)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        and i, width, #-16 @ Align backward
        LoopWidthPer16Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32:
            veor.f32 vecSum0, vecSum0, vecSum0
            veor.f32 vecSum1, vecSum1, vecSum1
            veor.f32 vecSum2, vecSum2, vecSum2
            veor.f32 vecSum3, vecSum3, vecSum3
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            @ for (row = 0, k = 0; row < kernSize; ++row, k += step)
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            mov row, #0
            mov inPtr_, inPtr
            LoopKernelPer16Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32:
                vld1.u8 { q15 }, [inPtr_], step @ q15 = vecInPtr
                ldr vthzKernPtr_, [vthzKernPtr, row]
                add row, row, #SizeOfFloat32InBytes
                cmp row, kernSizeInBytes
                vmov.f32 vecCoeffx[0], vthzKernPtr_

                vmovl.u8 vec2, q15x
                vmovl.u8 vec3, q15y

                vmovl.u16 vec0, vec2x
				vmovl.u16 vec1, vec2y
				vmovl.u16 vec2, vec3x
				vmovl.u16 vec3, vec3y

                vcvt.f32.u32 vec0, vec0
                vcvt.f32.u32 vec1, vec1
                vcvt.f32.u32 vec2, vec2
                vcvt.f32.u32 vec3, vec3

                vmla.f32 vecSum0, vec0, vecCoeffx[0]
                vmla.f32 vecSum1, vec1, vecCoeffx[0]
                vmla.f32 vecSum2, vec2, vecCoeffx[0]
                vmla.f32 vecSum3, vec3, vecCoeffx[0]
                
                blt LoopKernelPer16Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32
                @@ EndOf_LoopKernelPer16Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32 @@

            vcvt.u32.f32 vec0, vecSum0
            vcvt.u32.f32 vec1, vecSum1
            vcvt.u32.f32 vec2, vecSum2
            vcvt.u32.f32 vec3, vecSum3

            vmovn.u32 q13x, vec0
            vmovn.u32 q13y, vec1
            vmovn.u32 q14x, vec2
            vmovn.u32 q14y, vec3
            vmovn.u16 q15x, q13
            vmovn.u16 q15y, q14
            vst1.u8 q15, [outPtr]!

            add inPtr, inPtr, #16
            subs i, i, #16
            bne LoopWidthPer16Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32
            @@ EndOf_LoopWidthPer16Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32 @@


            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            @ for (; i < width - 3; i += 4)
            @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            and i, width, #15 @  % 16
            lsrs i, i, #2 @ / 4
            beq EndOf_LoopWidthPer4Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32
            LoopWidthPer4Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32:
                veor.f32 vecSum0, vecSum0, vecSum0
                @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                @ for (row = 0, k = 0; row < kernSize; ++row, k += step)
                @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                mov row, #0
                mov inPtr_, inPtr
                LoopKernelPer4Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32:
                    vld1.u8 { q15x }, [inPtr_], step @ q15x = vecInPtrn
                    ldr vthzKernPtr_, [vthzKernPtr, row]
                    add row, row, #SizeOfFloat32InBytes
                    cmp row, kernSizeInBytes
                    vmov.f32 vecCoeffx[0], vthzKernPtr_

                    vmovl.u8 vec0, q15x
                    vmovl.u16 vec1, vec0x
                    vcvt.f32.u32 vec2, vec1
                    vmla.f32 vecSum0, vec2, vecCoeffx[0]
                    
                    blt LoopKernelPer4Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32
                    @@ EndOf_LoopKernelPer4Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32 @@

                vcvt.u32.f32 vec0, vecSum0
                vmovn.u32 q13x, vec0
                vmovn.u16 q14x, q13

                @ do not use str (outPtr not #4 bytes aligned)
                vmov.u8 row, q14x[0]
                vmov.u8 inPtr_, q14x[1]
                vmov.u8 vthzKernPtr_, q14x[2]
                strb row, [outPtr, #0]
                vmov.u8 row, q14x[3]
                strb inPtr_, [outPtr, #1]
                strb vthzKernPtr_, [outPtr, #2]
                strb row, [outPtr, #3]

                add inPtr, inPtr, #4
                add outPtr, outPtr, #4
                subs i, i, #1
                bne LoopWidthPer4Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32
                EndOf_LoopWidthPer4Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32:
                @@ EndOf_LoopWidthPer4Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32 @@


                @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                @ for (; i < width; i += 1)
                @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                ands i, width, #3 @  modulo 4
                beq EndOf_LoopWidthPer1Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32
                LoopWidthPer1Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32:
                    veor.f32 vecSum0x, vecSum0x, vecSum0x
                    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                    @ for (row = 0, k = 0; row < kernSize; ++row, k += step)
                    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                    mov row, #0
                    mov inPtr_, inPtr
                    LoopKernelPer1Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32:
                        vld1.u8 { q15x }, [inPtr_], step @ q15x = vecInPtrn
                        ldr vthzKernPtr_, [vthzKernPtr, row]
                        vmov.f32 vecCoeffx[0], vthzKernPtr_

                        vmovl.u8 vec0, q15x
                        vmovl.u16 vec1, vec0x
                        vmla.f32 vecSum0x, vec1x, vecCoeffx[0]

                        add row, row, #SizeOfFloat32InBytes
                        cmp row, kernSizeInBytes
                        blt LoopKernelPer1Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32
                        @@ EndOf_LoopKernelPer1Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32 @@

                    vcvt.u32.f32 vec0, vecSum0
                    vmov.u8 row, vec0x[0]
                    strb row, [outPtr, #0]

                    add inPtr, inPtr, #1
                    add outPtr, outPtr, #1
                    subs i, i, #1
                    bne LoopWidthPer1Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32
                    EndOf_LoopWidthPer1Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32:
                    @@ EndOf_LoopWidthPer1Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32 @@

        add inPtr, inPtr, pad
        add outPtr, outPtr, pad
        subs height, height, #1
		bne LoopHeight_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32
        @@ EndOf_LoopHeight_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32 @@



    .unreq inPtr
	.unreq outPtr
	.unreq width
	.unreq height
	.unreq step
	.unreq pad
	.unreq vthzKernPtr
	.unreq kernSize

    .unreq kernSizeInBytes

    .unreq i
    .unreq row
    .unreq inPtr_
    .unreq vthzKernPtr_

    .unreq vecSum0
    #undef vecSum0x
    .unreq vecSum1
    .unreq vecSum2
    .unreq vecSum3
    .unreq vecCoeff
    #undef vecCoeffx
    #undef vec0
    #undef vec0x
    #undef vec0y
    #undef vec1
    #undef vec1x
    #undef vec1y
    #undef vec2
    #undef vec2x
    #undef vec2y
    #undef vec3
    #undef vec3x
    #undef vec3y

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 8
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN



#endif /* defined(__arm__) && !defined(__aarch64__) */

#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__aarch64__)
.include "compv_common_arm64.S"

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
# arg(6) -> const compv_float32_t* vthzKernPtr
# arg(7) -> compv_uscalar_t kernSize
.macro CompVMathConvlt1VtHz_8u32f8u_Macro_NEON64 fusedMultiplyAdd
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SAVE_NEON_REGS

	## Set arguments ##
	inPtr .req r0
	outPtr .req r1
	width .req r2
	height .req r3
	step .req r4
	pad .req r5
	vthzKernPtr .req r6
	kernSize .req r7

    i .req r8
    row .req r9
    inPtr_ .req r10
    coeffs .req r11

    #define vecSum0     v0
    #define vecSum1     v1
    #define vecSum2     v2
    #define vecSum3     v3
    #define vecCoeff    v4
    #define vec0        v5
    #define vec1        v6
    #define vec2        v7
    #define vec3        v8

    .equ SizeOfFloat32InBytes, 4
	
    ###########################################################
    # for (j = 0; j < height; ++j)
    ###########################################################
    LoopHeight_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@:
        ###########################################################
        # for (i = 0; i < width - 15; i += 16)
        ###########################################################
        and i, width, #-16 // Align backward (FIXME: use bic)
        LoopWidthPer16Samples_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@:
            movi vecSum0.4s, #0
            movi vecSum1.4s, #0
            movi vecSum2.4s, #0
            movi vecSum3.4s, #0
            ###########################################################
            # for (row = 0, k = 0; row < kernSize; ++row, k += step)
            ###########################################################
            mov row, kernSize
            mov inPtr_, inPtr
            mov coeffs, vthzKernPtr
            LoopKernelPer16Samples_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@:
                ldr q15, [inPtr_] // v15 = vecInPtr
				ld1r {vecCoeff.4s}, [coeffs], #SizeOfFloat32InBytes
				add inPtr_, inPtr_, step
                uxtl vec2.8h, v15.8b
                uxtl2 vec3.8h, v15.16b
                uxtl vec0.4s, vec2.4h
				uxtl2 vec1.4s, vec2.8h
				uxtl vec2.4s, vec3.4h
				uxtl2 vec3.4s, vec3.8h
                ucvtf v16.4s, vec0.4s
                ucvtf v17.4s, vec1.4s
                ucvtf v18.4s, vec2.4s
                ucvtf v19.4s, vec3.4s
                .if \fusedMultiplyAdd
                    fmla vecSum0.4s, v16.4s, vecCoeff.s[0]
                    fmla vecSum1.4s, v17.4s, vecCoeff.s[0]
                    fmla vecSum2.4s, v18.4s, vecCoeff.s[0]
                    fmla vecSum3.4s, v19.4s, vecCoeff.s[0]
                .else
                    fmul v20.4s, v16.4s, vecCoeff.s[0]
                    fmul v21.4s, v17.4s, vecCoeff.s[0]
                    fmul v22.4s, v18.4s, vecCoeff.s[0]
                    fmul v23.4s, v19.4s, vecCoeff.s[0]
                    fadd vecSum0.4s, vecSum0.4s, v20.4s
                    fadd vecSum1.4s, vecSum1.4s, v21.4s
                    fadd vecSum2.4s, vecSum2.4s, v22.4s
                    fadd vecSum3.4s, vecSum3.4s, v23.4s
                .endif
                subs row, row, #1
                bne LoopKernelPer16Samples_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@
                ## EndOf_LoopKernelPer16Samples_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64 ##
            
            fcvtzu vec0.4s, vecSum0.4s
            fcvtzu vec1.4s, vecSum1.4s
            fcvtzu vec2.4s, vecSum2.4s
            fcvtzu vec3.4s, vecSum3.4s

            xtn v13.4h, vec0.4s
            xtn v14.4h, vec2.4s
            xtn2 v13.8h, vec1.4s
            xtn2 v14.8h, vec3.4s
            xtn v15.8b, v13.8h
            xtn2 v15.16b, v14.8h            
            st1 {v15.16b}, [outPtr], #16
			subs i, i, #16
            add inPtr, inPtr, #16
            bne LoopWidthPer16Samples_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@
            ## EndOf_LoopWidthPer16Samples_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64 ##


            ###########################################################
            # for (; i < width - 3; i += 4)
            ###########################################################
            and i, width, #15 //  % 16
            lsr i, i, #2 // div 4
            cbz i, EndOf_LoopWidthPer4Samples_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@
            LoopWidthPer4Samples_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@:
                movi vecSum0.4s, #0
                ###########################################################
                # for (row = 0, k = 0; row < kernSize; ++row, k += step)
                ###########################################################
                mov row, kernSize
                mov inPtr_, inPtr
                mov coeffs, vthzKernPtr
                LoopKernelPer4Samples_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@:
                    ldr q15, [inPtr_] // v15 = vecInPtr
					ld1r {vecCoeff.4s}, [coeffs], #SizeOfFloat32InBytes
					add inPtr_, inPtr_, step
                    uxtl vec0.8h, v15.8b
                    uxtl vec1.4s, vec0.4h
                    ucvtf vec2.4s, vec1.4s 
                    .if \fusedMultiplyAdd
                        fmla vecSum0.4s, vec2.4s, vecCoeff.s[0]
                    .else
                        fmul vec3.4s, vec2.4s, vecCoeff.s[0]
                        fadd vecSum0.4s, vecSum0.4s, vec3.4s
                    .endif
                    sub row, row, #1
                    cbnz row, LoopKernelPer4Samples_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@
                    ## EndOf_LoopKernelPer4Samples_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64 ##

                fcvtzu vec0.4s, vecSum0.4s
                xtn v13.4h, vec0.4s
                xtn v14.8b, v13.8h

                # do not use str (outPtr not #4 bytes aligned)
				st1  { v14.s }[0], [outPtr], #4
                
				subs i, i, #1
                add inPtr, inPtr, #4
                bne LoopWidthPer4Samples_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@
                EndOf_LoopWidthPer4Samples_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@:
                ## EndOf_LoopWidthPer4Samples_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64 ##


                ###########################################################
                # for (; i < width; i += 1)
                ###########################################################
                ands i, width, #3 //  modulo 4
                beq EndOf_LoopWidthPer1Samples_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@
                LoopWidthPer1Samples_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@:
                    movi vecSum0.4s, #0
                    ###########################################################
                    # for (row = 0, k = 0; row < kernSize; ++row, k += step)
                    ###########################################################
                    mov row, kernSize
                    mov inPtr_, inPtr
                    mov coeffs, vthzKernPtr
                    LoopKernelPer1Samples_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@:
                        ldr q15, [inPtr_] // v15 = vecInPtr
						ld1r {vecCoeff.4s}, [coeffs], #SizeOfFloat32InBytes
						add inPtr_, inPtr_, step
						uxtl vec0.8h, v15.8b
						uxtl vec1.4s, vec0.4h
						ucvtf vec2.4s, vec1.4s 
						.if \fusedMultiplyAdd
							fmla vecSum0.4s, vec2.4s, vecCoeff.s[0]
						.else
							fmul vec3.4s, vec2.4s, vecCoeff.s[0]
							fadd vecSum0.4s, vecSum0.4s, vec3.4s
						.endif
						sub row, row, #1
                        cbnz row, LoopKernelPer1Samples_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@
                        ## EndOf_LoopKernelPer1Samples_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64 ##

					fcvtzu vec0.4s, vecSum0.4s
					xtn v13.4h, vec0.4s
					xtn v14.8b, v13.8h
					add inPtr, inPtr, i
					.set counter, 0
					.rept 4
						st1  { v14.b }[counter], [outPtr], #1
						.set counter, counter+1
						subs i, i, #1
						beq EndOf_LoopWidthPer1Samples_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@
					.endr
                    EndOf_LoopWidthPer1Samples_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@:
                    ## EndOf_LoopWidthPer1Samples_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64 ##

        subs height, height, #1
        add inPtr, inPtr, pad
        add outPtr, outPtr, pad
		bne LoopHeight_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@
        ## EndOf_LoopHeight_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64 ##



    .unreq inPtr
	.unreq outPtr
	.unreq width
	.unreq height
	.unreq step
	.unreq pad
	.unreq vthzKernPtr
	.unreq kernSize

    .unreq i
    .unreq row
    .unreq inPtr_
    .unreq coeffs

    #undef vecSum0     
    #undef vecSum1     
    #undef vecSum2     
    #undef vecSum3     
    #undef vecCoeff    
    #undef vec0        
    #undef vec1        
    #undef vec2        
    #undef vec3       

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64
    CompVMathConvlt1VtHz_8u32f8u_Macro_NEON64 0

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVMathConvlt1VtHz_8u32f8u_Asm_FMA_NEON64
    CompVMathConvlt1VtHz_8u32f8u_Macro_NEON64 1

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

	## Set arguments ##
	inPtr .req r0
	outPtr .req r1
	width .req r2
	height .req r3
	step .req r4
	pad .req r5
	vthzKernPtr .req r6
	kernSize .req r7

    i .req r8
    row .req r9
    inPtr_ .req r10
    coeffs .req r11

    #define vecSum0     v0
    #define vecSum1     v1
    #define vecSum2     v2
    #define vecSum3     v3
    #define vecCoeff    v4
    #define vec0        v5
    #define vec1        v6
    #define vec2        v7
    #define vec3        v8

    .equ SizeOfUint16InBytes, 2
	
    ###########################################################
    # for (j = 0; j < height; ++j)
    ###########################################################
    LoopHeight_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64:
        ###########################################################
        # for (i = 0; i < width - 15; i += 16)
        ###########################################################
        and i, width, #-16 // Align backward (FIXME: use bic)
        LoopWidthPer16Samples_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64:
            ###########################################################
            # for (row = 0, k = 0; row < kernSize; ++row, k += step)
            ###########################################################
            sub row, kernSize, #1
            add inPtr_, inPtr, step
            add coeffs, vthzKernPtr, #SizeOfUint16InBytes
            ldr q15, [inPtr] // v15 = vecInPtr
            ld1r {vecCoeff.8h}, [vthzKernPtr]
            uxtl v13.8h, v15.8b
            uxtl2 v14.8h, v15.16b
            umull vec0.4s, v13.4h, vecCoeff.4h
            umull2 vec1.4s, v13.8h, vecCoeff.8h
            umull vec2.4s, v14.4h, vecCoeff.4h
            umull2 vec3.4s, v14.8h, vecCoeff.8h
            shrn vecSum0.4h, vec0.4s, #16
            shrn vecSum1.4h, vec2.4s, #16
            shrn2 vecSum0.8h, vec1.4s, #16                
            shrn2 vecSum1.8h, vec3.4s, #16
            LoopKernelPer16Samples_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64:
                ldr q15, [inPtr_] // v15 = vecInPtr
				ld1r {vecCoeff.8h}, [coeffs], #SizeOfUint16InBytes
				add inPtr_, inPtr_, step
                uxtl v13.8h, v15.8b
                uxtl2 v14.8h, v15.16b
                umull vec0.4s, v13.4h, vecCoeff.4h
                umull2 vec1.4s, v13.8h, vecCoeff.8h
                umull vec2.4s, v14.4h, vecCoeff.4h
                umull2 vec3.4s, v14.8h, vecCoeff.8h
                shrn v16.4h, vec0.4s, #16
                shrn v17.4h, vec2.4s, #16
                shrn2 v16.8h, vec1.4s, #16                
                shrn2 v17.8h, vec3.4s, #16
                uqadd vecSum0.8h, vecSum0.8h, v16.8h
                uqadd vecSum1.8h, vecSum1.8h, v17.8h             
                subs row, row, #1
                bne LoopKernelPer16Samples_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64
                ## EndOf_LoopKernelPer16Samples_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64 ##
            
            xtn v15.8b, vecSum0.8h
            xtn2 v15.16b, vecSum1.8h           
            st1 {v15.16b}, [outPtr], #16
			subs i, i, #16
            add inPtr, inPtr, #16
            bne LoopWidthPer16Samples_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64
            ## EndOf_LoopWidthPer16Samples_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64 ##


        ###########################################################
        # .if (i < width - 7)
        ###########################################################
        and i, width, #15 //  modulo 16
        lsr i, i, 3 // div 8
        cbz i, EndOf_IfPer8Samples_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64
        IfPer8Samples_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64:
            ###########################################################
            # for (row = 0, k = 0; row < kernSize; ++row, k += step)
            ###########################################################
            sub row, kernSize, #1
            add inPtr_, inPtr, step
            add coeffs, vthzKernPtr, #SizeOfUint16InBytes
            ld1 {v15.8b}, [inPtr] // v15 = vecInPtr
            ld1r {vecCoeff.8h}, [vthzKernPtr]
            uxtl v13.8h, v15.8b
            umull vec0.4s, v13.4h, vecCoeff.4h
            umull2 vec1.4s, v13.8h, vecCoeff.8h
            shrn vecSum0.4h, vec0.4s, #16
            shrn2 vecSum0.8h, vec1.4s, #16                
            LoopKernelPer8Samples_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64:
                ld1 {v15.8b}, [inPtr_] // v15 = vecInPtr
				ld1r {vecCoeff.8h}, [coeffs], #SizeOfUint16InBytes
				add inPtr_, inPtr_, step
                uxtl v13.8h, v15.8b
                umull vec0.4s, v13.4h, vecCoeff.4h
                umull2 vec1.4s, v13.8h, vecCoeff.8h
                shrn v16.4h, vec0.4s, #16
                shrn2 v16.8h, vec1.4s, #16                
                uqadd vecSum0.8h, vecSum0.8h, v16.8h
                subs row, row, #1
                bne LoopKernelPer8Samples_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64
                ## EndOf_LoopKernelPer8Samples_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64 ##
            
            xtn v15.8b, vecSum0.8h
            st1 {v15.8b}, [outPtr], #8
            add inPtr, inPtr, #8
            EndOf_IfPer8Samples_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64:
            ## EndOf_IfPer8Samples_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64 ##


        ###########################################################
        # .if (i < width - 3)
        ###########################################################
        and i, width, #7 //  modulo 8
        lsr i, i, 2 // div 4
        cbz i, EndOf_IfPer4Samples_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64
        IfPer4Samples_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64:
            ###########################################################
            # for (row = 0, k = 0; row < kernSize; ++row, k += step)
            ###########################################################
            sub row, kernSize, #1
            add inPtr_, inPtr, step
            add coeffs, vthzKernPtr, #SizeOfUint16InBytes
            ld1 {v15.8b}, [inPtr] // v15 = vecInPtr
            ld1r {vecCoeff.8h}, [vthzKernPtr]
            uxtl v13.8h, v15.8b
            umull vec0.4s, v13.4h, vecCoeff.4h
            shrn vecSum0.4h, vec0.4s, #16
            LoopKernelPer4Samples_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64:
                ld1 {v15.8b}, [inPtr_] // v15 = vecInPtr
				ld1r {vecCoeff.4h}, [coeffs], #SizeOfUint16InBytes
				add inPtr_, inPtr_, step
                uxtl v13.8h, v15.8b
                umull vec0.4s, v13.4h, vecCoeff.4h
                shrn v16.4h, vec0.4s, #16
                uqadd vecSum0.4h, vecSum0.4h, v16.4h
                subs row, row, #1
                bne LoopKernelPer4Samples_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64
                ## EndOf_LoopKernelPer4Samples_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64 ##
            
            xtn v15.8b, vecSum0.8h
            # do not use str (outPtr not #4 bytes aligned)
			st1 { v15.s }[0], [outPtr], #4
            add inPtr, inPtr, #4
            EndOf_IfPer4Samples_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64:
            ## EndOf_IfPer4Samples_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64 ##


        ###########################################################
        # .if (i < width - 3)
        ###########################################################
        ands i, width, #3 //  modulo 4
        beq EndOf_IfPer1Samples_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64
        IfPer1Samples_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64:
            ###########################################################
            # for (row = 0, k = 0; row < kernSize; ++row, k += step)
            ###########################################################
            sub row, kernSize, #1
            add inPtr_, inPtr, step
            add coeffs, vthzKernPtr, #SizeOfUint16InBytes
            ld1 {v15.8b}, [inPtr] // v15 = vecInPtr
            ld1r {vecCoeff.8h}, [vthzKernPtr]
            uxtl v13.8h, v15.8b
            umull vec0.4s, v13.4h, vecCoeff.4h
            shrn vecSum0.4h, vec0.4s, #16
            LoopKernelPer1Samples_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64:
                ld1 {v15.8b}, [inPtr_] // v15 = vecInPtr
				ld1r {vecCoeff.4h}, [coeffs], #SizeOfUint16InBytes
				add inPtr_, inPtr_, step
                uxtl v13.8h, v15.8b
                umull vec0.4s, v13.4h, vecCoeff.4h
                shrn v16.4h, vec0.4s, #16
                uqadd vecSum0.4h, vecSum0.4h, v16.4h
                subs row, row, #1   
                bne LoopKernelPer1Samples_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64
                ## EndOf_LoopKernelPer1Samples_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64 ##
            
            xtn v15.8b, vecSum0.8h
            add inPtr, inPtr, i
            .set counter, 0
            .rept 4
                st1 { v15.b }[counter], [outPtr], #1
                .set counter, counter+1
                subs i, i, #1
                beq EndOf_IfPer1Samples_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64
            .endr
            EndOf_IfPer1Samples_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64:
            ## EndOf_IfPer1Samples_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64 ##

        subs height, height, #1
        add inPtr, inPtr, pad
        add outPtr, outPtr, pad
		bne LoopHeight_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64
        ## EndOf_LoopHeight_CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64 ##


    .unreq inPtr
	.unreq outPtr
	.unreq width
	.unreq height
	.unreq step
	.unreq pad
	.unreq vthzKernPtr
	.unreq kernSize

    .unreq i
    .unreq row
    .unreq inPtr_
    .unreq coeffs

    #undef vecSum0     
    #undef vecSum1     
    #undef vecSum2     
    #undef vecSum3     
    #undef vecCoeff    
    #undef vec0        
    #undef vec1        
    #undef vec2        
    #undef vec3       

	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__aarch64__) */

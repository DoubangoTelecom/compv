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
# arg(0) -> COMPV_ALIGNED(NEON) const uint8_t* inPtr
# arg(1) -> uint8_t* outPtr
# arg(2) -> compv_uscalar_t width
# arg(3) -> compv_uscalar_t height
# arg(4) -> compv_uscalar_t step
# arg(5) -> compv_uscalar_t pad
# arg(6) -> const compv_float32_t* vthzKernPtr
# arg(7) -> compv_uscalar_t kernSize
.macro CompVMathConvlt1VtHz_8u32f8u_Macro_NEON64 fusedMultiplyAdd
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 8
	COMPV_GAS_SAVE_NEON_REGS

	## Load arguments ##
    ldp_arg 0, r0, r1
    ldp_arg 2, r2, r3
    ldp_arg 4, r4, r5
    ldp_arg 6, r6, r7
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
    #define coeffsw r11w

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
        LoopWidthPer16Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@:
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
            LoopKernelPer16Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@:
                ld1 { v15.16b }, [inPtr_], step // v15 = vecInPtr
                ld1.s {vecCoeff}[0], [coeffs], #SizeOfFloat32InBytes
                
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
                bne LoopKernelPer16Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@
                ## EndOf_LoopKernelPer16Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64 ##
            
            fcvtzu vec0.4s, vecSum0.4s
            fcvtzu vec1.4s, vecSum1.4s
            fcvtzu vec2.4s, vecSum2.4s
            fcvtzu vec3.4s, vecSum3.4s

            xtn v13.4h, vec0.4s
            xtn2 v13.8h, vec1.4s
            xtn v14.4h, vec2.4s
            xtn2 v14.8h, vec3.4s
            xtn v15.8b, v13.8h
            xtn2 v15.16b, v14.8h            
            st1 {v15.16b}, [outPtr], #16
            add inPtr, inPtr, #16
            subs i, i, #16
            bne LoopWidthPer16Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@
            ## EndOf_LoopWidthPer16Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64 ##


            ###########################################################
            # for (; i < width - 3; i += 4)
            ###########################################################
            and i, width, #15 //  % 16
            lsr i, i, #2 // div 4
            cbz i, EndOf_LoopWidthPer4Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@
            LoopWidthPer4Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@:
                movi vecSum0.4s, #0
                ###########################################################
                # for (row = 0, k = 0; row < kernSize; ++row, k += step)
                ###########################################################
                mov row, kernSize
                mov inPtr_, inPtr
                mov coeffs, vthzKernPtr
                LoopKernelPer4Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@:
                    ld1 { v15.8b }, [inPtr_], step // v15[0] = vecInPtrn
                    ld1.s {vecCoeff}[0], [coeffs], #SizeOfFloat32InBytes
                    uxtl vec0.8h, v15.8b
                    uxtl vec1.4s, vec0.4h
                    ucvtf vec2.4s, vec1.4s 
                    .if \fusedMultiplyAdd
                        fmla vecSum0.4s, vec2.4s, vecCoeff.s[0]
                    .else
                        fmul vec3.4s, vec2.4s, vecCoeff.s[0]
                        fadd vecSum0.4s, vecSum0.4s, vec3.4s
                    .endif
                    subs row, row, #1
                    bne LoopKernelPer4Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@
                    ## EndOf_LoopKernelPer4Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64 ##

                fcvtzu vec0.4s, vecSum0.4s
                xtn v13.4h, vec0.4s
                xtn v14.8b, v13.8h

                # do not use str (outPtr not #4 bytes aligned)
                st1.b  { v14 }[0], [outPtr], #1
                st1.b  { v14 }[1], [outPtr], #1
                st1.b  { v14 }[2], [outPtr], #1
                st1.b  { v14 }[3], [outPtr], #1
                
                add inPtr, inPtr, #4
                subs i, i, #1
                bne LoopWidthPer4Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@
                EndOf_LoopWidthPer4Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@:
                ## EndOf_LoopWidthPer4Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64 ##


                ###########################################################
                # for (; i < width; i += 1)
                ###########################################################
                and i, width, #3 //  modulo 4
                cbz i, EndOf_LoopWidthPer1Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@
                LoopWidthPer1Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@:
                    movi vecSum0.8b, #0
                    ###########################################################
                    # for (row = 0, k = 0; row < kernSize; ++row, k += step)
                    ###########################################################
                    mov row, kernSize
                    mov inPtr_, inPtr
                    mov coeffs, vthzKernPtr
                    LoopKernelPer1Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@:
                        ld1 { v15.8b }, [inPtr_], step // v15[0] = vecInPtrn
                        ld1.s {vecCoeff}[0], [coeffs], #SizeOfFloat32InBytes
                        uxtl vec0.8h, v15.8b
                        uxtl vec1.4s, vec0.4h
                        ucvtf vec2.4s, vec1.4s
                        .if \fusedMultiplyAdd
                            fmla vecSum0.2s, vec2.2s, vecCoeff.s[0]
                        .else
                            fmul vec3.2s, vec2.2s, vecCoeff.s[0]
                            fadd vecSum0.2s, vecSum0.2s, vec3.2s
                        .endif
                        subs row, row, #1
                        bne LoopKernelPer1Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@
                        ## EndOf_LoopKernelPer1Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64 ##

                    fcvtzu vec0.2s, vecSum0.2s
                    st1.b  { vec0 }[0], [outPtr], #1
                    add inPtr, inPtr, #1
                    subs i, i, #1
                    bne LoopWidthPer1Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@
                    EndOf_LoopWidthPer1Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64\@:
                    ## EndOf_LoopWidthPer1Bytes_CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64 ##

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
    #undef coeffsw

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
	COMPV_GAS_UNSHADOW_ARGS 8
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN
.endm

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64
    CompVMathConvlt1VtHz_8u32f8u_Macro_NEON64 0

#########################################################################
COMPV_GAS_FUNCTION_DECLARE CompVMathConvlt1VtHz_8u32f8u_Asm_FMA_NEON64
    CompVMathConvlt1VtHz_8u32f8u_Macro_NEON64 1

#endif /* defined(__aarch64__) */

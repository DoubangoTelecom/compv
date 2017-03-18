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
@ arg(0) -> COMPV_ALIGNED(NEON) uint16_t* grad
@ arg(1) -> COMPV_ALIGNED(NEON) uint8_t* nms
@ arg(2) -> compv_uscalar_t width
@ arg(3) -> compv_uscalar_t height
@ arg(4) -> COMPV_ALIGNED(NEON) compv_uscalar_t stride
COMPV_GAS_FUNCTION_DECLARE CompVCannyNMSApply_Asm_NEON32
    COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 5
	COMPV_GAS_SAVE_NEON_REGS

	@@ Load arguments @@
	ldm_args r0-r4
	grad .req r0
	nms .req r1
	width .req r2
	height .req r3
	stride .req r4

    @ nmsTmp = r5
    @ col = r6
    @ vecZero = q15

    sub height, height, #1 @ row start at 1

    veor.u8 q15, q15, q15

    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ for (row_ = 1; row_ < height; ++row_)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    LoopHeight_CompVCannyNMSApply_Asm_NEON32:
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        @ for (col_ = 0; col_ < width; col_ += 8)
        @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        mov r6, #0
        mov r5, nms
        LoopWidth8_CompVCannyNMSApply_Asm_NEON32:
            @pld [r5, #(CACHE_LINE_SIZE*3)]
            vld1.u8 { q0x }, [r5 :64]!
            vcgt.u8 q0x, q0x, q15x
            vmov.u32 r10, q0x[0]
            vmov.u32 r11, q0x[1]
            orrs r10, r10, r11
            beq NothingToSupress_CompVCannyNMSApply_Asm_NEON32
                vmovl.u8 q0, q0x
                add r10, nms, r6
                add r11, grad, r6, LSL #1
                vld1.u16 { q1 }, [r11 :128]
                vsli.u16 q0, q0, #8
                vst1.u8 {q15x}, [r10 :64]
                vbic.u16 q1, q1, q0 @ supress
                vst1.u16 {q1}, [r11 :128]
                NothingToSupress_CompVCannyNMSApply_Asm_NEON32:

            add r6, r6, #8
            cmp r6, width
            blt LoopWidth8_CompVCannyNMSApply_Asm_NEON32
            @@ EndOf_CompVCannyNMSApply_Asm_NEON32 @@

        subs height, height, #1
        add grad, grad, stride, LSL #1
        add nms, nms, stride
        bne LoopHeight_CompVCannyNMSApply_Asm_NEON32
        @@ EndOf_CompVCannyNMSApply_Asm_NEON32 @@

	.unreq grad
    .unreq nms
    .unreq width
    .unreq height
    .unreq stride
	
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 5
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN




#endif /* defined(__arm__) && !defined(__aarch64__) */
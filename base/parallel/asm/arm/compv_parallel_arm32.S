#########################################################################
# Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__arm__) && !defined(__aarch64__)
.include "compv_common_arm32.S"

#if !defined(__APPLE__)
.arch armv7-a @ for LDREX/STREX
#endif

.data

.extern
 
.text

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Implementation based on http://infocenter.arm.com/help/topic/com.arm.doc.dht0008a/DHT0008A_arm_synchronization_primitives.pdf
.macro WAIT_FOR_UPDATE
    WFE
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Implementation based on http://infocenter.arm.com/help/topic/com.arm.doc.dht0008a/DHT0008A_arm_synchronization_primitives.pdf
.macro SIGNAL_UPDATE
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Implementation based on http://infocenter.arm.com/help/topic/com.arm.doc.dht0008a/DHT0008A_arm_synchronization_primitives.pdf
@ arg(0) -> COMPV_ALIGNED(4) void* semaphore
COMPV_GAS_FUNCTION_DECLARE CompVSemaphoreDec_Asm_ARMv7_ARM32
	CompVSemaphoreDec_Asm_ARMv7_ARM32_Label_1:
	LDREX   r1, [r0]
    CMP		r1, #0												@ Test if semaphore holds the value 0
    BEQ     CompVSemaphoreDec_Asm_ARMv7_ARM32_Label_2			@ If it does, block before retrying
    SUB     r1, #1												@ If not, decrement temporary copy
    STREX   r2, r1, [r0]										@ Attempt Store-Exclusive
    CMP     r2, #0												@ Check if Store-Exclusive succeeded
    BNE     CompVSemaphoreDec_Asm_ARMv7_ARM32_Label_1			@ If Store-Exclusive failed, retry from start
    DMB															@ Required before accessing protected resource
    COMPV_GAS_FUNCTION_RETURN
	
	CompVSemaphoreDec_Asm_ARMv7_ARM32_Label_2:					@ Take appropriate action while waiting for semaphore to be incremented
    WAIT_FOR_UPDATE												@ Wait for signal to retry
    B       CompVSemaphoreDec_Asm_ARMv7_ARM32_Label_1
    COMPV_GAS_FUNCTION_RETURN


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Implementation based on http://infocenter.arm.com/help/topic/com.arm.doc.dht0008a/DHT0008A_arm_synchronization_primitives.pdf
@ arg(0) -> COMPV_ALIGNED(4) void* semaphore
	COMPV_GAS_FUNCTION_DECLARE CompVSemaphoreInc_Asm_ARMv7_ARM32
	CompVSemaphoreInc_Asm_ARMv7_ARM32_Label_1:
	LDREX   r1, [r0]
    ADD     r1, #1												@ Increment temporary copy
    STREX   r2, r1, [r0]										@ Attempt Store-Exclusive
    CMP     r2, #0												@ Check if Store-Exclusive succeeded
    BNE     CompVSemaphoreInc_Asm_ARMv7_ARM32_Label_1			@ Store failed - retry immediately
    CMP     r0, #1												@ Store successful - test if incremented from zero
    DMB															@ Required before releasing protected resource
    BGE     CompVSemaphoreInc_Asm_ARMv7_ARM32_Label_2           @ If initial value was 0, signal update
    COMPV_GAS_FUNCTION_RETURN

	CompVSemaphoreInc_Asm_ARMv7_ARM32_Label_2:
    @ Signal waiting processors or processes
    SIGNAL_UPDATE
    COMPV_GAS_FUNCTION_RETURN

#endif /* defined(__arm__) && !defined(__aarch64__) */

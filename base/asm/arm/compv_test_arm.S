#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__arm__) || defined(__aarch64__)

#if defined(__aarch64__)
.include "compv_common_arm64.S"
#else
.include "compv_common_arm32.S"
#endif

.data

.extern k16_i16

.text

# BreakPoint BKPT

#if defined(__aarch64__)

#########################################################################
// r0 -> arg0
// r1 -> arg1
// r2 -> arg2
// r3 -> arg3
// r4 -> arg4
// r5 -> arg5
// r6 -> arg6
// r7 -> arg7
// r8 -> arg8
// r9 -> arg9
// r10 -> arg10
COMPV_GAS_FUNCTION_DECLARE CompVTest_Asm_NEON64
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 11 // When there are less than 8 args (0-7) then, no need to shadow/unshadow args.
	COMPV_GAS_SAVE_NEON_REGS
	COMPV_GAS_ALIGN_STACK 16, r9
	COMPV_GAS_MEMALLOC 128

	// ldp_arg needed only if threre are less than 8 args (0-7)
    ldp_arg 0, r0, r1
    ldp_arg 2, r2, r3
    ldp_arg 4, r4, r5
    ldp_arg 6, r6, r7
    ldp_arg 8, r8, r9
    ldr_arg 10, r10
	//name0 .req r0
	//name1 .req r1
	//name2 .req r2
	//name3 .req r3
	//name4 .req r4
	//name5 .req r5
	//name6 .req r6
	//name7 .req r7
	// use .unreq to undef

	COMPV_GAS_MEMFREE 128
	COMPV_GAS_UNALIGN_STACK r9, r10 // *must not* use the same register
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#else

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ r0 -> arg0
@ r1 -> arg1
@ r2 -> arg2
@ r3 -> arg3
@ r4 -> arg4
@ r5 -> arg5
@ r6 -> arg6
@ r7 -> arg7
@ r8 -> arg8
@ r9 -> arg9
@ r10 -> arg10
COMPV_GAS_FUNCTION_DECLARE CompVTest_Asm_NEON32
	COMPV_GAS_FUNCTION_PROLOG
	COMPV_GAS_SHADOW_ARGS_TO_STACK 11 @ When there are less than 4 args (0-3) then, no need to shadow/unshadow args. Num params must match COMPV_GAS_UNSHADOW_ARGS
	COMPV_GAS_SAVE_NEON_REGS
	@COMPV_GAS_ALIGN_STACK 16, r11
	@COMPV_GAS_MEMALLOC 128

	@ ldm_args needed only if threre are less than 4 args (0-3)

	@@ OPTION 1 @@
	@ load all one by one: on demand
	@ldr_arg 0, r0
	@ldr_arg 1, r1
	@ldr_arg 2, r2
	@ldr_arg 3, r3
	@ldr_arg 4, r4
	@ldr_arg 5, r5
	@ldr_arg 6, r6
	@ldr_arg 7, r7

	@@ OPTION 2 @@
	@ [r0-r3] already contains the first 4 arguments
	@ load additional arguments
	@ldr_arg 4, r4
	@ldr_arg 5, r5
	@ldr_arg 6, r6
	@ldr_arg 7, r7

	@@ OPTION 2 @@
	@ Or load all arguments once (faster)
	ldm_args r0-r10 @ load arguments in [r0-r10]
	@name0 .req r0
	@name1 .req r1
	@name2 .req r2
	@name3 .req r3
	@name4 .req r4
	@name5 .req r5
	@name6 .req r6
	@name7 .req r7
	@ use .unreq to undef

	@COMPV_GAS_MEMFREE 128
	@COMPV_GAS_UNALIGN_STACK r11
	COMPV_GAS_RESTORE_NEON_REGS
	COMPV_GAS_UNSHADOW_ARGS 11
	COMPV_GAS_FUNCTION_EPILOG
	COMPV_GAS_FUNCTION_RETURN

#endif /* !defined(__aarch64__) */

#endif /* defined(__arm__) */

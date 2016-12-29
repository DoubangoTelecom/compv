@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   @
@ File author: Mamadou DIOP (Doubango Telecom, France).                 @
@ License: GPLv3. For commercial license please contact us.             @
@ Source code: https://github.com/DoubangoTelecom/compv                 @
@ WebSite: http://compv.org                                             @
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

@ ARM32 calling convention: https://blogs.msdn.microsoft.com/ntdebugging/2014/05/29/understanding-arm-assembly-part-3/
@ ARMv7 calling convention: https://developer.apple.com/library/content/documentation/Xcode/Conceptual/iPhoneOSABIReference/Articles/ARMv7FunctionCallingConventions.html#//apple_ref/doc/uid/TP40009022-SW1
@ ARM64 calling convention: https://developer.apple.com/library/content/documentation/Xcode/Conceptual/iPhoneOSABIReference/Articles/ARM64FunctionCallingConventions.html#//apple_ref/doc/uid/TP40013702-SW1

@ Load and store alignent: http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0344b/ch16s06s07.html

@ ARM cycles count: http://pulsar.webshaker.net/ccc/index.php?lng=us

.equ prolog_bytes, 36

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Macro to export and declare a function
@ Example: COMPV_GAS_FUNCTION_DECLARE myFunction
.macro COMPV_GAS_FUNCTION_DECLARE funcname
	.global \funcname
\funcname:
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Macro to exit from the function (x86 'ret' equiv.)
.macro COMPV_GAS_FUNCTION_RETURN
	bx lr
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Function Prolog.
.macro COMPV_GAS_FUNCTION_PROLOG
	@push {r4-r7, lr}        @ save LR, R7, R4-R6, R7 = FP
	@add r7, sp, #12			@ adjust R7 to point to saved R7
	@push {r8, r10, r11}		@ save remaining GPRs (R8, R10, R11)
	push {r4-r11, lr}        @ 9 * 4 = 36 bytes = prolog_bytes
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Function Epilog. More info:
.macro COMPV_GAS_FUNCTION_EPILOG
	@pop {r8, r10, r11}      @ restore R8-R11
	@pop {r4-r7, pc}         @ restore R4-R6, saved R7, return to saved LR
	pop {r4-r11, lr}
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.macro COMPV_GAS_SAVE_NEON_REGS
	vstmdb sp!, {d8-d15}    @ save VFP/Advanced SIMD registers D8,  (aka S16-S31, Q4-Q7)
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.macro COMPV_GAS_RESTORE_NEON_REGS
	vldmia sp!, {d8-d15}    @ restore VFP/Advanced SIMD registers
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ 'r11' is reserved to address the arguments
.macro COMPV_GAS_SHADOW_ARGS_TO_STACK nn
	sub sp, sp, #(\nn * 4)
	mov r11, sp
	.if \nn >= 1
		str r0, [sp, #(4 * 0)]
	.endif
	.if \nn >= 2
		str r1, [sp, #(4 * 1)]
	.endif
	.if \nn >= 3
		str r2, [sp, #(4 * 2)]
	.endif
	.if \nn >= 4
		str r3, [sp, #(4 * 3)]
	.endif
	@@TODO(dmi): use rept ... endr@@
	.if \nn >= 5
		add r4, sp, #(prolog_bytes + (\nn * 4) + (0 * 4))
		ldr r4, [r4]
		str r4, [sp, #(4 * 4)]
	.endif
	.if \nn >= 6
		add r4, sp, #(prolog_bytes + (\nn * 4) + (1 * 4))
		ldr r4, [r4]
		str r4, [sp, #(4 * 5)]
	.endif
	.if \nn >= 7
		add r4, sp, #(prolog_bytes + (\nn * 4) + (2 * 4))
		ldr r4, [r4]
		str r4, [sp, #(4 * 6)]
	.endif
	.if \nn >= 8
		add r4, sp, #(prolog_bytes + (\nn * 4) + (3 * 4))
		ldr r4, [r4]
		str r4, [sp, #(4 * 7)]
	.endif
	.if \nn >= 9
		add r4, sp, #(prolog_bytes + (\nn * 4) + (4 * 4))
		ldr r4, [r4]
		str r4, [sp, #(4 * 8)]
	.endif
	.if \nn >= 10
		add r4, sp, #(prolog_bytes + (\nn * 4) + (5 * 4))
		ldr r4, [r4]
		str r4, [sp, #(4 * 9)]
	.endif	
	.if \nn >= 11
		.err Not implemented
	.endif
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ requires COMPV_GAS_SHADOW_ARGS_TO_STACK
.macro COMPV_GAS_UNSHADOW_ARGS nn
	add sp, sp, #(\nn * 4)
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ requires COMPV_GAS_SHADOW_ARGS_TO_STACK
.macro ldr_arg nn rx
	ldr \rx, [r11, #(\nn * 4)]
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ requires COMPV_GAS_SHADOW_ARGS_TO_STACK
@ Example: ldmia_args r0-r7
.macro ldmia_args registers
	ldmia r11, {\registers}
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.macro COMPV_GAS_MEMALLOC nn
	sub sp, sp, #\nn
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.macro COMPV_GAS_MEMFREE nn
	add sp, sp, #\nn
.endm


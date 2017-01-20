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

@ GNU ARM AS directives: https://www.sourceware.org/binutils/docs/as/ARM-Directives.html#ARM-Directives

@ VFP instructions: http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0489e/CJAGFGEE.html

@ AArch64 NEON instruction format: http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.den0024a/ch07s03.html

@ SSE -> NEON: http://codesuppository.blogspot.fr/2015/02/sse2neonh-porting-guide-and-header-file.html

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.equ NO, 0
.equ YES, 1
.equ CACHE_LINE_SIZE, 64
.equ PREFETCH_DISTANCE, (CACHE_LINE_SIZE << 2)
#if defined(__aarch64__)
	.equ COMPV_GAS_REG_SZ_BYTES, 8
#elif defined(__arm__)
	.equ COMPV_GAS_REG_SZ_BYTES, 4
#else
	#error "Unknown cpu architecture"
#endif
.equ prolog_bytes, (10 * COMPV_GAS_REG_SZ_BYTES) @ push {r4-r12, lr}

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
q0x .dn d0
q0y .dn d1
q1x .dn d2
q1y .dn d3
q2x .dn d4
q2y .dn d5
q3x .dn d6
q3y .dn d7
q4x .dn d8
q4y .dn d9
q5x .dn d10
q5y .dn d11
q6x .dn d12
q6y .dn d13
q7x .dn d14
q7y .dn d15
q8x .dn d16
q8y .dn d17
q9x .dn d18
q9y .dn d19
q10x .dn d20
q10y .dn d21
q11x .dn d22
q11y .dn d23
q12x .dn d24
q12y .dn d25
q13x .dn d26
q13y .dn d27
q14x .dn d28
q14y .dn d29
q15x .dn d30
q15y .dn d31

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
	push {r4-r12, lr}        @ (10 * COMPV_GAS_REG_SZ_BYTES) -> prolog_bytes
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Function Epilog. More info:
.macro COMPV_GAS_FUNCTION_EPILOG
	pop {r4-r12, lr}
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
@ COMPV_GAS_ALIGN_STACK <alignment> <register>
@ This macro aligns the stack to the given alignment (in bytes). The stack
@ is left such that the previous value of the stack pointer is the first
@ argument on the stack (ie, the inverse of this macro is 'pop sp.')
@ This macro uses one temporary register, which is not preserved, and thus
@ must be specified as an argument.
.macro COMPV_GAS_ALIGN_STACK alignment register
    mov         \register, sp
    and         sp, #-\alignment
    sub         sp, #(\alignment - COMPV_GAS_REG_SZ_BYTES)
    push        { \register }
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ COMPV_GAS_UNALIGN_STACK
@ Undo COMPV_GAS_ALIGN_STACK. This function will a temporary register, 
@ which is not preserved, and thus must be specified as an argument.
@ The temporary could be different than the one provide with COMPV_GAS_ALIGN_STACK.
.macro COMPV_GAS_UNALIGN_STACK register
	pop { \register } @ pop { sp } is illegal instruction
	mov sp, \register
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ 'r12' is reserved to address the arguments
.macro COMPV_GAS_SHADOW_ARGS_TO_STACK nn
	sub sp, sp, #(COMPV_GAS_REG_SZ_BYTES * \nn)
	mov r12, sp
	.if \nn >= 1
		str r0, [sp, #(COMPV_GAS_REG_SZ_BYTES * 0)]
	.endif
	.if \nn >= 2
		str r1, [sp, #(COMPV_GAS_REG_SZ_BYTES * 1)]
	.endif
	.if \nn >= 3
		str r2, [sp, #(COMPV_GAS_REG_SZ_BYTES * 2)]
	.endif
	.if \nn >= 4
		str r3, [sp, #(COMPV_GAS_REG_SZ_BYTES * 3)]
	.endif
	add r8, sp, #(prolog_bytes + (COMPV_GAS_REG_SZ_BYTES * \nn))
	add r9, sp, #(COMPV_GAS_REG_SZ_BYTES * 4)
	.rept \nn - 4
		ldr r10, [r8], #COMPV_GAS_REG_SZ_BYTES
		str r10, [r9], #COMPV_GAS_REG_SZ_BYTES
	.endr
	@.set counter, 0
	@.rept \nn - 4
	@	add r4, sp, #(prolog_bytes + (COMPV_GAS_REG_SZ_BYTES * \nn) + (COMPV_GAS_REG_SZ_BYTES * counter))
	@	ldr r4, [r4]
	@	str r4, [sp, #(COMPV_GAS_REG_SZ_BYTES * (counter + 4))]
	@	.set counter, counter+1
	@.endr
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ requires COMPV_GAS_SHADOW_ARGS_TO_STACK
.macro COMPV_GAS_UNSHADOW_ARGS nn
	add sp, sp, #(\nn * COMPV_GAS_REG_SZ_BYTES)
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ requires COMPV_GAS_SHADOW_ARGS_TO_STACK
.macro ldr_arg nn rx
	ldr \rx, [r12, #(\nn * COMPV_GAS_REG_SZ_BYTES)]
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ requires COMPV_GAS_SHADOW_ARGS_TO_STACK
@ Example: ldmia_args r0-r7
.macro ldmia_args registers
	ldmia r12, {\registers}
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.macro COMPV_GAS_MEMALLOC nn
	sub sp, sp, #\nn
.endm

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.macro COMPV_GAS_MEMFREE nn
	add sp, sp, #\nn
.endm


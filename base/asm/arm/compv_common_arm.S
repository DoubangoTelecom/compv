#########################################################################
# Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>   #
# File author: Mamadou DIOP (Doubango Telecom, France).                 #
# License: GPLv3. For commercial license please contact us.             #
# Source code: https://github.com/DoubangoTelecom/compv                 #
# WebSite: http://compv.org                                             #
#########################################################################
#if defined(__arm__) || defined(__aarch64__)

# ARM32 calling convention: https://blogs.msdn.microsoft.com/ntdebugging/2014/05/29/understanding-arm-assembly-part-3/
# ARMv7 calling convention: https://developer.apple.com/library/content/documentation/Xcode/Conceptual/iPhoneOSABIReference/Articles/ARMv7FunctionCallingConventions.html#//apple_ref/doc/uid/TP40009022-SW1
# ARM64 calling convention: https://developer.apple.com/library/content/documentation/Xcode/Conceptual/iPhoneOSABIReference/Articles/ARM64FunctionCallingConventions.html#//apple_ref/doc/uid/TP40013702-SW1
# ARM64 calling convention: https://en.wikipedia.org/wiki/Calling_convention#ARM_.28A64.29

# GDB / LLDB commands: https://developer.apple.com/library/content/documentation/IDEs/Conceptual/gdb_to_lldb_transition_guide/document/lldb-command-examples.html

# Load and store alignent: http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0344b/ch16s06s07.html

# ARM cycles count: http://pulsar.webshaker.net/ccc/index.php?lng=us

# ARM assembly to Visual Code: https://marketplace.visualstudio.com/items?itemName=dan-c-underwood.arm

# GNU ARM AS directives: https://www.sourceware.org/binutils/docs/as/ARM-Directives.html#ARM-Directives

# VFP instructions: http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0489e/CJAGFGEE.html

# AArch64 NEON instruction format: http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.den0024a/ch07s03.html

# SSE -> NEON: http://codesuppository.blogspot.fr/2015/02/sse2neonh-porting-guide-and-header-file.html

#########################################################################
.equ NO, 0
.equ YES, 1
.equ CACHE_LINE_SIZE, 64
.equ PREFETCH_DISTANCE, (CACHE_LINE_SIZE << 2)
#if defined(__llvm__)
    .equ COMPV_GAS_LLVM, 1
#endif
#if defined(__APPLE__)
    .equ COMPV_GAS_APPLE, 1
#endif
#if defined(__APPLE__) && defined(__llvm__)
    .equ COMPV_GAS_MANGLE, 1
    #define sym(funcname) _##funcname
#else
    .equ COMPV_GAS_MANGLE, 0
    #define sym(funcname) funcname
#endif

#########################################################################
# .dn to define neon registers aliases not supported by llvm.
q0x .req d0
q0y .req d1
q1x .req d2
q1y .req d3
q2x .req d4
q2y .req d5
q3x .req d6
q3y .req d7
q4x .req d8
q4y .req d9
q5x .req d10
q5y .req d11
q6x .req d12
q6y .req d13
q7x .req d14
q7y .req d15
q8x .req d16
q8y .req d17
q9x .req d18
q9y .req d19
q10x .req d20
q10y .req d21
q11x .req d22
q11y .req d23
q12x .req d24
q12y .req d25
q13x .req d26
q13y .req d27
q14x .req d28
q14y .req d29
q15x .req d30
q15y .req d31

#########################################################################
# Macro to mangle function names
.macro COMPV_GAS_FUNCTION_MANGLE funcname
    sym(\funcname)
.endmacro


#########################################################################
# Macro to export and declare a function
# Example: COMPV_GAS_FUNCTION_DECLARE myFunction
.macro COMPV_GAS_FUNCTION_DECLARE funcname
    .align 2
    #.global sym(\funcname)
    #sym(\funcname):
#if defined(__APPLE__) && defined(__llvm__)
        .global _\funcname
        _\funcname:
#else
        .global \funcname
        \funcname:
#endif
.endm

#########################################################################
.macro COMPV_GAS_MEMALLOC nn
	sub sp, sp, #\nn
.endm

#########################################################################
.macro COMPV_GAS_MEMFREE nn
	add sp, sp, #\nn
.endm

#endif /* defined(__arm__) || defined(__aarch64__) */

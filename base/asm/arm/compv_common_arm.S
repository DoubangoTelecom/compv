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
.equ COMPV_GAS_VEC_SZ_BYTES, 16
.equ COMPV_GAS_V_SZ_BYTES, 16
.equ COMPV_GAS_Q_SZ_BYTES, 16
.equ COMPV_GAS_FLOAT64_SZ_BYTES, 8
.equ COMPV_GAS_FLOAT32_SZ_BYTES, 4
.equ COMPV_GAS_INT32_SZ_BYTES, 4
.equ COMPV_GAS_UINT32_SZ_BYTES, COMPV_GAS_INT32_SZ_BYTES
.equ COMPV_GAS_INT16_SZ_BYTES, 2
.equ COMPV_GAS_UINT16_SZ_BYTES, COMPV_GAS_INT16_SZ_BYTES
.equ COMPV_GAS_INT8_SZ_BYTES, 1
.equ COMPV_GAS_UINT8_SZ_BYTES, COMPV_GAS_INT8_SZ_BYTES
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
# Macro to mangle function names
.macro COMPV_GAS_FUNCTION_MANGLE funcname
    sym(\funcname)
.endm


#########################################################################
# Macro to export and declare a function
# Example: COMPV_GAS_FUNCTION_DECLARE myFunction
.macro COMPV_GAS_FUNCTION_DECLARE funcname
    .align 4
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

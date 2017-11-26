/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_convlt.h"
#include "compv/base/compv_cpu.h"

#include "compv/base/math/intrin/x86/compv_math_convlt_intrin_avx2.h"
#include "compv/base/math/intrin/x86/compv_math_convlt_intrin_sse2.h"
#include "compv/base/math/intrin/arm/compv_math_convlt_intrin_neon.h"

COMPV_NAMESPACE_BEGIN()

// X86 (deprecated)
#if COMPV_ASM && COMPV_ARCH_X86 && 0
COMPV_EXTERNC void CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_X86_SSE2(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const uint16_t* vthzKernPtr, compv_uscalar_t kernSize);
COMPV_EXTERNC void CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_X86_AVX2(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const uint16_t* vthzKernPtr, compv_uscalar_t kernSize);
COMPV_EXTERNC void CompVMathConvlt1VtHz_8u32f8u_Asm_X86_SSE2(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize);
COMPV_EXTERNC void CompVMathConvlt1VtHz_8u32f8u_Asm_X86_AVX2(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize);
COMPV_EXTERNC void CompVMathConvlt1VtHz_8u32f8u_Asm_X86_FMA3_AVX2(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize);
COMPV_EXTERNC void CompVMathConvlt1VtHz_8u16s16s_Asm_X86_SSE2(const uint8_t* inPtr, int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const int16_t* vthzKernPtr, compv_uscalar_t kernSize);
COMPV_EXTERNC void CompVMathConvlt1VtHz_8u16s16s_Asm_X86_AVX2(const uint8_t* inPtr, int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const int16_t* vthzKernPtr, compv_uscalar_t kernSize);
COMPV_EXTERNC void CompVMathConvlt1VtHz_16s16s16s_Asm_X86_SSE2(const int16_t* inPtr, int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const int16_t* vthzKernPtr, compv_uscalar_t kernSize);
COMPV_EXTERNC void CompVMathConvlt1VtHz_16s16s16s_Asm_X86_AVX2(const int16_t* inPtr, int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const int16_t* vthzKernPtr, compv_uscalar_t kernSize);
#endif /* X86 */

// X64
#if COMPV_ASM && COMPV_ARCH_X64
COMPV_EXTERNC void CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_X64_SSE2(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const uint16_t* vthzKernPtr, compv_uscalar_t kernSize);
COMPV_EXTERNC void CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_X64_AVX2(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const uint16_t* vthzKernPtr, compv_uscalar_t kernSize);
COMPV_EXTERNC void CompVMathConvlt1VtHz_8u32f8u_Asm_X64_SSE2(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize);
COMPV_EXTERNC void CompVMathConvlt1VtHz_8u32f8u_Asm_X64_AVX2(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize);
COMPV_EXTERNC void CompVMathConvlt1VtHz_8u32f8u_Asm_X64_FMA3_AVX2(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize);
COMPV_EXTERNC void CompVMathConvlt1VtHz_8u32f32f_Asm_X64_SSE2(const uint8_t* inPtr, compv_float32_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize);
COMPV_EXTERNC void CompVMathConvlt1VtHz_8u32f32f_Asm_X64_AVX2(const uint8_t* inPtr, compv_float32_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize);
COMPV_EXTERNC void CompVMathConvlt1VtHz_32f32f32f_Asm_X64_SSE2(const compv_float32_t* inPtr, compv_float32_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize);
COMPV_EXTERNC void CompVMathConvlt1VtHz_32f32f32f_Asm_X64_AVX2(const compv_float32_t* inPtr, compv_float32_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize);
COMPV_EXTERNC void CompVMathConvlt1VtHz_32f32f8u_Asm_X64_SSE2(const compv_float32_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize);
COMPV_EXTERNC void CompVMathConvlt1VtHz_32f32f8u_Asm_X64_AVX2(const compv_float32_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize);
COMPV_EXTERNC void CompVMathConvlt1VtHz_8u16s16s_Asm_X64_SSE41(const uint8_t* inPtr, int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const int16_t* vthzKernPtr, compv_uscalar_t kernSize);
COMPV_EXTERNC void CompVMathConvlt1VtHz_8u16s16s_Asm_X64_AVX2(const uint8_t* inPtr, int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const int16_t* vthzKernPtr, compv_uscalar_t kernSize);
COMPV_EXTERNC void CompVMathConvlt1VtHz_16s16s16s_Asm_X64_SSE41(const int16_t* inPtr, int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const int16_t* vthzKernPtr, compv_uscalar_t kernSize);
COMPV_EXTERNC void CompVMathConvlt1VtHz_16s16s16s_Asm_X64_AVX2(const int16_t* inPtr, int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const int16_t* vthzKernPtr, compv_uscalar_t kernSize);
#endif /* X64 */

// ARM32
#if COMPV_ASM && COMPV_ARCH_ARM32
COMPV_EXTERNC void CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON32(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const uint16_t* vthzKernPtr, compv_uscalar_t kernSize);
COMPV_EXTERNC void CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize);
COMPV_EXTERNC void CompVMathConvlt1VtHz_8u32f8u_Asm_FMA_NEON32(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize);
COMPV_EXTERNC void CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32(const uint8_t* inPtr, int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const int16_t* vthzKernPtr, compv_uscalar_t kernSize);
COMPV_EXTERNC void CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32(const int16_t* inPtr, int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const int16_t* vthzKernPtr, compv_uscalar_t kernSize);
#endif /* ARM32 */

// ARM64
#if COMPV_ASM && COMPV_ARCH_ARM64
COMPV_EXTERNC void CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const uint16_t* vthzKernPtr, compv_uscalar_t kernSize);
COMPV_EXTERNC void CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize);
COMPV_EXTERNC void CompVMathConvlt1VtHz_8u32f8u_Asm_FMA_NEON64(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize);
COMPV_EXTERNC void CompVMathConvlt1VtHz_8u16s16s_Asm_NEON64(const uint8_t* inPtr, int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const int16_t* vthzKernPtr, compv_uscalar_t kernSize);
COMPV_EXTERNC void CompVMathConvlt1VtHz_16s16s16s_Asm_NEON64(const int16_t* inPtr, int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const int16_t* vthzKernPtr, compv_uscalar_t kernSize);
#endif /* ARM64 */

// InputType = uint8_t, KernelType = int16_t, OutputType = uint8_t, FixedPoint = true
template<> COMPV_BASE_API 
COMPV_ERROR_CODE CompVMathConvlt::convlt1VtHz_private_fxp_true(const uint8_t* inPtr, uint8_t* outPtr, size_t width, size_t height, size_t step, size_t pad, const uint16_t* vthzKernPtr, size_t kernSize)
{
	void(*CompVMathConvlt1VtHzFixedPoint_8u16u8u)(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const uint16_t* vthzKernPtr, compv_uscalar_t kernSize)
		= [](const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const uint16_t* vthzKernPtr, compv_uscalar_t kernSize) {
		COMPV_CHECK_CODE_NOP(CompVMathConvlt::convlt1VtHzFixedPoint_C(inPtr, outPtr, width, height, step, pad, vthzKernPtr, kernSize));
	};

#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSE2) && width >= 16) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathConvlt1VtHzFixedPoint_8u16u8u = CompVMathConvlt1VtHzFixedPoint_8u16u8u_Intrin_SSE2);
		COMPV_EXEC_IFDEF_ASM_X64(CompVMathConvlt1VtHzFixedPoint_8u16u8u = CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_X64_SSE2);
	}
	if (CompVCpu::isEnabled(kCpuFlagAVX2) && width >= 32) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathConvlt1VtHzFixedPoint_8u16u8u = CompVMathConvlt1VtHzFixedPoint_8u16u8u_Intrin_AVX2);
		COMPV_EXEC_IFDEF_ASM_X64(CompVMathConvlt1VtHzFixedPoint_8u16u8u = CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_X64_AVX2);
	}
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && width >= 16) {
		COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMathConvlt1VtHzFixedPoint_8u16u8u = CompVMathConvlt1VtHzFixedPoint_8u16u8u_Intrin_NEON);
		//COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathConvlt1VtHzFixedPoint_8u16u8u = CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON32);
		//COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathConvlt1VtHzFixedPoint_8u16u8u = CompVMathConvlt1VtHzFixedPoint_8u16u8u_Asm_NEON64);
	}
#endif

	CompVMathConvlt1VtHzFixedPoint_8u16u8u(inPtr, outPtr, static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(height), static_cast<compv_uscalar_t>(step), static_cast<compv_uscalar_t>(pad), vthzKernPtr, static_cast<compv_uscalar_t>(kernSize));
	
	return COMPV_ERROR_CODE_S_OK;
}

// InputType = uint8_t, KernelType = compv_float32_t, OutputType = uint8_t, FixedPoint = false
template<> COMPV_BASE_API 
COMPV_ERROR_CODE CompVMathConvlt::convlt1VtHz_private_fxp_false(const uint8_t* inPtr, uint8_t* outPtr, size_t width, size_t height, size_t step, size_t pad, const compv_float32_t* vthzKernPtr, size_t kernSize)
{
	void(*CompVMathConvlt1VtHz_8u32f8u)(const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize)
		= [](const uint8_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize)
	{
		COMPV_CHECK_CODE_NOP((CompVMathConvlt::convlt1VtHzKernelFloat_C<uint8_t, compv_float32_t, uint8_t>(inPtr, outPtr, width, height, step, pad, vthzKernPtr, kernSize)));
	};

#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSE2) && width >= 16) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathConvlt1VtHz_8u32f8u = CompVMathConvlt1VtHz_8u32f8u_Intrin_SSE2);
		COMPV_EXEC_IFDEF_ASM_X64(CompVMathConvlt1VtHz_8u32f8u = CompVMathConvlt1VtHz_8u32f8u_Asm_X64_SSE2);
	}
	if (CompVCpu::isEnabled(kCpuFlagAVX2) && width >= 32) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathConvlt1VtHz_8u32f8u = CompVMathConvlt1VtHz_8u32f8u_Intrin_AVX2);
		COMPV_EXEC_IFDEF_ASM_X64(CompVMathConvlt1VtHz_8u32f8u = CompVMathConvlt1VtHz_8u32f8u_Asm_X64_AVX2);
		if (CompVCpu::isEnabled(kCpuFlagFMA3)) {
			//COMPV_EXEC_IFDEF_ASM_X64(CompVMathConvlt1VtHz_8u32f8u = CompVMathConvlt1VtHz_8u32f8u_Asm_X64_FMA3_AVX2);
		}
	}
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && width >= 16) {
		COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMathConvlt1VtHz_8u32f8u = CompVMathConvlt1VtHz_8u32f8u_Intrin_NEON);
		//COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathConvlt1VtHz_8u32f8u = CompVMathConvlt1VtHz_8u32f8u_Asm_NEON32);
		//COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathConvlt1VtHz_8u32f8u = CompVMathConvlt1VtHz_8u32f8u_Asm_NEON64);
        if (CompVCpu::isEnabled(kCpuFlagARM_NEON_FMA)) {
            //COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathConvlt1VtHz_8u32f8u = CompVMathConvlt1VtHz_8u32f8u_Asm_FMA_NEON32);
            //COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathConvlt1VtHz_8u32f8u = CompVMathConvlt1VtHz_8u32f8u_Asm_FMA_NEON64);
        }
	}
#endif

	CompVMathConvlt1VtHz_8u32f8u(inPtr, outPtr, static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(height), static_cast<compv_uscalar_t>(step), static_cast<compv_uscalar_t>(pad), vthzKernPtr, static_cast<compv_uscalar_t>(kernSize));
	
	return COMPV_ERROR_CODE_S_OK;
}

// InputType = uint8_t, KernelType = compv_float32_t, OutputType = compv_float32_t, FixedPoint = false
template<> COMPV_BASE_API
COMPV_ERROR_CODE CompVMathConvlt::convlt1VtHz_private_fxp_false(const uint8_t* inPtr, compv_float32_t* outPtr, size_t width, size_t height, size_t step, size_t pad, const compv_float32_t* vthzKernPtr, size_t kernSize)
{
	void(*CompVMathConvlt1VtHz_8u32f32f)(const uint8_t* inPtr, compv_float32_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize)
		= [](const uint8_t* inPtr, compv_float32_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize)
	{
		COMPV_CHECK_CODE_NOP((CompVMathConvlt::convlt1VtHzKernelFloat_C<uint8_t, compv_float32_t, compv_float32_t>(inPtr, outPtr, width, height, step, pad, vthzKernPtr, kernSize)));
	};

#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSE2) && width >= 16) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathConvlt1VtHz_8u32f32f = CompVMathConvlt1VtHz_8u32f32f_Intrin_SSE2);
		COMPV_EXEC_IFDEF_ASM_X64(CompVMathConvlt1VtHz_8u32f32f = CompVMathConvlt1VtHz_8u32f32f_Asm_X64_SSE2);
	}
	if (CompVCpu::isEnabled(kCpuFlagAVX2) && width >= 32) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathConvlt1VtHz_8u32f32f = CompVMathConvlt1VtHz_8u32f32f_Intrin_AVX2);
		COMPV_EXEC_IFDEF_ASM_X64(CompVMathConvlt1VtHz_8u32f32f = CompVMathConvlt1VtHz_8u32f32f_Asm_X64_AVX2);
		if (CompVCpu::isEnabled(kCpuFlagFMA3)) {
			//COMPV_EXEC_IFDEF_ASM_X64(CompVMathConvlt1VtHz_8u32f32f = CompVMathConvlt1VtHz_8u32f32f_Asm_X64_FMA3_AVX2);
		}
	}
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && width >= 16) {
		COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMathConvlt1VtHz_8u32f32f = CompVMathConvlt1VtHz_8u32f32f_Intrin_NEON);
		//COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathConvlt1VtHz_8u32f32f = CompVMathConvlt1VtHz_8u32f32f_Asm_NEON32);
		//COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathConvlt1VtHz_8u32f32f = CompVMathConvlt1VtHz_8u32f32f_Asm_NEON64);
		if (CompVCpu::isEnabled(kCpuFlagARM_NEON_FMA)) {
			//COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathConvlt1VtHz_8u32f32f = CompVMathConvlt1VtHz_8u32f32f_Asm_FMA_NEON32);
			//COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathConvlt1VtHz_8u32f32f = CompVMathConvlt1VtHz_8u32f32f_Asm_FMA_NEON64);
		}
	}
#endif

	CompVMathConvlt1VtHz_8u32f32f(inPtr, outPtr, static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(height), static_cast<compv_uscalar_t>(step), static_cast<compv_uscalar_t>(pad), vthzKernPtr, static_cast<compv_uscalar_t>(kernSize));

	return COMPV_ERROR_CODE_S_OK;
}

// InputType = compv_float32_t, KernelType = compv_float32_t, OutputType = compv_float32_t, FixedPoint = false
template<> COMPV_BASE_API
COMPV_ERROR_CODE CompVMathConvlt::convlt1VtHz_private_fxp_false(const compv_float32_t* inPtr, compv_float32_t* outPtr, size_t width, size_t height, size_t step, size_t pad, const compv_float32_t* vthzKernPtr, size_t kernSize)
{
	void(*CompVMathConvlt1VtHz_32f32f32f)(const compv_float32_t* inPtr, compv_float32_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize)
		= [](const compv_float32_t* inPtr, compv_float32_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize)
	{
		COMPV_CHECK_CODE_NOP((CompVMathConvlt::convlt1VtHzKernelFloat_C<compv_float32_t, compv_float32_t, compv_float32_t>(inPtr, outPtr, width, height, step, pad, vthzKernPtr, kernSize)));
	};

#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSE2) && width >= 16) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathConvlt1VtHz_32f32f32f = CompVMathConvlt1VtHz_32f32f32f_Intrin_SSE2);
		COMPV_EXEC_IFDEF_ASM_X64(CompVMathConvlt1VtHz_32f32f32f = CompVMathConvlt1VtHz_32f32f32f_Asm_X64_SSE2);
	}
	if (CompVCpu::isEnabled(kCpuFlagAVX2) && width >= 32) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathConvlt1VtHz_32f32f32f = CompVMathConvlt1VtHz_32f32f32f_Intrin_AVX2);
		COMPV_EXEC_IFDEF_ASM_X64(CompVMathConvlt1VtHz_32f32f32f = CompVMathConvlt1VtHz_32f32f32f_Asm_X64_AVX2);
		if (CompVCpu::isEnabled(kCpuFlagFMA3)) {
			//COMPV_EXEC_IFDEF_ASM_X64(CompVMathConvlt1VtHz_32f32f32f = CompVMathConvlt1VtHz_32f32f32f_Asm_X64_FMA3_AVX2);
		}
	}
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && width >= 16) {
		COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMathConvlt1VtHz_32f32f32f = CompVMathConvlt1VtHz_32f32f32f_Intrin_NEON);
		//COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathConvlt1VtHz_32f32f32f = CompVMathConvlt1VtHz_32f32f32f_Asm_NEON32);
		//COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathConvlt1VtHz_32f32f32f = CompVMathConvlt1VtHz_32f32f32f_Asm_NEON64);
		if (CompVCpu::isEnabled(kCpuFlagARM_NEON_FMA)) {
			//COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathConvlt1VtHz_32f32f32f = CompVMathConvlt1VtHz_32f32f32f_Asm_FMA_NEON32);
			//COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathConvlt1VtHz_32f32f32f = CompVMathConvlt1VtHz_32f32f32f_Asm_FMA_NEON64);
		}
	}
#endif
	
	CompVMathConvlt1VtHz_32f32f32f(inPtr, outPtr, static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(height), static_cast<compv_uscalar_t>(step), static_cast<compv_uscalar_t>(pad), vthzKernPtr, static_cast<compv_uscalar_t>(kernSize));
	
	return COMPV_ERROR_CODE_S_OK;
}

// InputType = compv_float32_t, KernelType = compv_float32_t, OutputType = uint8_t, FixedPoint = false
template<> COMPV_BASE_API
COMPV_ERROR_CODE CompVMathConvlt::convlt1VtHz_private_fxp_false(const compv_float32_t* inPtr, uint8_t* outPtr, size_t width, size_t height, size_t step, size_t pad, const compv_float32_t* vthzKernPtr, size_t kernSize)
{
	void(*CompVMathConvlt1VtHz_32f32f8u)(const compv_float32_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize)
		= [](const compv_float32_t* inPtr, uint8_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const compv_float32_t* vthzKernPtr, compv_uscalar_t kernSize)
	{
		COMPV_CHECK_CODE_NOP((CompVMathConvlt::convlt1VtHzKernelFloat_C<compv_float32_t, compv_float32_t, uint8_t>(inPtr, outPtr, width, height, step, pad, vthzKernPtr, kernSize)));
	};

#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSE2) && width >= 16) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathConvlt1VtHz_32f32f8u = CompVMathConvlt1VtHz_32f32f8u_Intrin_SSE2);
		COMPV_EXEC_IFDEF_ASM_X64(CompVMathConvlt1VtHz_32f32f8u = CompVMathConvlt1VtHz_32f32f8u_Asm_X64_SSE2);
	}
	if (CompVCpu::isEnabled(kCpuFlagAVX2) && width >= 32) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathConvlt1VtHz_32f32f8u = CompVMathConvlt1VtHz_32f32f8u_Intrin_AVX2);
		COMPV_EXEC_IFDEF_ASM_X64(CompVMathConvlt1VtHz_32f32f8u = CompVMathConvlt1VtHz_32f32f8u_Asm_X64_AVX2);
		if (CompVCpu::isEnabled(kCpuFlagFMA3)) {
			//COMPV_EXEC_IFDEF_ASM_X64(CompVMathConvlt1VtHz_32f32f8u = CompVMathConvlt1VtHz_32f32f8u_Asm_X64_FMA3_AVX2);
		}
	}
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && width >= 16) {
		COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMathConvlt1VtHz_32f32f8u = CompVMathConvlt1VtHz_32f32f8u_Intrin_NEON);
		//COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathConvlt1VtHz_32f32f8u = CompVMathConvlt1VtHz_32f32f8u_Asm_NEON32);
		//COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathConvlt1VtHz_32f32f8u = CompVMathConvlt1VtHz_32f32f8u_Asm_NEON64);
		if (CompVCpu::isEnabled(kCpuFlagARM_NEON_FMA)) {
			//COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathConvlt1VtHz_32f32f8u = CompVMathConvlt1VtHz_32f32f8u_Asm_FMA_NEON32);
			//COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathConvlt1VtHz_32f32f8u = CompVMathConvlt1VtHz_32f32f8u_Asm_FMA_NEON64);
		}
	}
#endif

	CompVMathConvlt1VtHz_32f32f8u(inPtr, outPtr, static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(height), static_cast<compv_uscalar_t>(step), static_cast<compv_uscalar_t>(pad), vthzKernPtr, static_cast<compv_uscalar_t>(kernSize));

	return COMPV_ERROR_CODE_S_OK;
}

// InputType = uint8_t, KernelType = int16_t, OutputType = int16_t, FixedPoint = false
template<> COMPV_BASE_API 
COMPV_ERROR_CODE CompVMathConvlt::convlt1VtHz_private_fxp_false(const uint8_t* inPtr, int16_t* outPtr, size_t width, size_t height, size_t step, size_t pad, const int16_t* vthzKernPtr, size_t kernSize)
{
	void(*CompVMathConvlt1VtHz_8u16s16s)(const uint8_t* inPtr, int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const int16_t* vthzKernPtr, compv_uscalar_t kernSize)
		= [](const uint8_t* inPtr, int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const int16_t* vthzKernPtr, compv_uscalar_t kernSize)
	{
		COMPV_CHECK_CODE_NOP((CompVMathConvlt::convlt1VtHzKernelInt_C<uint8_t, int16_t, int16_t>(inPtr, outPtr, width, height, step, pad, vthzKernPtr, kernSize)));
	};

#if COMPV_ARCH_X86
	if (width >= 16) {
		if (CompVCpu::isEnabled(kCpuFlagSSE2)) {
			COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathConvlt1VtHz_8u16s16s = CompVMathConvlt1VtHz_8u16s16s_Intrin_SSE2);
		}
		if (CompVCpu::isEnabled(kCpuFlagSSE41)) {
			COMPV_EXEC_IFDEF_ASM_X64(CompVMathConvlt1VtHz_8u16s16s = CompVMathConvlt1VtHz_8u16s16s_Asm_X64_SSE41);
		}
	}
	if (CompVCpu::isEnabled(kCpuFlagAVX2) && width >= 32) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathConvlt1VtHz_8u16s16s = CompVMathConvlt1VtHz_8u16s16s_Intrin_AVX2);
		COMPV_EXEC_IFDEF_ASM_X64(CompVMathConvlt1VtHz_8u16s16s = CompVMathConvlt1VtHz_8u16s16s_Asm_X64_AVX2);
	}
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && width >= 16) {
		COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMathConvlt1VtHz_8u16s16s = CompVMathConvlt1VtHz_8u16s16s_Intrin_NEON);
		//COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathConvlt1VtHz_8u16s16s = CompVMathConvlt1VtHz_8u16s16s_Asm_NEON32);
		//COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathConvlt1VtHz_8u16s16s = CompVMathConvlt1VtHz_8u16s16s_Asm_NEON64);
	}
#endif

	CompVMathConvlt1VtHz_8u16s16s(inPtr, outPtr, static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(height), static_cast<compv_uscalar_t>(step), static_cast<compv_uscalar_t>(pad), vthzKernPtr, static_cast<compv_uscalar_t>(kernSize));

	return COMPV_ERROR_CODE_S_OK;
}

// InputType = int16_t, KernelType = int16_t, OutputType = int16_t, FixedPoint = false
template<> COMPV_BASE_API 
COMPV_ERROR_CODE CompVMathConvlt::convlt1VtHz_private_fxp_false(const int16_t* inPtr, int16_t* outPtr, size_t width, size_t height, size_t step, size_t pad, const int16_t* vthzKernPtr, size_t kernSize)
{
	void(*CompVMathConvlt1VtHz_16s16s16s)(const int16_t* inPtr, int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const int16_t* vthzKernPtr, compv_uscalar_t kernSize)
		= [](const int16_t* inPtr, int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t step, compv_uscalar_t pad, const int16_t* vthzKernPtr, compv_uscalar_t kernSize)
	{
		COMPV_CHECK_CODE_NOP((CompVMathConvlt::convlt1VtHzKernelInt_C<int16_t, int16_t, int16_t>(inPtr, outPtr, width, height, step, pad, vthzKernPtr, kernSize)));
	};

#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSE2) && width >= 16) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathConvlt1VtHz_16s16s16s = CompVMathConvlt1VtHz_16s16s16s_Intrin_SSE2);
		//COMPV_EXEC_IFDEF_ASM_X64(CompVMathConvlt1VtHz_16s16s16s = CompVMathConvlt1VtHz_16s16s16s_Asm_X64_SSE2);
	}
	if (CompVCpu::isEnabled(kCpuFlagAVX2) && width >= 32) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathConvlt1VtHz_16s16s16s = CompVMathConvlt1VtHz_16s16s16s_Intrin_AVX2);
		//COMPV_EXEC_IFDEF_ASM_X64(CompVMathConvlt1VtHz_16s16s16s = CompVMathConvlt1VtHz_16s16s16s_Asm_X64_AVX2);
	}
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && width >= 16) {
		COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMathConvlt1VtHz_16s16s16s = CompVMathConvlt1VtHz_16s16s16s_Intrin_NEON);
		//COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathConvlt1VtHz_16s16s16s = CompVMathConvlt1VtHz_16s16s16s_Asm_NEON32);
		//COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathConvlt1VtHz_16s16s16s = CompVMathConvlt1VtHz_16s16s16s_Asm_NEON64);
	}
#endif

	CompVMathConvlt1VtHz_16s16s16s(inPtr, outPtr, static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(height), static_cast<compv_uscalar_t>(step), static_cast<compv_uscalar_t>(pad), vthzKernPtr, static_cast<compv_uscalar_t>(kernSize));

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

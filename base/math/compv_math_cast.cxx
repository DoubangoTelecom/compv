/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_cast.h"
#include "compv/base/compv_cpu.h"

COMPV_NAMESPACE_BEGIN()

template <> COMPV_BASE_API
COMPV_ERROR_CODE CompVMathCast::process_static(const compv_float32_t* src, compv_float64_t* dst, const size_t width, const size_t height, const size_t stride)
{
	void(*CompVMathCastProcessStatic_32f64f)(const compv_float32_t* src, compv_float64_t* dst, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride)
		= [](const compv_float32_t* src, compv_float64_t* dst, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride)
	{
		COMPV_CHECK_CODE_NOP((CompVMathCast::process_static_C<compv_float32_t, compv_float64_t>(src, dst, width, height, stride)));
	};

#if COMPV_ARCH_X86
	if (width >= 4) {
		if (CompVCpu::isEnabled(kCpuFlagSSE2)) {
			//COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathCastProcessStatic_32f64f = CompVMathCastProcessStatic_32f64f_Intrin_SSE2);
		}
	}
	if (CompVCpu::isEnabled(kCpuFlagAVX2) && width >= 8) {
		//COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathCastProcessStatic_32f64f = CompVMathCastProcessStatic_32f64f_Intrin_AVX2);
		//COMPV_EXEC_IFDEF_ASM_X64(CompVMathCastProcessStatic_32f64f = CompVMathCastProcessStatic_32f64f_Asm_X64_AVX2);
	}
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && width >= 4) {
		//COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMathCastProcessStatic_32f64f = CompVMathCastProcessStatic_32f64f_Intrin_NEON);
		//COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathCastProcessStatic_32f64f = CompVMathCastProcessStatic_32f64f_Asm_NEON32);
		//COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathCastProcessStatic_32f64f = CompVMathCastProcessStatic_32f64f_Asm_NEON64);
	}
#endif

	CompVMathCastProcessStatic_32f64f(src, dst, static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(height), static_cast<compv_uscalar_t>(stride));

	return COMPV_ERROR_CODE_S_OK;
}

template <> COMPV_BASE_API
COMPV_ERROR_CODE CompVMathCast::process_static(const compv_float64_t* src, compv_float32_t* dst, const size_t width, const size_t height, const size_t stride)
{
	void(*CompVMathCastProcessStatic_64f32f)(const compv_float64_t* src, compv_float32_t* dst, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride)
		= [](const compv_float64_t* src, compv_float32_t* dst, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride)
	{
		COMPV_CHECK_CODE_NOP((CompVMathCast::process_static_C<compv_float64_t, compv_float32_t>(src, dst, width, height, stride)));
	};

#if COMPV_ARCH_X86
	if (width >= 4) {
		if (CompVCpu::isEnabled(kCpuFlagSSE2)) {
			//COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathCastProcessStatic_64f32f = CompVMathCastProcessStatic_64f32f_Intrin_SSE2);
		}
	}
	if (CompVCpu::isEnabled(kCpuFlagAVX2) && width >= 8) {
		//COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathCastProcessStatic_64f32f = CompVMathCastProcessStatic_64f32f_Intrin_AVX2);
		//COMPV_EXEC_IFDEF_ASM_X64(CompVMathCastProcessStatic_64f32f = CompVMathCastProcessStatic_64f32f_Asm_X64_AVX2);
	}
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && width >= 4) {
		//COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMathCastProcessStatic_64f32f = CompVMathCastProcessStatic_64f32f_Intrin_NEON);
		//COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathCastProcessStatic_64f32f = CompVMathCastProcessStatic_64f32f_Asm_NEON32);
		//COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathCastProcessStatic_64f32f = CompVMathCastProcessStatic_64f32f_Asm_NEON64);
	}
#endif

	CompVMathCastProcessStatic_64f32f(src, dst, static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(height), static_cast<compv_uscalar_t>(stride));

	return COMPV_ERROR_CODE_S_OK;
}

template <> COMPV_BASE_API
COMPV_ERROR_CODE CompVMathCast::process_static(const uint8_t* src, compv_float32_t* dst, const size_t width, const size_t height, const size_t stride)
{
	void(*CompVMathCastProcessStatic_8u32f)(const uint8_t* src, compv_float32_t* dst, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride)
		= [](const uint8_t* src, compv_float32_t* dst, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride)
	{
		COMPV_CHECK_CODE_NOP((CompVMathCast::process_static_C<uint8_t, compv_float32_t>(src, dst, width, height, stride)));
	};

#if COMPV_ARCH_X86
	if (width >= 16) {
		if (CompVCpu::isEnabled(kCpuFlagSSE2)) {
			//COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathCastProcessStatic_8u32f = CompVMathCastProcessStatic_8u32f_Intrin_SSE2);
		}
	}
	if (CompVCpu::isEnabled(kCpuFlagAVX2) && width >= 32) {
		//COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathCastProcessStatic_8u32f = CompVMathCastProcessStatic_8u32f_Intrin_AVX2);
		//COMPV_EXEC_IFDEF_ASM_X64(CompVMathCastProcessStatic_8u32f = CompVMathCastProcessStatic_8u32f_Asm_X64_AVX2);
	}
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && width >= 16) {
		//COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMathCastProcessStatic_8u32f = CompVMathCastProcessStatic_8u32f_Intrin_NEON);
		//COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathCastProcessStatic_8u32f = CompVMathCastProcessStatic_8u32f_Asm_NEON32);
		//COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathCastProcessStatic_8u32f = CompVMathCastProcessStatic_8u32f_Asm_NEON64);
	}
#endif

	CompVMathCastProcessStatic_8u32f(src, dst, static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(height), static_cast<compv_uscalar_t>(stride));

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

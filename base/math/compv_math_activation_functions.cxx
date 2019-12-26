/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_activation_functions.h"
#include "compv/base/compv_cpu.h"

#include "compv/base/math/intrin/x86/compv_math_activation_functions_intrin_sse41.h"
#include "compv/base/math/intrin/x86/compv_math_activation_functions_intrin_avx.h"
#include "compv/base/math/intrin/x86/compv_math_activation_functions_intrin_avx2.h"
#include "compv/base/math/intrin/arm/compv_math_activation_functions_intrin_neon64.h"

// More information about activation functions: https://towardsdatascience.com/activation-functions-neural-networks-1cbd9f8d91d6
// Some of these function comes from Tesseract and was adapted to make them SIMD-friendly and branchless

#define COMPV_THIS_CLASSNAME	"CompVMathActivationFunctions"

COMPV_NAMESPACE_BEGIN()

static void CompVMathActivationFunctionsTanh_64f64f_C(const compv_float64_t* lut_ptr, const compv_uscalar_t& lut_length_minus1, const compv_float64_t* scale1, const compv_uscalar_t& in_out_length, const compv_float64_t* in_ptr, compv_float64_t* out_ptr);
static void CompVMathActivationFunctionsTanhMul_64f64f_C(const compv_float64_t* lut_ptr, const compv_uscalar_t& lut_length_minus1, const compv_float64_t* scale1, const compv_uscalar_t& in_out_length, const compv_float64_t* in_ptr, const compv_float64_t* mul_ptr, compv_float64_t* out_ptr);
static void CompVMathActivationFunctionsLogistic_64f64f_C(const compv_float64_t* lut_ptr, const compv_uscalar_t& lut_length_minus1, const compv_float64_t* scale1, const compv_uscalar_t& in_out_length, const compv_float64_t* in_ptr, compv_float64_t* out_ptr);
static void CompVMathActivationFunctionsLogisticMul_64f64f_C(const compv_float64_t* lut_ptr, const compv_uscalar_t& lut_length_minus1, const compv_float64_t* scale1, const compv_uscalar_t& in_out_length, const compv_float64_t* in_ptr, const compv_float64_t* mul_ptr, compv_float64_t* out_ptr);

COMPV_ERROR_CODE CompVMathActivationFunctions::tanh(const compv_float64_t* lut_ptr, const size_t& lut_length, const compv_float64_t& scale, const size_t& in_out_length, const compv_float64_t* in_ptr, compv_float64_t* out_ptr)
{
	COMPV_CHECK_EXP_RETURN(
		!lut_ptr || !lut_length || !in_out_length || !in_ptr || !out_ptr,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER
	);
	void(*CompVMathActivationFunctionsTanh)(const compv_float64_t* lut_ptr, const compv_uscalar_t& lut_length_minus1, const compv_float64_t* scale1, const compv_uscalar_t& in_out_length, const compv_float64_t* in_ptr, compv_float64_t* out_ptr)
		= CompVMathActivationFunctionsTanh_64f64f_C;
#if COMPV_ARCH_X86
#	if 0 // TODO(dmi): "AVX" version faster -> "_mm256_i32gather_pd" to blame
	if (COMPV_IS_ALIGNED(in_out_length, 4) && CompVCpu::isEnabled(kCpuFlagAVX2)) {
		COMPV_EXEC_IFDEF_INTRIN_X86((CompVMathActivationFunctionsTanh = CompVMathActivationFunctionsTanh_64f64f_Intrin_AVX2));
	}
	else
#	endif
	if (COMPV_IS_ALIGNED(in_out_length, 4) && CompVCpu::isEnabled(kCpuFlagAVX)) {
		COMPV_EXEC_IFDEF_INTRIN_X86((CompVMathActivationFunctionsTanh = CompVMathActivationFunctionsTanh_64f64f_Intrin_AVX));
	}
	else if (COMPV_IS_ALIGNED(in_out_length, 2) && CompVCpu::isEnabled(kCpuFlagSSE41)) {
		COMPV_EXEC_IFDEF_INTRIN_X86((CompVMathActivationFunctionsTanh = CompVMathActivationFunctionsTanh_64f64f_Intrin_SSE41));
	}
#elif COMPV_ARCH_ARM
	if (COMPV_IS_ALIGNED(in_out_length, 2) && CompVCpu::isEnabled(kCpuFlagARM_NEON_FMA)) {
		COMPV_EXEC_IFDEF_INTRIN_ARM64((CompVMathActivationFunctionsTanh = CompVMathActivationFunctionsTanh_64f64f_Intrin_NEON64));
	}
#endif
	
	CompVMathActivationFunctionsTanh(
		lut_ptr, lut_length - 1,
		&scale, 
		in_out_length, in_ptr, out_ptr
	);

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathActivationFunctions::tanhMul(const compv_float64_t* lut_ptr, const size_t& lut_length, const compv_float64_t& scale, const size_t& in_out_length, const compv_float64_t* in_ptr, const compv_float64_t* mul_ptr, compv_float64_t* out_ptr)
{
	COMPV_CHECK_EXP_RETURN(
		!lut_ptr || !lut_length || !in_out_length || !in_ptr || !mul_ptr || !out_ptr,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER
	);
	void(*CompVMathActivationFunctionsTanhMul)(const compv_float64_t* lut_ptr, const compv_uscalar_t& lut_length_minus1, const compv_float64_t* scale1, const compv_uscalar_t& in_out_length, const compv_float64_t* in_ptr, const compv_float64_t* mul_ptr, compv_float64_t* out_ptr)
		= CompVMathActivationFunctionsTanhMul_64f64f_C;
#if COMPV_ARCH_X86
		if (COMPV_IS_ALIGNED(in_out_length, 4) && CompVCpu::isEnabled(kCpuFlagAVX)) {
			COMPV_EXEC_IFDEF_INTRIN_X86((CompVMathActivationFunctionsTanhMul = CompVMathActivationFunctionsTanhMul_64f64f_Intrin_AVX));
		}
		else if (COMPV_IS_ALIGNED(in_out_length, 2) && CompVCpu::isEnabled(kCpuFlagSSE41)) {
			COMPV_EXEC_IFDEF_INTRIN_X86((CompVMathActivationFunctionsTanhMul = CompVMathActivationFunctionsTanhMul_64f64f_Intrin_SSE41));
		}
#elif COMPV_ARCH_ARM
	if (COMPV_IS_ALIGNED(in_out_length, 2) && CompVCpu::isEnabled(kCpuFlagARM_NEON_FMA)) {
		COMPV_EXEC_IFDEF_INTRIN_ARM64((CompVMathActivationFunctionsTanhMul = CompVMathActivationFunctionsTanhMul_64f64f_Intrin_NEON64));
	}
#endif

	CompVMathActivationFunctionsTanhMul(
		lut_ptr, lut_length - 1,
		&scale,
		in_out_length, in_ptr, mul_ptr, out_ptr
	);

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathActivationFunctions::logistic(const compv_float64_t* lut_ptr, const size_t& lut_length, const compv_float64_t& scale, const size_t& in_out_length, const compv_float64_t* in_ptr, compv_float64_t* out_ptr)
{
	COMPV_CHECK_EXP_RETURN(
		!lut_ptr || !lut_length || !in_out_length || !in_ptr || !out_ptr,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER
	);
	void(*CompVMathActivationFunctionsLogistic)(const compv_float64_t* lut_ptr, const compv_uscalar_t& lut_length_minus1, const compv_float64_t* scale1, const compv_uscalar_t& in_out_length, const compv_float64_t* in_ptr, compv_float64_t* out_ptr)
		= CompVMathActivationFunctionsLogistic_64f64f_C;
#if COMPV_ARCH_X86
	if (COMPV_IS_ALIGNED(in_out_length, 4) && CompVCpu::isEnabled(kCpuFlagAVX)) {
		COMPV_EXEC_IFDEF_INTRIN_X86((CompVMathActivationFunctionsLogistic = CompVMathActivationFunctionsLogistic_64f64f_Intrin_AVX));
	}
	else if (COMPV_IS_ALIGNED(in_out_length, 2) && CompVCpu::isEnabled(kCpuFlagSSE41)) {
		COMPV_EXEC_IFDEF_INTRIN_X86((CompVMathActivationFunctionsLogistic = CompVMathActivationFunctionsLogistic_64f64f_Intrin_SSE41));
	}
#elif COMPV_ARCH_ARM
	if (COMPV_IS_ALIGNED(in_out_length, 2) && CompVCpu::isEnabled(kCpuFlagARM_NEON_FMA)) {
		COMPV_EXEC_IFDEF_INTRIN_ARM64((CompVMathActivationFunctionsLogistic = CompVMathActivationFunctionsLogistic_64f64f_Intrin_NEON64));
	}
#endif

	CompVMathActivationFunctionsLogistic(
		lut_ptr, lut_length - 1, 
		&scale, 
		in_out_length, in_ptr, out_ptr
	);

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathActivationFunctions::logisticMul(const compv_float64_t* lut_ptr, const size_t& lut_length, const compv_float64_t& scale, const size_t& in_out_length, const compv_float64_t* in_ptr, const compv_float64_t* mul_ptr, compv_float64_t* out_ptr)
{
	COMPV_CHECK_EXP_RETURN(
		!lut_ptr || !lut_length || !in_out_length || !in_ptr || !mul_ptr || !out_ptr,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER
	);
	void(*CompVMathActivationFunctionsLogisticMul)(const compv_float64_t* lut_ptr, const compv_uscalar_t& lut_length_minus1, const compv_float64_t* scale1, const compv_uscalar_t& in_out_length, const compv_float64_t* in_ptr, const compv_float64_t* mul_ptr, compv_float64_t* out_ptr)
		= CompVMathActivationFunctionsLogisticMul_64f64f_C;
#if COMPV_ARCH_X86
	if (COMPV_IS_ALIGNED(in_out_length, 4) && CompVCpu::isEnabled(kCpuFlagAVX)) {
		COMPV_EXEC_IFDEF_INTRIN_X86((CompVMathActivationFunctionsLogisticMul = CompVMathActivationFunctionsLogisticMul_64f64f_Intrin_AVX));
	}
	else if (COMPV_IS_ALIGNED(in_out_length, 2) && CompVCpu::isEnabled(kCpuFlagSSE41)) {
		COMPV_EXEC_IFDEF_INTRIN_X86((CompVMathActivationFunctionsLogisticMul = CompVMathActivationFunctionsLogisticMul_64f64f_Intrin_SSE41));
	}
#elif COMPV_ARCH_ARM
	if (COMPV_IS_ALIGNED(in_out_length, 2) && CompVCpu::isEnabled(kCpuFlagARM_NEON_FMA)) {
		COMPV_EXEC_IFDEF_INTRIN_ARM64((CompVMathActivationFunctionsLogisticMul = CompVMathActivationFunctionsLogisticMul_64f64f_Intrin_NEON64));
	}
#endif

	CompVMathActivationFunctionsLogisticMul(
		lut_ptr, lut_length - 1,
		&scale,
		in_out_length, in_ptr, mul_ptr, out_ptr
	);

	return COMPV_ERROR_CODE_S_OK;

	return COMPV_ERROR_CODE_S_OK;
}

static void CompVMathActivationFunctionsTanh_64f64f_C(const compv_float64_t* lut_ptr, const compv_uscalar_t& lut_length_minus1, const compv_float64_t* scale1, const compv_uscalar_t& in_out_length, const compv_float64_t* in_ptr, compv_float64_t* out_ptr)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation could be found.");
	const compv_float64_t& scale = *scale1;
	for (compv_uscalar_t i = 0; i < in_out_length; ++i) {
		compv_float64_t x = in_ptr[i];
		const compv_float64_t sign = (x < 0.0) ? -1.0 : 1.0;
		x = (x * sign) * scale;
		const compv_uscalar_t index = static_cast<compv_uscalar_t>(x);
		if (index < lut_length_minus1) {
			const compv_float64_t tanh_i0 = lut_ptr[index];
			const compv_float64_t tanh_i1 = lut_ptr[index + 1];
			out_ptr[i] = (tanh_i0 + (tanh_i1 - tanh_i0) * (x - index)) * sign;
		}
		else {
			out_ptr[i] = sign;
		}
	}
}

static void CompVMathActivationFunctionsTanhMul_64f64f_C(const compv_float64_t* lut_ptr, const compv_uscalar_t& lut_length_minus1, const compv_float64_t* scale1, const compv_uscalar_t& in_out_length, const compv_float64_t* in_ptr, const compv_float64_t* mul_ptr, compv_float64_t* out_ptr)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation could be found.");
	CompVMathActivationFunctionsTanh_64f64f_C(lut_ptr, lut_length_minus1, scale1, in_out_length, in_ptr, out_ptr);
	for (size_t i = 0; i < in_out_length; ++i) {
		out_ptr[i] *= mul_ptr[i];
	}
}

static void CompVMathActivationFunctionsLogistic_64f64f_C(const compv_float64_t* lut_ptr, const compv_uscalar_t& lut_length_minus1, const compv_float64_t* scale1, const compv_uscalar_t& in_out_length, const compv_float64_t* in_ptr, compv_float64_t* out_ptr)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation could be found.");
	const compv_float64_t& scale = *scale1;
	for (compv_uscalar_t i = 0; i < in_out_length; ++i) {
		compv_float64_t x = in_ptr[i];
		const compv_float64_t mask = (x < 0.0);
		const compv_float64_t sign = mask ? -1.0 : 1.0;
		x = (x * sign) * scale;
		const compv_uscalar_t index = static_cast<compv_uscalar_t>(x);
		if (index < lut_length_minus1) {
			const compv_float64_t l0 = lut_ptr[index];
			const compv_float64_t l1 = lut_ptr[index + 1];
			out_ptr[i] = ((l0 + (l1 - l0) * (x - index)) * sign) + mask;
		}
		else {
			out_ptr[i] = sign + mask;
		}
	}
}

static void CompVMathActivationFunctionsLogisticMul_64f64f_C(const compv_float64_t* lut_ptr, const compv_uscalar_t& lut_length_minus1, const compv_float64_t* scale1, const compv_uscalar_t& in_out_length, const compv_float64_t* in_ptr, const compv_float64_t* mul_ptr, compv_float64_t* out_ptr)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation could be found.");
	CompVMathActivationFunctionsLogistic_64f64f_C(lut_ptr, lut_length_minus1, scale1, in_out_length, in_ptr, out_ptr);
	for (size_t i = 0; i < in_out_length; ++i) {
		out_ptr[i] *= mul_ptr[i];
	}
}

COMPV_NAMESPACE_END()

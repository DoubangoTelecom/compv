/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_activation_functions.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_cpu.h"

#include "compv/base/math/intrin/x86/compv_math_activation_functions_intrin_sse2.h"
#include "compv/base/math/intrin/x86/compv_math_activation_functions_intrin_sse41.h"
#include "compv/base/math/intrin/x86/compv_math_activation_functions_intrin_avx2.h"
#include "compv/base/math/intrin/arm/compv_math_activation_functions_intrin_neon.h"

// More information about activation functions: https://towardsdatascience.com/activation-functions-neural-networks-1cbd9f8d91d6
// Some of these function comes from Tesseract and was adapted to make them SIMD-friendly and branchless

#define COMPV_THIS_CLASSNAME	"CompVMathActivationFunctions"

COMPV_NAMESPACE_BEGIN()

const float kMaxSoftmaxActivation = -86;
extern float ___expf_c(float x); // declared in "compv_math_exp.cxx"

static void CompVMathActivationFunctionsTanh_32f32f_C(const compv_float32_t* lut_ptr, const compv_uscalar_t& lut_length_minus1, const compv_float32_t* scale1, const compv_uscalar_t& in_out_length, const compv_float32_t* in_ptr, compv_float32_t* out_ptr);
static void CompVMathActivationFunctionsTanhMul_32f32f_C(const compv_float32_t* lut_ptr, const compv_uscalar_t& lut_length_minus1, const compv_float32_t* scale1, const compv_uscalar_t& in_out_length, const compv_float32_t* in_ptr, const compv_float32_t* mul_ptr, compv_float32_t* out_ptr);
static void CompVMathActivationFunctionsLogistic_32f32f_C(const compv_float32_t* lut_ptr, const compv_uscalar_t& lut_length_minus1, const compv_float32_t* scale1, const compv_uscalar_t& in_out_length, const compv_float32_t* in_ptr, compv_float32_t* out_ptr);
static void CompVMathActivationFunctionsLogisticMul_32f32f_C(const compv_float32_t* lut_ptr, const compv_uscalar_t& lut_length_minus1, const compv_float32_t* scale1, const compv_uscalar_t& in_out_length, const compv_float32_t* in_ptr, const compv_float32_t* mul_ptr, compv_float32_t* out_ptr);
static void CompVMathActivationFunctionsSoftmaxInPlace_32f32f_C(const compv_uscalar_t& in_out_length, compv_float32_t* in_out_ptr);

COMPV_ERROR_CODE CompVMathActivationFunctions::tanh(const compv_float32_t* lut_ptr, const size_t& lut_length, const compv_float32_t& scale, const size_t& in_out_length, const compv_float32_t* in_ptr, compv_float32_t* out_ptr)
{
	COMPV_CHECK_EXP_RETURN(
		!lut_ptr || !lut_length || !in_out_length || !in_ptr || !out_ptr,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER
	);
	void(*CompVMathActivationFunctionsTanh)(const compv_float32_t* lut_ptr, const compv_uscalar_t& lut_length_minus1, const compv_float32_t* scale1, const compv_uscalar_t& in_out_length, const compv_float32_t* in_ptr, compv_float32_t* out_ptr)
		= CompVMathActivationFunctionsTanh_32f32f_C;
#if COMPV_ARCH_X86
	if (COMPV_IS_ALIGNED(in_out_length, 4) && CompVCpu::isEnabled(kCpuFlagSSE41)) {
		COMPV_EXEC_IFDEF_INTRIN_X86((CompVMathActivationFunctionsTanh = CompVMathActivationFunctionsTanh_32f32f_Intrin_SSE41));
	}
	if (COMPV_IS_ALIGNED(in_out_length, 8) && CompVCpu::isEnabled(kCpuFlagAVX2)) {
		COMPV_EXEC_IFDEF_INTRIN_X86((CompVMathActivationFunctionsTanh = CompVMathActivationFunctionsTanh_32f32f_Intrin_AVX2));
	}
#elif COMPV_ARCH_ARM
	if (COMPV_IS_ALIGNED(in_out_length, 4) && CompVCpu::isEnabled(kCpuFlagARM_NEON)) {
		COMPV_EXEC_IFDEF_INTRIN_ARM((CompVMathActivationFunctionsTanh = CompVMathActivationFunctionsTanh_32f32f_Intrin_NEON));
	}
#endif
	
	CompVMathActivationFunctionsTanh(
		lut_ptr, lut_length - 1,
		&scale, 
		in_out_length, in_ptr, out_ptr
	);

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathActivationFunctions::tanhMul(const compv_float32_t* lut_ptr, const size_t& lut_length, const compv_float32_t& scale, const size_t& in_out_length, const compv_float32_t* in_ptr, const compv_float32_t* mul_ptr, compv_float32_t* out_ptr)
{
	COMPV_CHECK_EXP_RETURN(
		!lut_ptr || !lut_length || !in_out_length || !in_ptr || !mul_ptr || !out_ptr,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER
	);
	void(*CompVMathActivationFunctionsTanhMul)(const compv_float32_t* lut_ptr, const compv_uscalar_t& lut_length_minus1, const compv_float32_t* scale1, const compv_uscalar_t& in_out_length, const compv_float32_t* in_ptr, const compv_float32_t* mul_ptr, compv_float32_t* out_ptr)
		= CompVMathActivationFunctionsTanhMul_32f32f_C;
#if COMPV_ARCH_X86
		if (COMPV_IS_ALIGNED(in_out_length, 4) && CompVCpu::isEnabled(kCpuFlagSSE41)) {
			COMPV_EXEC_IFDEF_INTRIN_X86((CompVMathActivationFunctionsTanhMul = CompVMathActivationFunctionsTanhMul_32f32f_Intrin_SSE41));
		}
		if (COMPV_IS_ALIGNED(in_out_length, 8) && CompVCpu::isEnabled(kCpuFlagAVX2)) {
			COMPV_EXEC_IFDEF_INTRIN_X86((CompVMathActivationFunctionsTanhMul = CompVMathActivationFunctionsTanhMul_32f32f_Intrin_AVX2));
		}
#elif COMPV_ARCH_ARM
	if (COMPV_IS_ALIGNED(in_out_length, 4) && CompVCpu::isEnabled(kCpuFlagARM_NEON)) {
		COMPV_EXEC_IFDEF_INTRIN_ARM((CompVMathActivationFunctionsTanhMul = CompVMathActivationFunctionsTanhMul_32f32f_Intrin_NEON));
	}
#endif

	CompVMathActivationFunctionsTanhMul(
		lut_ptr, lut_length - 1,
		&scale,
		in_out_length, in_ptr, mul_ptr, out_ptr
	);

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathActivationFunctions::logistic(const compv_float32_t* lut_ptr, const size_t& lut_length, const compv_float32_t& scale, const size_t& in_out_length, const compv_float32_t* in_ptr, compv_float32_t* out_ptr)
{
	COMPV_CHECK_EXP_RETURN(
		!lut_ptr || !lut_length || !in_out_length || !in_ptr || !out_ptr,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER
	);
	void(*CompVMathActivationFunctionsLogistic)(const compv_float32_t* lut_ptr, const compv_uscalar_t& lut_length_minus1, const compv_float32_t* scale1, const compv_uscalar_t& in_out_length, const compv_float32_t* in_ptr, compv_float32_t* out_ptr)
		= CompVMathActivationFunctionsLogistic_32f32f_C;
#if COMPV_ARCH_X86
	if (COMPV_IS_ALIGNED(in_out_length, 4) && CompVCpu::isEnabled(kCpuFlagSSE41)) {
		COMPV_EXEC_IFDEF_INTRIN_X86((CompVMathActivationFunctionsLogistic = CompVMathActivationFunctionsLogistic_32f32f_Intrin_SSE41));
	}
	if (COMPV_IS_ALIGNED(in_out_length, 8) && CompVCpu::isEnabled(kCpuFlagAVX2)) {
		COMPV_EXEC_IFDEF_INTRIN_X86((CompVMathActivationFunctionsLogistic = CompVMathActivationFunctionsLogistic_32f32f_Intrin_AVX2));
	}
#elif COMPV_ARCH_ARM
	if (COMPV_IS_ALIGNED(in_out_length, 4) && CompVCpu::isEnabled(kCpuFlagARM_NEON)) {
		COMPV_EXEC_IFDEF_INTRIN_ARM((CompVMathActivationFunctionsLogistic = CompVMathActivationFunctionsLogistic_32f32f_Intrin_NEON));
	}
#endif

	CompVMathActivationFunctionsLogistic(
		lut_ptr, lut_length - 1, 
		&scale, 
		in_out_length, in_ptr, out_ptr
	);

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathActivationFunctions::logisticMul(const compv_float32_t* lut_ptr, const size_t& lut_length, const compv_float32_t& scale, const size_t& in_out_length, const compv_float32_t* in_ptr, const compv_float32_t* mul_ptr, compv_float32_t* out_ptr)
{
	COMPV_CHECK_EXP_RETURN(
		!lut_ptr || !lut_length || !in_out_length || !in_ptr || !mul_ptr || !out_ptr,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER
	);
	void(*CompVMathActivationFunctionsLogisticMul)(const compv_float32_t* lut_ptr, const compv_uscalar_t& lut_length_minus1, const compv_float32_t* scale1, const compv_uscalar_t& in_out_length, const compv_float32_t* in_ptr, const compv_float32_t* mul_ptr, compv_float32_t* out_ptr)
		= CompVMathActivationFunctionsLogisticMul_32f32f_C;
#if COMPV_ARCH_X86
	if (COMPV_IS_ALIGNED(in_out_length, 4) && CompVCpu::isEnabled(kCpuFlagSSE41)) {
		COMPV_EXEC_IFDEF_INTRIN_X86((CompVMathActivationFunctionsLogisticMul = CompVMathActivationFunctionsLogisticMul_32f32f_Intrin_SSE41));
	}
	if (COMPV_IS_ALIGNED(in_out_length, 8) && CompVCpu::isEnabled(kCpuFlagAVX2)) {
		COMPV_EXEC_IFDEF_INTRIN_X86((CompVMathActivationFunctionsLogisticMul = CompVMathActivationFunctionsLogisticMul_32f32f_Intrin_AVX2));
	}
#elif COMPV_ARCH_ARM
	if (COMPV_IS_ALIGNED(in_out_length, 4) && CompVCpu::isEnabled(kCpuFlagARM_NEON)) {
		COMPV_EXEC_IFDEF_INTRIN_ARM((CompVMathActivationFunctionsLogisticMul = CompVMathActivationFunctionsLogisticMul_32f32f_Intrin_NEON));
	}
#endif

	CompVMathActivationFunctionsLogisticMul(
		lut_ptr, lut_length - 1,
		&scale,
		in_out_length, in_ptr, mul_ptr, out_ptr
	);

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathActivationFunctions::softmaxInPlace(const size_t& in_out_length, compv_float32_t* in_out_ptr)
{
	COMPV_CHECK_EXP_RETURN(
		!in_out_length || !in_out_ptr,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER
	);
	void(*CompVMathActivationFunctionsSoftmaxInPlace)(const compv_uscalar_t& in_out_length, compv_float32_t* in_out_ptr)
		= CompVMathActivationFunctionsSoftmaxInPlace_32f32f_C;
#if COMPV_ARCH_X86
	if ((in_out_length >= 4) && CompVCpu::isEnabled(kCpuFlagSSE2)) {
		COMPV_EXEC_IFDEF_INTRIN_X86((CompVMathActivationFunctionsSoftmaxInPlace = CompVMathActivationFunctionsSoftmaxInPlace_32f32f_Intrin_SSE2));
	}
#elif COMPV_ARCH_ARM
	if ((in_out_length >= 4) && CompVCpu::isEnabled(kCpuFlagARM_NEON)) {
		COMPV_EXEC_IFDEF_INTRIN_ARM((CompVMathActivationFunctionsSoftmaxInPlace = CompVMathActivationFunctionsSoftmaxInPlace_32f32f_Intrin_NEON));
	}
#endif

	CompVMathActivationFunctionsSoftmaxInPlace(
		in_out_length, in_out_ptr
	);

	return COMPV_ERROR_CODE_S_OK;
}

static void CompVMathActivationFunctionsTanh_32f32f_C(const compv_float32_t* lut_ptr, const compv_uscalar_t& lut_length_minus1, const compv_float32_t* scale1, const compv_uscalar_t& in_out_length, const compv_float32_t* in_ptr, compv_float32_t* out_ptr)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation could be found.");
	const compv_float32_t& scale = *scale1;
	for (compv_uscalar_t i = 0; i < in_out_length; ++i) {
		compv_float32_t x = in_ptr[i];
		const compv_float32_t sign = (x < 0.0) ? -1.f : 1.f;
		x = (x * sign) * scale;
		const compv_uscalar_t index = static_cast<compv_uscalar_t>(x);
		if (index < lut_length_minus1) {
			const compv_float32_t tanh_i0 = lut_ptr[index];
			const compv_float32_t tanh_i1 = lut_ptr[index + 1];
			out_ptr[i] = (tanh_i0 + (tanh_i1 - tanh_i0) * (x - index)) * sign;
		}
		else {
			out_ptr[i] = sign;
		}
	}
}

static void CompVMathActivationFunctionsTanhMul_32f32f_C(const compv_float32_t* lut_ptr, const compv_uscalar_t& lut_length_minus1, const compv_float32_t* scale1, const compv_uscalar_t& in_out_length, const compv_float32_t* in_ptr, const compv_float32_t* mul_ptr, compv_float32_t* out_ptr)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation could be found.");
	CompVMathActivationFunctionsTanh_32f32f_C(lut_ptr, lut_length_minus1, scale1, in_out_length, in_ptr, out_ptr);
	for (size_t i = 0; i < in_out_length; ++i) {
		out_ptr[i] *= mul_ptr[i];
	}
}

static void CompVMathActivationFunctionsLogistic_32f32f_C(const compv_float32_t* lut_ptr, const compv_uscalar_t& lut_length_minus1, const compv_float32_t* scale1, const compv_uscalar_t& in_out_length, const compv_float32_t* in_ptr, compv_float32_t* out_ptr)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation could be found.");
	const compv_float32_t& scale = *scale1;
	for (compv_uscalar_t i = 0; i < in_out_length; ++i) {
		compv_float32_t x = in_ptr[i];
		const compv_float32_t mask = (x < 0.0);
		const compv_float32_t sign = mask ? -1.f : 1.f;
		x = (x * sign) * scale;
		const compv_uscalar_t index = static_cast<compv_uscalar_t>(x);
		if (index < lut_length_minus1) {
			const compv_float32_t l0 = lut_ptr[index];
			const compv_float32_t l1 = lut_ptr[index + 1];
			out_ptr[i] = ((l0 + (l1 - l0) * (x - index)) * sign) + mask;
		}
		else {
			out_ptr[i] = sign + mask;
		}
	}
}

static void CompVMathActivationFunctionsLogisticMul_32f32f_C(const compv_float32_t* lut_ptr, const compv_uscalar_t& lut_length_minus1, const compv_float32_t* scale1, const compv_uscalar_t& in_out_length, const compv_float32_t* in_ptr, const compv_float32_t* mul_ptr, compv_float32_t* out_ptr)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation could be found.");
	CompVMathActivationFunctionsLogistic_32f32f_C(lut_ptr, lut_length_minus1, scale1, in_out_length, in_ptr, out_ptr);
	for (size_t i = 0; i < in_out_length; ++i) {
		out_ptr[i] *= mul_ptr[i];
	}
}

static void CompVMathActivationFunctionsSoftmaxInPlace_32f32f_C(const compv_uscalar_t& in_out_length, compv_float32_t* in_out_ptr)
{
	float max_output = in_out_ptr[0];
	for (compv_uscalar_t i = 1; i < in_out_length; i++) {
		const float& output = in_out_ptr[i];
		max_output = COMPV_MATH_MAX(max_output, output);
	}

	float prob_total = 0;
	for (compv_uscalar_t i = 0; i < in_out_length; i++) {
		float prob = in_out_ptr[i] - max_output;
		prob = ___expf_c(std::min(std::max(kMaxSoftmaxActivation, prob), 0.f));
		prob_total += prob;
		in_out_ptr[i] = prob;
	}

	if (prob_total > 0.f && prob_total != 1.f) {
		const float prob_total_scale = 1 / prob_total;
		for (compv_uscalar_t i = 0; i < in_out_length; i++) {
			in_out_ptr[i] *= prob_total_scale;
		}
	}
}

COMPV_NAMESPACE_END()

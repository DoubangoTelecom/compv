/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_scale.h"
#include "compv/base/compv_generic_invoke.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/compv_cpu.h"

#include "compv/base/math/intrin/x86/compv_math_scale_intrin_sse2.h"
#include "compv/base/math/intrin/arm/compv_math_scale_intrin_neon.h"

COMPV_NAMESPACE_BEGIN()

#if COMPV_ASM && COMPV_ARCH_X64
COMPV_EXTERNC void CompVMathScaleScale_64f64f_Asm_X64_SSE2(const compv_float64_t* ptrIn, compv_float64_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride, const compv_float64_t* s1);
COMPV_EXTERNC void CompVMathScaleScale_64f64f_Asm_X64_AVX(const compv_float64_t* ptrIn, compv_float64_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride, const compv_float64_t* s1);

COMPV_EXTERNC void CompVMathScaleScale_32f32f_Asm_X64_SSE2(COMPV_ALIGNED(SSE) const compv_float32_t* ptrIn, COMPV_ALIGNED(SSE) compv_float32_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(SSE) const compv_uscalar_t stride, const compv_float32_t* s1);
COMPV_EXTERNC void CompVMathScaleScale_32f32f_Asm_X64_AVX(COMPV_ALIGNED(AVX) const compv_float32_t* ptrIn, COMPV_ALIGNED(AVX) compv_float32_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(AVX) const compv_uscalar_t stride, const compv_float32_t* s1);
#endif /* #if COMPV_ASM && COMPV_ARCH_X64 */

#if COMPV_ASM && COMPV_ARCH_ARM32
COMPV_EXTERNC void CompVMathScaleScale_32f32f_Asm_NEON32(COMPV_ALIGNED(NEON) const compv_float32_t* ptrIn, COMPV_ALIGNED(NEON) compv_float32_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(NEON) const compv_uscalar_t stride, const compv_float32_t* s1);
#endif /* COMPV_ARCH_ARM32 */

#if COMPV_ASM && COMPV_ARCH_ARM64
COMPV_EXTERNC void CompVMathScaleScale_32f32f_Asm_NEON64(COMPV_ALIGNED(NEON) const compv_float32_t* ptrIn, COMPV_ALIGNED(NEON) compv_float32_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(NEON) const compv_uscalar_t stride, const compv_float32_t* s1);
#endif /* COMPV_ARCH_ARM32 */

template<typename T>
static void CompVMathScaleScale_C(const T* ptrIn, T* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride, const T* s1)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPGPU implementation could be found");
	const T& s = *s1;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			ptrOut[i] = ptrIn[i] * s;
		}
		ptrIn += stride;
		ptrOut += stride;
	}
}

template<typename T>
static COMPV_ERROR_CODE CompVMathScaleScale(const CompVMatPtr &in, const double& s, CompVMatPtrPtr out)
{
	const size_t rows = in->rows();
	const size_t cols = in->cols();
	const size_t stride = in->stride();

	CompVMatPtr out_ = *out;
	if (out_ != in) { // This function allows "in == out"
		COMPV_CHECK_CODE_RETURN(CompVMat::newObj(&out_, in));
	}
		
	const T ss = static_cast<T>(s);
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		const T* ptrIn = in->ptr<const T>(ystart);
		T* ptrOut = out_->ptr<T>(ystart);
		if (std::is_same<T, compv_float64_t>::value) {
			void(*CompVMathScale_64f64f)(const compv_float64_t* ptrIn, compv_float64_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride, const compv_float64_t* s1)
				= nullptr;
			COMPV_CHECK_CODE_RETURN(CompVMathScale::hookScale_64f(&CompVMathScale_64f64f));
			CompVMathScale_64f64f(
				reinterpret_cast<const compv_float64_t*>(ptrIn), reinterpret_cast<compv_float64_t*>(ptrOut),
				cols, (yend - ystart), stride,
				reinterpret_cast<const compv_float64_t*>(&ss)
			);
		}
		else if (std::is_same<T, compv_float32_t>::value) {
			void(*CompVMathScale_32f32f)(const compv_float32_t* ptrIn, compv_float32_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride, const compv_float32_t* s1)
				= CompVMathScaleScale_C;
#if COMPV_ARCH_X86
			if (CompVCpu::isEnabled(kCpuFlagSSE2) && COMPV_IS_ALIGNED_SSE(ptrIn) && COMPV_IS_ALIGNED_SSE(ptrOut) && COMPV_IS_ALIGNED_SSE(stride * sizeof(compv_float32_t))) {
				COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathScale_32f32f = CompVMathScaleScale_32f32f_Intrin_SSE2);
				COMPV_EXEC_IFDEF_ASM_X64(CompVMathScale_32f32f = CompVMathScaleScale_32f32f_Asm_X64_SSE2);
			}
			if (CompVCpu::isEnabled(kCpuFlagAVX) && COMPV_IS_ALIGNED_AVX(ptrIn) && COMPV_IS_ALIGNED_AVX(ptrOut) && COMPV_IS_ALIGNED_AVX(stride * sizeof(compv_float32_t))) {
				COMPV_EXEC_IFDEF_ASM_X64(CompVMathScale_32f32f = CompVMathScaleScale_32f32f_Asm_X64_AVX);
			}
#elif COMPV_ARCH_ARM
			if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && COMPV_IS_ALIGNED_NEON(ptrIn) && COMPV_IS_ALIGNED_NEON(ptrOut) && COMPV_IS_ALIGNED_NEON(stride * sizeof(compv_float32_t))) {
				COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMathScale_32f32f = CompVMathScaleScale_32f32f_Intrin_NEON);
				COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathScale_32f32f = CompVMathScaleScale_32f32f_Asm_NEON32);
				COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathScale_32f32f = CompVMathScaleScale_32f32f_Asm_NEON64);
			}
#endif
			CompVMathScale_32f32f(
				reinterpret_cast<const compv_float32_t*>(ptrIn), reinterpret_cast<compv_float32_t*>(ptrOut),
				cols, (yend - ystart), stride,
				reinterpret_cast<const compv_float32_t*>(&ss)
			);
		}
		else {
			CompVMathScaleScale_C(
				ptrIn, ptrOut,
				cols, (yend - ystart), stride,
				&ss
			);
		}
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		cols,
		rows,
		(cols * 1)
	));

	*out = out_;
	return COMPV_ERROR_CODE_S_OK;
}

// out[i] = (in[i] * s)
COMPV_ERROR_CODE CompVMathScale::scale(const CompVMatPtr &in, const double& s, CompVMatPtrPtr out)
{
	COMPV_CHECK_EXP_RETURN(!in || !out || in->planeCount() != 1
		, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVGenericFloatInvokeCodeRawType(in->subType(), CompVMathScaleScale, in, s, out);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathScale::hookScale_64f(
	void(**CompVMathScaleScale_64f64f)(const compv_float64_t* ptrIn, compv_float64_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride, const compv_float64_t* s1)
)
{
	COMPV_CHECK_EXP_RETURN(!CompVMathScaleScale_64f64f, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	*CompVMathScaleScale_64f64f = CompVMathScaleScale_C;
#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSE2)) {
		COMPV_EXEC_IFDEF_INTRIN_X86(*CompVMathScaleScale_64f64f = CompVMathScaleScale_64f64f_Intrin_SSE2);
		COMPV_EXEC_IFDEF_ASM_X64(*CompVMathScaleScale_64f64f = CompVMathScaleScale_64f64f_Asm_X64_SSE2);
	}
	if (CompVCpu::isEnabled(kCpuFlagAVX)) {
		COMPV_EXEC_IFDEF_ASM_X64(*CompVMathScaleScale_64f64f = CompVMathScaleScale_64f64f_Asm_X64_AVX);
	}
#elif COMPV_ARCH_ARM
#endif
	return COMPV_ERROR_CODE_S_OK;
}


COMPV_NAMESPACE_END()

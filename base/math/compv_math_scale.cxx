/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_scale.h"
#include "compv/base/compv_generic_invoke.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/compv_cpu.h"


COMPV_NAMESPACE_BEGIN()

template<typename T>
static void CompVMathScaleScale_C(const T* ptrIn, T* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride, const T* s1)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation could be found");
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
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation could be found");
	const size_t rows = in->rows();
	const size_t cols = in->cols();
	const size_t stride = in->stride();

	CompVMatPtr out_ = *out;
	if (out_ != in) { // This function allows "in == out"
		COMPV_CHECK_CODE_RETURN(CompVMat::newObj(&out_, in));
	}

	const T* ptrIn = in->ptr<const T>();
	T* ptrOut = out_->ptr<T>();
	const T ss = static_cast<T>(s);

	CompVMathScaleScale_C(
		ptrIn, ptrOut,
		cols, rows, stride,
		&ss
	);

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
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_exp.h"
#include "compv/base/compv_generic_invoke.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/compv_cpu.h"


COMPV_NAMESPACE_BEGIN()

template<typename T>
static void CompVMathExpExp_C(const T* ptrIn, T* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation could be found");
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			ptrOut[i] = std::exp(ptrIn[i]);
		}
		ptrIn += stride;
		ptrOut += stride;
	}
}

template<typename T>
static COMPV_ERROR_CODE CompVMathExpExp(const CompVMatPtr &in, CompVMatPtrPtr out)
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

	CompVMathExpExp_C(
		ptrIn, ptrOut,
		cols, rows, stride
	);

	*out = out_;
	return COMPV_ERROR_CODE_S_OK;
}

// out[i] = std::exp(in[i])
COMPV_ERROR_CODE CompVMathExp::exp(const CompVMatPtr &in, CompVMatPtrPtr out)
{
	COMPV_CHECK_EXP_RETURN(!in || !out || in->planeCount() != 1
		, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVGenericFloatInvokeCodeRawType(in->subType(), CompVMathExpExp, in, out);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathExp::hookExp_64f(
	void(**CompVMathExpExp_64f64f)(const compv_float64_t* ptrIn, compv_float64_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride)
)
{
	COMPV_CHECK_EXP_RETURN(!CompVMathExpExp_64f64f, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	*CompVMathExpExp_64f64f = CompVMathExpExp_C;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

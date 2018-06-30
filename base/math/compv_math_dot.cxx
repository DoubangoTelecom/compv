/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_dot.h"
#include "compv/base/compv_generic_invoke.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/compv_cpu.h"


COMPV_NAMESPACE_BEGIN()

template<typename T>
static void CompVMathDotDot_C(const T* ptrA, const T* ptrB, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t strideA, const compv_uscalar_t strideB, T* ret)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation could be found");
	T sum = 0;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			sum += ptrA[i] * ptrB[i];
		}
		ptrA += strideA;
		ptrB += strideB;
	}
	*ret = sum;
}

template<typename T>
static void CompVMathDotDotSub_C(const T* ptrA, const T* ptrB, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t strideA, const compv_uscalar_t strideB, T* ret)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation could be found");
	T sum = 0;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			const auto diff = (ptrA[i] - ptrB[i]);
			sum += diff * diff;
		}
		ptrA += strideA;
		ptrB += strideB;
	}
	*ret = sum;
}

template<typename T>
static COMPV_ERROR_CODE CompVMathDotDot(const CompVMatPtr &A, const CompVMatPtr &B, double* ret)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation could be found");
	const size_t rows = A->rows();
	const size_t cols = A->cols();
	const size_t strideA = A->stride();
	const size_t strideB = B->stride();
	const T* ptrA = A->ptr<const T>();
	const T* ptrB = B->ptr<const T>();
	T sum;

	CompVMathDotDot_C(
		ptrA, ptrB,
		cols, rows, strideA, strideB,
		&sum
	);

	*ret = sum;
	return COMPV_ERROR_CODE_S_OK;
}

template<typename T>
static COMPV_ERROR_CODE CompVMathDotDotSub(const CompVMatPtr &A, const CompVMatPtr &B, double* ret)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation could be found");
	const size_t rows = A->rows();
	const size_t cols = A->cols();
	const size_t strideA = A->stride();
	const size_t strideB = B->stride();
	const T* ptrA = A->ptr<const T>();
	const T* ptrB = B->ptr<const T>();
	T sum;

	CompVMathDotDotSub_C(
		ptrA, ptrB,
		cols, rows, strideA, strideB,
		&sum
	);

	*ret = sum;
	return COMPV_ERROR_CODE_S_OK;
}

// ret = Dot_product(A, B)
// https://en.wikipedia.org/wiki/Dot_product
COMPV_ERROR_CODE CompVMathDot::dot(const CompVMatPtr &A, const CompVMatPtr &B, double* ret)
{
	COMPV_CHECK_EXP_RETURN(!A || !B || !ret || A->cols() != B->cols() || A->rows() != B->rows() || A->subType() != B->subType() || A->planeCount() != B->planeCount()
		, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVGenericFloatInvokeCodeRawType(A->subType(), CompVMathDotDot, A, B, ret);
	return COMPV_ERROR_CODE_S_OK;
}

// ret = Dot_product((A-B), (A-B))
// https://en.wikipedia.org/wiki/Dot_product
COMPV_ERROR_CODE CompVMathDot::dotSub(const CompVMatPtr &A, const CompVMatPtr &B, double* ret)
{
	COMPV_CHECK_EXP_RETURN(!A || !B || !ret || A->cols() != B->cols() || A->rows() != B->rows() || A->subType() != B->subType() || A->planeCount() != B->planeCount()
		, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVGenericFloatInvokeCodeRawType(A->subType(), CompVMathDotDotSub, A, B, ret);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathDot::hookDotSub_64f(
	void(**CompVMathDotDotSub_64f64f)(const compv_float64_t* ptrA, const compv_float64_t* ptrB, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t strideA, const compv_uscalar_t strideB, compv_float64_t* ret)
)
{
	COMPV_CHECK_EXP_RETURN(!CompVMathDotDotSub_64f64f, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	*CompVMathDotDotSub_64f64f = CompVMathDotDotSub_C;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathDot::hookDot_64f(
	void(**CompVMathDotDot_64f64f)(const compv_float64_t* ptrA, const compv_float64_t* ptrB, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t strideA, const compv_uscalar_t strideB, compv_float64_t* ret)
)
{
	COMPV_CHECK_EXP_RETURN(!CompVMathDotDot_64f64f, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	*CompVMathDotDot_64f64f = CompVMathDotDot_C;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

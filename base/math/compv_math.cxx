/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
/*
Most of trig approx. are implemented using document at "documentation/trig_approximations.pdf"
*/
#include "compv/base/math/compv_math.h"
#include "compv/base/math/compv_math_trig.h"
#include "compv/base/math/compv_math_op_sub.h"
#include "compv/base/math/compv_math_op_mul.h"
#include "compv/base/math/compv_math_op_minmax.h"
#include "compv/base/math/compv_math_clip.h"
#include "compv/base/math/compv_math_dot.h"
#include "compv/base/math/compv_math_scale.h"
#include "compv/base/math/compv_math_exp.h"
#include "compv/base/math/compv_math_activation_functions.h"
#include "compv/base/parallel/compv_parallel.h"

COMPV_NAMESPACE_BEGIN()

#define COMPV_MATH_ACTIVATION_FUNCTIONS_MIN_SAMPLES_PER_THREAD	(50 * 50)

COMPV_BASE_API const float kfMathTrigPi = 3.1415926535897932384626433f; // PI
COMPV_BASE_API const float kfMathTrigPiTimes2 = 2.f * kfMathTrigPi; // PI * 2
COMPV_BASE_API const float kfMathTrigPiOver2 = kfMathTrigPi / 2.f; // PI / 2
COMPV_BASE_API const float kfMathTrigPiOver180 = kfMathTrigPi / 180.f; // PI/180
COMPV_BASE_API const float kfMathTrig180OverPi = 180.f / kfMathTrigPi; // 180/PI

COMPV_BASE_API const float kMathTrigC1 = 0.99940307f;
COMPV_BASE_API const float kMathTrigC2 = -0.49558072f;
COMPV_BASE_API const float kMathTrigC3 = 0.03679168f;

// IMPORTANT: ARM32 and ARM64 asm code have theses numbers embedded which means "NEVER CHANGE THEM". Or if you really think it's a good idea to change them then, also
// change them in "compv_math_trig_arm32_neon.S" and "compv_math_trig_arm64_neon.S"
COMPV_BASE_API const double kMathTrigAtan2Eps = 2.2204460492503131e-016;
COMPV_BASE_API const float kMathTrigAtan2P1 = 57.2836266f;
COMPV_BASE_API const float kMathTrigAtan2P3 = -18.6674461f;
COMPV_BASE_API const float kMathTrigAtan2P5 = 8.91400051f;
COMPV_BASE_API const float kMathTrigAtan2P7 = -2.53972459f;

COMPV_ERROR_CODE CompVMath::fastAtan2(const CompVMatPtr& y, const CompVMatPtr& x, CompVMatPtrPtr r, const bool angleInDeg)
{
	COMPV_CHECK_CODE_RETURN(CompVMathTrig::fastAtan2(y, x, r, angleInDeg));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMath::hypot(const CompVMatPtr& x, const CompVMatPtr& y, CompVMatPtrPtr r)
{
	COMPV_CHECK_CODE_RETURN(CompVMathTrig::hypot(x, y, r));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMath::hypot_naive(const CompVMatPtr& x, const CompVMatPtr& y, CompVMatPtrPtr r)
{
	COMPV_CHECK_CODE_RETURN(CompVMathTrig::hypot_naive(x, y, r));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMath::sub(const CompVMatPtr& A, const CompVMatPtr& B, CompVMatPtrPtr R, const bool enforceSingleThread COMPV_DEFAULT(false))
{
	COMPV_CHECK_CODE_RETURN(CompVMathOpSub::sub(A, B, R, enforceSingleThread));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMath::sub(const CompVMatPtr& A, const double& subVal, CompVMatPtrPtr R, const bool enforceSingleThread COMPV_DEFAULT(false))
{
	COMPV_CHECK_CODE_RETURN(CompVMathOpSub::sub(A, subVal, R, enforceSingleThread));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMath::subMul(const CompVMatPtr& A, const double& subVal, const double& mulVal, CompVMatPtrPtr R, const bool enforceSingleThread COMPV_DEFAULT(false))
{
	COMPV_CHECK_CODE_RETURN(CompVMathOpSub::subMul(A, subVal, mulVal, R, enforceSingleThread));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMath::mulAB(const CompVMatPtr &A, const CompVMatPtr &B, CompVMatPtrPtr R)
{
	COMPV_CHECK_CODE_RETURN(CompVMathOpMul::mulAB(A, B, R));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMath::mulABt(const CompVMatPtr &A, const CompVMatPtr &B, CompVMatPtrPtr R)
{
	COMPV_CHECK_CODE_RETURN(CompVMathOpMul::mulABt(A, B, R));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMath::mulAtA(const CompVMatPtr &A, CompVMatPtrPtr R)
{
	COMPV_CHECK_CODE_RETURN(CompVMathOpMul::mulAtA(A, R));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMath::clip3(const CompVMatPtr& in, const double minn, const double maxx, CompVMatPtrPtr out)
{
	COMPV_CHECK_CODE_RETURN(CompVMathClip::clip3(in, minn, maxx, out));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMath::clip2(const CompVMatPtr& in, const double maxx, CompVMatPtrPtr out)
{
	COMPV_CHECK_CODE_RETURN(CompVMathClip::clip2(in, maxx, out));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMath::dot(const CompVMatPtr &A, const CompVMatPtr &B, double* ret)
{
	COMPV_CHECK_CODE_RETURN(CompVMathDot::dot(A, B, ret));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMath::dotSub(const CompVMatPtr &A, const CompVMatPtr &B, double* ret)
{
	COMPV_CHECK_CODE_RETURN(CompVMathDot::dotSub(A, B, ret));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMath::scale(const CompVMatPtr &in, const double& s, CompVMatPtrPtr out, const bool enforceSingleThread COMPV_DEFAULT(false))
{
	COMPV_CHECK_CODE_RETURN(CompVMathScale::scale(in, s, out, enforceSingleThread));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMath::exp(const CompVMatPtr &in, CompVMatPtrPtr out)
{
	COMPV_CHECK_CODE_RETURN(CompVMathExp::exp(in, out));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMath::minMax(const CompVMatPtr &A, double& minn, double& maxx)
{
	COMPV_CHECK_CODE_RETURN(CompVMathOpMinMax::minMax(A, minn, maxx));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMath::minn(const CompVMatPtr &A, double& minn)
{
	COMPV_CHECK_CODE_RETURN(CompVMathOpMinMax::minn(A, minn));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMath::tanh(const float* lut_ptr, const size_t& lut_length, const float& scale, const CompVMatPtr &inMat, CompVMatPtrPtr outMat)
{
	COMPV_CHECK_EXP_RETURN(
		!lut_ptr || !lut_length || !inMat || !outMat || !inMat->isRawTypeMatch<float>(),
		COMPV_ERROR_CODE_E_INVALID_PARAMETER
	);
	CompVMatPtr outMat_;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObj(&outMat_, inMat));
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		for (size_t row = ystart; row < yend; ++row) {
			COMPV_CHECK_CODE_RETURN(CompVMathActivationFunctions::tanh(
				lut_ptr, lut_length, scale,
				inMat->cols(), inMat->ptr<const float>(row), outMat_->ptr<float>(row)
			));
		}
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		inMat->cols(),
		inMat->rows(),
		COMPV_MATH_ACTIVATION_FUNCTIONS_MIN_SAMPLES_PER_THREAD
	));

	*outMat = outMat_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMath::tanhMul(const float* lut_ptr, const size_t& lut_length, const float& scale, const CompVMatPtr &inMat, const CompVMatPtr &mulMat, CompVMatPtrPtr outMat)
{
	COMPV_CHECK_EXP_RETURN(
		!lut_ptr || !lut_length || !inMat || !mulMat || !outMat || !inMat->isRawTypeMatch<float>() ||
		mulMat->subType() != inMat->subType() || mulMat->cols() != inMat->cols() || mulMat->rows() != inMat->rows(),
		COMPV_ERROR_CODE_E_INVALID_PARAMETER
	);
	CompVMatPtr outMat_;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObj(&outMat_, inMat));
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		for (size_t row = ystart; row < yend; ++row) {
			COMPV_CHECK_CODE_RETURN(CompVMathActivationFunctions::tanhMul(
				lut_ptr, lut_length, scale,
				inMat->cols(), inMat->ptr<const float>(row), mulMat->ptr<const float>(row), outMat_->ptr<float>(row)
			));
		}
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		inMat->cols(),
		inMat->rows(),
		COMPV_MATH_ACTIVATION_FUNCTIONS_MIN_SAMPLES_PER_THREAD
	));

	*outMat = outMat_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMath::logistic(const float* lut_ptr, const size_t& lut_length, const float& scale, const CompVMatPtr &inMat, CompVMatPtrPtr outMat)
{
	COMPV_CHECK_EXP_RETURN(
		!lut_ptr || !lut_length || !inMat || !outMat || !inMat->isRawTypeMatch<float>(),
		COMPV_ERROR_CODE_E_INVALID_PARAMETER
	);
	CompVMatPtr outMat_;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObj(&outMat_, inMat));
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		for (size_t row = ystart; row < yend; ++row) {
			COMPV_CHECK_CODE_RETURN(CompVMathActivationFunctions::logistic(
				lut_ptr, lut_length, scale,
				inMat->cols(), inMat->ptr<const float>(row), outMat_->ptr<float>(row)
			));
		}
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		inMat->cols(),
		inMat->rows(),
		COMPV_MATH_ACTIVATION_FUNCTIONS_MIN_SAMPLES_PER_THREAD
	));

	*outMat = outMat_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMath::logisticMul(const float* lut_ptr, const size_t& lut_length, const float& scale, const CompVMatPtr &inMat, const CompVMatPtr &mulMat, CompVMatPtrPtr outMat)
{
	COMPV_CHECK_EXP_RETURN(
		!lut_ptr || !lut_length || !inMat || !mulMat || !outMat || !inMat->isRawTypeMatch<float>() ||
		mulMat->subType() != inMat->subType() || mulMat->cols() != inMat->cols() || mulMat->rows() != inMat->rows(),
		COMPV_ERROR_CODE_E_INVALID_PARAMETER
	);
	CompVMatPtr outMat_;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObj(&outMat_, inMat));
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		for (size_t row = ystart; row < yend; ++row) {
			COMPV_CHECK_CODE_RETURN(CompVMathActivationFunctions::logisticMul(
				lut_ptr, lut_length, scale,
				inMat->cols(), inMat->ptr<const float>(row), mulMat->ptr<const float>(row), outMat_->ptr<float>(row)
			));
		}
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		inMat->cols(),
		inMat->rows(),
		COMPV_MATH_ACTIVATION_FUNCTIONS_MIN_SAMPLES_PER_THREAD
	));

	*outMat = outMat_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMath::softmaxInPlace(CompVMatPtr &inOutMat)
{
	COMPV_CHECK_EXP_RETURN(
		!inOutMat || !inOutMat->isRawTypeMatch<float>(),
		COMPV_ERROR_CODE_E_INVALID_PARAMETER
	);
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		for (size_t row = ystart; row < yend; ++row) {
			COMPV_CHECK_CODE_RETURN(CompVMathActivationFunctions::softmaxInPlace(
				inOutMat->cols(), inOutMat->ptr<float>(row)
			));
		}
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		inOutMat->cols(),
		inOutMat->rows(),
		COMPV_MATH_ACTIVATION_FUNCTIONS_MIN_SAMPLES_PER_THREAD
	));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

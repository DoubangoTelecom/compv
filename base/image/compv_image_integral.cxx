/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/image/compv_image_integral.h"


COMPV_NAMESPACE_BEGIN()

template<typename InputType, typename OutputType>
static void CompVImageIntegralProcess_C(const InputType* in, OutputType* sum, OutputType* sumsq, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t in_stride, const compv_uscalar_t sum_stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPGPU implementation could be found");
	for (compv_uscalar_t j = 0; j < height; j++) {
		OutputType row_sum = 0, row_sumsq = 0;
		sum[-1] = 0, sumsq[-1] = 0;
		const OutputType* sump = sum - sum_stride;
		const OutputType* sumsqp = sumsq - sum_stride;
		for (compv_uscalar_t i = 0; i < width; i++) {
			row_sum += in[i];
			row_sumsq += in[i] * in[i];
			sum[i] = row_sum + sump[i];
			sumsq[i] = row_sumsq + sumsqp[i];
		}
		in += in_stride;
		sum += sum_stride;
		sumsq += sum_stride;
	}
}
static void CompVImageIntegralProcess_8u64f_C(const uint8_t* in, compv_float64_t* sum, compv_float64_t* sumsq, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t in_stride, const compv_uscalar_t sum_stride) {
	CompVImageIntegralProcess_C<uint8_t, compv_float64_t>(in, sum, sumsq, width, height, in_stride, sum_stride);
}

// For now only grayscale images (Single plane, 8u) are accepted as input
// Output is always double
COMPV_ERROR_CODE CompVImageIntegral::process(const CompVMatPtr& imageIn, CompVMatPtrPtr imageSum, CompVMatPtrPtr imageSumsq)
{
	COMPV_CHECK_EXP_RETURN(!imageIn || !imageSum || !imageSumsq || imageIn->planeCount() != 1 || imageIn->elmtInBytes() != sizeof(uint8_t), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	const compv_uscalar_t width = static_cast<compv_uscalar_t>(imageIn->cols());
	const compv_uscalar_t height = static_cast<compv_uscalar_t>(imageIn->rows());
	const compv_uscalar_t in_stride = static_cast<compv_uscalar_t>(imageIn->stride());

	CompVMatPtr sum_ = (*imageSum == imageIn) ? nullptr : *imageSum, sumsq_ = (*imageSumsq == imageIn) ? nullptr : *imageSumsq;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&sum_, height + 1, width + 1));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&sumsq_, sum_->rows(), sum_->cols()));
	COMPV_CHECK_CODE_RETURN(sum_->zero_row(0));
	COMPV_CHECK_CODE_RETURN(sumsq_->zero_row(0));

	CompVImageIntegralProcess_C(
		imageIn->ptr<const uint8_t>(),
		sum_->ptr<compv_float64_t>(1, 1), sumsq_->ptr<compv_float64_t>(1, 1), 
		width, height, in_stride, sum_->stride()
	);

	*imageSum = sum_;
	*imageSumsq = sumsq_;

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()



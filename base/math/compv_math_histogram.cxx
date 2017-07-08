/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_histogram.h"

COMPV_NAMESPACE_BEGIN()

COMPV_ERROR_CODE CompVMathHistogram::process(const CompVMatPtr& data, CompVMatPtrPtr histogram)
{
	COMPV_CHECK_EXP_RETURN(!data || data->isEmpty() || !histogram, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Input data is null or empty");

	if (data->elmtInBytes() == sizeof(uint8_t)) {
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<int32_t>(histogram, 1, 256));
		COMPV_CHECK_CODE_RETURN((*histogram)->zero_rows());
		int32_t* histogramPtr = (*histogram)->ptr<int32_t>();
		const int planes = static_cast<int>(data->planeCount());
		for (int p = 0; p < planes; ++p) {
			COMPV_CHECK_CODE_RETURN(CompVMathHistogram::process_8u(data->ptr<uint8_t>(0, 0, p), data->cols(p), data->rows(p), data->strideInBytes(p), histogramPtr));
		}
		return COMPV_ERROR_CODE_S_OK;
	}

	COMPV_DEBUG_ERROR("Input format not supported");
	return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
}

// Up to the caller to set 'histogramPtr' values to zeros
COMPV_ERROR_CODE CompVMathHistogram::process_8u(const uint8_t* dataPtr, size_t width, size_t height, size_t stride, int32_t* histogramPtr)
{
	// Private function, no need to check imput parameters

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation found");

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");

	size_t i, j;

	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; ++i) {
			++histogramPtr[dataPtr[i]];
		}
		dataPtr += stride;
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

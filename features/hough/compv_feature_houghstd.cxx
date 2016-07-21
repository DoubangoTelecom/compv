/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/features/hough/compv_feature_houghstd.h"

COMPV_NAMESPACE_BEGIN()

CompVHoughStd::CompVHoughStd(int32_t rho /*= 1*/, float theta /*= kfMathTrigPiOver180*/, int threshold /*= 1*/)
:CompVHough(COMPV_HOUGH_STANDARD_ID)
, m_nRho(rho)
, m_fTheta(theta)
, m_nThreshold(threshold)
{

}

CompVHoughStd:: ~CompVHoughStd()
{

}

// override CompVSettable::set
COMPV_ERROR_CODE CompVHoughStd::set(int id, const void* valuePtr, size_t valueSize)
{
	COMPV_CHECK_EXP_RETURN(valuePtr == NULL || valueSize == 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (id) {
	case -1:
	default:
		return CompVSettable::set(id, valuePtr, valueSize);
	}
}

// override CompVHough::process
COMPV_ERROR_CODE CompVHoughStd::process(const CompVPtrArray(uint8_t)& edges, CompVPtrArray(CompVCoordPolar2f)& coords)
{
	COMPV_CHECK_EXP_RETURN(!edges || edges->isEmpty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	CompVPtrArray(CompVHoughEntry) accumulator;

	const size_t cols = edges->cols();
	const size_t rows = edges->rows();

	const size_t maxRhoCount = COMPV_MATH_ROUNDFU_2_INT(((cols + rows) * 2 + 1) / m_nRho, size_t);
	const size_t maxThetaCount = COMPV_MATH_ROUNDFU_2_INT(kfMathTrigPi / m_fTheta, size_t);

	COMPV_CHECK_CODE_RETURN(CompVArray<CompVHoughEntry>::newObjStrideless(&accumulator, maxRhoCount, maxThetaCount));
	COMPV_CHECK_CODE_RETURN(accumulator->zero_all());

	const size_t stride = edges->strideInBytes();
	const uint8_t* pixels = edges->ptr(0);
	CompVHoughEntry* entry;

	for (size_t row = 1; row < rows; ++row) {
		for (size_t col = 1; col < cols; ++col) {
			if (pixels[col]) {
				for (size_t t = 0; t < maxThetaCount; ++t) {
					float theta = t * m_fTheta;
					float r = col * cos(theta) + row * sin(theta); // FIXME: use m_nRho here and must be float (combined with sin and cos tables)
					size_t rabs = COMPV_MATH_ROUNDFU_2_INT(fabs(r), size_t);
					if (rabs < maxRhoCount) {
						entry = const_cast<CompVHoughEntry*>(accumulator->ptr(rabs, t));
						entry->rho = r;
						entry->theta = theta;
						entry->count += 1;
					}
					else {
						COMPV_DEBUG_ERROR("Not expected");
					}
				}
			}
		}
		pixels += stride;
	}

	uint8_t* nms = (uint8_t*)CompVMem::calloc(accumulator->rows() * accumulator->cols(), sizeof(uint8_t));

	// NMS
	for (size_t row = 1; row < accumulator->rows(); ++row) {
		for (size_t col = 1; col < accumulator->cols(); ++col) {
			if (accumulator->ptr(row, col)->count > m_nThreshold) {
				//COMPV_DEBUG_INFO("Line at (%d, %f)", m_nRho * row, m_fTheta * (float)col);
				const CompVHoughEntry* e = accumulator->ptr(row, col);
				const CompVHoughEntry* top = accumulator->ptr(row - 1, col);
				const CompVHoughEntry* bottom = accumulator->ptr(row + 1, col);
				if (e[-1].count >= e->count || e[+1].count >= e->count || top[-1].count >= e->count || top[0].count >= e->count || top[+1].count >= e->count || bottom[-1].count >= e->count || bottom[0].count >= e->count || bottom[+1].count >= e->count) {
					nms[row * accumulator->cols() + col] = 1;
				}
			}
		}
	}
	for (size_t row = 1; row < accumulator->rows(); ++row) {
		for (size_t col = 1; col < accumulator->cols(); ++col) {
			if (nms[row * accumulator->cols() + col]) {
				const_cast<CompVHoughEntry*>(accumulator->ptr(row, col))->count = 0;
				//COMPV_DEBUG_INFO("Suppressed");
			}
		}
	}

	size_t count = 0;
	for (size_t row = 1; row < accumulator->rows(); ++row) {
		for (size_t col = 1; col < accumulator->cols(); ++col) {
			if (accumulator->ptr(row, col)->count > m_nThreshold) {
				//COMPV_DEBUG_INFO("Line at (%d, %f)", m_nRho * row, m_fTheta * (float)col);
				++count;
			}
		}
	}

	if (count) {
		COMPV_CHECK_CODE_RETURN(CompVArray<CompVCoordPolar2f>::newObjStrideless(&coords, 1, count));
		size_t i = 0;
		CompVCoordPolar2f* coord;
		for (size_t row = 1; row < accumulator->rows(); ++row) {
			for (size_t col = 1; col < accumulator->cols(); ++col) {
				if (accumulator->ptr(row, col)->count > m_nThreshold) {
					coord = const_cast<CompVCoordPolar2f*>(coords->ptr(0, i++));
					coord->rho = m_nRho * (float)row;
					coord->theta = m_fTheta * (float)col;
				}
			}
		}
	}
	else {
		coords = NULL;
	}

	CompVMem::free((void**)&nms);

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVHoughStd::newObj(CompVPtr<CompVHough* >* hough, int32_t rho /*= 1*/, float theta /*= kfMathTrigPiOver180*/, int threshold /*= 1*/)
{
	COMPV_CHECK_EXP_RETURN(!hough, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVPtr<CompVHoughStd* >hough_ = new CompVHoughStd(rho, theta, threshold);
	COMPV_CHECK_EXP_RETURN(!hough_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	*hough = *hough_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

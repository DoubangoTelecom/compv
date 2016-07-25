/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/features/hough/compv_feature_houghstd.h"

static bool COMPV_INLINE __compareAccCountDec(const compv::CompVCoordPolar2f* i, const  compv::CompVCoordPolar2f* j) {
	return (i->count > j->count);
}

COMPV_NAMESPACE_BEGIN()

CompVHoughStd::CompVHoughStd(float rho /*= 1.f*/, float theta /*= kfMathTrigPiOver180*/, int threshold /*= 1*/)
:CompVHough(COMPV_HOUGH_STANDARD_ID)
, m_fRho(rho)
, m_fTheta(theta)
, m_nThreshold(threshold)
, m_nWidth(0)
, m_nHeight(0)
, m_nMaxLines(INT_MAX)
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
	case COMPV_HOUGH_SET_FLT32_RHO:{
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(float) || *((float*)valuePtr) <= 0.f, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		float fRho = *((float*)valuePtr);
		COMPV_CHECK_CODE_RETURN(initCoords(fRho, m_fTheta, m_nThreshold, m_nWidth, m_nHeight));
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_HOUGH_SET_FLT32_THETA:{
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(float) || *((float*)valuePtr) <= 0.f, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		float fTheta = *((float*)valuePtr);
		COMPV_CHECK_CODE_RETURN(initCoords(m_fRho, fTheta, m_nThreshold, m_nWidth, m_nHeight));
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_HOUGH_SET_INT32_THRESHOLD:{
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int32_t) || *((int32_t*)valuePtr) <= 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		int32_t nThreshold = *((int32_t*)valuePtr);
		COMPV_CHECK_CODE_RETURN(initCoords(m_fRho, m_fTheta, nThreshold, m_nWidth, m_nHeight));
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_HOUGH_SET_INT32_MAXLINES:{
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int32_t) || *((int32_t*)valuePtr) <= 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_nMaxLines = *((int32_t*)valuePtr);
		return COMPV_ERROR_CODE_S_OK;
	}
	default:
		return CompVSettable::set(id, valuePtr, valueSize);
	}
}

// override CompVHough::process
COMPV_ERROR_CODE CompVHoughStd::process(const CompVPtrArray(uint8_t)& edges, CompVPtrArray(compv_float32x2_t)& coords)
{
	COMPV_CHECK_EXP_RETURN(!edges || edges->isEmpty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	COMPV_CHECK_CODE_RETURN(initCoords(m_fRho, m_fTheta, m_nThreshold, edges->cols(), edges->rows()));

	size_t cols = edges->cols();
	size_t rows = edges->rows();

	const size_t edgeStride = edges->strideInBytes();
	const uint8_t* pixels = edges->ptr(0);
	
	const compv_float32_t* pSinRho = m_SinRho->ptr();
	const compv_float32_t* pCosRho = m_CosRho->ptr();
	const size_t maxThetaCount = m_Accumulator->cols();
	compv_float32_t rho, fBarrier = static_cast<compv_float32_t>(m_nBarrier);
	size_t nmsStride, accStride;
	uint8_t* pNMS;
	int32_t *pACC, rhoInt32;
	compv_float32_t colf, rowf;

	COMPV_CHECK_CODE_RETURN(m_NMS->strideInElts(nmsStride));
	COMPV_CHECK_CODE_RETURN(m_Accumulator->strideInElts(accStride));

	// Accumulator gathering
	pACC = const_cast<int32_t*>(m_Accumulator->ptr(m_nBarrier));
	for (size_t row = 0; row < rows; ++row) {
		for (size_t col = 0; col < cols; ++col) {
			if (pixels[col]) {
				colf = static_cast<compv_float32_t>(col);
				rowf = static_cast<compv_float32_t>(row);
				for (size_t t = 0; t < maxThetaCount; ++t) {
					rho = colf * pCosRho[t] + rowf * pSinRho[t];
					rhoInt32 = COMPV_MATH_ROUNDF_2_INT(rho, int32_t);
					pACC[t - (rhoInt32 * accStride)]++;
				}
			}
		}
		pixels += edgeStride;
	}

	// Create box
	if (!m_Coords) {
		COMPV_CHECK_CODE_RETURN(CompVPtrBoxNew(CompVCoordPolar2f)(&m_Coords));
	}
	
	// Non-maxima suppression (NMS)
	COMPV_CHECK_CODE_RETURN(nms());

	// NMS-apply
	CompVCoordPolar2f* coord;
	pNMS = const_cast<uint8_t*>(m_NMS->ptr());
	pACC = const_cast<int32_t*>(m_Accumulator->ptr());
	m_Coords->reset();
	for (size_t row = 0; row < m_Accumulator->rows(); ++row) {
		for (size_t col = 0; col < m_Accumulator->cols(); ++col) {
			if (pNMS[col]) {
				pNMS[col] = 0; // reset for  next time
			}
			else if (pACC[col] > m_nThreshold) {
				COMPV_CHECK_CODE_RETURN(m_Coords->new_item(&coord));				
				coord->count = pACC[col];
				coord->rho = fBarrier - static_cast<compv_float32_t>(row);
				coord->theta = col * m_fTheta;
			}
			pACC[col] = 0;
		}
		pNMS += nmsStride;
		pACC += accStride;
	}	

	if (!m_Coords->empty()) {
		COMPV_CHECK_CODE_RETURN(m_Coords->sort(__compareAccCountDec));
		size_t maxLines = COMPV_MATH_MIN(m_Coords->size(), (size_t)m_nMaxLines);
		m_Coords->resize(maxLines);
		COMPV_CHECK_CODE_RETURN(CompVArray<compv_float32x2_t>::newObjStrideless(&coords, 1, maxLines));
		compv_float32x2_t* f32x2 = const_cast<compv_float32x2_t*>(coords->ptr());
		coord = m_Coords->ptr();
		for (size_t i = 0; i < maxLines; ++i){
			f32x2[i][0] = coord[i].rho;
			f32x2[i][1] = coord[i].theta;
		}		
	}
	else {
		coords = NULL;
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVHoughStd::newObj(CompVPtr<CompVHough* >* hough, float rho /*= 1.f*/, float theta /*= kfMathTrigPiOver180*/, int threshold /*= 1*/)
{
	COMPV_CHECK_EXP_RETURN(!hough, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVPtr<CompVHoughStd* >hough_ = new CompVHoughStd(rho, theta, threshold);
	COMPV_CHECK_EXP_RETURN(!hough_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	*hough = *hough_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVHoughStd::initCoords(float fRho, float fTheta, int32_t nThreshold, size_t nWidth/*=0*/, size_t nHeight/*=0*/)
{
	nWidth = !nWidth ? m_nWidth : nWidth;
	nHeight = !nHeight ? m_nHeight : nHeight;
	if (m_fRho != fRho || m_fTheta != fTheta || m_nWidth != nWidth || m_nHeight != nHeight) {
		// rho = x*cos(t) + y*sin(t), "cos" and "sin" in [-1, 1] which means we have (x+y)*2 values
		const size_t maxRhoCount = COMPV_MATH_ROUNDFU_2_INT((((nWidth + nHeight) << 1) + 1) / fRho, size_t);
		const size_t maxThetaCount = COMPV_MATH_ROUNDFU_2_INT(kfMathTrigPi / fTheta, size_t);
		COMPV_CHECK_CODE_RETURN(CompVArray<int32_t>::newObjAligned(&m_Accumulator, maxRhoCount, maxThetaCount));
		COMPV_CHECK_CODE_RETURN(m_Accumulator->zero_rows());
		COMPV_CHECK_CODE_RETURN(CompVArray<uint8_t>::newObjAligned(&m_NMS, maxRhoCount, maxThetaCount));
		COMPV_CHECK_CODE_RETURN(m_NMS->zero_rows());
		COMPV_CHECK_CODE_RETURN(CompVArray<compv_float32_t>::newObjAligned(&m_SinRho, 1, maxThetaCount));
		COMPV_CHECK_CODE_RETURN(CompVArray<compv_float32_t>::newObjAligned(&m_CosRho, 1, maxThetaCount));
		
		compv_float32_t* pSinRho = const_cast<compv_float32_t*>(m_SinRho->ptr());
		compv_float32_t* pCosRho = const_cast<compv_float32_t*>(m_CosRho->ptr());
		float tt;
		size_t t;
		for (t = 0, tt = 0.f; t < maxThetaCount; ++t, tt += fTheta) {
			pSinRho[t] = COMPV_MATH_SIN(tt) * m_fRho;
			pCosRho[t] = COMPV_MATH_COS(tt) * m_fRho;
		}
		
		m_fRho = fRho;
		m_fTheta = fTheta;
		m_nWidth = nWidth;
		m_nHeight = nHeight;
		m_nBarrier = m_nWidth + m_nHeight;
	}
	m_nThreshold = nThreshold;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVHoughStd::nms()
{
	size_t rows = m_Accumulator->rows();
	size_t cols = m_Accumulator->cols();
	const int32_t *pAccumulator = m_Accumulator->ptr(1), *curr, *top, *bottom;
	uint8_t* nms = const_cast<uint8_t*>(m_NMS->ptr(1));
	size_t nmsStride, accStride;
	int32_t t;
	COMPV_CHECK_CODE_RETURN(m_NMS->strideInElts(nmsStride));
	COMPV_CHECK_CODE_RETURN(m_Accumulator->strideInElts(accStride));
	int stride = static_cast<int>(accStride);

	for (size_t row = 1; row < rows - 1; ++row) {
		for (size_t col = 1; col < cols - 1; ++col) {
			if (pAccumulator[col] > m_nThreshold) {
				t = pAccumulator[col];
				curr = &pAccumulator[col];
				top = &pAccumulator[col - stride];
				bottom = &pAccumulator[col + stride];
				if (curr[-1] > t || curr[+1] > t || top[-1] > t || top[0] > t || top[+1] > t || bottom[-1] > t || bottom[0] > t || bottom[+1] > t) {
					nms[col] = 1;
				}
			}
		}
		pAccumulator += accStride;
		nms += nmsStride;
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

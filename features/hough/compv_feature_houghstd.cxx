/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/features/hough/compv_feature_houghstd.h"

COMPV_NAMESPACE_BEGIN()

CompVHoughStd::CompVHoughStd(float rho /*= 1.f*/, float theta /*= kfMathTrigPiOver180*/, int threshold /*= 1*/)
:CompVHough(COMPV_HOUGH_STANDARD_ID)
, m_fRho(rho)
, m_fTheta(theta)
, m_nThreshold(threshold)
, m_nWidth(0)
, m_nHeight(0)
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

static void nonMaxSupp3(CompVPtrArray(CompVHoughEntry)& accumulator, uint8_t* nms, int32_t threshold)
{
	for (size_t row = 1; row < accumulator->rows() - 1; ++row) {
		for (size_t col = 1; col < accumulator->cols() - 1; ++col) {
			if (accumulator->ptr(row, col)->count > threshold) {
				const CompVHoughEntry* e = accumulator->ptr(row, col);
				const CompVHoughEntry* top = accumulator->ptr(row - 1, col);
				const CompVHoughEntry* bottom = accumulator->ptr(row + 1, col);
				if (e[-1].count > e->count || e[+1].count > e->count || top[-1].count > e->count || top[0].count > e->count || top[+1].count > e->count || bottom[-1].count > e->count || bottom[0].count > e->count || bottom[+1].count > e->count) {
					nms[row * accumulator->cols() + col] = 1;
				}
			}
		}
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
	compv_float32_t* pRho = const_cast<compv_float32_t*>(m_Rho->ptr());
	const size_t maxThetaCount = m_Accumulator->cols();

	// Accumulator gathering
	for (size_t row = 0; row < rows; ++row) {
		for (size_t col = 0; col < cols; ++col) {
			if (pixels[col]) {
				for (size_t t = 0; t < maxThetaCount; ++t) {
					float theta = t * m_fTheta;
					float r = col * pCosRho[t] + row * pSinRho[t] ;
					size_t rabs = COMPV_MATH_ROUNDFU_2_INT(fabs(r), size_t);
					if (r < 0) {
						rabs += m_nBarrier;
					}
					const_cast<int32_t*>(m_Accumulator->ptr(rabs, t))[0]++;
					const_cast<compv_float32_t*>(m_Rho->ptr(rabs, t))[0] = (float)COMPV_MATH_ROUNDFU_2_INT(r, int); // FIXME
				}
			}
		}
		pixels += edgeStride;
	}
	
	// Non-maxima suppression (NMS)
	COMPV_CHECK_CODE_RETURN(nms());

	// NMS-apply
	uint8_t* pNMS = const_cast<uint8_t*>(m_NMS->ptr());
	int32_t* pACC = const_cast<int32_t*>(m_Accumulator->ptr());
	size_t count = 0, nmsStride, accStride;
	COMPV_CHECK_CODE_RETURN(m_NMS->strideInElts(nmsStride));
	COMPV_CHECK_CODE_RETURN(m_Accumulator->strideInElts(accStride));
	for (size_t row = 0; row < m_Accumulator->rows(); ++row) {
		for (size_t col = 0; col < m_Accumulator->cols(); ++col) {
			if (pNMS[col]) {
				pACC[col] = 0;
				pNMS[col] = 0; // reset for  next time
			}
			else if (pACC[col] > m_nThreshold) {
				++count;
			}
		}
		pNMS += nmsStride;
		pACC += accStride;
	}	

	if (count) {
		CompVPtrArray(size_t) Idx;
		COMPV_CHECK_CODE_RETURN(CompVArray<size_t>::newObjAligned(&Idx, 1, count));
		size_t* indexes = const_cast<size_t*>(Idx->ptr());
		size_t index, oldIndex, rhoStride;
		COMPV_CHECK_CODE_RETURN(m_Rho->strideInElts(rhoStride));
		for (size_t i = 0; i < count; ++i) {
			indexes[i] = i;
		}

		CompVPtrArray(CompVCoordPolar2f) coords_;
		COMPV_CHECK_CODE_RETURN(CompVArray<CompVCoordPolar2f>::newObjStrideless(&coords_, 1, count));
		CompVCoordPolar2f* coord = const_cast<CompVCoordPolar2f*>(coords_->ptr());
		pACC = const_cast<int32_t*>(m_Accumulator->ptr());
		const compv_float32_t* pRho = m_Rho->ptr();
		for (size_t row = 0; row < m_Accumulator->rows(); ++row) {
			for (size_t col = 0; col < m_Accumulator->cols(); ++col) {
				if (pACC[col]) {
					if (pACC[col] > m_nThreshold) {
						coord->rho = pRho[col];
						coord->theta = col * m_fTheta;
						coord->count = pACC[col];
						++coord;
					}
					pACC[col] = 0; // reset for next time
				}
			}
			pACC += accStride;
			pRho += rhoStride;
		}
		// sorting
		bool sorted, wasSorted = true;
		coord = const_cast<CompVCoordPolar2f*>(coords_->ptr());
		do {
			sorted = true;
			for (size_t i = 0; i < count - 1; ++i) {
				index = indexes[i];
				if (coord[indexes[i]].count < coord[indexes[i + 1]].count) {
					oldIndex = indexes[i];
					indexes[i] = indexes[i + 1];
					indexes[i + 1] = oldIndex;
					sorted = false;
					wasSorted = false;
				}
			}
		} while (!sorted);
		// FIXME
		/*if (wasSorted) {
			coords = coords_;
		}
		else*/ {
			COMPV_CHECK_CODE_RETURN(CompVArray<compv_float32x2_t>::newObjStrideless(&coords, 1, count));
			compv_float32x2_t* f32x2 = const_cast<compv_float32x2_t*>(coords->ptr());
			for (size_t col = 0; col < count; ++col) {
				f32x2[col][0] = coords_->ptr(0, indexes[col])->rho;
				f32x2[col][1] = coords_->ptr(0, indexes[col])->theta;
			}
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
		COMPV_CHECK_CODE_RETURN(CompVArray<compv_float32_t>::newObjAligned(&m_Rho, maxRhoCount, maxThetaCount));
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

	for (size_t row = 1; row < rows - 1; ++row) {
		for (size_t col = 1; col < cols - 1; ++col) {
			if (pAccumulator[col] > m_nThreshold) {
				t = pAccumulator[col];
				curr = &pAccumulator[col];
				top = m_Accumulator->ptr(row - 1, col); // FIXME
				bottom = m_Accumulator->ptr(row + 1, col); // FIXME
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

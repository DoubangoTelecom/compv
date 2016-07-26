/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/features/hough/compv_feature_houghstd.h"
#include "compv/compv_engine.h"

#include "compv/intrinsics/x86/features/hough/compv_feature_houghstd_intrin_sse2.h"

static bool COMPV_INLINE __compareAccCountDec(const compv::CompVCoordPolar2f* i, const  compv::CompVCoordPolar2f* j) {
	return (i->count > j->count);
}

#define COMPV_FEATURE_HOUGHSTD_NMS_MIN_SAMPLES_PER_THREAD	(20*20)

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

	// Init coords (sine and cosine tables)
	COMPV_CHECK_CODE_RETURN(initCoords(m_fRho, m_fTheta, m_nThreshold, edges->cols(), edges->rows()));

	// Accumulator gathering
	COMPV_CHECK_CODE_RETURN(acc_gather(edges));
	
	// Thread dispatcher
	size_t threadsCount = 1;
	CompVPtr<CompVThreadDispatcher11* >threadDisp = CompVEngine::getThreadDispatcher11();
	if (threadDisp && !threadDisp->isMotherOfTheCurrentThread()) {
		threadsCount = m_Accumulator->rows() / COMPV_FEATURE_HOUGHSTD_NMS_MIN_SAMPLES_PER_THREAD;
		threadsCount = COMPV_MATH_MIN_3(threadsCount, m_Accumulator->rows(), (size_t)threadDisp->getThreadsCount());
	}

	// Non-maxima suppression (NMS)
	if (threadsCount > 1) {
		CompVAsyncTaskIds taskIds;
		taskIds.reserve(threadsCount);
		const size_t countAny = (size_t)(m_Accumulator->rows() / threadsCount);
		const size_t countLast = (size_t)countAny + (m_Accumulator->rows() % threadsCount);
		auto funcPtrNmsGather = [&](size_t rowStart, size_t rowCount) -> COMPV_ERROR_CODE {
			COMPV_CHECK_CODE_RETURN(nms_gather(rowStart, rowCount));
			return COMPV_ERROR_CODE_S_OK;
		};
		size_t rowStart, threadIdx;
		// NMS gathering
		for (threadIdx = 0, rowStart = 0; threadIdx < threadsCount - 1; ++threadIdx, rowStart += countAny) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrNmsGather, rowStart, countAny), taskIds));
		}
		COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrNmsGather, rowStart, countLast), taskIds));
		COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds));
	}
	else {
		COMPV_CHECK_CODE_RETURN(nms_gather(0, m_Accumulator->rows()));
	}

	// NMS-apply
	COMPV_CHECK_CODE_RETURN(nms_apply());

	// Set result
	if (!m_Coords->empty()) {
		CompVCoordPolar2f* coord;
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
		COMPV_CHECK_CODE_RETURN(CompVArray<int32_t>::newObjAligned(&m_SinRho, 1, maxThetaCount));
		COMPV_CHECK_CODE_RETURN(CompVArray<int32_t>::newObjAligned(&m_CosRho, 1, maxThetaCount));
		
		int32_t* pSinRho = const_cast<int32_t*>(m_SinRho->ptr());
		int32_t* pCosRho = const_cast<int32_t*>(m_CosRho->ptr());
		float tt;
		size_t t;
		for (t = 0, tt = 0.f; t < maxThetaCount; ++t, tt += fTheta) {
			pSinRho[t] = static_cast<int32_t>((COMPV_MATH_SIN(tt) * m_fRho) * 65535.f);
			pCosRho[t] = static_cast<int32_t>((COMPV_MATH_COS(tt) * m_fRho) * 65535.f);
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

COMPV_ERROR_CODE CompVHoughStd::acc_gather(const CompVPtrArray(uint8_t)& edges)
{
	size_t nmsStride, accStride;
	COMPV_CHECK_CODE_RETURN(m_NMS->strideInElts(nmsStride));
	COMPV_CHECK_CODE_RETURN(m_Accumulator->strideInElts(accStride));
	const int32_t accStrideInt32 = static_cast<int32_t>(accStride);
	int32_t rowsInt32 = static_cast<int32_t>(edges->rows());
	int32_t colsInt32 = static_cast<int32_t>(edges->cols());
	const size_t edgeStride = edges->strideInBytes();
	const uint8_t* pixels = edges->ptr(0);
	const int32_t maxThetaCount = static_cast<int32_t>(m_Accumulator->cols());
	const int32_t* pSinRho = m_SinRho->ptr();
	const int32_t* pCosRho = m_CosRho->ptr();
	int32_t *pACC = const_cast<int32_t*>(m_Accumulator->ptr(m_nBarrier));

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
	for (int32_t row = 0; row < rowsInt32; ++row) {
		HoughStdAccGatherRow_C(pACC, accStrideInt32, pixels, 0, colsInt32, maxThetaCount, row, pCosRho, pSinRho);
		pixels += edgeStride;
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVHoughStd::nms_gather(size_t rowStart, size_t rowCount)
{
	size_t rows = m_Accumulator->rows();
	size_t maxCols = m_Accumulator->cols() - 1;	
	size_t rowEnd = COMPV_MATH_MIN((rowStart + rowCount), (rows - 1));
	rowStart = COMPV_MATH_MAX(1, rowStart);

	const int32_t *pACC = m_Accumulator->ptr(rowStart);
	uint8_t* pNMS = const_cast<uint8_t*>(m_NMS->ptr(rowStart));
	size_t nmsStride, accStride, row;
	COMPV_CHECK_CODE_RETURN(m_NMS->strideInElts(nmsStride));
	COMPV_CHECK_CODE_RETURN(m_Accumulator->strideInElts(accStride));

#if COMPV_ARCH_X86
	void (*HoughStdNmsGatherRow)(const int32_t * pAcc, compv_uscalar_t nAccStride, uint8_t* pNms, int32_t nThreshold, compv_uscalar_t width) = NULL;
	if (maxCols >= 4 && CompVCpu::isEnabled(compv::kCpuFlagSSE2)) {
		COMPV_EXEC_IFDEF_INTRIN_X86(HoughStdNmsGatherRow = HoughStdNmsGatherRow_Intrin_SSE2);
	}
	if (HoughStdNmsGatherRow) {
		for (row = rowStart; row < rowEnd; ++row) {
			HoughStdNmsGatherRow(pACC, (compv_uscalar_t)accStride, pNMS, m_nThreshold, (compv_uscalar_t)maxCols);
			pACC += accStride, pNMS += nmsStride;
		}
		return COMPV_ERROR_CODE_S_OK;
	}
#endif /* COMPV_ARCH_X86 */

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
	for (row = rowStart; row < rowEnd; ++row) {
		HoughStdNmsGatherRow_C(pACC, accStride, pNMS, m_nThreshold, 1, maxCols);
		pACC += accStride, pNMS += nmsStride;
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVHoughStd::nms_apply()
{
	int32_t rows = static_cast<int32_t>(m_Accumulator->rows());
	size_t cols = m_Accumulator->cols();
	uint8_t* pNMS = const_cast<uint8_t*>(m_NMS->ptr());
	int32_t* pACC = const_cast<int32_t*>(m_Accumulator->ptr());
	size_t nmsStride, accStride;
	int32_t nBarrier = static_cast<int32_t>(m_nBarrier);
	COMPV_CHECK_CODE_RETURN(m_NMS->strideInElts(nmsStride));
	COMPV_CHECK_CODE_RETURN(m_Accumulator->strideInElts(accStride));
	if (!m_Coords) {
		COMPV_CHECK_CODE_RETURN(CompVPtrBoxNew(CompVCoordPolar2f)(&m_Coords));
	}
	else {
		m_Coords->reset();
	}

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
	for (int32_t row = 0; row < rows; ++row) {
		HoughStdNmsApplyRow_C(pACC, pNMS, m_nThreshold, m_fTheta, nBarrier, row, 0, cols, m_Coords);
		pACC += accStride, pNMS += nmsStride;
	}
	return COMPV_ERROR_CODE_S_OK;
}

void HoughStdNmsGatherRow_C(const int32_t * pAcc, size_t nAccStride, uint8_t* pNms, int32_t nThreshold, size_t colStart, size_t maxCols)
{
	int32_t t;
	const int32_t *curr, *top, *bottom;
	int stride = static_cast<int>(nAccStride);
	for (size_t col = colStart; col < maxCols; ++col) {
		if (pAcc[col] > nThreshold) {
			t = pAcc[col];
			curr = &pAcc[col];
			top = &pAcc[col - stride];
			bottom = &pAcc[col + stride];
			if (curr[-1] > t || curr[+1] > t || top[-1] > t || top[0] > t || top[+1] > t || bottom[-1] > t || bottom[0] > t || bottom[+1] > t) {
				pNms[col] = 0xf;
			}
		}
	}
}

void HoughStdNmsApplyRow_C(int32_t* pACC, uint8_t* pNMS, int32_t threshold, compv_float32_t theta, int32_t barrier, int32_t row, size_t colStart, size_t maxCols, CompVPtrBox(CompVCoordPolar2f)& coords)
{
	// NMS-apply
	CompVCoordPolar2f* coord;	
	for (size_t col = colStart; col < maxCols; ++col) {
		if (pNMS[col]) {
			pNMS[col] = 0; // reset for  next time
		}
		else if (pACC[col] > threshold) {
			coords->new_item(&coord);
			coord->count = pACC[col];
			coord->rho = static_cast<compv_float32_t>(barrier - row);
			coord->theta = col * theta;
		}
		pACC[col] = 0;
	}	
}

void HoughStdAccGatherRow_C(int32_t* pACC, int32_t accStride, const uint8_t* pixels, int32_t colStart, int32_t maxCols, int32_t maxThetaCount, int32_t row, const int32_t* pCosRho, const int32_t* pSinRho)
{
	int32_t theta, rhoInt32;
	for (int32_t col = colStart; col < maxCols; ++col) {
		if (pixels[col]) {
			for (theta = 0; theta < maxThetaCount; ++theta) {
				rhoInt32 = (col * pCosRho[theta] + row * pSinRho[theta]) >> 16;
				pACC[theta - (rhoInt32 * accStride)]++; //!\\ Not thread-safe
			}
		}
	}
}

COMPV_NAMESPACE_END()

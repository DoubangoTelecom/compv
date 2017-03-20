/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/hough/compv_core_feature_houghstd.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/parallel/compv_parallel.h"

#define COMPV_THIS_CLASSNAME	"CompVHoughStd"

#define COMPV_FEATURE_HOUGHSTD_NMS_MIN_SAMPLES_PER_THREAD	(40*40)
#define COMPV_FEATURE_HOUGHSTD_ACC_MIN_SAMPLES_PER_THREAD	(20*20)

COMPV_NAMESPACE_BEGIN()

// threshold used for NMS
CompVHoughStd::CompVHoughStd(float rho COMPV_DEFAULT(1.f), float theta COMPV_DEFAULT(kfMathTrigPiOver180), size_t threshold COMPV_DEFAULT(1))
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
COMPV_ERROR_CODE CompVHoughStd::set(int id, const void* valuePtr, size_t valueSize) /*Overrides(CompVCaps)*/
{
	COMPV_CHECK_EXP_RETURN(!valuePtr || !valueSize, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (id) {
	case COMPV_HOUGH_SET_FLT32_RHO: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(compv_float32_t) || *reinterpret_cast<const compv_float32_t*>(valuePtr) <= 0.f || *reinterpret_cast<const compv_float32_t*>(valuePtr) > 1.f, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		const compv_float32_t fRho = *reinterpret_cast<const compv_float32_t*>(valuePtr);
		COMPV_CHECK_CODE_RETURN(initCoords(fRho, m_fTheta, m_nThreshold, m_nWidth, m_nHeight));
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_HOUGH_SET_FLT32_THETA: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(compv_float32_t) || *reinterpret_cast<const compv_float32_t*>(valuePtr) <= 0.f, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		const compv_float32_t fTheta = *reinterpret_cast<const compv_float32_t*>(valuePtr);
		COMPV_CHECK_CODE_RETURN(initCoords(m_fRho, fTheta, m_nThreshold, m_nWidth, m_nHeight));
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_HOUGH_SET_INT_THRESHOLD: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int) || *reinterpret_cast<const int*>(valuePtr) <= 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		const int nThreshold = *reinterpret_cast<const int*>(valuePtr);
		COMPV_CHECK_CODE_RETURN(initCoords(m_fRho, m_fTheta, static_cast<size_t>(nThreshold), m_nWidth, m_nHeight));
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_HOUGH_SET_INT_MAXLINES: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_nMaxLines = static_cast<size_t>(reinterpret_cast<const int*>(valuePtr) <= 0  ? INT_MAX : *reinterpret_cast<const int*>(valuePtr));
		return COMPV_ERROR_CODE_S_OK;
	}
	default: {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Set with id %d not implemented", id);
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}
	}
}

COMPV_ERROR_CODE CompVHoughStd::process(const CompVMatPtr& edges, CompVHoughLineVector& lines) /*Overrides(CompVHough)*/
{
	COMPV_CHECK_EXP_RETURN(!edges || edges->isEmpty() || edges->subType() != COMPV_SUBTYPE_PIXELS_Y, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Edges null or not grayscale");

	// Init coords (sine and cosine tables)
	COMPV_CHECK_CODE_RETURN(initCoords(m_fRho, m_fTheta, m_nThreshold, edges->cols(), edges->rows()));

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation");

	lines.clear();
	
	// Compute number of threads
	size_t /*threadsCountNMS = 1,*/ threadsCountACC = 1;
#if 0
	CompVPtr<CompVThreadDispatcher11* >threadDisp = CompVEngine::getThreadDispatcher11();
	if (threadDisp && !threadDisp->isMotherOfTheCurrentThread()) {
		threadsCountNMS = (m_NMS->rows() * m_NMS->cols()) / COMPV_FEATURE_HOUGHSTD_NMS_MIN_SAMPLES_PER_THREAD;
		threadsCountNMS = COMPV_MATH_MIN_3(threadsCountNMS, m_NMS->rows(), (size_t)threadDisp->getThreadsCount());
		threadsCountACC = (edges->rows() * edges->cols()) / COMPV_FEATURE_HOUGHSTD_ACC_MIN_SAMPLES_PER_THREAD;
		threadsCountACC = COMPV_MATH_MIN_3(threadsCountACC, edges->rows(), (size_t)threadDisp->getThreadsCount());
	}
#endif

	// Accumulator gathering
	CompVHoughAccThreadsCtx accThreadsCtx = { 0 };
	accThreadsCtx.threadsCount = threadsCountACC;
#if 0
	if (threadsCountACC > 1) {
		COMPV_CHECK_CODE_RETURN(CompVMutex::newObj(&accThreadsCtx.mutex));
		const size_t countAny = (size_t)(edges->rows() / threadsCountACC);
		const size_t countLast = (size_t)countAny + (edges->rows() % threadsCountACC);
		auto funcPtrAccGather = [&](size_t rowStart, size_t rowCount, size_t threadIdx) -> COMPV_ERROR_CODE {
			COMPV_CHECK_CODE_RETURN(acc_gather(rowStart, rowCount, edges, &accThreadsCtx));
			return COMPV_ERROR_CODE_S_OK;
		};
		size_t rowStart, threadIdx;
		CompVAsyncTaskIds taskIds;
		taskIds.reserve(threadsCountACC);
		for (threadIdx = 0, rowStart = 0; threadIdx < threadsCountACC - 1; ++threadIdx, rowStart += countAny) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrAccGather, rowStart, countAny, threadIdx), taskIds));
		}
		COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrAccGather, rowStart, countLast, threadIdx), taskIds));
		COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds));
	}
	else 
#endif
	{
		COMPV_CHECK_CODE_RETURN(acc_gather(0, edges->rows(), edges, &accThreadsCtx));
	}

	// Non-maxima suppression (NMS)
#if 0
	if (threadsCountNMS > 1)
	{
		std::vector<CompVPtrBox(CompVCoordPolar2f) > coords(threadsCountNMS);
		const size_t countAny = (size_t)(m_NMS->rows() / threadsCountNMS);
		const size_t countLast = (size_t)countAny + (m_NMS->rows() % threadsCountNMS);
		auto funcPtrNmsGather = [&](size_t rowStart, size_t rowCount) -> COMPV_ERROR_CODE {
			COMPV_CHECK_CODE_RETURN(nms_gather(rowStart, rowCount, accThreadsCtx.acc));
			return COMPV_ERROR_CODE_S_OK;
		};
		auto funcPtrNmsApply = [&](size_t rowStart, size_t rowCount, size_t threadIdx) -> COMPV_ERROR_CODE {
			COMPV_CHECK_CODE_RETURN(nms_apply(rowStart, rowCount, accThreadsCtx.acc, coords[threadIdx]));
			return COMPV_ERROR_CODE_S_OK;
		};
		size_t rowStart, threadIdx;
		// NMS gathering
		CompVAsyncTaskIds taskIds;
		taskIds.reserve(threadsCountNMS);
		for (threadIdx = 0, rowStart = 0; threadIdx < threadsCountNMS - 1; ++threadIdx, rowStart += countAny) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrNmsGather, rowStart, countAny), taskIds));
		}
		COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrNmsGather, rowStart, countLast), taskIds));
		COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds));
		// NMS-apply
		taskIds.clear();
		for (threadIdx = 0, rowStart = 0; threadIdx < threadsCountNMS - 1; ++threadIdx, rowStart += countAny) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrNmsApply, rowStart, countAny, threadIdx), taskIds));
		}
		COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrNmsApply, rowStart, countLast, threadIdx), taskIds));
		for (threadIdx = 0; threadIdx < threadsCountNMS; ++threadIdx) {
			COMPV_CHECK_CODE_RETURN(threadDisp->waitOne(taskIds[threadIdx]));
			if (coords[threadIdx] && !coords[threadIdx]->empty()) {
				COMPV_CHECK_CODE_RETURN(m_Coords->append(coords[threadIdx]->begin(), coords[threadIdx]->end()));
			}
		}
	}
	else 
#endif
	{
		COMPV_CHECK_CODE_RETURN(nms_gather(0, m_NMS->rows(), accThreadsCtx.acc));
		COMPV_CHECK_CODE_RETURN(nms_apply(0, m_NMS->rows(), accThreadsCtx.acc, lines));
	}

	// Sort result and retain best
	if (!lines.empty()) {
		std::sort(lines.begin(), lines.end(), [](const CompVHoughLine & a, const CompVHoughLine & b) -> bool {
			return a.strength > b.strength;
		});
		const size_t maxLines = COMPV_MATH_MIN(lines.size(), m_nMaxLines);
		lines.resize(maxLines);
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVHoughStd::newObj(CompVHoughPtrPtr hough, float rho COMPV_DEFAULT(1.f), float theta COMPV_DEFAULT(kfMathTrigPiOver180), size_t threshold COMPV_DEFAULT(1))
{
	COMPV_CHECK_EXP_RETURN(!hough || rho <=0 || rho > 1.f, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVHoughPtr hough_ = new CompVHoughStd(rho, theta, threshold);
	COMPV_CHECK_EXP_RETURN(!hough_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	*hough = *hough_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVHoughStd::initCoords(float fRho, float fTheta, size_t nThreshold, size_t nWidth/*=0*/, size_t nHeight/*=0*/)
{
	nWidth = !nWidth ? m_nWidth : nWidth;
	nHeight = !nHeight ? m_nHeight : nHeight;
	if (m_fRho != fRho || m_fTheta != fTheta || m_nWidth != nWidth || m_nHeight != nHeight) {
		// rho = x*cos(t) + y*sin(t), "cos" and "sin" in [-1, 1] which means we have (x+y)*2 values
		const size_t maxRhoCount = COMPV_MATH_ROUNDFU_2_NEAREST_INT((((nWidth + nHeight) << 1) + 1) / fRho, size_t);
		const size_t maxThetaCount = COMPV_MATH_ROUNDFU_2_NEAREST_INT(kfMathTrigPi / fTheta, size_t);
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<uint8_t>(&m_NMS, maxRhoCount, maxThetaCount));
		COMPV_CHECK_CODE_RETURN(m_NMS->zero_rows());
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<int32_t>(&m_SinRho, 1, maxThetaCount));
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<int32_t>(&m_CosRho, 1, maxThetaCount));

		int32_t* pSinRho = m_SinRho->ptr<int32_t>();
		int32_t* pCosRho = m_CosRho->ptr<int32_t>();
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

COMPV_ERROR_CODE CompVHoughStd::acc_gather(size_t rowStart, size_t rowCount, const CompVMatPtr& edges, CompVHoughAccThreadsCtx* threadsCtx)
{
	CompVMatPtr acc; // CompVMatPtr<int32_t>
	size_t nmsStride, accStride;
	COMPV_CHECK_CODE_RETURN(m_NMS->strideInElts(nmsStride));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<int32_t>(&acc, m_NMS->rows(), m_NMS->cols()));
	COMPV_CHECK_CODE_RETURN(acc->zero_rows());
	accStride = acc->stride();
	//const int32_t accStrideInt32 = static_cast<int32_t>(accStride);
	int rEnd = static_cast<int>(COMPV_MATH_MIN((rowStart + rowCount), edges->rows()));
	int rStart = static_cast<int>(COMPV_MATH_MAX(0, rowStart));
	int colsInt = static_cast<int>(edges->cols());
	const size_t edgeStride = edges->strideInBytes();
	const uint8_t* pixels = edges->ptr<const uint8_t>(rStart);
	const int32_t maxThetaCount = static_cast<int32_t>(acc->cols());
	const int32_t* pSinRho = m_SinRho->ptr<const int32_t>();
	const int32_t* pCosRho = m_CosRho->ptr<const int32_t>();
	int32_t *pACC = acc->ptr<int32_t>(m_nBarrier);
	int32_t theta, rhoInt32;
	int maxCol = -1;
	int maxRow = -1;
	bool haveFastImpl = false;
	bool multiThreaded = (threadsCtx->threadsCount > 1);

#if COMPV_ARCH_X86 && 0
	void(*HoughStdAccGatherRow)(int32_t* pACC, int32_t accStride, const uint8_t* pixels, int32_t maxCols, int32_t maxThetaCount, int32_t row, COMPV_ALIGNED(X) const int32_t* pCosRho, COMPV_ALIGNED(X) const int32_t* pSinRho, int32_t* maxCol) = NULL;
	if (maxThetaCount >= 4 && CompVCpu::isEnabled(compv::kCpuFlagSSE41) && COMPV_IS_ALIGNED_SSE(pixels) && COMPV_IS_ALIGNED_SSE(edgeStride) && COMPV_IS_ALIGNED_SSE(pCosRho) && COMPV_IS_ALIGNED_SSE(pSinRho)) {
		COMPV_EXEC_IFDEF_INTRIN_X86((HoughStdAccGatherRow = HoughStdAccGatherRow_Intrin_SSE41, haveFastImpl = true));
	}
	if (HoughStdAccGatherRow) {
		int32_t mCol = -1;
		for (int32_t row = rStart; row < rEnd; ++row) {
			HoughStdAccGatherRow(pACC, accStrideInt32, pixels, colsInt32, maxThetaCount, row, pCosRho, pSinRho, &mCol);
			if (multiThreaded && mCol != -1) {
				maxCol = COMPV_MATH_MAX(maxCol, mCol);
				maxRow = row;
			}
			pixels += edgeStride;
		}
	}
#endif /* COMPV_ARCH_X86 */

	if (!haveFastImpl) {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD for GPU implementation found");
		int col;
		for (int row = rStart; row < rEnd; ++row) {
			for (col = 0; col < colsInt; ++col) {
				if (pixels[col]) {
					if (multiThreaded) {
						maxRow = row;
						maxCol = COMPV_MATH_MAX(maxCol, col);
					}
					for (theta = 0; theta < maxThetaCount; ++theta) {
						rhoInt32 = (col * pCosRho[theta] + row * pSinRho[theta]) >> 16;
						pACC[theta - (rhoInt32 * accStride)]++; //!\\ Not thread-safe
					}
				}
			}
			pixels += edgeStride;
		}
	}
	if (multiThreaded) {
		// multi-threaded
		COMPV_CHECK_CODE_RETURN(threadsCtx->mutex->lock());
		if (!threadsCtx->acc) {
			threadsCtx->acc = acc;
		}
		else if (maxCol != -1) { // sum only if there is at least #1 non-null pixel
			const size_t sumStart = m_nBarrier - (maxCol + maxRow); // "cos=1, sin=1"
			const size_t sumEnd = m_nBarrier - (-maxCol); //"cos=-1, sin=0"
			COMPV_CHECK_CODE_ASSERT(CompVMathUtils::sum2<int32_t>(
				threadsCtx->acc->ptr<const int32_t>(sumStart), 
				acc->ptr<const int32_t>(sumStart), 
				threadsCtx->acc->ptr<int32_t>(sumStart),
				acc->cols(), 
				(sumEnd - sumStart), 
				acc->stride())
			);
		}
		COMPV_CHECK_CODE_RETURN(threadsCtx->mutex->unlock());
	}
	else {
		// single-threaded
		threadsCtx->acc = acc;
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVHoughStd::nms_gather(size_t rowStart, size_t rowCount, CompVMatPtr& acc)
{
	size_t rows = m_NMS->rows();
	size_t maxCols = m_NMS->cols() - 1;
	size_t rowEnd = COMPV_MATH_MIN((rowStart + rowCount), (rows - 1));
	rowStart = COMPV_MATH_MAX(1, rowStart);

	const int32_t *pACC = acc->ptr<const int32_t>(rowStart);
	uint8_t* pNMS = m_NMS->ptr<uint8_t>(rowStart);
	size_t nmsStride, accStride, row;
	COMPV_CHECK_CODE_RETURN(m_NMS->strideInElts(nmsStride));
	accStride = acc->stride();

#if COMPV_ARCH_X86 && 0 
	void(*HoughStdNmsGatherRow)(const int32_t * pAcc, compv_uscalar_t nAccStride, uint8_t* pNms, int32_t nThreshold, compv_uscalar_t width) = NULL;
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

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
	for (row = rowStart; row < rowEnd; ++row) {
		HoughStdNmsGatherRow_C(pACC, accStride, pNMS, m_nThreshold, 1, maxCols);
		pACC += accStride, pNMS += nmsStride;
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVHoughStd::nms_apply(size_t rowStart, size_t rowCount, CompVMatPtr& acc, CompVHoughLineVector& lines)
{
	size_t cols = m_NMS->cols();
	int32_t rEnd = static_cast<int32_t>(COMPV_MATH_MIN((rowStart + rowCount), m_NMS->rows()));
	int32_t rStart = static_cast<int32_t>(COMPV_MATH_MAX(0, rowStart));
	uint8_t* pNMS = m_NMS->ptr<uint8_t>(rStart);
	int32_t* pACC = acc->ptr<int32_t>(rStart);
	size_t nmsStride, accStride;
	int32_t nBarrier = static_cast<int32_t>(m_nBarrier);
	COMPV_CHECK_CODE_RETURN(m_NMS->strideInElts(nmsStride));
	accStride = acc->stride();

#if COMPV_ARCH_X86 && 0
	void(*HoughStdNmsApplyRow)(COMPV_ALIGNED(X) int32_t* pACC, COMPV_ALIGNED(X) uint8_t* pNMS, int32_t threshold, compv_float32_t theta, int32_t barrier, int32_t row, size_t maxCols, CompVPtrBox(CompVCoordPolar2f)& coords) = NULL;
	if (cols >= 8 && CompVCpu::isEnabled(compv::kCpuFlagSSE2) && COMPV_IS_ALIGNED_SSE(pACC) && COMPV_IS_ALIGNED_SSE(pNMS)) {
		COMPV_EXEC_IFDEF_INTRIN_X86(HoughStdNmsApplyRow = HoughStdNmsApplyRow_Intrin_SSE2);
	}
	if (HoughStdNmsApplyRow) {
		for (int32_t row = rStart; row < rEnd; ++row) {
			HoughStdNmsApplyRow(pACC, pNMS, m_nThreshold, m_fTheta, nBarrier, row, cols, coords);
			pACC += accStride, pNMS += nmsStride;
		}
		return COMPV_ERROR_CODE_S_OK;
	}
#endif /* COMPV_ARCH_X86 */


	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
	for (int32_t row = rStart; row < rEnd; ++row) {
		HoughStdNmsApplyRow_C(pACC, pNMS, m_nThreshold, m_fTheta, nBarrier, row, 0, cols, lines);
		pACC += accStride, pNMS += nmsStride;
	}
	return COMPV_ERROR_CODE_S_OK;
}

void HoughStdNmsGatherRow_C(const int32_t * pAcc, size_t nAccStride, uint8_t* pNms, size_t nThreshold, size_t colStart, size_t maxCols)
{
	int32_t t;
	const int32_t *curr, *top, *bottom;
	int stride = static_cast<int>(nAccStride);
	const int32_t thresholdInt32 = static_cast<int32_t>(nThreshold);
	for (size_t col = colStart; col < maxCols; ++col) {
		if (pAcc[col] > thresholdInt32) {
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

void HoughStdNmsApplyRow_C(int32_t* pACC, uint8_t* pNMS, size_t threshold, compv_float32_t theta, int32_t barrier, int32_t row, size_t colStart, size_t maxCols, CompVHoughLineVector& lines)
{
	const int32_t thresholdInt32 = static_cast<int32_t>(threshold);
	for (size_t col = colStart; col < maxCols; ++col) {
		if (pNMS[col]) {
			pNMS[col] = 0; // reset for  next time
			// do not push the line
		}
		else if (pACC[col] > thresholdInt32) {
			lines.push_back(CompVHoughLine(
				static_cast<compv_float32_t>(barrier - row), 
				col * theta, 
				static_cast<size_t>(pACC[col])
			));
		}
	}
}

COMPV_NAMESPACE_END()

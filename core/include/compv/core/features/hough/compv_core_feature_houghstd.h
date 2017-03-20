/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_FEATURES_HOUGHSTD_H_)
#define _COMPV_CORE_FEATURES_HOUGHSTD_H_

#include "compv/core/compv_core_config.h"
#include "compv/core/compv_core_common.h"
#include "compv/base/compv_features.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

struct CompVHoughAccThreadsCtx {
	CompVMutexPtr mutex;
	CompVMatPtr acc; // CompVMatPtr<int32_t>
	size_t threadsCount;
};

class CompVHoughStd : public CompVHough
{
protected:
	CompVHoughStd(float rho = 1.f, float theta = kfMathTrigPiOver180, size_t threshold = 1);
public:
	virtual ~CompVHoughStd();
	COMPV_OBJECT_GET_ID(CompVHoughStd);

	virtual COMPV_ERROR_CODE set(int id, const void* valuePtr, size_t valueSize) override /*Overrides(CompVCaps)*/;
	virtual COMPV_ERROR_CODE process(const CompVMatPtr& edges, CompVHoughLineVector& lines) /*Overrides(CompVHough)*/;

	static COMPV_ERROR_CODE newObj(CompVPtr<CompVHough* >* hough, float rho = 1.f, float theta = kfMathTrigPiOver180, size_t threshold = 1);

private:
	COMPV_ERROR_CODE initCoords(float fRho, float fTheta, size_t nThreshold, size_t nWidth = 0, size_t nHeight = 0);
	COMPV_ERROR_CODE acc_gather(size_t rowStart, size_t rowCount, const CompVMatPtr& edges, CompVHoughAccThreadsCtx* threadsCtx);
	COMPV_ERROR_CODE nms_gather(size_t rowStart, size_t rowCount, CompVMatPtr& acc);
	COMPV_ERROR_CODE nms_apply(size_t rowStart, size_t rowCount, CompVMatPtr& acc, CompVHoughLineVector& lines);

private:
	float m_fRho;
	float m_fTheta;
	size_t m_nThreshold;
	size_t m_nMaxLines;
	size_t m_nWidth;
	size_t m_nHeight;
	size_t m_nBarrier;
	CompVMatPtr m_SinRho; // CompVMatPtr<int32_t>
	CompVMatPtr m_CosRho; // CompVMatPtr<int32_t>
	CompVMatPtr m_NMS; // CompVMatPtr<uint8_t>
};

void HoughStdNmsGatherRow_C(const int32_t * pAcc, size_t nAccStride, uint8_t* pNms, size_t nThreshold, size_t colStart, size_t maxCols);
void HoughStdNmsApplyRow_C(int32_t* pACC, uint8_t* pNMS, size_t threshold, compv_float32_t theta, int32_t barrier, int32_t row, size_t colStart, size_t maxCols, CompVHoughLineVector& lines);

COMPV_NAMESPACE_END()

#endif /* _COMPV_CORE_FEATURES_HOUGHSTD_H_ */

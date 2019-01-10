/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_FEATURES_CANNY_DETE_H_) 
#define _COMPV_CORE_FEATURES_CANNY_DETE_H_

#include "compv/core/compv_core_config.h"
#include "compv/core/compv_core_common.h"
#include "compv/base/compv_features.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(EdgeDeteCanny);

class CompVEdgeDeteCanny : public CompVEdgeDete
{
protected:
	CompVEdgeDeteCanny(float tLow, float tHigh, size_t kernSize = 3);
public:
	virtual ~CompVEdgeDeteCanny();
	COMPV_OBJECT_GET_ID(CompVEdgeDeteCanny);

	virtual COMPV_ERROR_CODE set(int id, const void* valuePtr, size_t valueSize) override /*Overrides(CompVCaps)*/;
	virtual COMPV_ERROR_CODE process(const CompVMatPtr& image, CompVMatPtrPtr edges, CompVMatPtrPtr directions = NULL) override /*Overrides(CompVEdgeDete)*/;

	static COMPV_ERROR_CODE newObj(CompVEdgeDetePtrPtr dete, float tLow, float tHigh, size_t kernSize = 3);

private:
	COMPV_ERROR_CODE nms_gather(CompVMatPtr edges, uint16_t tLow, size_t rowStart, size_t rowCount);
	void nms_apply();
	COMPV_ERROR_CODE hysteresis(CompVMatPtr edges, uint16_t tLow, uint16_t tHigh, size_t rowStart, size_t rowCount);
	COMPV_ERROR_CODE direction(const CompVMatPtr& edges, CompVMatPtr& dirs, size_t rowStart, size_t rowCount);

private:
	size_t m_nImageWidth;
	size_t m_nImageHeight;
	size_t m_nImageStride;
	int16_t* m_pGx;
	int16_t* m_pGy;
	uint16_t* m_pG;
	uint8_t* m_pNms;
	const int16_t* m_pcKernelVt;
	const int16_t* m_pcKernelHz;
	size_t m_nKernelSize;
	float m_fThresholdLow;
	float m_fThresholdHigh;
	int m_nThresholdType;
};


static const float kCannyTangentPiOver8 = 0.414213568f; // tan(22.5)
static const int32_t kCannyTangentPiOver8Int = static_cast<int32_t>(kCannyTangentPiOver8 * (1 << 16));
static const float kCannyTangentPiTimes3Over8 = 2.41421366f; // tan(67.5)
static const int32_t kCannyTangentPiTimes3Over8Int = static_cast<int32_t>(kCannyTangentPiTimes3Over8 * (1 << 16));

COMPV_NAMESPACE_END()

#endif /* _COMPV_CORE_FEATURES_CANNY_DETE_H_ */

/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_FEATURES_HOUGHSTD_H_)
#define _COMPV_FEATURES_HOUGHSTD_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/compv_box.h"
#include "compv/features/compv_feature.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVHoughStd : public CompVHough
{
protected:
	CompVHoughStd(float rho = 1.f, float theta = kfMathTrigPiOver180, int32_t threshold = 1);
public:
	virtual ~CompVHoughStd();
	virtual COMPV_INLINE const char* getObjectId() {
		return "CompVHoughStd";
	};

	// override CompVSettable::set
	virtual COMPV_ERROR_CODE set(int id, const void* valuePtr, size_t valueSize);
	// override CompVHough::process
	virtual COMPV_ERROR_CODE process(const CompVPtrArray(uint8_t)& edges, CompVPtrArray(compv_float32x2_t)& coords);

	static COMPV_ERROR_CODE newObj(CompVPtr<CompVHough* >* hough, float rho = 1.f, float theta = kfMathTrigPiOver180, int32_t threshold = 1);

private:
	COMPV_ERROR_CODE initCoords(float fRho, float fTheta, int32_t nThreshold, size_t nWidth = 0, size_t nHeight = 0);
	COMPV_ERROR_CODE nms();

private:
	float m_fRho;
	float m_fTheta;
	int32_t m_nThreshold;
	int32_t m_nMaxLines;
	size_t m_nWidth;
	size_t m_nHeight;
	size_t m_nBarrier;
	CompVPtrArray(int32_t) m_SinRho;
	CompVPtrArray(int32_t) m_CosRho;
	CompVPtrArray(int32_t) m_Accumulator;
	CompVPtrArray(uint8_t) m_NMS;
	CompVPtrBox(CompVCoordPolar2f) m_Coords;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_FEATURES_HOUGHSTD_H_ */

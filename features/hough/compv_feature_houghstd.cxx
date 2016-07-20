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
COMPV_ERROR_CODE CompVHoughStd::process(const CompVPtrArray(uint8_t)& edges)
{
	COMPV_CHECK_EXP_RETURN(!edges || edges->isEmpty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);


	float theta = 0.f;
	//int32_t rho;

	for (; theta < kfMathTrigPiTimes2; theta += m_fTheta) {
		//for (rho = 0; rho < )
	}

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

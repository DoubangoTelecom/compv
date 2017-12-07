/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_FEATURES_ORB_DETE_H_)
#define _COMPV_CORE_FEATURES_ORB_DETE_H_

#include "compv/core/compv_core_config.h"
#include "compv/core/compv_core_common.h"
#include "compv/base/compv_features.h"
#include "compv/base/compv_patch.h"
#include "compv/base/image/compv_image_scale_pyramid.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(CornerDeteORB);

class CompVCornerDeteORB : public CompVCornerDete
{
protected:
	CompVCornerDeteORB();
public:
	virtual ~CompVCornerDeteORB();
	COMPV_OBJECT_GET_ID(CompVCornerDeteORB);

	virtual COMPV_ERROR_CODE set(int id, const void* valuePtr, size_t valueSize) override /*Overrides(CompVCaps)*/;
	virtual COMPV_ERROR_CODE get(int id, const void** valuePtrPtr, size_t valueSize) override /*Overrides(CompVCaps)*/;
	virtual COMPV_ERROR_CODE process(const CompVMatPtr& image, CompVInterestPointVector& interestPoints) override /*Overrides(CompVCornerDete)*/;

	static COMPV_ERROR_CODE newObj(CompVCornerDetePtrPtr orb);

private:
	COMPV_ERROR_CODE createInterestPoints(size_t count = 0);
	COMPV_ERROR_CODE initDetector(CompVCornerDetePtr detector);
	COMPV_ERROR_CODE initDetectors();
	COMPV_ERROR_CODE processLevelAt(const CompVMatPtr& image, CompVPatchPtr& patch, CompVCornerDetePtr& detector, int level);

private:
	CompVImageScalePyramidPtr m_pyramid;
	std::vector<CompVInterestPointVector > m_vecInterestPointsAtLevelN;
	std::vector<CompVPatchPtr > m_vecPatches;
	std::vector<CompVCornerDetePtr > m_vecDetectors;
	int m_nMaxFeatures;
	int m_nPyramidLevels;
	int m_nThreshold;
	int m_nFastType;
	bool m_bNMS;
	int m_nPatchDiameter;
	CompVMatPtr m_ptrImageGray;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CORE_FEATURES_ORB_DETE_H_ */

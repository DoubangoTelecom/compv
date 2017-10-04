/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_IMAGE_SCALE_PYRAMID_H_)
#define _COMPV_BASE_IMAGE_SCALE_PYRAMID_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_mat.h"

COMPV_NAMESPACE_BEGIN()

#undef COMPV_PYRAMID_LEVEL_FIRST
#undef COMPV_PYRAMID_LEVEL_ALL
#define COMPV_PYRAMID_LEVEL_FIRST	1
#define COMPV_PYRAMID_LEVEL_ALL		0xffffff

COMPV_OBJECT_DECLARE_PTRS(ImageScalePyramid)

class COMPV_BASE_API CompVImageScalePyramid : public CompVObj
{
protected:
	CompVImageScalePyramid(float fScaleFactor = 0.83f, size_t nLevels = 8, COMPV_INTERPOLATION_TYPE eScaleType = COMPV_INTERPOLATION_TYPE_BILINEAR);
public:
	virtual ~CompVImageScalePyramid();
	COMPV_OBJECT_GET_ID(CompVImageScalePyramid);
	COMPV_INLINE size_t levels() {
		return m_nLevels;
	}
	COMPV_INLINE float scaleFactorsSum() {
		return m_fScaleFactorsSum;
	}
	COMPV_INLINE COMPV_INTERPOLATION_TYPE scaleType() {
		return m_eScaleType;
	}
	COMPV_INLINE float scaleFactorFirst() {
		return scaleFactor(COMPV_PYRAMID_LEVEL_FIRST);
	}
	float scaleFactor(size_t level = COMPV_PYRAMID_LEVEL_FIRST/*for level 0 it's always equal to 1.f*/);
	COMPV_ERROR_CODE process(const CompVMatPtr& inImage, size_t level = COMPV_PYRAMID_LEVEL_ALL);
	COMPV_ERROR_CODE image(size_t level, CompVMatPtrPtr image);

	static COMPV_ERROR_CODE newObj(CompVImageScalePyramidPtrPtr pyramid, float fScaleFactor = 0.83f, size_t nLevels = 8, COMPV_INTERPOLATION_TYPE eScaleType = COMPV_INTERPOLATION_TYPE_BILINEAR);

private:
	COMPV_ERROR_CODE processLevel(const CompVMatPtr& inImage, size_t level);

private:
	bool m_bValid;
	float m_fScaleFactor;
	float m_fScaleFactorsSum; // Sum of all scale factors (all levels added)
	size_t m_nLevels;
	COMPV_INTERPOLATION_TYPE m_eScaleType;
	CompVMatPtrPtr m_pImages;
	float *m_pScaleFactors;
	
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_IMAGE_SCALE_PYRAMID_H_ */

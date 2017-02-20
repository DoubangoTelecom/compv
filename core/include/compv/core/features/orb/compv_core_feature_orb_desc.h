/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_FEATURES_ORB_DESC_H_)
#define _COMPV_CORE_FEATURES_ORB_DESC_H_

#include "compv/core/compv_core_config.h"
#include "compv/core/compv_core_common.h"
#include "compv/base/compv_features.h"
#include "compv/base/compv_patch.h"
#include "compv/base/image/compv_image_scale_pyramid.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if !defined(COMPV_FEATURE_DESC_ORB_FXP_DESC)
#	define COMPV_FEATURE_DESC_ORB_FXP_DESC					0 // Disable/Enable 'describe()' fixed point implementation. /!\ Must be disabled as it's buggy.
#endif
#if !defined(COMPV_FEATURE_DESC_ORB_FXP_CONVLT)
#	define COMPV_FEATURE_DESC_ORB_FXP_CONVLT				1 // Disable/Enable 'convlt()' fixed point implementation
#endif

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(CornerDescORB);

class CompVCornerDescORB : public CompVCornerDesc
{
protected:
	CompVCornerDescORB();
public:
	virtual ~CompVCornerDescORB();
	COMPV_OBJECT_GET_ID(CompVCornerDescORB);

	virtual COMPV_ERROR_CODE set(int id, const void* valuePtr, size_t valueSize) override /*Overrides(CompVCaps)*/;
	virtual COMPV_ERROR_CODE get(int id, const void** valuePtrPtr, size_t valueSize) override /*Overrides(CompVCaps)*/;
	virtual COMPV_ERROR_CODE process(const CompVMatPtr& image, const CompVInterestPointVector& interestPoints, CompVMatPtrPtr descriptions) override /*Overrides(CompVCornerDesc)*/;

	static COMPV_ERROR_CODE newObj(CompVCornerDescPtrPtr orb);

private:
	COMPV_ERROR_CODE convlt(CompVImageScalePyramidPtr pPyramid, int level);
	COMPV_ERROR_CODE describe(CompVImageScalePyramidPtr pPyramid, CompVInterestPointVector::const_iterator begin, CompVInterestPointVector::const_iterator end, uint8_t* desc, size_t desc_stride);

private:
	// TODO(dmi): use internal detector: BRIEF (just like what is done for the detector and FAST internal dete)
	CompVImageScalePyramidPtr m_pyramid;
	CompVMatPtr m_kern_float;
	CompVMatPtr m_kern_fxp;
	CompVMatPtr m_image_blurred_prev;
	int m_nPatchDiameter;
	int m_nPatchBits;
	CompVMatPtr m_ptrImageGray;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CORE_FEATURES_ORB_DESC_H_ */

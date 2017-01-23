/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_FEATURES_ORB_DESC_H_)
#define _COMPV_FEATURES_ORB_DESC_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/compv_convlt.h"
#include "compv/compv_array.h"
#include "compv/image/scale/compv_imagescale_pyramid.h"
#include "compv/features/compv_feature.h"

#if !defined(COMPV_FEATURE_DESC_ORB_FXP_DESC)
#	define COMPV_FEATURE_DESC_ORB_FXP_DESC					0 // Disable/Enable 'describe()' fixed point implementation. /!\ Must be disabled as it's buggy.
#endif
#if !defined(COMPV_FEATURE_DESC_ORB_FXP_CONVLT)
#	define COMPV_FEATURE_DESC_ORB_FXP_CONVLT				1 // Disable/Enable 'convlt()' fixed point implementation
#endif

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVCornerDescORB : public CompVCornerDesc
{
protected:
    CompVCornerDescORB();
public:
    virtual ~CompVCornerDescORB();
    virtual COMPV_INLINE const char* getObjectId() {
        return "CompVCornerDescORB";
    };

    // override CompVSettable::set
    virtual COMPV_ERROR_CODE set(int id, const void* valuePtr, size_t valueSize);
    // override CompVCornerDesc::process
    virtual COMPV_ERROR_CODE process(const CompVPtr<CompVImage*>& image, const CompVPtr<CompVBoxInterestPoint* >& interestPoints, CompVPtr<CompVArray<uint8_t>* >* descriptions);

    static COMPV_ERROR_CODE newObj(CompVPtr<CompVCornerDesc* >* orb);

private:
    COMPV_ERROR_CODE convlt(CompVPtr<CompVImageScalePyramid * > pPyramid, int level);
    COMPV_ERROR_CODE describe(CompVPtr<CompVImageScalePyramid * > pPyramid, const CompVInterestPoint* begin, const CompVInterestPoint* end, uint8_t* desc);

private:
    // TODO(dmi): use internal detector: BRIEF (just like what is done for the detector and FAST internal dete)
    CompVPtr<CompVImageScalePyramid* > m_pyramid;
    CompVPtr<CompVConvlt<float>* > m_convlt;
    CompVPtr<CompVArray<float>* > m_kern_float;
    CompVPtr<CompVArray<uint16_t>* > m_kern_fxp;
    CompVPtr<CompVImage* > m_image_blurred_prev;
    bool m_bMediaTypeVideo;
    int m_nPatchDiameter;
    int m_nPatchBits;
    void(*m_funBrief256_31_Float32)(const uint8_t* img_center, compv_scalar_t img_stride, const float* cos1, const float* sin1, COMPV_ALIGNED(x) void* out);
#if COMPV_FEATURE_DESC_ORB_FXP_DESC
    void(*m_funBrief256_31_Fxp)(const uint8_t* img_center, compv_scalar_t img_stride, const int16_t* cos1, const int16_t* sin1, COMPV_ALIGNED(x) void* out);
#endif
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_FEATURES_ORB_DESC_H_ */

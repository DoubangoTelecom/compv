/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_IMAGE_SCALE_IMAGESCALE_PYRAMID_H_)
#define _COMPV_IMAGE_SCALE_IMAGESCALE_PYRAMID_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/image/compv_image.h"

COMPV_NAMESPACE_BEGIN()

#define COMPV_PYRAMOD_LEVEL_FIRST 1

class COMPV_API CompVImageScalePyramid : public CompVObj
{
protected:
    CompVImageScalePyramid(float fScaleFactor, int32_t nLevels, COMPV_SCALE_TYPE eScaleType = COMPV_SCALE_TYPE_BILINEAR);
public:
    virtual ~CompVImageScalePyramid();
    virtual COMPV_INLINE const char* getObjectId() {
        return "CompVImageScalePyramid";
    };
    COMPV_INLINE int32_t getLevels() {
        return m_nLevels;
    }
    COMPV_INLINE float getScaleFactorsSum() {
        return m_fScaleFactorsSum;
    }
    COMPV_INLINE COMPV_SCALE_TYPE getScaleType() {
        return m_eScaleType;
    }
    COMPV_INLINE float getScaleFactorFirst() {
        return getScaleFactor(COMPV_PYRAMOD_LEVEL_FIRST);
    }
    float getScaleFactor(int32_t level = COMPV_PYRAMOD_LEVEL_FIRST/*for level 0 it's always equal to 1.f*/);
    COMPV_ERROR_CODE process(const CompVPtr<CompVImage*>& inImage, int32_t level = -1);
    COMPV_ERROR_CODE getImage(int32_t level, CompVPtr<CompVImage *>* image);

    static COMPV_ERROR_CODE newObj(float fScaleFactor, int32_t nLevels, COMPV_SCALE_TYPE eScaleType, CompVPtr<CompVImageScalePyramid*>* pyramid);

private:
    COMPV_ERROR_CODE processLevelAt(const CompVPtr<CompVImage*>& inImage, int32_t level);
    static COMPV_ERROR_CODE processLevelAt_AsynExec(const struct compv_asynctoken_param_xs* pc_params);

private:
    float m_fScaleFactor;
    float m_fScaleFactorsSum; // Sum of all scale factors (all levels added)
    int32_t m_nLevels;
    COMPV_SCALE_TYPE m_eScaleType;
    CompVPtr<CompVImage *>* m_pImages;
    float *m_pScaleFactors;
    bool m_bValid;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_IMAGE_SCALE_IMAGESCALE_PYRAMID_H_ */

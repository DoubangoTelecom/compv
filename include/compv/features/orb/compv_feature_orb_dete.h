/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_FEATURES_ORB_DETE_H_)
#define _COMPV_FEATURES_ORB_DETE_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/image/scale/compv_imagescale_pyramid.h"
#include "compv/features/compv_feature.h"
#include "compv/parallel/compv_mutex.h"
#include "compv/compv_patch.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVFeatureDeteORB : public CompVFeatureDete
{
protected:
    CompVFeatureDeteORB();
public:
    virtual ~CompVFeatureDeteORB();
    virtual COMPV_INLINE const char* getObjectId() {
        return "CompVFeatureDeteORB";
    };

    // override CompVSettable::set
    virtual COMPV_ERROR_CODE set(int id, const void* valuePtr, size_t valueSize);
    // override CompVSettable::get
    virtual COMPV_ERROR_CODE get(int id, const void*& valuePtr, size_t valueSize);
    // override CompVFeatureDete::process
    virtual COMPV_ERROR_CODE process(const CompVPtr<CompVImage*>& image, CompVPtr<CompVBoxInterestPoint* >& interestPoints);

    static COMPV_ERROR_CODE newObj(CompVPtr<CompVFeatureDete* >* orb);

private:
    COMPV_ERROR_CODE createInterestPoints(int32_t count = -1);
    COMPV_ERROR_CODE freeInterestPoints(int32_t count = -1);
    COMPV_ERROR_CODE createPatches(int32_t count = -1);
    COMPV_ERROR_CODE freePatches(int32_t count = -1);
    COMPV_ERROR_CODE createDetectors(int32_t count = -1);
    COMPV_ERROR_CODE initDetector(CompVPtr<CompVFeatureDete* >& detector);
    COMPV_ERROR_CODE initDetectors();
    COMPV_ERROR_CODE freeDetectors(int32_t count = -1);
    COMPV_ERROR_CODE processLevelAt(const CompVPtr<CompVImage*>& image, CompVPtr<CompVPatch* >& patch, CompVPtr<CompVFeatureDete* >& detector, int level);
    static COMPV_ERROR_CODE processLevelAt_AsynExec(const struct compv_asynctoken_param_xs* pc_params);

private:
    CompVPtr<CompVImageScalePyramid* > m_pyramid;
    CompVPtr<CompVBoxInterestPoint* >* m_pInterestPointsAtLevelN;
    CompVPtr<CompVPatch* >* m_pPatches;
    size_t m_nPatches;
    CompVPtr<CompVFeatureDete* >* m_pDetectors;
    size_t m_nDetectors;
    int32_t m_nMaxFeatures;
    int32_t m_nPyramidLevels;
    int32_t m_nThreshold;
    int32_t m_nFastType;
    bool m_bNMS;
    int m_nPatchDiameter;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_FEATURES_ORB_DETE_H_ */

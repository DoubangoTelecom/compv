/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_FEATURES_FEATURE_H_)
#define _COMPV_FEATURES_FEATURE_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/compv_obj.h"
#include "compv/compv_settable.h"
#include "compv/compv_array.h"
#include "compv/compv_buffer.h"
#include "compv/compv_interestpoint.h"
#include "compv/image/compv_image.h"

COMPV_NAMESPACE_BEGIN()

class CompVFeatureDete;
class CompVFeatureDesc;


struct CompVFeatureFactory {
    int id;
    const char* name;
    COMPV_ERROR_CODE(*newObjDete)(CompVPtr<CompVFeatureDete* >* dete);
    COMPV_ERROR_CODE(*newObjDesc)(CompVPtr<CompVFeatureDesc* >* desc);
};

/* Feature detectors and descriptors setters and getters */
enum {
    /* Common to all features */
    COMPV_FEATURE_GET_PTR_PYRAMID,

    /* FAST (Features from Accelerated Segment Test) */
    COMPV_FAST_ID,
    COMPV_FAST_SET_INT32_THRESHOLD,
    COMPV_FAST_SET_INT32_MAX_FEATURES,
    COMPV_FAST_SET_INT32_FAST_TYPE,
    COMPV_FAST_SET_BOOL_NON_MAXIMA_SUPP,
    COMPV_FAST_TYPE_9,
    COMPV_FAST_TYPE_12,

    /*  ORB (Oriented FAST and Rotated BRIEF) */
    COMPV_ORB_ID,
    COMPV_ORB_SET_INT32_INTERNAL_DETE_ID,
    COMPV_ORB_SET_INT32_FAST_THRESHOLD,
    COMPV_ORB_SET_BOOL_FAST_NON_MAXIMA_SUPP,
    COMPV_ORB_SET_INT32_PYRAMID_LEVELS,
    COMPV_ORB_SET_INT32_PYRAMID_SCALE_TYPE,
    COMPV_ORB_SET_FLOAT_PYRAMID_SCALE_FACTOR,
    COMPV_ORB_SET_INT32_MAX_FEATURES,
    COMPV_ORB_SET_INT32_STRENGTH_TYPE,
    COMPV_ORB_SET_INT32_BRIEF_PATCH_SIZE,
    COMPV_ORB_STRENGTH_TYPE_FAST,
    COMPV_ORB_STRENGTH_TYPE_HARRIS,
};

// Class: CompVFeature
class COMPV_API CompVFeature : public CompVObj, public CompVSettable
{
protected:
    CompVFeature();
public:
    virtual ~CompVFeature();
    static COMPV_ERROR_CODE init();
    static COMPV_ERROR_CODE addFactory(const CompVFeatureFactory* factory);
    static const CompVFeatureFactory* findFactory(int deteId);

private:
    COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
    static std::map<int, const CompVFeatureFactory*> s_Factories;
    COMPV_DISABLE_WARNINGS_END()
};

// Class: CompVFeatureDete
class COMPV_API CompVFeatureDete : public CompVObj, public CompVSettable
{
protected:
    CompVFeatureDete(int id);
public:
    virtual ~CompVFeatureDete();
    COMPV_INLINE int getId() {
        return m_nId;
    }
    virtual COMPV_ERROR_CODE process(const CompVPtr<CompVImage*>& image, CompVPtr<CompVBoxInterestPoint* >& interestPoints) = 0;
    static COMPV_ERROR_CODE newObj(int deteId, CompVPtr<CompVFeatureDete* >* dete);

private:
    int m_nId;
};

// Class: CompVFeatureDesc
class COMPV_API CompVFeatureDesc : public CompVObj, public CompVSettable
{
protected:
    CompVFeatureDesc(int id);
public:
    virtual ~CompVFeatureDesc();
    COMPV_INLINE int getId() {
        return m_nId;
    }
	// Detector must be attached to descriptor only if describe() use the same input as the previous detect()
    virtual COMPV_ERROR_CODE attachDete(CompVPtr<CompVFeatureDete* > dete) {
        m_AttachedDete = dete;
        return COMPV_ERROR_CODE_S_OK;
    }
    virtual COMPV_ERROR_CODE dettachDete() {
        m_AttachedDete = NULL;
        return COMPV_ERROR_CODE_S_OK;
    }
    virtual COMPV_ERROR_CODE process(const CompVPtr<CompVImage*>& image, const CompVPtr<CompVBoxInterestPoint* >& interestPoints, CompVPtr<CompVArray<uint8_t>* >* descriptions) = 0;
    static COMPV_ERROR_CODE newObj(int descId, CompVPtr<CompVFeatureDesc* >* desc);

protected:
    COMPV_INLINE CompVPtr<CompVFeatureDete* >& getAttachedDete() {
        return m_AttachedDete;
    }

private:
    int m_nId;
    COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
    CompVPtr<CompVFeatureDete* >m_AttachedDete;
    COMPV_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_FEATURES_FEATURE_H_ */

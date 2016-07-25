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

class CompVCornerDete;
class CompVCornerDesc;
class CompVEdgeDete;
class CompVHough;

struct CompVFeatureFactory {
    int id;
    const char* name;
    COMPV_ERROR_CODE(*newObjCornerDete)(CompVPtr<CompVCornerDete* >* dete);
    COMPV_ERROR_CODE(*newObjCornerDesc)(CompVPtr<CompVCornerDesc* >* desc);
	COMPV_ERROR_CODE(*newObjEdgeDete)(CompVPtr<CompVEdgeDete* >* dete, float tLow /*= 0.68f*/, float tHigh /*= 0.68f*2.f*/, int32_t kernSize /*= 3*/);
	COMPV_ERROR_CODE(*newObjHough)(CompVPtr<CompVHough* >* hough, float rho /*= 1.f*/, float theta /*= kfMathTrigPiOver180*/, int32_t threshold /*= 1*/);
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
    COMPV_ORB_SET_FLT32_PYRAMID_SCALE_FACTOR,
    COMPV_ORB_SET_INT32_MAX_FEATURES,
    COMPV_ORB_SET_INT32_STRENGTH_TYPE,
    COMPV_ORB_SET_INT32_BRIEF_PATCH_SIZE,
    COMPV_ORB_STRENGTH_TYPE_FAST,
    COMPV_ORB_STRENGTH_TYPE_HARRIS,

	/* Canny */
	COMPV_CANNY_ID,
	COMPV_CANNY_SET_INT32_KERNEL_SIZE,
	COMPV_CANNY_SET_FLT32_THRESHOLD_LOW,
	COMPV_CANNY_SET_FLT32_THRESHOLD_HIGH,

	/* Sobel */
	COMPV_SOBEL_ID,

	/* Scharr */
	COMPV_SCHARR_ID,

	/* Prewitt */
	COMPV_PREWITT_ID,

	/* Hough */
	COMPV_HOUGH_STANDARD_ID,
	COMPV_HOUGH_SET_INT32_RHO,
	COMPV_HOUGH_SET_FLT32_THETA,
	COMPV_HOUGH_SET_INT32_THRESHOLD
};

// https://en.wikipedia.org/wiki/Sobel_operator#Alternative_operators
static const int16_t CompVScharrGx_vt[3] = { 3, 10, 3 };
static const int16_t CompVScharrGx_hz[3] = { -1, 0, 1 };
// https://en.wikipedia.org/wiki/Sobel_operator
static const int16_t CompVSobel3x3Gx_vt[3] = { 1, 2, 1 };
static const int16_t CompVSobel3x3Gx_hz[3] = { -1, 0, 1 };
static const int16_t CompVSobel5x5Gx_vt[5] = { 1, 4, 6, 4, 1 };
static const int16_t CompVSobel5x5Gx_hz[5] = { 1, 2, 0, -2, -1 };
// https://en.wikipedia.org/wiki/Prewitt_operator
static const int16_t CompVPrewittGx_vt[3] = { 1, 1, 1 };
static const int16_t CompVPrewittGx_hz[3] = { -1, 0, 1 };

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

// Class: CompVFeatureBase
class COMPV_API CompVFeatureBase : public CompVObj, public CompVSettable
{
protected:
	CompVFeatureBase(int id);
public:
	virtual ~CompVFeatureBase();
	COMPV_INLINE int getId() {
		return m_nId;
	}
private:
	int m_nId;
};

// Class: CompVCornerDete
class COMPV_API CompVCornerDete : public CompVFeatureBase
{
protected:
    CompVCornerDete(int id);
public:
    virtual ~CompVCornerDete();
    virtual COMPV_ERROR_CODE process(const CompVPtr<CompVImage*>& image, CompVPtr<CompVBoxInterestPoint* >& interestPoints) = 0;
    static COMPV_ERROR_CODE newObj(int deteId, CompVPtr<CompVCornerDete* >* dete);
};

// Class: CompVCornerDesc
class COMPV_API CompVCornerDesc : public CompVFeatureBase
{
protected:
    CompVCornerDesc(int id);
public:
    virtual ~CompVCornerDesc();
	// Detector must be attached to descriptor only if describe() use the same input as the previous detect()
    COMPV_INLINE virtual COMPV_ERROR_CODE attachDete(CompVPtr<CompVCornerDete* > dete) {
        m_AttachedDete = dete;
        return COMPV_ERROR_CODE_S_OK;
    }
    COMPV_INLINE virtual COMPV_ERROR_CODE dettachDete() {
        m_AttachedDete = NULL;
        return COMPV_ERROR_CODE_S_OK;
    }
    virtual COMPV_ERROR_CODE process(const CompVPtr<CompVImage*>& image, const CompVPtr<CompVBoxInterestPoint* >& interestPoints, CompVPtr<CompVArray<uint8_t>* >* descriptions) = 0;
    static COMPV_ERROR_CODE newObj(int descId, CompVPtr<CompVCornerDesc* >* desc);

protected:
    COMPV_INLINE CompVPtr<CompVCornerDete* >& getAttachedDete() {
        return m_AttachedDete;
    }

private:
    COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
    CompVPtr<CompVCornerDete* >m_AttachedDete;
    COMPV_DISABLE_WARNINGS_END()
};

// Class: CompVEdgeDete
class COMPV_API CompVEdgeDete : public CompVFeatureBase
{
protected:
	CompVEdgeDete(int id);
public:
	virtual ~CompVEdgeDete();
	virtual COMPV_ERROR_CODE process(const CompVPtr<CompVImage*>& image, CompVPtrArray(uint8_t)& edges) = 0;
	static COMPV_ERROR_CODE newObj(int deteId, CompVPtr<CompVEdgeDete* >* dete, float tLow = 0.68f, float tHigh = 0.68f*2.f, int32_t kernSize = 3);
};

// Class: CompVHough
class COMPV_API CompVHough : public CompVFeatureBase
{
protected:
	CompVHough(int id);
public:
	virtual ~CompVHough();
	virtual COMPV_ERROR_CODE process(const CompVPtrArray(uint8_t)& edges, CompVPtrArray(compv_float32x2_t)& coords) = 0;
	static COMPV_ERROR_CODE newObj(int id, CompVPtr<CompVHough* >* hough, float rho = 1.f, float theta = kfMathTrigPiOver180, int32_t threshold = 1);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_FEATURES_FEATURE_H_ */

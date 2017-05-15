/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_FEATURES_H_)
#define _COMPV_BASE_FEATURES_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_caps.h"
#include "compv/base/compv_mat.h"
#include "compv/base/compv_box_interestpoint.h"

#include <map>
#include <vector>

#if !defined(COMPV_FEATURE_DETE_EDGE_THRESHOLD_LOW)
#	define COMPV_FEATURE_DETE_EDGE_THRESHOLD_LOW	(0.80f) // low threshold -> low speed (more data to process)
#endif
#if !defined(COMPV_FEATURE_DETE_EDGE_THRESHOLD_HIGH)
#	define COMPV_FEATURE_DETE_EDGE_THRESHOLD_HIGH	(COMPV_FEATURE_DETE_EDGE_THRESHOLD_LOW * 2.f)
#endif


COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(CornerDete)
COMPV_OBJECT_DECLARE_PTRS(CornerDesc)
COMPV_OBJECT_DECLARE_PTRS(EdgeDete)
COMPV_OBJECT_DECLARE_PTRS(Hough)

struct CompVFeatureFactory {
	int id;
	const char* name;
	COMPV_ERROR_CODE(*newObjCornerDete)(CompVCornerDetePtrPtr dete);
	COMPV_ERROR_CODE(*newObjCornerDesc)(CompVCornerDescPtrPtr desc);
	COMPV_ERROR_CODE(*newObjEdgeDete)(CompVEdgeDetePtrPtr dete, float tLow /*= COMPV_FEATURE_DETE_EDGE_THRESHOLD_LOW */, float tHigh /*= COMPV_FEATURE_DETE_EDGE_THRESHOLD_HIGH*/, size_t kernSize /*= 3*/);
	COMPV_ERROR_CODE(*newObjHough)(CompVHoughPtrPtr hough, float rho /*= 1.f*/, float theta /*= kfMathTrigPiOver180*/, size_t threshold /*= 1*/);
};

/* Feature detectors and descriptors setters and getters */
enum {
	/* Common to all features */
	COMPV_FEATURE_GET_PTR_PYRAMID,

	/* FAST (Features from Accelerated Segment Test) */
	COMPV_FAST_ID,
	COMPV_FAST_SET_INT_THRESHOLD,
	COMPV_FAST_SET_INT_MAX_FEATURES,
	COMPV_FAST_SET_INT_FAST_TYPE,
	COMPV_FAST_SET_BOOL_NON_MAXIMA_SUPP,
	COMPV_FAST_TYPE_9,
	COMPV_FAST_TYPE_12,

	/*  ORB (Oriented FAST and Rotated BRIEF) */
	COMPV_ORB_ID,
	COMPV_ORB_SET_INT_INTERNAL_DETE_ID,
	COMPV_ORB_SET_INT_FAST_THRESHOLD,
	COMPV_ORB_SET_BOOL_FAST_NON_MAXIMA_SUPP,
	COMPV_ORB_SET_INT_PYRAMID_LEVELS,
	COMPV_ORB_SET_INT_PYRAMID_SCALE_TYPE,
	COMPV_ORB_SET_FLT32_PYRAMID_SCALE_FACTOR,
	COMPV_ORB_SET_INT_MAX_FEATURES,
	COMPV_ORB_SET_INT_STRENGTH_TYPE,
	COMPV_ORB_SET_INT_BRIEF_PATCH_SIZE,
	COMPV_ORB_STRENGTH_TYPE_FAST,
	COMPV_ORB_STRENGTH_TYPE_HARRIS,

	/* Canny */
	COMPV_CANNY_ID,
	COMPV_CANNY_SET_INT_KERNEL_SIZE,
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
	COMPV_HOUGH_SET_FLT32_RHO,
	COMPV_HOUGH_SET_FLT32_THETA,
	COMPV_HOUGH_SET_INT_THRESHOLD,
	COMPV_HOUGH_SET_INT_MAXLINES
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
class COMPV_BASE_API CompVFeature : public CompVObj, public CompVCaps
{
protected:
	CompVFeature();
public:
	virtual ~CompVFeature();
	static COMPV_ERROR_CODE addFactory(const CompVFeatureFactory* factory);
	static const CompVFeatureFactory* findFactory(int deteId);

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	static std::map<int, const CompVFeatureFactory*> s_Factories;
	COMPV_VS_DISABLE_WARNINGS_END()
};

// Class: CompVFeatureBase
class COMPV_BASE_API CompVFeatureBase : public CompVObj, public CompVCaps
{
protected:
	CompVFeatureBase(int id);
public:
	virtual ~CompVFeatureBase();
	COMPV_INLINE int id() {
		return m_nId;
	}
private:
	int m_nId;
};

// Class: CompVCornerDete
class COMPV_BASE_API CompVCornerDete : public CompVFeatureBase
{
protected:
	CompVCornerDete(int id);
public:
	virtual ~CompVCornerDete();
	virtual COMPV_ERROR_CODE process(const CompVMatPtr& image, CompVInterestPointVector& interestPoints) = 0;
	static COMPV_ERROR_CODE newObj(CompVCornerDetePtrPtr dete, int deteId);
};

// Class: CompVCornerDesc
class COMPV_BASE_API CompVCornerDesc : public CompVFeatureBase
{
protected:
	CompVCornerDesc(int id);
public:
	virtual ~CompVCornerDesc();
	// Detector must be attached to descriptor only if describe() use the same input as the previous detect()
	COMPV_INLINE virtual COMPV_ERROR_CODE attachDete(CompVCornerDetePtr dete) {
		m_ptrAttachedDete = dete;
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_INLINE virtual COMPV_ERROR_CODE dettachDete() {
		m_ptrAttachedDete = NULL;
		return COMPV_ERROR_CODE_S_OK;
	}
	virtual COMPV_ERROR_CODE process(const CompVMatPtr& image, const CompVInterestPointVector& interestPoints, CompVMatPtrPtr descriptions) = 0;
	static COMPV_ERROR_CODE newObj(CompVCornerDescPtrPtr desc, int descId, CompVCornerDetePtr dete = NULL);

protected:
	COMPV_INLINE CompVCornerDetePtr& attachedDete() {
		return m_ptrAttachedDete;
	}

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
		CompVCornerDetePtr m_ptrAttachedDete;
	COMPV_VS_DISABLE_WARNINGS_END()
};

// Class: CompVEdgeDete
class COMPV_BASE_API CompVEdgeDete : public CompVFeatureBase
{
protected:
	CompVEdgeDete(int id);
public:
	virtual ~CompVEdgeDete();
	virtual COMPV_ERROR_CODE process(const CompVMatPtr& image, CompVMatPtrPtr edges, CompVMatPtrPtr directions = NULL) = 0;
	static COMPV_ERROR_CODE newObj(CompVEdgeDetePtrPtr dete, int deteId, float tLow = COMPV_FEATURE_DETE_EDGE_THRESHOLD_LOW, float tHigh = COMPV_FEATURE_DETE_EDGE_THRESHOLD_HIGH, size_t kernSize = 3);
};

// Class: CompVHough
class COMPV_BASE_API CompVHough : public CompVFeatureBase
{
protected:
	CompVHough(int id);
public:
	virtual ~CompVHough();
	virtual COMPV_ERROR_CODE process(const CompVMatPtr& edges, CompVHoughLineVector& lines, const CompVMatPtr& directions = NULL) = 0;
	static COMPV_ERROR_CODE newObj(CompVHoughPtrPtr hough, int id, float rho = 1.f, float theta = kfMathTrigPiOver180, size_t threshold = 1);
};


COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_FEATURES_H_ */

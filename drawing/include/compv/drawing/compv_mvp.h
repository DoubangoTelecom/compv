/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_MVP_H_)
#define _COMPV_DRAWING_MVP_H_

#include "compv/drawing/compv_config.h"
#include "compv/base/compv_obj.h"
#include "compv/drawing/compv_common.h"
#include "compv/drawing/compv_common.h"

#if !defined(COMPV_MVP_PROJ_FOVY)
#	define COMPV_MVP_PROJ_FOVY			90.f
#endif
#if !defined(COMPV_MVP_PROJ_ASPECT_RATIO)
#	define COMPV_MVP_PROJ_ASPECT_RATIO	1.f
#endif
#if !defined(COMPV_MVP_PROJ_NEAR)
#	define COMPV_MVP_PROJ_NEAR			0.1f
#endif
#if !defined(COMPV_MVP_PROJ_FAR)
#	define COMPV_MVP_PROJ_FAR			100.f
#endif

#if !defined(COMPV_MVP_VIEW_EYE)
#	define COMPV_MVP_VIEW_EYE		0, 0, 1 // Camera is at (0, 0, 0), in World Space
#endif
#if !defined(COMPV_MVP_VIEW_TARGET)
#	define COMPV_MVP_VIEW_TARGET	0, 0, 0 // and looks at the origin
#endif
#if !defined(COMPV_MVP_VIEW_UP)
#	define COMPV_MVP_VIEW_UP		0, 1, 0 // Head is up(set to 0, -1, 0 to look upside - down)
#endif

COMPV_NAMESPACE_BEGIN()

enum COMPV_DRAWING_PROJECTION {
	COMPV_DRAWING_PROJECTION_2D,
	COMPV_DRAWING_PROJECTION_3D
};

//
//	CompVDrawingMat4f
//
class CompVDrawingMat4f;
typedef CompVPtr<CompVDrawingMat4f* > CompVDrawingMat4fPtr;
typedef CompVDrawingMat4fPtr* CompVDrawingMat4fPtrPtr;

class COMPV_DRAWING_API CompVDrawingMat4f : public CompVObj
{
protected:
	CompVDrawingMat4f();
public:
	virtual ~CompVDrawingMat4f();
	virtual const float* ptr()const = 0;
	virtual COMPV_ERROR_CODE translate(const CompVDrawingVec3f& vec3f) = 0;
	virtual COMPV_ERROR_CODE scale(const CompVDrawingVec3f& vec3f) = 0;
	virtual COMPV_ERROR_CODE rotate(float angle, const CompVDrawingVec3f& vec3f) = 0;
};

//
//	CompVDrawingModel
//
class CompVDrawingModel;
typedef CompVPtr<CompVDrawingModel* > CompVDrawingModelPtr;
typedef CompVDrawingModelPtr* CompVDrawingModelPtrPtr;

class COMPV_DRAWING_API CompVDrawingModel : public CompVObj
{
protected:
	CompVDrawingModel();
public:
	virtual ~CompVDrawingModel();
	virtual CompVDrawingMat4fPtr matrix() = 0;
	virtual COMPV_ERROR_CODE reset() = 0;
};

//
//	CompVDrawingView
//
class CompVDrawingView;
typedef CompVPtr<CompVDrawingView* > CompVDrawingViewPtr;
typedef CompVDrawingViewPtr* CompVDrawingViewPtrPtr;

class COMPV_DRAWING_API CompVDrawingView : public CompVObj
{
protected:
	CompVDrawingView();
public:
	virtual ~CompVDrawingView();

	virtual CompVDrawingMat4fPtr matrix() = 0;
	virtual COMPV_ERROR_CODE setCamera(const CompVDrawingVec3f& eye, const CompVDrawingVec3f& target, const CompVDrawingVec3f& up) = 0;
	virtual COMPV_ERROR_CODE reset() = 0;
};

//
//	CompVDrawingProjection
//
class CompVDrawingProjection;
typedef CompVPtr<CompVDrawingProjection* > CompVDrawingProjectionPtr;
typedef CompVDrawingProjectionPtr* CompVDrawingProjectionPtrPtr;

class COMPV_DRAWING_API CompVDrawingProjection : public CompVObj
{
protected:
	CompVDrawingProjection(COMPV_DRAWING_PROJECTION eType);
public:
	virtual ~CompVDrawingProjection();
	COMPV_INLINE COMPV_DRAWING_PROJECTION type()const { return m_eType; }

	virtual CompVDrawingMat4fPtr matrix() = 0;
	virtual COMPV_ERROR_CODE reset() = 0;

private:
	COMPV_DRAWING_PROJECTION m_eType;
};

//
//	CompVDrawingProjection2D
//
class CompVDrawingProjection2D;
typedef CompVPtr<CompVDrawingProjection2D* > CompVDrawingProjection2DPtr;
typedef CompVDrawingProjection2DPtr* CompVDrawingProjection2DPtrPtr;

class COMPV_DRAWING_API CompVDrawingProjection2D : public CompVDrawingProjection
{
protected:
	CompVDrawingProjection2D();
public:
	virtual ~CompVDrawingProjection2D();

	virtual COMPV_ERROR_CODE setOrtho(float left, float right, float bottom, float top, float zNear, float zFar) = 0;
};

//
//	CompVDrawingProjection3D
//
class CompVDrawingProjection3D;
typedef CompVPtr<CompVDrawingProjection3D* > CompVDrawingProjection3DPtr;
typedef CompVDrawingProjection3DPtr* CompVDrawingProjection3DPtrPtr;

class COMPV_DRAWING_API CompVDrawingProjection3D : public CompVDrawingProjection
{
protected:
	CompVDrawingProjection3D();
public:
	virtual ~CompVDrawingProjection3D();

	virtual COMPV_ERROR_CODE setPerspective(float fovy = COMPV_MVP_PROJ_FOVY, float aspect = COMPV_MVP_PROJ_ASPECT_RATIO, float near = COMPV_MVP_PROJ_NEAR, float far = COMPV_MVP_PROJ_FAR) = 0;
};

//
//	CompVMVP
//
class CompVMVP;
typedef CompVPtr<CompVMVP* > CompVMVPPtr;
typedef CompVMVPPtr* CompVMVPPtrPtr;

class COMPV_DRAWING_API CompVMVP : public CompVObj
{
protected:
	CompVMVP();
public:
	virtual ~CompVMVP();
	virtual CompVDrawingMat4fPtr matrix() = 0;
	virtual CompVDrawingModelPtr model() = 0;
	virtual CompVDrawingViewPtr view() = 0;
	virtual CompVDrawingProjection2DPtr projection2D() = 0;
	virtual CompVDrawingProjection3DPtr projection3D() = 0;
	virtual COMPV_ERROR_CODE reset() = 0;
	
	static COMPV_ERROR_CODE newObj(CompVMVPPtrPtr mvp, COMPV_DRAWING_PROJECTION eProjectionType);
	static COMPV_ERROR_CODE newObjProjection2D(CompVMVPPtrPtr mvp);
	static COMPV_ERROR_CODE newObjProjection3D(CompVMVPPtrPtr mvp);

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_DRAWING_MVP_H_ */

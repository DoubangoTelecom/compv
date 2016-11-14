/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_MVP_H_)
#define _COMPV_DRAWING_MVP_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_common.h"
#include "compv/drawing/compv_common.h"

COMPV_NAMESPACE_BEGIN()

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
	virtual COMPV_ERROR_CODE setEyePos(const CompVDrawingVec3f& vec3f) = 0;
	virtual COMPV_ERROR_CODE setTargetPos(const CompVDrawingVec3f& vec3f) = 0;
	virtual COMPV_ERROR_CODE setUpPos(const CompVDrawingVec3f& vec3f) = 0;
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
	CompVDrawingProjection();
public:
	virtual ~CompVDrawingProjection();

	virtual CompVDrawingMat4fPtr matrix() = 0;
	virtual COMPV_ERROR_CODE setFOVY(float fovy = 90.f) = 0;
	virtual COMPV_ERROR_CODE setAspectRatio(float aspect = -1.f) = 0;
	virtual COMPV_ERROR_CODE setNearFar(float near = 0.1f, float far = 100.f) = 0;
	virtual COMPV_ERROR_CODE reset() = 0;
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
	virtual CompVDrawingProjectionPtr projection() = 0;
	virtual COMPV_ERROR_CODE reset() = 0;

	static COMPV_ERROR_CODE newObj(CompVMVPPtrPtr mvp);

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_DRAWING_MVP_H_ */

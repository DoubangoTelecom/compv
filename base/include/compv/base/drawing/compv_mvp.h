/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_DRAWING_MVP_H_)
#define _COMPV_BASE_DRAWING_MVP_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_common.h"

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

enum COMPV_PROJECTION {
	COMPV_PROJECTION_2D,
	COMPV_PROJECTION_3D
};

//
//	CompVMat4f
//
class CompVMat4f;
typedef CompVPtr<CompVMat4f* > CompVMat4fPtr;
typedef CompVMat4fPtr* CompVMat4fPtrPtr;

class COMPV_BASE_API CompVMat4f : public CompVObj
{
protected:
	CompVMat4f();
public:
	virtual ~CompVMat4f();
	virtual const float* ptr()const = 0;
	virtual COMPV_ERROR_CODE translate(const CompVVec3f& vec3f) = 0;
	virtual COMPV_ERROR_CODE scale(const CompVVec3f& vec3f) = 0;
	virtual COMPV_ERROR_CODE rotate(float angle, const CompVVec3f& vec3f) = 0;
};

//
//	CompVModel
//
class CompVModel;
typedef CompVPtr<CompVModel* > CompVModelPtr;
typedef CompVModelPtr* CompVModelPtrPtr;

class COMPV_BASE_API CompVModel : public CompVObj
{
protected:
	CompVModel();
public:
	virtual ~CompVModel();
	virtual CompVMat4fPtr matrix() = 0;
	virtual COMPV_ERROR_CODE reset() = 0;
};

//
//	CompVView
//
class CompVView;
typedef CompVPtr<CompVView* > CompVViewPtr;
typedef CompVViewPtr* CompVViewPtrPtr;

class COMPV_BASE_API CompVView : public CompVObj
{
protected:
	CompVView();
public:
	virtual ~CompVView();

	virtual CompVMat4fPtr matrix() = 0;
	virtual COMPV_ERROR_CODE setCamera(const CompVVec3f& eye, const CompVVec3f& target, const CompVVec3f& up) = 0;
	virtual COMPV_ERROR_CODE reset() = 0;
};

//
//	CompVProj
//
class CompVProj;
typedef CompVPtr<CompVProj* > CompVProjPtr;
typedef CompVProjPtr* CompVProjPtrPtr;

class COMPV_BASE_API CompVProj : public CompVObj
{
protected:
	CompVProj(COMPV_PROJECTION eType);
public:
	virtual ~CompVProj();
	COMPV_INLINE COMPV_PROJECTION type()const { return m_eType; }

	virtual CompVMat4fPtr matrix() = 0;
	virtual COMPV_ERROR_CODE reset() = 0;

private:
	COMPV_PROJECTION m_eType;
};

//
//	CompVProj2D
//
class CompVProj2D;
typedef CompVPtr<CompVProj2D* > CompVProj2DPtr;
typedef CompVProj2DPtr* CompVProj2DPtrPtr;

class COMPV_BASE_API CompVProj2D : public CompVProj
{
protected:
	CompVProj2D();
public:
	virtual ~CompVProj2D();

	virtual COMPV_ERROR_CODE setOrtho(float left, float right, float bottom, float top, float zNear, float zFar) = 0;
};

//
//	CompVProj3D
//
class CompVProj3D;
typedef CompVPtr<CompVProj3D* > CompVProj3DPtr;
typedef CompVProj3DPtr* CompVProj3DPtrPtr;

class COMPV_BASE_API CompVProj3D : public CompVProj
{
protected:
	CompVProj3D();
public:
	virtual ~CompVProj3D();

	virtual COMPV_ERROR_CODE setPerspective(float fovy = COMPV_MVP_PROJ_FOVY, float aspect = COMPV_MVP_PROJ_ASPECT_RATIO, float near = COMPV_MVP_PROJ_NEAR, float far = COMPV_MVP_PROJ_FAR) = 0;
};

//
//	CompVMVP
//
class CompVMVP;
typedef CompVPtr<CompVMVP* > CompVMVPPtr;
typedef CompVMVPPtr* CompVMVPPtrPtr;

class COMPV_BASE_API CompVMVP : public CompVObj
{
protected:
	CompVMVP();
public:
	virtual ~CompVMVP();
	virtual CompVMat4fPtr matrix() = 0;
	virtual CompVModelPtr model() = 0;
	virtual CompVViewPtr view() = 0;
	virtual CompVProj2DPtr projection2D() = 0;
	virtual CompVProj3DPtr projection3D() = 0;
	virtual COMPV_ERROR_CODE reset() = 0;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_DRAWING_MVP_H_ */

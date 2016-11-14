/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_OPENGL_MVP_GLM_H_)
#define _COMPV_DRAWING_OPENGL_MVP_GLM_H_

#include "compv/base/compv_config.h"
#include "compv/drawing/opengl/compv_headers_gl.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/drawing/compv_mvp.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_common.h"

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

//
//	CompVDrawingMat4fGLM
//
class CompVDrawingMat4fGLM;
typedef CompVPtr<CompVDrawingMat4fGLM* > CompVDrawingMat4fGLMPtr;
typedef CompVDrawingMat4fGLMPtr* CompVDrawingMat4fGLMPtrPtr;

class CompVDrawingMat4fGLM : public CompVDrawingMat4f
{
protected:
	CompVDrawingMat4fGLM(const glm::mat4& mat4 = glm::mat4(1.0f));
public:
	virtual ~CompVDrawingMat4fGLM();
	COMPV_GET_OBJECT_ID("CompVDrawingMat4fGLM");
	CompVDrawingMat4fGLM& operator=(const glm::mat4& mat4) { m_Matrix = mat4; return *this; }
	operator const glm::mat4&() const { return m_Matrix; }
	virtual const float* ptr()const;
	virtual COMPV_ERROR_CODE translate(const CompVDrawingVec3f& vec3f);
	virtual COMPV_ERROR_CODE scale(const CompVDrawingVec3f& vec3f);
	virtual COMPV_ERROR_CODE rotate(float angle, const CompVDrawingVec3f& vec3f);

	static COMPV_ERROR_CODE newObj(CompVDrawingMat4fGLMPtrPtr mat4f, const glm::mat4& mat4 = glm::mat4(1.0f));

private:
	glm::mat4 m_Matrix;
};

//
//	CompVDrawingModelGLM
//
class CompVDrawingModelGLM;
typedef CompVPtr<CompVDrawingModelGLM* > CompVDrawingModelGLMPtr;
typedef CompVDrawingModelGLMPtr* CompVDrawingModelGLMPtrPtr;

class CompVDrawingModelGLM : public CompVDrawingModel
{
protected:
	CompVDrawingModelGLM();
public:
	virtual ~CompVDrawingModelGLM();
	operator const glm::mat4&() const { return **m_ptrMatrix; }
	CompVDrawingModelGLM& operator=(const glm::mat4& mat4) { **m_ptrMatrix = mat4; return *this; }
	COMPV_GET_OBJECT_ID("CompVDrawingModelGLM");
	virtual CompVDrawingMat4fPtr matrix();
	virtual COMPV_ERROR_CODE reset();
	static COMPV_ERROR_CODE newObj(CompVDrawingModelGLMPtrPtr model);
private:
	CompVDrawingMat4fGLMPtr m_ptrMatrix;
};


//
//	CompVDrawingViewGLM
//
class CompVDrawingViewGLM;
typedef CompVPtr<CompVDrawingViewGLM* > CompVDrawingViewGLMPtr;
typedef CompVDrawingViewGLMPtr* CompVDrawingViewGLMPtrPtr;

class CompVDrawingViewGLM : public CompVDrawingView
{
protected:
	CompVDrawingViewGLM();
public:
	virtual ~CompVDrawingViewGLM();
	operator const glm::mat4&() const { return **m_ptrMatrix; }
	CompVDrawingViewGLM& operator=(const glm::mat4& mat4) { **m_ptrMatrix = mat4; return *this; }
	COMPV_GET_OBJECT_ID("CompVDrawingViewGLM");
	virtual CompVDrawingMat4fPtr matrix();
	virtual COMPV_ERROR_CODE setEyePos(const CompVDrawingVec3f& vec3f);
	virtual COMPV_ERROR_CODE setTargetPos(const CompVDrawingVec3f& vec3f);
	virtual COMPV_ERROR_CODE setUpPos(const CompVDrawingVec3f& vec3f);
	virtual COMPV_ERROR_CODE reset();
	
	static COMPV_ERROR_CODE newObj(CompVDrawingViewGLMPtrPtr view);

private:
	glm::vec3 m_vec3Eye;
	glm::vec3 m_vec3Target;
	glm::vec3 m_vec3Up;
	CompVDrawingMat4fGLMPtr m_ptrMatrix;
};


//
//	CompVDrawingProjectionGLM
//
class CompVDrawingProjectionGLM;
typedef CompVPtr<CompVDrawingProjectionGLM* > CompVDrawingProjectionGLMPtr;
typedef CompVDrawingProjectionGLMPtr* CompVDrawingProjectionGLMPtrPtr;

class CompVDrawingProjectionGLM : public CompVDrawingProjection
{
protected:
	CompVDrawingProjectionGLM();
public:
	virtual ~CompVDrawingProjectionGLM();
	operator const glm::mat4&() const { return **m_ptrMatrix; }
	CompVDrawingProjectionGLM& operator=(const glm::mat4& mat4) { **m_ptrMatrix = mat4; return *this; }
	COMPV_GET_OBJECT_ID("CompVDrawingProjectionGLM");

	virtual CompVDrawingMat4fPtr matrix();
	virtual COMPV_ERROR_CODE setFOVY(float fovy = 90.f);
	virtual COMPV_ERROR_CODE setAspectRatio(float aspect = -1.f);
	virtual COMPV_ERROR_CODE setNearFar(float near = 0.1f, float far = 100.f);
	virtual COMPV_ERROR_CODE reset();
	
	static COMPV_ERROR_CODE newObj(CompVDrawingProjectionGLMPtrPtr proj);

private:
	float m_fFOVY;
	float m_fAspectRatio;
	float m_fNear;
	float m_fFar;
	CompVDrawingMat4fGLMPtr m_ptrMatrix;
};

//
//	CompVMVPGLM
//
class CompVMVPGLM;
typedef CompVPtr<CompVMVPGLM* > CompVMVPGLMPtr;
typedef CompVMVPGLMPtr* CompVMVPGLMPtrPtr;

class CompVMVPGLM : public CompVMVP
{
protected:
	CompVMVPGLM();
public:
	virtual ~CompVMVPGLM();
	operator const glm::mat4&() const { return **m_ptrMatrix; }
	CompVMVPGLM& operator=(const glm::mat4& mat4) { **m_ptrMatrix = mat4; return *this; }
	COMPV_GET_OBJECT_ID("CompVMVPGLM");

	virtual CompVDrawingMat4fPtr matrix();
	virtual CompVDrawingModelPtr model();
	virtual CompVDrawingViewPtr view();
	virtual CompVDrawingProjectionPtr projection();
	virtual COMPV_ERROR_CODE reset();

	static COMPV_ERROR_CODE newObj(CompVMVPGLMPtrPtr mvp);

private:
	CompVDrawingModelGLMPtr m_ptrModel;
	CompVDrawingViewGLMPtr m_ptrView;
	CompVDrawingProjectionGLMPtr m_ptrProjection;
	CompVDrawingMat4fGLMPtr m_ptrMatrix;
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_DRAWING_OPENGL_MVP_GLM_H_ */

/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_OPENGL_MVP_GLM_H_)
#define _COMPV_DRAWING_OPENGL_MVP_GLM_H_

#include "compv/drawing/compv_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/drawing/compv_mvp.h"
#include "compv/base/compv_obj.h"
#include "compv/drawing/compv_common.h"

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
	COMPV_GET_OBJECT_ID(CompVDrawingMat4fGLM);
	COMPV_INLINE const glm::mat4& matrixGLM()const { return m_Matrix; }
	CompVDrawingMat4fGLM& operator=(const glm::mat4& mat4) { m_Matrix = mat4; return *this; }
	operator const glm::mat4&() const { return matrixGLM(); }
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
	COMPV_INLINE const CompVDrawingMat4fGLMPtr& matrixGLM()const { return m_ptrMatrix; }
	operator const glm::mat4&() const { return **matrixGLM(); }
	CompVDrawingModelGLM& operator=(const glm::mat4& mat4) { **m_ptrMatrix = mat4; return *this; }
	COMPV_GET_OBJECT_ID(CompVDrawingModelGLM);
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
	COMPV_INLINE const CompVDrawingMat4fGLMPtr& matrixGLM()const { return m_ptrMatrix; }
	operator const glm::mat4&() const { return **matrixGLM(); }
	CompVDrawingViewGLM& operator=(const glm::mat4& mat4) { **m_ptrMatrix = mat4; return *this; }
	COMPV_GET_OBJECT_ID(CompVDrawingViewGLM);
	virtual CompVDrawingMat4fPtr matrix();
	virtual COMPV_ERROR_CODE setCamera(const CompVDrawingVec3f& eye, const CompVDrawingVec3f& target, const CompVDrawingVec3f& up);
	virtual COMPV_ERROR_CODE reset();
	
	static COMPV_ERROR_CODE newObj(CompVDrawingViewGLMPtrPtr view);

private:
	CompVDrawingMat4fGLMPtr m_ptrMatrix;
};

//
//	CompVDrawingProjection3DGLM
//
class CompVDrawingProjection3DGLM;
typedef CompVPtr<CompVDrawingProjection3DGLM* > CompVDrawingProjection3DGLMPtr;
typedef CompVDrawingProjection3DGLMPtr* CompVDrawingProjection3DGLMPtrPtr;

class CompVDrawingProjection3DGLM : public CompVDrawingProjection3D
{
protected:
	CompVDrawingProjection3DGLM();
public:
	virtual ~CompVDrawingProjection3DGLM();
	COMPV_INLINE const CompVDrawingMat4fGLMPtr& matrixGLM()const { return m_ptrMatrix; }
	operator const glm::mat4&() const { return **matrixGLM(); }
	CompVDrawingProjection3DGLM& operator=(const glm::mat4& mat4) { **m_ptrMatrix = mat4; return *this; }
	COMPV_GET_OBJECT_ID(CompVDrawingProjection3DGLM);

	virtual CompVDrawingMat4fPtr matrix();
	virtual COMPV_ERROR_CODE reset();
	virtual COMPV_ERROR_CODE setPerspective(float fovy = COMPV_MVP_PROJ_FOVY, float aspect = COMPV_MVP_PROJ_ASPECT_RATIO, float near = COMPV_MVP_PROJ_NEAR, float far = COMPV_MVP_PROJ_FAR);

	static COMPV_ERROR_CODE newObj(CompVDrawingProjection3DGLMPtrPtr proj);

private:
	CompVDrawingMat4fGLMPtr m_ptrMatrix;
};


//
//	CompVDrawingProjection2DGLM
//
class CompVDrawingProjection2DGLM;
typedef CompVPtr<CompVDrawingProjection2DGLM* > CompVDrawingProjection2DGLMPtr;
typedef CompVDrawingProjection2DGLMPtr* CompVDrawingProjection2DGLMPtrPtr;

class CompVDrawingProjection2DGLM : public CompVDrawingProjection2D
{
protected:
	CompVDrawingProjection2DGLM();
public:
	virtual ~CompVDrawingProjection2DGLM();
	COMPV_INLINE const CompVDrawingMat4fGLMPtr& matrixGLM()const { return m_ptrMatrix; }
	operator const glm::mat4&() const { return **matrixGLM(); }
	CompVDrawingProjection2DGLM& operator=(const glm::mat4& mat4) { **m_ptrMatrix = mat4; return *this; }
	COMPV_GET_OBJECT_ID(CompVDrawingProjection2DGLM);

	virtual CompVDrawingMat4fPtr matrix();
	virtual COMPV_ERROR_CODE reset();
	virtual COMPV_ERROR_CODE setOrtho(float left = -1.f, float right = 1.f, float bottom = -1.f, float top = 1.f, float zNear = -1.f, float zFar = 1.f);

	static COMPV_ERROR_CODE newObj(CompVDrawingProjection2DGLMPtrPtr proj);

private:
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
	COMPV_GET_OBJECT_ID(CompVMVPGLM);

	virtual CompVDrawingMat4fPtr matrix();
	virtual CompVDrawingModelPtr model();
	virtual CompVDrawingViewPtr view();
	virtual CompVDrawingProjection2DPtr projection2D();
	virtual CompVDrawingProjection3DPtr projection3D();
	virtual COMPV_ERROR_CODE reset();

	static COMPV_ERROR_CODE newObj(CompVMVPGLMPtrPtr mvp, COMPV_DRAWING_PROJECTION eProjectionType);

private:
	CompVDrawingModelGLMPtr m_ptrModel;
	CompVDrawingViewGLMPtr m_ptrView;
	CompVDrawingProjection2DGLMPtr m_ptrProjection2D;
	CompVDrawingProjection3DGLMPtr m_ptrProjection3D;
	CompVDrawingMat4fGLMPtr m_ptrMatrix;
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_DRAWING_OPENGL_MVP_GLM_H_ */

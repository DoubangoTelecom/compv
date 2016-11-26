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
//	CompVMat4fGLM
//
class CompVMat4fGLM;
typedef CompVPtr<CompVMat4fGLM* > CompVMat4fGLMPtr;
typedef CompVMat4fGLMPtr* CompVMat4fGLMPtrPtr;

class CompVMat4fGLM : public CompVMat4f
{
protected:
	CompVMat4fGLM(const glm::mat4& mat4 = glm::mat4(1.0f));
public:
	virtual ~CompVMat4fGLM();
	COMPV_GET_OBJECT_ID(CompVMat4fGLM);
	COMPV_INLINE const glm::mat4& matrixGLM()const { return m_Matrix; }
	CompVMat4fGLM& operator=(const glm::mat4& mat4) { m_Matrix = mat4; return *this; }
	operator const glm::mat4&() const { return matrixGLM(); }
	virtual const float* ptr()const;
	virtual COMPV_ERROR_CODE translate(const CompVVec3f& vec3f);
	virtual COMPV_ERROR_CODE scale(const CompVVec3f& vec3f);
	virtual COMPV_ERROR_CODE rotate(float angle, const CompVVec3f& vec3f);
	

	static COMPV_ERROR_CODE newObj(CompVMat4fGLMPtrPtr mat4f, const glm::mat4& mat4 = glm::mat4(1.0f));

private:
	glm::mat4 m_Matrix;
};

//
//	CompVModelGLM
//
class CompVModelGLM;
typedef CompVPtr<CompVModelGLM* > CompVModelGLMPtr;
typedef CompVModelGLMPtr* CompVModelGLMPtrPtr;

class CompVModelGLM : public CompVModel
{
protected:
	CompVModelGLM();
public:
	virtual ~CompVModelGLM();
	COMPV_INLINE const CompVMat4fGLMPtr& matrixGLM()const { return m_ptrMatrix; }
	operator const glm::mat4&() const { return **matrixGLM(); }
	CompVModelGLM& operator=(const glm::mat4& mat4) { **m_ptrMatrix = mat4; return *this; }
	COMPV_GET_OBJECT_ID(CompVModelGLM);
	virtual CompVMat4fPtr matrix();
	virtual COMPV_ERROR_CODE reset();
	static COMPV_ERROR_CODE newObj(CompVModelGLMPtrPtr model);
private:
	CompVMat4fGLMPtr m_ptrMatrix;
};


//
//	CompVViewGLM
//
class CompVViewGLM;
typedef CompVPtr<CompVViewGLM* > CompVViewGLMPtr;
typedef CompVViewGLMPtr* CompVViewGLMPtrPtr;

class CompVViewGLM : public CompVView
{
protected:
	CompVViewGLM();
public:
	virtual ~CompVViewGLM();
	COMPV_INLINE const CompVMat4fGLMPtr& matrixGLM()const { return m_ptrMatrix; }
	operator const glm::mat4&() const { return **matrixGLM(); }
	CompVViewGLM& operator=(const glm::mat4& mat4) { **m_ptrMatrix = mat4; return *this; }
	COMPV_GET_OBJECT_ID(CompVViewGLM);
	virtual CompVMat4fPtr matrix();
	virtual COMPV_ERROR_CODE setCamera(const CompVVec3f& eye, const CompVVec3f& target, const CompVVec3f& up);
	virtual COMPV_ERROR_CODE reset();
	
	static COMPV_ERROR_CODE newObj(CompVViewGLMPtrPtr view);

private:
	CompVMat4fGLMPtr m_ptrMatrix;
};

//
//	CompVProj3DGLM
//
class CompVProj3DGLM;
typedef CompVPtr<CompVProj3DGLM* > CompVProj3DGLMPtr;
typedef CompVProj3DGLMPtr* CompVProj3DGLMPtrPtr;

class CompVProj3DGLM : public CompVProj3D
{
protected:
	CompVProj3DGLM();
public:
	virtual ~CompVProj3DGLM();
	COMPV_INLINE const CompVMat4fGLMPtr& matrixGLM()const { return m_ptrMatrix; }
	operator const glm::mat4&() const { return **matrixGLM(); }
	CompVProj3DGLM& operator=(const glm::mat4& mat4) { **m_ptrMatrix = mat4; return *this; }
	COMPV_GET_OBJECT_ID(CompVProj3DGLM);

	virtual CompVMat4fPtr matrix();
	virtual COMPV_ERROR_CODE reset();
	virtual COMPV_ERROR_CODE setPerspective(float fovy = COMPV_MVP_PROJ_FOVY, float aspect = COMPV_MVP_PROJ_ASPECT_RATIO, float near = COMPV_MVP_PROJ_NEAR, float far = COMPV_MVP_PROJ_FAR);

	static COMPV_ERROR_CODE newObj(CompVProj3DGLMPtrPtr proj);

private:
	CompVMat4fGLMPtr m_ptrMatrix;
};


//
//	CompVProj2DGLM
//
class CompVProj2DGLM;
typedef CompVPtr<CompVProj2DGLM* > CompVProj2DGLMPtr;
typedef CompVProj2DGLMPtr* CompVProj2DGLMPtrPtr;

class CompVProj2DGLM : public CompVProj2D
{
protected:
	CompVProj2DGLM();
public:
	virtual ~CompVProj2DGLM();
	COMPV_INLINE const CompVMat4fGLMPtr& matrixGLM()const { return m_ptrMatrix; }
	operator const glm::mat4&() const { return **matrixGLM(); }
	CompVProj2DGLM& operator=(const glm::mat4& mat4) { **m_ptrMatrix = mat4; return *this; }
	COMPV_GET_OBJECT_ID(CompVProj2DGLM);

	virtual CompVMat4fPtr matrix();
	virtual COMPV_ERROR_CODE reset();
	virtual COMPV_ERROR_CODE setOrtho(float left = -1.f, float right = 1.f, float bottom = -1.f, float top = 1.f, float zNear = -1.f, float zFar = 1.f);

	static COMPV_ERROR_CODE newObj(CompVProj2DGLMPtrPtr proj);

private:
	CompVMat4fGLMPtr m_ptrMatrix;
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

	virtual CompVMat4fPtr matrix();
	virtual CompVModelPtr model();
	virtual CompVViewPtr view();
	virtual CompVProj2DPtr projection2D();
	virtual CompVProj3DPtr projection3D();
	virtual COMPV_ERROR_CODE reset();

	static COMPV_ERROR_CODE newObj(CompVMVPGLMPtrPtr mvp, COMPV_DRAWING_PROJECTION eProjectionType);

private:
	CompVModelGLMPtr m_ptrModel;
	CompVViewGLMPtr m_ptrView;
	CompVProj2DGLMPtr m_ptrProjection2D;
	CompVProj3DGLMPtr m_ptrProjection3D;
	CompVMat4fGLMPtr m_ptrMatrix;
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_DRAWING_OPENGL_MVP_GLM_H_ */

/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GL_MVP_H_)
#define _COMPV_GL_MVP_H_

#include "compv/gl/compv_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/base/drawing/compv_mvp.h"
#include "compv/base/compv_obj.h"

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

//
//	CompVGLMat4f
//
class CompVGLMat4f;
typedef CompVPtr<CompVGLMat4f* > CompVGLMat4fPtr;
typedef CompVGLMat4fPtr* CompVGLMat4fPtrPtr;

class CompVGLMat4f : public CompVMat4f
{
protected:
	CompVGLMat4f(const glm::mat4& mat4 = glm::mat4(1.0f));
public:
	virtual ~CompVGLMat4f();
	COMPV_GET_OBJECT_ID(CompVGLMat4f);
	COMPV_INLINE const glm::mat4& matrixGLM()const { return m_Matrix; }
	CompVGLMat4f& operator=(const glm::mat4& mat4) { m_Matrix = mat4; return *this; }
	operator const glm::mat4&() const { return matrixGLM(); }

	COMPV_OVERRIDE_DECL1("CompVMat4f", const float*, ptr)() const override;
	COMPV_OVERRIDE_DECL0("CompVMat4f", translate)(const CompVVec3f& vec3f) override;
	COMPV_OVERRIDE_DECL0("CompVMat4f", scale)(const CompVVec3f& vec3f) override;
	COMPV_OVERRIDE_DECL0("CompVMat4f", rotate)(float angle, const CompVVec3f& vec3f) override;	

	static COMPV_ERROR_CODE newObj(CompVGLMat4fPtrPtr mat4f, const glm::mat4& mat4 = glm::mat4(1.0f));

private:
	glm::mat4 m_Matrix;
};

//
//	CompVGLModel
//
class CompVGLModel;
typedef CompVPtr<CompVGLModel* > CompVGLModelPtr;
typedef CompVGLModelPtr* CompVGLModelPtrPtr;

class CompVGLModel : public CompVModel
{
protected:
	CompVGLModel();
public:
	virtual ~CompVGLModel();
	COMPV_GET_OBJECT_ID(CompVGLModel);
	COMPV_INLINE const CompVGLMat4fPtr& matrixGLM()const { return m_ptrMatrix; }
	operator const glm::mat4&() const { return **matrixGLM(); }
	CompVGLModel& operator=(const glm::mat4& mat4) { **m_ptrMatrix = mat4; return *this; }
	
	COMPV_OVERRIDE_DECL1("CompVModel", CompVMat4fPtr, matrix)() override;
	COMPV_OVERRIDE_DECL0("CompVModel", reset)() override;
	
	static COMPV_ERROR_CODE newObj(CompVGLModelPtrPtr model);
private:
	CompVGLMat4fPtr m_ptrMatrix;
};


//
//	CompVGLView
//
class CompVGLView;
typedef CompVPtr<CompVGLView* > CompVGLViewPtr;
typedef CompVGLViewPtr* CompVGLViewPtrPtr;

class CompVGLView : public CompVView
{
protected:
	CompVGLView();
public:
	virtual ~CompVGLView();
	COMPV_GET_OBJECT_ID(CompVGLView);
	COMPV_INLINE const CompVGLMat4fPtr& matrixGLM()const { return m_ptrMatrix; }
	operator const glm::mat4&() const { return **matrixGLM(); }
	CompVGLView& operator=(const glm::mat4& mat4) { **m_ptrMatrix = mat4; return *this; }
	
	COMPV_OVERRIDE_DECL1("CompVView", CompVMat4fPtr, matrix)() override;
	COMPV_OVERRIDE_DECL0("CompVView", setCamera)(const CompVVec3f& eye, const CompVVec3f& target, const CompVVec3f& up) override;
	COMPV_OVERRIDE_DECL0("CompVView", reset)() override;
	
	static COMPV_ERROR_CODE newObj(CompVGLViewPtrPtr view);

private:
	CompVGLMat4fPtr m_ptrMatrix;
};

//
//	CompVGLProj3D
//
class CompVGLProj3D;
typedef CompVPtr<CompVGLProj3D* > CompVGLProj3DPtr;
typedef CompVGLProj3DPtr* CompVGLProj3DPtrPtr;

class CompVGLProj3D : public CompVProj3D
{
protected:
	CompVGLProj3D();
public:
	virtual ~CompVGLProj3D();
	COMPV_GET_OBJECT_ID(CompVGLProj3D);
	COMPV_INLINE const CompVGLMat4fPtr& matrixGLM()const { return m_ptrMatrix; }
	operator const glm::mat4&() const { return **matrixGLM(); }
	CompVGLProj3D& operator=(const glm::mat4& mat4) { **m_ptrMatrix = mat4; return *this; }
	
	COMPV_OVERRIDE_DECL1("CompVProj3D", CompVMat4fPtr, matrix)() override;
	COMPV_OVERRIDE_DECL0("CompVProj3D", setPerspective)(float fovy = COMPV_MVP_PROJ_FOVY, float aspect = COMPV_MVP_PROJ_ASPECT_RATIO, float near = COMPV_MVP_PROJ_NEAR, float far = COMPV_MVP_PROJ_FAR) override;
	COMPV_OVERRIDE_DECL0("CompVProj3D", reset)() override;

	static COMPV_ERROR_CODE newObj(CompVGLProj3DPtrPtr proj);

private:
	CompVGLMat4fPtr m_ptrMatrix;
};


//
//	CompVGLProj2D
//
class CompVGLProj2D;
typedef CompVPtr<CompVGLProj2D* > CompVGLProj2DPtr;
typedef CompVGLProj2DPtr* CompVGLProj2DPtrPtr;

class CompVGLProj2D : public CompVProj2D
{
protected:
	CompVGLProj2D();
public:
	virtual ~CompVGLProj2D();
	COMPV_GET_OBJECT_ID(CompVGLProj2D);
	COMPV_INLINE const CompVGLMat4fPtr& matrixGLM()const { return m_ptrMatrix; }
	operator const glm::mat4&() const { return **matrixGLM(); }
	CompVGLProj2D& operator=(const glm::mat4& mat4) { **m_ptrMatrix = mat4; return *this; }

	COMPV_OVERRIDE_DECL1("CompVProj2D", CompVMat4fPtr, matrix)() override;
	COMPV_OVERRIDE_DECL0("CompVProj2D", setOrtho)(float left = -1.f, float right = 1.f, float bottom = -1.f, float top = 1.f, float zNear = -1.f, float zFar = 1.f) override;
	COMPV_OVERRIDE_DECL0("CompVProj2D", reset)() override;

	static COMPV_ERROR_CODE newObj(CompVGLProj2DPtrPtr proj);

private:
	CompVGLMat4fPtr m_ptrMatrix;
};

//
//	CompVGLMVP
//
class CompVGLMVP;
typedef CompVPtr<CompVGLMVP* > CompVGLMVPPtr;
typedef CompVGLMVPPtr* CompVGLMVPPtrPtr;

class COMPV_GL_API CompVGLMVP : public CompVMVP
{
protected:
	CompVGLMVP();
public:
	virtual ~CompVGLMVP();
	operator const glm::mat4&() const { return **m_ptrMatrix; }
	CompVGLMVP& operator=(const glm::mat4& mat4) { **m_ptrMatrix = mat4; return *this; }
	COMPV_GET_OBJECT_ID(CompVGLMVP);

	COMPV_OVERRIDE_DECL1("CompVMVP", CompVMat4fPtr, matrix)() override;
	COMPV_OVERRIDE_DECL1("CompVMVP", CompVModelPtr, model)() override;
	COMPV_OVERRIDE_DECL1("CompVMVP", CompVViewPtr, view)() override;
	COMPV_OVERRIDE_DECL1("CompVMVP", CompVProj2DPtr, projection2D)() override;
	COMPV_OVERRIDE_DECL1("CompVMVP", CompVProj3DPtr, projection3D)() override;
	COMPV_OVERRIDE_DECL0("CompVMVP", reset)() override;

	static COMPV_ERROR_CODE newObj(CompVGLMVPPtrPtr mvp, COMPV_PROJECTION eProjectionType);

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	CompVGLModelPtr m_ptrModel;
	CompVGLViewPtr m_ptrView;
	CompVGLProj2DPtr m_ptrProjection2D;
	CompVGLProj3DPtr m_ptrProjection3D;
	CompVGLMat4fPtr m_ptrMatrix;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_GL_MVP_H_ */

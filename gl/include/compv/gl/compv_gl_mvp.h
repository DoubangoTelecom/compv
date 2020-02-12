/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GL_MVP_H_)
#define _COMPV_GL_MVP_H_

#include "compv/gl/compv_gl_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/base/drawing/compv_mvp.h"
#include "compv/base/compv_obj.h"

COMPV_GCC_DISABLE_WARNINGS_BEGIN("-Wdocumentation") // xcode warnings about the documentation in GLM
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
COMPV_GCC_DISABLE_WARNINGS_END()

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

//
//	CompVGLMat4f
//
COMPV_OBJECT_DECLARE_PTRS(GLMat4f)

class CompVGLMat4f : public CompVMat4f
{
protected:
    CompVGLMat4f(const glm::mat4& mat4 = glm::mat4(1.0f));
public:
    virtual ~CompVGLMat4f();
    COMPV_OBJECT_GET_ID(CompVGLMat4f);
    COMPV_INLINE const glm::mat4& matrixGLM()const {
        return m_Matrix;
    }
    CompVGLMat4f& operator=(const glm::mat4& mat4) {
        m_Matrix = mat4;
        return *this;
    }
    operator const glm::mat4&() const {
        return matrixGLM();
    }

    virtual const float* ptr() const override /* Overrides(CompVMat4f) */;
	virtual COMPV_ERROR_CODE translate(const CompVVec3f& vec3f) override /* Overrides(CompVMat4f) */;
	virtual COMPV_ERROR_CODE scale(const CompVVec3f& vec3f) override /* Overrides(CompVMat4f) */;
	virtual COMPV_ERROR_CODE rotate(float angle, const CompVVec3f& vec3f) override /* Overrides(CompVMat4f) */;

    static COMPV_ERROR_CODE newObj(CompVGLMat4fPtrPtr mat4f, const glm::mat4& mat4 = glm::mat4(1.0f));

private:
    glm::mat4 m_Matrix;
};

//
//	CompVGLModel
//
COMPV_OBJECT_DECLARE_PTRS(GLModel)

class CompVGLModel : public CompVModel
{
protected:
    CompVGLModel();
public:
    virtual ~CompVGLModel();
    COMPV_OBJECT_GET_ID(CompVGLModel);
    COMPV_INLINE const CompVGLMat4fPtr& matrixGLM()const {
        return m_ptrMatrix;
    }
    operator const glm::mat4&() const {
        return **matrixGLM();
    }
    CompVGLModel& operator=(const glm::mat4& mat4) {
        **m_ptrMatrix = mat4;
        return *this;
    }

    virtual CompVMat4fPtr matrix() override /* Overrides(CompVModel) */;
	virtual COMPV_ERROR_CODE reset() override /* Overrides(CompVModel) */;

    static COMPV_ERROR_CODE newObj(CompVGLModelPtrPtr model);
private:
    CompVGLMat4fPtr m_ptrMatrix;
};


//
//	CompVGLView
//
COMPV_OBJECT_DECLARE_PTRS(GLView)

class CompVGLView : public CompVView
{
protected:
    CompVGLView();
public:
    virtual ~CompVGLView();
    COMPV_OBJECT_GET_ID(CompVGLView);
    COMPV_INLINE const CompVGLMat4fPtr& matrixGLM()const {
        return m_ptrMatrix;
    }
    operator const glm::mat4&() const {
        return **matrixGLM();
    }
    CompVGLView& operator=(const glm::mat4& mat4) {
        **m_ptrMatrix = mat4;
        return *this;
    }

	virtual CompVMat4fPtr matrix() override /*Overrides(CompVView)*/;
	virtual COMPV_ERROR_CODE setCamera(const CompVVec3f& eye, const CompVVec3f& target, const CompVVec3f& up) override /*Overrides(CompVView)*/;
	virtual COMPV_ERROR_CODE reset() override /*Overrides(CompVView)*/;

    static COMPV_ERROR_CODE newObj(CompVGLViewPtrPtr view);

private:
    CompVGLMat4fPtr m_ptrMatrix;
};

//
//	CompVGLProj3D
//
COMPV_OBJECT_DECLARE_PTRS(GLProj3D)

class CompVGLProj3D : public CompVProj3D
{
protected:
    CompVGLProj3D();
public:
    virtual ~CompVGLProj3D();
    COMPV_OBJECT_GET_ID(CompVGLProj3D);
    COMPV_INLINE const CompVGLMat4fPtr& matrixGLM()const {
        return m_ptrMatrix;
    }
    operator const glm::mat4&() const {
        return **matrixGLM();
    }
    CompVGLProj3D& operator=(const glm::mat4& mat4) {
        **m_ptrMatrix = mat4;
        return *this;
    }

    virtual CompVMat4fPtr matrix() override /*Overrides(CompVProj3D)*/;
	virtual COMPV_ERROR_CODE setPerspective(float fovy = COMPV_MVP_PROJ_FOVY, float aspect = COMPV_MVP_PROJ_ASPECT_RATIO, float near = COMPV_MVP_PROJ_NEAR, float far = COMPV_MVP_PROJ_FAR) override /*Overrides(CompVProj3D)*/;
	virtual COMPV_ERROR_CODE reset() override /*Overrides(CompVProj3D)*/;

    static COMPV_ERROR_CODE newObj(CompVGLProj3DPtrPtr proj);

private:
    CompVGLMat4fPtr m_ptrMatrix;
};


//
//	CompVGLProj2D
//
COMPV_OBJECT_DECLARE_PTRS(GLProj2D)

class CompVGLProj2D : public CompVProj2D
{
protected:
    CompVGLProj2D();
public:
    virtual ~CompVGLProj2D();
    COMPV_OBJECT_GET_ID(CompVGLProj2D);
    COMPV_INLINE const CompVGLMat4fPtr& matrixGLM()const {
        return m_ptrMatrix;
    }
    operator const glm::mat4&() const {
        return **matrixGLM();
    }
    CompVGLProj2D& operator=(const glm::mat4& mat4) {
        **m_ptrMatrix = mat4;
        return *this;
    }

    virtual CompVMat4fPtr matrix() override /*Overrides(CompVProj2D)*/;
	virtual COMPV_ERROR_CODE setOrtho(float left = -1.f, float right = 1.f, float bottom = -1.f, float top = 1.f, float zNear = -1.f, float zFar = 1.f) override /*Overrides(CompVProj2D)*/;
	virtual COMPV_ERROR_CODE reset() override /*Overrides(CompVProj2D)*/;

    static COMPV_ERROR_CODE newObj(CompVGLProj2DPtrPtr proj);

private:
    CompVGLMat4fPtr m_ptrMatrix;
};

//
//	CompVGLMVP
//
COMPV_OBJECT_DECLARE_PTRS(GLMVP)

class COMPV_GL_API CompVGLMVP : public CompVMVP
{
protected:
    CompVGLMVP();
public:
    virtual ~CompVGLMVP();
	COMPV_OBJECT_GET_ID(CompVGLMVP);
    operator const glm::mat4&() const {
        return **m_ptrMatrix;
    }
    CompVGLMVP& operator=(const glm::mat4& mat4) {
        **m_ptrMatrix = mat4;
        return *this;
    }

	virtual CompVMat4fPtr matrix() override /*Overrides(CompVMVP)*/;
	virtual CompVModelPtr model() override /*Overrides(CompVMVP)*/;
	virtual CompVViewPtr view() override /*Overrides(CompVMVP)*/;
	virtual CompVProj2DPtr projection2D() override /*Overrides(CompVMVP)*/;
	virtual CompVProj3DPtr projection3D() override /*Overrides(CompVMVP)*/;
    virtual COMPV_ERROR_CODE reset() override /*Overrides(CompVMVP)*/;

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

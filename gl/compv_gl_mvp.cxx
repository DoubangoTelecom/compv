/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/compv_gl_mvp.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/gl/compv_gl.h"
#include "compv/gl/compv_gl_utils.h"

#define COMPV_GLM_MAT4(CompVObjPtr) (const glm::mat4&)(**(CompVObjPtr))

COMPV_NAMESPACE_BEGIN()

//
//	CompVGLMat4f
//
CompVGLMat4f::CompVGLMat4f(const glm::mat4& mat4 /*= glm::mat4(1.0f)*/)
    : CompVMat4f()
    , m_Matrix(mat4)
{

}

CompVGLMat4f::~CompVGLMat4f()
{

}

const float* CompVGLMat4f::ptr() const /*Overrides(CompVMat4f)*/
{
    return &m_Matrix[0][0];
}

COMPV_ERROR_CODE CompVGLMat4f::translate(const CompVVec3f& vec3f) /*Overrides(CompVMat4f)*/
{
    m_Matrix = glm::translate(m_Matrix, glm::vec3(vec3f.x, vec3f.y, vec3f.z));
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLMat4f::scale(const CompVVec3f& vec3f) /*Overrides(CompVMat4f)*/
{
    m_Matrix = glm::scale(m_Matrix, glm::vec3(vec3f.x, vec3f.y, vec3f.z));
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLMat4f::rotate(float angle, const CompVVec3f& vec3f) /*Overrides(CompVMat4f)*/
{
    m_Matrix = glm::rotate(m_Matrix, angle, glm::vec3(vec3f.x, vec3f.y, vec3f.z));
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLMat4f::newObj(CompVGLMat4fPtrPtr mat4f, const glm::mat4& mat4 /*= glm::mat4(1.0f)*/)
{
    COMPV_CHECK_EXP_RETURN(!mat4f, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVGLMat4fPtr mat4f_ = new CompVGLMat4f(mat4);
    COMPV_CHECK_EXP_RETURN(!mat4f_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

    *mat4f = mat4f_;
    return COMPV_ERROR_CODE_S_OK;
}


//
//	CompVGLModel
//

CompVGLModel::CompVGLModel()
    : CompVModel()
{

}

CompVGLModel::~CompVGLModel()
{

}

 CompVMat4fPtr CompVGLModel::matrix() /*Overrides(CompVModel)*/
{
    return *m_ptrMatrix;
}

COMPV_ERROR_CODE CompVGLModel::reset() /*Overrides(CompVModel)*/
{
    if (!m_ptrMatrix) {
        COMPV_CHECK_CODE_RETURN(CompVGLMat4f::newObj(&m_ptrMatrix));
    }
    **m_ptrMatrix = glm::mat4(1.0f); // Identity
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLModel::newObj(CompVGLModelPtrPtr model)
{
    COMPV_CHECK_EXP_RETURN(!model, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVGLModelPtr model_ = new CompVGLModel();
    COMPV_CHECK_EXP_RETURN(!model_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    COMPV_CHECK_CODE_RETURN(model_->reset());

    *model = model_;
    return COMPV_ERROR_CODE_S_OK;
}

//
//	CompVGLView
//

CompVGLView::CompVGLView()
    : CompVView()
{

}

CompVGLView::~CompVGLView()
{

}

CompVMat4fPtr CompVGLView::matrix() /*Overrides(CompVView)*/
{
    return *m_ptrMatrix;
}

COMPV_ERROR_CODE CompVGLView::setCamera(const CompVVec3f& eye, const CompVVec3f& target, const CompVVec3f& up) /*Overrides(CompVView)*/
{
    COMPV_CHECK_EXP_RETURN(!m_ptrMatrix, COMPV_ERROR_CODE_E_INVALID_STATE);
    **m_ptrMatrix = glm::lookAt(glm::vec3(eye.x, eye.y, eye.z), glm::vec3(target.x, target.y, target.z), glm::vec3(up.x, up.y, up.z));
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLView::reset() /*Overrides(CompVView)*/
{
    if (!m_ptrMatrix) {
        COMPV_CHECK_CODE_RETURN(CompVGLMat4f::newObj(&m_ptrMatrix));
    }
    **m_ptrMatrix = glm::lookAt(glm::vec3(COMPV_MVP_VIEW_EYE), glm::vec3(COMPV_MVP_VIEW_TARGET), glm::vec3(COMPV_MVP_VIEW_UP));
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLView::newObj(CompVGLViewPtrPtr view)
{
    COMPV_CHECK_EXP_RETURN(!view, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVGLViewPtr view_ = new CompVGLView();
    COMPV_CHECK_EXP_RETURN(!view_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    COMPV_CHECK_CODE_RETURN(view_->reset());

    *view = view_;
    return COMPV_ERROR_CODE_S_OK;
}

//
//	CompVGLProj3D
//
CompVGLProj3D::CompVGLProj3D()
    : CompVProj3D()
{
    COMPV_CHECK_CODE_ASSERT(reset());
}

CompVGLProj3D::~CompVGLProj3D()
{

}

CompVMat4fPtr CompVGLProj3D::matrix() /*Overrides(CompVProj3D)*/
{
    return *m_ptrMatrix;
}

COMPV_ERROR_CODE CompVGLProj3D::setPerspective(float fovy COMPV_DEFAULT(COMPV_MVP_PROJ_FOVY), float aspect COMPV_DEFAULT(COMPV_MVP_PROJ_ASPECT_RATIO), float near_ COMPV_DEFAULT(OMPV_MVP_PROJ_NEAR), float far_ COMPV_DEFAULT(COMPV_MVP_PROJ_FAR)) /*Overrides(CompVProj3D)*/
{
    COMPV_CHECK_EXP_RETURN(!m_ptrMatrix, COMPV_ERROR_CODE_E_INVALID_STATE);
    **m_ptrMatrix = glm::perspective(glm::radians(fovy), aspect, near_, far_);
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLProj3D::reset() /*Overrides(CompVProj3D)*/
{
    if (!m_ptrMatrix) {
        COMPV_CHECK_CODE_RETURN(CompVGLMat4f::newObj(&m_ptrMatrix));
    }
    COMPV_CHECK_CODE_RETURN(setPerspective());
    return COMPV_ERROR_CODE_S_OK;
}



COMPV_ERROR_CODE CompVGLProj3D::newObj(CompVGLProj3DPtrPtr proj)
{
    COMPV_CHECK_EXP_RETURN(!proj, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVGLProj3DPtr proj_ = new CompVGLProj3D();
    COMPV_CHECK_EXP_RETURN(!proj_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    COMPV_CHECK_CODE_RETURN(proj_->reset());

    *proj = proj_;
    return COMPV_ERROR_CODE_S_OK;
}


//
//	CompVGLProj2D
//
CompVGLProj2D::CompVGLProj2D()
    : CompVProj2D()
{

}

CompVGLProj2D::~CompVGLProj2D()
{

}

CompVMat4fPtr CompVGLProj2D::matrix() /*Overrides(CompVProj2D)*/
{
    return *m_ptrMatrix;
}

COMPV_ERROR_CODE CompVGLProj2D::setOrtho(float left COMPV_DEFAULT(-1.f), float right COMPV_DEFAULT(1.f), float bottom COMPV_DEFAULT(-1.f), float top COMPV_DEFAULT(1.f), float zNear COMPV_DEFAULT(-1.f), float zFar COMPV_DEFAULT(1.f)) /*Overrides(CompVProj2D)*/
{
    COMPV_CHECK_EXP_RETURN(!m_ptrMatrix, COMPV_ERROR_CODE_E_INVALID_STATE);
    **m_ptrMatrix = glm::ortho(left, right, bottom, top, zNear, zFar);
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLProj2D::reset() /*Overrides(CompVProj2D)*/
{
    if (!m_ptrMatrix) {
        COMPV_CHECK_CODE_RETURN(CompVGLMat4f::newObj(&m_ptrMatrix));
    }
    COMPV_CHECK_CODE_RETURN(setOrtho());
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLProj2D::newObj(CompVGLProj2DPtrPtr proj)
{
    COMPV_CHECK_EXP_RETURN(!proj, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVGLProj2DPtr proj_ = new CompVGLProj2D();
    COMPV_CHECK_EXP_RETURN(!proj_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    COMPV_CHECK_CODE_RETURN(proj_->reset());

    *proj = proj_;
    return COMPV_ERROR_CODE_S_OK;
}

//
//	CompVGLMVP
//
CompVGLMVP::CompVGLMVP()
    : CompVMVP()
{

}

CompVGLMVP::~CompVGLMVP()
{

}

CompVMat4fPtr CompVGLMVP::matrix() /*Overrides(CompVMVP)*/
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("update only if one of the matrices is dirty");
    **m_ptrMatrix =
        (m_ptrProjection2D ? m_ptrProjection2D->matrixGLM()->matrixGLM() : m_ptrProjection3D->matrixGLM()->matrixGLM())
        * m_ptrView->matrixGLM()->matrixGLM()
        * m_ptrModel->matrixGLM()->matrixGLM();
    return *m_ptrMatrix;
}

CompVModelPtr CompVGLMVP::model() /*Overrides(CompVMVP)*/
{
    return *m_ptrModel;
}

CompVViewPtr CompVGLMVP::view() /*Overrides(CompVMVP)*/
{
    return *m_ptrView;
}

CompVProj2DPtr CompVGLMVP::projection2D() /*Overrides(CompVMVP)*/
{
    if (m_ptrProjection2D) {
        return *m_ptrProjection2D;
    }
    COMPV_DEBUG_ERROR("Requesting 2D projection from 3D projection");
    return NULL;
}

CompVProj3DPtr CompVGLMVP::projection3D() /*Overrides(CompVMVP)*/
{
    if (m_ptrProjection3D) {
        return *m_ptrProjection3D;
    }
    COMPV_DEBUG_ERROR("Requesting 3D projection from 2D projection");
    return NULL;
}

COMPV_ERROR_CODE CompVGLMVP::reset() /*Overrides(CompVMVP)*/
{
    COMPV_CHECK_CODE_RETURN(model()->reset());
    COMPV_CHECK_CODE_RETURN(view()->reset());
    if (m_ptrProjection2D) {
        COMPV_CHECK_CODE_RETURN(m_ptrProjection2D->reset());
    }
    else if (m_ptrProjection3D) {
        COMPV_CHECK_CODE_RETURN(m_ptrProjection3D->reset());
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLMVP::newObj(CompVGLMVPPtrPtr mvp, COMPV_PROJECTION eProjectionType)
{
    COMPV_CHECK_EXP_RETURN(!mvp, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVGLMVPPtr mvp_ = new CompVGLMVP();
    COMPV_CHECK_EXP_RETURN(!mvp_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    COMPV_CHECK_CODE_RETURN(CompVGLModel::newObj(&mvp_->m_ptrModel));
    COMPV_CHECK_CODE_RETURN(CompVGLView::newObj(&mvp_->m_ptrView));
    switch (eProjectionType) {
    case COMPV_NAMESPACE::COMPV_PROJECTION_2D:
        COMPV_CHECK_CODE_RETURN(CompVGLProj2D::newObj(&mvp_->m_ptrProjection2D));
        break;
    case COMPV_NAMESPACE::COMPV_PROJECTION_3D:
        COMPV_CHECK_CODE_RETURN(CompVGLProj3D::newObj(&mvp_->m_ptrProjection3D));
        break;
    default:
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
        break;
    }
    const glm::mat4& mvpMatrix =
        (mvp_->m_ptrProjection2D ? mvp_->m_ptrProjection2D->matrixGLM()->matrixGLM() : mvp_->m_ptrProjection3D->matrixGLM()->matrixGLM())
        * mvp_->m_ptrView->matrixGLM()->matrixGLM()
        * mvp_->m_ptrModel->matrixGLM()->matrixGLM();
    COMPV_CHECK_CODE_RETURN(CompVGLMat4f::newObj(&mvp_->m_ptrMatrix, mvpMatrix));

    *mvp = mvp_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

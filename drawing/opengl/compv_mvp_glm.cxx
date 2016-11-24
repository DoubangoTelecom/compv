/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/opengl/compv_mvp_glm.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/drawing/compv_drawing.h"
#include "compv/gl/compv_gl_utils.h"

#define COMPV_GLM_MAT4(CompVObjPtr) (const glm::mat4&)(**(CompVObjPtr))

COMPV_NAMESPACE_BEGIN()

//
//	CompVDrawingMat4fGLM
//
CompVDrawingMat4fGLM::CompVDrawingMat4fGLM(const glm::mat4& mat4 /*= glm::mat4(1.0f)*/)
	: CompVDrawingMat4f()
	, m_Matrix(mat4)
{
	
}

CompVDrawingMat4fGLM::~CompVDrawingMat4fGLM()
{

}

const float* CompVDrawingMat4fGLM::ptr()const
{
	return &m_Matrix[0][0];
}

COMPV_ERROR_CODE CompVDrawingMat4fGLM::translate(const CompVDrawingVec3f& vec3f)
{
	m_Matrix = glm::translate(m_Matrix, glm::vec3(vec3f.x, vec3f.y, vec3f.z));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDrawingMat4fGLM::scale(const CompVDrawingVec3f& vec3f)
{
	m_Matrix = glm::scale(m_Matrix, glm::vec3(vec3f.x, vec3f.y, vec3f.z));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDrawingMat4fGLM::rotate(float angle, const CompVDrawingVec3f& vec3f)
{
	m_Matrix = glm::rotate(m_Matrix, angle, glm::vec3(vec3f.x, vec3f.y, vec3f.z));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDrawingMat4fGLM::newObj(CompVDrawingMat4fGLMPtrPtr mat4f, const glm::mat4& mat4 /*= glm::mat4(1.0f)*/)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(!mat4f, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVDrawingMat4fGLMPtr mat4f_ = new CompVDrawingMat4fGLM(mat4);
	COMPV_CHECK_EXP_RETURN(!mat4f_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	*mat4f = mat4f_;
	return COMPV_ERROR_CODE_S_OK;
}


//
//	CompVDrawingModelGLM
//

CompVDrawingModelGLM::CompVDrawingModelGLM()
: CompVDrawingModel()
{

}

CompVDrawingModelGLM::~CompVDrawingModelGLM()
{

}

CompVDrawingMat4fPtr CompVDrawingModelGLM::matrix()
{
	return *m_ptrMatrix;
}

COMPV_ERROR_CODE CompVDrawingModelGLM::reset()
{
	if (!m_ptrMatrix) {
		COMPV_CHECK_CODE_RETURN(CompVDrawingMat4fGLM::newObj(&m_ptrMatrix));
	}
	**m_ptrMatrix = glm::mat4(1.0f); // Identity
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDrawingModelGLM::newObj(CompVDrawingModelGLMPtrPtr model)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(!model, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVDrawingModelGLMPtr model_ = new CompVDrawingModelGLM();
	COMPV_CHECK_EXP_RETURN(!model_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	COMPV_CHECK_CODE_RETURN(model_->reset());

	*model = model_;
	return COMPV_ERROR_CODE_S_OK;
}

//
//	CompVDrawingViewGLM
//

CompVDrawingViewGLM::CompVDrawingViewGLM()
	: CompVDrawingView()
{
	
}

CompVDrawingViewGLM::~CompVDrawingViewGLM()
{

}

CompVDrawingMat4fPtr CompVDrawingViewGLM::matrix()
{
	return *m_ptrMatrix;
}

COMPV_ERROR_CODE CompVDrawingViewGLM::reset()
{
	if (!m_ptrMatrix) {
		COMPV_CHECK_CODE_RETURN(CompVDrawingMat4fGLM::newObj(&m_ptrMatrix));
	}
	**m_ptrMatrix = glm::lookAt(glm::vec3(COMPV_MVP_VIEW_EYE), glm::vec3(COMPV_MVP_VIEW_TARGET), glm::vec3(COMPV_MVP_VIEW_UP));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDrawingViewGLM::setCamera(const CompVDrawingVec3f& eye, const CompVDrawingVec3f& target, const CompVDrawingVec3f& up)
{
	COMPV_CHECK_EXP_RETURN(!m_ptrMatrix, COMPV_ERROR_CODE_E_INVALID_STATE);
	**m_ptrMatrix = glm::lookAt(glm::vec3(eye.x, eye.y, eye.z), glm::vec3(target.x, target.y, target.z), glm::vec3(up.x, up.y, up.z));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDrawingViewGLM::newObj(CompVDrawingViewGLMPtrPtr view)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(!view, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVDrawingViewGLMPtr view_ = new CompVDrawingViewGLM();
	COMPV_CHECK_EXP_RETURN(!view_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	COMPV_CHECK_CODE_RETURN(view_->reset());
	
	*view = view_;
	return COMPV_ERROR_CODE_S_OK;
}

//
//	CompVDrawingProjection3DGLM
//
CompVDrawingProjection3DGLM::CompVDrawingProjection3DGLM()
	: CompVDrawingProjection3D()
{
	COMPV_CHECK_CODE_ASSERT(reset());
}

CompVDrawingProjection3DGLM::~CompVDrawingProjection3DGLM()
{

}

CompVDrawingMat4fPtr CompVDrawingProjection3DGLM::matrix()
{
	return *m_ptrMatrix;
}

COMPV_ERROR_CODE CompVDrawingProjection3DGLM::reset()
{
	if (!m_ptrMatrix) {
		COMPV_CHECK_CODE_RETURN(CompVDrawingMat4fGLM::newObj(&m_ptrMatrix));
	}
	COMPV_CHECK_CODE_RETURN(setPerspective());
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDrawingProjection3DGLM::setPerspective(float fovy /*= COMPV_MVP_PROJ_FOVY*/, float aspect /*= COMPV_MVP_PROJ_ASPECT_RATIO*/, float near_ /*= COMPV_MVP_PROJ_NEAR*/, float far_ /*= COMPV_MVP_PROJ_FAR*/)
{
	COMPV_CHECK_EXP_RETURN(!m_ptrMatrix, COMPV_ERROR_CODE_E_INVALID_STATE);
	**m_ptrMatrix = glm::perspective(glm::radians(fovy), aspect, near_, far_);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDrawingProjection3DGLM::newObj(CompVDrawingProjection3DGLMPtrPtr proj)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(!proj, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVDrawingProjection3DGLMPtr proj_ = new CompVDrawingProjection3DGLM();
	COMPV_CHECK_EXP_RETURN(!proj_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	COMPV_CHECK_CODE_RETURN(proj_->reset());

	*proj = proj_;
	return COMPV_ERROR_CODE_S_OK;
}


//
//	CompVDrawingProjection2DGLM
//
CompVDrawingProjection2DGLM::CompVDrawingProjection2DGLM()
	: CompVDrawingProjection2D()
{
	
}

CompVDrawingProjection2DGLM::~CompVDrawingProjection2DGLM()
{
	
}

CompVDrawingMat4fPtr CompVDrawingProjection2DGLM::matrix()
{
	return *m_ptrMatrix;
}

COMPV_ERROR_CODE CompVDrawingProjection2DGLM::reset()
{
	if (!m_ptrMatrix) {
		COMPV_CHECK_CODE_RETURN(CompVDrawingMat4fGLM::newObj(&m_ptrMatrix));
	}
	COMPV_CHECK_CODE_RETURN(setOrtho());
	return COMPV_ERROR_CODE_S_OK;
}
	
COMPV_ERROR_CODE CompVDrawingProjection2DGLM::setOrtho(float left /*= -1.f*/, float right /*= 1.f*/, float bottom  /*-1.f*/, float top /*=1.f*/, float zNear /*=-1.f*/, float zFar /*= 1.f*/)
{
	COMPV_CHECK_EXP_RETURN(!m_ptrMatrix, COMPV_ERROR_CODE_E_INVALID_STATE);
	**m_ptrMatrix = glm::ortho(left, right, bottom, top, zNear, zFar);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDrawingProjection2DGLM::newObj(CompVDrawingProjection2DGLMPtrPtr proj)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(!proj, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVDrawingProjection2DGLMPtr proj_ = new CompVDrawingProjection2DGLM();
	COMPV_CHECK_EXP_RETURN(!proj_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	COMPV_CHECK_CODE_RETURN(proj_->reset());

	*proj = proj_;
	return COMPV_ERROR_CODE_S_OK;
}

//
//	CompVMVPGLM
//
CompVMVPGLM::CompVMVPGLM()
	: CompVMVP()
{

}

CompVMVPGLM::~CompVMVPGLM()
{
	
}

CompVDrawingMat4fPtr CompVMVPGLM::matrix()
{
	// FIXME(dmi): update only if one of the matrices is dirty
	**m_ptrMatrix = 
		(m_ptrProjection2D ? m_ptrProjection2D->matrixGLM()->matrixGLM() : m_ptrProjection3D->matrixGLM()->matrixGLM())
		* m_ptrView->matrixGLM()->matrixGLM()
		* m_ptrModel->matrixGLM()->matrixGLM();
	return *m_ptrMatrix;
}

CompVDrawingModelPtr CompVMVPGLM::model()
{
	return *m_ptrModel;
}

CompVDrawingViewPtr CompVMVPGLM::view()
{
	return *m_ptrView;
}

CompVDrawingProjection2DPtr CompVMVPGLM::projection2D()
{
	if (m_ptrProjection2D) {
		return *m_ptrProjection2D;
	}
	COMPV_DEBUG_ERROR("Requesting 2D projection from 3D projection");
	return NULL;
}

CompVDrawingProjection3DPtr CompVMVPGLM::projection3D()
{
	if (m_ptrProjection3D) {
		return *m_ptrProjection3D;
	}
	COMPV_DEBUG_ERROR("Requesting 3D projection from 2D projection");
	return NULL;
}

COMPV_ERROR_CODE CompVMVPGLM::reset()
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

COMPV_ERROR_CODE CompVMVPGLM::newObj(CompVMVPGLMPtrPtr mvp, COMPV_DRAWING_PROJECTION eProjectionType)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(!mvp, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMVPGLMPtr mvp_ = new CompVMVPGLM();
	COMPV_CHECK_EXP_RETURN(!mvp_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	COMPV_CHECK_CODE_RETURN(CompVDrawingModelGLM::newObj(&mvp_->m_ptrModel));
	COMPV_CHECK_CODE_RETURN(CompVDrawingViewGLM::newObj(&mvp_->m_ptrView));
	switch (eProjectionType)
	{
	case compv::COMPV_DRAWING_PROJECTION_2D:
		COMPV_CHECK_CODE_RETURN(CompVDrawingProjection2DGLM::newObj(&mvp_->m_ptrProjection2D));
		break;
	case compv::COMPV_DRAWING_PROJECTION_3D:
		COMPV_CHECK_CODE_RETURN(CompVDrawingProjection3DGLM::newObj(&mvp_->m_ptrProjection3D));
		break;
	default:
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
		break;
	}
	const glm::mat4& mvpMatrix =
		(mvp_->m_ptrProjection2D ? mvp_->m_ptrProjection2D->matrixGLM()->matrixGLM() : mvp_->m_ptrProjection3D->matrixGLM()->matrixGLM())
		* mvp_->m_ptrView->matrixGLM()->matrixGLM()
		* mvp_->m_ptrModel->matrixGLM()->matrixGLM();
	COMPV_CHECK_CODE_RETURN(CompVDrawingMat4fGLM::newObj(&mvp_->m_ptrMatrix, mvpMatrix));

	*mvp = mvp_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

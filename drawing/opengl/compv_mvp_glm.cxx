/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/opengl/compv_mvp_glm.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/drawing/compv_drawing.h"
#include "compv/drawing/opengl/compv_utils_gl.h"

#define COMPV_GLM_MAT4(CompVObjPtr) (const glm::mat4&)(**(CompVObjPtr))

#if !defined(COMPV_MVP_PROJ_FOVY)
#	define COMPV_MVP_PROJ_FOVY			90.f
#endif
#if !defined(COMPV_MVP_PROJ_ASPECT_RATIO)
#	define COMPV_MVP_PROJ_ASPECT_RATIO	-1.f
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
	**m_ptrMatrix = glm::mat4(1.0f); // Identity
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDrawingModelGLM::newObj(CompVDrawingModelGLMPtrPtr model)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(!model, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVDrawingModelGLMPtr model_ = new CompVDrawingModelGLM();
	COMPV_CHECK_EXP_RETURN(!model_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	COMPV_CHECK_CODE_RETURN(CompVDrawingMat4fGLM::newObj(&model_->m_ptrMatrix));

	*model = model_;
	return COMPV_ERROR_CODE_S_OK;
}

//
//	CompVDrawingViewGLM
//

CompVDrawingViewGLM::CompVDrawingViewGLM()
	: CompVDrawingView()
{
	m_vec3Eye = glm::vec3(COMPV_MVP_VIEW_EYE);
	m_vec3Target = glm::vec3(COMPV_MVP_VIEW_TARGET);
	m_vec3Up = glm::vec3(COMPV_MVP_VIEW_UP);
}

CompVDrawingViewGLM::~CompVDrawingViewGLM()
{

}

CompVDrawingMat4fPtr CompVDrawingViewGLM::matrix()
{
	return *m_ptrMatrix;
}

COMPV_ERROR_CODE CompVDrawingViewGLM::setEyePos(const CompVDrawingVec3f& vec3f)
{
	m_vec3Eye.x = vec3f.x,	m_vec3Eye.y = vec3f.y, m_vec3Eye.z = vec3f.z;
	**m_ptrMatrix = glm::lookAt(m_vec3Eye, m_vec3Target, m_vec3Up);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDrawingViewGLM::setTargetPos(const CompVDrawingVec3f& vec3f)
{
	m_vec3Target.x = vec3f.x, m_vec3Target.y = vec3f.y, m_vec3Target.z = vec3f.z;
	**m_ptrMatrix = glm::lookAt(m_vec3Eye, m_vec3Target, m_vec3Up);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDrawingViewGLM::setUpPos(const CompVDrawingVec3f& vec3f)
{
	m_vec3Up.x = vec3f.x, m_vec3Up.y = vec3f.y, m_vec3Up.z = vec3f.z;
	**m_ptrMatrix = glm::lookAt(m_vec3Eye, m_vec3Target, m_vec3Up);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDrawingViewGLM::reset()
{
	m_vec3Eye = glm::vec3(COMPV_MVP_VIEW_EYE);
	m_vec3Target = glm::vec3(COMPV_MVP_VIEW_TARGET);
	m_vec3Up = glm::vec3(COMPV_MVP_VIEW_UP);
	**m_ptrMatrix = glm::lookAt(m_vec3Eye, m_vec3Target, m_vec3Up);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDrawingViewGLM::newObj(CompVDrawingViewGLMPtrPtr view)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(!view, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVDrawingViewGLMPtr view_ = new CompVDrawingViewGLM();
	COMPV_CHECK_EXP_RETURN(!view_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	COMPV_CHECK_CODE_RETURN(CompVDrawingMat4fGLM::newObj(&view_->m_ptrMatrix, glm::lookAt(view_->m_vec3Eye, view_->m_vec3Target, view_->m_vec3Up)));

	*view = view_;
	return COMPV_ERROR_CODE_S_OK;
}

//
//	CompVDrawingProjectionGLM
//
CompVDrawingProjectionGLM::CompVDrawingProjectionGLM()
	: CompVDrawingProjection()
	, m_fFOVY(COMPV_MVP_PROJ_FOVY)
	, m_fAspectRatio(COMPV_MVP_PROJ_ASPECT_RATIO)
	, m_fNear(COMPV_MVP_PROJ_NEAR)
	, m_fFar(COMPV_MVP_PROJ_FAR)
{

}

CompVDrawingProjectionGLM::~CompVDrawingProjectionGLM()
{

}

CompVDrawingMat4fPtr CompVDrawingProjectionGLM::matrix()
{
	return *m_ptrMatrix;
}

COMPV_ERROR_CODE CompVDrawingProjectionGLM::setFOVY(float fovy /*= 90.f*/)
{
	m_fFOVY = fovy;
	**m_ptrMatrix = glm::perspective(glm::radians(m_fFOVY), m_fAspectRatio < 0.f ? 1.f : m_fAspectRatio, m_fNear, m_fFar);
	return COMPV_ERROR_CODE_S_OK;
}
	
COMPV_ERROR_CODE CompVDrawingProjectionGLM::setAspectRatio(float aspect /*= -1.f*/)
{
	m_fAspectRatio = aspect;
	**m_ptrMatrix = glm::perspective(glm::radians(m_fFOVY), m_fAspectRatio < 0.f ? 1.f : m_fAspectRatio, m_fNear, m_fFar);
	return COMPV_ERROR_CODE_S_OK;
}
	
COMPV_ERROR_CODE CompVDrawingProjectionGLM::setNearFar(float near_ /*= 0.1f*/, float far_ /*= 100.f*/)
{
	m_fNear = near_;
	m_fFar = far_;
	**m_ptrMatrix = glm::perspective(glm::radians(m_fFOVY), m_fAspectRatio < 0.f ? 1.f : m_fAspectRatio, m_fNear, m_fFar);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDrawingProjectionGLM::reset()
{
	m_fFOVY = COMPV_MVP_PROJ_FOVY;
	m_fAspectRatio = COMPV_MVP_PROJ_ASPECT_RATIO;
	m_fNear = COMPV_MVP_PROJ_NEAR;
	m_fFar = COMPV_MVP_PROJ_FAR;
	**m_ptrMatrix = glm::perspective(glm::radians(m_fFOVY), m_fAspectRatio < 0.f ? 1.f : m_fAspectRatio, m_fNear, m_fFar);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDrawingProjectionGLM::newObj(CompVDrawingProjectionGLMPtrPtr proj)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(!proj, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVDrawingProjectionGLMPtr proj_ = new CompVDrawingProjectionGLM();
	COMPV_CHECK_EXP_RETURN(!proj_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	COMPV_CHECK_CODE_RETURN(CompVDrawingMat4fGLM::newObj(&proj_->m_ptrMatrix, glm::perspective(glm::radians(proj_->m_fFOVY), proj_->m_fAspectRatio < 0.f ? 1.f : proj_->m_fAspectRatio, proj_->m_fNear, proj_->m_fFar)));

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
	**m_ptrMatrix = COMPV_GLM_MAT4(m_ptrProjection) * COMPV_GLM_MAT4(m_ptrView) * COMPV_GLM_MAT4(m_ptrModel);
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

CompVDrawingProjectionPtr CompVMVPGLM::projection()
{
	return *m_ptrProjection;
}

COMPV_ERROR_CODE CompVMVPGLM::reset()
{
	COMPV_CHECK_CODE_RETURN(model()->reset());
	COMPV_CHECK_CODE_RETURN(view()->reset());
	COMPV_CHECK_CODE_RETURN(projection()->reset());
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMVPGLM::newObj(CompVMVPGLMPtrPtr mvp)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(!mvp, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMVPGLMPtr mvp_ = new CompVMVPGLM();
	COMPV_CHECK_EXP_RETURN(!mvp_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	COMPV_CHECK_CODE_RETURN(CompVDrawingModelGLM::newObj(&mvp_->m_ptrModel));
	COMPV_CHECK_CODE_RETURN(CompVDrawingViewGLM::newObj(&mvp_->m_ptrView));
	COMPV_CHECK_CODE_RETURN(CompVDrawingProjectionGLM::newObj(&mvp_->m_ptrProjection));
	COMPV_CHECK_CODE_RETURN(CompVDrawingMat4fGLM::newObj(&mvp_->m_ptrMatrix, COMPV_GLM_MAT4(mvp_->m_ptrProjection) * COMPV_GLM_MAT4(mvp_->m_ptrView) * COMPV_GLM_MAT4(mvp_->m_ptrModel)));

	*mvp = mvp_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

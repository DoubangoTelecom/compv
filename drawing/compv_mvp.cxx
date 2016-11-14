/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/compv_mvp.h"
#include "compv/drawing/compv_drawing.h"
#include "compv/drawing/opengl/compv_mvp_glm.h"

COMPV_NAMESPACE_BEGIN()

//
//	CompVDrawingMat4f
//

CompVDrawingMat4f::CompVDrawingMat4f()
{

}

CompVDrawingMat4f::~CompVDrawingMat4f()
{

}

//
//	CompVDrawingModel
//

CompVDrawingModel::CompVDrawingModel()
{

}

CompVDrawingModel::~CompVDrawingModel()
{

}

//
//	CompVDrawingView
//

CompVDrawingView::CompVDrawingView()
{

}

CompVDrawingView::~CompVDrawingView()
{

}

//
//	CompVDrawingProjection
//
CompVDrawingProjection::CompVDrawingProjection()
{

}

CompVDrawingProjection::~CompVDrawingProjection()
{
}

//
//	CompVMVP
//

CompVMVP::CompVMVP()
{

}

CompVMVP::~CompVMVP()
{

}

COMPV_ERROR_CODE CompVMVP::newObj(CompVMVPPtrPtr mvp)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(!mvp, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMVPPtr mvp_;

#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
	CompVMVPGLMPtr glmMVP_;
	COMPV_CHECK_CODE_RETURN(CompVMVPGLM::newObj(&glmMVP_));
	mvp_ = *glmMVP_;
#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

	COMPV_CHECK_EXP_RETURN(!(*mvp = mvp_), COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()


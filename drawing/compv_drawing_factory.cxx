/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/compv_drawing_factory.h"
#include "compv/drawing/skia/compv_canvas_skia.h"
#include "compv/gl/compv_gl_mvp.h"

COMPV_NAMESPACE_BEGIN()

//
//	CompVDrawingCanvasImpl
//
COMPV_ERROR_CODE CompVDrawingCanvasImpl::newObj(CompVCanvasImplPtrPtr canvasImpl)
{
	COMPV_CHECK_EXP_RETURN(!canvasImpl, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVCanvasImplPtr canvasImpl_;
#if defined(HAVE_SKIA)
	CompVCanvasImplSkiaPtr canvasImplSkia;
	COMPV_CHECK_CODE_RETURN(CompVCanvasImplSkia::newObj(&canvasImplSkia));
	canvasImpl_ = *canvasImplSkia;
#endif /* HAVE_SKIA */
	COMPV_CHECK_EXP_RETURN(!(*canvasImpl = canvasImpl_), COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}


//
//	CompVDrawingMVP
//
COMPV_ERROR_CODE CompVDrawingMVP::newObj2D(CompVMVPPtrPtr mvp)
{
	COMPV_CHECK_EXP_RETURN(!mvp, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMVPPtr mvp_;
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
	CompVGLMVPPtr glMVP_;
	COMPV_CHECK_CODE_RETURN(CompVGLMVP::newObj(&glMVP_, COMPV_PROJECTION_2D));
	mvp_ = *glMVP_;
#endif
	COMPV_CHECK_EXP_RETURN(!(*mvp = mvp_), COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDrawingMVP::newObj3D(CompVMVPPtrPtr mvp)
{
	COMPV_CHECK_EXP_RETURN(!mvp, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMVPPtr mvp_;
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
	CompVGLMVPPtr glMVP_;
	COMPV_CHECK_CODE_RETURN(CompVGLMVP::newObj(&glMVP_, COMPV_PROJECTION_3D));
	mvp_ = *glMVP_;
#endif
	COMPV_CHECK_EXP_RETURN(!(*mvp = mvp_), COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()


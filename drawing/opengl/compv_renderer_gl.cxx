/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/opengl/compv_renderer_gl.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/drawing/compv_drawing.h"

#include "compv/drawing/opengl/compv_renderer_gl_rgb.h"

COMPV_NAMESPACE_BEGIN()

CompVRendererGL::CompVRendererGL(COMPV_PIXEL_FORMAT ePixelFormat)
	: CompVRenderer(ePixelFormat)
{

}

CompVRendererGL::~CompVRendererGL()
{

}

COMPV_ERROR_CODE CompVRendererGL::newObj(CompVRendererGLPtrPtr glRenderer, COMPV_PIXEL_FORMAT ePixelFormat, const CompVSurface* surface)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(!glRenderer || !surface, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(!surface->isGLEnabled(), COMPV_ERROR_CODE_E_INVALID_PARAMETER); // Make sure both the window and surface have an GL context attached
	
	CompVRendererGLPtr glRenderer_;
	switch (ePixelFormat) {
	case COMPV_PIXEL_FORMAT_R8G8B8:
	{
		CompVRendererGLRgbPtr glRgbRenderer_;
		COMPV_CHECK_CODE_RETURN(CompVRendererGLRgb::newObj(&glRgbRenderer_, ePixelFormat, surface));
		glRenderer_ = dynamic_cast<CompVRendererGL*>(*glRgbRenderer_);
		break;
	}
	default:
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
		break;
	}
	
	COMPV_CHECK_EXP_RETURN(!(*glRenderer = glRenderer_), COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) ||defined(HAVE_OPENGLES) */
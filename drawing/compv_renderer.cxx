/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/compv_renderer.h"
#include "compv/drawing/compv_drawing.h"
#include "compv/drawing/compv_surface.h"

#include "compv/drawing/opengl/compv_renderer_gl.h"

COMPV_NAMESPACE_BEGIN()

compv_window_id_t CompVRenderer::s_nRendererId = 0;

CompVRenderer::CompVRenderer(COMPV_PIXEL_FORMAT ePixelFormat)
	: m_nId(compv_atomic_inc(&CompVRenderer::s_nRendererId))
	, m_ePixelFormat(ePixelFormat)
{
}

CompVRenderer::~CompVRenderer()
{

}

COMPV_ERROR_CODE CompVRenderer::newObj(CompVRendererPtrPtr renderer, COMPV_PIXEL_FORMAT ePixelFormat, const CompVSurface* surface)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(!renderer || !surface, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVRendererPtr renderer_;

#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
	if (surface->isGLEnabled()) {
		CompVRendererGLPtr glRenderer;
		COMPV_CHECK_CODE_RETURN(CompVRendererGL::newObj(&glRenderer, ePixelFormat, surface));
		renderer_ = *glRenderer;
	}
#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

	COMPV_CHECK_EXP_RETURN(!(*renderer = renderer_), COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
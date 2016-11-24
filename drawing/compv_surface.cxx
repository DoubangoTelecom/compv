/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/compv_surface.h"
#include "compv/drawing/compv_drawing.h"
#include "compv/drawing/compv_window.h"

#include "compv/drawing/opengl/compv_surface_gl.h"

COMPV_NAMESPACE_BEGIN()

compv_surface_id_t CompVSurface::s_nSurfaceId = 0;

CompVSurface::CompVSurface(size_t width, size_t height)
	: CompVObj()
	, m_nId(compv_atomic_inc(&CompVSurface::s_nSurfaceId))
	, m_nWidth(width)
	, m_nHeight(height)
	, m_bActive(true)
{
}

CompVSurface::~CompVSurface()
{

}

COMPV_ERROR_CODE CompVSurface::setViewport(CompVViewportPtr viewport)
{
	COMPV_CHECK_EXP_RETURN(!viewport, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	m_ptrViewport = viewport;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVSurface::newObj(CompVSurfacePtrPtr surface, const CompVWindow* window)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(!surface, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVSurfacePtr surface_;

#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
	CompVSurfaceGLPtr glSurface_;
	if (window->isGLEnabled()) {
		COMPV_CHECK_CODE_RETURN(CompVSurfaceGL::newObj(&glSurface_, window->width(), window->height()));
		surface_ = *glSurface_;
	}
#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

	COMPV_CHECK_EXP_RETURN(!(*surface = surface_), COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()


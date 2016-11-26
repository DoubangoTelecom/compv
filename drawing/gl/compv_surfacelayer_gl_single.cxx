/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/gl/compv_surfacelayer_gl_single.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/base/math/compv_math.h"
#include "compv/drawing/compv_drawing.h"
#include "compv/drawing/gl/compv_window_gl.h"

COMPV_NAMESPACE_BEGIN()

CompVSingleSurfaceLayerGL::CompVSingleSurfaceLayerGL()
{

}

CompVSingleSurfaceLayerGL::~CompVSingleSurfaceLayerGL()
{

}

// Overrides(CompVSingleSurfaceLayer)
CompVSurfacePtr CompVSingleSurfaceLayerGL::surface()
{
	return *m_ptrSurfaceGL;
}

// Overrides(CompVSurfaceLayer)
COMPV_ERROR_CODE CompVSingleSurfaceLayerGL::blit()
{
	COMPV_CHECK_EXP_RETURN(!m_ptrSurfaceGL, COMPV_ERROR_CODE_E_INVALID_STATE);
	if (m_ptrSurfaceGL->isActive() && m_ptrSurfaceGL->renderer()) {
		COMPV_CHECK_CODE_RETURN(m_ptrSurfaceGL->blitRenderer(kCompVGLPtrSystemFrameBuffer));
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVSingleSurfaceLayerGL::updateSize(size_t newWidth, size_t newHeight)
{
	COMPV_CHECK_EXP_RETURN(!m_ptrSurfaceGL, COMPV_ERROR_CODE_E_INVALID_STATE);
	COMPV_CHECK_CODE_RETURN(m_ptrSurfaceGL->updateSize(newWidth, newHeight));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVSingleSurfaceLayerGL::newObj(CompVSingleSurfaceLayerGLPtrPtr layer, size_t width, size_t height)
{
	COMPV_CHECK_EXP_RETURN(!layer || !width || !height, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVSingleSurfaceLayerGLPtr layer_ = new CompVSingleSurfaceLayerGL();
	COMPV_CHECK_EXP_RETURN(!layer_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	COMPV_CHECK_CODE_RETURN(CompVSurfaceGL::newObj(&layer_->m_ptrSurfaceGL, width, height));

	*layer = layer_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

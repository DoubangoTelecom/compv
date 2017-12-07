/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/compv_gl_surfacelayer_single.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/base/math/compv_math.h"

COMPV_NAMESPACE_BEGIN()

CompVGLSingleSurfaceLayer::CompVGLSingleSurfaceLayer()
{

}

CompVGLSingleSurfaceLayer::~CompVGLSingleSurfaceLayer()
{

}

// Overrides(CompVSurfaceLayer)
CompVSurfacePtr CompVGLSingleSurfaceLayer::cover()
{
    return *m_ptrSurfaceGL;
}

// Overrides(CompVSurfaceLayer)
COMPV_ERROR_CODE CompVGLSingleSurfaceLayer::blit()
{
    COMPV_CHECK_EXP_RETURN(!m_ptrSurfaceGL, COMPV_ERROR_CODE_E_INVALID_STATE);
    if (m_ptrSurfaceGL->isActive()) {
		// Blitters
		CompVGLBlitterPtr blitterRenderer = (m_ptrSurfaceGL->renderer() && m_ptrSurfaceGL->rendererGL())
			? m_ptrSurfaceGL->rendererGL()->blitter()
			: nullptr;
		CompVGLBlitterPtr blitterCover = m_ptrSurfaceGL->blitter();
		// FBOs
		CompVGLFboPtr fboRenderer = blitterRenderer
			? blitterRenderer->fbo()
			: nullptr;
		CompVGLFboPtr fboCover = blitterCover
			? blitterCover->fbo()
			: nullptr;
		// Blit()
		if (fboRenderer) {
			COMPV_CHECK_CODE_RETURN(m_ptrSurfaceGL->blit(fboRenderer, kCompVGLPtrSystemFrameBuffer));
		}
		if (fboCover) {
			COMPV_CHECK_CODE_RETURN(m_ptrSurfaceGL->blit(fboCover, kCompVGLPtrSystemFrameBuffer));
		}
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLSingleSurfaceLayer::updateSize(size_t newWidth, size_t newHeight)
{
    COMPV_CHECK_EXP_RETURN(!m_ptrSurfaceGL, COMPV_ERROR_CODE_E_INVALID_STATE);
    COMPV_CHECK_CODE_RETURN(m_ptrSurfaceGL->updateSize(newWidth, newHeight));
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLSingleSurfaceLayer::close()
{
    if (m_ptrSurfaceGL) {
        COMPV_CHECK_CODE_ASSERT(m_ptrSurfaceGL->close());
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLSingleSurfaceLayer::newObj(CompVGLSingleSurfaceLayerPtrPtr layer, size_t width, size_t height)
{
    COMPV_CHECK_EXP_RETURN(!layer || !width || !height, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVGLSingleSurfaceLayerPtr layer_ = new CompVGLSingleSurfaceLayer();
    COMPV_CHECK_EXP_RETURN(!layer_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    COMPV_CHECK_CODE_RETURN(CompVGLSurface::newObj(&layer_->m_ptrSurfaceGL, width, height));

    *layer = layer_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

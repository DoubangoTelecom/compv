/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/compv_gl_surfacelayer_multi.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/base/math/compv_math.h"

COMPV_NAMESPACE_BEGIN()

CompVGLMultiSurfaceLayer::CompVGLMultiSurfaceLayer()
{

}

CompVGLMultiSurfaceLayer::~CompVGLMultiSurfaceLayer()
{
    m_mapSurfaces.clear();
}

// Public API
COMPV_ERROR_CODE CompVGLMultiSurfaceLayer::addSurface(CompVSurfacePtrPtr surface, size_t width, size_t height, bool activate COMPV_DEFAULT(true)) /* Overrides(CompVMultiSurfaceLayer) */
{
    COMPV_CHECK_EXP_RETURN(!surface || !width || !height, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVGLSurfacePtr surface_;
    COMPV_CHECK_CODE_RETURN(CompVGLSurface::newObj(&surface_, width, height));
	if (!activate) {
		COMPV_CHECK_CODE_RETURN(surface_->deActivate());
	}
    m_mapSurfaces[surface_->id()] = surface_;
    *surface = *surface_;
    return COMPV_ERROR_CODE_S_OK;
}

// Public API
COMPV_ERROR_CODE CompVGLMultiSurfaceLayer::removeSurface(const CompVSurfacePtr surface) /* Overrides(CompVMultiSurfaceLayer) */
{
    COMPV_CHECK_EXP_RETURN(!surface, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    m_mapSurfaces.erase(surface->id());
    return COMPV_ERROR_CODE_S_OK;
}

// Public API
COMPV_ERROR_CODE CompVGLMultiSurfaceLayer::blit() /* Overrides(CompVMultiSurfaceLayer) */
{
    COMPV_CHECK_EXP_RETURN(!m_ptrCoverSurfaceGL, COMPV_ERROR_CODE_E_INVALID_STATE);
    COMPV_CHECK_CODE_RETURN(m_ptrCoverSurfaceGL->blitter()->requestFBO(m_ptrCoverSurfaceGL->width(), m_ptrCoverSurfaceGL->height()));
    CompVGLFboPtr fboCover = m_ptrCoverSurfaceGL->blitter()->fbo();
    for (std::map<compv_surface_id_t, CompVGLSurfacePtr>::iterator it = m_mapSurfaces.begin(); it != m_mapSurfaces.end(); ++it) {
        if (it->second->isActive()) {
			COMPV_CHECK_CODE_RETURN(it->second->blitRenderer(fboCover));
        }
    }
    if (m_ptrCoverSurfaceGL->isActive()) {
        COMPV_CHECK_CODE_RETURN(m_ptrCoverSurfaceGL->blit(fboCover, kCompVGLPtrSystemFrameBuffer));
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLMultiSurfaceLayer::updateSize(size_t newWidth, size_t newHeight)
{
    COMPV_CHECK_EXP_RETURN(!m_ptrCoverSurfaceGL, COMPV_ERROR_CODE_E_INVALID_STATE);
    COMPV_CHECK_CODE_RETURN(m_ptrCoverSurfaceGL->updateSize(newWidth, newHeight));

    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLMultiSurfaceLayer::close()
{
    if (m_ptrCoverSurfaceGL) {
        COMPV_CHECK_CODE_ASSERT(m_ptrCoverSurfaceGL->close());
    }
    for (std::map<compv_surface_id_t, CompVGLSurfacePtr>::iterator it = m_mapSurfaces.begin(); it != m_mapSurfaces.end(); ++it) {
        COMPV_CHECK_CODE_ASSERT(it->second->close());
    }
    m_mapSurfaces.clear();
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLMultiSurfaceLayer::newObj(CompVGLMultiSurfaceLayerPtrPtr layer, size_t width, size_t height)
{
    COMPV_CHECK_EXP_RETURN(!layer || !width || !height, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVGLMultiSurfaceLayerPtr layer_ = new CompVGLMultiSurfaceLayer();
    COMPV_CHECK_EXP_RETURN(!layer_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    COMPV_CHECK_CODE_RETURN(CompVGLSurface::newObj(&layer_->m_ptrCoverSurfaceGL, width, height));
#if 0 // Requires GL context
    COMPV_CHECK_CODE_RETURN(CompVGLFbo::newObj(&layer_->m_ptrFBO, width, height));
    COMPV_CHECK_CODE_RETURN(layer_->m_ptrCoverSurfaceGL->setCanvasFBO(layer_->m_ptrFBO));
#endif

    *layer = layer_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

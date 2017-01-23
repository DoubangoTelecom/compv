/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/compv_gl_surfacelayer_matching.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/base/math/compv_math.h"

COMPV_NAMESPACE_BEGIN()

static const size_t kTrainQuerySeparatorWidth = 32;

CompVGLMatchingSurfaceLayer::CompVGLMatchingSurfaceLayer()
{

}

CompVGLMatchingSurfaceLayer::~CompVGLMatchingSurfaceLayer()
{

}

COMPV_OVERRIDE_IMPL0("CompVMatchingSurfaceLayer", CompVGLMatchingSurfaceLayer::drawMatches)(CompVMatPtr trainImage, CompVMatPtr queryImage)
{
    COMPV_CHECK_EXP_RETURN(!trainImage || !queryImage || trainImage->isEmpty() || queryImage->isEmpty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    size_t coverWidth = trainImage->cols() + kTrainQuerySeparatorWidth + queryImage->cols();
    size_t coverHeight = COMPV_MATH_MAX(trainImage->rows(), queryImage->rows());

    // Create/FBO used in the cover surface
    COMPV_CHECK_CODE_RETURN(m_ptrCoverSurfaceGL->blitter()->requestFBO(coverWidth, coverHeight));

    if (m_ptrTrainSurfaceGL->isActive() && m_ptrQuerySurfaceGL->isActive() && m_ptrCoverSurfaceGL->isActive()) {
        CompVGLFboPtr fboCover = m_ptrCoverSurfaceGL->blitter()->fbo();
        // FIXME
        static const int trainPoint[] = { 463, 86 };
        static const int queryPoint[] = { 463, 86 };
        static int count = 0;
        char buff_[33] = { 0 };
        snprintf(buff_, sizeof(buff_), "%d", static_cast<int>(count++));
        std::string text = "Hello doubango telecom [" + std::string(buff_) + "]";

        const int offsetx = static_cast<int>(kTrainQuerySeparatorWidth + trainImage->cols());

        // TODO(dmi): This is too slow, maybe use two textures and program a shader

        // Draw Train points
        COMPV_CHECK_CODE_RETURN(m_ptrTrainSurfaceGL->drawImage(trainImage));
        COMPV_CHECK_CODE_RETURN(m_ptrTrainSurfaceGL->renderer()->canvas()->drawText(text.c_str(), text.length(), trainPoint[0], trainPoint[1]));
        COMPV_CHECK_CODE_RETURN(m_ptrTrainSurfaceGL->viewport()->reset(CompViewportSizeFlags::makeStatic(), 0, 0, static_cast<int>(trainImage->cols()), static_cast<int>(trainImage->rows())));
        COMPV_CHECK_CODE_RETURN(m_ptrTrainSurfaceGL->blitRenderer(fboCover));

        // Draw Query points
        COMPV_CHECK_CODE_RETURN(m_ptrQuerySurfaceGL->drawImage(queryImage));
        COMPV_CHECK_CODE_RETURN(m_ptrQuerySurfaceGL->renderer()->canvas()->drawText(text.c_str(), text.length(), queryPoint[0], queryPoint[1]));
        COMPV_CHECK_CODE_RETURN(m_ptrQuerySurfaceGL->viewport()->reset(CompViewportSizeFlags::makeStatic(), offsetx, 0, static_cast<int>(queryImage->cols()), static_cast<int>(queryImage->rows())));
        COMPV_CHECK_CODE_RETURN(m_ptrQuerySurfaceGL->blitRenderer(fboCover));

        // Draw lines
        COMPV_CHECK_CODE_RETURN(m_ptrCoverSurfaceGL->canvas()->drawLine(trainPoint[0], trainPoint[1], queryPoint[0] + offsetx, queryPoint[1]));
    }

    return COMPV_ERROR_CODE_S_OK;
}

COMPV_OVERRIDE_IMPL0("CompSurfaceLayer", CompVGLMatchingSurfaceLayer::blit)()
{
    if (m_ptrCoverSurfaceGL && m_ptrCoverSurfaceGL->isActive()) {
        CompVGLFboPtr fboCover = m_ptrCoverSurfaceGL->blitter()->fbo();
        COMPV_CHECK_CODE_RETURN(m_ptrCoverSurfaceGL->blit(fboCover, kCompVGLPtrSystemFrameBuffer));
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLMatchingSurfaceLayer::updateSize(size_t newWidth, size_t newHeight)
{
    COMPV_CHECK_EXP_RETURN(!m_ptrCoverSurfaceGL, COMPV_ERROR_CODE_E_INVALID_STATE);
    COMPV_CHECK_CODE_RETURN(m_ptrCoverSurfaceGL->updateSize(newWidth, newHeight));
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLMatchingSurfaceLayer::close()
{
    if (m_ptrCoverSurfaceGL) {
        COMPV_CHECK_CODE_NOP(m_ptrCoverSurfaceGL->close());
    }
    if (m_ptrTrainSurfaceGL) {
        COMPV_CHECK_CODE_NOP(m_ptrTrainSurfaceGL->close());
    }
    if (m_ptrQuerySurfaceGL) {
        COMPV_CHECK_CODE_NOP(m_ptrQuerySurfaceGL->close());
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLMatchingSurfaceLayer::newObj(CompVGLMatchingSurfaceLayerPtrPtr layer, size_t width, size_t height)
{
    COMPV_CHECK_EXP_RETURN(!layer || !width || !height, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVGLMatchingSurfaceLayerPtr layer_;
    layer_ = new CompVGLMatchingSurfaceLayer();
    COMPV_CHECK_EXP_RETURN(!layer_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    COMPV_CHECK_CODE_RETURN(CompVGLSurface::newObj(&layer_->m_ptrTrainSurfaceGL, width, height));
    COMPV_CHECK_CODE_RETURN(CompVGLSurface::newObj(&layer_->m_ptrQuerySurfaceGL, width, height));
    COMPV_CHECK_CODE_RETURN(CompVGLSurface::newObj(&layer_->m_ptrCoverSurfaceGL, width, height));

    *layer = layer_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

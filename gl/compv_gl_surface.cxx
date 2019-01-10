/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/compv_gl_surface.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/gl/compv_gl.h"
#include "compv/gl/compv_gl_common.h"
#include "compv/gl/compv_gl_utils.h"
#include "compv/gl/compv_gl_func.h"

static const std::string& kProgramVertexData =
#	if defined(HAVE_OPENGLES)
	"precision mediump float;"
#	endif
    "	attribute vec4 position;"
    "	attribute vec2 texCoord;"
    "	varying vec2 texCoordVarying;"
    "	uniform mat4 MVP;"
    "	void main() {"
    "		gl_Position = MVP * position;"
    "		texCoordVarying = texCoord;"
    "	}";

static const std::string& kProgramFragData =
#	if defined(HAVE_OPENGLES)
	"precision mediump float;"
#	endif
    "	varying vec2 texCoordVarying;"
    "	uniform sampler2D SamplerRGBA;"
    "	void main() {"
    "		gl_FragColor = texture2D(SamplerRGBA, texCoordVarying).rgba;"
    "	}";

COMPV_NAMESPACE_BEGIN()

CompVGLSurface::CompVGLSurface(size_t width, size_t height)
    : CompVSurface(width, height)
    , m_bInit(false)
{
}

CompVGLSurface::~CompVGLSurface()
{
    COMPV_CHECK_CODE_NOP(deInit());
}

CompVGLCanvasPtr CompVGLSurface::canvasGL()
{
	return m_ptrCanvas;
}

CompVGLRendererPtr CompVGLSurface::rendererGL()
{
	return m_ptrRenderer;
}

COMPV_ERROR_CODE CompVGLSurface::setMVP(CompVMVPPtr mvp) /*Overrides(CompVSurface)*/
{
    COMPV_CHECK_CODE_RETURN(m_ptrBlitter->setMVP(mvp));
    return COMPV_ERROR_CODE_S_OK;
}

CompVRendererPtr CompVGLSurface::renderer() /*Overrides(CompVSurface)*/
{
    if (m_ptrRenderer) {
        return *m_ptrRenderer;
    }
    return nullptr;
}

CompVCanvasPtr CompVGLSurface::canvas() /*Overrides(CompVSurface)*/
{
	return *m_ptrCanvas;
}

CompVCanvasPtr CompVGLSurface::requestCanvas(size_t width COMPV_DEFAULT(0), size_t height COMPV_DEFAULT(0)) /*Overrides(CompVSurface)*/
{
	if (!m_ptrCanvas) {
		COMPV_CHECK_EXP_BAIL(!CompVGLUtils::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
		COMPV_CHECK_CODE_BAIL(init());
		COMPV_CHECK_EXP_BAIL(!m_ptrBlitter->isInitialized(), COMPV_ERROR_CODE_E_INVALID_STATE);
		if (!m_ptrBlitter->fbo()) {
			if (!width) {
				width = m_ptrBlitter->width();
			}
			if (!height) {
				height = m_ptrBlitter->height();
			}
			COMPV_CHECK_CODE_BAIL(m_ptrBlitter->requestFBO(width, height));
		}
		COMPV_CHECK_CODE_BAIL(CompVGLCanvas::newObj(&m_ptrCanvas, m_ptrBlitter->fbo()));
	}
bail:
	return *m_ptrCanvas;
}

COMPV_ERROR_CODE CompVGLSurface::drawImage(const CompVMatPtr& mat, const CompVViewportPtr& viewport COMPV_DEFAULT(nullptr)) /*Overrides(CompVSurface)*/
{
	COMPV_CHECK_EXP_RETURN(!mat || mat->isEmpty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	COMPV_CHECK_CODE_RETURN(init());

	const COMPV_SUBTYPE pixelFormat = CompVGLUtils::subType(mat);
	if (!m_ptrRenderer || m_ptrRenderer->pixelFormat() != pixelFormat) {
		COMPV_CHECK_CODE_RETURN(CompVGLRenderer::newObj(&m_ptrRenderer, pixelFormat));
	}
	COMPV_CHECK_CODE_RETURN(m_ptrRenderer->drawImage(mat, viewport));

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLSurface::blit(const CompVGLFboPtr ptrFboSrc, const CompVGLFboPtr ptrFboDst)
{
    COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
    COMPV_CHECK_EXP_RETURN(!ptrFboSrc || (!ptrFboDst && ptrFboDst != kCompVGLPtrSystemFrameBuffer), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	CompVRectInt src, dst, view;
    COMPV_CHECK_CODE_BAIL(err = init());

    COMPV_CHECK_CODE_BAIL(err = m_ptrBlitter->bind()); // bind to VAO and activate the program
    if (ptrFboDst == kCompVGLPtrSystemFrameBuffer) {
        COMPV_glBindFramebuffer(GL_FRAMEBUFFER, kCompVGLNameSystemFrameBuffer);
        COMPV_glBindRenderbuffer(GL_RENDERBUFFER, kCompVGLNameSystemRenderBuffer);
    }
    else {
        COMPV_CHECK_CODE_BAIL(err = ptrFboDst->bind());
    }

    COMPV_glActiveTexture(GL_TEXTURE0);
    COMPV_glBindTexture(GL_TEXTURE_2D, ptrFboSrc->nameTexture());

	// Set viewport
	src.left = src.top = dst.left = dst.top = 0;
	src.right = static_cast<int>(ptrFboSrc ? ptrFboSrc->width() : width());
	src.bottom = static_cast<int>(ptrFboSrc ? ptrFboSrc->height() : height());
	dst.right = static_cast<int>(ptrFboDst ? ptrFboDst->width() : width());
	dst.bottom = static_cast<int>(ptrFboDst ? ptrFboDst->height() : height());
	COMPV_CHECK_CODE_BAIL(err = CompVViewport::viewport(src, dst, viewport(), &view));
	COMPV_glViewport(static_cast<GLsizei>(view.left), static_cast<GLsizei>(view.top), static_cast<GLsizei>(view.right - view.left), static_cast<GLsizei>(view.bottom - view.top));

	// Draw elements
    COMPV_glDrawElements(GL_TRIANGLES, m_ptrBlitter->indicesCount(), GL_UNSIGNED_BYTE, 0);

bail:
    COMPV_CHECK_CODE_NOP(m_ptrBlitter->unbind());
    if (ptrFboDst) {
        COMPV_CHECK_CODE_NOP(ptrFboDst->unbind());
    }
    COMPV_glActiveTexture(GL_TEXTURE0);
    COMPV_glBindTexture(GL_TEXTURE_2D, 0);
    return err;
}

COMPV_ERROR_CODE CompVGLSurface::blitRenderer(const CompVGLFboPtr ptrFboDst)
{
    COMPV_CHECK_EXP_RETURN(!m_ptrRenderer, COMPV_ERROR_CODE_E_INVALID_STATE, "No renderer associated to this surface");
    COMPV_CHECK_CODE_RETURN(blit(m_ptrRenderer->blitter()->fbo(), ptrFboDst));
    return COMPV_ERROR_CODE_S_OK;
}

// This update the destination size for blitting not the texture size
// FIXME: rename to something more obvious
COMPV_ERROR_CODE CompVGLSurface::updateSize(size_t newWidth, size_t newHeight)
{
    if (CompVSurface::m_nWidth != newWidth || CompVSurface::m_nHeight != newHeight) {
        COMPV_CHECK_CODE_RETURN(m_ptrBlitter->updateSize(newWidth, newHeight, newWidth));
        CompVSurface::m_nWidth = newWidth;
        CompVSurface::m_nHeight = newHeight;
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLSurface::close()
{
    if (m_ptrRenderer) {
        COMPV_CHECK_CODE_NOP(m_ptrRenderer->close());
    }
    if (m_ptrBlitter) {
        COMPV_CHECK_CODE_NOP(m_ptrBlitter->close());
    }
    COMPV_CHECK_CODE_NOP(deInit());
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLSurface::init()
{
    if (m_bInit) {
        return COMPV_ERROR_CODE_S_OK;
    }
    COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT); // Make sure we have a GL context
    m_bInit = true;
    COMPV_ERROR_CODE err;
    COMPV_CHECK_CODE_BAIL(err = m_ptrBlitter->init(CompVSurface::width(), CompVSurface::height(), CompVSurface::width(), kProgramVertexData, kProgramFragData, true/*haveMVP*/, true/*ToScreenYes*/));
bail:
    if (COMPV_ERROR_CODE_IS_NOK(err)) {
        COMPV_CHECK_CODE_NOP(deInit());
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLSurface::deInit()
{
    if (!m_bInit) {
        return COMPV_ERROR_CODE_S_OK;
    }
    COMPV_CHECK_EXP_RETURN(!CompVGLUtils::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT); // Make sure we have a GL context
    COMPV_CHECK_CODE_NOP(m_ptrBlitter->deInit());
    m_bInit = false;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLSurface::newObj(CompVGLSurfacePtrPtr glSurface, size_t width, size_t height)
{
    COMPV_CHECK_EXP_RETURN(!glSurface || !width || !height, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(!CompVGL::isInitialized(), COMPV_ERROR_CODE_E_NOT_INITIALIZED);

    CompVGLSurfacePtr glSurface_ = new CompVGLSurface(width, height);
    COMPV_CHECK_EXP_RETURN(!glSurface_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    COMPV_CHECK_CODE_RETURN(CompVGLBlitter::newObj(&glSurface_->m_ptrBlitter));
    COMPV_CHECK_CODE_RETURN(CompVViewport::newObj(&glSurface_->m_ptrViewport, CompViewportSizeFlags::makeDynamicAspectRatio(), static_cast<int>(0), static_cast<int>(0), static_cast<int>(width), static_cast<int>(height)));

    *glSurface = glSurface_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

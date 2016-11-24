/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/opengl/compv_surface_gl.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/drawing/compv_drawing.h"
#include "compv/drawing/compv_window.h"
#include "compv/drawing/compv_program.h"
#include "compv/drawing/compv_canvas.h"
#include "compv/drawing/opengl/compv_consts_gl.h"
#include "compv/drawing/opengl/compv_utils_gl.h"
#include "compv/gl/compv_gl_func.h"

// FIXME: OpenGL error handling not ok, impossible to find which function cause the error (erros stacked)

static const std::string& kProgramVertexData =
#	if defined(HAVE_OPENGLES)
"	precision mediump float;"
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
"	varying vec2 texCoordVarying;"
"	uniform sampler2D SamplerRGBA;"
"	void main() {"
"		gl_FragColor = texture2D(SamplerRGBA, texCoordVarying).rgba;"
"	}";

COMPV_NAMESPACE_BEGIN()

CompVSurfaceGL::CompVSurfaceGL(size_t width, size_t height)
	: CompVSurface(width, height)
	, m_bInit(false)
{
}

CompVSurfaceGL::~CompVSurfaceGL()
{
	COMPV_CHECK_CODE_ASSERT(deInit());
}

COMPV_ERROR_CODE CompVSurfaceGL::setMVP(CompVMVPPtr mvp)
{
	COMPV_CHECK_CODE_RETURN(CompVBlitterGL::setMVP(mvp));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVSurfaceGL::setViewport(CompVViewportPtr viewport)
{
	COMPV_CHECK_CODE_RETURN(CompVSurface::setViewport(viewport)); // Base class implementation
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVSurfaceGL::drawImage(CompVMatPtr mat, CompVRendererPtrPtr renderer /*= NULL*/)
{
#if 1
	COMPV_CHECK_EXP_RETURN(!mat || mat->isEmpty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	COMPV_CHECK_CODE_RETURN(init());

	// FIXME(dmi): remove if 'm_uNameFrameBuffer' is passed as parameter
	//--COMPV_CHECK_CODE_RETURN(m_ptrFBO->bind()); // Draw to framebuffer

	const COMPV_PIXEL_FORMAT pixelFormat = static_cast<COMPV_PIXEL_FORMAT>(mat->subType());
	if (!m_ptrRenderer || m_ptrRenderer->pixelFormat() != pixelFormat) {
		COMPV_CHECK_CODE_RETURN(CompVRendererGL::newObj(&m_ptrRenderer, pixelFormat));
		// FIXME: setViewport
	}
	COMPV_CHECK_CODE_RETURN(m_ptrRenderer->drawImage(mat));

	if (renderer) {
		*renderer = *m_ptrRenderer;
	}

#if 0
	COMPV_glBindFramebuffer(GL_FRAMEBUFFER, m_uNameFrameBuffer);
	uint8_t* data = (uint8_t*)malloc(640 * 480 * 4);
	COMPV_glReadBuffer(GL_COLOR_ATTACHMENT0);
	COMPV_glReadPixels(0, 0, 640, 480, GL_RGBA, GL_UNSIGNED_BYTE, data);
	FILE* file = fopen("C:/Projects/image.rgba", "wb+");
	fwrite(data, 1, (640 * 480 * 4), file);
	fclose(file);
	free(data);
	COMPV_glBindFramebuffer(GL_FRAMEBUFFER, 0); // System's framebuffer
#endif

	// FIXME(dmi): remove if 'm_uNameFrameBuffer' is passed as parameter
	//--COMPV_CHECK_CODE_RETURN(m_ptrFBO->unbind()); // Draw to system

#endif


	return COMPV_ERROR_CODE_S_OK;
}

// Overrides(CompVCanvas) 
COMPV_ERROR_CODE CompVSurfaceGL::canvasBind()
{
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	if (m_ptrCanvasFBO) {
		COMPV_CHECK_CODE_RETURN(m_ptrCanvasFBO->bind());
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_glBindFramebuffer(GL_FRAMEBUFFER, kCompVGLNameSystemFrameBuffer);
	COMPV_glBindRenderbuffer(GL_RENDERBUFFER, kCompVGLNameSystemRenderBuffer);
	return COMPV_ERROR_CODE_S_OK;
}

// Overrides(CompVCanvas) 
COMPV_ERROR_CODE CompVSurfaceGL::canvasUnbind()
{
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	if (m_ptrCanvasFBO) {
		COMPV_CHECK_CODE_RETURN(m_ptrCanvasFBO->unbind());
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_glBindFramebuffer(GL_FRAMEBUFFER, kCompVGLNameSystemFrameBuffer);
	COMPV_glBindRenderbuffer(GL_RENDERBUFFER, kCompVGLNameSystemRenderBuffer);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVSurfaceGL::blit(const CompVFBOGLPtr ptrFboSrc, const CompVFBOGLPtr ptrFboDst)
{
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	COMPV_CHECK_EXP_RETURN(!ptrFboSrc || (!ptrFboDst && ptrFboDst != kCompVGLPtrSystemFrameBuffer), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	COMPV_CHECK_CODE_BAIL(err = init());

	COMPV_CHECK_CODE_BAIL(err = CompVBlitterGL::bind()); // bind to VAO and activate the program
	if (ptrFboDst == kCompVGLPtrSystemFrameBuffer) {
		COMPV_glBindFramebuffer(GL_FRAMEBUFFER, kCompVGLNameSystemFrameBuffer);
		COMPV_glBindRenderbuffer(GL_RENDERBUFFER, kCompVGLNameSystemRenderBuffer);
	}
	else {
		COMPV_CHECK_CODE_BAIL(err = ptrFboDst->bind());
	}

	COMPV_glActiveTexture(GL_TEXTURE0);
	COMPV_glBindTexture(GL_TEXTURE_2D, ptrFboSrc->nameTexture());
	// FIXME: compute once
	
	{
		COMPV_DEBUG_INFO_CODE_FOR_TESTING(); // FIXME: compute once
		CompVDrawingRect rcViewport;
		const size_t dstWidth = ptrFboDst ? ptrFboDst->width() : CompVBlitterGL::width();
		const size_t dstHeight = ptrFboDst ? ptrFboDst->height() : CompVBlitterGL::height();
		COMPV_CHECK_CODE_BAIL(err = CompVViewport::viewport(
			CompVDrawingRect::makeFromWidthHeight(0, 0, static_cast<int>(ptrFboSrc->width()), static_cast<int>(ptrFboSrc->height())),
			CompVDrawingRect::makeFromWidthHeight(0, 0, static_cast<int>(dstWidth), static_cast<int>(dstHeight)),
			m_ptrViewport, &rcViewport));
		const GLsizei viewportW = static_cast<GLsizei>(rcViewport.right - rcViewport.left);
		const GLsizei viewportH = static_cast<GLsizei>(rcViewport.bottom - rcViewport.top);
		const GLsizei viewportX = static_cast<GLsizei>(rcViewport.left);
		const GLsizei viewportY = (ptrFboDst == kCompVGLPtrSystemFrameBuffer) ? rcViewport.top : static_cast<GLsizei>(CompVViewport::yFromBottomLeftToTopLeft(static_cast<int>(dstHeight), static_cast<int>(ptrFboSrc->height()), rcViewport.top));
		COMPV_glViewport(viewportX, viewportY, viewportW, viewportH);
	}

	//glViewport(0, 0, static_cast<GLsizei>(dstWidth), static_cast<GLsizei>(dstHeight));
	COMPV_glDrawElements(GL_TRIANGLES, CompVBlitterGL::indicesCount(), GL_UNSIGNED_BYTE, 0);

bail:
	COMPV_CHECK_CODE_ASSERT(CompVBlitterGL::unbind());
	if (ptrFboDst) {
		COMPV_CHECK_CODE_ASSERT(ptrFboDst->unbind());
	}
	COMPV_glActiveTexture(GL_TEXTURE0);
	COMPV_glBindTexture(GL_TEXTURE_2D, 0);
	return err;
}

COMPV_ERROR_CODE CompVSurfaceGL::blitRenderer(const CompVFBOGLPtr ptrFboDst)
{
	COMPV_CHECK_EXP_RETURN(!m_ptrRenderer, COMPV_ERROR_CODE_E_INVALID_STATE);
	COMPV_CHECK_CODE_RETURN(blit(m_ptrRenderer->fbo(), ptrFboDst));
	return COMPV_ERROR_CODE_S_OK;
}

// This update the destination size for blitting not the texture size
// FIXME: rename to something more evident
COMPV_ERROR_CODE CompVSurfaceGL::updateSize(size_t newWidth, size_t newHeight)
{
	CompVSurface::m_nWidth = newWidth;
	CompVSurface::m_nHeight = newHeight;
	COMPV_CHECK_CODE_RETURN(CompVBlitterGL::setSize(newWidth, newHeight, newWidth));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVSurfaceGL::setCanvasFBO(CompVFBOGLPtr fbo)
{
	m_ptrCanvasFBO = fbo;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVSurfaceGL::init()
{
	if (m_bInit) {
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT); // Make sure we have a GL context
	COMPV_CHECK_CODE_RETURN(CompVBlitterGL::init(CompVSurface::width(), CompVSurface::height(), CompVSurface::width(), kProgramVertexData, kProgramFragData, true/*haveMVP*/, true/*ToScreenYes*/)); // Base class implementation
	
	m_bInit = true;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVSurfaceGL::deInit()
{
	if (!m_bInit) {
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::isGLContextSet(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT); // Make sure we have a GL context
	COMPV_CHECK_CODE_RETURN(CompVBlitterGL::deInit()); // Base class implementation
	m_bInit = false;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVSurfaceGL::newObj(CompVSurfaceGLPtrPtr glSurface, size_t width, size_t height)
{
	COMPV_CHECK_EXP_RETURN(!glSurface || !width || !height, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());

	CompVSurfaceGLPtr glSurface_ = new CompVSurfaceGL(width, height);
	COMPV_CHECK_EXP_RETURN(!glSurface_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	COMPV_CHECK_CODE_RETURN(CompVViewport::newObj(&glSurface_->m_ptrViewport, CompViewportSizeFlags::makeDynamicAspectRatio()));
	
	*glSurface = glSurface_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

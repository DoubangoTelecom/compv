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

// FIXME: OpenGL error handling not ok, impossible to find which function cause the error (erros stacked)

static const std::string& kProgramVertexData =
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

CompVSurfaceGL::CompVSurfaceGL(int width, int height)
	: CompVSurface(width, height)
	, m_bInit(false)
	, m_bDirty(true)
	, m_bBeginDraw(false)
{

}

CompVSurfaceGL::~CompVSurfaceGL()
{
	COMPV_CHECK_CODE_ASSERT(deInit());
}

CompVMVPPtr CompVSurfaceGL::MVP()
{
	return CompVBlitterGL::MVP();
}

COMPV_ERROR_CODE CompVSurfaceGL::drawImage(CompVMatPtr mat, CompVRendererPtrPtr renderer /*= NULL*/)
{
#if 1
	COMPV_CHECK_EXP_RETURN(!mat || mat->isEmpty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(!m_bBeginDraw, COMPV_ERROR_CODE_E_INVALID_STATE);
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	COMPV_CHECK_CODE_RETURN(init());

	// FIXME(dmi): remove if 'm_uNameFrameBuffer' is passed as parameter
	//--COMPV_CHECK_CODE_RETURN(m_ptrFBO->bind()); // Draw to framebuffer

	const COMPV_PIXEL_FORMAT pixelFormat = static_cast<COMPV_PIXEL_FORMAT>(mat->subType());
	if (!m_ptrRenderer || m_ptrRenderer->pixelFormat() != pixelFormat) {
		COMPV_CHECK_CODE_RETURN(CompVRendererGL::newObj(&m_ptrRenderer, pixelFormat));
	}
	COMPV_CHECK_CODE_RETURN(m_ptrRenderer->drawImage(mat));

	if (renderer) {
		*renderer = *m_ptrRenderer;
	}

#if 0
	glBindFramebuffer(GL_FRAMEBUFFER, m_uNameFrameBuffer);
	uint8_t* data = (uint8_t*)malloc(640 * 480 * 4);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, 640, 480, GL_RGBA, GL_UNSIGNED_BYTE, data);
	FILE* file = fopen("C:/Projects/image.rgba", "wb+");
	fwrite(data, 1, (640 * 480 * 4), file);
	fclose(file);
	free(data);
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // System's framebuffer
#endif

	// FIXME(dmi): remove if 'm_uNameFrameBuffer' is passed as parameter
	//--COMPV_CHECK_CODE_RETURN(m_ptrFBO->unbind()); // Draw to system

#endif
	unmakeDirty();

	return COMPV_ERROR_CODE_S_OK;
}

// Overrides(CompVCanvas) 
COMPV_ERROR_CODE CompVSurfaceGL::canvasBind()
{
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Draw to system framebuffer
	return COMPV_ERROR_CODE_S_OK;
}

// Overrides(CompVCanvas) 
COMPV_ERROR_CODE CompVSurfaceGL::canvasUnbind()
{
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVSurfaceGL::beginDraw()
{
	COMPV_CHECK_EXP_RETURN(m_bBeginDraw, COMPV_ERROR_CODE_E_INVALID_STATE);
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	COMPV_CHECK_CODE_BAIL(err = init());
	// glBindFramebuffer(GL_FRAMEBUFFER, 0); // Draw to system framebuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	m_bBeginDraw = true;
	
bail:
	makeDirty();
	return err;
}

COMPV_ERROR_CODE CompVSurfaceGL::endDraw()
{
	COMPV_CHECK_EXP_RETURN(!m_bBeginDraw, COMPV_ERROR_CODE_E_INVALID_STATE);
	m_bBeginDraw = false;
	if (isDirty()) {
		COMPV_DEBUG_INFO("SurfaceGL with id = %u is dirty, do not draw!", getId());
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT);
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	COMPV_CHECK_CODE_BAIL(err = init());
	COMPV_CHECK_CODE_BAIL(err = CompVBlitterGL::bind()); // bind to VAO and activate the program
	// draw to current FB
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Draw to system buffer
	//glViewport(x_, y_, width_, height_);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_ptrFBO->nameTexture());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_ptrRenderer->fbo()->nameTexture());

	glViewport(0, 0, static_cast<GLsizei>(CompVBlitterGL::width()), static_cast<GLsizei>(CompVBlitterGL::height()));
	glDrawElements(GL_TRIANGLES, CompVBlitterGL::indicesCount(), GL_UNSIGNED_BYTE, 0);


bail:
	makeDirty();
	COMPV_CHECK_CODE_ASSERT(CompVBlitterGL::unbind());
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	return err;
}

COMPV_ERROR_CODE CompVSurfaceGL::init()
{
	if (m_bInit) {
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT); // Make sure we have a GL context
	COMPV_CHECK_CODE_RETURN(CompVBlitterGL::init(getWidth(), getHeight(), getWidth(), kProgramVertexData, kProgramFragData, true/*haveMVP*/, true/*ToScreenYes*/)); // Base class implementation
	m_bInit = true;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVSurfaceGL::deInit()
{
	if (!m_bInit) {
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_CHECK_EXP_RETURN(!CompVUtilsGL::haveCurrentContext(), COMPV_ERROR_CODE_E_GL_NO_CONTEXT); // Make sure we have a GL context
	COMPV_CHECK_CODE_RETURN(CompVBlitterGL::deInit()); // Base class implementation
	m_bInit = false;
	return COMPV_ERROR_CODE_S_OK;
}


COMPV_ERROR_CODE CompVSurfaceGL::newObj(CompVSurfaceGLPtrPtr glSurface, const CompVWindow* window)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(!glSurface || !window, COMPV_ERROR_CODE_E_INVALID_PARAMETER); // Check input pointers validity

	CompVSurfaceGLPtr glSurface_ = new CompVSurfaceGL(window->getWidth(), window->getHeight());
	COMPV_CHECK_EXP_RETURN(!glSurface_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	
	*glSurface = glSurface_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

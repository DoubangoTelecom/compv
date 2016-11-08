/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/opengl/compv_window_gl.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/drawing/compv_drawing.h"
#include "compv/drawing/opengl/compv_utils_gl.h"

COMPV_NAMESPACE_BEGIN()

CompVWindowGL::CompVWindowGL(int width, int height, const char* title)
	: CompVWindow(width, height, title)
	, CompVLock()
	, m_bDrawing(false)
{
	
}

CompVWindowGL::~CompVWindowGL()
{

}

bool CompVWindowGL::isInitialized()const
{
	return CompVLock::isInitialized(); // base class initialization check
}

// Overrides(CompVWindow::surface)
COMPV_ERROR_CODE CompVWindowGL::beginDraw()
{
	CompVAutoLock<CompVWindowGL>(this);
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	COMPV_CHECK_EXP_BAIL(m_bDrawing, (err = COMPV_ERROR_CODE_E_INVALID_STATE));
	COMPV_CHECK_CODE_BAIL(err = makeGLContextCurrent());

	glDisable(GL_DEPTH_TEST); // Required by Skia, otherwise we'll need to use 'glPushAttrib(GL_ALL_ATTRIB_BITS); glPopAttrib();' before/after canvas drawing
	glDisable(GL_BLEND);
	glViewport(0, 0, static_cast<GLsizei>(getWidth()), static_cast<GLsizei>(getHeight()));
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	if (m_ptrGLSurface) {
		COMPV_CHECK_CODE_BAIL(err = m_ptrGLSurface->clear());
	}

	m_bDrawing = true;
bail:
	return err;
}

// Overrides(CompVWindow::surface)
COMPV_ERROR_CODE CompVWindowGL::endDraw()
{
	CompVAutoLock<CompVWindowGL>(this);
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	COMPV_CHECK_EXP_BAIL(!m_bDrawing, (err = COMPV_ERROR_CODE_E_INVALID_STATE));
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Draw to system buffer

	if (m_ptrGLSurface) {
		COMPV_CHECK_CODE_BAIL(err = m_ptrGLSurface->blit());
	}

	COMPV_CHECK_CODE_BAIL(err = swapGLBuffers());

bail:
	m_bDrawing = false;
	COMPV_CHECK_CODE_ASSERT(unmakeGLContextCurrent());
	return err;
}

// Overrides(CompVWindow::surface)
CompVSurfacePtr CompVWindowGL::surface()
{
	CompVAutoLock<CompVWindowGL>(this);
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	COMPV_CHECK_EXP_BAIL(!m_bDrawing, (err = COMPV_ERROR_CODE_E_INVALID_STATE));
	if (!m_ptrGLSurface) {
		// Create the surface
		CompVSurfaceGLPtr glSurface_;
		COMPV_CHECK_CODE_BAIL(err = CompVSurfaceGL::newObj(&glSurface_, this));
		// If GL activation (enabled/disabled) must be the same on both the window and surace
		COMPV_CHECK_EXP_BAIL(isGLEnabled() != glSurface_->isGLEnabled(), err = COMPV_ERROR_CODE_E_INVALID_STATE);
		m_ptrGLSurface = glSurface_;
	}
	return *m_ptrGLSurface;
bail:
	return NULL;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

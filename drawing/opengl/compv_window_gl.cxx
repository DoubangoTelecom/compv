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

#define COMPV_THIS_CLASS_NAME "CompVWindowGL"

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
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Switch to system buffer
	// TODO(dmi): 'GL_DEPTH_TEST' not working with skia:  we need to use 'glPushAttrib(GL_ALL_ATTRIB_BITS); glPopAttrib();' before/after canvas drawing
	// 'GL_DEPTH_TEST' is needed for 3D projection
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glViewport(0, 0, static_cast<GLsizei>(getWidth()), static_cast<GLsizei>(getHeight())); // FIXME: width and height must be dynamic
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Clear surfaces
	for (std::vector<CompVSurfaceGLPtr >::iterator it = m_vecGLSurfaces.begin(); it != m_vecGLSurfaces.end(); ++it) {
		COMPV_CHECK_CODE_BAIL(err = (*it)->beginDraw());
	}

	m_bDrawing = true;
bail:
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		unmakeGLContextCurrent();
	}
	return err;
}

// Overrides(CompVWindow::surface)
COMPV_ERROR_CODE CompVWindowGL::endDraw()
{
	CompVAutoLock<CompVWindowGL>(this);
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	COMPV_CHECK_EXP_BAIL(!m_bDrawing, (err = COMPV_ERROR_CODE_E_INVALID_STATE));
	
	// Switch to system buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Blit (aka draw surfaces to back buffer)
	for (std::vector<CompVSurfaceGLPtr >::iterator it = m_vecGLSurfaces.begin(); it != m_vecGLSurfaces.end(); ++it) {
		COMPV_CHECK_CODE_BAIL(err = (*it)->endDraw());
	}

	// Swap (aka 'present' the final redering to the window, means switch front/back buffers)
	COMPV_CHECK_CODE_BAIL(err = swapGLBuffers());

bail:
	m_bDrawing = false;
	COMPV_CHECK_CODE_ASSERT(unmakeGLContextCurrent());
	return err;
}

// Overrides(CompVWindow::surface)
size_t CompVWindowGL::numSurface()
{
	CompVAutoLock<CompVWindowGL>(this);
	return m_vecGLSurfaces.size();
}

COMPV_ERROR_CODE CompVWindowGL::removeAllSurfaces()
{
	CompVAutoLock<CompVWindowGL>(this);
	m_vecGLSurfaces.clear();
	return COMPV_ERROR_CODE_S_OK;
}

// Overrides(CompVWindow::surface)
COMPV_ERROR_CODE CompVWindowGL::addSurface()
{
	CompVAutoLock<CompVWindowGL>(this);
	CompVSurfaceGLPtr glSurface_;
	COMPV_CHECK_CODE_RETURN(CompVSurfaceGL::newObj(&glSurface_, this));
	m_vecGLSurfaces.push_back(glSurface_);
	return COMPV_ERROR_CODE_S_OK;
}

// Overrides(CompVWindow::surface)
COMPV_ERROR_CODE CompVWindowGL::removeSurface(size_t index)
{
	CompVAutoLock<CompVWindowGL>(this);
	COMPV_CHECK_EXP_RETURN(index >= m_vecGLSurfaces.size(), COMPV_ERROR_CODE_E_OUT_OF_BOUND);
	m_vecGLSurfaces.erase(m_vecGLSurfaces.begin() + index);
	return COMPV_ERROR_CODE_S_OK;
}

// Overrides(CompVWindow::surface)
CompVSurfacePtr CompVWindowGL::surface(size_t index /*= 0*/)
{
	CompVAutoLock<CompVWindowGL>(this);
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	COMPV_CHECK_EXP_BAIL(index >= m_vecGLSurfaces.size(), err = COMPV_ERROR_CODE_E_OUT_OF_BOUND);
	return *m_vecGLSurfaces[index];
bail:
	return NULL;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

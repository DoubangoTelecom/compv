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

// FIXME: remove layers when window is closed


//
//	CompVWindowGL
//

CompVWindowGL::CompVWindowGL(size_t width, size_t height, const char* title)
	: CompVWindowPriv(width, height, title)
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
	COMPV_CHECK_EXP_BAIL(m_bDrawing || !context(), (err = COMPV_ERROR_CODE_E_INVALID_STATE));
	COMPV_CHECK_CODE_BAIL(err = context()->makeCurrent());
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Switch to system buffer
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	// TODO(dmi): 'GL_DEPTH_TEST' not working with skia:  we need to use 'glPushAttrib(GL_ALL_ATTRIB_BITS); glPopAttrib();' before/after canvas drawing
	// 'GL_DEPTH_TEST' is needed for 3D projection
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glViewport(0, 0, static_cast<GLsizei>(CompVWindow::width()), static_cast<GLsizei>(CompVWindow::height())); // FIXME: width and height must be dynamic
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Clear surfaces
	//for (std::vector<CompVSurfaceGLPtr >::iterator it = m_vecGLSurfaces.begin(); it != m_vecGLSurfaces.end(); ++it) {
	//	COMPV_CHECK_CODE_BAIL(err = (*it)->beginDraw());
	//}

	m_bDrawing = true;
bail:
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_CHECK_CODE_ASSERT(context()->unmakeCurrent());
	}
	return err;
}

// Overrides(CompVWindow::surface)
COMPV_ERROR_CODE CompVWindowGL::endDraw()
{
	CompVAutoLock<CompVWindowGL>(this);
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	COMPV_CHECK_EXP_BAIL(!m_bDrawing || !context(), (err = COMPV_ERROR_CODE_E_INVALID_STATE));
	COMPV_CHECK_EXP_BAIL(!CompVContextGL::isSet(), (err = COMPV_ERROR_CODE_E_GL_NO_CONTEXT));

	// Swap (aka 'present' the final redering to the window, means switch front/back buffers)
	COMPV_CHECK_CODE_BAIL(err = context()->swabBuffers());

bail:
	m_bDrawing = false;
	COMPV_CHECK_CODE_ASSERT(context()->unmakeCurrent());
	return err;
}

// Overrides(CompVWindow::newSingleLayerSurface)
COMPV_ERROR_CODE CompVWindowGL::addSingleLayerSurface(CompVSingleSurfaceLayerPtrPtr layer)
{
	CompVAutoLock<CompVWindowGL>(this);
	COMPV_CHECK_EXP_RETURN(!layer, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVSingleSurfaceLayerGLPtr layer_;
	COMPV_CHECK_CODE_RETURN(CompVSingleSurfaceLayerGL::newObj(&layer_, this));
	m_mapSingleSurfaceLayers[layer_->id()] = layer_;
	*layer = *layer_;
	return COMPV_ERROR_CODE_S_OK;
}

// Overrides(CompVWindow::removeSingleLayerSurface)
COMPV_ERROR_CODE CompVWindowGL::removeSingleLayerSurface(const CompVSingleSurfaceLayerPtr& layer)
{
	CompVAutoLock<CompVWindowGL>(this);
	COMPV_CHECK_EXP_RETURN(!layer, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	m_mapSingleSurfaceLayers.erase(layer->id());
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindowGL::addMatchingLayerSurface(CompVMatchingSurfaceLayerPtrPtr layer)
{
	CompVAutoLock<CompVWindowGL>(this);
	COMPV_CHECK_EXP_RETURN(!layer, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMatchingSurfaceLayerGLPtr layer_;
	COMPV_CHECK_CODE_RETURN(CompVMatchingSurfaceLayerGL::newObj(&layer_, this));
	m_mapMatchingSurfaceLayers[layer_->id()] = layer_;
	*layer = *layer_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVWindowGL::removeMatchingLayerSurface(const CompVMatchingSurfaceLayerPtr& layer)
{
	CompVAutoLock<CompVWindowGL>(this);
	COMPV_CHECK_EXP_RETURN(!layer, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	m_mapMatchingSurfaceLayers.erase(layer->id());
	return COMPV_ERROR_CODE_S_OK;
}

// Overrides(CompVWindowPriv)
COMPV_ERROR_CODE CompVWindowGL::priv_updateSize(size_t newWidth, size_t newHeight)
{
	CompVAutoLock<CompVWindowGL>(this);
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	
	COMPV_CHECK_CODE_BAIL(err = context()->makeCurrent());

	CompVWindow::m_nWidth = newWidth;
	CompVWindow::m_nHeight = newHeight;

	// COMPV_CHECK_CODE_BAIL(err = beginDraw()); // TODO(dmi): deadlock
	//for (std::vector<CompVSurfaceGLPtr >::iterator it = m_vecGLSurfaces.begin(); it != m_vecGLSurfaces.end(); ++it) {
	//	COMPV_CHECK_CODE_BAIL(err = (*it)->updateSize(newWidth, newHeight));
	//}
	for (std::map<compv_surfacelayer_id_t, CompVSingleSurfaceLayerGLPtr>::iterator it = m_mapSingleSurfaceLayers.begin(); it != m_mapSingleSurfaceLayers.end(); ++it) {
		COMPV_CHECK_CODE_BAIL(err = it->second->updateSize(newWidth, newHeight));
	}

	for (std::map<compv_surfacelayer_id_t, CompVMatchingSurfaceLayerGLPtr>::iterator it = m_mapMatchingSurfaceLayers.begin(); it != m_mapMatchingSurfaceLayers.end(); ++it) {
		COMPV_CHECK_CODE_BAIL(err = it->second->updateSize(newWidth, newHeight));
	}

	//COMPV_CHECK_CODE_BAIL(err = endDraw()); // blit all surfaces using up-to-data sizes

	for (std::map<compv_windowlistener_id_t, CompVWindowListenerPtr>::iterator it = m_mapListeners.begin(); it != m_mapListeners.end(); ++it) {
		COMPV_CHECK_CODE_BAIL(err = it->second->onSizeChanged(newWidth, newHeight));
	}
	
bail:
	COMPV_CHECK_CODE_ASSERT(err = context()->unmakeCurrent());
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */
